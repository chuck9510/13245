#include <stdbool.h>
#include <stdint.h>

#include "user_define.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "utils/ringbuf.h"
#include "modbus.h"
#include "modbusRTU.h"


#if MODBUS_SUPPORT
extern unsigned char g_mbapTransactionIdentifier[];
extern unsigned char g_mbapProtocolIdentifier[];
extern unsigned int  g_mbapADULength[];
extern unsigned char g_mbapSlaveAddress;
extern unsigned char g_mbapFunctionId;
//extern tRingBufObject g_sRxBuf;
extern tRingBufObject g_sRxBuf[MAX_S2E_PORTS];
extern uint32_t g_ui32SysClock;






int i = 0;

void ModbusTimerInit()
{
	uint32_t time_interval_1d5_char;
	g_enableTimer = 0;
	//
    // COnfiguring the Timer0
	//
	time_interval_1d5_char = 12 * 1.5 / g_sParameters.sPort[0].ui32BaudRate *g_ui32SysClock;
	UARTprintf("t_1d5_char 0x%X\n",time_interval_1d5_char);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	SysCtlDelay(200);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); //Full-width periodic timer

//	TimerLoadSet(TIMER0_BASE, TIMER_A, g_ui32SysClock*25/20);	 // to configure for 50msec--> 150ms
	TimerLoadSet(TIMER0_BASE, TIMER_A, g_ui32SysClock);
//	TimerLoadSet(TIMER0_BASE, TIMER_A, time_interval_1d5_char*20*10); // *20 is 3 ms; *10 ,1.5ms o/k
//	TimerLoadSet(TIMER0_BASE, TIMER_A, time_interval_1d5_char*20);//ok
//	TimerLoadSet(TIMER0_BASE, TIMER_A, time_interval_1d5_char*10);//
//	TimerLoadSet(TIMER0_BASE, TIMER_A, time_interval_1d5_char);
	// if wifi port baudrate 230400; 1.5 char time: 12bit*1.5/230400; convert to times, x/120Mhz = 1.5char/1s; so, x = 1.5char_time * 120MHz;
	
	 //
    // Setup the interrupts for the timer timeouts.
    //
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
   
}

void WiFi_RTU_ModbusTimer3Init()
{
	uint32_t time_interval_1d5_char;
	g_WiFi_RTU_enableTimer = 0;
	//
    // COnfiguring the Timer0
	//
	time_interval_1d5_char = 12 * 1.5 / g_sParameters.ui32WiFiModuleBaudRate *g_ui32SysClock;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
	SysCtlDelay(200);
	TimerConfigure(TIMER3_BASE, TIMER_CFG_PERIODIC); //Full-width periodic timer

//	TimerLoadSet(TIMER3_BASE, TIMER_A, g_ui32SysClock*15/20);	 // to configure for 50msec--> 150ms , need to set 1.5char between char, over 3.5char between frames;
//	TimerLoadSet(TIMER3_BASE, TIMER_A, g_ui32SysClock*12*1.5/230400); //9735
//	TimerLoadSet(TIMER3_BASE, TIMER_A, 93750*2); //9735 *2 = 18750, 1.56mS;
	TimerLoadSet(TIMER3_BASE, TIMER_A, time_interval_1d5_char*10);
// if wifi port baudrate 230400; 1.5 char time: 12bit*1.5/230400; convert to times, x/120Mhz = 1.5char/1s; so, x = 1.5char_time * 120MHz;
	 //
    // Setup the interrupts for the timer timeouts.
    //
    IntEnable(INT_TIMER3A);
    TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);

}
void WiFi_RTU_ModbusTimer3Arm(void)
{
 	// Enable the timers.
    //

   TimerEnable(TIMER3_BASE, TIMER_A);
   g_WiFi_RTU_enableTimer = 1;

}

void WiFi_RTU_ModbusTimer3DisArm(void)
{
	// Disableable the timers.
    //
   TimerDisable(TIMER3_BASE, TIMER_A);
   g_WiFi_RTU_enableTimer = 0;

}


void ModbusTimerArm(void)
{
 	// Enable the timers.
    //

   TimerEnable(TIMER0_BASE, TIMER_A);
   g_enableTimer = 1;

}

void ModbusTimerDisArm(void)
{
	// Disableable the timers.
    //
   TimerDisable(TIMER0_BASE, TIMER_A);
   g_enableTimer = 0;

}

