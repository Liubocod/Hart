#include	"main.h"
#include	"usart.h"
#include	"Eeprom.h"
#include	"EchoHandle.h"
#include 	"HartHandle.h"
#include 	"DistanceDataHandle.h"
#include	"perf_management.h"
#include	"IWR1443.h"

#define ASSERT_HART_WRITE_PROTECT(INSTANCE) (INSTANCE == 0x5a)||(INSTANCE == 0xa5)
/* 长帧地址断言函数，判断请求帧地址是否与本地地址匹配 */
#define HART_UNIQUE_ADDRESS_ASSERT()   ((TempC == (DEVICE_TYPE0 & 0x3f))\
                                  &&(HartDataBuffer[2] == DEVICE_TYPE)\
                                      &&(HartDataBuffer[3] == TempAddress)\
                                          &&(HartDataBuffer[4] == DEVICE_ID1)\
                                            &&(HartDataBuffer[5] == DEVICE_ID2))
#define IS_BROADCAST_ADDRESS   (((HartDataBuffer[1] == 0)\
                                    &&(HartDataBuffer[2] == 0)\
                                      &&(HartDataBuffer[3] == 0)\
                                          &&(HartDataBuffer[4] == 0)\
                                            &&(HartDataBuffer[5] == 0))\
                                              &&(HartDataBuffer[6] == 0X0B))

#define IS_VALID_SHORT_F_DELIMITER        (HartDataBuffer[0]== SHORTFRAME_M)\
                                          ||(HartDataBuffer[0]== SHORTFRAME_VALID1)\
                                            ||(HartDataBuffer[0]== SHORTFRAME_VALID2)\
                                              ||(HartDataBuffer[0]== SHORTFRAME_VALID3)
#define IS_VALID_LONG_F_DELIMITER        (HartDataBuffer[0]== LONGFRAME_M)\
                                          ||(HartDataBuffer[0]== LONGFRAME_VALID1)\
                                            ||(HartDataBuffer[0]== LONGFRAME_VALID2)\
                                              ||(HartDataBuffer[0]== LONGFRAME_VALID3)
#define IS_PRIMARY_MASTER                ((HartDataBuffer[1]>>7&0x01) == 0x01)
                                                
#define IS_SECOND_MASTER                 ((HartDataBuffer[1]>>7&0x01) == 0)

/* 通信错误状态字节宏定义 */    
typedef enum
{
  NoCommunicationErr = 0,
  CommunicationMask = 0x80,
  VerticalParityError = 0x40,
  OverrunError = 0x20,
  FramingError = 0x10,
  LongitudinalParityError = 0x08,
  CommunicationFailure = 0x04,
  BufferOverflow = 0x02,
}HartCommunicationErrStatusDef;


/* 通信错误状态字节宏定义 */    
typedef enum
{
  InvalidSelection = 2,
  VariableCodeInvalidSelection = 2,
  PassedParameterTooLarge = 3,
  PassedParameterTooSmall = 4,
  TooFewDataBytesReceived = 5,
  ConfigChangeCounterMismatch = 9,
  InvalidDateCodeDetected = 9,
  IncorrectLoopCurrentModeorValue = 9,
  LowerRangeValueTooHigh = 9,
  LowerRangeValueTooLow = 10,
  LoopCurrentNotActive = 11,
  UpperRangeValueTooHigh = 11,
  UpperRangeValueTooLow = 12,
  UpperAndLowerRangeValuesOutOfLimits = 13,
  Statusbytesmismatch = 14,
  InvalidUnitsCode = 18,
  InvalidSpan = 29,
}HartSingleDefinitionErrorDef;

typedef enum
{
  High = 0,
  Low = 1,
  HoldLastOutputValue = 239,
  NotUsed = 250,
  None = 251,
  Unknown = 252,
  Special = 253
}AlarmSelectionCodesDef;
typedef enum
{
  RespondCodeNormal = 0,
  DeviceSpecificCommandError = 6,
  CommandNotExcuted = 64,
  WriteProtectMode = 7,
}CommonRespondCodeDef;
/* 链路层帧解析字段 */
typedef enum
{
  Preamble,
  FrameReceive,
  FrameDelimiterAnalysis,
  FrameGapError,
  FrameCountCorrectly,
  FrameResponding,
  FrameRespondOver,
}LinkLayerFieldDef;

typedef enum
{
  ShortAddressIndex = 2,
  LongAddressIndex = 6,
}StartAddressDef;
//响应码分为正常，报警，以及错误
typedef enum
{
  Notification,
  Warning,
  Error,
}RespondCodeTypeDef;
//响应码状态定义
typedef struct 
{
  uint8_t value;
  RespondCodeTypeDef Type;
}HartRespondCodeDef;

//设备变量表，命令9
typedef struct 
{
  uint8_t DeviceVariableIndex;
  uint8_t DeviceVariableClassification;
  uint8_t UnitsCode;
  uint8_t ValueIndex;
  uint8_t DeviceVariableStatus;
}DeviceVariableTableDef;
uint8_t	HartDataBuffer[RXBUFFERSIZE_HART];
StartAddressDef DataFieldStartAddress;
uint8_t TxRxCount,TxNumber,DelimiterFlag = 0,FrameByteNuber;
uint16_t HartWorkCount;
void	HartModifyCurrent(uint8_t TempModifyComMode);
static void RespondCodeAndDeviceStatusHandle(CommonRespondCodeDef HandleFlag,
                                                        uint8_t ExactByteNumber,bool8 IsReadCmd);

static bool8 HartRFrameTrueHandle();
static void HartDataAnanlysisHandle();
static bool8 Hart_ReceiveCompleteHandle();

static bool8 ParityErrorCheck();
static void ParityErrorRespondHandle();
static void CommunicationErrorResponseModeSelect(bool8 IsResponse);
static void ParityErrorClear();

static bool8 FramingErrorCheck();
static void FramingErrorRespondHandle();
static void FramingErrorClear();

static void HartDelimterConfig();
static void HartBurstModeConfig(bool8 IsBurst);
static void HartRespondCodeConfig(RespondCodeTypeDef Type,uint8_t value);
static void HartDeviceStatusVarify(void );
static void HART_CMD0Config();
static void HartConfigChangeFlagReset();
static void HartDeviceStatusConfig(uint8_t flag,HartDeviceStatusDef DeviceStatus);
/* 回路电流模式设备状态核实更新函数，一旦电流模式发生改变，立即更新到设备状态字节上 */
static void HartLoopCurrentModeVarify();
static void HartConfigChangeFlagCountIncrease();
static void HartCmd12Config();
static void HartCmd13Config();
static void HartCmd16Config();
static void HartCmd20Config();
static uint8_t HartConfigChangeFlagHandle(bool8 IsClear,uint8_t Primarycoldstartflag);
static void HartConfigChangeFlagSave(bool8 IsReset);
static void HartCmd9UndefineVariableHandle(uint8_t VariableIndex,uint8_t i);
static void HartCmd9DefineVariableHandle(uint8_t VariableIndex,uint8_t i);
static void HartCmd9FixedVariableHandle(uint8_t VariableIndex,uint8_t i);
static void HartMoreStatusAvailableModeVarify();
static void HARTCMD133Config(void);
static void HARTCMD135Config();
static void HARTCMD133Config(void);
static void HARTCMD139Config();
static void HARTCMD137Config();
static void HARTCMD143Config();
static RealUnitCodeDef HartUnitCodeConvertToRealCode(HartUnitDef HartUnitCode);
static HartUnitDef RealCodeConvertToHartUnitCode(RealUnitCodeDef RealUnitCode);
static void HART_GapCounterDecrease();


union
{
  float TempF;
  unsigned char TempChar[4];
}Hart_FloatToChar;

LinkLayerFieldDef G_FrameFieldStatus = Preamble;
HartRespondCodeDef G_HartRespondCodeStatus = {0,Notification};  
HartDeviceStatusDef G_HartDeviceStatus = FieldDeviceNormal;
/* 接收帧字节间隔计数器，用于判断是否出现数据流丢失 */                                                
uint32_t G_FrameGapCounter = 0;  
 
/* HART 命令9需要设备变量信息，因此在这里定义设备变量表，记录设备变量基本信息 */
const DeviceVariableTableDef DeviceVariableTable[] =
{
  0,69,45,0,0,//PV
  //1,84,39,3,0,//电流
};
//Premary master Status cached value.
uint8_t G_PMasterStatusCachedValue = 0;
//Secondary master Status cached value.
uint8_t G_SMasterStatusCachedValue = 0;
//More Status Available Bit value.
uint8_t G_MoreStatusAvailableBit = 0;
uint8_t HartReceiveData = 0;

//---------------------------------------------------------------------------------
//从机响应码处理函数，响应主机时需要提供响应码，如果响应码出现异常，可通过本函数进行配置。
static void HartRespondCodeConfig(RespondCodeTypeDef Type,uint8_t value)
{
  G_HartRespondCodeStatus.Type = Type;
  G_HartRespondCodeStatus.value = value;
}
uint8_t	ReckonHartCRC(uint8_t CRCNumber)
{
	uint8_t TempC,CheckCode;
	CheckCode = 0xFF;
	for(TempC = 0; TempC < CRCNumber; TempC++)
	{
		CheckCode ^= HartDataBuffer[TempC];
	}
	CheckCode = (~CheckCode);
        return CheckCode;
}

void	EnableHartRx(void)
{
  HAL_StatusTypeDef Result;
  ENABLE_HART_RX;
  DelimiterFlag = 0;
  TxRxCount = 0;
  G_FrameFieldStatus = Preamble;
  FrameByteNuber = 0;
  Result = HAL_UART_Receive_IT(&huart1, &HartReceiveData, 1);
  if(Result != HAL_OK)
  {
    //printf
  }
}

void	EnableHartTx(void)
{
  uint8_t TempC;
  ENABLE_HART_TX;
  TxNumber = HartDataBuffer[DataFieldStartAddress+1]+2+DataFieldStartAddress;
  HartDataBuffer[TxNumber] = ReckonHartCRC(TxNumber);
  for(TempC = TxNumber; TempC > 0; TempC--)
  {
          HartDataBuffer[TempC+DEFAULT_PREAM+1] = HartDataBuffer[TempC];
  }
  HartDataBuffer[DEFAULT_PREAM+1] = HartDataBuffer[0];
  for(TempC = 0; TempC <= DEFAULT_PREAM; TempC++)
          HartDataBuffer[TempC] = 0xFF;
  TxNumber = TxNumber+DEFAULT_PREAM+2;
  
  HAL_UART_Transmit_IT(&huart1, (uint8_t *)HartDataBuffer, TxNumber);
  G_FrameFieldStatus = FrameResponding;
}

void	FloatToTFrameChar(float FloatVar,uint8_t DataIndex)
{
	uint8_t *point_Float;
	point_Float = (uint8_t *)&FloatVar;
	
	HartDataBuffer[DataIndex] = point_Float[3];
	HartDataBuffer[DataIndex+1] = point_Float[2];
	HartDataBuffer[DataIndex+2] = point_Float[1];
	HartDataBuffer[DataIndex+3] = point_Float[0];	
}

float	*RFrameCharToFloat (uint8_t DataIndex )//tranfer 4 char variables at the proper location of the struct Hart_TFrame into 1 float variable 
{
	Hart_FloatToChar.TempChar[3] = HartDataBuffer[DataIndex];
	Hart_FloatToChar.TempChar[2] = HartDataBuffer[DataIndex+1];
	Hart_FloatToChar.TempChar[1] = HartDataBuffer[DataIndex+2];
	Hart_FloatToChar.TempChar[0] = HartDataBuffer[DataIndex+3];
	return &Hart_FloatToChar.TempF;
}

void	ReadParaF_Hart(uint16_t ParaAddress, uint8_t *ParaValueP)
{
	uint8_t TempC;
	GetPara_Float(ParaValueP,ParaAddress);
	TempC = ParaValueP[0];
	ParaValueP[0] = ParaValueP[3];
	ParaValueP[3] = TempC;
	TempC = ParaValueP[1];
	ParaValueP[1] = ParaValueP[2];	
	ParaValueP[2] = TempC;	
}
/*命令0响应配置处理函数，用来配置设备基本信息*/
static void HART_CMD0Config()
{
  HartDataBuffer[DataFieldStartAddress+4] = 0xFE;
  HartDataBuffer[DataFieldStartAddress+5] = DEVICE_TYPE0;//MANUFACTURE_ID;
  HartDataBuffer[DataFieldStartAddress+6] = DEVICE_TYPE;
  HartDataBuffer[DataFieldStartAddress+7] = DEFAULT_PREAM;
  HartDataBuffer[DataFieldStartAddress+8] = UNIV_CMD_REV;
  HartDataBuffer[DataFieldStartAddress+9] = TRANS_SPEC_REV;
  HartDataBuffer[DataFieldStartAddress+10] = SOFT_REV;
  HartDataBuffer[DataFieldStartAddress+11] = HARD_REV;
  HartDataBuffer[DataFieldStartAddress+12] = FLAGS;
  HartDataBuffer[DataFieldStartAddress+13] = DEVICE_ID0;
  HartDataBuffer[DataFieldStartAddress+14] = DEVICE_ID1;
  HartDataBuffer[DataFieldStartAddress+15] = DEVICE_ID2;
  HartDataBuffer[DataFieldStartAddress+16] = DEFAULT_PREAM;
  HartDataBuffer[DataFieldStartAddress+17] = 00;
  
  HartDataBuffer[DataFieldStartAddress+18] = *GetPara_Char(ConfigChangeCounter);
  HartDataBuffer[DataFieldStartAddress+19] = *(GetPara_Char(ConfigChangeCounter)+1);
  //需要与48号命令的Extended Field Device Status保持一致。
  HartDataBuffer[DataFieldStartAddress+20] = 0x11;

  HartDataBuffer[DataFieldStartAddress+21] = MANUFACTURE_ID0;
  HartDataBuffer[DataFieldStartAddress+22] = MANUFACTURE_ID;
  HartDataBuffer[DataFieldStartAddress+23] = MANUFACTURE_ID0;
  HartDataBuffer[DataFieldStartAddress+24] = MANUFACTURE_ID;
  HartDataBuffer[DataFieldStartAddress+25] = 1;
}
/*返回设备类型代码，版本和设备标识码*/
bool8	HARTCMD0(void)//READ UNIQUE IDENTIFIER
{
  HART_CMD0Config();
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,24,TRUE);
  return TRUE;
}

