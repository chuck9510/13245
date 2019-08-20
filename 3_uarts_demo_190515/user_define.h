
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/uart.h"
#include "utils/ringbuf.h"
#include "config.h"
#include "priorities.h"
#include "serial.h"
#include "telnet.h"
#include "FreeRTOSConfig.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "inc/hw_nvic.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "utils/swupdate.h"

#include "modbusTCP.h"


#define ETHPHY_LED_CFG (uint16_t)0x0510

#define modbus_enable 1


#define origin_code 0x00
#define revise_code 0x01
#define WiFi_Port    (uint32_t)100
#define COM2_Port    (uint32_t)2
#define COM1_Port    (uint32_t)1
#define COM_Port    (uint32_t)0
#define ResponseFromWiFi_size 1200
#define wifi_scan_qty 15

//#define frame_len_continous 40
//#define Delay_mS_for_230400 2
//#define Delay_mS_for_230400 (frame_len_continous*12*1000/230400)
//#define Delay_mS_for_9600 50
//#define Delay_mS_for_9600 50
extern uint8_t SSID_absent_qty;
extern uint8_t ResponseFromWiFi[ResponseFromWiFi_size];
extern uint32_t ReceiveWiFiResponse(void);
extern uint8_t ResponseFromWiFi_IP[110];
extern uint8_t ResponseFromWiFi_Mac[40];
extern uint8_t ResponseFromWiFi_SCAN[ResponseFromWiFi_size];//800
extern void WiFiStatus_LED(void);
//*****************************************************************************
//
// These defines are used to describe the device locator protocol.
//
//*****************************************************************************
#define TAG_CMD                 0xff
#define TAG_STATUS              0xfe
//#define CMD_DISCOVER_TARGET     0x02
#define CMD_DISCOVER_TARGET     0x01
//#define UDP_CMD_SCAN_DEVICE   			0x01
#define UDP_CMD_RST_DEVICE     			0x02
#define UDP_CMD_READ_DEVICE_CONFIG      0x03
#define UDP_CMD_SAVE_DEVICE_CONFIG      0x04
#define UDP_CMD_BASIC_DEVICE_CONFIG     0x05
#define UDP_CMD_UR_PORT0_CONFIG   		0x06
#define UDP_CMD_UR_PORT1_CONFIG  		0x07
#define UDP_CMD_UR_PORT2_CONFIG    		0x08
#define UDP_CMD_READ_TEMP_CONFIG    	0x0a
#define UDP_CMD_LOAD_DEFAULT_CONFIG    	0x0b
//new added command for TI Master;
#define UDP_CMD_CTL_Master_Slave    	0x80

#define CMD_Read_Slave_Quantity    		0x40
#define CMD_Read_GPIO    				0x41
#define CMD_Write_GPIO    				0x42
#define CMD_Ctrl_Buzzer    				0x43
#define CMD_Control_Slave_Proximity    	0x44
#define CMD_Burn_Slave_ID   		 	0x48

#define Buzzer_Long_Ring   			 	0x80
#define Buzzer_Short_Ring   		 	0x00
#define mSecond   					 	0x40
#define Second   		 				0x00
typedef enum
{
/***********/
TAG,
LEN,
CMD,
MAC,
USERACCOUT,
/***********/
SLAVE_ID,
FUNC_ID,
DATA,
SLAVE_CRC0,
SLAVE_CRC1,
FRAME_ERR,
CRC_SUM
}
tCOM_RCV_Status;

tCOM_RCV_Status rec_char_status;
extern xQueueHandle g_Que_TI_Master_to_Slave;
extern xQueueHandle g_Que_TI_Master_to_Slave1;
uint8_t slave_data_rdy_syn;

unsigned char g_highCrcbyte_TI_Master;
unsigned char g_lowCrcbyte_TI_Master;

/*UART4 --> PORT1*/
#define S2E_PORT1_UART_PERIPH   SYSCTL_PERIPH_UART4		//The peripheral on which the Port 1 UART resides.
#define S2E_PORT1_UART_PORT     UART4_BASE				//! The port on which the Port 1 UART resides.
#define S2E_PORT1_UART_INT      INT_UART4				//! The interrupt on which the Port 1 UART resides.

