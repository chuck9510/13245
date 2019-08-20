//*****************************************************************************
//
// serial.c - Serial port driver for S2E Module.
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

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/ringbuf.h"
#include "config.h"
#include "serial.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "user_define.h"
#include <string.h>
//*****************************************************************************
//
// External References.
//
//*****************************************************************************
extern uint32_t g_ui32SysClock;

extern _Bool WiFiCMD_Mode_FLag;  //1, wificmd mode; 0, normal mode;
//extern _Bool Switch_WiFi_to_COM; // 1,wifi to com; 0, Lan to com;
//extern uint8_t ResponseFromWiFi[256]; // save the response data from wifi module;
extern uint8_t SendCommBufferToWiFi[20]; // save the command to be sent to wifi module;

//*****************************************************************************
//
//! \addtogroup serial_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! The buffer used to hold characters received from the serial Port0.
//
//*****************************************************************************
static uint8_t g_pui8RX0Buffer[RX_RING_BUF_SIZE];

//*****************************************************************************
//
//! The buffer used to hold characters to be sent to the serial Port0.
//
//*****************************************************************************
static uint8_t g_pui8TX0Buffer[TX_RING_BUF_SIZE];

//*****************************************************************************
//
//! The buffer used to hold characters received from the serial Port1.
//
//*****************************************************************************
uint8_t g_pui8RX1Buffer[RX_RING_BUF_SIZE];

//*****************************************************************************
//
//! The buffer used to hold characters to be sent to the serial Port1.
//
//*****************************************************************************
uint8_t g_pui8TX1Buffer[TX_RING_BUF_SIZE];

//*****************************************************************************
//
//! The buffer used to hold characters received from the serial Port2.
//
//*****************************************************************************
uint8_t g_pui8RX2Buffer[RX_RING_BUF_SIZE];

//*****************************************************************************
//
//! The buffer used to hold characters to be sent to the serial Port2.
//
//*****************************************************************************
uint8_t g_pui8TX2Buffer[TX_RING_BUF_SIZE];

//*****************************************************************************
//
//! The ring buffers used to hold characters received from the serial ports.
//
//*****************************************************************************
tRingBufObject g_sRxBuf[MAX_S2E_PORTS];

//*****************************************************************************
//
//! The ring buffers used to hold characters to be sent to the serial ports.
//
//*****************************************************************************
tRingBufObject g_sTxBuf[MAX_S2E_PORTS];
//*****************************************************************************
//
//! The ring buffers for wifi page access.
//
//*****************************************************************************
tRingBufObject wifi_pageRxBuf;
tRingBufObject wifi_pageTxBuf;
static uint8_t wifi_pageui8RXBuffer[RX_RING_BUF_SIZE];
static uint8_t wifi_pageui8TXBuffer[TX_RING_BUF_SIZE];

//*****************************************************************************
//
//! The ring buffer for com0 to LAN, used for COM-LAN and COM-WLAN both;
//
//*****************************************************************************
tRingBufObject Data_From_COM_To_LAN;
static uint8_t Data_From_COM_To_LAN_Buffer[RX_RING_BUF_SIZE];

//*****************************************************************************
//
//! The ring buffer for com1 to LAN, used for COM-LAN and COM-WLAN both;
//
//*****************************************************************************
tRingBufObject Data_From_COM1_To_LAN;
static uint8_t Data_From_COM1_To_LAN_Buffer[RX_RING_BUF_SIZE];

//*****************************************************************************
//
//! The ring buffer for com2 to LAN, used for COM-LAN and COM-WLAN both;
//
//*****************************************************************************
tRingBufObject Data_From_COM2_To_LAN;
static uint8_t Data_From_COM2_To_LAN_Buffer[RX_RING_BUF_SIZE];


//*****************************************************************************
//
//! The base address for the UART associated with a port.
//
//*****************************************************************************
const uint32_t g_ui32UARTBase[MAX_S2E_PORTS] =
{
		S2E_PORT0_UART_PORT,
		S2E_PORT1_UART_PORT,
		S2E_PORT2_UART_PORT
};

//*****************************************************************************
//
//! The interrupt for the UART associated with a port.
//
//*****************************************************************************
static const uint32_t g_ui32UARTInterrupt[MAX_S2E_PORTS] =
{
		S2E_PORT0_UART_INT,
		S2E_PORT1_UART_INT,
		S2E_PORT2_UART_INT
};

//*****************************************************************************
//
//! The current baud rate setting of the serial port
//
//*****************************************************************************
static uint32_t g_ui32CurrentBaudRate[MAX_S2E_PORTS] =
{
		0,
		0,
		0
};

//*****************************************************************************
//
//! Enables transmitting and receiving.
//!
//! \param ui32Port is the UART port number to be accessed.
//!
//! Sets the UARTEN, and RXE bits, and enables the transmit and receive
//! FIFOs.
//!
//! \return None.
//
//*****************************************************************************
static void
SerialUARTEnable(uint32_t ui32Port)
{
	//
	// Enable the FIFO.
	//
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_LCRH) |= UART_LCRH_FEN;

	//
	// Enable RX, TX, and the UART.
	//
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_CTL) |= (UART_CTL_UARTEN |
			UART_CTL_RXE |
			UART_CTL_TXE);
}

//*****************************************************************************
//
//! Checks the availability of the serial port output buffer.
//!
//! \param ui32Port is the UART port number to be accessed.
//!
//! This function checks to see if there is room on the UART transmit buffer
//! for additional data.
//!
//! \return Returns \b true if the transmit buffer is full, \b false
//! otherwise.
//
//*****************************************************************************
bool
SerialSendFull(uint32_t ui32Port)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Return the number of bytes available in the tx ring buffer.
	//
	return(RingBufFull(&g_sTxBuf[ui32Port]));
}

//*****************************************************************************
//
//! Sends a character to the UART.
//!
//! \param ui32Port is the UART port number to be accessed.
//! \param ui8Char is the character to be sent.
//!
//! This function sends a character to the UART.  The character will either be
//! directly written into the UART FIFO or into the UART transmit buffer, as
//! appropriate.
//!
//! \return None.
//
//*****************************************************************************
void
SerialSend(uint32_t ui32Port, uint8_t ui8Char)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Disable the UART transmit interrupt while determining how to handle this
	// character.  Failure to do so could result in the loss of this character,
	// or stalled output due to this character being placed into the UART
	// transmit buffer but never transferred out into the UART FIFO.
	//
	UARTIntDisable(g_ui32UARTBase[ui32Port], UART_INT_TX);

	//
	// See if the transmit buffer is empty and there is space in the FIFO.
	//
	if(RingBufEmpty(&g_sTxBuf[ui32Port]) &&
			(UARTSpaceAvail(g_ui32UARTBase[ui32Port])))
	{
		//
		// Write this character directly into the FIFO.
		//
		RS485_TxEnable(ui32Port);
		UARTCharPut(g_ui32UARTBase[ui32Port], ui8Char);
		//        UARTprintf("t%02X",ui8Char);
		/*
		  if((ui8Char==0x0A)||(ui8Char==0x0D))
		  UARTprintf("t%02X",ui8Char);
		  else UARTprintf("%c",ui8Char);
		 */
		//        UARTprintf("t%c",ui8Char);
		RS485_TxDisable(ui32Port);
	}

	//
	// See if there is room in the transmit buffer.
	//
	else if(!RingBufFull(&g_sTxBuf[ui32Port]))
	{
		//
		// Put this character into the transmit buffer.
		//

		RingBufWriteOne(&g_sTxBuf[ui32Port], ui8Char);
	}
	if(ui32Port == COM_Port)
		txdata_sending = 2;
	else if(ui32Port == WiFi_Port)
	{
		WiFi_Status_Retval = WiFi_Datain;
		WiFiLEDflashtime = 2;
	}
	//
	// Enable the UART transmit interrupt.
	//
	UARTIntEnable(g_ui32UARTBase[ui32Port], UART_INT_TX);
}

