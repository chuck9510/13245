//*****************************************************************************
//
// config.h - Configuration of the serial to Ethernet converter.
//
// Copyright (c) 2014-2015 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.0.12573 of the EK-TM4C1294XL Firmware Package.
//
//*****************************************************************************

#define origin_code 0x00
#define revise_code 0x01
#define NET_NAME_LEN 21 // from 13 to 21;
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "config.h"

//#include "user_define.h"
//*****************************************************************************
//
//! \addtogroup config_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// During debug, DEBUG_UART may be defined with values 0 or 1 to select which
// of the two UARTs are used to output debug messages.  Debug messages will be
// interleaved with any telnet data also being carried via that UART so great
// care must be exercised when enabling this debug option.  Typically, you
// should use only a single telnet connection when debugging with DEBUG_UART
// set to use the inactive UART.
//
//*****************************************************************************
//#define DEBUG_UART 0
//extern void UARTprintf(const char *pcString, ...);
//#define DEBUG_UART
#ifdef DEBUG_UART
#define DEBUG_MSG               UARTprintf
#else
#define DEBUG_MSG               while(0) ((int (*)(char *, ...))0)
#endif

//void SetClientNum();//0823
//*****************************************************************************
//
//! The number of serial to Ethernet ports supported by this module.
//
//*****************************************************************************
#define MAX_S2E_PORTS           3 //changed to 1 from 2;

uint8_t Switch_WiFi_to_COM; // 1,wifi to com;
uint8_t Switch_LAN_to_COM; // 1,lan to com;

//*****************************************************************************
//
//! This structure defines the port parameters used to configure the UART and
//! telnet session associated with a single instance of a port on the S2E
//! module.
//
//*****************************************************************************
typedef struct
{


	//
	//! The data size to be use for the serial port, specified in bits.  Valid
	//! values are 5, 6, 7, and 8.
	//
	uint8_t ui8DataSize;//	1

	//
	//! The parity to be use for the serial port, specified as an enumeration.
	//! Valid values are 1 for no parity, 2 for odd parity, 3 for even parity,
	//! 4 for mark parity, and 5 for space parity.
	//
	uint8_t ui8Parity;//	1

	//
	//! The number of stop bits to be use for the serial port, specified as
	//! a number.  Valid values are 1 and 2.
	//
	uint8_t ui8StopBits;//	1

	//
	//! The flow control to be use for the serial port, specified as an
	//! enumeration.  Valid values are 1 for no flow control and 3 for HW
	//! (RTS/CTS) flow control.
	//
	uint8_t ui8FlowControl;//	1

	//
	//! The baud rate to be used for the UART, specified in bits-per-second
	//! (bps).
	//
	uint32_t ui32BaudRate;//	4

	//
	//! The timeout for the TCP connection used for the telnet session,
	//! specified in seconds.  A value of 0 indicates no timeout is to
	//! be used.
	//
	uint32_t ui32TelnetTimeout;//	4

	//
	//! The local TCP port number to be listened on when in server mode or
	//! from which the outgoing connection will be initiated in client mode.
	//
	uint16_t ui16TelnetLocalPort;//	2

	//
	//! The TCP port number to which a connection will be established when in
	//! telnet client mode.
	//
	uint16_t ui16TelnetRemotePort;//	2

	//
	//! The IP address which will be connected to when in telnet client mode.
	//
	uint32_t ui32TelnetIPAddr;//	4

	//
	//! Miscellaneous flags associated with this connection.
	//Bit1:  1为Raw
	//		 0为 Telnet
	//Bit2:  1为Modbus enable,即RTU-TCP
	//		 0为 Modbus disable
	//Bit3:  1为RFC2217 enable
	//		 0为 RFC2217 disab
	//Bit4:  1为RTU-RTU
	//		 0 为 非RTU-RTU
	//
	uint8_t ui8Flags;//	1

#if origin_code

	//! Padding to ensure consistent parameter block alignment, and
	//! to allow for future expansion of port parameters.
	//
	uint8_t ui8Reserved0[19];
#endif

#if revise_code
	uint8_t uiTelnetURL[20];			//	30			IP 地址或域名都以 ASCII 码发送
	//			例子为： 192.168.0.1
	//! Work mode
	//
	uint8_t ucWorkMode;					//	1		03	工作方式：
	//			, 1 ： TCP Server , 2： TCP Client 3 ： UDP
	//
	//! Series package length;
	//
	uint8_t uiPackLen[4];				//		4		C8 00 00 00		串口打包长度

	//! Series package time;
	//
	uint8_t uiPackTime;					//		1		0A		串口打包时间

	//! 	UC timecount
	//
	uint8_t ucTimeCount;					//		1		91		请将读取回的值原样写入

	//! 	TCP server type
	//
	uint8_t uiTCPServerType;				//		1		1		高 4 位： TCPserver 模式下连接的数量
	//						低 4 位： TCPserver 模式下连接 的样式
	//							（ 1~3 ）
	//						1 ：透明传输
	//						2 ：按 ID 发送（没 ID 的包丢弃）
	//						3 ： 按 ID 发送 （ 没 ID 的包发送给所有客户端）
	//! 	Transit Mode
	//
	uint8_t uiTransitMode;  // 	1	RS232 /RS485 /RS485/422 4 wires;

	//! 	Transit Mode
	//
	//uint8_t uiWiFiCMDMode;  //1     enter_CMD, exit_CMD;
	//! RFC2217
	uint8_t ucRFC2217;			//	1

#endif

}
tPortParameters;//	61 bytes