#define S2E_PORT1_RX_PERIPH     SYSCTL_PERIPH_GPIOK		//! The peripheral on which the Port 1 RX pin resides.
#define S2E_PORT1_RX_PORT       GPIO_PORTK_BASE			//! The GPIO port on which the Port 1 RX pin resides.
#define S2E_PORT1_RX_PIN        GPIO_PIN_0				//! The GPIO pin on which the Port 1 RX pin resides.
#define S2E_PORT1_RX_CONFIG     GPIO_PK0_U4RX			//! The GPIO pin configuration for Port 1 RX pin.

#define S2E_PORT1_TX_PERIPH     SYSCTL_PERIPH_GPIOK		//! The peripheral on which the Port 1 TX pin resides.
#define S2E_PORT1_TX_PORT       GPIO_PORTK_BASE			//! The GPIO port on which the Port 1 TX pin resides.
#define S2E_PORT1_TX_PIN        GPIO_PIN_1				//! The GPIO pin on which the Port 1 TX pin resides.
#define S2E_PORT1_TX_CONFIG     GPIO_PK1_U4TX			//! The GPIO pin configuration for Port 1 TX pin.

#define S2E_PORT1_RTS_PERIPH    SYSCTL_PERIPH_GPIOK		//! The peripheral on which the Port 1 RTS pin resides.
#define S2E_PORT1_RTS_PORT      GPIO_PORTK_BASE			//! The GPIO port on which the Port 1 RTS pin resides.
#define S2E_PORT1_RTS_PIN       GPIO_PIN_2				//! The GPIO pin on which the Port 1 RTS pin resides.
#define S2E_PORT1_RTS_CONFIG    GPIO_PK2_U4RTS			//! The GPIO pin configuration for Port 1 RTS pin.

#define S2E_PORT1_CTS_PERIPH    SYSCTL_PERIPH_GPIOK		//The peripheral on which the Port 1 CTS pin resides.
#define S2E_PORT1_CTS_PORT      GPIO_PORTK_BASE			//The GPIO port on which the Port 1 CTS pin resides.
#define S2E_PORT1_CTS_PIN       GPIO_PIN_3				//The GPIO pin on which the Port 1 CTS pin resides.
#define S2E_PORT1_CTS_CONFIG    GPIO_PK3_U4CTS			//The GPIO pin configuration for Port 1 CTS pin.

/*UART1 --> PORT2*/
#define S2E_PORT2_UART_PERIPH   SYSCTL_PERIPH_UART1		//The peripheral on which the Port 1 UART resides.
#define S2E_PORT2_UART_PORT     UART1_BASE				//! The port on which the Port 1 UART resides.
#define S2E_PORT2_UART_INT      INT_UART1				//! The interrupt on which the Port 1 UART resides.

#define S2E_PORT2_RX_PERIPH     SYSCTL_PERIPH_GPIOB		//! The peripheral on which the Port 1 RX pin resides.
#define S2E_PORT2_RX_PORT       GPIO_PORTB_BASE			//! The GPIO port on which the Port 1 RX pin resides.
#define S2E_PORT2_RX_PIN        GPIO_PIN_0				//! The GPIO pin on which the Port 1 RX pin resides.
#define S2E_PORT2_RX_CONFIG     GPIO_PB0_U1RX			//! The GPIO pin configuration for Port 1 RX pin.

#define S2E_PORT2_TX_PERIPH     SYSCTL_PERIPH_GPIOB		//! The peripheral on which the Port 1 TX pin resides.
#define S2E_PORT2_TX_PORT       GPIO_PORTB_BASE			//! The GPIO port on which the Port 1 TX pin resides.
#define S2E_PORT2_TX_PIN        GPIO_PIN_1				//! The GPIO pin on which the Port 1 TX pin resides.
#define S2E_PORT2_TX_CONFIG     GPIO_PB1_U1TX			//! The GPIO pin configuration for Port 1 TX pin.