//*****************************************************************************
//
//! Receives a character from the UART.
//!
//! \param ui32Port is the UART port number to be accessed.
//!
//! This function receives a character from the relevant port's UART buffer.
//!
//! \return Returns -1 if no data is available or the oldest character held in
//! the receive ring buffer.
//
//*****************************************************************************
int32_t
SerialReceive(uint32_t ui32Port)
{
	uint32_t ui32Data;

	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// See if the receive buffer is empty and there is space in the FIFO.
	//
	if(RingBufEmpty(&g_sRxBuf[ui32Port]))
	{
		//
		// Return -1 (EOF) to indicate no data available.
		//
		return(-1);
	}

	//
	// Read a single character.
	//
	ui32Data = (long)RingBufReadOne(&g_sRxBuf[ui32Port]);

	//
	// Return the data that was read.
	//
	if(ui32Port == COM_Port)
		rxdata_receiving = 2;
	else if(ui32Port == WiFi_Port)
	{
		WiFi_Status_Retval = WiFi_Datain;
		WiFiLEDflashtime = 2;
	}

	return(ui32Data);
}

//*****************************************************************************
//
//! Returns number of characters available in the serial ring buffer.
//!
//! \param ui32Port is the UART port number to be accessed.
//!
//! This function will return the number of characters available in the
//! serial ring buffer.
//!
//! \return The number of characters available in the ring buffer..
//
//*****************************************************************************
uint32_t
SerialReceiveAvailable(uint32_t ui32Port)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Return the value.
	//
	return(RingBufUsed(&g_sRxBuf[ui32Port]));
}

//*****************************************************************************
//
//! Configures the serial port baud rate.
//!
//! \param ui32Port is the serial port number to be accessed.
//! \param ui32BaudRate is the new baud rate for the serial port.
//!
//! This function configures the serial port's baud rate.  The current
//! configuration for the serial port will be read.  The baud rate will be
//! modified, and the port will be reconfigured.
//!
//! \return None.
//
//*****************************************************************************
void
SerialSetBaudRate(uint32_t ui32Port, uint32_t ui32BaudRate)
{
	uint32_t ui32Div;

	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);
	ASSERT(ui32BaudRate != 0);

	//
	// Save the baud rate for future reference.
	//
	g_ui32CurrentBaudRate[ui32Port] = ui32BaudRate;

	//
	// Get and check the clock use by the UART.
	//
	ASSERT(g_ui32SysClock >= (ui32BaudRate * 16));

	//
	// Stop the UART.
	//
	UARTDisable(g_ui32UARTBase[ui32Port]);
	// set HSE reg;
	//   HWREG(g_ui32UARTBase[ui32Port] + UART_O_CTL) = HWREG(g_ui32UARTBase[ui32Port] + UART_O_CTL)| UART_CTL_HSE;
	//   HWREG(g_ui32UARTBase[ui32Port] + UART_O_LCRH) = HWREG(g_ui32UARTBase[ui32Port] + UART_O_LCRH);

	//
	// Compute the fractional baud rate divider.
	//
	ui32Div = (((g_ui32SysClock * 8) / ui32BaudRate) + 1) / 2; // change 8 to 4;
	//    UARTprintf("div is :%d \n",ui32Div);


	//
	// Set the baud rate.
	//
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_IBRD) = ui32Div / 64;
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_FBRD) = ui32Div % 64;
	//    UARTprintf("div/64 is :%d \n",ui32Div/64);
	//    UARTprintf("div%64 is :%d \n",ui32Div%64);



	//
	// Clear the flags register.
	//
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_FR) = 0;

	//
	// Start the UART.
	//
	SerialUARTEnable(ui32Port);
}

//*****************************************************************************
//
//! Queries the serial port baud rate.
//!
//! \param ui32Port is the serial port number to be accessed.
//!
//! This function will read the UART configuration and return the currently
//! configured baud rate for the selected port.
//!
//! \return The current baud rate of the serial port.
//
//*****************************************************************************
uint32_t
SerialGetBaudRate(uint32_t ui32Port)
{
	uint32_t ui32CurrentBaudRate, ui32CurrentConfig, ui32Dif, ui32Temp;

	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Get the current configuration of the UART.
	//
	UARTConfigGetExpClk(g_ui32UARTBase[ui32Port], g_ui32SysClock,
			&ui32CurrentBaudRate, &ui32CurrentConfig);

	//
	// Calculate the difference between the reported baud rate and the
	// stored nominal baud rate.
	//
	if(ui32CurrentBaudRate > g_ui32CurrentBaudRate[ui32Port])
	{
		ui32Dif = ui32CurrentBaudRate - g_ui32CurrentBaudRate[ui32Port];
	}
	else
	{
		ui32Dif = g_ui32CurrentBaudRate[ui32Port] - ui32CurrentBaudRate;
	}

	//
	// Calculate the 1% value of nominal baud rate.
	//
	ui32Temp = g_ui32CurrentBaudRate[ui32Port] / 100;

	//
	// If the difference between calculated and nominal is > 1%, report the
	// calculated rate.  Otherwise, report the nominal rate.
	//
	if(ui32Dif > ui32Temp)
	{
		//   	UARTprintf("if >1%,return cal Baudrate: 0x%x \n",ui32CurrentBaudRate);
		return(ui32CurrentBaudRate);
	}

	//
	// Return the current serial port baud rate.
	//
	//   UARTprintf("if <1%,return globe Baudrate: 0x%x \n",g_ui32CurrentBaudRate[ui32Port]);
	return(g_ui32CurrentBaudRate[ui32Port]);
}

//*****************************************************************************
//
//! Configures the serial port data size.
//!
//! \param ui32Port is the serial port number to be accessed.
//! \param ui8DataSize is the new data size for the serial port.
//!
//! This function configures the serial port's data size.  The current
//! configuration for the serial port will be read.  The data size will be
//! modified, and the port will be reconfigured.
//!
//! \return None.
//
//*****************************************************************************
void
SerialSetDataSize(uint32_t ui32Port, uint8_t ui8DataSize)
{
	uint32_t ui32CurrentBaudRate, ui32CurrentConfig, ui32NewConfig;

	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);
	ASSERT((ui8DataSize >= 5) && (ui8DataSize <= 8));

	//
	// Stop the UART.
	//
	UARTDisable(g_ui32UARTBase[ui32Port]);

	//
	// Get the current configuration of the UART.
	//
	UARTConfigGetExpClk(g_ui32UARTBase[ui32Port], g_ui32SysClock,
			&ui32CurrentBaudRate, &ui32CurrentConfig);

	//
	// Update the configuration with a new data length.
	//
	switch(ui8DataSize)
	{
	case 5:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_WLEN_MASK);
		ui32NewConfig |= UART_CONFIG_WLEN_5;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8DataSize = ui8DataSize;
		break;
	}

	case 6:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_WLEN_MASK);
		ui32NewConfig |= UART_CONFIG_WLEN_6;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8DataSize = ui8DataSize;
		break;
	}

	case 7:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_WLEN_MASK);
		ui32NewConfig |= UART_CONFIG_WLEN_7;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8DataSize = ui8DataSize;
		break;
	}

	case 8:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_WLEN_MASK);
		ui32NewConfig |= UART_CONFIG_WLEN_8;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8DataSize = ui8DataSize;
		break;
	}

	default:
	{
		ui32NewConfig = ui32CurrentConfig;
		break;
	}
	}

	//
	// Set parity, data length, and number of stop bits.
	//
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_LCRH) = ui32NewConfig;

	//
	// Clear the flags register.
	//
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_FR) = 0;

	//
	// Start the UART.
	//
	SerialUARTEnable(ui32Port);
}