typedef struct
{
	//command name;
	uint8_t ui8CMDName;

	//command length;
	uint8_t ui8CMDLen;
}
tUser_Command;

/*
// wifi commands;

typedef struct
{
    //name;
    uint8_t name[20];

    //WiFi enter CMD command ;
    uint8_t len;
}
tWiFi_Command;

typedef struct
{
    //scan command;
	tWiFi_Command ui8WiFiScan;

    //WiFi enter CMD command ;
	tWiFi_Command ui8enterCMDmode;
}
tWiFi_CommandSet;
 */
//*****************************************************************************
//
//UPD commands for APP operation;
//
//*****************************************************************************



typedef struct  {
	tUser_Command	SCAN;
	tUser_Command	Reset;
	tUser_Command	Rdconfig;
	tUser_Command	Basiconfig;
	tUser_Command	Saveconfig;
	tUser_Command	Loaddefault;
	tUser_Command	Com0Config;
	tUser_Command	Com1Config;
}APPCommand_Set;

//*****************************************************************************
//
//User APP Commands;
//
//*****************************************************************************
static const APPCommand_Set APP_CMD_INS =
{
		//scan command;
		{0x01,0x01},
		//reset command;
		{0x02,0X13},
		//read config command;
		{0x03,0X13},
		//basic config command;
		{0x05,0X56},
		//save config command;
		{0x04,0X13},
		//load default command;
		{0x0B,0X13},
		//COM0 config command;
		{0x06,0X52},
		//COM1 config command;
		{0x07,0X52},
		//read temp config command;
		//	{0x0A,0X13}
};



//*****************************************************************************
//
//! Bit 0/1 of field ucFlags in tPortParameters is used to indicate whether to
//! operate as a telnet server , a telnet client or UDP
//0823
//*****************************************************************************
//#define PORT_FLAG_CONTACT_MODE   0x03

//*****************************************************************************
//
// Helpful labels used to determine if we are operating as a telnet client or
// server (based on the state of the PORT_FLAG_TELNET_MODE bit in the ucFlags
// field of tPortParameters).
//0823
//*****************************************************************************
#define PORT_TELNET_SERVER      0x03
#define PORT_TELNET_CLIENT      0x01
#define PORT_UDP      0x00

//*****************************************************************************
//
//! Bit 2 of field ucFlags in tPortParameters is used to indicate whether to
//! bypass the telnet protocol (raw data mode).
//0823
//*****************************************************************************
#define PORT_FLAG_PROTOCOL      0x02 // changed from 04 to 02; meet youren app definition; bit1: 1, raw;0, telnet; bit3: 1, modebus_en;0, modbus_dis;