#define S2E_PORT2_RTS_PERIPH    SYSCTL_PERIPH_GPION		//! The peripheral on which the Port 1 RTS pin resides.
#define S2E_PORT2_RTS_PORT      GPIO_PORTN_BASE			//! The GPIO port on which the Port 1 RTS pin resides.
#define S2E_PORT2_RTS_PIN       GPIO_PIN_0				//! The GPIO pin on which the Port 1 RTS pin resides.
#define S2E_PORT2_RTS_CONFIG    GPIO_PN0_U1RTS			//! The GPIO pin configuration for Port 1 RTS pin.

#define S2E_PORT2_CTS_PERIPH    SYSCTL_PERIPH_GPION		//The peripheral on which the Port 1 CTS pin resides.
#define S2E_PORT2_CTS_PORT      GPIO_PORTN_BASE			//The GPIO port on which the Port 1 CTS pin resides.
#define S2E_PORT2_CTS_PIN       GPIO_PIN_1				//The GPIO pin on which the Port 1 CTS pin resides.
#define S2E_PORT2_CTS_CONFIG    GPIO_PN1_U1CTS			//The GPIO pin configuration for Port 1 CTS pin.

xQueueHandle g_QueWiFiCMD;
xQueueHandle g_QueWiFiDataBack;
xQueueHandle g_QueWiFi_APDataBack;
xQueueHandle g_QueWiFi_AP_NoDHCP_DataBack;
xQueueHandle g_QueWiFi_STA_DataBack;
xQueueHandle g_QueWiFi_IPDataBack;
xQueueHandle g_QueWiFiScan_DataBack;
xQueueHandle g_QueWiFi_STA_Rejoin_DataBack;
extern void wifi_cmd_err_exit(void);
extern void wifi_disconnect_rejoin(void);

#define EnterCMDMode    	0x01
#define ExitCMDMode     	0x02
#define WiFiSCAN     		0x03 //
#define SetNETWORK_SSID     0x04 //   "set w s %s";
#define SetNETNETWORK_PASS  0x05 //	  "set w p %s";
#define WiFiCMDSave  		0x06 //   "save";
#define WiFiCMDJoin  		0x07 //	  "join";
#define WiFiCMDAPMode 		0x08 //   "apmode";
#define WiFiCMDReboot 		0x09 //   "reboot";
#define LoadFactory 		0x0A //   "factory RESET";
#define GetMAC 				0x0B //   "get mac";   feedback Mac address;
#define GetSSID 			0x0C //   "get wlan";  feedback ssid;
#define WiFiSTA_Net_Connection 			0x0D //   commands group;  feedback AOK;
#define WiFiAP_Net_Connection 			0x0E //   commands group;  feedback AOK;
#define GetAPSSID 			0x11 //   "get wlan";  feedback;
#define GetWiFiMAC 			0x12 //   "get mac";  feedback ;
#define GetWiFi_IPAdress 	0x13 //   "get ip";  feedback ;
#define GetWiFiAP_SMAdress 	0x14 //   "get ip";  feedback ;
#define EnDHCPServer 		0x15 //   "set ip dhcp 4";  feedback;
#define SetAP_Frequency 	0x16 //   "set w c %d";  feedback;
#define SetAP_Security 		0x17 //   "set apmode passphrase";  feedback;
#define APMode_Non_DHCP_Mode_Setting 		0x18 //   commands group;  feedback AOK;
#define APMode_DHCP_Mode_Setting 		0x19 //   commands group;  feedback AOK;
#define EnAPMode 			0x20			 //enable apmode; "set wlan join 7"
									//"set apmode ssid %s" <string> // Set up network broadcast SSID
#define STAMode_Non_DHCP_Mode_Setting 			0x21			 //commands group;  feedback AOK;
#define  STAMode_DHCP_Mode_Setting						0x22			 //commands group;  feedback AOK;

#define WiFi_DataTrans_TCP_Server		0x23
#define WiFi_DataTrans_TCP_Client		0x24
#define WiFi_DataTrans_UDP				0x25
#define WiFi_DeviceID_Altered			0x26
#define WiFi_DataTrans_State			0x27
#define WiFlyPage_TCP_Close				0x28
#define WiFi_Rejoin						0x29

