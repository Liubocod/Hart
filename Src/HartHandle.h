#include "DataType.h"
#define	RXBUFFERSIZE_HART		255

#define	LONGFRAME_M			        0x82
#define	LONGFRAME_VALID1			0x8A
#define	LONGFRAME_VALID2			0x92
#define	LONGFRAME_VALID3			0x9A

#define	SHORTFRAME_M			        0x02
#define	SHORTFRAME_VALID1			0x0A
#define	SHORTFRAME_VALID2			0x12
#define	SHORTFRAME_VALID3			0x1A


#define	LONGFRAME_S			0x86
#define	SHORTFRAME_S			0x06
#define	LONGADDRESSNUMBER		5
#define	SHORTADDRESSNUMBER		1

#define ACTUALCONTENTBYTES		1
#define RESPONSECODE_L			2
#define RESPONSECODE_H			3

#define CMD_ILLEGAL			0xFF






//命令0xae [0x2c(空高)、0x19(单位)、0x30(料高)、0xfa(1个字节)、0x29(1个字节)、0xba(2个字节)、0x02(2个字节)
/*manufacture define for hart device identification*/
#define MANUFACTURE_ID0			0x61
#define MANUFACTURE_ID			0x53
#define DEVICE_TYPE0			0xE5
#define DEVICE_TYPE			0x40
#define DEFAULT_PREAM			0x05
#define LEAST_PREAM			0x02  //链路层最低响应前导符个数
#define UNIV_CMD_REV			0x07
#define TRANS_SPEC_REV			0x01
#define SOFT_REV 			0x01
#define HARD_REV			0x00
#define FLAGS				0x00
#define DEVICE_ID			0xBB5577
#define DEVICE_ID0			0x40
#define DEVICE_ID1			0x9c
#define DEVICE_ID2			0x80
typedef enum
{
  HART_RFRAME_STANDBY,//空闲状态
  HART_RFRAME_TRUE,//接收帧正确无误
  HART_RFRAME_COUNT_ERR,//计数错误处理
  HART_RFRAME_PARITY_ERR,//奇校验错误处理
}HartReceiveFrameStatusDef;

//Specific char-type param index define.
typedef enum
{
FakeWaveGainIndex =	                0,
DampingTimeIndex =	                1,
DielectricConstantIndex =	        2,
CurrentDirectionIndex =	                3,
LanguageIndex =	                        4,
PVUnitIndex =	                        5,
UserPasswordIndex =	                6,
ComAddressIndex =	                7,

BaudRateIndex =	                        8,
WaveLostProccessSELIndex =	        9,
JumpSELIndex =	                        10,
DLineUseorUnuseIndex =	                11,
RLineUseorUnuseIndex =	                12,
CurrentAlarmSELIndex =	                13,
CurrentStartSELIndex =	                14,
ThresholdIndex =	                15,
EchoAdjudgeSELIndex =	                16,
LoopCurrentModeIndex =  	        17,
PDateIndex =	                        18,
PSNIndex =	                        19,
PDescriptorIndex =	                20,
PTagIndex =	                        21,
FinalAsmNoIndex =	                22,
Enable_LCD485Index =	                23,
EnableHartIndex =	                24,
Eeprom_EnablePara_ProtectIndex =	25,
ComAddress_485Index =	                26,
ParityIndex =	                        27,
StopBitsIndex =	                        28,
WordLengthIndex =	                29,
PLongTag =      	                30,
ConfigChangeFlag =       	        31,
ConfigChangeCounter =       	        32,
PMessageIndex =	                        33,
}ParamC_IndexDef;
//Specific float-type param index define.
typedef enum
{
  LowerRangeIndex = 0,
  UpperRangeIndex = 1,
  RadarLengthIndex = 2,
  InputM_SpeedIndex = 3,
  SystemZeroDistIndex = 4,
  SystemProportionIndex = 5,
  HartUpperRangeIndex = 6,
  HartLowerRangeIndex = 7,
  OutputM_SpeedIndex = 8,
  CurrentEchoAIndex = 9,
  PWM4mAIndex = 10,
  PWM18mAIndex = 11,
  MappingUpperIndex = 12,
  MappingLowIndex = 13,
  WaveLostSetPositionIndex = 14,
  JumpDistIndex = 15,
  JumpWaitTimeIndex = 16,
  JumpChangeSpeedIndex = 17,
  FrontEchoAIndex = 18,
  ConfirmEchoTimeIndex = 19,
  MinDistinctivenessIndex = 20,
}ParamF_IndexDef;
typedef enum 
{
  DeviceMalfunction = 0x80,
  ConfigurationChanged = 0x40,
  ColdStart = 0x20,
  MoreStatusAvailable = 0x10,
  LoopCurrentFixed = 0x08,
  LoopCurrentSaturated = 0x04,
  NonPrimaryVariableOutofLimits = 0x02,
  PrimaryVariableOutofLimits = 0x01,
  FieldDeviceNormal = 0x00,
}HartDeviceStatusDef;
typedef enum
{
  Unit_ft = 44,
  Unit_m = 45,
  Unit_in = 47,
  Unit_cm = 48,
  Unit_mm = 49,
  Unit_ndef = 255,
}HartUnitDef;
typedef enum
{
  RealUnit_ft = 0,
  RealUnit_m = 1,
  RealUnit_in = 2,
  RealUnit_cm = 3,
  RealUnit_mm = 4,
  RealUnit_ndef = 255,
}RealUnitCodeDef;
void	AnalyzeHartProtocol(void);
void	EnableHartTx(void);
void	EnableHartRx(void);
void	UART1_IRQHandler(void);
//test
void HART_PhysicalTestProgress2(uint8_t flag);
void HART_PhysicalTestProgress1();
void HartDeviceStatusByteHandle(HartDeviceStatusDef HartDeviceStatus,bool8 IsReset);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HART_GapCheck();