//*****************************************************************************
//
//! Queries the serial port data size.
//!
//! \param ui32Port is the serial port number to be accessed.
//!
//! This function will read the UART configuration and return the currently
//! configured data size for the selected port.
//!
//! \return None.
//
//*****************************************************************************
uint8_t
SerialGetDataSize(uint32_t ui32Port)
{
	uint32_t ui32CurrentBaudRate, ui32CurrentConfig;
	uint8_t ui8CurrentDataSize;

	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Get the current configuration of the UART.
	//
	UARTConfigGetExpClk(g_ui32UARTBase[ui32Port], g_ui32SysClock,
			&ui32CurrentBaudRate, &ui32CurrentConfig);

	//
	// Determine the current data size.
	//
	switch(ui32CurrentConfig & UART_CONFIG_WLEN_MASK)
	{
	case UART_CONFIG_WLEN_5:
	{
		ui8CurrentDataSize = 5;
		break;
	}

	case UART_CONFIG_WLEN_6:
	{
		ui8CurrentDataSize = 6;
		break;
	}

	case UART_CONFIG_WLEN_7:
	{
		ui8CurrentDataSize = 7;
		break;
	}

	case UART_CONFIG_WLEN_8:
	{
		ui8CurrentDataSize = 8;
		break;
	}

	default:
	{
		ui8CurrentDataSize = 0;
		break;
	}
	}

	//
	// Return the current data size.
	//
	return(ui8CurrentDataSize);
}

//*****************************************************************************
//
//! Configures the serial port parity.
//!
//! \param ui32Port is the serial port number to be accessed.
//! \param ui8Parity is the new parity for the serial port.
//!
//! This function configures the serial port's parity.  The current
//! configuration for the serial port will be read.  The parity will be
//! modified, and the port will be reconfigured.
//!
//! \return None.
//
//*****************************************************************************
void
SerialSetParity(uint32_t ui32Port, uint8_t ui8Parity)
{
	uint32_t ui32CurrentBaudRate, ui32CurrentConfig, ui32NewConfig;

	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);
	ASSERT((ui8Parity == SERIAL_PARITY_NONE) ||
			(ui8Parity == SERIAL_PARITY_ODD) ||
			(ui8Parity == SERIAL_PARITY_EVEN) ||
			(ui8Parity == SERIAL_PARITY_MARK) ||
			(ui8Parity == SERIAL_PARITY_SPACE));

	//
	// Stop the UART.
	//
	UARTDisable(g_ui32UARTBase[ui32Port]);

	//
	// Get the current configuration of the UART.
	//
	UARTConfigGetExpClk(g_ui32UARTBase[ui32Port], g_ui32SysClock,
			&ui32CurrentBaudRate, &ui32CurrentConfig);

	//
	// Update the configuration with a new parity.
	//
	switch(ui8Parity)
	{
	case SERIAL_PARITY_NONE:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_PAR_MASK);
		ui32NewConfig |= UART_CONFIG_PAR_NONE;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8Parity = ui8Parity;
		break;
	}

	case SERIAL_PARITY_ODD:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_PAR_MASK);
		ui32NewConfig |= UART_CONFIG_PAR_ODD;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8Parity = ui8Parity;
		break;
	}

	case SERIAL_PARITY_EVEN:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_PAR_MASK);
		ui32NewConfig |= UART_CONFIG_PAR_EVEN;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8Parity = ui8Parity;
		break;
	}

	case SERIAL_PARITY_MARK:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_PAR_MASK);
		ui32NewConfig |= UART_CONFIG_PAR_ONE;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8Parity = ui8Parity;
		break;
	}

	case SERIAL_PARITY_SPACE:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_PAR_MASK);
		ui32NewConfig |= UART_CONFIG_PAR_ZERO;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8Parity = ui8Parity;
		break;
	}

	default:
	{
		ui32NewConfig = ui32CurrentConfig;
		break;
	}
	}

	//
	// Set parity, data length, and number of stop bits.
	//
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_LCRH) = ui32NewConfig;

	//
	// Clear the flags register.
	//
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_FR) = 0;

	//
	// Start the UART.
	//
	SerialUARTEnable(ui32Port);
}

//*****************************************************************************
//
//! Queries the serial port parity.
//!
//! \param ui32Port is the serial port number to be accessed.
//!
//! This function will read the UART configuration and return the currently
//! configured parity for the selected port.
//!
//! \return Returns the current parity setting for the port.  This will be one
//! of \b SERIAL_PARITY_NONE, \b SERIAL_PARITY_ODD, \b SERIAL_PARITY_EVEN,
//! \b SERIAL_PARITY_MARK, or \b SERIAL_PARITY_SPACE.
//
//*****************************************************************************
uint8_t
SerialGetParity(uint32_t ui32Port)
{
	uint32_t ui32CurrentBaudRate, ui32CurrentConfig;
	uint8_t ui8CurrentParity;

	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Get the current configuration of the UART.
	//
	UARTConfigGetExpClk(g_ui32UARTBase[ui32Port], g_ui32SysClock,
			&ui32CurrentBaudRate, &ui32CurrentConfig);

	//
	// Determine the current data size.
	//
	switch(ui32CurrentConfig & UART_CONFIG_PAR_MASK)
	{
	case UART_CONFIG_PAR_NONE:
	{
		ui8CurrentParity = SERIAL_PARITY_NONE;
		break;
	}

	case UART_CONFIG_PAR_ODD:
	{
		ui8CurrentParity = SERIAL_PARITY_ODD;
		break;
	}

	case UART_CONFIG_PAR_EVEN:
	{
		ui8CurrentParity = SERIAL_PARITY_EVEN;
		break;
	}

	case UART_CONFIG_PAR_ONE:
	{
		ui8CurrentParity = SERIAL_PARITY_MARK;
		break;
	}

	case UART_CONFIG_PAR_ZERO:
	{
		ui8CurrentParity = SERIAL_PARITY_SPACE;
		break;
	}

	default:
	{
		ui8CurrentParity = SERIAL_PARITY_NONE;
		break;
	}
	}

	//
	// Return the current data size.
	//
	return(ui8CurrentParity);
}

//*****************************************************************************
//
//! Configures the serial port stop bits.
//!
//! \param ui32Port is the serial port number to be accessed.
//! \param ui8StopBits is the new stop bits for the serial port.
//!
//! This function configures the serial port's stop bits.  The current
//! configuration for the serial port will be read.  The stop bits will be
//! modified, and the port will be reconfigured.
//!
//! \return None.
//
//*****************************************************************************
void
SerialSetStopBits(uint32_t ui32Port, uint8_t ui8StopBits)
{
	uint32_t ui32CurrentBaudRate, ui32CurrentConfig, ui32NewConfig;

	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);
	ASSERT((ui8StopBits >= 1) && (ui8StopBits <= 2));

	//
	// Stop the UART.
	//
	UARTDisable(g_ui32UARTBase[ui32Port]);

	//
	// Get the current configuration of the UART.
	//
	UARTConfigGetExpClk(g_ui32UARTBase[ui32Port], g_ui32SysClock,
			&ui32CurrentBaudRate, &ui32CurrentConfig);

	//
	// Update the configuration with a new stop bits.
	//
	switch(ui8StopBits)
	{
	case 1:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_STOP_MASK);
		ui32NewConfig |= UART_CONFIG_STOP_ONE;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8StopBits = ui8StopBits;
		break;
	}

	case 2:
	{
		ui32NewConfig = (ui32CurrentConfig & ~UART_CONFIG_STOP_MASK);
		ui32NewConfig |= UART_CONFIG_STOP_TWO;
		if(ui32Port == 0)
			g_sParameters.sPort[ui32Port].ui8StopBits = ui8StopBits;
		break;
	}

	default:
	{
		ui32NewConfig = ui32CurrentConfig;
		break;
	}
	}

	//
	// Set parity, data length, and number of stop bits.
	//
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_LCRH) = ui32NewConfig;

	//
	// Clear the flags register.
	//
	HWREG(g_ui32UARTBase[ui32Port] + UART_O_FR) = 0;

	//
	// Start the UART.
	//
	SerialUARTEnable(ui32Port);
}