#define open_chars "~o~"
#define CMD_set_comm_open "set comm open "open_chars
#define close_chars "~c~"
#define CMD_set_comm_close "set comm close "close_chars
#define Link_OK "*HELLO*"
#define ACCOUNT_NAME_LEN  	6
#define ACCOUNT_PASSWORD_LEN  	6
#define CMD_AOK "OK"
#define NETWORK_SSID "mima12345678"
#define NETWORK_PASS "12345678"
//extern tWiFi_CommandSet sWiFi_CommandSet;
extern void vTaskDelay( portTickType xTicksToDelay ) PRIVILEGED_FUNCTION;
//extern int WiFi_STA_Network_Connect();
//extern int WiFi_AP_Network_Connect();
extern int SetWiFi_UR_Bardrate(uint32_t baudrate);
extern void EnterCMD(void);
extern void ExitCMD(void);
extern int module_send_cmd(const char *response, const char *fmt, ...);
extern void wifimoduledefault(void);
extern void UARTprintf(const char *pcString, ...);
extern int APMode_Enable_DHCP_Server_Set(void); // Dynamic ip mode;
extern int APMode_Disable_DHCP_Server_Set(void); // Static ip mode;
extern uint32_t COM_WiFiTaskInit(void);
extern uint32_t COM_1_WiFiTaskInit(void);
extern uint32_t COM_2_WiFiTaskInit(void);
extern int module_cmd_mode();
extern int STAMode_DHCP_Mode_Set(void); // Station, Dynamic ip mode;;
extern int STAMode_Non_DHCP_Mode_Set(void); // Station, Static ip mode;
extern uint32_t WiFi_StatusLED_TaskInit(void);
extern uint32_t WiFiCMDTaskInit(void);
extern uint32_t WiFi_WebAccess_TaskInit(void);
extern void WiFiModulereboot(void);
extern void SysCtlReset(void);
extern int usnprintf(char * restrict s, size_t n, const char * restrict format, ...);
extern int STAMode_Scan(void);
extern void _UR4_SerialSetConfig(uint32_t UR4_Baudrate);
extern int WiFiGetIP(void);
extern int WiFiGet_DataTrans_State(void);
extern uint32_t WiFiGetLocalPort(void);
extern uint32_t WiFiGetRemotePort(void);
extern uint32_t WiFiGetRemoteIP(void);
extern int WiFiGetProl(void);
extern void AllStatusLEDoff(void);
extern void SysHaltLED_RED(void);
extern void SysreadyLED_Green(void);
extern void WiFi_RX_TX_LED_Defaultoff(void);
extern void delayms(uint32_t count);
extern int SetWiFi_UR_Baudrate(uint32_t baudrate);
extern void WiFiINI(void);
extern void WiFiReset(void);
extern int ReceiveWifiResponse(void);
extern void TCP_CLOSE(void);
extern uint8_t RSSI[wifi_scan_qty],Scaned_AP_Num;
extern char WiFi_SSID_temp[wifi_scan_qty][NET_NAME_LEN];
extern uint8_t securitymode[wifi_scan_qty];
extern tConfigParameters g_sWorkingDefaultParameters;

extern void ModbusInit(void);
extern void ModbusListen(unsigned short usModbusPort);

extern void APMode_Default_Set(void);

extern void WiFiData_TCP_S(void);
extern void WiFiData_TCP_C(void);
extern void WiFiData_UDP(void);
//extern void WiFiData_TCP_C_OLD(void);	//1128
extern void Syn_Modname(void);



#define RN_NOT_ERR_OK(_retval, _function) ((_retval = _function) != RN_ERR_OK)
enum
{
	RN_ERR_OK 					= 0,
	RN_ERR_INIT 				= 1,
	RN_ERR_XMIT 				= 2,
	RN_ERR_READ 				= 3,
	RN_ERR_WRITE 				= 4,
	RN_ERR_EMPTY 				= 5,
	RN_ERR_TIMEOUT 				= 6,
	RN_ERR_FULL					= 7,
	RN_ERR_Fault				= 8,  // find response, but don't have the expected string;
	RN_ERR_CMDFault				= 9,
};