void
Timer3IntHandler(void)
{
    // Clear the timer interrupt.
    //
    TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    //
  	//

//    UARTprintf("tmr3 triger\n\n");

    if(g_sParameters.datachansel==1) // 1:LAN_COM
    {
    	switch(modbusRTUState_LAN) //wifi-com;
    		{
    			case  MODBUS_RTU_STATE_IDLE:
    			case  MODBUS_RTU_STATE_INIT:
    			g_time3Out++;
    	//		UARTprintf("g_time3Out 0x%X\n",g_time3Out);
    			if(g_time3Out > 0xF) //change 0xF to 0xFF;
    				{
    	//			 UARTprintf("g_time3Out 0x%X > 0xF\n",g_time3Out);
    				g_time3Out = 0;

    				WiFi_RTU_ModbusTimer3DisArm();
    				modbusRTUState_LAN = MODBUS_RTU_STATE_INIT;

    				}
    						break;
    			default:
    	//			UARTprintf("modbusRTUState3 isn't idle or init\n");
    				WiFi_RTU_ModbusTimer3DisArm();
    				modbusRTUState_LAN = MODBUS_RTU_STATE_INIT;
    					  	break;
    		}
    }
    else if(g_sParameters.datachansel==2) // wifi to com;
	{
    	switch(modbusRTUState3) //wifi-com;
    		{
    			case  MODBUS_RTU_STATE_IDLE:
    			case  MODBUS_RTU_STATE_INIT:
    			g_time3Out++;
    	//		UARTprintf("g_time3Out 0x%X\n",g_time3Out);
    			if(g_time3Out > 0xF) //change 0xF to 0xFF;
    				{
    	//			 UARTprintf("g_time3Out 0x%X > 0xF\n",g_time3Out);
    				g_time3Out = 0;

    				WiFi_RTU_ModbusTimer3DisArm();
    				modbusRTUState3 = MODBUS_RTU_STATE_INIT;

    				}
    						break;
    			default:
    	//			UARTprintf("modbusRTUState3 isn't idle or init\n");
    				WiFi_RTU_ModbusTimer3DisArm();
    				modbusRTUState3 = MODBUS_RTU_STATE_INIT;
    					  	break;
    		}
	}




}

void
Timer0IntHandler(void)
{
    // Clear the timer interrupt.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    //
  	//
//    UARTprintf("tmr0 triger\n\n");

        	if(
        		//	(g_sParameters.modbus == Modbus_en) ||
        		((g_sParameters.sPort[COM_Port].ui8Flags &
        				PORT_PROTOCOL_MODBUS) == PORT_MODBUS_RTU_TCP) // 1:LAN_COM
    					)//modbus_en;

    {
        		switch(modbusRTUState)
        		{
        			case  MODBUS_RTU_STATE_IDLE:
        			case  MODBUS_RTU_STATE_INIT:
        			g_timeOut++;
//        			UARTprintf("t:0x%X\n",g_timeOut);
//        			if(g_timeOut > 0xF) //change 0xF to 0xFF;
        				if(g_timeOut > 0x5) //change 0xF to 0xFF;
        				{
        	//			 UARTprintf("g_timeOut 0x%X > 0xF\n",g_timeOut);
        				g_timeOut = 0;
        			if(!CheckModbusTCPStatemachine())//wait client stae, enter to excute reset;
        					{
//        				UARTprintf("rst\n");
        						ModbusTimerDisArm();
        						ResetModbusRTUStatemachine();
        						ResetModbusTCPStatemachine();
        					}
        				}
        						break;
        			default:
        	//			UARTprintf("modbusRTUState isn't idle or init\n");
        						ModbusTimerDisArm();
        						ResetModbusRTUStatemachine();
        						ResetModbusTCPStatemachine();
        					  	break;
        		}
    }
	
}

void CheckModbusRTUStatemachine()
{
if(modbusRTUState ==  MODBUS_RTU_STATE_FRAME_START)
	ResetModbusRTUStatemachine();
}

void SetInitModbusRTUStatemachine(void)
	{
	 modbusRTUState = MODBUS_RTU_STATE_INIT;
	}

 void ResetModbusRTUStatemachine()
{
 modbusRTUState = MODBUS_RTU_STATE_INIT;
}

void initModbusRTUStatemachine()
{
 	int i;

 	g_serialRecvCount = 0;
 	g_slaveAddressRTU = 0x0;
  	g_funcIDRTU = 0x0;
	g_frameLengthRTU = 0x0;
	g_frameDataCount = 0;

 for(i=0;i<RTU_MAX_LENGTH;i++)
 	  g_mbapRTU[i] = 0;
 g_mbapRTU[0] = g_mbapTransactionIdentifier[0];
 g_mbapRTU[1] = g_mbapTransactionIdentifier[1];

 g_mbapRTU[2] = g_mbapProtocolIdentifier[0];
 g_mbapRTU[3] = g_mbapProtocolIdentifier[1];

 g_mbapRTU[4] = g_mbapADULength[0];
 g_mbapRTU[5] = g_mbapADULength[1];

 g_serialRecvCount = 6;
	g_crcRTU[0] = 1;
 	g_crcRTU[1] = 0;

	g_timeOut = 0;

 if(!(!g_mbapRTU[2] && !g_mbapRTU[3]))
 {
 // Protocol ID not valid
  modbusRTUState = MODBUS_RTU_STATE_INVALID;
  return;
 }

 

}