/*返回主变量（料高数据类型浮点数）*/
bool8	HARTCMD1(void)//READ PRIMARY VARIABLE
{
  HartDataBuffer[DataFieldStartAddress+4] = (uint8_t)RealCodeConvertToHartUnitCode((RealUnitCodeDef)*GetPara_Char(PVUnitIndex));//HART_Variables.PVUnit;
  FloatToTFrameChar(HartReadMasterVariable(0),DataFieldStartAddress+5);
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,7,TRUE);
  return TRUE;
}

/*返回电流输出值和百分比（数据类型浮点数）*/
bool8	HARTCMD2(void)//READ P. V. CURRENT AND PERCENT OF RANGE
{
  float SetOutCurrentValue = 0;
  if(*GetPara_Char(LoopCurrentModeIndex) == 0)
  {
    SetOutCurrentValue = SetCurrentGet();
    SetOutCurrentVal(SetOutCurrentValue);
    FloatToTFrameChar(SetOutCurrentValue,DataFieldStartAddress+4);
  }
  else
  {
    FloatToTFrameChar(HartReadMasterVariable(3),DataFieldStartAddress+4);
  }
  FloatToTFrameChar(HartReadMasterVariable(2),DataFieldStartAddress+8);
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,10,TRUE);
  return TRUE;
}

/*返回输出电流、距离单位、距离值（空高）、*/
bool8	HARTCMD3(void)//READ DYNAMIC VARIABLES AND P. V. CURRENT
{
  float SetOutCurrentValue = 0;
  if(*GetPara_Char(LoopCurrentModeIndex) == 0)
  {
    SetOutCurrentValue = SetCurrentGet();
    SetOutCurrentVal(SetOutCurrentValue);
    FloatToTFrameChar(SetOutCurrentValue,DataFieldStartAddress+4);
  }
  else
  {
    FloatToTFrameChar(HartReadMasterVariable(3),DataFieldStartAddress+4);
  }
  HartDataBuffer[DataFieldStartAddress+8] = (uint8_t)RealCodeConvertToHartUnitCode((RealUnitCodeDef)*GetPara_Char(PVUnitIndex));
  FloatToTFrameChar(HartReadMasterVariable(0),DataFieldStartAddress+9);
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,11,TRUE);
  return TRUE;
}
/*更改通信地址，通信模式*/
bool8	HARTCMD6(void)//address
{
  if(HartDataBuffer[DataFieldStartAddress+1] == 0)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else if(HartDataBuffer[DataFieldStartAddress+1] == 1)
  {
    SavePara_CharType(MODIFYPARAMODE_HART,7,HartDataBuffer[DataFieldStartAddress+2]);
    if(HartDataBuffer[DataFieldStartAddress+2] == 0)
    {
      SavePara_CharType(MODIFYPARAMODE_HART,LoopCurrentModeIndex,1);
      HartDeviceStatusByteHandle(LoopCurrentFixed,TRUE);
    }
    else if(HartDataBuffer[DataFieldStartAddress+2] < 64)
    {
      //多点模式需要固定输出4毫安电流，同时设置回路电流状态值
      SavePara_CharType(MODIFYPARAMODE_HART,LoopCurrentModeIndex,0);
      HartDeviceStatusByteHandle(LoopCurrentFixed,FALSE);
      SetOutCurrentVal(4.0);
    }
    HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(ComAddressIndex);
    HartDataBuffer[DataFieldStartAddress+5] = *GetPara_Char(LoopCurrentModeIndex);
    HartConfigChangeFlagCountIncrease();
  }
  else if(HartDataBuffer[DataFieldStartAddress+2] < 64)
  {
    SavePara_CharType(MODIFYPARAMODE_HART,ComAddressIndex,HartDataBuffer[DataFieldStartAddress+2]);
    SavePara_CharType(MODIFYPARAMODE_HART,LoopCurrentModeIndex,HartDataBuffer[DataFieldStartAddress+3]);
    HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(ComAddressIndex);
    HartDataBuffer[DataFieldStartAddress+5] = *GetPara_Char(LoopCurrentModeIndex);
    if(*GetPara_Char(LoopCurrentModeIndex) == 0)
    {
      SetOutCurrentVal(4.0);
      HartDeviceStatusByteHandle(LoopCurrentFixed,FALSE);
    }
    else if(*GetPara_Char(LoopCurrentModeIndex) == 1)
    {
      HartDeviceStatusByteHandle(LoopCurrentFixed,TRUE);
    }
    HartConfigChangeFlagCountIncrease();
  }
  else
  {
    HartRespondCodeConfig(Error,InvalidSelection);
  }
  //后续需要添加数据有效性检测，参考通用命令手册,判断无误后才能保存参数，
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,4,FALSE);
  return TRUE;
}
/*读取通信地址，通信模式*/
bool8	HARTCMD7()//READ DYNAMIC VARIABLES AND P. V. CURRENT
{
        HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(ComAddressIndex);
	HartDataBuffer[DataFieldStartAddress+5] = *GetPara_Char(LoopCurrentModeIndex);
        RespondCodeAndDeviceStatusHandle(RespondCodeNormal,4,TRUE);
        return TRUE;
}
/* Read Dynamic Variable Classifications */
bool8	HARTCMD8()
{
        HartDataBuffer[DataFieldStartAddress+4] = DeviceVariableTable[0].DeviceVariableClassification;
	HartDataBuffer[DataFieldStartAddress+5] = DeviceVariableTable[1].DeviceVariableClassification;
        //目前没有设定第三和第四变量
        HartDataBuffer[DataFieldStartAddress+6] = DeviceVariableTable[1].DeviceVariableClassification;
        HartDataBuffer[DataFieldStartAddress+7] = DeviceVariableTable[1].DeviceVariableClassification;
        RespondCodeAndDeviceStatusHandle(RespondCodeNormal,6,TRUE);
        return TRUE;
}
/* HART命令9未定义设备变量处理函数 */
static void HartCmd9UndefineVariableHandle(uint8_t VariableIndex,uint8_t i)
{
  //未定义的设备变量处理
  //DeviceVariableCode
  HartDataBuffer[DataFieldStartAddress+5+8*i] = VariableIndex;
  //DeviceVariableClassification
  HartDataBuffer[DataFieldStartAddress+6+8*i] = 0;
  //UnitsCode
  HartDataBuffer[DataFieldStartAddress+7+8*i] = 250;
  HartDataBuffer[DataFieldStartAddress+8+8*i] = 0x7F;
  HartDataBuffer[DataFieldStartAddress+9+8*i] = 0xA0;
  HartDataBuffer[DataFieldStartAddress+10+8*i] = 0x00;
  HartDataBuffer[DataFieldStartAddress+11+8*i] = 0x00;
  HartDataBuffer[DataFieldStartAddress+12+8*i] = 0x30;
}
/* HART命令9定义设备变量处理函数 */
static void HartCmd9DefineVariableHandle(uint8_t VariableIndex,uint8_t i)
{
  //DeviceVariableCode
  HartDataBuffer[DataFieldStartAddress+5+8*i] = DeviceVariableTable[VariableIndex].DeviceVariableIndex;
  //DeviceVariableClassification
  HartDataBuffer[DataFieldStartAddress+6+8*i] = DeviceVariableTable[VariableIndex].DeviceVariableClassification;
  //UnitsCode，兼容DD
  HartDataBuffer[DataFieldStartAddress+7+8*i] = (uint8_t)RealCodeConvertToHartUnitCode((RealUnitCodeDef)*GetPara_Char(PVUnitIndex));
  FloatToTFrameChar(HartReadMasterVariable(DeviceVariableTable[VariableIndex].ValueIndex),DataFieldStartAddress+8+8*i);
  HartDataBuffer[DataFieldStartAddress+12+8*i] = DeviceVariableTable[VariableIndex].DeviceVariableStatus;
}
/* HART命令9固定设备变量处理函数 */
static void HartCmd9FixedVariableHandle(uint8_t VariableIndex,uint8_t i)
{
  switch(VariableIndex)
  {
  case 242: //电池电压
    HartDataBuffer[DataFieldStartAddress+5+8*i] = VariableIndex;
    //DeviceVariableClassification
    HartDataBuffer[DataFieldStartAddress+6+8*i] = 0;
    //UnitsCode
    HartDataBuffer[DataFieldStartAddress+7+8*i] = 58;//volts
    HartDataBuffer[DataFieldStartAddress+8+8*i] = 0x7F;
    HartDataBuffer[DataFieldStartAddress+9+8*i] = 0xA0;
    HartDataBuffer[DataFieldStartAddress+10+8*i] = 0x00;
    HartDataBuffer[DataFieldStartAddress+11+8*i] = 0x00;
    HartDataBuffer[DataFieldStartAddress+12+8*i] = 0x30;
    break;
  case 243: //电池寿命
    HartDataBuffer[DataFieldStartAddress+5+8*i] = VariableIndex;
    //DeviceVariableClassification
    HartDataBuffer[DataFieldStartAddress+6+8*i] = 0;
    //UnitsCode
    HartDataBuffer[DataFieldStartAddress+7+8*i] = 53;//days
    HartDataBuffer[DataFieldStartAddress+8+8*i] = 0x7F;
    HartDataBuffer[DataFieldStartAddress+9+8*i] = 0xA0;
    HartDataBuffer[DataFieldStartAddress+10+8*i] = 0x00;
    HartDataBuffer[DataFieldStartAddress+11+8*i] = 0x00;
    HartDataBuffer[DataFieldStartAddress+12+8*i] = 0x30; 
    break;
  case 244: //Percent range
    HartDataBuffer[DataFieldStartAddress+5+8*i] = VariableIndex;
    //DeviceVariableClassification
    HartDataBuffer[DataFieldStartAddress+6+8*i] = 0;
    //UnitsCode
    HartDataBuffer[DataFieldStartAddress+7+8*i] = 54;//%
    FloatToTFrameChar(HartReadMasterVariable(2),DataFieldStartAddress+8+8*i);
    HartDataBuffer[DataFieldStartAddress+12+8*i] = 0x30; 
    break;
  case 245: 
    HartDataBuffer[DataFieldStartAddress+5+8*i] = VariableIndex;
    //DeviceVariableClassification
    HartDataBuffer[DataFieldStartAddress+6+8*i] = 0;
    //UnitsCode
    HartDataBuffer[DataFieldStartAddress+7+8*i] = 39;//milliamperes mA
    FloatToTFrameChar(HartReadMasterVariable(3),DataFieldStartAddress+8+8*i);
    HartDataBuffer[DataFieldStartAddress+12+8*i] = 0x30; 
    break;
  case 246: 
    HartCmd9DefineVariableHandle(0,i);
    HartDataBuffer[DataFieldStartAddress+5+8*i] = VariableIndex;
    break;
  case 247: 
  case 248: 
  case 249: 
  case 250: 
    HartCmd9UndefineVariableHandle(VariableIndex,i);
    break;
  default: break;
  }
}
/*设备变量及状态*/
bool8	HARTCMD9()// Read Device Variables with Status
{
  uint8_t VariableNum = HartDataBuffer[DataFieldStartAddress+1];
  uint8_t VariableIndex[8] = {0};
  uint8_t i = 0,VariableCount = 0;
  //测试过程中，出现过字节个数超过9个的情况，可能是基金会预留的功能，当前版本需要屏蔽这种情况，后续升级再做打算
  if(VariableNum >= 9)
  {
    VariableNum = 8;
  }
  if((VariableNum <= 8) && (VariableNum != 0 ))
  {
    for(i = 0;i < VariableNum; i++)
    {
      VariableIndex[i] = HartDataBuffer[DataFieldStartAddress+2+i];
      if(VariableIndex[i] >= 251)
      {
        HartRespondCodeConfig(Error,VariableCodeInvalidSelection);
        RespondCodeAndDeviceStatusHandle(RespondCodeNormal,2,TRUE);
        return TRUE;
      }
    }
    HartDataBuffer[DataFieldStartAddress+4] = 0;
    VariableCount = sizeof(DeviceVariableTable)/sizeof(DeviceVariableTableDef);
    for(i = 0; i < VariableNum;i++)
    {
      if((VariableIndex[i] >= VariableCount) && (VariableIndex[i] < 242))
      {
        HartCmd9UndefineVariableHandle(VariableIndex[i],i);
      }
      else if(VariableIndex[i] >= 242)
      {
        HartCmd9FixedVariableHandle(VariableIndex[i],i);
      }
      else
      {
        HartCmd9DefineVariableHandle(VariableIndex[i],i);
      }
      //data timestamp,后期设定该变量会后进行补充。
      HartDataBuffer[DataFieldStartAddress+13+8*i] = 0;
      HartDataBuffer[DataFieldStartAddress+14+8*i] = 0;
      HartDataBuffer[DataFieldStartAddress+15+8*i] = 0;
      HartDataBuffer[DataFieldStartAddress+16+8*i] = 0;
      //从DataFieldStartAddress+4开始保存发送数据的
      RespondCodeAndDeviceStatusHandle(RespondCodeNormal,15+8*i,TRUE);
    }
  }
  else if(VariableNum == 0 )
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
    RespondCodeAndDeviceStatusHandle(RespondCodeNormal,2,TRUE);
  }
  return TRUE;
}
/*返回仪表信息Tag、描述、日期*/
bool8	HARTCMD11(void)//read message
{
  uint8_t TempC;
  bool8 ErrorIndicator = FALSE;
  uint8_t *ptr = NULL;
  ptr = GetPara_Char(PTagIndex);
  uint8_t TagLength = TAG_LENGTH-2;

  //命令11是用来判断TAG匹配并返回设备信息，TAG有6个字节，一般少于6字节一定不匹配，大于有可能匹配
  if(HartDataBuffer[DataFieldStartAddress+1] < 6)
  {
    ErrorIndicator = TRUE;
  }
  else
  {
    for( TempC = 0; TempC < TagLength; TempC++)
    {
      if(HartDataBuffer[DataFieldStartAddress+2+TempC]  != ptr[TempC])
      {
        ErrorIndicator = TRUE;
        break;
      }
    }
  }
  if(ErrorIndicator == FALSE)
  {
    HART_CMD0Config();
    RespondCodeAndDeviceStatusHandle(RespondCodeNormal,24,TRUE);
  }
  else
  {
    EnableHartRx();
    return FALSE;
  }
  return TRUE;
}
/* HART命令12配置函数 */
static void HartCmd12Config()
{
  uint8_t TempC;
  for(TempC = 0; TempC < MESSAGE_LENGTH; TempC++)
          HartDataBuffer[DataFieldStartAddress+TempC+4] = *(GetPara_Char(PMessageIndex)+TempC);
}
/*返回仪表信息*/
bool8	HARTCMD12(void)//read message
{
  HartCmd12Config();
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,26,TRUE);
  return TRUE;
}
/* HART命令13配置函数 */
static void HartCmd13Config()
{
  uint8_t TempC;
  uint8_t DateLength = DATE_LENGTH-5;
  uint8_t DescriptorLength = DESCRIPTOR_LENGTH-4;
  uint8_t TagLength = TAG_LENGTH-2;
  if((G_HartRespondCodeStatus.Type == Error)
     &&(G_HartRespondCodeStatus.value != 0 ))
  {  
    
  }
  else
  {
    for(TempC = 0; TempC < TagLength; TempC++)
            HartDataBuffer[DataFieldStartAddress+TempC+4] = *(GetPara_Char(PTagIndex)+TempC);
    for(TempC = 0; TempC < DescriptorLength; TempC++)
            HartDataBuffer[DataFieldStartAddress+TempC+10] = *(GetPara_Char(PDescriptorIndex)+TempC);
    for(TempC = 0; TempC < DateLength; TempC++)
            HartDataBuffer[DataFieldStartAddress+TempC+22] = *(GetPara_Char(PDateIndex)+TempC);
  }
}
/* Read Tag, Descriptor, Date */
bool8	HARTCMD13(void)//read message
{
  HartCmd13Config();
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,23,TRUE);
  return TRUE;
}
/**unsigned char HARTCMD14()//函数名称和参量和返回值类型。
* 变量范围: 
*
* 返回值说明: HART命令号
*
* 功能介绍: 读仪表序列号、主变量单位、高位盲区、低位盲区、主变量最小跨度
*
* 关键备注:
*
* 未来提高:
* 
*/

