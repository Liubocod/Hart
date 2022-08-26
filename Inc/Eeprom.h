/**
 ******************************************************************************
  * File Name          : Eeprom.h

*/
#ifndef __Eeprom_H
#define __Eeprom_H
#ifdef __cplusplus
 extern "C" {
#endif

   
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "main.h"

#define ENABLEWRITEEEPROM			HAL_GPIO_WritePin(EEROM_WP_GPIO_Port, EEROM_WP_Pin, GPIO_PIN_RESET);
#define DISABLEWRITEEEPROM			HAL_GPIO_WritePin(EEROM_WP_GPIO_Port, EEROM_WP_Pin, GPIO_PIN_SET);

#define EEPROM_PAGESIZE				32     /* EEPROM used     */
#define EEPROM_ADDRESS				0xA0    /* EEPROM Device Address  */
#define FACTORYPARA_EEPROM_ADDRESS		2048    /* factory para EEPROM Address  参数空间为2K byte*/
#define FAKEINFO_EEPROM_ADDRESS			4096    /*  虚假回波信息空间为2K byte*/
#define FAKEINFO_MAX_INDEX			32    /*  虚假回波信息空间为2K byte*/
#define PARAMODIFYINFOINDEX			6144	/*参数更改记录空间为256 byte 12*20*/

#define MODIFYPARAINFO_MAXCOUNT			12

#define WRITEEEPROM_END				0x5A
#define READEEPROM_END				0xA5
#define STARTREADWRITEEEPROM			0
#define READWRITEEEPROM_MAXTIME			300//ms

#define WRITEEEPROM_DELAY			5//mS

#define FIRST_TIME				0xFFFF
#define NOT_FIRST_TIME				0xA55A

#define	BYTE_LENGTH				1
#define	FLOAT_LENGTH				4

#define TAG_LENGTH 				8
#define DESCRIPTOR_LENGTH 			16
#define FINAL_ASM_NO_LENGTH 			3
#define SENSOR_SN_LENGTH 			10
#define DATE_LENGTH 				8
#define USERPASSWORD_LENGTH 			5
//Mr.liu's code
//HART新增宏定义
#define MESSAGE_LENGTH 			        24
#define LONG_TAG_LENGTH 			32
#define ADDITIONAL_DEV_STATUS 			25
   

#define EXPANDFLOATPARANUMBERS			40
#define FLOATEXPANDPARA_STARTA			21
#define FLOATPARANUMBERS			FLOATEXPANDPARA_STARTA+EXPANDFLOATPARANUMBERS

#define EXPANDCHARPARANUMBERS			40
//Mr.liu modified！！！
#define CHAREXPANDPARA_STARTA			34
#define CHARPARANUMBERS				CHAREXPANDPARA_STARTA+EXPANDCHARPARANUMBERS

#define USERPARATOFACTORPARA			0x5A
#define FACTORYPARATOUSERPARA			0xA5
#define DEFAULTPARATOUSERPARA			0x55
#define USERPARA_NO_OPS				0

#define DISABLEPARA_PROTECT			0xA5

#define ENABLE_WRITEEEPROM			0x5A
#define DISABLE_WRITEEEPROM			0

#define CHECKRAMPARA_PERIOD			30//s

#define FACTORY_ID_ADDRESS			26

#define USERMAXLOWERRANGE_D			8
#define USERMAXLOWERRANGE_R			15
#define USERMAXLOWERRANGE_U			30
typedef enum
{
	WAVELOST_SEL_REMAIN			= 0x00,   
	WAVELOST_SEL_ALARM			= 0x01,  
	WAVELOST_SEL_SETPOSITION		= 0x02,
	WAVELOST_SEL_TREND			= 0x03,
}WavelostSEL_TypeDef;

typedef enum
{
	JUMP_SEL_DIRECT				= 0x00,   
	JUMP_SEL_WAIT				= 0x01,  
	JUMP_SEL_SPEED				= 0x02,
	JUMP_SEL_TREND				= 0x03,
}JumpSEL_TypeDef;

typedef enum
{
	ECHOADJUDGE_SEL_MAX			= 0x00,   
	ECHOADJUDGE_SEL_FRONT			= 0x01,  
	ECHOADJUDGE_SEL_CONTINUITY		= 0x02,
	ECHOADJUDGE_SEL_RATIO			= 0x03,
}EchoAdjudgeSEL_TypeDef;

typedef enum
{
	USE					= 0x00,   
	UNUSE					= 0x01,  
}EnableLine_TypeDef;

#define PV_UNIT_DEFAULT				0x2D
#define UPPERRANGE_DEFAULT			0
#define LOWERRANGE_DEFAULT			20
#define UPPERRANGELIMIT_DEFAULT			0
#define LOWERRANGELIMIT_DEFAULT			20
#define	DAMPINGTIME_DEFAULT			10
#define PWM4MA_DEFAULT				63866
#define PWM18MA_DEFAULT				21000

#define PARATABLE_INDEX_ADDRESS			0
#define PARATABLE_INDEX_DEFAULT_V		1
#define PARATABLE_INDEX_MAX_V			2
#define PARATABLE_INDEX_MIN_V			3
#define PARATABLE_INDEX_BYTES			4
//#define	
typedef	struct
{
	uint8_t	JumpSEL;
	float	JumpDist;
	float	JumpWaitTime;	
	float	JumpChangeSpeed;	
}JumpPara__TypeDef;

typedef	struct
{
	uint8_t	WaveLostProccessSEL;
	float	WaveLostSetPosition;	
}WaveLostPara__TypeDef;

typedef	struct
{
	uint8_t	CurrentDirection;
	uint8_t	CurrentAlarmSEL;
	uint8_t CurrentStartSEL;
	float	PWM18mA;
	float	PWM4mA;
}PWMCurrentPara__TypeDef;

typedef	struct
{
	uint8_t	EchoAdjudgeSEL;
	uint8_t	FakeWaveGain;
	uint8_t	Threshold;
	float	FrontEchoA;
	float	CurrentEchoA;
	float	ConfirmEchoTime;
	float	MinDistinctiveness;
}EchoAdjudgePara__TypeDef;

typedef	struct
{
	float	UpperRange;
	float	LowerRange;
}RangePara__TypeDef;

typedef	struct
{
	float	SystemZeroDist;
	float	SystemProportion;
}DistCoefficientPara__TypeDef;

typedef	struct
{
	float	MappingUpper;
	float	MappingLow;
}MappingPara__TypeDef;

typedef	struct
{
	uint8_t	BaudRate;
	uint8_t	WordLength;
	uint8_t	Parity;
	uint8_t	StopBits;
	uint8_t	ComAddress;
	uint8_t	ComAddress_485;
	uint8_t	EnableHart;
	uint8_t	Enable_LCD485;
	uint8_t LoopCurrentMode;
}ComPara__TypeDef;

typedef	struct
{
	uint8_t	CharPara[EXPANDCHARPARANUMBERS];
	float	FloatPara[EXPANDFLOATPARANUMBERS];
}ExpandPara__TypeDef;

typedef	struct
{
	uint8_t	DampingTime;
	uint8_t	PVUnit;
	uint8_t	Material;
	uint8_t	DielectricConstant;
//Mr.liu modified！！！
        uint8_t	        ConfigChangeFlag;//HART新增
        uint8_t	ConfigChangeCounter[2];//HART新增
	float	OutputM_Speed;
	float	InputM_Speed;
	float	RadarLength;			// length of guided radar		//to be save 					
}BasicPara__TypeDef;

typedef struct                     
{
	uint8_t LinePointsNum,ScalePointsNum;
	uint8_t	DLineUseorUnuse,RLineUseorUnuse;
	float LinearParaFB[20][2],ScaleParaFB[20][2];
}LinearPara__TypeDef;

typedef struct
{	
	uint8_t	MeterStatus;
	uint8_t PDate[DATE_LENGTH];
	uint8_t PSN[SENSOR_SN_LENGTH];
	uint8_t PDescriptor[DESCRIPTOR_LENGTH];
	uint8_t	PTag[TAG_LENGTH];
//Mr.liu modified！！！
        uint8_t	PLongTag[LONG_TAG_LENGTH];//HART新增
	uint8_t	FinalAsmNo[FINAL_ASM_NO_LENGTH];
//Mr.liu modified！！！
        uint8_t PMessage[MESSAGE_LENGTH];//HART新增
	uint8_t UserPassword[USERPASSWORD_LENGTH];
	uint8_t Language;
}SystemInformationP__TypeDef;

typedef	struct
{
	uint16_t	Eeprom_InitFlagI;
	uint8_t	Eeprom_EnablePara_Protect;
	SystemInformationP__TypeDef	SystemInformationP;
	BasicPara__TypeDef		BasicPara;
	ComPara__TypeDef		ComPara;
	DistCoefficientPara__TypeDef	DistCoefficient;
	RangePara__TypeDef		RangeValue;
	RangePara__TypeDef		RangeLimit;
	PWMCurrentPara__TypeDef		PWMCurrentPara;
	WaveLostPara__TypeDef		WaveLostPara;
	JumpPara__TypeDef		JumpPara;
	EchoAdjudgePara__TypeDef	EchoAdjudgePara;
	MappingPara__TypeDef		MappingPara;
	LinearPara__TypeDef		LinearPara;
	ExpandPara__TypeDef		ExpandPara;
	uint16_t	EepromPara_CRC;
}EepromParaS__TypeDef;

typedef	union
{
	EepromParaS__TypeDef	EepromParaS;
	uint8_t EepromParaC[sizeof(EepromParaS)];
}EepromParaU__TypeDef;

typedef	union
{
	float	FloatUnion;
	uint8_t CharUnion[4];
}FloatCharU__TypeDef;

//#pragma pack(push)
//#pragma pack(1)
typedef	struct
{
	float ModfiyTime[2];
//	float ModfiyTimeH;
	uint8_t ModfiyComMode;
	uint16_t ModfiyParaAddress;
	FloatCharU__TypeDef BeforeModifyParaV;
	FloatCharU__TypeDef UpdateModifyParaV;
}RecordModifyPara__TypeDef;
//#pragma pack(pop)

typedef	union
{
	RecordModifyPara__TypeDef	RecordModifyParaS;
	uint8_t RecordModifyParaC[sizeof(RecordModifyParaS)];
}RecordModifyParaU__TypeDef;

void	GetPara_Float(uint8_t *TempDataBuffer,uint16_t ParaAddress);
uint8_t *GetPara_Char(uint16_t ParaAddress);
void	ReadParaL_LCD(uint16_t ParaAddress, float *ParaValueP);
void	SavePara_FloatType(uint8_t TempModifyComMode,uint16_t ParaAddress, float *ParaValueP);
void	SavePara_StringType(uint16_t ParaAddress, uint8_t *ParaValueP);
void	SavePara_CharType(uint8_t TempModifyComMode,uint16_t ParaAddress, uint8_t ParaValue);
void	SavePara_LineType(uint16_t ParaAddress, float *ParaValueP);
void	Save_Eeprom_EnablePara_Protect(uint8_t ParaValueP[13]);
void	SaveUserPara(void);
void	Set_PWM18mABias_Value(float ParaValue);
void	Set_PWM4mABias_Value(float ParaValue);
void	ModifyCurrent(uint8_t TempModifyComMode);
void	InitPara(void);
uint8_t	GetComMethod(void);
uint8_t	GetComAddress(void);
uint8_t	GetComAddress_485(void);
uint8_t	GetAllUserPara_HandleMethod(void);
void	SetAllUserPara_HandleMethod(uint8_t TempModifyComMode,uint8_t ParaValue);
void	SetReadWriteEepromStatus(uint8_t TempStatus);
void	CheckRamPara(void);
void	RecordPWMCurrentPara(void);
void	SavePara_PointsNum(uint8_t ParaValue);
void	SaveModifyParaInfo(void);
void	ReadModifyParaInfo(uint8_t *ModifyParaInfoEEprom,uint8_t ReadModifyParaInfo_Index);
void	GetModifyParaInfoP(float Fram_ModifyParaInfo);
void	PWM20_PWM18_Current(float ParaValue);
uint8_t	GetPara_PointsNum(void);

void	ReadEeprom(uint16_t Memory_Address,uint8_t *RxpData,uint16_t ByteNumber);
void	WriteEeprom(uint16_t Memory_Address,uint8_t *pData,uint16_t ByteNumber);

void	SetSaveFakeInfo(uint8_t TempFakeInfo_Status);
uint16_t GetRFWorkTimeMax(void);
float	GetVconMinVolage(void);
float	GetUnitConv_Ratio(void);
float	GetRFGainPara(void);

//Mr.liu modified！！！
extern uint8_t GetPara_CharMax(uint16_t ParaAddress);
extern uint8_t GetPara_CharMin(uint16_t ParaAddress);
extern float GetPara_FloatMin(uint16_t ParaAddress);
extern float GetPara_FloatMax(uint16_t ParaAddress);
float UnitParaRatioGet(uint8_t PVUnit);
#ifdef __cplusplus
}
#endif
#endif /*__SwitchHandle_H */