//*****************************************************************************
//
//! Queries the serial port stop bits.
//!
//! \param ui32Port is the serial port number to be accessed.
//!
//! This function will read the UART configuration and return the currently
//! configured stop bits for the selected port.
//!
//! \return None.
//
//*****************************************************************************
uint8_t
SerialGetStopBits(uint32_t ui32Port)
{
	uint32_t ui32CurrentBaudRate, ui32CurrentConfig;
	uint8_t ui8CurrentStopBits;

	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Get the current configuration of the UART.
	//
	UARTConfigGetExpClk(g_ui32UARTBase[ui32Port], g_ui32SysClock,
			&ui32CurrentBaudRate, &ui32CurrentConfig);

	//
	// Determine the current data size.
	//
	switch(ui32CurrentConfig & UART_CONFIG_STOP_MASK)
	{
	case UART_CONFIG_STOP_ONE:
	{
		ui8CurrentStopBits = 1;
		break;
	}

	case UART_CONFIG_STOP_TWO:
	{
		ui8CurrentStopBits = 2;
		break;
	}

	default:
	{
		ui8CurrentStopBits = 0;
		break;
	}
	}

	//
	// Return the current data size.
	//
	return(ui8CurrentStopBits);
}

//*****************************************************************************
//
//! Configures the serial port flow control option.
//!
//! \param ui32Port is the UART port number to be accessed.
//! \param ui8FlowControl is the new flow control setting for the serial port.
//!
//! This function configures the serial port's flow control.  This function
//! will enable/disable the hardware flow control setting in the UART based on
//! the value of the flow control setting.
//!
//! \return None.
//
//*****************************************************************************
void
SerialSetFlowControl(uint32_t ui32Port, uint8_t ui8FlowControl)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);
	ASSERT((ui8FlowControl == SERIAL_FLOW_CONTROL_NONE) ||
			(ui8FlowControl == SERIAL_FLOW_CONTROL_HW));

	//
	// Save the new flow control setting.
	//
	if(ui32Port == 0)
		g_sParameters.sPort[ui32Port].ui8FlowControl = ui8FlowControl;

	//
	// Enable flow control in the UART.
	//

	//    if(g_sParameters.sPort[ui32Port].ui8FlowControl == SERIAL_FLOW_CONTROL_HW)
	if(ui8FlowControl == SERIAL_FLOW_CONTROL_HW)
	{
		HWREG(g_ui32UARTBase[ui32Port] + UART_O_CTL) |=
				(UART_FLOWCONTROL_TX | UART_FLOWCONTROL_RX);

		//
		// Debug. Remove later.
		//

		if(ui32Port == 0)
		{
			GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_0, //from 4 to 0; UR1_RTS
					GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
		}
		else
		{
			GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_2, //UR4_RTS
					GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
		}

	}

	//
	// Disable flow control in the UART.
	//
	else
	{
		HWREG(g_ui32UARTBase[ui32Port] + UART_O_CTL) &=
				~(UART_FLOWCONTROL_TX | UART_FLOWCONTROL_RX);

		//
		// Debug. Remove later.
		//

		if(ui32Port == 0)
		{
			GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_0, //from 4 to 0;
					GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		}
		else
		{
			GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_2,
					GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		}

	}
}

//*****************************************************************************
//
//! Queries the serial port flow control.
//!
//! \param ui32Port is the serial port number to be accessed.
//!
//! This function will return the currently configured flow control for
//! the selected port.
//!
//! \return None.
//
//*****************************************************************************
uint8_t
SerialGetFlowControl(uint32_t ui32Port)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Return the current flow control.
	//
	return(g_sParameters.sPort[ui32Port].ui8FlowControl);
}


//*****************************************************************************
//
//! Queries the serial port transit mode.
//!
//! \param ui32Port is the serial port number to be accessed.
//!
//! This function will return the currently configured transit mode for
//! the selected port.
//!
//! \return None.
//
//*****************************************************************************
uint8_t
SerialGetTransitMode(uint32_t ui32Port)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Return the current flow control.
	//
	//	UARTprintf("CGI return get value after set: %d !\n",g_sParameters.sPort[ui32Port].uiTransitMode);
	return(g_sParameters.sPort[ui32Port].uiTransitMode);
}

//****************************************************
//chris_0820
//wifi mode
uint8_t
GetWifiMode(void)
{
	return(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode);
}
//0907
//wifi mode
uint8_t
GetNetChannel(void)
{
	return(g_sParameters.datachansel);
}
//****************************************************
//chris_0820
//AP security mode
uint8_t
WIFIGetAPSecMode(void)
{
	return(g_sParameters.sWiFiModule.sWiFiAPSet.ucSecurityMode);
}
//****************************************************
//chris_0820
//AP network mode
uint8_t
WIFIGetAPNetworkMode(void)
{
	return( g_sParameters.sWiFiModule.sWiFiAPSet.ucAPNetMode);
}
//****************************************************
//chris_0820
//AP channel
uint8_t
WIFIGetWIFIChannelMode(void)
{
	return(g_sParameters.sWiFiModule.sWiFiAPSet.ucFrequency_Channel);
}
//****************************************************
//chris_0820
//DHCP Type
uint8_t
WIFIGetWIFIDHCPType(void)
{
	return(g_sParameters.sWiFiModule.sWiFiAPSet.ucDHCPType);
}
//****************************************************
//chris_0820
//STA Security Mode
uint8_t
WIFIGetSTASecMode(void)
{
	return(g_sParameters.sWiFiModule.sWiFiStaionSet.ucSecurityMode);
}
//****************************************************
//chris_0820
//STA Encryption Type
uint8_t
WIFIGetSTAEType(void)
{
	return( g_sParameters.sWiFiModule.sWiFiStaionSet.ucEncryptionType);
}
//****************************************************
//chris_0820
//STA WLAN Connection Type
uint8_t
WIFIGetSTAConnType(void)
{
	return( g_sParameters.sWiFiModule.sWiFiStaionSet.ucStationConnType);
}
//****************************************************
//0822
//RFC2217
uint8_t
GetRFC2217Value(void)
{
	return((g_sParameters.sPort->ui8Flags & RFC2217_SET) ? RFC2217_en : RFC2217_dis);
}
//****************************************************
//0822
//modbus
uint8_t
GetModbusValue(void)
{
	if((g_sParameters.sPort->ui8Flags & Modbus_SET) == 0)
		return Modbus_dis;
	else if((g_sParameters.sPort->ui8Flags & Modbus_SET) == Modbus_EN)
		return Modbus_en;
	else if((g_sParameters.sPort->ui8Flags & Modbus_SET) == ModbusRTU_RTU)
		return Modbus_RTU_RTU;
	else return Modbus_dis;
}
//****************************************************
uint8_t
WiFiData_ProtocolGet(void)
{
	uint8_t pValue;
	g_sParameters.sWiFiModule.sWiFiModSel.WiFiTransPcolFlag	&= ~0x07;
	if(strstr((const char *)ResponseFromWiFi_IP,"CLIENT") !=NULL)
	{
		//UARTprintf("**CLIENT**");
		g_sParameters.sWiFiModule.sWiFiModSel.WiFiTransPcolFlag |= WiFi_CLIENT;
		pValue = PORT_TELNET_CLIENT;
	}
	else if(strstr((const char *)ResponseFromWiFi_IP,"UDP") !=NULL)
	{
		//UARTprintf("**UDP**");
		g_sParameters.sWiFiModule.sWiFiModSel.WiFiTransPcolFlag |= WiFi_UDP;
		pValue = PORT_UDP;
	}
	else
	{
		//UARTprintf("**SERVER**");
		g_sParameters.sWiFiModule.sWiFiModSel.WiFiTransPcolFlag |= WiFi_SERVER;
		pValue = PORT_TELNET_SERVER;
	}
	return(pValue);
}

