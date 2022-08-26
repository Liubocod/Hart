#include	"Eeprom.h"
/*------------------------- External variables --------------------------------------------------------*/
extern EepromParaU__TypeDef	EepromPara;
/*-----------------------------------------------------------------------------------------------------*/
const	uint16_t Char_Para_Table[CHARPARANUMBERS][5] = 
{/*参数相对地址、默认值、最大值、最小值、字节个数*/
(&EepromPara.EepromParaS.EchoAdjudgePara.FakeWaveGain-EepromPara.EepromParaC),15,99,0,1,//0
(&EepromPara.EepromParaS.BasicPara.DampingTime-EepromPara.EepromParaC),DAMPINGTIME_DEFAULT,99,0,1,//1
(&EepromPara.EepromParaS.BasicPara.DielectricConstant-EepromPara.EepromParaC),2,56,0,1,//2
(&EepromPara.EepromParaS.PWMCurrentPara.CurrentDirection-EepromPara.EepromParaC),0,1,0,1,//3
(&EepromPara.EepromParaS.SystemInformationP.Language-EepromPara.EepromParaC),0,2,0,1,//4
(&EepromPara.EepromParaS.BasicPara.PVUnit-EepromPara.EepromParaC),1,6,0,1,//5
(&EepromPara.EepromParaS.SystemInformationP.UserPassword[0]-EepromPara.EepromParaC),0x30,250,0,USERPASSWORD_LENGTH,//6
(&EepromPara.EepromParaS.ComPara.ComAddress-EepromPara.EepromParaC),0,99,0,1,//7
(&EepromPara.EepromParaS.ComPara.BaudRate-EepromPara.EepromParaC),0,8,0,1,//8
(&EepromPara.EepromParaS.WaveLostPara.WaveLostProccessSEL-EepromPara.EepromParaC),WAVELOST_SEL_REMAIN,3,0,1,//9
(&EepromPara.EepromParaS.JumpPara.JumpSEL-EepromPara.EepromParaC),JUMP_SEL_DIRECT,4,0,1,//10
(&EepromPara.EepromParaS.LinearPara.DLineUseorUnuse-EepromPara.EepromParaC),UNUSE,1,0,1,//11
(&EepromPara.EepromParaS.LinearPara.RLineUseorUnuse-EepromPara.EepromParaC),UNUSE,1,0,1,//12
(&EepromPara.EepromParaS.PWMCurrentPara.CurrentAlarmSEL-EepromPara.EepromParaC),2,4,0,1,//13
(&EepromPara.EepromParaS.PWMCurrentPara.CurrentStartSEL-EepromPara.EepromParaC),0,4,0,1,//14
(&EepromPara.EepromParaS.EchoAdjudgePara.Threshold-EepromPara.EepromParaC),15,250,0,1,//15
(&EepromPara.EepromParaS.EchoAdjudgePara.EchoAdjudgeSEL-EepromPara.EepromParaC),ECHOADJUDGE_SEL_MAX,4,0,1,//16

(&EepromPara.EepromParaS.ComPara.LoopCurrentMode-EepromPara.EepromParaC),0,4,0,1,//17
(&EepromPara.EepromParaS.SystemInformationP.PDate[0]-EepromPara.EepromParaC),0x30,250,0,DATE_LENGTH,//18
(&EepromPara.EepromParaS.SystemInformationP.PSN[0]-EepromPara.EepromParaC),0,250,0,SENSOR_SN_LENGTH,//19
(&EepromPara.EepromParaS.SystemInformationP.PDescriptor[0]-EepromPara.EepromParaC),0,250,0,DESCRIPTOR_LENGTH,//20
(&EepromPara.EepromParaS.SystemInformationP.PTag[0]-EepromPara.EepromParaC),0x30,250,0,TAG_LENGTH,//21
(&EepromPara.EepromParaS.SystemInformationP.FinalAsmNo[0]-EepromPara.EepromParaC),0x30,250,0,FINAL_ASM_NO_LENGTH,//22
(&EepromPara.EepromParaS.ComPara.Enable_LCD485-EepromPara.EepromParaC),0,1,0,1,//23
(&EepromPara.EepromParaS.ComPara.EnableHart -EepromPara.EepromParaC),0,1,0,1,//24
(&EepromPara.EepromParaS.Eeprom_EnablePara_Protect-EepromPara.EepromParaC),0xA5,0xA5,0x5A,1,//25
(&EepromPara.EepromParaS.ComPara.ComAddress_485-EepromPara.EepromParaC),0,200,0,1,//26
(&EepromPara.EepromParaS.ComPara.Parity-EepromPara.EepromParaC),0,3,0,1,//27
(&EepromPara.EepromParaS.ComPara.StopBits-EepromPara.EepromParaC),0,3,0,1,//28
(&EepromPara.EepromParaS.ComPara.WordLength-EepromPara.EepromParaC),0,3,0,1,//29
(&EepromPara.EepromParaS.SystemInformationP.PLongTag[0]-EepromPara.EepromParaC),0,255,0,LONG_TAG_LENGTH,
(&EepromPara.EepromParaS.BasicPara.ConfigChangeFlag-EepromPara.EepromParaC),1,3,0,1,
(&EepromPara.EepromParaS.BasicPara.ConfigChangeCounter[0]-EepromPara.EepromParaC),1,255,0,2,
(&EepromPara.EepromParaS.SystemInformationP.PMessage[0]-EepromPara.EepromParaC),0,255,0,MESSAGE_LENGTH,//HART新增参数

((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[0]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[1]-EepromPara.EepromParaC),10,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[2]-EepromPara.EepromParaC),1,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[3]-EepromPara.EepromParaC),42,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[4]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[5]-EepromPara.EepromParaC),70,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[6]-EepromPara.EepromParaC),12,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[7]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[8]-EepromPara.EepromParaC),81,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[9]-EepromPara.EepromParaC),33,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[10]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[11]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[12]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[13]-EepromPara.EepromParaC),1,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[14]-EepromPara.EepromParaC),3,36,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[15]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[16]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[17]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[18]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[19]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[20]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[21]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[22]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[23]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[24]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[25]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[26]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[27]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[28]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[29]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[30]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[31]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[32]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[33]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[34]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[35]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[36]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[37]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[38]-EepromPara.EepromParaC),0,99,0,1,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.CharPara[39]-EepromPara.EepromParaC),0,99,0,1,
};