//*****************************************************************************
//
// Helpful labels used to determine if we are operating in raw data mode, or
// in telnet protocol mode.
//0823
//*****************************************************************************
#define PORT_PROTOCOL_TELNET    0x00
#define PORT_PROTOCOL_RAW       PORT_FLAG_PROTOCOL
#define PORT_PROTOCOL_MODBUS       Modbus_SET
#define PORT_MODBUS_RTU_TCP       Modbus_EN
#define PORT_MODBUS_RTU_RTU       ModbusRTU_RTU

//*****************************************************************************
//
// The length of the ucModName array in the tConfigParameters structure.  NOTE:
// Be extremely careful if changing this since the structure packing relies
// upon this values!
//
//*****************************************************************************
#if origin_code
#define MOD_NAME_LEN            40
#endif

#if revise_code
#define MOD_NAME_LEN            18
#endif

#define   WiFi_SERVER           0x04
#define   WiFi_CLIENT           0x02
#define   WiFi_UDP              0x01

#define PORT_TELNET_S_C      0x03
#define PORT_TELNET_CLIENT_OLD	0x02
#define PORT_TELNET_CLIENT      0x01
#define PORT_UDP      0x00
typedef struct{
	uint8_t ucWiFiMode;
	uint8_t ucWiFiTransMode;
	uint8_t WiFiTransPcolFlag;//	bit2 TCP SERVER;bit1 TCP Client; bit0 UDP;
	uint32_t ui32DataIPAddr;//	4
	uint16_t ui16DataRemotePort;//	2
	uint16_t ui16DataLocalPort;//	2
}tWiFiModSel;

typedef struct{
	uint8_t ucAPNetMode;
	uint8_t ui8NetName[NET_NAME_LEN];
	uint8_t ucFrequency_Channel;
	uint8_t ucSecurityMode;
	uint32_t ui32AP_IP;
	uint32_t ui32APSubnetMask;
	uint8_t ucDHCPType;
	uint32_t ui32APGateWay;
	uint8_t ui8SetPasswords[9];// add 9 bytes;0906
}tWiFiAPSet;

typedef struct{
	uint8_t ui8NetName[NET_NAME_LEN];//NET_NAME_LEN=13
	uint8_t ucSecurityMode;
	uint8_t ucEncryptionType;
	uint8_t ui8InputPasswords[13];// change form 6 to 9; add 6
	uint32_t ui32STASubnetMask;
	uint8_t ucStationConnType;
	uint32_t sWiFiSTAstaticGateway;
	uint32_t sWiFiSTAstaticIP;
}tWiFiStaionSet;

typedef struct{
	//	uint8_t ui8NetName[NET_NAME_LEN];//NET_NAME_LEN=8
	uint8_t ucWiFiMAC[6];

	tWiFiModSel sWiFiModSel;
	tWiFiAPSet sWiFiAPSet;
	tWiFiStaionSet sWiFiStaionSet;
}tWiFiParameters;