void ModbusRTURecvTCPSend(unsigned char ucChar)
{
	//Enable timer for 3.5 char
	 

	switch(modbusRTUState)
	{
		case  MODBUS_RTU_STATE_IDLE:
			UARTprintf("idle\n");
					break;
		case  MODBUS_RTU_STATE_INIT:
//			UARTprintf("init\n");
				initModbusRTUStatemachine();
				modbusRTUState = MODBUS_RTU_STATE_SLAVE_ID;
		
		case MODBUS_RTU_STATE_SLAVE_ID:
//			UARTprintf("sid\n");
					i = 0;			
					g_slaveAddressRTU = ucChar;
//					UARTprintf("sid:0x%02X\n",ucChar);
					if(	g_slaveAddressRTU == g_mbapSlaveAddress)
					{
//						UARTprintf("sid match\n");
				 	g_mbapRTU[g_serialRecvCount] = ucChar;
					g_serialRecvCount++;
					modbusRTUState = MODBUS_RTU_STATE_FUNC_ID;
					ModbusTimerArm();
					}
					else
					{
					 modbusRTUState = MODBUS_RTU_STATE_IDLE;
					 ResetModbusTCPStatemachine();
					 }
					break;
		case MODBUS_RTU_STATE_FUNC_ID:
//			UARTprintf("fid\n");
					g_funcIDRTU = ucChar;
//					UARTprintf("func id:%d\n",ucChar);
					if(	g_funcIDRTU == g_mbapFunctionId)
					{
//						UARTprintf("func id match:%d\n",g_mbapFunctionId);
				 	g_mbapRTU[g_serialRecvCount] = ucChar;
					g_serialRecvCount++;
						if(g_funcIDRTU == MODBUS_READ_COIL || 
							g_funcIDRTU == MODBUS_READ_DISCRETE_INPUT ||
							g_funcIDRTU == MODBUS_READ_HOLDING_REGISTERS ||
							g_funcIDRTU ==  MODBUS_READ_INPUT_REGISTERS)
							{
								modbusRTUState = MODBUS_RTU_STATE_FRAME_LENGTH;
								ModbusTimerArm();
							}
						else if(g_funcIDRTU == MODBUS_WRITE_SINGLE_COIL || 
							g_funcIDRTU == MODBUS_WRITE_SINGLE_REGISTERS ||
							g_funcIDRTU == MODBUS_WRITE_MULTIPLE_COILS ||
							g_funcIDRTU ==  MODBUS_WRITE_MULTIPLE_REGISTERS)
							{
								g_frameLengthRTU = 4;
								modbusRTUState = MODBUS_RTU_STATE_FRAME_START;
								ModbusTimerArm();	
							}
						else
						{
							 modbusRTUState = MODBUS_RTU_STATE_IDLE;
						ResetModbusTCPStatemachine();
						}

					}
					else
					{
					 modbusRTUState = MODBUS_RTU_STATE_IDLE;

					 ResetModbusTCPStatemachine();
					}
					break;
		case MODBUS_RTU_STATE_FRAME_LENGTH:
//			UARTprintf("flen\n");
					g_frameLengthRTU = ucChar;
//					UARTprintf("frame length:%d\n",ucChar);
					if(	g_frameLengthRTU < (RTU_MAX_LENGTH - 4))
					{

				 	g_mbapRTU[g_serialRecvCount] = ucChar;
//				 	UARTprintf(":%d\n",ucChar);
					g_serialRecvCount++;
					modbusRTUState = MODBUS_RTU_STATE_FRAME_START;
					ModbusTimerArm();
					}
					else
					{
					 modbusRTUState = MODBUS_RTU_STATE_IDLE;
					 ResetModbusTCPStatemachine();
					 }
					 break;
		case MODBUS_RTU_STATE_FRAME_START:
//			UARTprintf("fsat\n");
					g_mbapRTU[g_serialRecvCount] = ucChar;
					g_serialRecvCount++;
					g_frameDataCount++;
//					UARTprintf("framedata count :%d,g_frameLengthRTU:%d \n",g_frameDataCount,g_frameLengthRTU);
					if(	g_frameDataCount >= g_frameLengthRTU)
					{
//						UARTprintf("data rec end,enter crc1\n");
				 	modbusRTUState = MODBUS_RTU_STATE_FRAME_CRC1;
					}
					ModbusTimerArm();
					 break;
		
		case  MODBUS_RTU_STATE_FRAME_CRC1:
//			UARTprintf("fcrc1\n");
					g_crcRTU[0] = ucChar;
//					UARTprintf("crc1 :%d\n",ucChar);
					g_serialRecvCount++		;
					modbusRTUState = MODBUS_RTU_STATE_FRAME_CRC2;
					ModbusTimerArm();	
					break;

		case  MODBUS_RTU_STATE_FRAME_CRC2:
//			UARTprintf("fcrc2\n");
					g_crcRTU[1] = ucChar;
//					UARTprintf("crc2 :%d\n",ucChar);
					g_serialRecvCount++;
					modbusRTUState = MODBUS_RTU_STATE_FRAME_SENT;	
					
		case  MODBUS_RTU_STATE_FRAME_SENT:
//			UARTprintf("fsent\n");
					// Send the Frame 	g_mbapRTU
//			UARTprintf("data to be wr to ringbuffer:\n");
						for(i = 0; i < (g_serialRecvCount-2); i++)
                       	{
							/*
						 if((RingBufFree(&g_sRxBuf[COM_Port]) >= 1))
						 	{
						//	 UARTprintf("num:%d, value:%d\n",i,g_mbapRTU[i]);
							RingBufWriteOne(&g_sRxBuf[COM_Port], g_mbapRTU[i]);//vivek
							*/

						 if((RingBufFree(&Data_From_COM_To_LAN) >= (g_serialRecvCount-2)))
						 	{
						//	 UARTprintf("num:%d, value:%d\n",i,g_mbapRTU[i]);
							RingBufWriteOne(&Data_From_COM_To_LAN, g_mbapRTU[i]);//vivek
//							SerialSend(WiFi_Port, g_mbapRTU[i]);	// wifi modbus rtu to tcp, not applicable;


//							 RS485_TxEnable();
//							 SysCtlDelay(20);
//							 UARTwrite(&g_sRxBuf[COM_Port], g_mbapRTU[i]);
//							 SysCtlDelay(20);
//							 RS485_TxDisable();
							 //send(g_mbapRTU, RTU_MAX_LENGTH);
							}
                       	}	
//						UARTprintf("\n");
					modbusRTUState = MODBUS_RTU_STATE_IDLE;
					ResetModbusTCPStatemachine();
					break;

		case  MODBUS_RTU_STATE_INVALID:
			UARTprintf("invalid id\n");
		default:
		{
			ResetModbusRTUStatemachine();
		}
//			UARTprintf("default,reset\n");

								break;
	}
}

