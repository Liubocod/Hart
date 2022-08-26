#define LCD_ADDRESS						0x8E    /* LCD Address  */
#define	LCDBUFFERSIZE						30
/* (Rise time = 0ns, Fall time = 0ns) */
#define	LCD_TIMING						0xA00077B0//0xF010E5FE//

#define	UPDATELCDTIME						200//mS

#define	TRANSMITNUMBER_PERONCE					20

#define	READLCD							0
#define WRITELCD						1

#define	LCDREADDATA						0
#define LCDWRITEDATA						4

#define	LCDBUFFERMAXLENGH					100

#define LCDCURVE_START_SET					2
#define LCDCURVE_END_SET					250

#define LCD_CHARPARA_STARTADDRESS_IIC				10    /* LCD Address  */
#define LCD_FLOATPARA_STARTADDRESS_IIC				34    /* LCD Address  */

#define EXPANDCHARPARA_STARTADDRESS_IIC				90
#define EXPANDCHARPARA_ENDADDRESS_IIC				100
#define EXPANDCHARPARA2_STARTADDRESS_IIC			103
#define EXPANDCHARPARA2_ENDADDRESS_IIC				109
#define EXPANDCHARPARA3_STARTADDRESS_IIC			109
#define EXPANDCHARPARA3_ENDADDRESS_IIC				130

#define EXPANDFLOATPARA_STARTADDRESS_IIC			151
#define EXPANDFLOATPARA_ENDADDRESS_IIC				191
#define EXPANDFLOATPARA1_STARTADDRESS_IIC			170
#define EXPANDFLOATPARA1_ENDADDRESS_IIC				190

#define IIC_LCD_DATABUFF_ADDRESS				0    /*   */
#define IIC_LCD_DATABUFF_PARANUMBER				1    /*   */
#define IIC_LCD_DATABUFF_CMD					2    /*   */
#define IIC_LCD_DATABUFF_BYTENUMBER				3    /*  */

#define PERFVIEW_FUNCTION_INDEX             			200   /* ��ѯ���ܱ������ܱ�� */
#define PERFAPPOINTCLEAR_FUNCTION_INDEX     			201   /* ���ܱ���ָ�����㹦�ܱ�� */
#define PERFALLCLEAR_FUNCTION_INDEX         			202   /* ���ܱ���ȫ�����㹦�ܱ�� */

#define	EVENTSETUP_FUNCTION_WORD_LABEL				203
#define	EVENT_DEMAND_FUNCTION_WORD_LABEL			204
#define	EVENTNUM_DEMAND_FUNCTION_WORD_LABEL			205
#define	TIMESN_DEMAND_FUNCTION_WORD_LABEL			206
#define	EVENTCLEAR_FUNCTION_WORD_LABEL				207

#define	READ_MULECHO_INFORMATION_INDEX				208

#define	READ_UNIQUECODE_FUNCTION_INDEX           		210   // ��ȡΨһ�빦�ܱ��
#define	RETURN_CALCULATIONCODE_FUNCTION_INDEX			211   // ���ؼ����빦�ܱ��
#define	ACTIVATECODE_GENERATE_FUNCTION_INDEX			212   // ���������ɹ��ܱ�ţ��ù���Ϊ�����������Ǳ�ʱ���ɼ�����ʱ��ͨѶ
#define	ACTIVATECODE_STATUS_FUNCTION_INDEX			213   // ������״̬���ܱ�ţ��ù���Ϊ�Ǳ�ƽ������ʱ��������ʱ��ѯ��������ȷ���

#define	READ_PARAMODIFY_INFORMATION_INDEX			218
#define	READ_JUMPCHO_INFORMATION_INDEX				219
#define	CLEAR_JUMPCURVE_INDEX					220
#define	READ_AUXIFO_INDEX					221

#define   DATA_BYTE_NUM_45            				45
#define   DATA_BYTE_NUM_29           			        29
#define   DATA_BYTE_NUM_21           			        21
#define   DATA_BYTE_NUM_19           			        19
#define   DATA_BYTE_NUM_15            				15
#define   DATA_BYTE_NUM_13					13
#define   DATA_BYTE_NUM_12					12
#define   DATA_BYTE_NUM_11					11
#define   DATA_BYTE_NUM_9					9
#define   DATA_BYTE_NUM_7					7
#define   DATA_BYTE_NUM_6					6

#define   PACKAGE_HEAD						0xAA
#define   PACKAGE_TAIL						0xCC

typedef	union
{
	uint8_t	LCDBufferC[LCDBUFFERMAXLENGH];
	uint16_t LCDBufferI[LCDBUFFERMAXLENGH/2];
	float LCDBufferF[LCDBUFFERMAXLENGH/4];
}LcdDataBuffer__TypeDef;

void	InitLcdData(void);
void	LcdHandle(void);
void	IncUpdateLCDCount(void);
//void	UpdateLCDCurveData(void);
void	SartHandleLcdR(void);
