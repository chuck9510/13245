//*****************************************************************************
//
// serial_task.c - Task to handle serial peripherals and their interrupts.
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

#include <string.h>
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

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "user_define.h"
#include "WiFly_Webpage.h"
//*****************************************************************************
//
// The stack size for the Serial task.
//
//*****************************************************************************
//#define STACKSIZE_SerialTASK    1024 // from 512

//*****************************************************************************
//
// Queue used to pass information from the UART interrupt handlers to the task.
// This queue is also used to signal the task that an event has occurred.
//
//*****************************************************************************
xQueueHandle g_QueSerial;

//*****************************************************************************
//
//! Possible serial event types.
//
//*****************************************************************************
typedef enum
{
	RX,
	TX
}
tSerialEventType;

//*****************************************************************************
//
//! This structure is used for holding the state of a given serial interrupt.
//! On every event, an instance of this structure is passed from the serial
//! interrupt handler to the serial task.
//
//*****************************************************************************
typedef struct
{
	tSerialEventType eEventType;
	uint8_t ui8Port;
	uint8_t ui8Char;
}
tSerialEvent;

//*****************************************************************************
//
//! Handles the UART interrupt.
//!
//! \param ui8Port is the serial port number to be accessed.
//!
//! This function is called when either of the UARTs generate an interrupt.
//! An interrupt will be generated when data is received and when the transmit
//! FIFO becomes half empty.  The transmit and receive FIFOs are processed as
//! appropriate.
//!
//! \return None.
//
//*****************************************************************************
void Fill_Modem_Status(uint32_t ui32Status)
{
	//	uint32_t modem_state;
	// read the responding bits, and fill into modemstus variable;
	//bit0:RIMIS; bit1: CTSMIS; bit2: DCDMIS; bit3: DSRMIS; bit4:RXMIS bit5:TXMIS bit6:RTMIS

	//	modem_state = ui32Status&0x7F;

}

static void
SerialUARTIntHandler(uint8_t ui8Port)
{
	signed portBASE_TYPE bYield;
	uint32_t ui32Status;
	tSerialEvent sEvent;
	wifiini_flag = 0;
	wifidataavail_flag = 0;
	//   i=0;
	//
	// Set the port for which this interrupt occurred.
	//
	sEvent.ui8Port = ui8Port;

	//
	// Get the cause of the interrupt.
	//
	ui32Status = UARTIntStatus(g_ui32UARTBase[ui8Port], true);

	//   Fill_Modem_Status();
	//
	// Clear the cause of the interrupt.
	//
	UARTIntClear(g_ui32UARTBase[ui8Port], ui32Status);

	//  if((!wifiini_flag)||(ui8Port == COM_Port))


	//    {
	//
	// See if there is data to be processed in the receive FIFO.
	//
	if(ui32Status & (UART_INT_RT | UART_INT_RX))
	{
		//
		// Loop while there are characters available in the receive FIFO.
		//
		 //UARTprintf("%d rx interrupt\n",ui8Port);
		while(UARTCharsAvail(g_ui32UARTBase[ui8Port]))
		{

			//
			// Set the interrupt type to help the serial task handle the
			// information appropriately.
			//
			sEvent.eEventType = RX;

			//
			// Get the next character from the receive FIFO.
			//
			sEvent.ui8Char = UARTCharGet(g_ui32UARTBase[ui8Port]);
			// UARTprintf("%02X ",sEvent.ui8Char);
			//    	            if(ui8Port== WiFi_Port)
			//    	            {
			//  	cnt_debug0++;
			//    UARTprintf("%02X ",sEvent.ui8Char);
			//    	            	UARTprintf("%c ",sEvent.ui8Char);
			// set flag to indicate com datain;
			//  INT_COM_Data_In = true;
			//    	            }

			//
			// Send the information regarding this event to back of the Queue.
			// This also signals to the task that a serial event has occurred.
			//
			xQueueSendToBackFromISR(g_QueSerial, &sEvent, &bYield);


			//
			// If a context switch is necessary, schedule one for when the ISR
			// exits.
			//
			if(bYield)
			{
				vPortYield();
			}
		}
	}
	//    rxdata_receiving = false;
	//
	// See if there is space to be filled in the transmit FIFO.
	//
	if(ui32Status & UART_INT_TX)
	{
			// UARTprintf("%d TX interrupt\n",ui8Port);
		//
		// Set the interrupt type to help the serial task handle the
		// information appropriately.
		//
		sEvent.eEventType = TX;
		//        UARTprintf("TX interrupt: %d",TX);
		//
		// Send the information regarding this event to back of the Queue.
		// This also signals to the task that a serial event has occurred.
		//
		xQueueSendToBackFromISR(g_QueSerial, &sEvent, &bYield);

		//
		// If a context switch is necessary, schedule one for when the ISR
		// exits.
		//
		if(bYield)
		{
			vPortYield();
		}
	}

}
//*****************************************************************************
//
//! Handles the UART0 interrupt.
//!
//! This function is called when the UART generates an interrupt.  An interrupt
//! will be generated when data is received and when the transmit FIFO becomes
//! half empty.  These interrupts are handled by the SerialUARTIntHandler()
//! function.
//! \return None.
//
//*****************************************************************************
void
SerialPort0IntHandler(void)
{


	SerialUARTIntHandler(0);
}