bool8	HARTCMD14(void)//READ PRIMARY VARIABLE SENSOR INFORMATION
{
  HartDataBuffer[DataFieldStartAddress+1] = 18;
  HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = FieldDeviceNormal;
  HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = RespondCodeNormal;

  HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(FinalAsmNoIndex);
  HartDataBuffer[DataFieldStartAddress+5] = *(GetPara_Char(FinalAsmNoIndex)+1);
  HartDataBuffer[DataFieldStartAddress+6] = *(GetPara_Char(FinalAsmNoIndex)+2);
  HartDataBuffer[DataFieldStartAddress+7] = (uint8_t)RealCodeConvertToHartUnitCode((RealUnitCodeDef)*GetPara_Char(PVUnitIndex));//HART_Variables.PVUnit;
  ReadParaF_Hart(HartLowerRangeIndex, &HartDataBuffer[DataFieldStartAddress+8]);
  ReadParaF_Hart(HartUpperRangeIndex, &HartDataBuffer[DataFieldStartAddress+12]);
  FloatToTFrameChar(0.001,DataFieldStartAddress+16);
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,18,TRUE);
  return TRUE;
}

/*返回仪表高低位量程、阻尼时间、写保护码、*/
bool8	HARTCMD15(void)//READ PRIMARY VARIABLE OUTPUT INFORMATION
{
  HartDataBuffer[DataFieldStartAddress+4] = Low;//alarm slect
  HartDataBuffer[DataFieldStartAddress+5] = NotUsed;//Unit;
  HartDataBuffer[DataFieldStartAddress+6] = (uint8_t)RealCodeConvertToHartUnitCode((RealUnitCodeDef)*GetPara_Char(PVUnitIndex));//Unit;
  ReadParaF_Hart(UpperRangeIndex, &HartDataBuffer[DataFieldStartAddress+7]);
  ReadParaF_Hart(LowerRangeIndex, &HartDataBuffer[DataFieldStartAddress+11]);
  FloatToTFrameChar((float)(*GetPara_Char(DampingTimeIndex)),DataFieldStartAddress+15);
  HartDataBuffer[DataFieldStartAddress+19] = *GetPara_Char(Eeprom_EnablePara_ProtectIndex);
  //5A是开启写保护，A5是关闭写保护
  if(HartDataBuffer[DataFieldStartAddress+19] == 0xA5)
  {
    HartDataBuffer[DataFieldStartAddress+19] = 0;
  }
  else if(HartDataBuffer[DataFieldStartAddress+19] == 0x5a)
  {
    HartDataBuffer[DataFieldStartAddress+19] = 1;
  }
  else
    HartDataBuffer[DataFieldStartAddress+19] = 250;
  HartDataBuffer[DataFieldStartAddress+20] = NotUsed;//Reserved
  HartDataBuffer[DataFieldStartAddress+21] = 0;//Analog Channel Flags，无此功能
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,20,TRUE);
  return TRUE;
}
static void HartCmd16Config()
{
  HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(FinalAsmNoIndex);
  HartDataBuffer[DataFieldStartAddress+5] = *(GetPara_Char(FinalAsmNoIndex)+1);
  HartDataBuffer[DataFieldStartAddress+6] = *(GetPara_Char(FinalAsmNoIndex)+2);
}
/*返回仪表序列号*/
bool8	HARTCMD16(void)//READ FINAL ASSEMBLY NUMBER
{
  HartCmd16Config();
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,5,TRUE);
  return TRUE;
}

/*修改仪表Tag、描述、日期*/
bool8	HARTCMD17(void)//WRITE TAG, DESCRIPTOR, DATE
{
  if(HartDataBuffer[DataFieldStartAddress+1] < 24)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    SavePara_StringType(PMessageIndex,&HartDataBuffer[DataFieldStartAddress+2]);
    HartConfigChangeFlagCountIncrease();
  }
  
  HartCmd12Config();
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,26,FALSE);
  return TRUE;
}
/*修改仪表Tag、描述、日期*/
bool8	HARTCMD18(void)//WRITE TAG, DESCRIPTOR, DATE
{
  if(HartDataBuffer[DataFieldStartAddress+1] < 21)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    if((HartDataBuffer[DataFieldStartAddress+20] >= 0xff)
          ||(HartDataBuffer[DataFieldStartAddress+21] >= 0xff))
    {
      HartRespondCodeConfig(Error,InvalidDateCodeDetected);
    }
    else
    {
      SavePara_StringType(PTagIndex,&HartDataBuffer[DataFieldStartAddress+2]);
      SavePara_StringType(PDescriptorIndex,&HartDataBuffer[DataFieldStartAddress+8]);
      SavePara_StringType(PDateIndex,&HartDataBuffer[DataFieldStartAddress+20]);
      HartConfigChangeFlagCountIncrease();
    }
  }
  HartCmd13Config();
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,23,FALSE);
  return TRUE;
}

/*修改仪表序列号*/
bool8	HARTCMD19(void)//WRITE FINAL ASSEMBLY NUMBER
{
  if(HartDataBuffer[DataFieldStartAddress+1] < 3)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    SavePara_StringType(FinalAsmNoIndex,&HartDataBuffer[DataFieldStartAddress+2]);
    HartConfigChangeFlagCountIncrease();
  }
  HartCmd16Config();
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,5,FALSE);
  return TRUE;
}
/* HART命令20配置函数 */
static void HartCmd20Config()
{
  uint8_t TempC;
  for(TempC = 0; TempC < LONG_TAG_LENGTH; TempC++)
          HartDataBuffer[DataFieldStartAddress+TempC+4] = *(GetPara_Char(PLongTag)+TempC);
}
/*返回仪表信息LONG_TAG消息*/
bool8	HARTCMD20(void)//read Long Tag
{
  HartCmd20Config();
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,34,TRUE);
  return TRUE;
}
/*读取与长标识关联的唯一识别，主机发送长标识，从机接收后进行长标识匹配，通过则返回命令0的信息*/
bool8	HARTCMD21(void)
{
  HARTCMD0();
  return TRUE;
}
/*写仪表信息LONG_TAG消息*/
bool8	HARTCMD22(void)//read Long Tag
{
  if(HartDataBuffer[DataFieldStartAddress+1] < 32)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    SavePara_StringType(PLongTag,&HartDataBuffer[DataFieldStartAddress+2]);
    HartConfigChangeFlagCountIncrease();
  }
  HartCmd20Config();
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,34,FALSE);
  return TRUE;
}
/*修改仪表阻尼时间*/
bool8	HARTCMD34(void)//WRITE PRIMARY VARIABLE DAMPING VALUE
{
  uint8_t TempC;
  float DampingValue = 0;
  if(HartDataBuffer[DataFieldStartAddress+1] < 4)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    DampingValue = *((RFrameCharToFloat(DataFieldStartAddress+2)));
    if(DampingValue > GetPara_CharMax(DampingTimeIndex))
    {
      HartRespondCodeConfig(Error,PassedParameterTooLarge);
    }
    else if(DampingValue < GetPara_CharMin(DampingTimeIndex))
    {
      HartRespondCodeConfig(Error,PassedParameterTooSmall);
    }
    else
    {
      //本机的阻尼时间范围是0~99的整数
      TempC = (uint8_t)DampingValue;
      SavePara_CharType(MODIFYPARAMODE_HART,DampingTimeIndex,TempC);
      FloatToTFrameChar((float)(*GetPara_Char(DampingTimeIndex)),DataFieldStartAddress+4);
      HartConfigChangeFlagCountIncrease();
    }
  }

  //后续需要添加数据有效性检测，参考通用命令手册,判断无误后才能保存参数，
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,6,FALSE);
  return TRUE;
}

