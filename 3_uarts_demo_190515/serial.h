//*****************************************************************************
//
// serial.h - Prototypes for the Serial Port Driver
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

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "utils/ringbuf.h"


//0820
//*****************************************************************************
//
// Values used to indicate various transit modes.
//
//
//*****************************************************************************

#define RS232                 ((uint8_t)1)
#define RS485_2_wires         ((uint8_t)2)
#define RS422_RS485_4_wires   ((uint8_t)3)

//*****************************************************************************
//
// Values used to indicate various parity modes for SerialSetParity and
// SerialGetParity.
//
//*****************************************************************************
#define SERIAL_PARITY_NONE      ((uint8_t)1)
#define SERIAL_PARITY_ODD       ((uint8_t)2)
#define SERIAL_PARITY_EVEN      ((uint8_t)3)
#define SERIAL_PARITY_MARK      ((uint8_t)4)
#define SERIAL_PARITY_SPACE     ((uint8_t)5)

//*****************************************************************************
//
// Values used to indicate various flow control modes for SerialSetFlowControl
// and SerialGetFlowControl.
//
//*****************************************************************************
#define SERIAL_FLOW_CONTROL_NONE                                              \
                                ((uint8_t)1)
#define SERIAL_FLOW_CONTROL_HW  ((uint8_t)3)

//*****************************************************************************
//
// Values used to indicate various WIFI modes.
//
//
//*****************************************************************************

#define AP_Mode           	     ((uint8_t)1)
#define STA_Mode		         ((uint8_t)2)
//*****************************************************************************
//0907
//data channel select
//*****************************************************************************

#define LAN_COM           	     ((uint8_t)1)
#define WLAN_COM		         ((uint8_t)2)
#define Both_COM		         ((uint8_t)3)
//*****************************************************************************
//
// Values used to indicate various WIFI NET modes.
//
//
//*****************************************************************************
#define NM11b_g_mixed_mode           	     ((uint8_t)1)
#define NM11b_only		        		 ((uint8_t)2)
#define NM11g_only		        		 ((uint8_t)3)

//*****************************************************************************
//
// Values used to indicate various WIFI Channel select.
//
//
//*****************************************************************************
#define AutoSelect          	     		 ((uint8_t)1)
#define FREQ2_412GHz		        		 ((uint8_t)2)
#define FREQ2_417GHz		        		 ((uint8_t)3)
#define FREQ2_422GHz		         	     ((uint8_t)4)
#define FREQ2_427GHz		        		 ((uint8_t)5)
#define FREQ2_432GHz		        		 ((uint8_t)6)
#define FREQ2_437GHz		           	     ((uint8_t)7)
#define FREQ2_442GHz		        		 ((uint8_t)8)
#define FREQ2_447GHz		        		 ((uint8_t)9)
#define FREQ2_452GHz		           	     ((uint8_t)10)
#define FREQ2_457GHz		        		 ((uint8_t)11)
#define FREQ2_462GHz		        		 ((uint8_t)12)
//*****************************************************************************
//
// Values used to indicate various WIFI AP Security Mode select.
//
//*****************************************************************************
#define Disable          	     	 ((uint8_t)1)
#define WEP_128		        		 ((uint8_t)2)
#define WPA_PSK          	     	 ((uint8_t)3)
#define WPA2_PSK		        	 ((uint8_t)4)

//*****************************************************************************
//
// Values used to indicate various WIFI Station Security Mode select.
//
//*****************************************************************************
#define STA_WEP		        		 ((uint8_t)1)
#define STA_WPA_PSK          	     	 ((uint8_t)2)
#define STA_WPA2_PSK		        	 ((uint8_t)4)
#define OPEN		        	 ((uint8_t)0)
//*****************************************************************************
//
// Values used to indicate various WIFI Encryption Type select.
//
//*****************************************************************************
#define TKIP		        	 ((uint8_t)1)
#define AES          	     	 ((uint8_t)2)

//*****************************************************************************
//
// Values used to indicate various WIFI DHCP select.
//
//*****************************************************************************
#define Server		        		 ((uint8_t)1)
#define DHCP_Disable          	     	 ((uint8_t)2)

