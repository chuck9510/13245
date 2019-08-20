//*****************************************************************************
//
// modbus.h - Definitions for the modbus command interface.
//
// Copyright (c) 2014 Texas Instruments Incorporated.  All rights reserved.
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
// Note:  This is a deriviate of the RDK-S2E firmware (telnet.h) modified for
// support of Modbus functions.
//
//*****************************************************************************

#ifndef __MODBUS_H__
#define __MODBUS_H__

//*****************************************************************************
//
// API Function prototypes
//
//*****************************************************************************
extern void ModbusInit(void);
extern void ModbusListen(unsigned short usTelnetPort);
extern void ModbusClose(void);
extern void ModbusHandler(void);
extern unsigned short ModbusGetLocalPort(void);
extern unsigned short ModbusGetRemotePort(void);
extern void ModbusNotifyLinkStatus(bool bLinkStatusUp);
#endif // __MODBUS_H__