/*修改仪表高低位量程*/
bool8	HARTCMD35(void)//WRITE PRIMARY VARIABLE DAMPING VALUE
{
  float PV_URV = 0;
  float PV_LRV = 0;
  HartUnitDef UnitCode = Unit_ndef;
  RealUnitCodeDef RealUnitCode = RealUnit_ndef;
  float UnitParaRatio = 0;
  if(HartDataBuffer[DataFieldStartAddress+1] < 9)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    PV_URV = *((RFrameCharToFloat(DataFieldStartAddress+3)));
    PV_LRV = *((RFrameCharToFloat(DataFieldStartAddress+7)));
    UnitCode = (HartUnitDef)HartDataBuffer[DataFieldStartAddress+2];
    RealUnitCode = HartUnitCodeConvertToRealCode(UnitCode); 
    if(RealUnitCode != RealUnit_ndef)
    {
      UnitParaRatio = UnitParaRatioGet((uint8_t)RealUnitCode);
    }
    if((PV_URV > GetPara_FloatMax(UpperRangeIndex)*UnitParaRatio) 
       || (PV_URV < GetPara_FloatMin(UpperRangeIndex)*UnitParaRatio) 
         || (PV_LRV > GetPara_FloatMax(LowerRangeIndex)*UnitParaRatio)
           ||(PV_LRV < GetPara_FloatMin(LowerRangeIndex)*UnitParaRatio)
             ||((PV_URV > GetPara_FloatMax(UpperRangeIndex)*UnitParaRatio)
                    &&(PV_LRV < GetPara_FloatMin(LowerRangeIndex)*UnitParaRatio))
                      ||(PV_LRV == PV_URV)
                        ||(PV_LRV < PV_URV)
                          ||(RealUnitCode == RealUnit_ndef))
    {
      //HART涉及到底单位有5个：44，45，47，48，49，其他均为无效码
      if(RealUnitCode == RealUnit_ndef)
      {
        HartRespondCodeConfig(Error,InvalidUnitsCode);
      }
      else
      {
        if(PV_URV > GetPara_FloatMax(UpperRangeIndex)*UnitParaRatio)
        {
          HartRespondCodeConfig(Error,UpperRangeValueTooHigh);
        }
        if(PV_URV < GetPara_FloatMin(UpperRangeIndex)*UnitParaRatio)
        {
          HartRespondCodeConfig(Error,UpperRangeValueTooLow);
        }
        if(PV_LRV > GetPara_FloatMax(LowerRangeIndex)*UnitParaRatio)
        {
          HartRespondCodeConfig(Error,LowerRangeValueTooHigh);
        }
        if(PV_LRV < GetPara_FloatMin(LowerRangeIndex)*UnitParaRatio)
        {
          HartRespondCodeConfig(Error,LowerRangeValueTooLow);
        }
        if(PV_LRV == PV_URV)
        {
          HartRespondCodeConfig(Error,InvalidSpan);
        }
        if((PV_URV > GetPara_FloatMax(UpperRangeIndex)*UnitParaRatio)
                      &&(PV_LRV < GetPara_FloatMin(LowerRangeIndex)*UnitParaRatio))
        {
          HartRespondCodeConfig(Error,UpperAndLowerRangeValuesOutOfLimits);
        }
        RespondCodeAndDeviceStatusHandle(RespondCodeNormal,2,FALSE);
        return TRUE;
      }
    }
      
    SavePara_FloatType(MODIFYPARAMODE_HART,UpperRangeIndex,RFrameCharToFloat(DataFieldStartAddress+3));
    SavePara_FloatType(MODIFYPARAMODE_HART,LowerRangeIndex,RFrameCharToFloat(DataFieldStartAddress+7));
    HartDataBuffer[DataFieldStartAddress+4] = UnitCode;
    ReadParaF_Hart(1, &HartDataBuffer[DataFieldStartAddress+5]);
    ReadParaF_Hart(0, &HartDataBuffer[DataFieldStartAddress+9]);
    HartConfigChangeFlagCountIncrease();
  }

  //后续需要添加数据有效性检测，参考通用命令手册,判断无误后才能保存参数，
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,11,FALSE);
  return TRUE;
}

/* 复位配置更改标志命令 */
bool8	HARTCMD38(void)
{
  uint16_t ChangeCounter = 0;
  uint16_t ConfigurationChangeCounter = 0;
  ConfigurationChangeCounter = *GetPara_Char(ConfigChangeCounter)<<8;
  ConfigurationChangeCounter += *(GetPara_Char(ConfigChangeCounter)+1);
  if(HartDataBuffer[DataFieldStartAddress+1] >= 2)
  {
    ChangeCounter = HartDataBuffer[DataFieldStartAddress+2]<<8;
    ChangeCounter += HartDataBuffer[DataFieldStartAddress+3];
    if(ChangeCounter == ConfigurationChangeCounter)
    {
      HartConfigChangeFlagReset();
    }
    else
    {
      HartRespondCodeConfig(Error,ConfigChangeCounterMismatch);
    }
  }
  else if(HartDataBuffer[DataFieldStartAddress+1] == 0)
  {
    //一般认为主机协议版本低于HART6
    HartConfigChangeFlagReset();
  }
  else
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  HartDataBuffer[DataFieldStartAddress+4] = ConfigurationChangeCounter>>8;
  HartDataBuffer[DataFieldStartAddress+5] = ConfigurationChangeCounter;
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,4,TRUE);
  return TRUE;
}

/*输出固定电流值（范围2-24）或者如果接收到上位机传送的固定电流值为1进行电流较准*/
bool8 HARTCMD40()//ENTER/EXIT FIXED PRIMARY VARIABLE CURRENT MODE
{
  float TempF;
  if(HartDataBuffer[DataFieldStartAddress+1] < 4)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    TempF = *(RFrameCharToFloat(DataFieldStartAddress+2));
    if((TempF < 3)||(TempF > 24))
    {
      if((TempF < 3)&&(TempF != 0))
      {
        HartRespondCodeConfig(Error,PassedParameterTooSmall);
        TempF = 0;
      }
      else if(TempF > 24)
      {
        HartRespondCodeConfig(Error,PassedParameterTooLarge);
        TempF = 0;
      }
      else if(TempF == 0)
      {
        SavePara_CharType(MODIFYPARAMODE_HART,LoopCurrentModeIndex,1);
        HartDeviceStatusByteHandle(LoopCurrentFixed,TRUE);
        SetOutCurrentVal(0);
      }
    }
    else
    {
      //loop current fixed
      if(*GetPara_Char(ComAddressIndex) != 0)
      {
        HartRespondCodeConfig(Error,LoopCurrentNotActive);
      }
      else
      {
        SavePara_CharType(MODIFYPARAMODE_HART,LoopCurrentModeIndex,0);
        HartDeviceStatusByteHandle(LoopCurrentFixed,FALSE);
        SetOutCurrentVal(TempF);	
        //后续需要添加数据有效性检测，参考通用命令手册,判断无误后才能保存参数，
        //异常需要更新响应码。
        FloatToTFrameChar(TempF,DataFieldStartAddress+4);
        HartConfigChangeFlagCountIncrease();
      }
    }
  }
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,6,FALSE);
  return TRUE;
}

/*修改仪表测量值单位*/
bool8	HARTCMD44()//unit
{
  HartUnitDef HartUnit = Unit_ndef;
  RealUnitCodeDef RealHartUnit = RealUnit_ndef;
  if(HartDataBuffer[DataFieldStartAddress+1] < 1)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    HartUnit = (HartUnitDef)HartDataBuffer[DataFieldStartAddress+2];
    RealHartUnit = HartUnitCodeConvertToRealCode(HartUnit);
    if(RealHartUnit != RealUnit_ndef)
    {
      SavePara_CharType(MODIFYPARAMODE_HART,PVUnitIndex,RealHartUnit);
      HartDataBuffer[DataFieldStartAddress+4] = (uint8_t)RealCodeConvertToHartUnitCode((RealUnitCodeDef)*GetPara_Char(PVUnitIndex));
      HartConfigChangeFlagCountIncrease();
    }
    else
    {
      HartRespondCodeConfig(Error,InvalidSelection);
    }
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,3,FALSE);
  return TRUE;
}
/*调整输出电流零点，*/
bool8	HARTCMD45()
{
  uint8_t TempC;
  float TempF = 0;
  TempF = *(RFrameCharToFloat(DataFieldStartAddress+2));
  if(HartDataBuffer[DataFieldStartAddress+1] < 4)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    if((TempF < 3)||(TempF > 24))
    {
      if(TempF < 3)
      {
        HartRespondCodeConfig(Error,PassedParameterTooSmall);
      }
      else if(TempF > 24)
      {
        HartRespondCodeConfig(Error,PassedParameterTooLarge);
      }
    }
    else
    {
      //loop current fixed
      if(*GetPara_Char(ComAddressIndex) != 0)
      {
          HartRespondCodeConfig(Error,LoopCurrentNotActive);
      }
      else
      {
        if(SetCurrentGet() == 20.000)
        {
          HartRespondCodeConfig(Error,IncorrectLoopCurrentModeorValue);
        }
        else
        {
          Set_PWM4mABias_Value(TempF);
          for(TempC = DataFieldStartAddress+8; TempC < DataFieldStartAddress+4; TempC--)
          {
                  HartDataBuffer[TempC] = HartDataBuffer[TempC-2];
          }
          ModifyCurrent(MODIFYPARAMODE_HART);
          SetOutCurrentVal(4.0);
        }
      }
    }
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,6,FALSE);
  return TRUE;
}

/*调整输出电流增益，*/
bool8 HARTCMD46()//18mA CURRENT DAC GAIN
{
  uint8_t TempC;
  float TempF = 0;
  TempF = *(RFrameCharToFloat(DataFieldStartAddress+2));
  if(HartDataBuffer[DataFieldStartAddress+1] < 4)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    if((TempF < 3)||(TempF > 24))
    {
      if(TempF < 3)
      {
        HartRespondCodeConfig(Error,PassedParameterTooSmall);
      }
      else if(TempF > 24)
      {
        HartRespondCodeConfig(Error,PassedParameterTooLarge);
      }
    }
    else
    {
      //loop current fixed
      if(*GetPara_Char(ComAddressIndex) != 0)
      {
          HartRespondCodeConfig(Error,LoopCurrentNotActive);
      }
      else
      {
        if(SetCurrentGet() == 4.000)
        {
          HartRespondCodeConfig(Error,IncorrectLoopCurrentModeorValue);
        }
        else
        {
          PWM20_PWM18_Current(TempF);
          for(TempC = DataFieldStartAddress+8; TempC < DataFieldStartAddress+4; TempC--)
          {
                  HartDataBuffer[TempC] = HartDataBuffer[TempC-2];
          }
          ModifyCurrent(MODIFYPARAMODE_HART);
          SetOutCurrentVal(20.0);
        }
      }
    }
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,6,FALSE);
  return TRUE;
}
/* 附加设备状态读取 */
bool8	HARTCMD48(void)//read Long Tag
{
  uint8_t TempC,missmatch = 0;
  
  if((HartDataBuffer[DataFieldStartAddress+1] > 0)
     &&(HartDataBuffer[DataFieldStartAddress+1] < ADDITIONAL_DEV_STATUS))
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    //如果个数为0，需要直接返回状态信息
    if(HartDataBuffer[DataFieldStartAddress+1] != 0)
    {
      //目前只应用了一个状态字节
      if(HartDataBuffer[DataFieldStartAddress+2] == WorkStatus)
      {
        //其余状态字节均设置为0
        for(TempC = 1; TempC < ADDITIONAL_DEV_STATUS; TempC++)
        {
          if((HartDataBuffer[DataFieldStartAddress+2+TempC] != 0)
             &&(TempC != 6))
          {
            missmatch = 1;
            break;
          }
        }
        if(missmatch == 1)
        {
          HartRespondCodeConfig(Warning,Statusbytesmismatch);
        }
        else
        {
          if(IS_PRIMARY_MASTER)
          {
            G_PMasterStatusCachedValue = WorkStatus;
          }
          if(IS_SECOND_MASTER)
          {
            G_SMasterStatusCachedValue = WorkStatus;
          }
          HartMoreStatusAvailableModeVarify();
        }
      }
      else
      {
        HartRespondCodeConfig(Warning,Statusbytesmismatch);
      }
    }
    //最后一个字节使用工作状态码，返回给主机
    HartDataBuffer[DataFieldStartAddress+4] = WorkStatus;
    for(TempC = 1; TempC < ADDITIONAL_DEV_STATUS; TempC++)
    {
      //因为第48号命令需要使用设备状态变量报警功能
      if(TempC == 6)
      {
        HartDataBuffer[DataFieldStartAddress+TempC+4] = 0x11;
      }
      else
      {
        HartDataBuffer[DataFieldStartAddress+TempC+4] = 0;
      }
    }
  }
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,27,TRUE);
  return TRUE;
}
/*修改仪表主变量序列号*/
bool8 HARTCMD49()//WRITE PRIMARY VARIABLE SENSOR SERIAL NUMBER
{
  if(HartDataBuffer[DataFieldStartAddress+1] < 3)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    SavePara_StringType(FinalAsmNoIndex,&HartDataBuffer[DataFieldStartAddress+2]); 
    HartCmd16Config();
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,5,FALSE);
  return TRUE;
}

/*
 * 返回仪表物位速度（float 8）、回波加值（char 0）、介电常数（char 2）
 * 命令128
 */
bool8 HARTCMD128()//READ base para
{
  ReadParaF_Hart(8, &HartDataBuffer[DataFieldStartAddress+4]);
  HartDataBuffer[DataFieldStartAddress+8] = *GetPara_Char(0);
  HartDataBuffer[DataFieldStartAddress+9] = *GetPara_Char(2);
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,8,TRUE);
  return TRUE;
}
/* 修改仪表介电常数 （char 2）
 * 命令129
 */
bool8 HARTCMD129()//write material and field status
{
  if(HartDataBuffer[DataFieldStartAddress+1] < 1)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    if((HartDataBuffer[DataFieldStartAddress+2] <= GetPara_CharMax(DielectricConstantIndex))
       &&(HartDataBuffer[DataFieldStartAddress+2]>= GetPara_CharMin(DielectricConstantIndex)))
    {
      SavePara_CharType(MODIFYPARAMODE_HART,DielectricConstantIndex,HartDataBuffer[DataFieldStartAddress+2]);
      HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(DielectricConstantIndex);
    }
    else
    {
      HartRespondCodeConfig(Error,InvalidSelection);
    }
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,3,FALSE);
  return TRUE;
}