//****************************************************
//chris_web
void
SerialGetWorkMode(uint32_t ui32Port)
{

}

//****************************************************
//chris_web

void SerialGetMaxTCP(uint32_t ui32Port)
{

}


//*****************************************************************************
//
//! Purges the serial port data queue(s).
//!
//! \param ui32Port is the serial port number to be accessed.
//! \param ui8PurgeCommand is the command indicating which queue's to purge.
//!
//! This function will purge data from the tx, rx, or both serial port queues.
//!
//! \return None.
//
//*****************************************************************************
void
SerialPurgeData(uint32_t ui32Port, uint8_t ui8PurgeCommand)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);
	ASSERT((ui8PurgeCommand >= 1) && (ui8PurgeCommand <= 3));

	//
	// Disable the UART.
	//
	UARTDisable(g_ui32UARTBase[ui32Port]);

	//
	// Purge the receive data if requested.
	//
	if(ui8PurgeCommand & 0x01)
	{
		RingBufFlush(&g_sRxBuf[ui32Port]);
	}

	//
	// Purge the transmit data if requested.
	//
	if(ui8PurgeCommand & 0x02)
	{
		RingBufFlush(&g_sTxBuf[ui32Port]);
	}

	//
	// Re-enable the UART.
	//
	SerialUARTEnable(ui32Port);
}



//*****************************************************************************
//

//! This function sets the WiFi serial port to match WiFI Module UR default configuration. 9600
//!
//! \return None.
//
//*****************************************************************************
//static void
//_SerialSetConfig(uint32_t ui32Port, const tPortParameters *psPort)
void _UR4_SerialSetConfig(uint32_t UR4_Baudrate)
{
	//
	// Disable interrupts.
	//
	IntDisable(g_ui32UARTInterrupt[WiFi_Port]);

	//
	// Set the baud rate.
	//
	SerialSetBaudRate(WiFi_Port, UR4_Baudrate);


	//
	// Set the data size.
	//
	SerialSetDataSize(WiFi_Port, 8); // data size 8;

	//
	// Set the parity.
	//
	SerialSetParity(WiFi_Port, SERIAL_PARITY_NONE); // SERIAL_PARITY_EVEN,SERIAL_PARITY_NONE;

	//
	// Set the stop bits.
	//
	SerialSetStopBits(WiFi_Port, 1);//1, stop bit;

	//
	// Set the flow control.
	//
	SerialSetFlowControl(WiFi_Port, SERIAL_FLOW_CONTROL_HW);// SERIAL_FLOW_CONTROL_HW; SERIAL_FLOW_CONTROL_NONE;

	//
	// Purge the Serial Tx/Rx Ring Buffers.
	//
	SerialPurgeData(WiFi_Port, 0x03);

	//
	// (Re)enable the UART transmit and receive interrupts.
	//
	UARTIntEnable(g_ui32UARTBase[WiFi_Port],
			(UART_INT_RX | UART_INT_RT | UART_INT_TX));
	IntEnable(g_ui32UARTInterrupt[WiFi_Port]);
}


//*****************************************************************************
//
//! \internal
//!
//! Configures the serial port to a default setup.
//!
//! \param ui32Port is the UART port number to be accessed.
//!
//! This function resets the serial port to a default configuration.
//!
//! \return None.
//
//*****************************************************************************
//static void
//_SerialSetConfig(uint32_t ui32Port, const tPortParameters *psPort)
void _SerialSetConfig(uint32_t ui32Port, const tPortParameters *psPort)
{
	//
	// Disable interrupts.
	//
	IntDisable(g_ui32UARTInterrupt[ui32Port]);

	//
	// Set the baud rate.
	//
	//UARTprintf("set baud 0x%d \n",psPort->ui32BaudRate);
	SerialSetBaudRate(ui32Port, psPort->ui32BaudRate);

	//
	// Set wificmd mode.
	//
	//    SerialSetWiFiCMDMode(ui32Port, psPort->uiWiFiCMDMode);  //pony

	//
	// Set transit mode.
	//
	//UARTprintf("set baud 0x%d \n",psPort->uiTransitMode);
	SerialSetTransitMode(ui32Port, psPort->uiTransitMode);  //pony

	//
	// Set the data size.
	//
	//UARTprintf("set baud 0x%d \n",psPort->ui8DataSize);
	SerialSetDataSize(ui32Port, psPort->ui8DataSize);


	//
	// Set the parity.
	//
	//UARTprintf("set baud 0x%d \n",psPort->ui8Parity);
	SerialSetParity(ui32Port, psPort->ui8Parity);


	//
	// Set the stop bits.
	//
	//UARTprintf("set baud 0x%d \n",psPort->ui8StopBits);
	SerialSetStopBits(ui32Port, psPort->ui8StopBits);


	//
	// Set the flow control.
	//
	//UARTprintf("set baud 0x%d \n",psPort->ui8FlowControl);
	SerialSetFlowControl(ui32Port, psPort->ui8FlowControl);



	//
	// Purge the Serial Tx/Rx Ring Buffers.
	//
	SerialPurgeData(ui32Port, 0x03);

	//
	// (Re)enable the UART transmit and receive interrupts.
	//
	// if RFC2217 is enabled, enable (UART_INT_RX | UART_INT_RT | UART_INT_TX |  \
	//  UART_INT_DSR | UART_INT_DCD | UART_INT_CTS | UART_INT_RI)
	if((psPort->ucRFC2217 == 1)&&(ui32Port == 0)) // 1: RFC2217 disabled;
		UARTIntEnable(g_ui32UARTBase[ui32Port],
				(UART_INT_RX | UART_INT_RT | UART_INT_TX));
	else if((psPort->ucRFC2217 == 2)&&(ui32Port == 0)) // 1: RFC2217 enabled;
		UARTIntEnable(g_ui32UARTBase[ui32Port],
				(UART_INT_RX | UART_INT_RT | UART_INT_TX |
						UART_INT_DSR | UART_INT_DCD | UART_INT_CTS | UART_INT_RI));
	else if (ui32Port == 1 || ui32Port == 2)		//chuck add Port2
		UARTIntEnable(g_ui32UARTBase[ui32Port],
				(UART_INT_RX | UART_INT_RT | UART_INT_TX));

	IntEnable(g_ui32UARTInterrupt[ui32Port]);
}

//*****************************************************************************
//
//! Configures the serial port to a default setup.
//!
//! \param ui32Port is the UART port number to be accessed.
//!
//! This function resets the serial port to a default configuration.
//!
//! \return None.
//
//*****************************************************************************
void
SerialSetDefault(uint32_t ui32Port)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Configure the serial port with default parameters.
	//
	_SerialSetConfig(ui32Port, &g_psDefaultParameters->sPort[ui32Port]);
}