const	float Float_Para_Table[FLOATPARANUMBERS][5] = 
{/*参数相对地址、默认值、最大值、最小值*/
((uint8_t volatile*)&EepromPara.EepromParaS.RangeValue.LowerRange-EepromPara.EepromParaC),30,128,0.2,4,/*量程低位*/
((uint8_t volatile*)&EepromPara.EepromParaS.RangeValue.UpperRange-EepromPara.EepromParaC),0,128,-20,4,/*量程高位*/
((uint8_t volatile*)&EepromPara.EepromParaS.BasicPara.RadarLength-EepromPara.EepromParaC),0,35,0,4,/*缆长*/
((uint8_t volatile*)&EepromPara.EepromParaS.BasicPara.InputM_Speed-EepromPara.EepromParaC),0.5,100,0,4,/*进料速度*/
((uint8_t volatile*)&EepromPara.EepromParaS.DistCoefficient.SystemZeroDist-EepromPara.EepromParaC),0,20,-20,4,/*零点*/
((uint8_t volatile*)&EepromPara.EepromParaS.DistCoefficient.SystemProportion-EepromPara.EepromParaC),1,10,0.5,4,/*比例*/
((uint8_t volatile*)&EepromPara.EepromParaS.RangeLimit.UpperRange-EepromPara.EepromParaC),0,128,0,4,/*高位盲区*/
((uint8_t volatile*)&EepromPara.EepromParaS.RangeLimit.LowerRange-EepromPara.EepromParaC),30.5,128,0,4,/*位盲区*/
((uint8_t volatile*)&EepromPara.EepromParaS.BasicPara.OutputM_Speed-EepromPara.EepromParaC),0.2,100,0,4,/*出料速度*/
((uint8_t volatile*)&EepromPara.EepromParaS.EchoAdjudgePara.CurrentEchoA-EepromPara.EepromParaC),0.03,3,0,4,/*当前回波优势*/
((uint8_t volatile*)&EepromPara.EepromParaS.PWMCurrentPara.PWM4mA-EepromPara.EepromParaC),63866,65535,35000,4,
((uint8_t volatile*)&EepromPara.EepromParaS.PWMCurrentPara.PWM18mA-EepromPara.EepromParaC),21000,35000,10000,4,
((uint8_t volatile*)&EepromPara.EepromParaS.MappingPara.MappingUpper-EepromPara.EepromParaC),0,100,0,4,/*高位映射*/
((uint8_t volatile*)&EepromPara.EepromParaS.MappingPara.MappingLow-EepromPara.EepromParaC),0,100,0,4,/*低位映射*/
((uint8_t volatile*)&EepromPara.EepromParaS.WaveLostPara.WaveLostSetPosition-EepromPara.EepromParaC),0,100,0,4,/*指定距离*/
((uint8_t volatile*)&EepromPara.EepromParaS.JumpPara.JumpDist-EepromPara.EepromParaC),0.5,100,0.01,4,/*跳变距离*/
((uint8_t volatile*)&EepromPara.EepromParaS.JumpPara.JumpWaitTime-EepromPara.EepromParaC),60,9000,0,4,/*跳变时间*/
((uint8_t volatile*)&EepromPara.EepromParaS.JumpPara.JumpChangeSpeed-EepromPara.EepromParaC),0.05,100,0,4,/*跳变速度*/
((uint8_t volatile*)(&EepromPara.EepromParaS.EchoAdjudgePara.FrontEchoA)-EepromPara.EepromParaC),0.03,3,0,4,/*前波优势*/
((uint8_t volatile*)&EepromPara.EepromParaS.EchoAdjudgePara.ConfirmEchoTime-EepromPara.EepromParaC),10,250,1,4,/*确认时间*/
((uint8_t volatile*)&EepromPara.EepromParaS.EchoAdjudgePara.MinDistinctiveness-EepromPara.EepromParaC),0.13,2,0,4,/*最小分辨率*/

((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[0]-EepromPara.EepromParaC),6000,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[1]-EepromPara.EepromParaC),1.0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[2]-EepromPara.EepromParaC),1.0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[3]-EepromPara.EepromParaC),1.0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[4]-EepromPara.EepromParaC),0.5,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[5]-EepromPara.EepromParaC),7.6,12,7,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[6]-EepromPara.EepromParaC),27.69,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[7]-EepromPara.EepromParaC),77,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[8]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[9]-EepromPara.EepromParaC),0.5,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[10]-EepromPara.EepromParaC),0.3,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[11]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[12]-EepromPara.EepromParaC),0.2,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[13]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[14]-EepromPara.EepromParaC),100,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[15]-EepromPara.EepromParaC),220,8600,160,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[16]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[17]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[18]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[19]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[20]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[21]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[22]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[23]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[24]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[25]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[26]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[27]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[28]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[29]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[30]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[31]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[32]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[33]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[34]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[35]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[36]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[37]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[38]-EepromPara.EepromParaC),0,99999,-9999,4,
((uint8_t volatile*)&EepromPara.EepromParaS.ExpandPara.FloatPara[39]-EepromPara.EepromParaC),0,99999,-9999,4,
};