/*  
 * 修改仪表物位速度（float 8）
 * 命令130
 */
bool8	HARTCMD130()//write base para
{
  float OutputM_Speed = 0;
  if(HartDataBuffer[DataFieldStartAddress+1] < 4)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    OutputM_Speed = *((RFrameCharToFloat(DataFieldStartAddress+2)));
    if((OutputM_Speed <= GetPara_FloatMax(OutputM_SpeedIndex))
       &&(OutputM_Speed >= GetPara_FloatMin(OutputM_SpeedIndex)))
    {
      SavePara_FloatType(MODIFYPARAMODE_HART,OutputM_SpeedIndex,RFrameCharToFloat(DataFieldStartAddress+2));
      ReadParaF_Hart(OutputM_SpeedIndex, &HartDataBuffer[DataFieldStartAddress+4]);
    }
    else
    {
      if(OutputM_Speed > GetPara_FloatMax(OutputM_SpeedIndex))
      {
        HartRespondCodeConfig(Error,PassedParameterTooLarge);
      }
      else if(OutputM_Speed < GetPara_FloatMin(OutputM_SpeedIndex))
      {
        HartRespondCodeConfig(Error,PassedParameterTooSmall);
      }
    }
  }
  //后续需要添加数据有效性检测，参考通用命令手册,判断无误后才能保存参数，
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,6,FALSE);
  
  return TRUE;
}
/*修改虚假回波加值*/
bool8	HARTCMD131()
{
  if(HartDataBuffer[DataFieldStartAddress+1] < 1)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    if((HartDataBuffer[DataFieldStartAddress+2] <= GetPara_CharMax(FakeWaveGainIndex))
       &&(HartDataBuffer[DataFieldStartAddress+2]>= GetPara_CharMin(FakeWaveGainIndex)))
    {
      SavePara_CharType(MODIFYPARAMODE_HART,FakeWaveGainIndex,HartDataBuffer[DataFieldStartAddress+2]);
      HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(FakeWaveGainIndex);
    }
    else
    {
      if(HartDataBuffer[DataFieldStartAddress+2] > GetPara_CharMax(FakeWaveGainIndex))
      {
        HartRespondCodeConfig(Error,PassedParameterTooLarge);
      }
      else if(HartDataBuffer[DataFieldStartAddress+2] < GetPara_CharMin(FakeWaveGainIndex))
      {
        HartRespondCodeConfig(Error,PassedParameterTooSmall);
      }
    }
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,3,FALSE);
  return TRUE;
}
/*
* 修改仪表高低位盲区
* 参数设置顺序：高位盲区（EEPROM地址7），低位盲区（EEPROM地址6）。
* 响应帧格式：响应码，设备状态码，高位盲区，低位盲区。
*/
bool8 HARTCMD132()//WRITE PV_UPPER_SENSOR_LIMIT
{
  float HartLowerRange = 0,HartUpperRange = 0;
  if(HartDataBuffer[DataFieldStartAddress+1] < 8)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    HartLowerRange = *((RFrameCharToFloat(DataFieldStartAddress+2)));
    HartUpperRange = *((RFrameCharToFloat(DataFieldStartAddress+6)));
    if(HartLowerRange > GetPara_FloatMax(HartLowerRangeIndex)
       ||(HartUpperRange > GetPara_FloatMax(HartUpperRangeIndex)))
    {
      HartRespondCodeConfig(Error,PassedParameterTooLarge);
    }
    else if((HartLowerRangeIndex < GetPara_FloatMin(HartLowerRangeIndex))
            ||(HartLowerRange < GetPara_FloatMin(HartUpperRangeIndex)))
    {
      HartRespondCodeConfig(Error,PassedParameterTooSmall);
    }
    else
    {
      SavePara_FloatType(MODIFYPARAMODE_HART,HartLowerRangeIndex,RFrameCharToFloat(DataFieldStartAddress+2));
      SavePara_FloatType(MODIFYPARAMODE_HART,HartUpperRangeIndex,RFrameCharToFloat(DataFieldStartAddress+6));
      ReadParaF_Hart(HartLowerRangeIndex, &HartDataBuffer[DataFieldStartAddress+4]);	
      ReadParaF_Hart(HartUpperRangeIndex, &HartDataBuffer[DataFieldStartAddress+8]);
    }
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,10,FALSE);
  return TRUE;
}
static void HARTCMD133Config(void)//响应帧配置
{
  HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(16);
  ReadParaF_Hart(18, &HartDataBuffer[DataFieldStartAddress+5]);
  ReadParaF_Hart(19, &HartDataBuffer[DataFieldStartAddress+9]);
  ReadParaF_Hart(20, &HartDataBuffer[DataFieldStartAddress+13]);
}
/*返回仪表回波判决方式、前波优势（浮点数）、回波确认时间（浮点数）、最小分辨率（浮点数）*/
bool8 HARTCMD133(void)//READ base para
{
  HARTCMD133Config();
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,15,TRUE);
  return TRUE;
}
/*修改仪表回波判决方式、前波优势（浮点数）、回波确认时间（浮点数）、最小分辨率（浮点数）*/
bool8 HARTCMD134()// base para
{
  float FrontEchoA = 0;
  float ConfirmEchoTime = 0;
  float MinDistinctiveness = 0;
  if(HartDataBuffer[DataFieldStartAddress+1] < 13)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    FrontEchoA = *((RFrameCharToFloat(DataFieldStartAddress+3)));
    ConfirmEchoTime = *((RFrameCharToFloat(DataFieldStartAddress+7)));
    MinDistinctiveness = *((RFrameCharToFloat(DataFieldStartAddress+11)));
    if((HartDataBuffer[DataFieldStartAddress+2] > GetPara_CharMax(EchoAdjudgeSELIndex))
         &&(HartDataBuffer[DataFieldStartAddress+2] < GetPara_CharMin(EchoAdjudgeSELIndex)))
    {
      HartRespondCodeConfig(Error,InvalidSelection);
    }
    else if((FrontEchoA > GetPara_FloatMax(FrontEchoAIndex))
         ||(ConfirmEchoTime > GetPara_FloatMax(ConfirmEchoTimeIndex))
           ||(MinDistinctiveness > GetPara_FloatMax(MinDistinctivenessIndex)))
    {
      HartRespondCodeConfig(Error,PassedParameterTooLarge);
    }
    else if((FrontEchoA < GetPara_FloatMin(FrontEchoAIndex))
            ||(ConfirmEchoTime < GetPara_FloatMin(ConfirmEchoTimeIndex))
              ||(MinDistinctiveness < GetPara_FloatMin(MinDistinctivenessIndex)))
    {
      HartRespondCodeConfig(Error,PassedParameterTooSmall);
    }
    else
    {
      SavePara_CharType(MODIFYPARAMODE_HART,EchoAdjudgeSELIndex,HartDataBuffer[DataFieldStartAddress+2]);
      SavePara_FloatType(MODIFYPARAMODE_HART,FrontEchoAIndex,RFrameCharToFloat(DataFieldStartAddress+3));
      SavePara_FloatType(MODIFYPARAMODE_HART,ConfirmEchoTimeIndex,RFrameCharToFloat(DataFieldStartAddress+7));
      SavePara_FloatType(MODIFYPARAMODE_HART,MinDistinctivenessIndex,RFrameCharToFloat(DataFieldStartAddress+11));
      HARTCMD133Config();
    }
  }
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,15,FALSE);
  return TRUE;
}
void HARTCMD135Config()
{
  HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(9);
  ReadParaF_Hart(14, &HartDataBuffer[DataFieldStartAddress+5]);
}
/*返回仪表失波选择、失波指定位置（浮点数）*/
bool8 HARTCMD135()//READ base para
{
  HARTCMD135Config();
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,7,TRUE);
  return TRUE;
}

/*修改仪表失波选择、失波指定位置（浮点数）*/
bool8 HARTCMD136()// base para
{	
  float WaveLostSetPosition = 0;
  if(HartDataBuffer[DataFieldStartAddress+1] < 5)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    WaveLostSetPosition =  *((RFrameCharToFloat(DataFieldStartAddress+3)));
    if((HartDataBuffer[DataFieldStartAddress+2] > GetPara_CharMax(WaveLostProccessSELIndex))
         &&(HartDataBuffer[DataFieldStartAddress+2] < GetPara_CharMin(WaveLostProccessSELIndex)))
    {
      HartRespondCodeConfig(Error,InvalidSelection);
    }
    else if(WaveLostSetPosition > GetPara_FloatMax(WaveLostSetPositionIndex))
    {
      HartRespondCodeConfig(Error,PassedParameterTooLarge);
    }
    else if(WaveLostSetPosition < GetPara_FloatMin(WaveLostSetPositionIndex))
    {
      HartRespondCodeConfig(Error,PassedParameterTooSmall);
    }
    else
    {
      SavePara_CharType(MODIFYPARAMODE_HART,WaveLostProccessSELIndex,HartDataBuffer[DataFieldStartAddress+2]);
      SavePara_FloatType(MODIFYPARAMODE_HART,WaveLostSetPositionIndex,RFrameCharToFloat(DataFieldStartAddress+3));
      HARTCMD135Config();
    }
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,7,FALSE);
  return TRUE;
}
static void HARTCMD137Config()
{
  HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(10);
  ReadParaF_Hart(15, &HartDataBuffer[DataFieldStartAddress+5]);
  ReadParaF_Hart(16, &HartDataBuffer[DataFieldStartAddress+9]);
  ReadParaF_Hart(17, &HartDataBuffer[DataFieldStartAddress+13]);
}
/*返回仪表跳变方式、跳变距离（浮点数）、等待时间（浮点数）、跳变速度（浮点数）*/
bool8 HARTCMD137()//READ base para
{
  HARTCMD137Config();
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,15,TRUE);
  return TRUE;
}
/*修改仪表跳变方式、跳变距离（浮点数）、等待时间（浮点数）、跳变速度（浮点数）*/
bool8 HARTCMD138()// base para
{
  float FrontEchoA = 0;
  float ConfirmEchoTime = 0;
  float MinDistinctiveness = 0;
  if(HartDataBuffer[DataFieldStartAddress+1] < 13)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    FrontEchoA = *((RFrameCharToFloat(DataFieldStartAddress+3)));
    ConfirmEchoTime = *((RFrameCharToFloat(DataFieldStartAddress+7)));
    MinDistinctiveness = *((RFrameCharToFloat(DataFieldStartAddress+11)));
    if((HartDataBuffer[DataFieldStartAddress+2] > GetPara_CharMax(JumpSELIndex))
         &&(HartDataBuffer[DataFieldStartAddress+2] < GetPara_CharMin(JumpSELIndex)))
    {
      HartRespondCodeConfig(Error,InvalidSelection);
    }
    else if((FrontEchoA > GetPara_FloatMax(JumpDistIndex))
         ||(ConfirmEchoTime > GetPara_FloatMax(JumpWaitTimeIndex))
           ||(MinDistinctiveness > GetPara_FloatMax(JumpChangeSpeedIndex)))
    {
      HartRespondCodeConfig(Error,PassedParameterTooLarge);
    }
    else if((FrontEchoA < GetPara_FloatMin(JumpDistIndex))
            ||(ConfirmEchoTime < GetPara_FloatMin(JumpWaitTimeIndex))
              ||(MinDistinctiveness < GetPara_FloatMin(JumpChangeSpeedIndex)))
    {
      HartRespondCodeConfig(Error,PassedParameterTooSmall);
    }
    else
    {
      SavePara_CharType(MODIFYPARAMODE_HART,JumpSELIndex,HartDataBuffer[DataFieldStartAddress+2]);
      SavePara_FloatType(MODIFYPARAMODE_HART,JumpDistIndex,RFrameCharToFloat(DataFieldStartAddress+3));
      SavePara_FloatType(MODIFYPARAMODE_HART,JumpWaitTimeIndex,RFrameCharToFloat(DataFieldStartAddress+7));
      SavePara_FloatType(MODIFYPARAMODE_HART,JumpChangeSpeedIndex,RFrameCharToFloat(DataFieldStartAddress+11));
      HARTCMD137Config();
    }
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,15,FALSE);
  return TRUE;
}
static void HARTCMD139Config()
{  
  HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(3);
  HartDataBuffer[DataFieldStartAddress+5] = *GetPara_Char(13);
  HartDataBuffer[DataFieldStartAddress+6] = *GetPara_Char(14);
}
/*返回仪表电流方向、电流报警、启动电流*/
bool8 HARTCMD139()//READ base para
{
  HARTCMD139Config();
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,5,TRUE);
  return TRUE;
}
/*修改仪表电流方向、电流报警、启动电流*/
bool8 HARTCMD140()
{
  if(HartDataBuffer[DataFieldStartAddress+1] < 3)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    if(((HartDataBuffer[DataFieldStartAddress+2] > GetPara_CharMax(CurrentDirectionIndex))
         &&(HartDataBuffer[DataFieldStartAddress+2] < GetPara_CharMin(CurrentDirectionIndex)))
       ||((HartDataBuffer[DataFieldStartAddress+2] > GetPara_CharMax(CurrentAlarmSELIndex))
          &&(HartDataBuffer[DataFieldStartAddress+2] < GetPara_CharMin(CurrentAlarmSELIndex)))
         ||((HartDataBuffer[DataFieldStartAddress+2] > GetPara_CharMax(CurrentStartSELIndex))
            &&(HartDataBuffer[DataFieldStartAddress+2] < GetPara_CharMin(CurrentStartSELIndex))))
    {
      HartRespondCodeConfig(Error,InvalidSelection);
    }
    else
    {
      SavePara_CharType(MODIFYPARAMODE_HART,CurrentDirectionIndex,HartDataBuffer[DataFieldStartAddress+2]);
      SavePara_CharType(MODIFYPARAMODE_HART,CurrentAlarmSELIndex,HartDataBuffer[DataFieldStartAddress+3]);
      SavePara_CharType(MODIFYPARAMODE_HART,CurrentStartSELIndex,HartDataBuffer[DataFieldStartAddress+4]);
      HARTCMD139Config();
    }
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,5,FALSE);
  return TRUE;
}
/*更改写参数保护*/
bool8 HARTCMD141()
{
  uint8_t i = 0,CheckCode[12] = {0};
  //为了兼容HART测试，进行写保护码转换，0对应关闭-0xA5，1对应开启-0x5A
  if(HartDataBuffer[DataFieldStartAddress+14] == 0)
  {
    HartDataBuffer[DataFieldStartAddress+14] = 0xA5;
  }
  else
  {
    //为非0值时，均设置为开启写保护
    HartDataBuffer[DataFieldStartAddress+14] = 0x5a;
  }
  for(i = 0; i < 12 ;i++)
  {
    CheckCode[i] = HartDataBuffer[DataFieldStartAddress+2+i];
  }
  Save_Eeprom_EnablePara_Protect(&HartDataBuffer[DataFieldStartAddress+2]);
  for(i = 0; i < 12 ;i++)
  {
    HartDataBuffer[DataFieldStartAddress+4+i] = CheckCode[i];
  }
  //5A是开启写保护，A5是关闭写保护
  if(*GetPara_Char(Eeprom_EnablePara_ProtectIndex) == 0xA5)
  {
    HartDataBuffer[DataFieldStartAddress+4] = 0;
  }
  else if(*GetPara_Char(Eeprom_EnablePara_ProtectIndex) == 0x5a)
  {
    HartDataBuffer[DataFieldStartAddress+4] = 1;
  }
  else
    HartDataBuffer[DataFieldStartAddress+4] = 250;
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,3,TRUE);
  return TRUE;
}
/*整体更改仪表参数，0x5A保存出厂参数、0x55恢复默认参数、0xA5恢复出厂参数*/
bool8 HARTCMD142()// base para
{
  SetAllUserPara_HandleMethod(MODIFYPARAMODE_HART,HartDataBuffer[DataFieldStartAddress+2]);
  
  HartDataBuffer[DataFieldStartAddress+4] = GetAllUserPara_HandleMethod();
  //后续需要添加数据有效性检测，参考通用命令手册,判断无误后才能保存参数，
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,3,FALSE);
  return TRUE;
}
static void HARTCMD143Config()
{
  ReadParaF_Hart(4, &HartDataBuffer[DataFieldStartAddress+4]);
  ReadParaF_Hart(5, &HartDataBuffer[DataFieldStartAddress+8]);
}
/*返回仪表较准零点参数（浮点数）、较准比例参数（浮点数）*/
bool8 HARTCMD143()// base para
{
  HARTCMD143Config();
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,10,TRUE);
  return TRUE;
}
/*修改仪表零点较准参数（浮点数）、比例修正参数（浮点数）*/
bool8 HARTCMD144()
{
  float SystemZeroDist = 0,SystemProportion = 0;
  if(HartDataBuffer[DataFieldStartAddress+1] < 8)
  {
    HartRespondCodeConfig(Error,TooFewDataBytesReceived);
  }
  else
  {
    SystemZeroDist = *((RFrameCharToFloat(DataFieldStartAddress+2)));
    SystemProportion = *((RFrameCharToFloat(DataFieldStartAddress+6)));
    if(SystemZeroDist > GetPara_FloatMax(SystemZeroDistIndex)
       ||(SystemProportion > GetPara_FloatMax(SystemProportionIndex)))
    {
      HartRespondCodeConfig(Error,PassedParameterTooLarge);
    }
    else if((SystemZeroDist < GetPara_FloatMin(SystemZeroDistIndex))
            ||(SystemProportion < GetPara_FloatMin(SystemProportionIndex)))
    {
      HartRespondCodeConfig(Error,PassedParameterTooSmall);
    }
    else
    {
      SavePara_FloatType(MODIFYPARAMODE_HART,SystemZeroDistIndex,RFrameCharToFloat(DataFieldStartAddress+2));
      SavePara_FloatType(MODIFYPARAMODE_HART,SystemProportionIndex,RFrameCharToFloat(DataFieldStartAddress+6));
      HARTCMD143Config();
    }
  }
  //异常需要更新响应码。
  RespondCodeAndDeviceStatusHandle(RespondCodeNormal,10,FALSE);
  return TRUE;
}
#ifdef EXTENDED_COMMAND
/*返回仪表高低位映射*/
uint8_t HARTCMD245()//read 100per and 0per value
{
	HartDataBuffer[DataFieldStartAddress+1] = 11;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;

	HartDataBuffer[DataFieldStartAddress+4] = 0;//HART_Variables.DataAccuracy;//这个参数现在没有用
	ReadParaF_Hart(12, &HartDataBuffer[DataFieldStartAddress+5]);
	ReadParaF_Hart(13, &HartDataBuffer[DataFieldStartAddress+9]);

	return(245);
}