//*****************************************************************************
//
//! This structure contains the S2E module parameters that are saved to flash.
//! A copy exists in RAM for use during the execution of the application, which
//! is loaded from flash at startup.  The modified parameter block can also be
//! written back to flash for use on the next power cycle.
//
//*****************************************************************************
typedef struct
{
	//
	//! The sequence number of this parameter block.  When in RAM, this value
	//! is not used.  When in flash, this value is used to determine the
	//! parameter block with the most recent information.
	//
	uint8_t ui8SequenceNum;//	1

	//
	//! The CRC of the parameter block.  When in RAM, this value is not used.
	//! When in flash, this value is used to validate the contents of the
	//! parameter block (to avoid using a partially written parameter block).
	//
	uint8_t ui8CRC;//	1

	//
	//! The version of this parameter block.  This can be used to distinguish
	//! saved parameters that correspond to an old version of the parameter
	//! block.
	//
	uint8_t ui8Version;//	1

	//
	//! Character field used to store various bit flags.
	//
	uint8_t ui8Flags;//	1

	//
	//! Padding to ensure consistent parameter block alignment.
	//
	//    uint8_t ui8Reserved0[4];//	4

	//    uint8_t ui8Reserved1[4];//	4

	//
	//! The configuration parameters for each port available on the S2E
	//! module.
	//
	//tPortParameters sPort[1];//61 bytes;-10 =51
	tPortParameters sPort[3];//61 bytes;-10 =51			chuck for adding uart

	//
	//! The configuration parameters for WiFI port available on the S2E
	//! module.
	//
	tWiFiParameters sWiFiModule;//83bytes

	//
	//! An ASCII string used to identify the module to users via web
	//! configuration.
	//
	uint8_t ui8ModName[MOD_NAME_LEN];//MOD_NAME_LEN=18

	//
	//! The static IP address to use if DHCP is not in use.
	//
	uint32_t ui32StaticIP;//	4

	//
	//! The default gateway IP address to use if DHCP is not in use.
	//
	uint32_t ui32GatewayIP;//	4

	//
	//! The subnet mask to use if DHCP is not in use.
	//
	uint32_t ui32SubnetMask;//	4

#if origin_code
	//
	//! Padding to ensure the whole structure is 256 bytes long.
	//
	uint8_t ui8Reserved2[116];
#endif

#if revise_code
	//! Username
	uint8_t ui8Username[6];			//	6

	//! Password
	uint8_t ui8Passwords[6];			//	6			MAC Adress 不用放这里;

	//uint8_t ucNetSendTime;  //need check use;

	//! UPNP port
	uint8_t usLocationURLPort[2];			//	2		2019

	//! HTTP service port
	uint8_t usHTTPServerPort[2];			//	2		5000

	//! Write back what you read;
	uint8_t ucUserFlag;			//	1

	//! Device ID
	uint8_t uiID[2];			//	2		0100

	//! Device Type
	uint8_t ucIDType;			//	1



	//! MAC Adress
	uint8_t ucUserMAC[6];		//		6		FF FF FF FF FF FF

	//! Data channel select
	uint8_t datachansel;		//  1   	1:LAN_COM     2:WLAN_COM
	// debug for YOUREN APP;
	//   uint8_t youren_TBD[15];
	//
	//! The static IP address to use if DHCP is not in use.
	//
	uint32_t ui32WiFiModuleBaudRate;//	4

	// Modbus_Enable;
	uint8_t modbus;

	//    uint32_t onoff_counter;//	4
	//
	//! Padding to ensure the whole structure is 256 bytes long.
	//
	//   uint8_t ui8Reserved2[15];		//-4 = 15 for on off test;
	uint32_t counter_for_resetok;//	4
	//    uint8_t ui8Reserved2[3];        // can be used.

	uint8_t ui8Reserved3[2];    // can't be used; it seems not existed! no this area!
#endif
}
tConfigParameters;//256 bytes

//*****************************************************************************
//
//! If this flag is set in the ucFlags field of tConfigParameters, the module
//! uses a static IP.  If not, DHCP and AutoIP are used to obtain an IP
//! address.
//
//*****************************************************************************
#define CONFIG_FLAG_STATICIP    0x80

//*****************************************************************************
//
//! The offset in EEPROM to be used for storing parameters.
//
//*****************************************************************************
#define EEPROM_PB_START         0x00000000

//*****************************************************************************
//
//! The size of the parameter block to save.  This must be a power of 2,
//! and should be large enough to contain the tConfigParameters structure.
//
//*****************************************************************************
#define EEPROM_PB_SIZE          512			//chuck

//*****************************************************************************
//
//! The size of the ring buffers used for interface between the UART and
//! telnet session (RX).
//
//*****************************************************************************
#define RX_RING_BUF_SIZE        (256 * 8)

//*****************************************************************************
//
//! The size of the ring buffers used for interface between the UART and
//! telnet session (TX).
//
//*****************************************************************************
#define TX_RING_BUF_SIZE        (256 * 8)

//*****************************************************************************
//
//! Enable RFC-2217 (COM-PORT-OPTION) in the telnet server code.
//0822
//*****************************************************************************
//uint8_t RFC2217_EN=1;
#define CONFIG_RFC2217_ENABLED  1


#if origin_code

//*****************************************************************************
//
//! The peripheral on which the Port 0 UART resides.
//
//*****************************************************************************
#define S2E_PORT0_UART_PERIPH   SYSCTL_PERIPH_UART4

