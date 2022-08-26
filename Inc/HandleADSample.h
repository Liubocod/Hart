/**
 ******************************************************************************
  * File Name          : HandleADSample.h

*/
#ifndef __HandleADSample_H
#define __HandleADSample_H
#ifdef __cplusplus
 extern "C" {
#endif

   
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "main.h"

#define		AD_VREF						2.50
#define		AD_FULLADRESULT					4095
#define		AD_CHANNEL_NUMBERS				8
#define		CURRENT_CHANNEL_NUMBERS				1
#define		CHANNEL_NUMBERS					9

#define		CURRENT_CHANNEL_RES				10//ŷ
#define		VCON_CAP_ADSAMPLELENGTH				3*50
#define		MUL_ADSAMPLELENGTH				AD_CHANNEL_NUMBERS*35
#define		EXTERNALCURRENT_MAX				20
#define		EXTERNALCURRENT_MIN				4
#define		VCON_VIN_DV_VOLTAGE				3.5
#define		VCON_CAP_MAX_VOLTAGE				16
#define		VCON_CAP_MIN_VOLTAGE				11
#define		VCON_THRESHOLD_MIN_VOLTAGE			8
#define		INITPOWER_COUNT   				60
#define		VCON_CAP_GAIN					9.242//(680+82.5)/82.5
#define		VIN_GAIN					15.732//(825+56)/56
#define		VCC_33_GAIN					2

#define		SAMPLEPERIOD					300	//ms 350
#define		MULTISAMPLEPERIOD				8000	//ms
#define 	SAMPLEWAIT					2
#define 	SAMPLECOMPLETE					1
#define 	SAMPLESTART					0

#define		DISABLERECORD					0
#define		ENABLERECORD					1

#define 	VCON_CAP_SAMPLE					0
#define 	MULTICHANNEL_SAMPLE				1

#define		ECHO_SIGNAL_Pin					GPIO_PIN_0
#define 	ECHO_SIGNAL_GPIO_Port				GPIOC
#define 	VCON_CAP_Pin					GPIO_PIN_3
#define 	VCON_CAP_GPIO_Port				GPIOA
#define 	VIN_CPU_Pin					GPIO_PIN_4
#define 	VIN_CPU_GPIO_Port				GPIOA
#define 	VCC_25_Pin					GPIO_PIN_5
#define 	VCC_25_GPIO_Port				GPIOA
#define 	VCC33_PW_Pin					GPIO_PIN_2
#define 	VCC33_PW_GPIO_Port				GPIOC
#define 	VCC33_CPU_Pin					GPIO_PIN_1
#define 	VCC33_CPU_GPIO_Port				GPIOC
#define 	C4_20mAIn_Pin					GPIO_PIN_6
#define 	C4_20mAIn_GPIO_Port				GPIOA

#define		RF_POWER_12_Pin					GPIO_PIN_1
#define 	RF_POWER_12_GPIO_Port				GPIOB
#define 	RF_POWER_13_Pin					GPIO_PIN_5
#define 	RF_POWER_13_GPIO_Port				GPIOC
#define 	RF_POWER_18_Pin					GPIO_PIN_4
#define 	RF_POWER_18_GPIO_Port				GPIOC
#define 	RF_POWER_22_Pin					GPIO_PIN_0
#define 	RF_POWER_22_GPIO_Port				GPIOB

/* ADC internal channels related definitions */
/* Internal voltage reference VrefInt */
#define		VREFINT_CAL_ADDR				((uint16_t*) (0x1FFF75AAUL)) /* Internal voltage reference, address of parameter VREFINT_CAL: VrefInt ADC raw data acquired at temperature 30 DegC (tolerance: +-5 DegC), Vref+ = 3.0 V (tolerance: +-10 mV). */
#define		VREFINT_CAL_VREF				( 3000UL)                    /* Analog voltage reference (Vref+) value with which temperature sensor has been calibrated in production (tolerance: +-10 mV) (unit: mV). */
/* Temperature sensor */
#define		TEMPSENSOR_CAL1_ADDR				((uint16_t*) (0x1FFF75A8UL)) /* Internal temperature sensor, address of parameter TS_CAL1: On STM32L4, temperature sensor ADC raw data acquired at temperature  30 DegC (tolerance: +-5 DegC), Vref+ = 3.0 V (tolerance: +-10 mV). */
#define		TEMPSENSOR_CAL2_ADDR				((uint16_t*) (0x1FFF75CAUL)) /* Internal temperature sensor, address of parameter TS_CAL2: On STM32L4, temperature sensor ADC raw data acquired at temperature defined by TEMPSENSOR_CAL2_TEMP (tolerance: +-5 DegC), Vref+ = 3.0 V (tolerance: +-10 mV). */
#define		TEMPSENSOR_CAL1_TEMP 				(30L)           /* Internal temperature sensor, temperature at which temperature sensor has been calibrated in production for data into TEMPSENSOR_CAL1_ADDR (tolerance: +-5 DegC) (unit: DegC). */
#define		TEMPSENSOR_CAL2_TEMP				(130L)                       /* Internal temperature sensor, temperature at which temperature sensor has been calibrated in production for data into TEMPSENSOR_CAL2_ADDR (tolerance: +-5 DegC) (unit: DegC). */
#define		TEMPSENSOR_CAL_VREFANALOG			(3000UL)                     /* Analog voltage reference (Vref+) voltage with which temperature sensor has been calibrated in production (+-10 mV) (unit: mV). */
#define		VREFANALOG_VOLTAGE				(2500UL)                     /* Analog voltage reference (Vref+) voltage with which temperature sensor has been calibrated in production (+-10 mV) (unit: mV). */

#define		CAL_AVG_SLOPE					TEMPSENSOR_CAL_VREFANALOG*((int32_t)*TEMPSENSOR_CAL2_ADDR - (int32_t)*TEMPSENSOR_CAL1_ADDR)/4095                    /* Analog voltage reference (Vref+) voltage with which temperature sensor has been calibrated in production (+-10 mV) (unit: mV). */
#define		TS_CAL1_V					(TEMPSENSOR_CAL_VREFANALOG*(int32_t)*TEMPSENSOR_CAL1_ADDR)/4095/* Analog voltage reference (Vref+) voltage with which temperature sensor has been calibrated in production (+-10 mV) (unit: mV). */

#define		IWR1443_MINVOLTAGE				0.8
typedef union
{
	uint32_t ADSampleBuffer_32BIT[MUL_ADSAMPLELENGTH/2];
	uint16_t ADSampleBuffer_16BIT[MUL_ADSAMPLELENGTH];
}MultiChannel_ADSampleBuffer__TypeDef;

typedef union
{
	uint32_t ADSampleBuffer_32BIT[VCON_CAP_ADSAMPLELENGTH/2];
	uint16_t ADSampleBuffer_16BIT[VCON_CAP_ADSAMPLELENGTH];
}VCON_CAP_ADSampleBuffer__TypeDef;

typedef	struct
{
//	float	MeasureValure_Max;
//	float	MeasureValure_Min;
	float	MeasureValure_Current;
}RecordMeasureValure__TypeDef;

void	ADC2_GPIO_Configuration(void);
void	RecordMeasureValureMax_Min(void);
void	InitADSamplePara(void);
void	HandleADSample(void);
void	SetSampleStatus_Complete(void);
void	MultiChannel_StartSample(void);

#ifdef __cplusplus
}
#endif
#endif /*__SwitchHandle_H */