//*****************************************************************************
//
//! Handles the UART1 interrupt.
//!
//! This function is called when the UART generates an interrupt.  An interrupt
//! will be generated when data is received and when the transmit FIFO becomes
//! half empty.  These interrupts are handled by the SerialUARTIntHandler()
//! function.
//!
//! \return None.
//
//*****************************************************************************
void
SerialPort1IntHandler(void)
{
	SerialUARTIntHandler(1);
}
//*****************************************************************************
//
//! Handles the UART2 interrupt.
//!
//! This function is called when the UART generates an interrupt.  An interrupt
//! will be generated when data is received and when the transmit FIFO becomes
//! half empty.  These interrupts are handled by the SerialUARTIntHandler()
//! function.
//!
//! \return None.
//
//*****************************************************************************
void
SerialPort2IntHandler(void)
{
	//UARTprintf("uart1-interrupt\n");
	SerialUARTIntHandler(2);
}


void Webserver_Data_Out_Processing_TCPChar(tSerialEvent sEvent)
{
	uint8_t ui8Temp;
	//
	// Loop while there is space in the transmit FIFO and characters to
	// be sent.
	//
	while(!RingBufEmpty(&wifi_pageTxBuf) &&
			UARTSpaceAvail(g_ui32UARTBase[sEvent.ui8Port]))
	{

		//
		// Write the next character into the transmit FIFO.
		//

		ui8Temp = RingBufReadOne(&wifi_pageTxBuf);
		UARTCharPut(g_ui32UARTBase[sEvent.ui8Port],
				ui8Temp);

		//UARTprintf("%c",ui8Temp);
	}
	//   i= RingBufUsed(&wifi_pageTxBuf);
	// UARTprintf("\nrb left:%d\n",i);
}

void Webserver_Data_In_Processing_TCPChar(tSerialEvent sEvent)
{
	//
	// The first part of the queue message is the type of event.  Check if
	// it's an RX or receive timeout interrupt.
	//
	if(sEvent.eEventType == RX)
	{
		//
		// If this is a Telnet IAC character, write it twice.
		//
		while(RingBufFree(&wifi_pageRxBuf)<1){
			// clear;
			RingBufFlush(&wifi_pageRxBuf);

			UARTprintf("wibuf full\n");
			//    	WiFi_TCP_connected = false;
			//    	httpState = HTTP_CONN_WAIT;
			//		 wifibuffer_used= RingBufUsed(&wifi_pageRxBuf);
			//		 UARTprintf("\nleft>:%d\n",wifibuffer_used);

		};

		RingBufWriteOne(&wifi_pageRxBuf, sEvent.ui8Char);

	}

}

void COM_Data_Out_Sending_Not_TCPChar(tSerialEvent sEvent)
{
	uint8_t ui8Temp;
	//RS485_TxEnable(sEvent.ui8Port);
	//
	// Loop while there is space in the transmit FIFO and characters to
	// be sent.
	//
	while(!RingBufEmpty(&g_sTxBuf[sEvent.ui8Port]) &&
			UARTSpaceAvail(g_ui32UARTBase[sEvent.ui8Port]))
	{

		//
		// Write the next character into the transmit FIFO.
		//


		ui8Temp = RingBufReadOne(&g_sTxBuf[sEvent.ui8Port]);
		UARTCharPut(g_ui32UARTBase[sEvent.ui8Port],
				ui8Temp);
		//        UARTprintf("%02X",ui8Temp);
		/*
        if(sEvent.ui8Port == WiFi_Port)
        UARTprintf("%02X",ui8Temp);
		 */
		//   UARTprintf("data to com: %c \n",ui8Temp);
	}
	//RS485_TxDisable(sEvent.ui8Port);
}