//*****************************************************************************
//
//! Configures the serial port according to the current working parameter
//! values.
//!
//! \param ui32Port is the UART port number to be accessed.  Valid values are 0
//! and 1.
//!
//! This function configures the serial port according to the current working
//! parameters in g_sParameters.sPort for the specified port.  The actual
//! parameter set is then read back and g_sParameters.sPort updated to ensure
//! that the structure is correctly synchronized with the hardware.
//!
//! \return None.
//
//*****************************************************************************
void
SerialSetCurrent(uint32_t ui32Port)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Configure the serial port with current parameters.
	//
	//   UARTprintf("set baudrate is %d\n\n",g_sParameters.sPort[ui32Port].ui32BaudRate);
	//UARTprintf("port NO. is %d\n\n",ui32Port);
	_SerialSetConfig(ui32Port, &g_sParameters.sPort[ui32Port]);

	//
	// Get the current settings.
	//
	g_sParameters.sPort[ui32Port].ui32BaudRate = SerialGetBaudRate(ui32Port);
	g_sParameters.sPort[ui32Port].ui8DataSize = SerialGetDataSize(ui32Port);
	g_sParameters.sPort[ui32Port].ui8Parity = SerialGetParity(ui32Port);
	g_sParameters.sPort[ui32Port].ui8StopBits = SerialGetStopBits(ui32Port);
	g_sParameters.sPort[ui32Port].ui8FlowControl = SerialGetFlowControl(ui32Port);
	g_sParameters.sPort[ui32Port].uiTransitMode = SerialGetTransitMode(ui32Port); //pony
	//    g_sParameters.sPort[ui32Port].uiWiFiCMDMode = SerialGetWiFiCMDMode(ui32Port); //pony
	//UARTprintf("get baudrate is %d\n\n",g_sParameters.sPort[ui32Port].ui32BaudRate);
	//UARTprintf("after excecute, mode is %d\n",g_sParameters.sPort[ui32Port].uiTransitMode);
}
/*
void
SerialSetWiFiCMDMode(uint32_t ui32Port, uint8_t ui8WiFiCMDMode)
{
    //
    // Check the arguments.
    //
    ASSERT(ui32Port < MAX_S2E_PORTS);
    ASSERT((ui8WiFiCMDMode == enter_CMD) ||
               (ui8WiFiCMDMode == exit_CMD) );
    // Save the new transmit mode setting.
    g_sParameters.sPort[ui32Port].uiWiFiCMDMode = ui8WiFiCMDMode;
    if(ui8WiFiCMDMode == 1) // 1: enter_cmd; 2: exit_cmd;
    WiFiCMD_Mode_FLag = 1;
    else WiFiCMD_Mode_FLag = 0;
    UARTprintf("if 1 enter cmd mode : %d \n",WiFiCMD_Mode_FLag );


}
 */

//*****************************************************************************
//
//! Set tranist mode.
//!
//! \param ui32Port is the UART port number to be accessed.
//!
//! This function resets the serial port to a default configuration.
//!
//! \return None.
//
//*****************************************************************************
void
SerialSetTransitMode(uint32_t ui32Port,uint32_t ui8TransmitMode)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);
	ASSERT((ui8TransmitMode == RS232) ||
			(ui8TransmitMode == RS485_2_wires) ||
			(ui8TransmitMode == RS422_RS485_4_wires));

	//
	// Save the new transmit mode setting.
	//
	g_sParameters.sPort[ui32Port].uiTransitMode = ui8TransmitMode;

	ROM_GPIOPinTypeGPIOOutput(COM1_Mode0_Port, COM1_Mode1_Pin | COM1_Mode0_Pin | COM1_DIR_Pin);//PP2/PP3/PP4
	MAP_GPIOPadConfigSet(COM1_Mode0_Port, COM1_Mode1_Pin | COM1_Mode0_Pin | COM1_DIR_Pin, GPIO_STRENGTH_2MA,
			GPIO_PIN_TYPE_STD_WPD);

	ROM_GPIOPinTypeGPIOOutput(COM1_TERM_Port,COM1_TERM_Pin);//PQ4
	MAP_GPIOPadConfigSet(COM1_TERM_Port,COM1_TERM_Pin, GPIO_STRENGTH_2MA,
			GPIO_PIN_TYPE_STD_WPD);

	switch(ui8TransmitMode)
	{
	case RS232 :
		ROM_GPIOPinWrite(COM1_TERM_Port, COM1_TERM_Pin, 0);//rs232
		ROM_GPIOPinWrite(COM1_Mode0_Port, COM1_Mode1_Pin, 0);
		ROM_GPIOPinWrite(COM1_Mode0_Port, COM1_Mode0_Pin, COM1_Mode0_Pin);
		ROM_GPIOPinWrite(COM1_Mode0_Port, COM1_DIR_Pin, 0);
		//	     ROM_GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_4, 0);
		//	     ROM_GPIOPinWrite(GPIO_PORTP_BASE, (GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2), GPIO_PIN_3);
		break;
	case RS422_RS485_4_wires :
		ROM_GPIOPinWrite(COM1_TERM_Port, COM1_TERM_Pin, COM1_TERM_Pin);
		ROM_GPIOPinWrite(COM1_Mode0_Port, COM1_Mode1_Pin, COM1_Mode1_Pin);
		ROM_GPIOPinWrite(COM1_Mode0_Port, COM1_Mode0_Pin, COM1_Mode0_Pin);
		ROM_GPIOPinWrite(COM1_Mode0_Port, COM1_DIR_Pin, COM1_DIR_Pin);
		break;
	case RS485_2_wires :
		ROM_GPIOPinWrite(COM1_TERM_Port, COM1_TERM_Pin, COM1_TERM_Pin);
		ROM_GPIOPinWrite(COM1_Mode0_Port, COM1_Mode1_Pin, COM1_Mode1_Pin);
		ROM_GPIOPinWrite(COM1_Mode0_Port, COM1_Mode0_Pin, 0);
		ROM_GPIOPinWrite(COM1_Mode0_Port, COM1_DIR_Pin, 0);
		break;
	default: break;
	}
}


//*****************************************************************************
//
//! Configures the serial port to the factory default setup.
//!
//! \param ui32Port is the UART port number to be accessed.
//!
//! This function resets the serial port to a default configuration.
//!
//! \return None.
//
//*****************************************************************************
void
SerialSetFactory(uint32_t ui32Port)
{
	//
	// Check the arguments.
	//
	ASSERT(ui32Port < MAX_S2E_PORTS);

	//
	// Configure the serial port with current parameters.
	//
	_SerialSetConfig(ui32Port, &g_psFactoryParameters->sPort[ui32Port]);
}

