/**
 ******************************************************************************
  * File Name          : EchoHandle.h

*/
#ifndef __EchoHandle_H
#define __EchoHandle_H
#ifdef __cplusplus
 extern "C" {
#endif

#define	MAXADVALUE				4095
#define	REFVALUE				2.5

#define	CALDISTANCEECHOWIDTH			2

#define	ONE_PONIT_M_45				0.04454
#define	ONE_PONIT_M_90				0.0878
#define	ONE_PONIT_M_180				0.0878//0.1756
#define FALLING_EDGE				0
#define RISING_EDGE				1
#define INIT_EDGE				2

#define NOJUMP					0
#define ECHOCONFIRM				1
#define DISTANCECONFIRM				2
#define JUMPEND					3

#define WAVE_NUM_MAX				50

#define BEST_WAVE_NOT_FIND			0xFF

#define	ECHOMINWIDTH_26G			4
#define	ECHOMINVALUE_26G			0x4000

#define	MAXVAILDECHONUMBERS			6

#define DISTANCEERROR				-1

#define MAX_NUMBERS_IWR1443PARA			26
   
#define	AMPLITUDE_COV_PARA_RF			77.2*129//该系数是用户设定的电压值转换成跟1443的值相对应
#define	AMPLITUDE_COV_PARA_LCD			77.3//(255/3.3)
#define	AMPLITUDE_COV_SAMPLE_255		129//该系数是用户设定的电压值转换成跟1443的值相对应
   
#define	EMPFYCAN_AREA				1
#define	UNEMPFYCAN_AREA				0

#define	MAXRECORDJUMPCOUNT			5
#define MAXFLITER_COUNT				12
#define FFT_NUMBER				8192

#define FAKE_CHANGE				1
#define FAKE_NO_CHANGE				0
extern uint8_t LcdEcho[LCDCURVE_LENGH];

#pragma pack(push)
#pragma pack(1)
typedef struct params_recv_t{
	uint8_t code_algo;
	uint8_t fft_window_enable:1;
	uint8_t code_window:7;	
	uint16_t code_range:4;
	uint16_t adc_data_scale:3;
	uint16_t fft_out_scale:3;
	uint16_t Para_BackUp:5;
	uint16_t log_enable:1;
	float cur_pos_start;// fake echo pos x1
	float cur_pos_end;// fake echo pos x2
	float display_start;
	float display_end;
	uint16_t display_points;// fake echo add
	uint16_t echo_bias_correct_enable:1;
	uint16_t echo_bias_correct_code:6;
	uint16_t fake_echo_filter_len:5;
	uint16_t write_calib_params_to_flash:1;
	uint16_t IWR_AWR_SW:3;
	float detect_start;
	float detect_end;
	uint16_t detect_seral_no;// fake echo filter width (defualt: 1)
	uint16_t baud_rate:4;
	uint16_t custom_calib_mode:1;
	uint16_t rx_gain:6;
	uint16_t hpf1:2;
	uint16_t hpf2:2;
	uint16_t fft_log:1;
	uint8_t RF_Rx:4;
	uint8_t RF_Tx:4;
	uint8_t num_multi_chirp;
	uint16_t period_frame;
	float start_freq;
	float slope;
	uint32_t dbg_level:4;
	uint32_t nAvg:4;
//--------------------------------------
	uint32_t dump_adc_only:1; //只上传ADC数据 暂时未用
	uint32_t dump_adc:1; //上传ADC数据使能
	uint32_t dump_dft:1; //上传FFT数据使能
	uint32_t dump_custom:1; //上传自定义数据使能 暂时未用
	uint32_t ldoBypassEnable:1; //ldoBypass使能 暂时不看
	uint32_t dft_mag:1; //FFT数据取模使能
	uint32_t reserved2:18; //预留
//--------------------------------------
	float slopeCorrection; //斜率校正参数
	float diatanceExclution; //波峰位置距离排斥参数
	uint16_t CrcResult;
}IWR1443_Params__TypeDef;
#pragma pack(pop)

typedef struct
{
	uint16_t start;
	uint16_t end;
	uint16_t max_value_point;
	uint16_t max_value;
	uint16_t min_value;
	uint16_t weight;
	uint16_t width;
	uint16_t area;
	uint16_t height;  				// compare with the start and end and fake wave curve.
	uint16_t Distinctiveness;
	unsigned char first;  				//if the wave is first one that compare with other wave, fist =1, elese first =0;
	unsigned char qualify; 				// if it bigger then fake wave, and wide enough, and so on.
	unsigned char checked;				// if it checked by qualify condition;
	unsigned char wave_type;  			// 0 smal  1 
//	uint16_t up_edge_length;
//	uint16_t down_edge_length;
//	uint16_t flat_edge_length;
}Echo_para__TypeDef;

typedef struct
{
	float DistanceV;
	uint16_t AmplitudeV;
	uint8_t EchoFeatures;
	uint8_t EchoDistinctiveness;
//	float CurrentTime[2];
}VailEchoInformation__TypeDef;

typedef struct
{
	uint8_t JumpEchoCurve[100];
	float CurrentTime[2];
	VailEchoInformation__TypeDef JumpEchoInfo[5];
}JumpEchoInformation__TypeDef;

typedef	union
{
	JumpEchoInformation__TypeDef JumpEchoInfoStruct[MAXRECORDJUMPCOUNT*2];
	uint8_t JumpEchoInfoC[sizeof(JumpEchoInfoStruct)];
}JumpEchoInformationUnion__TypeDef;

/*
typedef	struct
{
	uint8_t code_range;
	uint8_t code_algo;
	uint8_t adc_data_scale;
	uint8_t fft_out_scale;
	float cur_pos_start;
	float cur_pos_end;
	float display_start;
	float display_end;
	uint16_t display_points;
	uint8_t baud_rate;
	uint8_t custom_calib_mode;
	uint16_t CrcResult;
}IWR1443_Params__TypeDef;
*/
typedef	union
{
	IWR1443_Params__TypeDef IWR1443_Params;
	uint8_t IWR1443_ParamsC[sizeof(IWR1443_Params)];
}IWR1443_Params_Trans__TypeDef;

typedef	struct
{
	uint8_t VaildEchoNumber;
	uint16_t AmplitudeV[5];
	float DistanceV[5];
}IWR1443_ChipInf__TypeDef;

typedef	struct
{
	uint8_t EchoDistinctiveness[5];
	uint16_t AmplitudeV[5];
	float DistanceV[5];
}IWR1443_EchoInf__TypeDef;

void	Init_26GEchoJudgeData(void);
void	EchoDataHandle(int16_t *EchoData);
void	ClearLowSampleBuffer(void);
void	ParaConvert(void);
void	SetCalDistanceFlag(void);
float	CalOneSecondPV(void);
//void	ReadCurveData(uint8_t TempCurve[LCDCURVE_LENGH]);
void	UpdateLCDCurveData(void);
uint8_t	GetMarkBestWavePostion(void);
uint8_t	GetBestEchoIndex(void);
uint16_t GetBestEchoDistinctiveness(void);
uint16_t GetBestEchoAmplitude(void);
void	SetCurveStart_EndPoint(float TempLcdStart,float TempLcdEnd);
void	StartUartTransmit(void);
void	UpdateConfigIWR1443Para(void);
void	ChipInfHandle(uint8_t *ChipDataInf);
void	EchoData_1443(uint16_t *LCDEchoData);
void	HandleShockWave(uint16_t *LCDEchoData);
void	ClearIWR1443Data(void);
void	Set_IWR1443_CalibMode(uint16_t TempCalibMode);
void	Get_MulEcho_Information(uint8_t TempEchoNumber,uint8_t *TempEchoInfo);
void	VailEchoConfirm_IncTime(void);

void	ReadJumpCurveInfo(uint8_t *JumpCuveInfo,uint8_t JumpCuveInfo_Index);
void	ClearJumpCurveInfo(void);
void	SetJumpEchoInfoP(float Fram_JumpEcho_Index);
uint16_t GetDataSize_1443(void);
uint8_t	GetShockWaveInf(void);

void	CreatFakeWavePointIndex(void);
void	ClearHighNoiseContiuing_Count(void);
void	SendFakeCurveInfo(uint8_t *TempCurveInfo);
void	CreatFakeCurve(float *TempF);
void	Handle_NoiseCal_EchoSW(void);
void	TransmitCurveInfo(uint8_t *TempDataBuffer,uint8_t TempStartDot,uint8_t TempNumber,uint8_t CurveSL);
void	ReadJumpCurveInfo_485(uint8_t *JumpCuveInfo,uint16_t JumpCuveInfo_StartA,uint16_t JumpCuveInfo_ByteNum);
void	SetLcdLCDCurveEnd(float TempLCDCurveEnd);
void	SendReadFakeCurveInfo(uint8_t *TempCurveInfo);
void	Sample_Data_Process(int16_t *EchoData);

#ifdef __cplusplus
}
#endif
#endif /*__SwitchHandle_H */