enum
{
	WiFi_Normal 								= 0,  // cmd execut ok;
	WiFi_URBUS_Error 							= 1,  // set UR rate to 115200, but can't enter cmd and exit;
	WiFi_INI_Error 								= 2,  // after set UR rate to 115200, send cmd and exit cmd;it's ok;
	WiFi_CMD_havedata_noexpected 				= 3,  // find response, but don't have the expected string;
	WiFi_EnterCMD_Error 						= 4,
	WiFi_ExeCMD_Error 							= 5,
	WiFi_Datain 								= 6,  //
	WiFi_LED_Off								= 7,  //
	WiFi_STA_CNN_Pass_ERR						= 8,  //
};
uint8_t locator_scan; // for led flash use;
uint8_t WiFi_Status_Retval;
uint8_t rxdata_receiving;
uint8_t txdata_sending;
uint8_t WiFiLEDflashtime;
char wifi_ini_response_data[10];
//rn_err_types_t;
//typedef int err_t;
bool Response_UDP_CMD_sendtoWiFi;
extern uint32_t User_Check(uint8_t *USER_NAME,uint8_t *USER_PASSWORD);
//extern void Decode_Command(struct udp_pcb *pcb, struct pbuf *p,struct ip_addr *addr,u16_t port,uint8_t *command,uint8_t *response_data,uint8_t *received_data_frame);
extern uint32_t CRC_CheckOK(uint8_t *UDP_Received_data);
extern uint32_t COMPAR_WiFi_MAC_ADDR(uint8_t *MAC_IN_Query);
extern void clearRX_ringbuffer(uint8_t port);
extern void Cal_CRC_and_append(unsigned char *str, unsigned char len);
extern uint8_t ui8PreviousCMD;
extern uint8_t g_pui8LocatorData[36];
//static uint8_t g_pui8_RST_Response[4];
extern uint8_t g_pui8_Response_4Byt[4];
extern uint8_t g_pui8_Read_Conf_Response[256];
extern uint8_t ui8PreviousCMD;

bool wifi_restoring;
bool bWiFiChanged;
bool WiFiINI_OK;
bool wifiini_flag;
bool wifidataavail_flag;
bool WiFi_to_COM_TX_Finish;
bool LAN_to_COM_TX_Finish;
bool wifi_sendpage;
xQueueHandle g_Que_COM_LAN_WiFi;
//bool factory_restore_flag;
extern char wifiini_data_response[100];

uint32_t WiFiDynamicIP;
extern _Bool WiFi_Assistant_Set_flag;
#define STACKSIZE_SerialTASK    512
#define STACKSIZE_WiFiCMDTASK    512
typedef enum
{
	vNETWORK_SSID,
	vNETWORK_PASS
}
tConstantParas;

union uSetWiFiParas {
	uint32_t SetValue;
	tConstantParas StringParas;
};



typedef struct
{
	uint8_t  ui8Name;
	bool	CMDExecute_status;
	union uSetWiFiParas WiFiPara0;
	union uSetWiFiParas WiFiPara1;
}
tWiFiCMD;


/*
typedef struct{
	char *pSSID;
	char *pPASSWORDS;
}tCONSTANT;

const tCONSTANT sWiFiCONST = {
		&NETWORK_SSID,
		&NETWORK_PASS

};
*/
#define WIFISTA_PASSWORD_LEN 13
char *ipconvtosring_use;
char wifi_ip_temp[17];

//*****************************************************************************
//
// Defines for setting up the system clock.
//
//*****************************************************************************
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)

//*****************************************************************************
//
// Interrupt priority definitions.  The top 3 bits of these values are
// significant with lower values indicating higher priority interrupts.
//
//*****************************************************************************
#define SYSTICK_INT_PRIORITY    0x80
//#define ETHERNET_INT_PRIORITY   0xC0
#define MODBUS_UART_BASE        UART1_BASE