//*****************************************************************************
//
//! Initializes the serial port driver.
//!
//! This function initializes and configures the serial port driver.
//!
//! \return None.
//
//*****************************************************************************
void
SerialInit(void)
{
	//	int i;
	//
	// Initialize the ring buffers used by the UART Drivers.
	//
	RingBufInit(&g_sRxBuf[0], g_pui8RX0Buffer, sizeof(g_pui8RX0Buffer));
	RingBufInit(&g_sTxBuf[0], g_pui8TX0Buffer, sizeof(g_pui8TX0Buffer));
	RingBufInit(&g_sRxBuf[1], g_pui8RX1Buffer, sizeof(g_pui8RX1Buffer));
	RingBufInit(&g_sTxBuf[1], g_pui8TX1Buffer, sizeof(g_pui8TX1Buffer));
	RingBufInit(&g_sRxBuf[2], g_pui8RX2Buffer, sizeof(g_pui8RX2Buffer));
	RingBufInit(&g_sTxBuf[2], g_pui8TX2Buffer, sizeof(g_pui8TX2Buffer));
	//for wifipage access
	RingBufInit(&wifi_pageRxBuf, wifi_pageui8RXBuffer, sizeof(wifi_pageui8RXBuffer));
	RingBufInit(&wifi_pageTxBuf, wifi_pageui8TXBuffer, sizeof(wifi_pageui8TXBuffer));
	//The ring buffer for com to LAN, used for COM-LAN and COM-WLAN both;
	RingBufInit(&Data_From_COM_To_LAN, Data_From_COM_To_LAN_Buffer, sizeof(Data_From_COM_To_LAN_Buffer));
	RingBufInit(&Data_From_COM1_To_LAN, Data_From_COM1_To_LAN_Buffer, sizeof(Data_From_COM1_To_LAN_Buffer));
	RingBufInit(&Data_From_COM2_To_LAN, Data_From_COM2_To_LAN_Buffer, sizeof(Data_From_COM2_To_LAN_Buffer));
	//
	// Enable and Configure Serial Port 0.        --UART2
	//
	SysCtlPeripheralEnable(S2E_PORT0_RX_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT0_TX_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT0_RTS_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT0_CTS_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT0_UART_PERIPH);
	GPIOPinConfigure(S2E_PORT0_RX_CONFIG);
	GPIOPinConfigure(S2E_PORT0_TX_CONFIG);
	GPIOPinConfigure(S2E_PORT0_RTS_CONFIG);
	GPIOPinConfigure(S2E_PORT0_CTS_CONFIG);
	GPIOPinTypeUART(S2E_PORT0_RX_PORT, S2E_PORT0_RX_PIN);
	GPIOPinTypeUART(S2E_PORT0_TX_PORT, S2E_PORT0_TX_PIN);
	GPIOPinTypeUART(S2E_PORT0_RTS_PORT, S2E_PORT0_RTS_PIN);
	GPIOPinTypeUART(S2E_PORT0_CTS_PORT, S2E_PORT0_CTS_PIN);




	//
	// Configure the Port 1 pins appropriately.		--UART4
	//
	SysCtlPeripheralEnable(S2E_PORT1_RX_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT1_TX_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT1_RTS_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT1_CTS_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT1_UART_PERIPH);
	GPIOPinConfigure(S2E_PORT1_RX_CONFIG);
	GPIOPinConfigure(S2E_PORT1_TX_CONFIG);
	GPIOPinConfigure(S2E_PORT1_RTS_CONFIG);
	GPIOPinConfigure(S2E_PORT1_CTS_CONFIG);
	GPIOPinTypeUART(S2E_PORT1_RX_PORT, S2E_PORT1_RX_PIN);
	GPIOPinTypeUART(S2E_PORT1_TX_PORT, S2E_PORT1_TX_PIN);
	GPIOPinTypeUART(S2E_PORT1_RTS_PORT, S2E_PORT1_RTS_PIN);
	GPIOPinTypeUART(S2E_PORT1_CTS_PORT, S2E_PORT1_CTS_PIN);

	//
	// Configure the Port 2 pins appropriately.		--UART1
	//
	SysCtlPeripheralEnable(S2E_PORT2_RX_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT2_TX_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT2_RTS_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT2_CTS_PERIPH);
//	SysCtlPeripheralEnable(S2E_PORT2_DCD_PERIPH);
//	SysCtlPeripheralEnable(S2E_PORT2_DSR_PERIPH);
//	SysCtlPeripheralEnable(S2E_PORT2_DTR_PERIPH);
//	SysCtlPeripheralEnable(S2E_PORT2_RI_PERIPH);
	SysCtlPeripheralEnable(S2E_PORT2_UART_PERIPH);

	GPIOPinConfigure(S2E_PORT2_RX_CONFIG);
	GPIOPinConfigure(S2E_PORT2_TX_CONFIG);
	GPIOPinConfigure(S2E_PORT2_RTS_CONFIG);
	GPIOPinConfigure(S2E_PORT2_CTS_CONFIG);
//	GPIOPinConfigure(S2E_PORT2_DCD_CONFIG);
//	GPIOPinConfigure(S2E_PORT2_DSR_CONFIG);
//	GPIOPinConfigure(S2E_PORT2_DTR_CONFIG);
//	GPIOPinConfigure(S2E_PORT2_RI_CONFIG);

	GPIOPinTypeUART(S2E_PORT2_RX_PORT, S2E_PORT2_RX_PIN);
	GPIOPinTypeUART(S2E_PORT2_TX_PORT, S2E_PORT2_TX_PIN);
	GPIOPinTypeUART(S2E_PORT2_RTS_PORT, S2E_PORT2_RTS_PIN);
	GPIOPinTypeUART(S2E_PORT2_CTS_PORT, S2E_PORT2_CTS_PIN);
//	GPIOPinTypeUART(S2E_PORT2_DCD_PORT, S2E_PORT2_DCD_PIN);
//	GPIOPinTypeUART(S2E_PORT2_DSR_PORT, S2E_PORT2_DSR_PIN);
//	GPIOPinTypeUART(S2E_PORT2_DTR_PORT, S2E_PORT2_DTR_PIN);
//	GPIOPinTypeUART(S2E_PORT2_RI_PORT, S2E_PORT2_RI_PIN);

	//
	// Configure Port 0.
	//
	/*
	UARTprintf("fac ui8SequenceNum:%X\n",g_sParametersFactory.ui8SequenceNum);
	UARTprintf("fac ui8CRC:%X\n",g_sParametersFactory.ui8CRC);
	UARTprintf("fac ui8Version:%X\n",g_sParametersFactory.ui8Version);
	UARTprintf("fac ui8Flags:%X\n",g_sParametersFactory.ui8Flags);
//	UARTprintf("fac ui8Reserved0:%s\n",g_sParametersFactory.ui8Reserved0);
//	UARTprintf("fac ui8Reserved1:%s\n",g_sParametersFactory.ui8Reserved1);
	UARTprintf("fac ui32BaudRate:%d\n",g_sParametersFactory.sPort[1].ui32BaudRate);

	UARTprintf("fac ui8DataSize:%X\n",g_sParametersFactory.sPort[1].ui8DataSize);
	UARTprintf("fac ui8Parity:%X\n",g_sParametersFactory.sPort[1].ui8Parity);
	UARTprintf("fac ui8StopBits:%X\n",g_sParametersFactory.sPort[1].ui8StopBits);
	UARTprintf("fac ui8FlowControl:%X\n",g_sParametersFactory.sPort[1].ui8FlowControl);
	UARTprintf("fac ui32TelnetTimeout:%X\n",g_sParametersFactory.sPort[1].ui32TelnetTimeout);
	UARTprintf("fac ui16TelnetLocalPort:%X\n",g_sParametersFactory.sPort[1].ui16TelnetLocalPort);
	UARTprintf("fac ui16TelnetRemotePort:%X\n",g_sParametersFactory.sPort[1].ui16TelnetRemotePort);
	UARTprintf("fac ui32TelnetIPAddr:%X\n",g_sParametersFactory.sPort[1].ui32TelnetIPAddr);

//	UARTprintf("gs_paras bd:%d\n",g_psDefaultParameters->sPort->ui32BaudRate);


	UARTprintf("ui8SequenceNum:%X\n",g_sParameters.ui8SequenceNum);
	UARTprintf("ui8CRC:%X\n",g_sParameters.ui8CRC);
	UARTprintf("ui8Version:%X\n",g_sParameters.ui8Version);
	UARTprintf("ui8Flags:%X\n",g_sParameters.ui8Flags);
//	UARTprintf("ui8Reserved0:%s\n",g_sParameters.ui8Reserved0);
//	UARTprintf("ui8Reserved1:%s\n",g_sParameters.ui8Reserved1);
	UARTprintf("gs_paras bd:%d\n",g_sParameters.sPort->ui32BaudRate);

	UARTprintf("ui8DataSize:%X\n",g_sParameters.sPort->ui8DataSize);
	UARTprintf("ui8Parity:%X\n",g_sParameters.sPort->ui8Parity);
	UARTprintf("ui8StopBits:%X\n",g_sParameters.sPort->ui8StopBits);
	UARTprintf("ui8FlowControl:%X\n",g_sParameters.sPort->ui8FlowControl);
	UARTprintf("ui32TelnetTimeout:%X\n",g_sParameters.sPort->ui32TelnetTimeout);
	UARTprintf("ui16TelnetLocalPort:%X\n",g_sParameters.sPort->ui16TelnetLocalPort);
	UARTprintf("ui16TelnetRemotePort:%X\n",g_sParameters.sPort->ui16TelnetRemotePort);
	UARTprintf("ui32TelnetIPAddr:%X\n",g_sParameters.sPort->ui32TelnetIPAddr);
	UARTprintf("ui8Flags:%X\n",g_sParameters.sPort->ui8Flags);
	UARTprintf("uiTelnetURL:%s\n",g_sParameters.sPort->uiTelnetURL);
	UARTprintf("ucWorkMode:%X\n",g_sParameters.sPort->ucWorkMode);
	UARTprintf("uiPackLen:%s\n",g_sParameters.sPort->uiPackLen);
	UARTprintf("uiPackTime:%X\n",g_sParameters.sPort->uiPackTime);
	UARTprintf("ucTimeCount:%X\n",g_sParameters.sPort->ucTimeCount);
	UARTprintf("uiTCPServerType:%X\n",g_sParameters.sPort->uiTCPServerType);
	UARTprintf("uiTransitMode:%X\n",g_sParameters.sPort->uiTransitMode);
	UARTprintf("ucRFC2217:%X\n",g_sParameters.sPort->ucRFC2217);


//	UARTprintf("usLocationURLPort:%s\n",g_sParameters.usLocationURLPort);
//	UARTprintf("usHTTPServerPort:%s\n",g_sParameters.usHTTPServerPort);
//	UARTprintf("ucUserFlag:%X\n",g_sParameters.ucUserFlag);
//	UARTprintf("uiID:%s\n",g_sParameters.uiID);
//	UARTprintf("ucIDType:%X\n",g_sParameters.ucIDType);
//	UARTprintf("ucUserMAC:%s\n",g_sParameters.ucUserMAC);

	UARTprintf("ucUserMAC:%X:%X:%X:%X:%X:%X\n",g_sParameters.ucUserMAC[0],g_sParameters.ucUserMAC[1],g_sParameters.ucUserMAC[2],g_sParameters.ucUserMAC[3],g_sParameters.ucUserMAC[4],g_sParameters.ucUserMAC[5]);

	UARTprintf("datachansel:%X\n",g_sParameters.datachansel);
	UARTprintf("ui32WiFiModuleBaudRate:%d\n",g_sParameters.ui32WiFiModuleBaudRate);
	UARTprintf("modbus:%X\n",g_sParameters.modbus);
//	UARTprintf("ui8Reserved2:1:%c 2:%c 3:%c 4:%c 5:%c 6:%c 7:%c \n",g_sParameters.ui8Reserved2[0],g_sParameters.ui8Reserved2[1],g_sParameters.ui8Reserved2[2],g_sParameters.ui8Reserved2[3],g_sParameters.ui8Reserved2[4],
//			g_sParameters.ui8Reserved2[5],g_sParameters.ui8Reserved2[6]);
//	UARTprintf("ui8Reserved3:1:%c 2:%c 3:%c 4:%c 5:%c 6:%c 7:%c \n",g_sParameters.ui8Reserved3[0],g_sParameters.ui8Reserved3[1],g_sParameters.ui8Reserved3[2],g_sParameters.ui8Reserved3[3],g_sParameters.ui8Reserved3[4],
//			g_sParameters.ui8Reserved3[5],g_sParameters.ui8Reserved3[6]);

//	UARTprintf("gs_paras bd:\n%s\n",g_sParameters);

	UARTprintf("factory setting:\n");
	for(i=0;i<=255;i++)
	{
		UARTprintf("%X ",g_sParametersFactory[i]);
	}
	UARTprintf("\n");

	UARTprintf("Default setting:\n");
	for(i=0;i<=255;i++)
	{
		UARTprintf("%X ",g_sParameters[i]);
	}
	UARTprintf("\n");
	 */


	SerialSetDefault(0); //COM CONNECTOR on PCB;
	//delayms(1000);

	//
	// Configure Port 1.
	//
	SerialSetDefault(1);
	//delayms(1000);

	SerialSetDefault(2);
	//   if(g_sParameters.ui32WiFiModuleBaudRate !=9600) // not restore;
	//_UR4_SerialSetConfig(WiFi_UR_to_MCU_UR_Baudrate); // For WiFi UR communication bus;
}


