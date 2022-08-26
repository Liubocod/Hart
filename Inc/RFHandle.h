#define	ECHO_IS_UNUSED				1
#define	ECHO_IS_USED				0

#define	STARTSAMPLE				0
#define	ECHOSAMPLEWAIT				8

#define FREQ_CONTROL				0
#define FREQ_START				1
#define LOW_TIME				200
#define ERRORUPLIMIT				6000

#define	FREQ_CON_S_SET				HAL_GPIO_WritePin(RF_FREQ_CON_S_GPIO_Port, RF_FREQ_CON_S_Pin, GPIO_PIN_SET);
#define	FREQ_CON_S_RESET			HAL_GPIO_WritePin(RF_FREQ_CON_S_GPIO_Port, RF_FREQ_CON_S_Pin, GPIO_PIN_RESET);

#define RFSWITCHOFF				FREQ_CON_S_SET;FREQ_CON_RESET;
#define RFTURNHIGH				FREQ_CON_S_RESET;FREQ_CON_RESET
#define RFTURNLOW				FREQ_CON_SET;FREQ_CON_S_SET

#define DizzyYU_P1	1
#define DizzyYU_P2	2
#define DizzyYU_P3	3
#define DizzyYU_P4	4
#define DizzyYU_P5	5
#define DizzyYU_Center	16
#define DizzyYU_N1	11
#define DizzyYU_N2	12
#define DizzyYU_N3	13
#define DizzyYU_N4	14
#define DizzyYU_N5	15

typedef union
{
	uint32_t LowSampleBuffer_32BIT[SAMPLE_DATA_LENGTH/2];
	uint16_t LowSampleBuffer_16BIT[SAMPLE_DATA_LENGTH];
}LowSampleBuffer__TypeDef;

void	PeriodProcess(void);
void	GetRFCurrentPeriod(uint32_t TempPeriod);
void	SampleEchoFlow(void);
void	ContralSampleRadio(void);
uint8_t	GetADSampleCount(void);
void	ResetADSampleCount(void);