extern uint32_t chartodigit(char*IPstring);
extern void ModbusTimerInit(void);
extern void RS485_TxEnable(uint32_t ui32Port);
extern void RS485_TxDisable(uint32_t ui32Port);
extern int CheckModbusTCPStatemachine(void);
extern void ModbusTCPReceiveSerialSend(unsigned long ulPort, unsigned char ucChar);
extern void ResetModbusTCPStatemachine(void);
extern int UARTwrite(const char *pcBuf, uint32_t ui32Len);
extern void ModbusHandler(void);
extern void Timer0IntHandler(void);
extern void Timer3IntHandler(void);
extern void ModbusRTURecvTCPSend(unsigned char ucChar);
extern void ModbusTimerDisArm(void);
typedef enum
{
MODBUS_RTU_STATE_IDLE,
MODBUS_RTU_STATE_INIT,
MODBUS_RTU_STATE_FRAME_START,
MODBUS_RTU_STATE_SLAVE_ID,
MODBUS_RTU_STATE_FUNC_ID,
MODBUS_RTU_STATE_FRAME_LENGTH_BEFORE, // by pony
MODBUS_RTU_STATE_FRAME_LENGTH,
MODBUS_RTU_STATE_FRAME_CRC1,
MODBUS_RTU_STATE_FRAME_CRC2,
MODBUS_RTU_STATE_COIL_ADDRESS_HIGH,
MODBUS_RTU_STATE_COIL_ADDRESS_LOW,
MODBUS_RTU_STATE_WRITE_DATA_HIGH,
MODBUS_RTU_STATE_WRITE_DATA_LOW,
MODBUS_RTU_STATE_FRAME_SENT,
MODBUS_RTU_STATE_INVALID
}
tMODBUSRTUState;

tMODBUSRTUState modbusRTUState;
tMODBUSRTUState modbusRTUState2;
tMODBUSRTUState modbusRTUState3;
tMODBUSRTUState modbusRTUState_LAN;
bool g_bFirmwareUpdate;
extern void ModbusPDU_Req_LAN_Port(unsigned char ucChar);
//extern uint8_t counter_filter;
//*****************************************************************************
//
// The system clock frequency.
//
//*****************************************************************************
uint32_t g_ui32SysClock;
extern uint8_t g_pui8MACAddr[6];
extern void SoftwareUpdateRequestCallback(void);
//#define HW_restore_GPIO_Pin GPIO_PIN_6  // change to GPIO_PIN_6/usb+;
#define WiFi_UR_to_MCU_UR_Baudrate 230400  //115200 230400 921600 460800

extern void SoftwareUpdateBegin(uint32_t ui32SysClock);
extern void SoftwareUpdateInit(tSoftwareUpdateRequested pfnCallback);
bool UDPdataready;
//struct udp_pcb *UDPpcb;
void *UDPpcb;
ip_addr_t UDPaddr;
extern void UDPInit(void);
extern void UDPHandler(void);
//bool webrestroe_flag;
extern bool wifi_app_CMD_processing;
extern bool wifiCMD_Executing;

#define Have_WiFi_Module_SKU 0

#define Debug_inf_On 1
#define HW_Triger_TCP 0
#define Rev_A_AD7491_Board 0
#define Rev_B_AD7491_Board 1
#define TI_Master_Board 0

bool PWR_OK_Flag;
extern void delayuS(uint32_t count);
extern void ConsolePutString(char* str);
extern void ConsolePutString_to_COM(char* str);
extern tConfigParameters g_sParameters;
extern const tConfigParameters g_sParametersFactory;
extern uint8_t response_from_master_Frame_40h[256];

extern bool wificmd_processing_flag;

#if TI_Master_Board
#define COM1_DIR_Pin 					GPIO_PIN_0
//#define COM1_DIR_Port 					GPIO_PORTL_BASE

#define COM1_Mode0_Pin 					GPIO_PIN_1
#define COM1_Mode0_Port 				GPIO_PORTL_BASE

#define COM1_Mode1_Pin 					GPIO_PIN_2
//#define COM1_Mode0_Port 				GPIO_PORTL_BASE

#define COM1_TERM_Pin 					GPIO_PIN_3
#define COM1_TERM_Port 					GPIO_PORTL_BASE