//*****************************************************************************
//
// Values used to indicate various WIFI WLAN Connection Type select.
//
//*****************************************************************************
#define DHCP          	     	 ((uint8_t)1)
#define Static_IP        		 ((uint8_t)2)
//*****************************************************************************
//
// RFC2217
//
//*****************************************************************************
#define RFC2217_dis          	     	 ((uint8_t)1)
#define RFC2217_en        		 ((uint8_t)2)
#define RFC2217_SET				 ((uint8_t)8)

//*****************************************************************************
//
// modbus
//
//*****************************************************************************
#define Modbus_dis          	     	 ((uint8_t)1)
#define Modbus_en        		 	 ((uint8_t)2)//RTU-TCP
#define Modbus_RTU_RTU        		 ((uint8_t)3)

#define Modbus_SET				 		 0x14
#define Modbus_EN				 	 	 0x04//RTU-TCP
#define ModbusRTU_RTU				 	 0x10
//*****************************************************************************
//
// Values used to indicate various flow control modes for SerialSetFlowOut
// and SerialGetFlowOut.
//
//*****************************************************************************
#define SERIAL_FLOW_OUT_SET     ((uint8_t)11)
#define SERIAL_FLOW_OUT_CLEAR   ((uint8_t)12)

extern const uint32_t g_ui32UARTBase[];
extern tRingBufObject g_sRxBuf[];
extern tRingBufObject g_sTxBuf[];
extern tRingBufObject wifi_pageRxBuf;
extern tRingBufObject wifi_pageTxBuf;

//*****************************************************************************
//
// Prototypes for the Serial interface functions.
//
//*****************************************************************************
extern bool SerialSendFull(uint32_t ui32Port);
extern void SerialSend(uint32_t ui32Port, uint8_t ui8Char);
extern int32_t SerialReceive(uint32_t ui32Port);
extern uint32_t SerialReceiveAvailable(uint32_t ui32Port);
extern void SerialSetBaudRate(uint32_t ui32Port, uint32_t ui32BaudRate);
extern uint32_t SerialGetBaudRate(uint32_t ui32Port);
extern void SerialSetDataSize(uint32_t ui32Port, uint8_t ui8DataSize);
extern uint8_t SerialGetDataSize(uint32_t ui32Port);
extern void SerialSetParity(uint32_t ui32Port, uint8_t ui8Parity);
extern uint8_t SerialGetParity(uint32_t ui32Port);
extern void SerialSetStopBits(uint32_t ui32Port, uint8_t ui8StopBits);
extern uint8_t SerialGetStopBits(uint32_t ui32Port);
extern void SerialSetFlowControl(uint32_t ui32Port,
                                 uint8_t ui8FlowControl);
extern uint8_t SerialGetFlowControl(uint32_t ui32Port);
extern void SerialPurgeData(uint32_t ui32Port,
                            uint8_t ui8PurgeCommand);
extern void SerialSetDefault(uint32_t ui32Port);
extern void SerialSetFactory(uint32_t ui32Port);
extern void SerialSetCurrent(uint32_t ui32Port);
extern void SerialInit(void);

extern void SerialSetTransitMode(uint32_t ui32Port,uint32_t ui8TransmitMode);
extern void SerialSetWiFiCMDMode(uint32_t ui32Port, uint8_t ui8WiFiCMDMode);
extern uint8_t SerialGetTransitMode(uint32_t ui32Port);
extern void ConfigUpdateIPAddress(void);
extern void ConfigUpdateIPAddress(void);
extern void SerialGetWorkMode(uint32_t ui32Port);//chris_web
extern void SerialGetMaxTCP(uint32_t ui32Port);//chris_web

//wifi functions
//chris_0820
extern uint8_t GetWifiMode(void);
extern uint8_t WIFIGetAPSecMode(void);
extern uint8_t WIFIGetAPNetworkMode(void);
extern uint8_t WIFIGetWIFIChannelMode(void);
extern uint8_t WIFIGetWIFIDHCPType(void);
extern uint8_t WIFIGetSTASecMode(void);
extern uint8_t WIFIGetSTAEType(void);
extern uint8_t WIFIGetSTAConnType(void);
extern uint8_t GetRFC2217Value(void);//0822
extern uint8_t GetModbusValue(void);
extern uint8_t GetNetChannel(void);
extern uint8_t WiFiData_ProtocolGet(void);


#endif // __SERIAL_H__