//*****************************************************************************
//
//! The port on which the Port 0 UART resides.
//
//*****************************************************************************
#define S2E_PORT0_UART_PORT     UART4_BASE

//*****************************************************************************
//
//! The interrupt on which the Port 0 UART resides.
//
//*****************************************************************************
#define S2E_PORT0_UART_INT      INT_UART4

//*****************************************************************************
//
//! The peripheral on which the Port 0 RX pin resides.
//
//*****************************************************************************
#define S2E_PORT0_RX_PERIPH     SYSCTL_PERIPH_GPIOK

//*****************************************************************************
//
//! The GPIO port on which the Port 0 RX pin resides.
//
//*****************************************************************************
#define S2E_PORT0_RX_PORT       GPIO_PORTK_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 RX pin resides.
//
//*****************************************************************************
#define S2E_PORT0_RX_PIN        GPIO_PIN_0

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 RX pin.
//
//*****************************************************************************
#define S2E_PORT0_RX_CONFIG     GPIO_PK0_U4RX

//*****************************************************************************
//
//! The peripheral on which the Port 0 TX pin resides.
//
//*****************************************************************************
#define S2E_PORT0_TX_PERIPH     SYSCTL_PERIPH_GPIOK

//*****************************************************************************
//
//! The GPIO port on which the Port 0 TX pin resides.
//
//*****************************************************************************
#define S2E_PORT0_TX_PORT       GPIO_PORTK_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 TX pin resides.
//
//*****************************************************************************
#define S2E_PORT0_TX_PIN        GPIO_PIN_1

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 TX pin.
//
//*****************************************************************************
#define S2E_PORT0_TX_CONFIG     GPIO_PK1_U4TX

//*****************************************************************************
//
//! The peripheral on which the Port 0 RTS pin resides.
//
//*****************************************************************************
#define S2E_PORT0_RTS_PERIPH    SYSCTL_PERIPH_GPIOK

//*****************************************************************************
//
//! The GPIO port on which the Port 0 RTS pin resides.
//
//*****************************************************************************
#define S2E_PORT0_RTS_PORT      GPIO_PORTK_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 RTS pin resides.
//
//*****************************************************************************
#define S2E_PORT0_RTS_PIN       GPIO_PIN_2

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 RTS pin.
//
//*****************************************************************************
#define S2E_PORT0_RTS_CONFIG    GPIO_PK2_U4RTS

//*****************************************************************************
//
//! The peripheral on which the Port 0 CTS pin resides.
//
//*****************************************************************************
#define S2E_PORT0_CTS_PERIPH    SYSCTL_PERIPH_GPIOK

//*****************************************************************************
//
//! The GPIO port on which the Port 0 CTS pin resides.
//
//*****************************************************************************
#define S2E_PORT0_CTS_PORT      GPIO_PORTK_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 CTS pin resides.
//
//*****************************************************************************
#define S2E_PORT0_CTS_PIN       GPIO_PIN_3

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 CTS pin.
//
//*****************************************************************************
#define S2E_PORT0_CTS_CONFIG    GPIO_PK3_U4CTS

#endif

#if revise_code