#define HW_Restore_Pin 					GPIO_PIN_2
#define HW_Restore_Port 				GPIO_PORTQ_BASE

#define WiFi_Reset_Pin 					GPIO_PIN_5
#define WiFi_Reset_Port 				GPIO_PORTK_BASE

// define debug port to UR0;
#define Debugport_RX_Pin 					GPIO_PA0_U0RX
#define Debugport_TX_Pin 					GPIO_PA1_U0TX
#define Debugport_RTS_Pin 					GPIO_PH0_U0RTS
#define Debugport_CTS_Pin 					GPIO_PH1_U0CTS


#define Debugport_RX_PORT 					GPIO_PORTA_BASE
#define Debugport_TX_PORT 					GPIO_PORTA_BASE
#define Debugport_CTS_PORT 					GPIO_PORTH_BASE
#define Debugport_RTS_PORT 					GPIO_PORTH_BASE

#define Debugport_RX_GP_PIN       			GPIO_PIN_0
#define Debugport_TX_GP_PIN       			GPIO_PIN_1

#define Debugport_RTS_GP_PIN       			GPIO_PIN_0
#define Debugport_CTS_GP_PIN       			GPIO_PIN_1

#endif


 #if Rev_A_AD7491_Board
//#if TI_Master_Board
#define COM1_DIR_Pin 					GPIO_PIN_4
//#define COM1_Mode0_Port 				GPIO_PORTP_BASE

#define COM1_Mode0_Pin 					GPIO_PIN_3
#define COM1_Mode0_Port 				GPIO_PORTP_BASE

#define COM1_Mode1_Pin 					GPIO_PIN_2
//#define COM1_Mode0_Port 				GPIO_PORTP_BASE

#define COM1_TERM_Pin 					GPIO_PIN_4
#define COM1_TERM_Port 			 		GPIO_PORTQ_BASE

#define HW_Restore_Pin 					GPIO_PIN_6
#define HW_Restore_Port 				GPIO_PORTL_BASE

#define WiFi_Reset_Pin 					GPIO_PIN_0
#define WiFi_Reset_Port 				GPIO_PORTM_BASE

// define debug port to UR0;
#define Debugport_RX_Pin 					GPIO_PA0_U0RX
#define Debugport_TX_Pin 					GPIO_PA1_U0TX
#define Debugport_RTS_Pin 					GPIO_PH0_U0RTS
#define Debugport_CTS_Pin 					GPIO_PH1_U0CTS


#define Debugport_RX_PORT 					GPIO_PORTA_BASE
#define Debugport_TX_PORT 					GPIO_PORTA_BASE
#define Debugport_CTS_PORT 					GPIO_PORTH_BASE
#define Debugport_RTS_PORT 					GPIO_PORTH_BASE

#define Debugport_RX_GP_PIN       			GPIO_PIN_0
#define Debugport_TX_GP_PIN       			GPIO_PIN_1

#define Debugport_RTS_GP_PIN       			GPIO_PIN_0
#define Debugport_CTS_GP_PIN       			GPIO_PIN_1


#endif

#if Rev_B_AD7491_Board
#define COM1_DIR_Pin 					GPIO_PIN_4
//#define COM1_DIR_Port 					GPIO_PORTP_BASE

#define COM1_Mode0_Pin 					GPIO_PIN_3
#define COM1_Mode0_Port 				GPIO_PORTP_BASE

#define COM1_Mode1_Pin 					GPIO_PIN_2
//#define COM1_Mode1_Port 				GPIO_PORTP_BASE

#define COM1_TERM_Pin 					GPIO_PIN_4
#define COM1_TERM_Port 					GPIO_PORTQ_BASE

#define HW_Restore_Pin 					GPIO_PIN_7
#define HW_Restore_Port 				GPIO_PORTD_BASE

#define WiFi_Reset_Pin 					GPIO_PIN_0
#define WiFi_Reset_Port 				GPIO_PORTM_BASE