void COM_Data_In_Handle_Direct(tSerialEvent sEvent)
{
#if  Have_WiFi_Module_SKU
	if(WiFi_Setting)
	{
		Switch_WiFi_to_COM = 0;
		if(sEvent.ui8Port==WiFi_Port)
		{
			RingBufWriteOne(&g_sRxBuf[sEvent.ui8Port], sEvent.ui8Char);
		}
	}
	else
#endif
		if(sEvent.ui8Port==WiFi_Port)
		{
			UARTprintf("WiFi_Port:%c",sEvent.ui8Char);
			// whether RTU to RTU mode enabled?
			if(((g_sParameters.sPort->ui8Flags & PORT_PROTOCOL_MODBUS) == PORT_MODBUS_RTU_RTU)
					&&(sEvent.ui8Port == WiFi_Port)) //2:WLAN_COM
			{
				//	 UARTprintf("..%c\n",sEvent.ui8Char);
				UARTprintf("f:0x%02X msr:%d \n",g_sParameters.sPort->ui8Flags,modbusRTUState3);
				ModbusPDU_Req_WiFi_Port(sEvent.ui8Char);
				//	SerialSend(COM_Port, sEvent.ui8Char);
			}
			else

				// send to com, data from wifi port to com port;
				//		SerialSend(COM_Port, sEvent.ui8Char);
				RingBufWriteOne(&g_sRxBuf[sEvent.ui8Port], sEvent.ui8Char);

		}
		else if(sEvent.ui8Port==COM_Port)
		{
			//UARTprintf("COM_Port:%d",sEvent.ui8Char);
			// whether RTU to ModbusTCP mode enabled?
			if(
					((g_sParameters.sPort->ui8Flags &
							PORT_PROTOCOL_MODBUS) == PORT_MODBUS_RTU_TCP) &&(sEvent.ui8Port == COM_Port)
			)//modbus_en;

			{
				//			UARTprintf("0x%02X ",sEvent.ui8Char);
				ModbusTimerDisArm();
				ModbusRTURecvTCPSend(sEvent.ui8Char);
				UARTprintf("f:0x%02X msr:%d \n",g_sParameters.sPort->ui8Flags,modbusRTUState);
			}
			// whether RTU to RTU mode enabled for com in?
			else if(((g_sParameters.sPort->ui8Flags & PORT_PROTOCOL_MODBUS) == PORT_MODBUS_RTU_RTU)
					&&(sEvent.ui8Port == COM_Port))
			{
				//			UARTprintf("0x%02X ",sEvent.ui8Char);
				ModbusTimerDisArm();
				ModbusRTURecv_COM_Port(sEvent.ui8Char);
				UARTprintf("f:0x%02X msr:%d \n",g_sParameters.sPort->ui8Flags,modbusRTUState2);
			}
			else
			{

				RingBufWriteOne(&g_sRxBuf[COM_Port], sEvent.ui8Char);
				/*
			// wifi to com only, send to wifi port only;
			if((Switch_WiFi_to_COM ==1)&&(Switch_LAN_to_COM == 0)&&(Webpage_Visit_Req== false) )
//				SerialSend(WiFi_Port, sEvent.ui8Char);
				RingBufWriteOne(&g_sRxBuf[COM_Port], sEvent.ui8Char);
			// LAN to com only, send to LAN port only;
			else if((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 1) )
				RingBufWriteOne(&Data_From_COM_To_LAN, sEvent.ui8Char);
			// Both;
			else if ((Switch_WiFi_to_COM ==1)&&(Switch_LAN_to_COM == 1)&&(Webpage_Visit_Req== false) )
			{
//				SerialSend(WiFi_Port, sEvent.ui8Char);
				RingBufWriteOne(&g_sRxBuf[COM_Port], sEvent.ui8Char);
				RingBufWriteOne(&Data_From_COM_To_LAN, sEvent.ui8Char);
//					 UARTprintf("%c",sEvent.ui8Char);
			}
				 */
				/*
			else if ((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 0)&&(Webpage_Visit_Req== false) )
			{

				RingBufWriteOne(&g_sRxBuf[sEvent.ui8Port], sEvent.ui8Char);
					 UARTprintf("%c",sEvent.ui8Char);
			}
				 */
			}

		}
		else if(sEvent.ui8Port==COM1_Port)
		{
			//UARTprintf("COM_Port:%d",sEvent.ui8Char);
			// whether RTU to ModbusTCP mode enabled?
			if(
					((g_sParameters.sPort->ui8Flags &
							PORT_PROTOCOL_MODBUS) == PORT_MODBUS_RTU_TCP) &&(sEvent.ui8Port == COM1_Port)
			)//modbus_en;

			{
				//			UARTprintf("0x%02X ",sEvent.ui8Char);
				ModbusTimerDisArm();
				ModbusRTURecvTCPSend(sEvent.ui8Char);
				UARTprintf("f:0x%02X msr:%d \n",g_sParameters.sPort->ui8Flags,modbusRTUState);
			}
			// whether RTU to RTU mode enabled for com in?
			else if(((g_sParameters.sPort->ui8Flags & PORT_PROTOCOL_MODBUS) == PORT_MODBUS_RTU_RTU)
					&&(sEvent.ui8Port == COM1_Port))
			{
				//			UARTprintf("0x%02X ",sEvent.ui8Char);
				ModbusTimerDisArm();
				ModbusRTURecv_COM_Port(sEvent.ui8Char);
				UARTprintf("f:0x%02X msr:%d \n",g_sParameters.sPort->ui8Flags,modbusRTUState2);
			}
			else
			{
				RingBufWriteOne(&g_sRxBuf[COM1_Port], sEvent.ui8Char);
				/*
			// wifi to com only, send to wifi port only;
			if((Switch_WiFi_to_COM ==1)&&(Switch_LAN_to_COM == 0)&&(Webpage_Visit_Req== false) )
//				SerialSend(WiFi_Port, sEvent.ui8Char);
				RingBufWriteOne(&g_sRxBuf[COM_Port], sEvent.ui8Char);
			// LAN to com only, send to LAN port only;
			else if((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 1) )
				RingBufWriteOne(&Data_From_COM_To_LAN, sEvent.ui8Char);
			// Both;
			else if ((Switch_WiFi_to_COM ==1)&&(Switch_LAN_to_COM == 1)&&(Webpage_Visit_Req== false) )
			{
//				SerialSend(WiFi_Port, sEvent.ui8Char);
				RingBufWriteOne(&g_sRxBuf[COM_Port], sEvent.ui8Char);
				RingBufWriteOne(&Data_From_COM_To_LAN, sEvent.ui8Char);
//					 UARTprintf("%c",sEvent.ui8Char);
			}
				 */
				/*
			else if ((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 0)&&(Webpage_Visit_Req== false) )
			{

				RingBufWriteOne(&g_sRxBuf[sEvent.ui8Port], sEvent.ui8Char);
					 UARTprintf("%c",sEvent.ui8Char);
			}
				 */
			}

		}
		else if(sEvent.ui8Port==COM2_Port)
		{

			//UARTprintf("COM_Port:%d",sEvent.ui8Char);
			// whether RTU to ModbusTCP mode enabled?
			if(
					((g_sParameters.sPort->ui8Flags &
							PORT_PROTOCOL_MODBUS) == PORT_MODBUS_RTU_TCP) &&(sEvent.ui8Port == COM2_Port)
			)//modbus_en;

			{
				//			UARTprintf("0x%02X ",sEvent.ui8Char);
				ModbusTimerDisArm();
				ModbusRTURecvTCPSend(sEvent.ui8Char);
				UARTprintf("f:0x%02X msr:%d \n",g_sParameters.sPort->ui8Flags,modbusRTUState);
			}
			// whether RTU to RTU mode enabled for com in?
			else if(((g_sParameters.sPort->ui8Flags & PORT_PROTOCOL_MODBUS) == PORT_MODBUS_RTU_RTU)
					&&(sEvent.ui8Port == COM2_Port))
			{
				//			UARTprintf("0x%02X ",sEvent.ui8Char);
				ModbusTimerDisArm();
				ModbusRTURecv_COM_Port(sEvent.ui8Char);
				UARTprintf("f:0x%02X msr:%d \n",g_sParameters.sPort->ui8Flags,modbusRTUState2);
			}
			else
			{
				RingBufWriteOne(&g_sRxBuf[COM2_Port], sEvent.ui8Char);
			}
		}
}