/*修改仪表高低位映射*/
uint8_t HARTCMD246()//write 100per and 0per value
{
	SavePara_FloatType(MODIFYPARAMODE_HART,12,RFrameCharToFloat(DataFieldStartAddress+2));
	;
	SavePara_FloatType(MODIFYPARAMODE_HART,13,RFrameCharToFloat(DataFieldStartAddress+6));

	HARTCMD245();
	return(246);
}


/*返回仪表频率值（浮点数）、距离值（浮点数）、物料回波位置、物料回波最大值*/
unsigned char HARTCMD186()
{
	float TempF;
	TempF = GetBestEchoAmplitude();
	FloatToTFrameChar(HartReadMasterVariable(1),DataFieldStartAddress+4);
	FloatToTFrameChar(TempF,DataFieldStartAddress+8);
	HartDataBuffer[DataFieldStartAddress+12] =  GetMarkBestWavePostion();;
	HartDataBuffer[DataFieldStartAddress+13] = (uint8_t)(GetBestEchoAmplitude()/AMPLITUDE_COV_SAMPLE_255);
	HartDataBuffer[DataFieldStartAddress+1] = 12;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;
	return(186);
}

/*返回仪表信噪比、距离值（浮点数）、物料回波位置、分辨率*/
unsigned char HARTCMD187()
{
	FloatToTFrameChar(HartReadMasterVariable(1),DataFieldStartAddress+4);
	HartDataBuffer[DataFieldStartAddress+8] = 0;//(unsigned char)SignalNoiseRadio;
	HartDataBuffer[DataFieldStartAddress+9] =  GetMarkBestWavePostion();;
	HartDataBuffer[DataFieldStartAddress+10] = (uint8_t)(GetBestEchoDistinctiveness());
	HartDataBuffer[DataFieldStartAddress+1] = 9;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;
	return(187);
}

/*仪表根据接收到数据，分段创建虚假回波、新建虚假回波*/
unsigned char HARTCMD189()// base para
{
	float TempF[3];
	TempF[0] = *((RFrameCharToFloat(DataFieldStartAddress+2)));
	TempF[1] = *((RFrameCharToFloat(DataFieldStartAddress+6)));
	TempF[2] = *((RFrameCharToFloat(DataFieldStartAddress+10)));
	CreatFakeCurve(TempF);
	
	HartDataBuffer[DataFieldStartAddress+1] = 10;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;
	return(189);
}
/*仪表根据接收到的起点和终点数据更新回波曲线*/
void	HARTCMD188()//creat empty wave
{
	float TempF[2];
	TempF[0] = *(RFrameCharToFloat(DataFieldStartAddress+2));
	TempF[1] = *(RFrameCharToFloat(DataFieldStartAddress+6));
	SetCurveStart_EndPoint(TempF[0],TempF[1]);
	
	HartDataBuffer[DataFieldStartAddress+1] = 10;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;
}

/*返回仪表电流方向、启动电流、信号阈值*/
unsigned char HARTCMD230()//read current property
{
	HartDataBuffer[DataFieldStartAddress+1] = 4;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;

	HartDataBuffer[DataFieldStartAddress+4] = *GetPara_Char(3);
	HartDataBuffer[DataFieldStartAddress+5] = *GetPara_Char(14);
        HartDataBuffer[DataFieldStartAddress+7] = *GetPara_Char(15);
	return(230);
}

/*修改仪表电流方向、启动电流、信号阈值*/
unsigned char HARTCMD231()//write current property
{
	SavePara_CharType(MODIFYPARAMODE_HART,3,HartDataBuffer[DataFieldStartAddress+2]);
	SavePara_CharType(MODIFYPARAMODE_HART,14,HartDataBuffer[DataFieldStartAddress+3]);
        SavePara_CharType(MODIFYPARAMODE_HART,15,HartDataBuffer[DataFieldStartAddress+5]);
	HARTCMD230();
	return(231);
}

/*修改仪表日期*/
unsigned char HARTCMD242()//write Date
{
	uint8_t TempC;

	SavePara_StringType(18,&HartDataBuffer[DataFieldStartAddress+2]);
	
	HartDataBuffer[DataFieldStartAddress+1] = 10;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;
	for(TempC = 0; TempC < 8; TempC++)
	{
          HartDataBuffer[DataFieldStartAddress+4+TempC] = *(GetPara_Char(18)+TempC);
	}

	return(242);
}

/*修改仪表序列号*/
uint8_t HARTCMD243()//write PVSensorSN
{
	uint8_t TempC;
	SavePara_StringType(19,&HartDataBuffer[DataFieldStartAddress+2]);

	HartDataBuffer[DataFieldStartAddress+1] = SENSOR_SN_LENGTH+2;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;
	for(TempC = 0; TempC < SENSOR_SN_LENGTH; TempC++)
	{
          HartDataBuffer[DataFieldStartAddress+4+TempC] = *(GetPara_Char(19)+TempC);
	}
	return(243);
}

/*修改仪表描述*/
uint8_t HARTCMD244()//write FlagOfChange_Descriptor
{
	uint8_t TempC;
	SavePara_StringType(20,&HartDataBuffer[DataFieldStartAddress+2]);

	HartDataBuffer[DataFieldStartAddress+1] = DESCRIPTOR_LENGTH+2;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;
	for(TempC = 0; TempC < DESCRIPTOR_LENGTH; TempC++)
	{
          HartDataBuffer[DataFieldStartAddress+4+TempC] = *(GetPara_Char(20)+TempC);
	}
	return(244);
}


void	HARTCMD251(void)//自己定义的命令，手操器读产品出厂日期
{
	uint8_t TempC;
	HartDataBuffer[DataFieldStartAddress+1] = DATE_LENGTH+2;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;

	for(TempC = 0; TempC < DATE_LENGTH; TempC++)
		HartDataBuffer[DataFieldStartAddress+TempC+4] = *(GetPara_Char(18)+TempC);
}

void	HARTCMD252(void)// 自己定义的命令，手操器读产品序列号
{
	uint8_t TempC;
	HartDataBuffer[DataFieldStartAddress+1] = SENSOR_SN_LENGTH+2;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;

	for(TempC = 0; TempC < SENSOR_SN_LENGTH; TempC++)
          HartDataBuffer[DataFieldStartAddress+TempC+4] = *(GetPara_Char(19)+TempC);
}

void	HARTCMD253(void)// 自己定义的命令，手操器读产品型号
{
	uint8_t TempC;
	HartDataBuffer[DataFieldStartAddress+1] = DESCRIPTOR_LENGTH+2;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;

	for(TempC = 0; TempC < DESCRIPTOR_LENGTH; TempC++)
          HartDataBuffer[DataFieldStartAddress+TempC+4] = *(GetPara_Char(20)+TempC);
}