// define debug port to UR0;
#define Debugport_RX_Pin 					GPIO_PA0_U0RX
#define Debugport_TX_Pin 					GPIO_PA1_U0TX
#define Debugport_RTS_Pin 					GPIO_PH0_U0RTS
#define Debugport_CTS_Pin 					GPIO_PH1_U0CTS


#define Debugport_RX_PORT 					GPIO_PORTA_BASE
#define Debugport_TX_PORT 					GPIO_PORTA_BASE
#define Debugport_CTS_PORT 					GPIO_PORTH_BASE
#define Debugport_RTS_PORT 					GPIO_PORTH_BASE

#define Debugport_RX_GP_PIN       			GPIO_PIN_0
#define Debugport_TX_GP_PIN       			GPIO_PIN_1

#define Debugport_RTS_GP_PIN       			GPIO_PIN_0
#define Debugport_CTS_GP_PIN       			GPIO_PIN_1
#endif

typedef enum
{
	TCPChar1,
	TCPChar2,
	TCPChar3,
	TCPChar4,

} TCPcharCheck;
TCPcharCheck TCPChar_Status;
bool WiFi_TCP_connected;
typedef enum
{
	WiFi_Raw_Char1,
	WiFi_Raw_Char2,
	WiFi_Raw_Char3,

} TCPRawcharCheck;
TCPRawcharCheck WiFi_Raw_TCP_Status;

typedef enum
{
	WiFi_Origin_Char1,
	WiFi_Origin_Char2,
	WiFi_Origin_Char3,
	WiFi_Origin_Char4,
	WiFi_Origin_Char5,
} TCPOrigin_charCheck;
TCPOrigin_charCheck WiFi_Origin_TCP_Status;
bool Webpage_Visit_Req;
char previous_char;
char previouschar;
uint8_t cnt_data_or_web_request,temp_for_judge_web_request[4];
bool save_WiFi_TCP_connected;
bool save_Webpage_Visit_Req;
bool GetWiFi_IP_CMD_Executing;
bool TCP_close_processing;

#define retry_time_for_IP_and_Join_CMD 1
#define CMD_retry_time 100// send cmd function, if is CMD command, retry time set to 2000, eneral is 1000;
#define CMD_retry_time_external 1  //set ur baudrate;
#define MODULE_ENTER_CMD_MODE_RETRIES  5 //not used;
#define RX_CMD_RESPONSE_TIMEOUT_scan 3000 // *10 ms from 20000 to 2000
#define CMD_RESPONSE_TIMEOUT_general 2000 // now, join cmd fail rate 20%, increase timeout from 200 to 500;

#define MODULE_RX_CMD_RESPONSE_TIMEOUT_INI 1000 // *10 ms, receive pin7=0,... factory reset information of wifi;


#define WiFi_Setting  (wifi_restoring == true)||(wifiCMD_Executing==true)  \
||(WiFi_Assistant_Set_flag==true)||(GetWiFi_IP_CMD_Executing ==true)       \
||(TCP_close_processing==true)||(wifi_app_CMD_processing ==true)||(wificmd_processing_flag==true)
uint16_t cnt_debug0,I_cmd;
extern tRingBufObject Data_From_COM_To_LAN;
extern tRingBufObject Data_From_COM1_To_LAN;
extern tRingBufObject Data_From_COM2_To_LAN;
extern void WiFiSTAJoin(void);
extern uint32_t WiFiIPGetandConvert(void);
extern void Timer1_Init();
extern void restore_call(void);
extern void PWM_Init(void);
extern int GetWiFiMACAddress(void);
extern void ModbusPDU_Req_WiFi_Port(unsigned char ucChar);
extern void ModbusRTURecv_COM_Port(unsigned char ucChar);
bool Find_TCP_open_in_Command;
//#define Mosbus_RTU_SUPPORT 1;
bool Mosbus_RTU_SUPPORT;
extern uint8_t pui8TempRTU[256],i32IndexRTU;
extern void resume_channel_connection(void);
extern int ReceiveWifiResponse_INI(void);
extern void Make_Delay(uint8_t port);
extern void send_reboot_cmd(void);
extern int WiFiGetIP_SysRST(void);
#define watchdog_EN 	0
extern void watchdog_ini(void);