//*****************************************************************************
//
//! The peripheral on which the Port 0 UART resides.
//
//*****************************************************************************
//#define S2E_PORT0_UART_PERIPH   SYSCTL_PERIPH_UART4
//#define S2E_PORT0_UART_PERIPH   SYSCTL_PERIPH_UART1
#define S2E_PORT0_UART_PERIPH   SYSCTL_PERIPH_UART2	//Chuck
//*****************************************************************************
//
//! The port on which the Port 0 UART resides.
//
//*****************************************************************************
//#define S2E_PORT0_UART_PORT     UART4_BASE
//#define S2E_PORT0_UART_PORT     UART1_BASE
#define S2E_PORT0_UART_PORT     UART2_BASE	//Chuck
//*****************************************************************************
//
//! The interrupt on which the Port 0 UART resides.
//
//*****************************************************************************
//#define S2E_PORT0_UART_INT      INT_UART4
//#define S2E_PORT0_UART_INT      INT_UART1
#define S2E_PORT0_UART_INT      INT_UART2	//Chuck
//*****************************************************************************
//
//! The peripheral on which the Port 0 RX pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_RX_PERIPH     SYSCTL_PERIPH_GPIOK
//#define S2E_PORT0_RX_PERIPH     SYSCTL_PERIPH_GPIOB  //pin95 PB0
#define S2E_PORT0_RX_PERIPH     SYSCTL_PERIPH_GPIOD  //chuck	PD4
//*****************************************************************************
//
//! The GPIO port on which the Port 0 RX pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_RX_PORT       GPIO_PORTK_BASE
//#define S2E_PORT0_RX_PORT       GPIO_PORTB_BASE //Pin95
#define S2E_PORT0_RX_PORT       GPIO_PORTD_BASE //Pin125
//*****************************************************************************
//
//! The GPIO pin on which the Port 0 RX pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_RX_PIN        GPIO_PIN_0
#define S2E_PORT0_RX_PIN        GPIO_PIN_4

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 RX pin.
//
//*****************************************************************************
//#define S2E_PORT0_RX_CONFIG     GPIO_PK0_U4RX
//#define S2E_PORT0_RX_CONFIG     GPIO_PB0_U1RX
#define S2E_PORT0_RX_CONFIG     GPIO_PD4_U2RX
//*****************************************************************************
//
//! The peripheral on which the Port 0 TX pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_TX_PERIPH     SYSCTL_PERIPH_GPIOK
//#define S2E_PORT0_TX_PERIPH     SYSCTL_PERIPH_GPIOB   //pin96 PB1
#define S2E_PORT0_TX_PERIPH     SYSCTL_PERIPH_GPIOD   //chuck	PD5

//*****************************************************************************
//
//! The GPIO port on which the Port 0 TX pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_TX_PORT       GPIO_PORTK_BASE
//#define S2E_PORT0_TX_PORT       GPIO_PORTB_BASE
#define S2E_PORT0_TX_PORT       GPIO_PORTD_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 TX pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_TX_PIN        GPIO_PIN_1
#define S2E_PORT0_TX_PIN        GPIO_PIN_5

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 TX pin.
//
//*****************************************************************************
//#define S2E_PORT0_TX_CONFIG     GPIO_PK1_U4TX
#define S2E_PORT0_TX_CONFIG     GPIO_PD5_U2TX

//*****************************************************************************
//
//! The peripheral on which the Port 0 RTS pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_RTS_PERIPH    SYSCTL_PERIPH_GPIOK
//#define S2E_PORT0_RTS_PERIPH    SYSCTL_PERIPH_GPION // pin107 PN0
#define S2E_PORT0_RTS_PERIPH    SYSCTL_PERIPH_GPIOD // pin107 PD6

//*****************************************************************************
//
//! The GPIO port on which the Port 0 RTS pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_RTS_PORT      GPIO_PORTK_BASE
//#define S2E_PORT0_RTS_PORT      GPIO_PORTN_BASE
#define S2E_PORT0_RTS_PORT      GPIO_PORTD_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 RTS pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_RTS_PIN       GPIO_PIN_2
//#define S2E_PORT0_RTS_PIN       GPIO_PIN_0
#define S2E_PORT0_RTS_PIN       GPIO_PIN_6

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 RTS pin.
//
//*****************************************************************************
//#define S2E_PORT0_RTS_CONFIG    GPIO_PK2_U4RTS
//#define S2E_PORT0_RTS_CONFIG    GPIO_PN0_U1RTS
#define S2E_PORT0_RTS_CONFIG    GPIO_PD6_U2RTS

//*****************************************************************************
//
//! The peripheral on which the Port 0 CTS pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_CTS_PERIPH    SYSCTL_PERIPH_GPIOK
//#define S2E_PORT0_CTS_PERIPH    SYSCTL_PERIPH_GPION //pin108 PN1
#define S2E_PORT0_CTS_PERIPH    SYSCTL_PERIPH_GPIOD //pin108 PD7

