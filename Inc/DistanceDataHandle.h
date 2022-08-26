#define	INNERPROPORTION_26G					1
#define	INNERZERO_26G						0

#define	INNERPROPORTION_6G					1
#define	INNERZERO_6G						0

#define	BEST_WAVE_NOT_FIND					0xFF
#define	CONFIRMJUMP						2

#define MEANING_TIANBIAO_SEL_DIRECT	 			0
#define MEANING_TIANBIAO_SEL_WAITTIME				1
#define MEANING_TIANBIAO_SEL_JUMPSPEED				2
#define MEANING_TIANBIAO_SEL_QUSHIGENZONG			3

#define MEANING_WaveLostProccessSel_BAOCHI	 		0
#define MEANING_WaveLostProccessSel_SHEDING			1
#define MEANING_WaveLostProccessSel_QUSHIYIDONG			2		
#define MEANING_WaveLostProccessSel_ALARM			3

#define MEANING_XIANXING_JULI_BUYONG				1
#define MEANING_XIANXING_JULI_YONG	 			0
#define MEANING_XIANXING_BILI_BUYONG				1
#define MEANING_XIANXING_BILI_YONG	 			0

#define	NOSINGNAL_MAXTIME					60//s

#define	PVEVEN_NUMBER_MAX					100

#define	REVENGEOUTTIME						20
#define	REVENGEPERIOD						1820

#define	METEROUTSTATUS_A					0
#define	METEROUTSTATUS_B					1
#define	METEROUTSTATUS_C					2
#define	METEROUTSTATUS_D					3
#define	METEROUTSTATUS_E					4
#define	METEROUTSTATUS_F					5

extern float OnePoint_M_Para;
typedef	struct
{
	float	MasterVariablePV;
	float	MasterVariablePercent;	
	float	MasterVariablePVCurrent;	
	float	MasterVariableSetCurrent;	
}MasterVariable__TypeDef;

void	ClearNoSampleSignal(void);
void	UpdateCurrentPWM(void);
void	Init26GInnerDistanceCoefficient(void);
void	Init6GInnerDistanceCoefficient(void);
void	DistanceEchoProccess(void);
void	ReadMMasterVariable(float *ParaValueP);
void	ReadMMasterVariablePVCurrent(float *ParaValueP);
void	SetOutCurrentVal(float CurrentVal);
uint8_t ReadMeterWorkStatus(void);
uint16_t DistanceOutToPointIndexInt(float DistanceOut);
float	GetPreOneSecondEvenValure(void);
float	GetMasterVariablePV(void);
uint16_t GetWaitCounter(void);
float	HartReadMasterVariable(uint8_t Index);
float	ReadCurrentVariable(void);
void	CurrentFloat(float current);
float	DistanceCalibration(float InputDistanceValure);
void	SetUpdateCode_Current(void);
void	SetReBoot_Current(void);
void	RunActivationSystem(void);
void	QuickOutDistance(float TempDistance);
void	SetOneSecondEvenValure(float TempEvenValure);
void	SetMeterOutStatus(uint8_t CurrentStatus);
float   SetCurrentGet();
uint8_t	GetMeterOutStatus(void);

//Mr. Liu's code .
extern uint8_t WorkStatus;