void COM_Data_In_Processing_Not_TCPChar(tSerialEvent sEvent)
{
	//
	// If Telnet protocol is enabled, check for incoming IAC character,
	// and escape it.
	//
	if(((g_sParameters.sPort[sEvent.ui8Port].ui8Flags &
			PORT_FLAG_PROTOCOL) == PORT_PROTOCOL_TELNET) &&(sEvent.ui8Port == COM_Port)
	)
	{
		//
		// If this is a Telnet IAC character, write it twice.
		//

		if((sEvent.ui8Char == TELNET_IAC)
				&&(RingBufFree(&g_sRxBuf[sEvent.ui8Port]) >= 2)
		)
		{
			// 	while(RingBufFree(&g_sRxBuf[sEvent.ui8Port]) < 2){};
			RingBufWriteOne(&g_sRxBuf[sEvent.ui8Port], sEvent.ui8Char);
			//              RingBufWriteOne(&g_sRxBuf[sEvent.ui8Port], sEvent.ui8Char);

			//				UARTprintf("telet protocol: %X \n",(g_sParameters.sPort[sEvent.ui8Port].ui8Flags & PORT_FLAG_PROTOCOL));
		}

		//
		// If not a Telnet IAC character, only write it once.
		//
		else if((sEvent.ui8Char != TELNET_IAC)
				&&(RingBufFree(&g_sRxBuf[sEvent.ui8Port]) >= 1)
		)
		{
			//	while(RingBufFree(&g_sRxBuf[sEvent.ui8Port]) < 1){};
			/*
        	if((g_sParameters.modbus == Modbus_en)&&(sEvent.ui8Port == COM_Port))
        	{
                //
                // Write the data once.
                //
                ModbusTimerDisArm();
                ModbusRTURecvTCPSend(sEvent.ui8Char);
        	}
			 */
			//        	else{
			RingBufWriteOne(&g_sRxBuf[sEvent.ui8Port], sEvent.ui8Char);
			//       		 UARTprintf("%c",sEvent.ui8Char);
			//        	}
			//	 RingBufWriteOne(&g_sRxBuf[sEvent.ui8Port], sEvent.ui8Char);
		}
	}

	//
	// If not Telnet, then only write the data once.
	//
	else
	{
		//UARTprintf("not telnet:%c",sEvent.ui8Char);
		COM_Data_In_Handle_Direct(sEvent);
	}
}
void Origin_Data_StateMachine(tSerialEvent sEvent)
{

	switch(WiFi_Origin_TCP_Status)
	{
	case WiFi_Origin_Char1:
		if(sEvent.ui8Char == '~')
		{
			WiFi_Origin_TCP_Status = WiFi_Origin_Char2;
		}
		else if(sEvent.ui8Char == 'G')
		{

			WiFi_Origin_TCP_Status = WiFi_Origin_Char3;
		}
		else //raw data,need to fill in ringbuffer;
		{
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			WiFi_Origin_TCP_Status = WiFi_Origin_Char1;
			// TCPChar4 need to jump tp TCPChar1;
			TCPChar_Status = TCPChar1;
		}
		break;
	case WiFi_Origin_Char2:
		if(sEvent.ui8Char == 'c')
		{
			//	Webserver_Data_In_Processing_TCPChar(sEvent);
			WiFi_Origin_TCP_Status = WiFi_Origin_Char4;
		}

		else{
			//		while(RingBufFree(&g_sRxBuf[WiFi_Port]) < 1){};
			// send char;
			RingBufWriteOne(&g_sRxBuf[WiFi_Port], '~');
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			WiFi_Origin_TCP_Status = WiFi_Origin_Char1;
			// TCPChar4 need to jump tp TCPChar1;
			TCPChar_Status = TCPChar1;
		}
		break;
	case WiFi_Origin_Char3:
		if(sEvent.ui8Char == 'E')
		{
			//	Webserver_Data_In_Processing_TCPChar(sEvent);
			WiFi_Origin_TCP_Status = WiFi_Origin_Char5;
		}

		else{
			//		while(RingBufFree(&g_sRxBuf[WiFi_Port]) < 1){};
			// send char;
			RingBufWriteOne(&g_sRxBuf[WiFi_Port], 'G');
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			WiFi_Origin_TCP_Status = WiFi_Origin_Char1;
			// TCPChar4 need to jump tp TCPChar1;
			TCPChar_Status = TCPChar1;
		}
		break;
	case WiFi_Origin_Char4:
		if(sEvent.ui8Char == '~')
		{
			// set flag tcp close;
			UARTprintf("\ntcp closed\n");
			WiFi_TCP_connected = false;
			Webpage_Visit_Req = false;
			WiFi_Origin_TCP_Status = WiFi_Origin_Char1;
			TCPChar_Status = TCPChar1;
		}
		else{
			//	while(RingBufFree(&g_sRxBuf[WiFi_Port]) < 2){};
			// send char;
			RingBufWriteOne(&g_sRxBuf[WiFi_Port], '~');
			RingBufWriteOne(&g_sRxBuf[WiFi_Port], 'c');
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			WiFi_Origin_TCP_Status = WiFi_Origin_Char1;
			// TCPChar4 need to jump tp TCPChar1;
			TCPChar_Status = TCPChar1;
		}
		break;
	case WiFi_Origin_Char5:
		if(sEvent.ui8Char == 'T')
		{
			// set visit webpage flag;
			WiFi_TCP_connected = true;
			Webpage_Visit_Req = true;
			UARTprintf("\nweb1\n");
			//send '~o~'first;
			RingBufWriteOne(&wifi_pageRxBuf, '~');
			RingBufWriteOne(&wifi_pageRxBuf, 'o');
			RingBufWriteOne(&wifi_pageRxBuf, '~');
			//send 'GET';
			RingBufWriteOne(&wifi_pageRxBuf, 'G');
			RingBufWriteOne(&wifi_pageRxBuf, 'E');
			RingBufWriteOne(&wifi_pageRxBuf, 'T');
			WiFi_Origin_TCP_Status = WiFi_Origin_Char1;
			TCPChar_Status = TCPChar1;
		}
		else{
			//it's not webpage visit request;it's user data,need to send to wifi RX ringbuffer;
			WiFi_TCP_connected = true;
			Webpage_Visit_Req = false;
			UARTprintf("\nweb0\n");
			//send '~o~'first;
			//		 RingBufWriteOne(&g_sRxBuf[WiFi_Port], '~');
			//		 RingBufWriteOne(&g_sRxBuf[WiFi_Port], 'o');
			//		 RingBufWriteOne(&g_sRxBuf[WiFi_Port], '~');
			RingBufWriteOne(&wifi_pageRxBuf, 'G');
			RingBufWriteOne(&wifi_pageRxBuf, 'E');
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			WiFi_Origin_TCP_Status = WiFi_Origin_Char1;
			// TCPChar4 need to jump tp TCPChar1;
			TCPChar_Status = TCPChar1;
		}
		break;



	default: ;break;

	}
}
void WiFi_TCP_Open_StatMachine(tSerialEvent sEvent)
{
	//	char web_check_string[] = "GET";
	switch(TCPChar_Status)
	{
	case TCPChar1:
		if(sEvent.ui8Char == '~')
		{
			TCPChar_Status = TCPChar2;
			previous_char ='\0';
			//	 Webserver_Data_In_Processing_TCPChar(sEvent);

		}
		else
		{
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			TCPChar_Status = TCPChar1;
		}
		break;
	case TCPChar2:
		if((sEvent.ui8Char == 'o')||(sEvent.ui8Char == 'c'))
		{
			//	Webserver_Data_In_Processing_TCPChar(sEvent);
			TCPChar_Status = TCPChar3;
			previous_char = sEvent.ui8Char;

		}

		else{
			while(RingBufFree(&g_sRxBuf[WiFi_Port]) < 1){};
			// send char;
			RingBufWriteOne(&g_sRxBuf[WiFi_Port], '~');
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			TCPChar_Status = TCPChar1;
		}
		break;
	case TCPChar3:
		if(sEvent.ui8Char == '~')
		{

			if(previous_char == 'o')
			{
				//	Webserver_Data_In_Processing_TCPChar(sEvent);
				TCPChar_Status = TCPChar4;
				WiFi_Origin_TCP_Status = WiFi_Origin_Char1;
				UARTprintf("\ntcp op\n");
				previous_char ='\0';
				// need to flush wifi webpage ringbuffer;
				//					 RingBufFlush(&wifi_pageRxBuf);
				//send '~o~';
				/*
					 RingBufWriteOne(&wifi_pageRxBuf, '~');
					 RingBufWriteOne(&wifi_pageRxBuf, 'o');
					 RingBufWriteOne(&wifi_pageRxBuf, '~');
				 */
				cnt_data_or_web_request = 0;

			}
			else if(previous_char == 'c')
			{
				//initialize statemachine;
				TCPChar_Status = TCPChar1;
				//	 WiFi_TCP_connected = false;
				//	 UARTprintf("\ntcp cl\n");
				//	 Webserver_Data_In_Processing_TCPChar(sEvent);
				//	 httpState = HTTP_CONN_WAIT;

				RingBufWriteOne(&wifi_pageRxBuf, '~');
				RingBufWriteOne(&wifi_pageRxBuf, 'c');
				RingBufWriteOne(&wifi_pageRxBuf, '~');

				previous_char ='\0';
				//	 wifibuffer_used= RingBufUsed(&wifi_pageRxBuf);
				//	 UARTprintf("\nleft>:%d\n",wifibuffer_used);
				//	 UARTprintf("\n>>:%d\n",TCPChar_Status);
			}




		}
		else
		{
			// need send previous '~o' first;

			while(RingBufFree(&g_sRxBuf[WiFi_Port]) < 2){};
			// send char;
			RingBufWriteOne(&g_sRxBuf[WiFi_Port], '~');
			RingBufWriteOne(&g_sRxBuf[WiFi_Port], 'o');
			// set TCP open flag; send '~o~' string to wifi ringbuffer;
			// the later WiFi_WebAccess task will recognize '~c~' string; if true, WiFi_TCP_connected flag invalid;
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			TCPChar_Status = TCPChar1;

		}

		break;
	case TCPChar4:
		//judge "GET" internet explorer request mark; whether it's a webpage request;
		Origin_Data_StateMachine(sEvent);
		break;

	default: ;break;

	}
}