//*****************************************************************************
//
//! The GPIO port on which the Port 0 CTS pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_CTS_PORT      GPIO_PORTK_BASE
//#define S2E_PORT0_CTS_PORT      GPIO_PORTN_BASE
#define S2E_PORT0_CTS_PORT      GPIO_PORTD_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 CTS pin resides.
//
//*****************************************************************************
//#define S2E_PORT0_CTS_PIN       GPIO_PIN_3
//#define S2E_PORT0_CTS_PIN       GPIO_PIN_1
#define S2E_PORT0_CTS_PIN       GPIO_PIN_7

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 CTS pin.
//
//*****************************************************************************
//#define S2E_PORT0_CTS_CONFIG    GPIO_PK3_U4CTS
#define S2E_PORT0_CTS_CONFIG    GPIO_PD7_U2CTS

//*****************************************************************************
//
//! The peripheral on which the Port 0 DCD pin resides.
//
//*****************************************************************************

#define S2E_PORT2_DCD_PERIPH    SYSCTL_PERIPH_GPIOE //pin13 PE2 instead of PN2;

//*****************************************************************************
//
//! The GPIO port on which the Port 0 DCD pin resides.
//
//*****************************************************************************

#define S2E_PORT2_DCD_PORT      GPIO_PORTE_BASE  //portN changed to Port E;

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 DCD pin resides.
//
//*****************************************************************************

#define S2E_PORT2_DCD_PIN       GPIO_PIN_2

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 DCD pin.
//
//*****************************************************************************

#define S2E_PORT2_DCD_CONFIG    GPIO_PE2_U1DCD   //portN changed to Port E;

//*****************************************************************************
//
//! The peripheral on which the Port 0 DSR pin resides.
//
//*****************************************************************************

#define S2E_PORT2_DSR_PERIPH    SYSCTL_PERIPH_GPION //pin110 PN3

//*****************************************************************************
//
//! The GPIO port on which the Port 0 DSR pin resides.
//
//*****************************************************************************

#define S2E_PORT2_DSR_PORT      GPIO_PORTN_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 DSR pin resides.
//
//*****************************************************************************

#define S2E_PORT2_DSR_PIN       GPIO_PIN_3

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 DSR pin.
//
//*****************************************************************************

#define S2E_PORT2_DSR_CONFIG    GPIO_PN3_U1DSR

//*****************************************************************************
//
//! The peripheral on which the Port 0 DTR pin resides.
//
//*****************************************************************************

#define S2E_PORT2_DTR_PERIPH    SYSCTL_PERIPH_GPION //pin111 PN4

//*****************************************************************************
//
//! The GPIO port on which the Port 0 DTR pin resides.
//
//*****************************************************************************

#define S2E_PORT2_DTR_PORT      GPIO_PORTN_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 DTR pin resides.
//
//*****************************************************************************

#define S2E_PORT2_DTR_PIN       GPIO_PIN_4

//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 DTR pin.
//
//*****************************************************************************

#define S2E_PORT2_DTR_CONFIG    GPIO_PN4_U1DTR

//*****************************************************************************
//
//! The peripheral on which the Port 0 RI pin resides.
//
//*****************************************************************************

#define S2E_PORT2_RI_PERIPH    SYSCTL_PERIPH_GPION //pin112 PN5

//*****************************************************************************
//
//! The GPIO port on which the Port 0 RI pin resides.
//
//*****************************************************************************

#define S2E_PORT2_RI_PORT      GPIO_PORTN_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 0 RI pin resides.
//
//*****************************************************************************

#define S2E_PORT2_RI_PIN       GPIO_PIN_5


//*****************************************************************************
//
//! The GPIO pin configuration for Port 0 RI pin.
//
//*****************************************************************************

#define S2E_PORT2_RI_CONFIG    GPIO_PN5_U1RI


#endif

#if origin_code
//*****************************************************************************
//
//! The peripheral on which the Port 1 UART resides.
//
//*****************************************************************************
#define S2E_PORT1_UART_PERIPH   SYSCTL_PERIPH_UART3

//*****************************************************************************
//
//! The port on which the Port 1 UART resides.
//
//*****************************************************************************
#define S2E_PORT1_UART_PORT     UART3_BASE

//*****************************************************************************
//
//! The interrupt on which the Port 1 UART resides.
//
//*****************************************************************************
#define S2E_PORT1_UART_INT      INT_UART3