void	HARTCMD254(void)// 自己定义的命令，手操器读产品版本号
{
	HartDataBuffer[DataFieldStartAddress+1] = 5;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
	HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;
	
	HartDataBuffer[DataFieldStartAddress+4] = VERSION0;
	HartDataBuffer[DataFieldStartAddress+5] = VERSION1;
	HartDataBuffer[DataFieldStartAddress+6] = VERSION2;

	HartDataBuffer[DataFieldStartAddress+7] = (uint8_t)(Get_IWR1443_Version());
	HartDataBuffer[DataFieldStartAddress+8] = (uint8_t)(Get_IWR1443_Version()>>8);
}
#endif 
/* HART接收帧正确处理函数，当接收帧符合HART协议后，通过本函数进行响应处理 */
static bool8 HartRFrameTrueHandle()
{
  switch ( HartDataBuffer[DataFieldStartAddress] )
  {
    case 0:
            return HARTCMD0();
            break;
    case 1:
            return HARTCMD1();
            break;
    case 2:
            return HARTCMD2();
            break;
    case 3:
            return HARTCMD3();
            break;
    case 6:
            return HARTCMD6();
            break;
    case 7:
            return HARTCMD7();
            break;
    case 8:
            return HARTCMD8();
            break;
    case 9:
            return HARTCMD9();
            break;
    case 11:
            return HARTCMD11();
            break;
    case 12:
            return HARTCMD12();
            break;
    case 13:
            return HARTCMD13();
            break;
    case 14:
            return HARTCMD14();
            break;
    case 15:
            return HARTCMD15();
            break;
    case 16:
            return HARTCMD16();
            break;
    case 18:
            return HARTCMD18();
            break;
    case 17:
            return HARTCMD17();
            break;
    case 19:
            return HARTCMD19();
            break;
    case 20:
            return HARTCMD20();
            break;
    case 21:
            return HARTCMD21();
            break;
    case 22:
            return HARTCMD22();
            break;
    case 34:
            return HARTCMD34();
            break;
    case 35:
            return HARTCMD35();
            break;
    case 38:
            return HARTCMD38();
            break;            
    case 40:
            return HARTCMD40();
            break;
    case 44:
            return HARTCMD44();
            break;
    case 45:
            return HARTCMD45();
            break;
    case 46:
            return HARTCMD46();
            break;  
    case 48:
            return HARTCMD48();
            break;
    case 49:
            return HARTCMD49();
            break;
    case 128:
            return HARTCMD128();
            break;
    case 129:
            return HARTCMD129();
            break;
    case 130:
            return HARTCMD130();
            break;
    case 131:
            return HARTCMD131();
            break;
    case 132:
            return HARTCMD132();//WRITE PV_UPPER_SENSOR_LIMIT
            break;
    case 133:
            return HARTCMD133();
            break;
    case 134:
            return HARTCMD134();
            break;
    case 135:
            return HARTCMD135();
            break;
    case 136:
            return HARTCMD136();
            break;
    case 137:
            return HARTCMD137();
            break;
    case 138:
            return HARTCMD138();
            break;
    case 139:
            return HARTCMD139();
            break;
    case 140:
            return HARTCMD140();
            break;
    case 141:
            return HARTCMD141();
            break;
    case 142:
            return HARTCMD142();
            break;
    case 143:
            return HARTCMD143();
            break;
    case 144:
            return HARTCMD144();
            break;
            
#ifdef EXTENDED_COMMAND
    case 186:
            HARTCMD186();
            break;
    case 187:
            HARTCMD187();
            break;
    case 188:
            HARTCMD188();
            break;
    case 189:
            HARTCMD189();
            break;
    case 190:
    case 191:
    case 192:
    case 193:
    case 194:
//				TempM = CMD-190;	
//				TempM = 20*TempM;
            HartDataBuffer[DataFieldStartAddress+1] = 22;
            HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = FieldDeviceNormal;
            HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;

            for(uint8_t TempC = 0; TempC < 20; TempC++)
                    HartDataBuffer[DataFieldStartAddress+4+TempC] = LcdEcho[20*(HartDataBuffer[DataFieldStartAddress]-190)+TempC];
            if(HartDataBuffer[DataFieldStartAddress] == 194)
                    SetCurveStart_EndPoint(0,0);
            break;
    
    case 230:
            HARTCMD230();
            break;
    case 231:
            HARTCMD231();
            break;
    case 242:
            HARTCMD242();
            break;
    case 243:
            HARTCMD243();
            break;
    case 244:
            HARTCMD244();
            break;
    case 245:
            HARTCMD245();
            break;
    case 246:
            HARTCMD246();
            break;
    case 251:
            HARTCMD251();
            break;
    case 252:
            HARTCMD252();
            break;
    case 253:
            HARTCMD253();
            break;
    case 254:
            return HARTCMD254();
            break;
#endif
    default:
            HartDataBuffer[DataFieldStartAddress+1] = 2;
            HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = CommandNotExcuted;
            HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = RespondCodeNormal;
      break;
  }
  return TRUE;
}
/*
* static void HartColdStartHandle()
* 变量范围: 
*      
*       
* 返回值说明: 
*
* 功能介绍: 
*       冷启动处理函数，复位或掉电需要对设备状态字节更新冷启动报警。
* 关键备注:
*
* 未来提高:
* 
*/
static void HartColdStartHandle()
{
  static uint8_t Primarycoldstartflag = 0x03;
  if(G_HartRespondCodeStatus.Type != Error)
  {
    if(((Primarycoldstartflag>>0 & 0x01) == 0x01)&&((HartDataBuffer[1]>>7&0x01) == 0x01))
    {
      HartDeviceStatusByteHandle(ColdStart,FALSE);
      Primarycoldstartflag &= ~(0x01);
    }
    if(((Primarycoldstartflag>>1 & 0x01) == 0x01)&&((HartDataBuffer[1]>>7&0x01) == 0))
    {
      HartDeviceStatusByteHandle(ColdStart,FALSE);
      Primarycoldstartflag &= ~(0x02);
    }
  }
}
/*
* static void RespondCodeAndDeviceStatusHandle(CommonRespondCodeDef HandleFlag,uint8_t ExactByteNumber)
* 变量范围: 
*       HandleFlag：处理标志，根据传参判断如何设置响应码
*       ExactByteNumber：准确的（也就是响应码没有异常时）的返回字节个数。
* 返回值说明: HART命令号
*
* 功能介绍: 
*       根据HandleFlag进行错误遍历分析，根据结果正确处理响应帧的响应码值。
* 关键备注:
*
* 未来提高:
* 
*/
static void RespondCodeAndDeviceStatusHandle(CommonRespondCodeDef HandleFlag,uint8_t ExactByteNumber,bool8 IsReadCmd)
{
  //冷启动报警
  HartColdStartHandle();
  //一经发现配置更改标志发生改变，立即更新设备状态字节
  HartDeviceStatusConfig(*GetPara_Char(ConfigChangeFlag),ConfigurationChanged);
  HartDeviceStatusConfig(G_MoreStatusAvailableBit,MoreStatusAvailable);
  
  //判断写保护，同时确认命令是否为读命令，读命令可以在写保护状态下访问数据
  if((*GetPara_Char(Eeprom_EnablePara_ProtectIndex) == ENABLE_WRITEEEPROM)
     &&(IsReadCmd == FALSE))
  {
    HartRespondCodeConfig(Error,WriteProtectMode);
  }
  if((G_HartRespondCodeStatus.Type == Error)
     &&(G_HartRespondCodeStatus.value != 0 ))
  {
      //后续补充其他状态判断函数
      HartDataBuffer[DataFieldStartAddress+1] = 2;
      HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = G_HartRespondCodeStatus.value;
  }
  else if((G_HartRespondCodeStatus.Type == Warning)
     &&(G_HartRespondCodeStatus.value != 0 ))
  {
    HartDataBuffer[DataFieldStartAddress+1] = ExactByteNumber;
    HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = G_HartRespondCodeStatus.value;
  }
  else
  {
    //后续补充其他状态判断函数
    HartDataBuffer[DataFieldStartAddress+1] = ExactByteNumber;
    HartDataBuffer[DataFieldStartAddress+RESPONSECODE_L] = RespondCodeNormal;
  }
  HartDataBuffer[DataFieldStartAddress+RESPONSECODE_H] = G_HartDeviceStatus;
  HartRespondCodeConfig(Notification,0);
  G_HartDeviceStatus = FieldDeviceNormal;
}
/* HART 多状态应用模式状态核实更新函数，一旦当前48号命令相关的设备状态发生改变，
立即更新到多状态模式变量上， */
static void HartMoreStatusAvailableModeVarify()
{
  if(G_PMasterStatusCachedValue == WorkStatus)
  {
    G_MoreStatusAvailableBit &= ~0x01;
  }
  else
  {
    G_MoreStatusAvailableBit |= 0x01;
  }
  if(G_SMasterStatusCachedValue == WorkStatus)
  {
    G_MoreStatusAvailableBit &= ~0x02;
  }
  else
  {
    G_MoreStatusAvailableBit |= 0x02;
  }
}
/* 回路电流模式设备状态核实更新函数，一旦电流模式发生改变，立即更新到设备状态字节上 */
static void HartLoopCurrentModeVarify()
{
  float SetOutCurrentValue = 0;
  if(*GetPara_Char(LoopCurrentModeIndex) == 0 )
  {
    SetOutCurrentValue = SetCurrentGet();
    SetOutCurrentVal(SetOutCurrentValue);
    HartDeviceStatusByteHandle(LoopCurrentFixed,FALSE);
  }
  else if(*GetPara_Char(LoopCurrentModeIndex) == 1 )
  {
    HartDeviceStatusByteHandle(LoopCurrentFixed,TRUE);
    SetOutCurrentVal(0);
  }
}
/* 配置更改标志核实更新函数，一旦设备配置标志发生变化，立刻更新到对应的设备状态字节上 */
static void HartDeviceStatusConfig(uint8_t flag,HartDeviceStatusDef DeviceStatus)
{
  //主机设备
  if(IS_PRIMARY_MASTER)
  {
    if((flag & 0x01) == 1)
    {
      HartDeviceStatusByteHandle(DeviceStatus,FALSE);
    }
    else if((flag & 0x01) == 0)
    {
      HartDeviceStatusByteHandle(DeviceStatus,TRUE);
    }
  }
  //副主机设备
  if(IS_SECOND_MASTER)
  {
    if((flag>>1 & 0x01) == 1)
    {
      HartDeviceStatusByteHandle(DeviceStatus,FALSE);
    }
    else if((flag>>1 & 0x01) == 0)
    {
      HartDeviceStatusByteHandle(DeviceStatus,TRUE);
    }
  }
}
/* 设备状态码核对，一旦发生设备状态问题，立即更新设备状态码 */
static void HartDeviceStatusVarify(void )
{
  //一经发现回路电流模式关闭，即刻更新设备固定电流状态码
  HartLoopCurrentModeVarify();
  //More Status Available 多状态应用位走查
  HartMoreStatusAvailableModeVarify();
}
/* HART协议分析函数，用来分析接收帧，并进行响应处理 */
void	AnalyzeHartProtocol(void)
{
  HartDataAnanlysisHandle();
  HartDeviceStatusVarify();
}
/* HART数据分析处理，通过本函数进行异常应答和正常应答 */
static void HartDataAnanlysisHandle()
{
  bool8 IsRequest = TRUE;
  //选择液晶485参数
  if(*GetPara_Char(EnableHartIndex) == 0)
  {
    if(G_FrameFieldStatus ==  FrameCountCorrectly)
    {
      if(Hart_ReceiveCompleteHandle() == TRUE)
      {
        IsRequest = HartRFrameTrueHandle();
        //有些命令是不需要应答的，比如命令11的tag比较不匹配时
        if(IsRequest ==  TRUE)
        {
          HartDataBuffer[0] |= 0x04;
          EnableHartTx();
        }
      } 
      else
      {
        EnableHartRx();
      }
    }
    else if(G_FrameFieldStatus == FrameGapError )
    {
      EnableHartRx();
    }
  }
}
/* HART数据帧接收完成判断，同时计算出数据帧记录的字节个数，
与实际接收的字节个数进行比较，实现响应通信错误码的设置 */
void HART_GapCheck()
{
  HART_GapCounterDecrease();
  //响应帧所有数据接收完成
  if((G_FrameFieldStatus == FrameReceive)&&(G_FrameGapCounter == 0))
  {
    /* Clear RXNE interrupt flag */
    __HAL_UART_SEND_REQ(&huart1, UART_RXDATA_FLUSH_REQUEST);
    HAL_UART_AbortReceive_IT(&huart1);
    G_FrameFieldStatus = FrameDelimiterAnalysis;
    DelimiterFlag = 0;
  }
  if(G_FrameFieldStatus == FrameDelimiterAnalysis)
  {
      if(TxRxCount < FrameByteNuber)
      {
        G_FrameFieldStatus = FrameGapError;
      }
      else if(TxRxCount >= FrameByteNuber)
      {
        G_FrameFieldStatus =  FrameCountCorrectly;
      }
  }
  if(G_FrameFieldStatus == FrameResponding)
  {
    //应答动作完成后，切换状态机
    if(huart1.gState == HAL_UART_STATE_READY)
    {
      G_FrameFieldStatus = FrameRespondOver;
    }
  }
  if(G_FrameFieldStatus == FrameRespondOver)
  {
    //完成应答帧发送后，重新启动接收程序。
    EnableHartRx();
  }
}
/* 
HART定界符配置函数，因为HART协议规定一些定界符是合法的，
但是在最终处理时都有使用物理层规定的形式处理，因此需要使用本函数对
定界符进行统一配置 
*/
static void HartDelimterConfig()
{
  if(DataFieldStartAddress == ShortAddressIndex)
  {
    HartDataBuffer[0] = SHORTFRAME_M;
  }
  else if(DataFieldStartAddress == LongAddressIndex)
  {
    HartDataBuffer[0] = LONGFRAME_M;
    if(IS_BROADCAST_ADDRESS == TRUE)
    {
      HartDataBuffer[1] = (DEVICE_TYPE0 & 0x3f);
      HartDataBuffer[2] = DEVICE_TYPE;
      HartDataBuffer[3] = DEVICE_ID0;
      HartDataBuffer[4] = DEVICE_ID1;
      HartDataBuffer[5] = DEVICE_ID2;
    }
  }
}
/* HART burst mode 配置函数 */
static void HartBurstModeConfig(bool8 IsBurst)
{
  if(IsBurst == FALSE)
  {
    HartDataBuffer[1] &= ~(1 << 6);
  }
  else if(IsBurst == TRUE)
  {
    HartDataBuffer[1] |= (1 << 6);
  }
}
/* HART 接收完整帧处理函数 */
static bool8 Hart_ReceiveCompleteHandle()
{
  uint8_t  TempC,TempAddress;
  HartWorkCount++;
  if(HartWorkCount > MAXRESETPERF_COUNT)
  {
    ResetPerfData(HARTCOMERRORTIMES_TH);
    HartWorkCount = 0;
  }
  RecordPerfToFRAM(HARTCOMTIMES_TOTAL);
  

  TxRxCount = 0;
  if((IS_VALID_SHORT_F_DELIMITER)&&((HartDataBuffer[1]&0x3F) == GetComAddress()))
  {
    //短帧只有0命令需要处理，其他命令号均无效
    if(HartDataBuffer[2] != 0)
    {
      /* 定界符异常，复位请求帧接收状态标志 */
      return FALSE;
    }
  }
  else if(IS_VALID_LONG_F_DELIMITER)
  {
    TempC = HartDataBuffer[1]&0x3F;
    TempAddress = DEVICE_ID0;
    if(HART_UNIQUE_ADDRESS_ASSERT() != TRUE )
    {
      if(IS_BROADCAST_ADDRESS != TRUE)
      {
        return FALSE;
      }
    }
  }
  else
  {
    return FALSE;
  }
  if((G_HartRespondCodeStatus.Type == Error)
     &&((G_HartRespondCodeStatus.value == CommunicationMask+VerticalParityError)
     ||(G_HartRespondCodeStatus.value == CommunicationMask+FramingError)))
  {
    return TRUE;
  }
  if((ReckonHartCRC(FrameByteNuber-1) != HartDataBuffer[FrameByteNuber-1]))
  {
    //通信错误需要进行应答，因此正确返回，但不必进行后续核实
    HartRespondCodeConfig(Error,CommunicationMask+LongitudinalParityError);
    RecordPerfToFRAM(HARTCOMERRORTIMES);					
    RecordPerfToFRAM(HARTCOMERRORTIMES_TH);
    return TRUE;
  }
  //为了不影响校验码计算，把定界符赋值放在最后处理
  HartDelimterConfig();
  //设备没有burst mode
  HartBurstModeConfig(FALSE);
  return TRUE;
}
/* 清除校验错误中断标志位 */
static void ParityErrorClear()
{
  huart1.Instance->ICR |= USART_ICR_PECF;
}
/* 奇校验异常处理 */
static void ParityErrorRespondHandle()
{
  HartRespondCodeConfig(Error,CommunicationMask+VerticalParityError);
  ParityErrorClear();
}
/* 奇校验检查函数 */
static bool8 ParityErrorCheck()
{
  if(huart1.ErrorCode & HAL_UART_ERROR_PE)
  {
    return FALSE;
  }
  return TRUE;
}