// new function;
void ModbusRTURecv_COM_Port(unsigned char ucChar)
{

	switch(modbusRTUState2)
	{

		case  MODBUS_RTU_STATE_IDLE:									//UARTprintf("\nMODBUS_RTU_STATE_IDLE\n");
				//do nothing
			UARTprintf("\nidle\n");
					break;


		case  MODBUS_RTU_STATE_INIT:									//UARTprintf("MODBUS_RTU_STATE_INIT\n");
				//initModbusRTUStatemachine();
				g_frameDataCount2 = 0;
				i32IndexRTU = 0;
				Qty_send = 0;
				modbusRTUState2 = MODBUS_RTU_STATE_SLAVE_ID;
//				UARTprintf("\nini:%d\n",ucChar);
//				ModbusTimerArm();
			//	break;

		case MODBUS_RTU_STATE_SLAVE_ID:									//UARTprintf("MODBUS_RTU_STATE_SLAVE_ID\n");

//						while(!SerialReceiveAvailable(pState->ui32SerialPort))		vTaskDelay( 10/portTICK_RATE_MS );
				//		pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);
						pui8TempRTU[i32IndexRTU] = ucChar;
						i32IndexRTU++;
						modbusRTUState2 = MODBUS_RTU_STATE_FUNC_ID;
//						UARTprintf("\nsid:%X\n",ucChar);
						ModbusTimerArm();
					break;

		case MODBUS_RTU_STATE_FUNC_ID:									//UARTprintf("MODBUS_RTU_STATE_FUNC_ID\n");
			//		while(!SerialReceiveAvailable(pState->ui32SerialPort))	vTaskDelay( 10/portTICK_RATE_MS );
			//		pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);
					pui8TempRTU[i32IndexRTU] = ucChar;
					g_funcIDRTU2 = pui8TempRTU[i32IndexRTU];
					i32IndexRTU++;
//					UARTprintf("\nfid\n");
						if( g_funcIDRTU2 == MODBUS_READ_COIL ||
							g_funcIDRTU2 == MODBUS_READ_DISCRETE_INPUT ||
							g_funcIDRTU2 == MODBUS_READ_HOLDING_REGISTERS ||
							g_funcIDRTU2 == MODBUS_READ_INPUT_REGISTERS ||
							g_funcIDRTU2 == MODBUS_READ_FILE ||
							g_funcIDRTU2 == MODBUS_WRITE_FILE ||
							g_funcIDRTU2 == MODBUS_RST_LINK )

							{
								modbusRTUState2 = MODBUS_RTU_STATE_FRAME_LENGTH;
								ModbusTimerArm();
							}
						else if(g_funcIDRTU2 == MODBUS_WRITE_SINGLE_COIL ||
								g_funcIDRTU2 == MODBUS_WRITE_SINGLE_REGISTERS ||
								g_funcIDRTU2 == MODBUS_WRITE_MULTIPLE_COILS ||
								g_funcIDRTU2 == MODBUS_WRITE_MULTIPLE_REGISTERS ||
								g_funcIDRTU2 == MODBUS_WRITE_MULTIPLE_REGISTERS2 ||
								g_funcIDRTU2 == MODBUS_WRITE_MULTIPLE_COILS2 )

							{
								g_frameLengthRTU2 = 4;
								modbusRTUState2 = MODBUS_RTU_STATE_FRAME_START;
								ModbusTimerArm();

							}
						else
							{
							 	 modbusRTUState2 = MODBUS_RTU_STATE_IDLE;
							 	 UARTprintf("Fc\n");
							}

					break;

		case MODBUS_RTU_STATE_FRAME_LENGTH:								//UARTprintf("MODBUS_RTU_STATE_FRAME_LENGTH\n");
			//		while(!SerialReceiveAvailable(pState->ui32SerialPort))	vTaskDelay( 10/portTICK_RATE_MS );
			//		pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);
					pui8TempRTU[i32IndexRTU] = ucChar;
					g_frameLengthRTU2 = pui8TempRTU[i32IndexRTU];
					i32IndexRTU++;
					if(	g_frameLengthRTU2 < (RTU_MAX_LENGTH - 4))//251 FB
					{


						//UARTprintf("len:%d\n",g_frameLengthRTU2);
						modbusRTUState2 = MODBUS_RTU_STATE_FRAME_START;
//						UARTprintf("\nlen ok\n");
						ModbusTimerArm();
					}
					else
					{
						UARTprintf("too long\n");
						modbusRTUState2 = MODBUS_RTU_STATE_IDLE;
					}
					break;

		case MODBUS_RTU_STATE_FRAME_START:								//UARTprintf("MODBUS_RTU_STATE_FRAME_START\n");
			//		while(!SerialReceiveAvailable(pState->ui32SerialPort))	vTaskDelay( 10/portTICK_RATE_MS );
			//		pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);		//UARTprintf("rev %02x\n",pui8Temp[i32Index]);
					pui8TempRTU[i32IndexRTU] = ucChar;
					i32IndexRTU++;

					//g_mbapRTU[g_serialRecvCount] = ucChar;
					//g_serialRecvCount++;
					g_frameDataCount2++;

//					UARTprintf("\ndata_s\n");
					if(	g_frameDataCount2 >= g_frameLengthRTU2)
					{
						modbusRTUState2 = MODBUS_RTU_STATE_FRAME_CRC1;
						ModbusTimerArm();
//						UARTprintf("revd:%d\n",g_frameDataCount2);
//						UARTprintf("\ndata_e\n");
					}
					ModbusTimerArm();
					 break;

		case  MODBUS_RTU_STATE_FRAME_CRC1:								//UARTprintf("MODBUS_RTU_STATE_FRAME_CRC1\n");
				//	while(!SerialReceiveAvailable(pState->ui32SerialPort))	vTaskDelay( 10/portTICK_RATE_MS );
				//	pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);
					pui8TempRTU[i32IndexRTU] = ucChar;
			        i32IndexRTU++;
					//g_crcRTU[0] = ucChar;
					//g_serialRecvCount++ ;
//			        UARTprintf("\ncrc1\n");
					modbusRTUState2 = MODBUS_RTU_STATE_FRAME_CRC2;
					ModbusTimerArm();
					break;

		case  MODBUS_RTU_STATE_FRAME_CRC2:								//UARTprintf("MODBUS_RTU_STATE_FRAME_CRC2\n");
			//		while(!SerialReceiveAvailable(pState->ui32SerialPort))	vTaskDelay( 10/portTICK_RATE_MS );
			//		pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);
					pui8TempRTU[i32IndexRTU] = ucChar;
			        i32IndexRTU++;
					//g_crcRTU[1] = ucChar;
					//g_serialRecvCount++ ;
//			        UARTprintf("\ncrc2\n");
					modbusRTUState2 = MODBUS_RTU_STATE_FRAME_SENT;

		case  MODBUS_RTU_STATE_FRAME_SENT:								//UARTprintf("MODBUS_RTU_STATE_FRAME_SENT\n");

			//		tcp_write(pState->pConnectPCB, pui8Temp, i32IndexRTU, 1);//serial-->telnet
			Qty_send=i32IndexRTU;
				//	int i;	for(i=0; i < i32IndexRTU; i++)
			while((i32IndexRTU) != 0)
					{
						// received completed RTU frame, and write to LAN_to_COM Buffer in one time;
				if((g_sParameters.datachansel==1)) //lan to com;
						RingBufWriteOne(&Data_From_COM_To_LAN, pui8TempRTU[Qty_send-i32IndexRTU]);
				//		RingBufWriteOne(&g_sTxBuf[WiFi_Port], pui8TempRTU[Qty_send-i32IndexRTU]);
				else if((g_sParameters.datachansel==2))
						SerialSend(WiFi_Port, pui8TempRTU[Qty_send-i32IndexRTU]);
				else if((g_sParameters.datachansel==3))
				{
					RingBufWriteOne(&Data_From_COM_To_LAN, pui8TempRTU[Qty_send-i32IndexRTU]);
					SerialSend(WiFi_Port, pui8TempRTU[Qty_send-i32IndexRTU]);
				}
				//		UARTprintf("%02x ",pui8TempRTU[Qty_send-i32IndexRTU]);
						i32IndexRTU--;

					}
					//	RingBufWriteOne(&Data_From_COM_To_LAN, pui8TempRTU[Qty_send-i32IndexRTU]);
						//	UARTprintf("\n");

				//	UARTprintf("telnet send  len:%d\n",i32Index);

					modbusRTUState2 = MODBUS_RTU_STATE_IDLE;					//	UARTprintf("MODBUS_RTU_STATE_INIT\n\n");

			//		goto end;


		case  MODBUS_RTU_STATE_INVALID:									//UARTprintf("MODBUS_RTU_STATE_INVALID\n");
		default:														//UARTprintf("default\n");
				modbusRTUState2 = MODBUS_RTU_STATE_IDLE;
				break;
	}
}