//*****************************************************************************
//
//! The peripheral on which the Port 1 RX pin resides.
//
//*****************************************************************************
#define S2E_PORT1_RX_PERIPH     SYSCTL_PERIPH_GPIOA

//*****************************************************************************
//
//! The GPIO port on which the Port 1 RX pin resides.
//
//*****************************************************************************
#define S2E_PORT1_RX_PORT       GPIO_PORTA_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 1 RX pin resides.
//
//*****************************************************************************
#define S2E_PORT1_RX_PIN        GPIO_PIN_4

//*****************************************************************************
//
//! The GPIO pin configuration for Port 1 RX pin.
//
//*****************************************************************************
#define S2E_PORT1_RX_CONFIG     GPIO_PA4_U3RX

//*****************************************************************************
//
//! The peripheral on which the Port 1 TX pin resides.
//
//*****************************************************************************
#define S2E_PORT1_TX_PERIPH     SYSCTL_PERIPH_GPIOA

//*****************************************************************************
//
//! The GPIO port on which the Port 1 TX pin resides.
//
//*****************************************************************************
#define S2E_PORT1_TX_PORT       GPIO_PORTA_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 1 TX pin resides.
//
//*****************************************************************************
#define S2E_PORT1_TX_PIN        GPIO_PIN_5

//*****************************************************************************
//
//! The GPIO pin configuration for Port 1 TX pin.
//
//*****************************************************************************
#define S2E_PORT1_TX_CONFIG     GPIO_PA5_U3TX

//*****************************************************************************
//
//! The peripheral on which the Port 1 RTS pin resides.
//
//*****************************************************************************
#define S2E_PORT1_RTS_PERIPH    SYSCTL_PERIPH_GPION

//*****************************************************************************
//
//! The GPIO port on which the Port 1 RTS pin resides.
//
//*****************************************************************************
#define S2E_PORT1_RTS_PORT      GPIO_PORTN_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 1 RTS pin resides.
//
//*****************************************************************************
#define S2E_PORT1_RTS_PIN       GPIO_PIN_4

//*****************************************************************************
//
//! The GPIO pin configuration for Port 1 RTS pin.
//
//*****************************************************************************
#define S2E_PORT1_RTS_CONFIG    GPIO_PN4_U3RTS

//*****************************************************************************
//
//! The peripheral on which the Port 1 CTS pin resides.
//
//*****************************************************************************
#define S2E_PORT1_CTS_PERIPH    SYSCTL_PERIPH_GPION

//*****************************************************************************
//
//! The GPIO port on which the Port 1 CTS pin resides.
//
//*****************************************************************************
#define S2E_PORT1_CTS_PORT      GPIO_PORTN_BASE

//*****************************************************************************
//
//! The GPIO pin on which the Port 1 CTS pin resides.
//
//*****************************************************************************
#define S2E_PORT1_CTS_PIN       GPIO_PIN_5

//*****************************************************************************
//
//! The GPIO pin configuration for Port 1 CTS pin.
//
//*****************************************************************************
#define S2E_PORT1_CTS_CONFIG    GPIO_PN5_U3CTS


#endif
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
// Flags sent to the main loop indicating that it should update the IP address
// after a short delay (to allow us to send a suitable page back to the web
// browser telling it the address has changed).
//
//*****************************************************************************
extern uint8_t g_ui8UpdateRequired;
extern uint8_t Switch_LAN_to_COM;

#define UPDATE_IP_ADDR          0x01
#define UPDATE_ALL              0x02

//*****************************************************************************
//
// Prototypes for the globals exported from the configuration module, along
// with public API function prototypes.
//
//*****************************************************************************
extern tConfigParameters g_sParameters;
extern const tConfigParameters *g_psDefaultParameters;
extern const tConfigParameters *const g_psFactoryParameters;
extern void ConfigInit(void);
extern void ConfigLoadFactory(void);
extern void ConfigLoad(void);
extern void ConfigSave(void);
extern void ConfigWebInit(void);
extern void ConfigUpdateIPAddress(void);
extern void ConfigUpdateAllParameters(bool bUpdateIP);


#endif // __CONFIG_H__