void WiFiRAW_Data_StateMachine(tSerialEvent sEvent)
{

	switch(WiFi_Raw_TCP_Status)
	{
	case WiFi_Raw_Char1:
		if(sEvent.ui8Char == '~')
		{
			WiFi_Raw_TCP_Status = WiFi_Raw_Char2;
		}
		else
		{
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			WiFi_Raw_TCP_Status = WiFi_Raw_Char1;
		}
		break;
	case WiFi_Raw_Char2:
		if(sEvent.ui8Char == 'c')
		{
			//	Webserver_Data_In_Processing_TCPChar(sEvent);
			WiFi_Raw_TCP_Status = WiFi_Raw_Char3;
		}

		else{
			while(RingBufFree(&g_sRxBuf[WiFi_Port]) < 1){};
			// send char;
			RingBufWriteOne(&g_sRxBuf[WiFi_Port], '~');
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			WiFi_Raw_TCP_Status = WiFi_Raw_Char1;
		}
		break;
	case WiFi_Raw_Char3:
		if(sEvent.ui8Char == '~')
		{
			// set flag tcp close;
			WiFi_TCP_connected = false;
			WiFi_Raw_TCP_Status = WiFi_Raw_Char1;
		}
		else{
			while(RingBufFree(&g_sRxBuf[WiFi_Port]) < 2){};
			// send char;
			RingBufWriteOne(&g_sRxBuf[WiFi_Port], '~');
			RingBufWriteOne(&g_sRxBuf[WiFi_Port], 'c');
			COM_Data_In_Processing_Not_TCPChar(sEvent);
			WiFi_Raw_TCP_Status = WiFi_Raw_Char1;
		}
		break;

	default: ;break;

	}
}
//*****************************************************************************
//
//! The Serial task handles data flow from the serial interrupt handler(s) to
//! the ring buffers and vice versa.
//!
//! On receiving RX interrupt, the interrupt handler passes the data received
//! to the serial task using Queue.  The serial task copies this data into the
//! ring buffer.
//!
//! On receiving a TX interrupt, indicating that space is available in TX FIFO,
//! the interrupt handler passes this information to the serial task using
//! Queue.  The serial task writes data from the ring buffer to TX FIFO.
//!
//
//*****************************************************************************
static void
SerialTask(void *pvParameters)
{
	tSerialEvent sEvent;
	//    uint8_t ui8Temp;

	// Loop forever.
	WiFi_TCP_connected = false;
	Webpage_Visit_Req = false;
	TCPChar_Status = TCPChar1;
	WiFi_Raw_TCP_Status = WiFi_Raw_Char1;

	//    modbusRTUState2 = MODBUS_RTU_STATE_INIT;
	modbusRTUState3 = MODBUS_RTU_STATE_INIT;
	modbusRTUState_LAN = MODBUS_RTU_STATE_INIT;
	i32IndexRTU = 0;
	//    pui8TempRTU[256] = {0};

	while(1)
	{
		// Block until a message is put on the g_QueSerial queue by the
		// interrupt handler.
		xQueueReceive(g_QueSerial, (void*) &sEvent, portMAX_DELAY);
		// The first part of the queue message is the type of event.  Check if
		// it's an RX or receive timeout interrupt.
		if(sEvent.eEventType == RX)
		{
			if(sEvent.ui8Port == WiFi_Port)
			{
				/*
    			  if((sEvent.ui8Char==0x0A)||(sEvent.ui8Char==0x0D))
    			  UARTprintf("r%02X",sEvent.ui8Char);
    			  else UARTprintf("%c",sEvent.ui8Char);
				 */
				//    			  UARTprintf("r%c",sEvent.ui8Char);
				//    			  UARTprintf("\n::%d\n",WiFi_TCP_connected);
				//    			  UARTprintf("\n>:%d\n",Webpage_Visit_Req);
				//    			  UARTprintf("\nW=%d\n",Switch_WiFi_to_COM);
				//    			  UARTprintf("\nL=%d\n",Switch_LAN_to_COM);
				//    			  UARTprintf("\nS=%d\n",g_sParameters.datachansel);
				//	  UARTprintf("data from Wifi-port");

				if(WiFi_TCP_connected == false) //cmd control wifi module;
				{

					if(WiFi_Setting)
					{
						Switch_WiFi_to_COM = 0;
						//        			 	  UARTprintf("r%02X",sEvent.ui8Char);
						//	  UARTprintf("\nrestor=%d\n",wifi_restoring);
						COM_Data_In_Processing_Not_TCPChar(sEvent);
					}
					else //raw data, need judge tcp connection and webpage requestion;
					{
						//			  UARTprintf("\n=%d\n",TCPChar_Status);
						WiFi_TCP_Open_StatMachine(sEvent);
					}

				}
				else // tcp connectted, true;
				{
					if(Webpage_Visit_Req == false) // not webpage request;is user data receiving;
					{
						if(WiFi_Setting)
						{
							Switch_WiFi_to_COM = 0;
							//	  UARTprintf("\nrestor=%d\n",wifi_restoring);
							COM_Data_In_Processing_Not_TCPChar(sEvent);
						}

						else
							// send to wifi ringbuffer;
							//raw user data;
							//need a statemachine to judge tcp close;
							WiFiRAW_Data_StateMachine(sEvent);

					}
					else // is webpage request;
					{
						if(WiFi_Setting)

						{
							Switch_WiFi_to_COM = 0;
							//		  UARTprintf("\ncmd=%d\n",wifiCMD_Executing);
							// send to wifi ringbuffer;
							COM_Data_In_Processing_Not_TCPChar(sEvent);
						}

						else
						{ // handle webpage contents;

							Webserver_Data_In_Processing_TCPChar(sEvent);

						}
					}

				}

			}
			else // not wifi port;
			{
				//UARTprintf("r%02X",sEvent.ui8Char);
				COM_Data_In_Processing_Not_TCPChar(sEvent);
			}

		}

		// Check if it's a TX interrupt, indicating that there is space
		// available in TX FIFO.

		//if tcp_conn is valid, COM to WiFi data tx path close; else ok;
		else
		{
			UARTprintf("tx:%d",sEvent.ui8Char);
			if(sEvent.ui8Port==WiFi_Port)
			{
				if(WiFi_TCP_connected == true)
				{
					if(Webpage_Visit_Req == false) //not webpage;
					{
						COM_Data_Out_Sending_Not_TCPChar(sEvent);
						//	  UARTprintf("1%c",sEvent.ui8Char);
					}
					else //webpage tx to wifi module;
					{
						//if restore mode;
						if(WiFi_Setting) // send wifi ringbuffer data to wifi port;
						{
							Switch_WiFi_to_COM = 0;
							COM_Data_Out_Sending_Not_TCPChar(sEvent);
							// send webpage data to wifi port
						}

						else //if(wifi_sendpage)
						{
							Webserver_Data_Out_Processing_TCPChar(sEvent);
						}

					}
				}
				// nothing; wifi tx data used by wifi only;
				else if(WiFi_TCP_connected == false)
					// keep COM to WiFi data path;
				{
					//    				 UARTprintf("\ncmd=%d\n",wifiCMD_Executing);
					//if resore mode;
					if(WiFi_Setting) // send wifi ringbuffer data to wifi port;
					{
						Switch_WiFi_to_COM = 0;
						COM_Data_Out_Sending_Not_TCPChar(sEvent);
						// send webpage data to wifi port
					}

					else if(wifi_sendpage)
						Webserver_Data_Out_Processing_TCPChar(sEvent);
					else
						COM_Data_Out_Sending_Not_TCPChar(sEvent);

				}
			}
			else
			{
				COM_Data_Out_Sending_Not_TCPChar(sEvent);
			}

		}

	}

}