// new function;
void ModbusPDU_Req_WiFi_Port(unsigned char ucChar)
{

	switch(modbusRTUState3)
	{

		case  MODBUS_RTU_STATE_IDLE:									//UARTprintf("\nMODBUS_RTU_STATE_IDLE\n");
				//do nothing
			UARTprintf("\nidle\n");
					break;


		case  MODBUS_RTU_STATE_INIT:									//UARTprintf("MODBUS_RTU_STATE_INIT\n");
				//initModbusRTUStatemachine();
				g_frameDataCount2 = 0;
				g_time3Out =0;
				i32IndexRTU = 0;
				Qty_send = 0;
				modbusRTUState3 = MODBUS_RTU_STATE_SLAVE_ID;
				WiFi_RTU_ModbusTimer3Arm();
//				UARTprintf("\ninit\n");
			//	break;

		case MODBUS_RTU_STATE_SLAVE_ID:									//UARTprintf("MODBUS_RTU_STATE_SLAVE_ID\n");

//						while(!SerialReceiveAvailable(pState->ui32SerialPort))		vTaskDelay( 10/portTICK_RATE_MS );
				//		pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);
						pui8TempRTU[i32IndexRTU] = ucChar;
						i32IndexRTU++;
						modbusRTUState3 = MODBUS_RTU_STATE_FUNC_ID;
						WiFi_RTU_ModbusTimer3Arm();
//						UARTprintf("\nsid\n");
					break;

		case MODBUS_RTU_STATE_FUNC_ID:									//UARTprintf("MODBUS_RTU_STATE_FUNC_ID\n");
			//		while(!SerialReceiveAvailable(pState->ui32SerialPort))	vTaskDelay( 10/portTICK_RATE_MS );
			//		pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);
					pui8TempRTU[i32IndexRTU] = ucChar;
					g_funcIDRTU2 = pui8TempRTU[i32IndexRTU];
					i32IndexRTU++;
//					UARTprintf("\nfid\n");
					/*
					 0x01 read coil: fix len,4;
					 0x03 read holding reg: fix len,4;
					 0x06 write single reg: fix len 4;

					 0x0F write multi coil 5+n,byte 6;
					 0x10 writ multi reg: 5+2n, byte 6;
					 0x14 read file records: len,third byte;
					 0x15 write file records:len,third byte;
					 * */
					if(
						g_funcIDRTU2 == MODBUS_READ_FILE ||						//0x14
						g_funcIDRTU2 == MODBUS_WRITE_FILE 						//0x15
				//		g_funcIDRTU2 == MODBUS_RST_LINK 						//0x19
						)

						{
							modbusRTUState3 = MODBUS_RTU_STATE_FRAME_LENGTH;
							WiFi_RTU_ModbusTimer3Arm();
						}
					else if(
							g_funcIDRTU2 == MODBUS_WRITE_MULTIPLE_COILS2 	||		//0x0F
							g_funcIDRTU2 == MODBUS_WRITE_MULTIPLE_REGISTERS2 		//0x10
					)
					{
							modbusRTUState3 = MODBUS_RTU_STATE_FRAME_LENGTH_BEFORE;
							WiFi_RTU_ModbusTimer3Arm();
							cnt_data_for_get_len = 0;
					}
					else if(
							g_funcIDRTU2 == MODBUS_READ_COIL ||   				//0x01
							g_funcIDRTU2 == MODBUS_READ_HOLDING_REGISTERS ||    //0x03
							g_funcIDRTU2 == MODBUS_WRITE_SINGLE_REGISTERS ||	//0x06
							g_funcIDRTU2 == MODBUS_READ_DISCRETE_INPUT ||		//0x02
							g_funcIDRTU2 == MODBUS_READ_INPUT_REGISTERS ||		//0x04
							g_funcIDRTU2 == MODBUS_WRITE_SINGLE_COIL 			//0x05
				//			g_funcIDRTU2 == MODBUS_WRITE_MULTIPLE_COILS ||		//0x07
				//			g_funcIDRTU2 == MODBUS_WRITE_MULTIPLE_REGISTERS 	//0x08
)

						{
							g_frameLengthRTU2 = 4;
							modbusRTUState3 = MODBUS_RTU_STATE_FRAME_START;
							WiFi_RTU_ModbusTimer3Arm();
						}
					else
						{
						 	 modbusRTUState3 = MODBUS_RTU_STATE_INIT;
//						 	 UARTprintf("Fw\n");
						}

					break;
		case MODBUS_RTU_STATE_FRAME_LENGTH_BEFORE:
					pui8TempRTU[i32IndexRTU] = ucChar;
					g_frameLengthRTU2 = pui8TempRTU[i32IndexRTU];
					i32IndexRTU++;
					cnt_data_for_get_len++;

					if((g_funcIDRTU2 == MODBUS_WRITE_MULTIPLE_COILS2)&&(cnt_data_for_get_len >= 4)) //0x0F
					{
						modbusRTUState3 = MODBUS_RTU_STATE_FRAME_LENGTH;
						WiFi_RTU_ModbusTimer3Arm();
					}
					if((g_funcIDRTU2 == MODBUS_WRITE_MULTIPLE_REGISTERS2)&&(cnt_data_for_get_len >= 4)) //0x10
					{
						modbusRTUState3 = MODBUS_RTU_STATE_FRAME_LENGTH;
						WiFi_RTU_ModbusTimer3Arm();
					}
					break;

		case MODBUS_RTU_STATE_FRAME_LENGTH:								//UARTprintf("MODBUS_RTU_STATE_FRAME_LENGTH\n");
			//		while(!SerialReceiveAvailable(pState->ui32SerialPort))	vTaskDelay( 10/portTICK_RATE_MS );
			//		pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);
					pui8TempRTU[i32IndexRTU] = ucChar;
					g_frameLengthRTU2 = pui8TempRTU[i32IndexRTU];
					i32IndexRTU++;


					if(	g_frameLengthRTU2 < (RTU_MAX_LENGTH - 4))//251 FB
					{


						//UARTprintf("len:%d\n",g_frameLengthRTU2);
						modbusRTUState3 = MODBUS_RTU_STATE_FRAME_START;
						WiFi_RTU_ModbusTimer3Arm();
//						UARTprintf("\nlen ok\n");
						//ModbusTimerArm();
					}
					else
					{
//						UARTprintf("too long\n");
						modbusRTUState3 = MODBUS_RTU_STATE_INIT;
					}
					break;

		case MODBUS_RTU_STATE_FRAME_START:								//UARTprintf("MODBUS_RTU_STATE_FRAME_START\n");
			//		while(!SerialReceiveAvailable(pState->ui32SerialPort))	vTaskDelay( 10/portTICK_RATE_MS );
			//		pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);		//UARTprintf("rev %02x\n",pui8Temp[i32Index]);
					pui8TempRTU[i32IndexRTU] = ucChar;
					i32IndexRTU++;

					//g_mbapRTU[g_serialRecvCount] = ucChar;
					//g_serialRecvCount++;
					g_frameDataCount2++;

//					UARTprintf("\ndata_s\n");
					if(	g_frameDataCount2 >= g_frameLengthRTU2)
					{
						modbusRTUState3 = MODBUS_RTU_STATE_FRAME_CRC1;
						WiFi_RTU_ModbusTimer3Arm();
//						UARTprintf("revd:%d\n",g_frameDataCount2);
//						UARTprintf("\ndata_e\n");
					}
					//ModbusTimerArm();
					 break;

		case  MODBUS_RTU_STATE_FRAME_CRC1:								//UARTprintf("MODBUS_RTU_STATE_FRAME_CRC1\n");
				//	while(!SerialReceiveAvailable(pState->ui32SerialPort))	vTaskDelay( 10/portTICK_RATE_MS );
				//	pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);
					pui8TempRTU[i32IndexRTU] = ucChar;
			        i32IndexRTU++;
					//g_crcRTU[0] = ucChar;
					//g_serialRecvCount++ ;
//			        UARTprintf("\ncrc1\n");
					modbusRTUState3 = MODBUS_RTU_STATE_FRAME_CRC2;
					WiFi_RTU_ModbusTimer3Arm();
					//ModbusTimerArm();
					break;

		case  MODBUS_RTU_STATE_FRAME_CRC2:								//UARTprintf("MODBUS_RTU_STATE_FRAME_CRC2\n");
			//		while(!SerialReceiveAvailable(pState->ui32SerialPort))	vTaskDelay( 10/portTICK_RATE_MS );
			//		pui8Temp[i32Index] = SerialReceive(pState->ui32SerialPort);
					pui8TempRTU[i32IndexRTU] = ucChar;
			        i32IndexRTU++;
					//g_crcRTU[1] = ucChar;
					//g_serialRecvCount++ ;
//			        UARTprintf("\ncrc2\n");
					modbusRTUState3 = MODBUS_RTU_STATE_FRAME_SENT;
					WiFi_RTU_ModbusTimer3DisArm();

		case  MODBUS_RTU_STATE_FRAME_SENT:								//UARTprintf("MODBUS_RTU_STATE_FRAME_SENT\n");

			//		tcp_write(pState->pConnectPCB, pui8Temp, i32IndexRTU, 1);//serial-->telnet
			Qty_send=i32IndexRTU;
				//	int i;	for(i=0; i < i32IndexRTU; i++)
			/*
			while((i32IndexRTU) != 0)
					{
						// received completed RTU frame, and write to LAN_to_COM Buffer in one time;
				//		RingBufWriteOne(&Data_From_COM_To_LAN, pui8TempRTU[Qty_send-i32IndexRTU]);
				//		RingBufWriteOne(&g_sTxBuf[COM_Port], pui8TempRTU[Qty_send-i32IndexRTU]);
						SerialSend(COM_Port, pui8TempRTU[Qty_send-i32IndexRTU]);
				//		UARTprintf("%02x ",pui8TempRTU[Qty_send-i32IndexRTU]);
						i32IndexRTU--;

					}
			*/

			int i;
			for(i=0; i < i32IndexRTU; i++)
			{
				SerialSend(COM_Port, pui8TempRTU[i]);
//				UARTprintf("%02x ",pui8TempRTU[i]);
			}
			WiFi_RTU_ModbusTimer3Arm();

					//	RingBufWriteOne(&Data_From_COM_To_LAN, pui8TempRTU[Qty_send-i32IndexRTU]);
						//	UARTprintf("\n");

				//	UARTprintf("telnet send  len:%d\n",i32Index);
					modbusRTUState2 = MODBUS_RTU_STATE_INIT;
					modbusRTUState3 = MODBUS_RTU_STATE_INIT;					//	UARTprintf("MODBUS_RTU_STATE_INIT\n\n");

			//		goto end;


		case  MODBUS_RTU_STATE_INVALID:									//UARTprintf("MODBUS_RTU_STATE_INVALID\n");
		default:														//UARTprintf("default\n");
				modbusRTUState3 = MODBUS_RTU_STATE_INIT;
				break;
	}
}

// new function; lan port RTU need to filter abnormal char, about 6 chars,
//it's difficult to make filter, so data from lan to com, send to COM directly, no RTU receiveing;


 #endif