/* 清除帧错误中断标志位 */
static void FramingErrorClear()
{
  huart1.Instance->ICR |= USART_ICR_FECF;
}
/* 帧错误异常处理 */
static void FramingErrorRespondHandle()
{
  HartRespondCodeConfig(Error,CommunicationMask+FramingError);
  FramingErrorClear();
}
/* 奇校验检查函数 */
static bool8 FramingErrorCheck()
{
  if(huart1.ErrorCode & HAL_UART_ERROR_FE)
  {
    return FALSE;
  }
  return TRUE;
}
/* 奇校验错误应答方式选择 */
static void CommunicationErrorResponseModeSelect(bool8 IsResponse)
{
  if(ParityErrorCheck() == FALSE)
  {
    if(IsResponse == TRUE)
    {
      ParityErrorRespondHandle();
    }
    else if(IsResponse == FALSE)
    {
      EnableHartRx();
    }
  }  
  if(FramingErrorCheck() == FALSE)
  {
    if(IsResponse == TRUE)
    {
      FramingErrorRespondHandle();
    }
    else if(IsResponse == FALSE)
    {
      EnableHartRx();
      FramingErrorClear();
    }
  }
}
/* HART 通信中断处理模块，用来对串口中断收发数据进行处理 */
void Hart_Receive_IT()
{
  //接收帧字节间隔不可以超过10个毫秒，这里设置为15
  G_FrameGapCounter = 15;
  //当前请求帧接收状态为前导码接收
  if(G_FrameFieldStatus == Preamble)
  {
    if(HartReceiveData == 0xFF)
    {
      DelimiterFlag++;
    }
    else
    {
      //如果在前导字节中发现奇校验错误，做无应答处理
      CommunicationErrorResponseModeSelect(FALSE);
      if(DelimiterFlag >= LEAST_PREAM)
      { 
        G_FrameFieldStatus = FrameReceive;
      }
      else
      {     
        EnableHartRx();
      }
    }
  }
  /* HART数据链路层规定，接收两个连续前导符，从机需要应答 */
  if(G_FrameFieldStatus == FrameReceive)
  {
    HartDataBuffer[TxRxCount++] = HartReceiveData;
    if(IS_VALID_SHORT_F_DELIMITER)
    {
      DataFieldStartAddress = ShortAddressIndex;
      FrameByteNuber = HartDataBuffer[3]+4+1;
    }
    else if(IS_VALID_LONG_F_DELIMITER)
    {
      DataFieldStartAddress = LongAddressIndex;
      FrameByteNuber = HartDataBuffer[7]+8+1;
    }
    else
    {
      /* 定界符异常，复位请求帧接收状态标志 */
      EnableHartRx();
    }
    //判定数据帧接收完成，但是不可以停止TxRxCount计数，因为Gap没有超时
    if(TxRxCount > FrameByteNuber)
    {
      //合法帧接收完毕后，屏蔽掉所有通信错误，因为后续字节为非法字节，不必理会。
      if((G_HartRespondCodeStatus.Type == Error)&&
         ((G_HartRespondCodeStatus.value == CommunicationMask+VerticalParityError)
         ||(G_HartRespondCodeStatus.value == CommunicationMask+FramingError)))
      {
        HartRespondCodeConfig(Notification,0);
      }
    }
    else
    {
      if(DataFieldStartAddress == ShortAddressIndex)
      {
        if((TxRxCount <= 2)||(TxRxCount == 4))
        {
          //定界符从机地址及字节个数出现奇校验错误做无应答处理
          CommunicationErrorResponseModeSelect(FALSE);
        }
        else
        {
          //命令，数据，校验码出现奇校验错误做应答处理
          CommunicationErrorResponseModeSelect(TRUE);
        }
      }
      else if(DataFieldStartAddress == LongAddressIndex)
      {
        if((TxRxCount <= 6)||(TxRxCount == 8))
        {
          //定界符从机地址及字节个数出现奇校验错误做无应答处理
          CommunicationErrorResponseModeSelect(FALSE);
        }
        else
        {
          //命令，数据，校验码出现奇校验错误做应答处理
          CommunicationErrorResponseModeSelect(TRUE);
        }

      }
    }
  }
  
  //else
  {
    /* Clear RXNE interrupt flag */
    //__HAL_UART_SEND_REQ(&huart1, UART_RXDATA_FLUSH_REQUEST);
  }
}

/* 数据帧连续判断时间间隔变量自减函数，1毫秒自减1 */
void HART_GapCounterDecrease()
{
  if(G_FrameGapCounter > 0)
  {
    G_FrameGapCounter--;
  }
  else
  {
    G_FrameGapCounter = 0;
  }
}
/* 设备状态字节设定 */
void HartDeviceStatusByteHandle(HartDeviceStatusDef HartDeviceStatus,bool8 IsReset)
{
  if(IsReset == FALSE)
  {
    G_HartDeviceStatus |= HartDeviceStatus;
  }
  else
  {
    G_HartDeviceStatus &= ~HartDeviceStatus;
  }
}
/* HART配置标志变量处理函数，可以清零或置位 */
static uint8_t HartConfigChangeFlagHandle(bool8 IsClear,uint8_t Primarycoldstartflag)
{
   if(IsClear == TRUE)
  {
    //主机设备
    if((HartDataBuffer[1]>>7&0x01) == 0x01)
    {
      Primarycoldstartflag &= ~(0x01);
    }
    //副主机设备
    if((HartDataBuffer[1]>>7&0x01) == 0)
    {
      Primarycoldstartflag &= ~(0x02);
    }
  }
  else if(IsClear == FALSE)
  {
    Primarycoldstartflag = 3;
  }
  return Primarycoldstartflag;
}
static void HartConfigChangeFlagSave(bool8 IsReset)
{
  //写保护加密码
  uint8_t ProtectCode[13] = {
    0x87,0x53,0x27,0x51,0x21,0x76,
    0x78,0xAC,0xD8,0xAE,0xDE,0x89,0xA5
  };
  uint8_t Primarycoldstartflag = 0;
  //从EEPROM读出当前状态
  Primarycoldstartflag = *GetPara_Char(ConfigChangeFlag);
  //在当前状态基础上，对相应的主机设备进行状态处理
  Primarycoldstartflag = HartConfigChangeFlagHandle(IsReset,Primarycoldstartflag);
  //在写保护模式下，可以自由清除配置更改标志
  if(*GetPara_Char(Eeprom_EnablePara_ProtectIndex) == ENABLE_WRITEEEPROM)
  {
    Save_Eeprom_EnablePara_Protect(ProtectCode);
    SavePara_CharType(MODIFYPARAMODE_HART,ConfigChangeFlag,Primarycoldstartflag);
    ProtectCode[12] = 0x5A;
    Save_Eeprom_EnablePara_Protect(ProtectCode);
  }
  else
  {
    SavePara_CharType(MODIFYPARAMODE_HART,ConfigChangeFlag,Primarycoldstartflag);
  }
}
/* 复位配置更改标志处理函数 */
static void HartConfigChangeFlagReset()
{
  HartConfigChangeFlagSave(TRUE);
}
/* HART配置更改标志计数自增函数，当从机执行一次配置操作，就会对配置更改计数器进行一次自增操作 */
static void HartConfigChangeFlagCountIncrease()
{
  uint16_t ConfigurationChangeCounter = 0;
  uint8_t CounterBuff[2] = {0};
  ConfigurationChangeCounter = *GetPara_Char(ConfigChangeCounter)<<8;
  ConfigurationChangeCounter += *(GetPara_Char(ConfigChangeCounter)+1);
  ConfigurationChangeCounter++;
  CounterBuff[0]  = ConfigurationChangeCounter>>8;
  CounterBuff[1] = ConfigurationChangeCounter;
  SavePara_StringType(ConfigChangeCounter,CounterBuff);
  //更改一次配置需要将对应的配置位置1
  HartConfigChangeFlagSave(FALSE);
}
/* 物理层测试函数 */
void HART_PhysicalTestProgress1()
{
  uint8_t TempC = 0;
  ENABLE_HART_TX;
  TxNumber = 31;
  HartDataBuffer[TxNumber] = ReckonHartCRC(TxNumber);
  for(TempC = 0; TempC < TxNumber; TempC++)
  {
          HartDataBuffer[TempC] = 0x00;
  }
  HAL_UART_Transmit_IT(&huart1, (uint8_t *)HartDataBuffer, TxNumber-1);
}
/* 物理层测试函数 */
void HART_PhysicalTestProgress2(uint8_t flag)
{
  if(flag == 0)
  {
    SetOutCurrentVal(20.0);  
  }
  else
  {
    SetOutCurrentVal(4.0);
  }
}

/* HART 串口 中断回调函数 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1)
  {
    Hart_Receive_IT();
    HartReceiveData = 0;
    HAL_UART_Receive_IT(&huart1, &HartReceiveData, 1);
  }
}

/* HART单位码转换为真码函数 */
RealUnitCodeDef HartUnitCodeConvertToRealCode(HartUnitDef HartUnitCode)
{
  RealUnitCodeDef realCode = RealUnit_ndef;
  switch(HartUnitCode)
  {
  case Unit_ft:
    realCode = RealUnit_ft;
    break;
  case Unit_m:
    realCode = RealUnit_m;
    break;
  case Unit_in:
    realCode = RealUnit_in;
    break;
  case Unit_cm:
    realCode = RealUnit_cm;
    break;
  case Unit_mm:
    realCode = RealUnit_mm;
    break;
  default:
    realCode = RealUnit_ndef;
    break;
  }
  return realCode;
}
/* 真码转换为HART单位码函数 */
HartUnitDef RealCodeConvertToHartUnitCode(RealUnitCodeDef RealUnitCode)
{
  HartUnitDef realCode = Unit_ndef;
  switch(RealUnitCode)
  {
  case RealUnit_ft:
    realCode = Unit_ft;
    break;
  case RealUnit_m:
    realCode = Unit_m;
    break;
  case RealUnit_in:
    realCode = Unit_in;
    break;
  case RealUnit_cm:
    realCode = Unit_cm;
    break;
  case RealUnit_mm:
    realCode = Unit_mm;
    break;
  default:
    realCode = Unit_ndef;
    break;
  }
  return realCode;
}