//*****************************************************************************
//
// Initialize the serial peripherals and the serial task.
//
//*****************************************************************************
uint32_t
SerialTaskInit(void)
{
	//
	// Initialize the serial port module.
	//
	SerialInit();
	//
	// Set the interrupt priorities.  As the UART interrupt handlers use
	// FreeRTOS APIs, the interrupt priorities of the UARTs should be between
	// configKERNEL_INTERRUPT_PRIORITY and
	// configMAX_SYSCALL_INTERRUPT_PRIORITY.
	//
	//    MAP_IntPrioritySet(INT_UART3, UART3_INT_PRIORITY);
	MAP_IntPrioritySet(INT_UART1, UART1_INT_PRIORITY); //com, port2
	MAP_IntPrioritySet(INT_UART2, UART2_INT_PRIORITY); //com, port0
	MAP_IntPrioritySet(INT_UART4, UART4_INT_PRIORITY); //com, port1
	/*
    if(g_sParameters.ui32WiFiModuleBaudRate==9600)
    {
    	// hadware timing for wifi factory resuming;
    	_UR4_SerialSetConfig(9600);
        wifimoduledefault();
    }
	 */
	//
	// Create a queue to pass information from the UART interrupt handlers to
	// the serial task.  This queue is also used to signal the serial task that
	// an event has occurred.  A queue is used, instead of a Semaphore, as data
	// has to be sent from the interrupt handler to the task and also because
	// the frequency of the UART interrupts could be quite high.  The queue
	// size can be adjusted based on the UART baud rate.
	//
	g_QueSerial = xQueueCreate(512, sizeof(tSerialEvent)); // change from 64 to 128
	if(g_QueSerial == 0)
	{
		return(1);
	}

	//
	// Create the Serial task.
	//
	if(xTaskCreate(SerialTask, (signed portCHAR *)"Serial",
			STACKSIZE_SerialTASK, NULL, tskIDLE_PRIORITY +
			PRIORITY_SERIAL_TASK, NULL) != pdTRUE)
	{
		return(1);
	}

	//
	// Success.
	//
	return(0);
}
