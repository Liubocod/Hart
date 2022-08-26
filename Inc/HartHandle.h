#define	RXBUFFERSIZE			50

#define	LONGFRAME_M			0x82
#define	SHORTFRAME_M			0x02
#define	LONGFRAME_S			0x86
#define	SHORTFRAME_S			0x06
#define	LONGADDRESSNUMBER		5
#define	SHORTADDRESSNUMBER		1

#define CMD_ILLEGAL			0xFF

#define HART_RFRAME_FAIL		0
#define HART_RFRAME_TRUE		1

/*status code define*/
#define CommNormal 			0x0
#define DeviceNormal 			0x40

//命令0xae [0x2c(空高)、0x19(单位)、0x30(料高)、0xfa(1个字节)、0x29(1个字节)、0xba(2个字节)、0x02(2个字节)
/*manufacture define for hart device identification*/
#define MANUFACTURE_ID			0x11
#define DEVICE_TYPE			0x10
#define DEFAULT_PREAM			0x06
#define UNIV_CMD_REV			0x05
#define TRANS_SPEC_REV			0x03
#define SOFT_REV 			0x03
#define HARD_REV			0x08
#define FLAGS				0x00
#define DEVICE_ID			0xBB5577
#define DEVICE_ID0			0x04
#define DEVICE_ID1			0x9c
#define DEVICE_ID2			0x80

void	AnalyzeHartProtocol(void);
void	EnableHartTx(void);
void	EnableHartRx(void);
void	UART1_IRQHandler(void);