void RS485_TxEnable(uint32_t ui32Port )	//TODO: to be called before uart3 transmit
{
	//	UARTModemControlSet(UART1_BASE,)
	//	UARTprintf("RS485 tx en\n");
	//	UARTprintf("RS485 TM:%d\n",g_sParameters.sPort->uiTransitMode);
	if((g_sParameters.sPort->uiTransitMode == RS485_2_wires)&&(ui32Port == 0)) // half work mode;
		//	if((g_sParameters.sPort->ui8FlowControl == 5)&&(ui32Port == 0))
	{
		GPIOPinWrite(COM1_Mode0_Port, COM1_DIR_Pin,COM1_DIR_Pin);
		//GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_4, 0);
		//	SysCtlDelay(100);
		//	delayuS(100);
		//	delayms(10);
		while(GPIOPinRead(COM1_Mode0_Port, COM1_DIR_Pin) != COM1_DIR_Pin);

	}

}

void RS485_TxDisable(uint32_t ui32Port )	//TODO: to be called after uart3 transmit
{
	//	uint8_t time_interval;
	//	UARTprintf("RS485 tx dis\n");
	if((g_sParameters.sPort->uiTransitMode == RS485_2_wires)&&(ui32Port == 0)) // half work mode;
		//	if((g_sParameters.sPort->ui8FlowControl == 5)&&(ui32Port == 0)) // for youren ap;
	{
		//	SysCtlDelay(100);
		//	delayuS(100);

		//		time_interval = (long int)(30*5*1000/(g_sParameters.sPort->ui32BaudRate));// need over 4 byte time interval;
		//		vTaskDelay( time_interval/portTICK_RATE_MS );
		//		UARTprintf("baud:%ld,time:%ld\n",g_sParameters.sPort->ui32BaudRate,time_interval);
		//		delayms(time_interval);
		// WiFi error information;
		/*
			switch(g_sParameters.sPort->ui32BaudRate)
			{
			//>19200, 95uS+750uS=845uS; char timeout interval and transmit time;
			case 921600: time_interval = 0.4;  break;//char time11.935uS;  11.935uS+750uS=762uS; 400uS is ok;

			case 460800: time_interval = 0.4;  break; //char time23.87uS;  23.87uS+750uS=773.87uS; 400uS is ok;
			case 256000: time_interval = 0.4;  break; //char time42.9uS;  42.9uS+750uS=792.9uS; 400uS is ok;
			case 230400: time_interval = 0.4;  break;  //char time47.7uS;  47.7uS+750uS=797.7uS; 400uS is ok;
			case 115200: time_interval = 0.4;  break; //char time95.5uS;  95.5uS+750uS=845.5uS; 0.4mS is ok;


			case 57600: time_interval = 0.5;  break; //char time:190.97uS; 190.97uS+750uS=941uS; 0.5mS is ok;
			case 38400: time_interval = 1;  break; //char time:286.46uS; 286.46uS+750uS=1036uS; 1mS is ok;
			//<=19200, timeout time: <1.5 *char_time +1 char time;
			case 19200: time_interval = 1.4;  break; //char time:573uS; *2.5=1.43mS;  1.4ms is ok;
			case 14400: time_interval = 1.4;  break; //char time:764uS; *2.5=1.91mS;  1.4ms is ok;
			case 9600: time_interval = 1.5;  break; //char time:1.145mS; *2.5=2.863mS;  1.5ms is ok;
			case 4800: time_interval = 3;  break; //char time:2.29mS; *2.5=5.73mS;  3ms is ok;
			case 2400: time_interval = 6;  break;   //char time:4.58ms; *2.5 = 11.458ms; so 6ms is ok;
			case 1200: time_interval = 12; break;//char time:9.16ms; *2.5 = 22.916ms; so 12ms is ok;
			case 600:  time_interval = 24; break;//char time:18.33ms; *2.5 = 45.83ms; so 24ms is ok;
			case 300:  time_interval = 48; break;//char time:36.66ms; *2.5 = 91.66ms; so 48ms is ok;
			case 120:  time_interval = 120;break;//char time:91.67ms; *2.5 = 229.17ma; so 120ms is ok;
			default:   time_interval = 3;  break;
			}


	//	delayms(time_interval);
			delay1us(time_interval*1000);
		 */
		while(UARTBusy(g_ui32UARTBase[ui32Port]));
		GPIOPinWrite(COM1_Mode0_Port, COM1_DIR_Pin,0);
		while(GPIOPinRead(COM1_Mode0_Port, COM1_DIR_Pin) != 0);

		//GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_4, GPIO_PIN_4);
	}

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
