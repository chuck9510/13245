/*
 * COM_WIFI.c
 *
 *  Created on: 2015Äê8ÔÂ8ÈÕ
 *      Author: pony
 */

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
#include <user_define.h>

//extern void Decode_Command(struct udp_pcb *pcb, struct pbuf *p,struct ip_addr *addr,u16_t port,uint8_t *command,uint8_t *response_data,uint8_t *received_data_frame);
//extern _Bool WiFiCMD_Mode_FLag;  //1, wificmd mode; 0, normal mode;
//extern _Bool Switch_WiFi_to_COM; // 1,wifi to com; 0, Lan to com;
//extern uint8_t ResponseFromWiFi[256]; // save the response data from wifi module;
extern uint8_t SendCMDBufferToWiFi[20]; // save the command to be sent to wifi module;
const char PWR_OK_String[12] = {'p','o','w','e','r',' ','o','n',' ','o','k','\0'};
//extern tWiFi_CommandSet sWiFi_CommandSet;
//*****************************************************************************
//
// The stack size for the Serial task.
//
//*****************************************************************************
#define STACKSIZE_SerialTASK    512
#define STACKSIZE_COM_WiFiTASK    512

xQueueHandle g_Que_COM_LAN_WiFi;
xQueueHandle g_Que_TI_Master_to_Slave;
xQueueHandle g_Que_TI_Master_to_Slave1;
xQueueHandle g_Que_TI_Master_to_Slave2;
//*****************************************************************************
//
// Hardware resote pin processing; Use USB D+(PL6) as an input to indicate user restore requestion;
//
//*****************************************************************************
bool wifi_app_CMD_processing;
// ensure can send 40 char continously;

void Make_Delay(uint8_t port)
{
	uint32_t baudrate,frame_len_continous_,pack_len_time_mS,pack_time_para_mS;
	uint8_t * packlen;
	baudrate = g_sParameters.sPort[port].ui32BaudRate;
	//	UARTprintf("BR9600 dy:%d\n",Delay_mS_for_9600/portTICK_RATE_MS);
	//	g_psParas->sPort[portNo].uiPackLen, detail definition is needed;
	//	g_psParas->sPort[portNo].uiPackTime, // one byte, 256 is the max value; so its unit should be seconds;
	// convert length format;

	pack_time_para_mS = g_sParameters.sPort[port].uiPackTime;
	//	UARTprintf("origin0-3:%d,%d,%d,%d\n",packlen[0]<<24,packlen[1]<<16,packlen[2]<<8,packlen[3]);
	//	UARTprintf("origin0-3:%d,%d,%d,%d\n",packlen[0],packlen[1],packlen[2],packlen[3]);

	//	frame_len_continous_ = g_sParameters.sPort[port].uiPackLen;
	//	pack_len_time_mS = 1000*frame_len_continous_ * 12 *6.1/baudrate;// scale is 6;

	packlen = g_sParameters.sPort[port].uiPackLen;
	frame_len_continous_ = (uint32_t)(packlen[0]<<24) + (uint32_t)(packlen[1]<<16) + (uint32_t)(packlen[2]<<8) + (uint32_t)packlen[3]; // 40 is 0x28;
	if(baudrate >=14400)
	{
		if(frame_len_continous_ == 0)
			pack_len_time_mS = 0;
		else if (frame_len_continous_ > 0)
			pack_len_time_mS = 1000*frame_len_continous_ * 12/baudrate + 80;
	}

	else
	{

		pack_len_time_mS = 1000*frame_len_continous_ * 12/baudrate;
	}

	//	UARTprintf("plen_mS:%d,len:%d\n",pack_len_time_mS,frame_len_continous_);
	//	UARTprintf("pt_mS:%d\n",pack_time_para_mS);
	if(g_sParameters.sPort[port].uiPackTime==0)
	{
		vTaskDelay( pack_len_time_mS/portTICK_RATE_MS );
	}
	else if((g_sParameters.sPort[port].uiPackTime>0))
	{
		vTaskDelay( pack_time_para_mS/portTICK_RATE_MS );
	}



	/*
	switch(baudrate)
	{
	case 230400:   vTaskDelay( Delay_mS_for_230400/portTICK_RATE_MS );  break; //frame_len_continous * 12 /230400 = 0.002;
	case 115200:   vTaskDelay( Delay_mS_for_230400*2/portTICK_RATE_MS );break;
	case 57600:	vTaskDelay( Delay_mS_for_230400*4/portTICK_RATE_MS );break;
	case 38400: vTaskDelay( Delay_mS_for_230400*8/portTICK_RATE_MS );break;
	case 19200: vTaskDelay( Delay_mS_for_230400*16/portTICK_RATE_MS );break;
	case 14400: vTaskDelay( 33/portTICK_RATE_MS );break; //frame_len_continous * 12 /14400 = 0.0333;
	case 9600:  vTaskDelay( Delay_mS_for_9600/portTICK_RATE_MS );  break; //frame_len_continous * 12 /9600 = 0.05;
	case 4800:  vTaskDelay( Delay_mS_for_9600*2/portTICK_RATE_MS );  break;
	case 2400:  vTaskDelay( Delay_mS_for_9600*4/portTICK_RATE_MS );  break;
	case 1200:  vTaskDelay( Delay_mS_for_9600*8/portTICK_RATE_MS );  break;
	case 600:   vTaskDelay( Delay_mS_for_9600*16/portTICK_RATE_MS );  break;
	case 300:   vTaskDelay( Delay_mS_for_9600*32/portTICK_RATE_MS );  break;
	default: break;

	}*/

}

void restore_call(void)
{

	ConfigLoadFactory();
	ConfigSave();
	UARTprintf("System restart now...\n");
	vTaskDelay( 200/portTICK_RATE_MS );
	SysCtlReset();

}

void User_HWrestore(void)
{
	//	uint32_t i32Val;
	uint8_t GPIO_ReadBack;
	GPIO_ReadBack = GPIOPinRead(HW_Restore_Port,HW_Restore_Pin);
#if Rev_A_AD7491_Board
	//	UARTprintf("i32val:%d, GPIO_ReadBack:%d\n",i32Val,GPIO_ReadBack);
	if((GPIO_ReadBack & HW_Restore_Pin) == 0) // press down restore pin;
	{
		vTaskDelay( 100/portTICK_RATE_MS );
		GPIO_ReadBack = GPIOPinRead(HW_Restore_Port,HW_Restore_Pin);
		if((GPIO_ReadBack & HW_Restore_Pin) == 0) // still zero;
			//restore setting and restart;
		{
			restore_call();
		}
	}
#else if (Rev_B_AD7491_Board||TI_Master_Board)
	//	UARTprintf("i32val:%d, GPIO_ReadBack:%d\n",i32Val,GPIO_ReadBack);
	if((GPIO_ReadBack & HW_Restore_Pin) == HW_Restore_Pin) // press down restore pin;
	{
		vTaskDelay( 100/portTICK_RATE_MS );
		GPIO_ReadBack = GPIOPinRead(HW_Restore_Port,HW_Restore_Pin);
		if((GPIO_ReadBack & HW_Restore_Pin) == HW_Restore_Pin) // still zero;
			//restore setting and restart;
		{
			restore_call();
		}
	}
#endif

}

void WiFiINI(void)
{
	int retval;
	//	if((g_sParameters.ui32WiFiModuleBaudRate==9600)&& WiFiINI_OK)
	if(g_sParameters.ui32WiFiModuleBaudRate==9600)
	{
		//		if(g_sParameters.datachansel==2)//1, lan-com;2; wilfi-com;
		//		Switch_WiFi_to_COM = 0;
		//		vTaskDelay( 60/portTICK_RATE_MS );
		wifimoduledefault();
		//    	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_PIN_7);// set wifi gpio9 high;

		_UR4_SerialSetConfig(9600);
		vTaskDelay( 60/portTICK_RATE_MS );
		//	wifiini_flag = 1;
		if(SetWiFi_UR_Baudrate(WiFi_UR_to_MCU_UR_Baudrate) == 0) //set wifi module baudrate to 115200;
		{
			UARTprintf("Succed to set WiFi Baudrate to %d! \n",WiFi_UR_to_MCU_UR_Baudrate);
			_UR4_SerialSetConfig(WiFi_UR_to_MCU_UR_Baudrate);// set mcu wifi ur port to 115200 also;
			g_sParameters.ui32WiFiModuleBaudRate = WiFi_UR_to_MCU_UR_Baudrate;
			g_sWorkingDefaultParameters= g_sParameters;
			ConfigSave();
			//			vTaskDelay( 6000/portTICK_RATE_MS );
			/*
			if (RN_NOT_ERR_OK(retval, module_send_cmd("CMD", "$$$")))
			{
				UARTprintf("Fail to cmd mode! \n");
			}
			else if(RN_NOT_ERR_OK(retval, module_send_cmd("DHCP", "apmode ponywang_test 1")))
			{
	                 //   RN_RED_LED = 1;
				UARTprintf("Fail to ap mode! \n");
			}
			 */
		}
		else
		{
			UARTprintf("wifi baudrate raise fail!\ncurrent wifi baudrate is 9600.\n");

		}
		//		 if(g_sParameters.datachansel==2)//1, lan-com;2; wilfi-com;
		//		 Switch_WiFi_to_COM = 1;
		//	    wifiini_flag = 0;
	}
	//	else {WiFi_Status_Retval = WiFi_EnterCMD_Error;}

}
void EnterCMD(void)
{
	uint8_t index,u8ichar;
	index =0;
	vTaskDelay( 250/portTICK_RATE_MS );

	while((RingBufUsed(&g_sRxBuf[WiFi_Port]) != 0) || (RingBufUsed(&g_sTxBuf[WiFi_Port])  != 0) )
	{
		u8ichar = SerialReceive(WiFi_Port);
		// 	 UARTprintf("%c",u8ichar);
	}; //wait until send condition ok;

	UARTprintf("send condition ok!\n");
	UARTprintf("prepare to enter CMD:\n");
	SerialSend(WiFi_Port, '$');
	SerialSend(WiFi_Port, '$');
	SerialSend(WiFi_Port, '$');

	vTaskDelay( 50/portTICK_RATE_MS );
	UARTprintf("\n response from wifi :" );
	while(SerialReceiveAvailable(WiFi_Port)) //when wifi rx ringbuffer has data, receive it;
	{
		ResponseFromWiFi[index] = (uint8_t)SerialReceive(WiFi_Port);
		//       UARTprintf("%c",ResponseFromWiFi[index] );
		index++;
	}
	SerialSend(WiFi_Port, '\r');
	SerialSend(WiFi_Port, '\n');
	UARTprintf("\n" );
	index =0;
	vTaskDelay( 50/portTICK_RATE_MS );
	// wait for <4.1.4> string;
	while(SerialReceiveAvailable(WiFi_Port)) //when wifi rx ringbuffer has data, receive it;
	{
		ResponseFromWiFi[index] = (uint8_t)SerialReceive(WiFi_Port);
		// 	 UARTprintf("%c",ResponseFromWiFi[index] );
		index++;
	}
	vTaskDelay( 250/portTICK_RATE_MS );
	// finish the receiving; judge the response data;
}

void ExitCMD(void)
{
	uint8_t index,u8ichar;
	index =0;
	//	vTaskDelay( 250/portTICK_RATE_MS );

	//	wait until send condition ok;
	while((RingBufUsed(&g_sRxBuf[WiFi_Port]) != 0) || (RingBufUsed(&g_sTxBuf[WiFi_Port])  != 0) )
	{
		u8ichar = SerialReceive(WiFi_Port);
		// 	 UARTprintf("%c",u8ichar);
	};

	//	UARTprintf("send condition ok!\n");
	//	UARTprintf("\nPrepare to exit CMD!\n");
	SerialSend(WiFi_Port, 'e');
	SerialSend(WiFi_Port, 'x');
	SerialSend(WiFi_Port, 'i');
	SerialSend(WiFi_Port, 't');
	SerialSend(WiFi_Port, '\r');
	SerialSend(WiFi_Port, '\n');

	vTaskDelay( 200/portTICK_RATE_MS );      //adjust delay time to receive response data;
	//	UARTprintf("\nResponse from WiFi:\n" );
	while(SerialReceiveAvailable(WiFi_Port)) //when wifi rx ringbuffer has data, receive it;
	{
		ResponseFromWiFi[index] = (uint8_t)SerialReceive(WiFi_Port);
		// 	 UARTprintf("%c",ResponseFromWiFi[index] );
		index++;
	}
	//	SerialSend(WiFi_Port, '/r');	 // send enter plus return '/n';
	//	UARTprintf("\n" );

	vTaskDelay( 250/portTICK_RATE_MS );
	// finish the receiving; judge the response data;
}

void WiFiCMDBufferINI(void)
{
	uint32_t index;
	for(index=0;index<256;index++)
		ResponseFromWiFi[index] = 0x00;
	for(index=0;index<20;index++)
		SendCMDBufferToWiFi[index] = 0x00;
}

//*****************************************************************************
//
// swap data between com and wifi uart;
//!
//
//*****************************************************************************
static void
COM_WiFiTask(void *pvParameters)
{
	uint8_t u8ichar,u8ichar_WiFi_In,MAC_Addr_Temp[6],*pui8Data;//UARTRxBuf[20];
	//	uint8_t data_rev_qty;
	uint8_t rec_char_status,len_counter,len_MAC_counter,char_temp;
	uint8_t UserAccount_Temp[12],len_useraccout_counter,Sub_DATA_Temp[20],sub_data_counter,DATA_len;
	uint8_t *command,len_frame,received_frame[512],*response_data;
	portTickType xLastWakeTime;
	uint8_t count_Tag;

	//	Switch_WiFi_to_COM = 0;
	//	_Bool CMDModeOK_Flag = 0;
	_Bool COMtoWiFiTxRdy =0;
	_Bool COM_to_LAN_ringbuffer_TxRdy =0;
	//	_Bool COM_to_LAN_ringbuffer_full =0;
	_Bool WiFitoCOMRxRdy =0;
	slave_data_rdy_syn = 0;
	int retval;
	//	uint32_t com_wifi_stack_remaining;
	//	_Bool WiFiModuleOK =0;
	//	tWiFiCMD sWiFiCMD;
	//	int retval;
	/*
	if(g_sParameters.ui32WiFiModuleBaudRate==9600)
	{
		_UR4_SerialSetConfig(9600);
	    if(SetWiFi_UR_Bardrate(115200) == 0) //set wifi module baudrate to 115200;
	    {
	    	UARTprintf("Succed to set WiFi Baudrate to 115200! \n");
	        _UR4_SerialSetConfig(115200);// set mcu wifi ur port to 115200 also;
	    }
	}

	 */
	xLastWakeTime = xTaskGetTickCount();
	while(1)
	{

		//add sending poweronok string to COM port each startup;

		/*
	   if((PWR_OK_Flag)&&(wifi_restoring == false))
	   {
		   ConsolePutString_to_COM((char *)PWR_OK_String);

		   vTaskDelay( 4000/portTICK_RATE_MS );
		   PWR_OK_Flag = false;
		//   g_sParameters.ui8Reserved1[0]++;
		//   UARTprintf("\n\npower on times: %d \n", g_sParameters.ui8Reserved1[0]);
		//	g_sWorkingDefaultParameters= g_sParameters;
		//	ConfigSave();
		//	vTaskDelay( 2000/portTICK_RATE_MS );
	//	   SysCtlReset();

	   }
		 */

#if watchdog_EN
		ROM_WatchdogReloadSet(WATCHDOG0_BASE, g_ui32SysClock*300);
#endif

		if(g_bFirmwareUpdate)
		{
			//
			// Transfer control to the bootloader.
			//
			SoftwareUpdateBegin(g_ui32SysClock);
		}
		// User_HWrestore();
		/*
	   wifi_app_CMD_processing = false;
		  Switch_LAN_to_COM = 1;
		  Switch_WiFi_to_COM = 1;
		 */


		// data transmit only;

		//resume lan-com setting;
		//data transfer mode;
		//		  if(g_sParameters.datachansel ==1 || g_sParameters.datachansel==3) // lan to com, or both;
		//		  Switch_LAN_to_COM = 1;
		if(!(WiFi_Setting))
			resume_channel_connection();







		//	     UARTprintf("Switch_LAN_to_COM =%d, Switch_WiFi_to_COM = %d",Switch_LAN_to_COM,Switch_WiFi_to_COM);
		if((Switch_WiFi_to_COM ==1)&&(Switch_LAN_to_COM == 0) ) // enable wifi to com channel;
		{
			//		  UARTprintf("\n Switch_WiFi_to_COM = %d\n",Switch_WiFi_to_COM);
			//		 UARTprintf("\n current data channel is WiFi to COM\n");
			// check com1 rx ringbuffer data available, and wifi tx ringbuffer have space.
			// if ok, com1 rx ringbuffer data  --->  wifi tx ringbuffer
			COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port);
			WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port));

			// make some delays according to different COM baudrate;
			if(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
				Make_Delay(COM_Port);

			while(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
			{
				if(COMtoWiFiTxRdy)
				{
					u8ichar = (uint8_t)SerialReceive(COM_Port);
					SerialSend(WiFi_Port, u8ichar);
					//        WiFi_Status_Retval = WiFi_Datain;
					//        WiFiLEDflashtime = 1;
				}
				if(WiFitoCOMRxRdy)
				{
					u8ichar_WiFi_In = (uint8_t)SerialReceive(WiFi_Port);
					SerialSend(COM_Port,u8ichar_WiFi_In );
					// wifi status led falsh 4 times;
					//        WiFi_Status_Retval = WiFi_Datain;
					//       WiFiLEDflashtime = 1;
					//		            WiFiModuleOK = true;
				}

				COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port);
				WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port));

			}

		}
		else  if((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 1) )
		{

			// check com1 rx ringbuffer data available, and com to lan ringbuffer, not full, have space.

			// Data_From_COM_To_LAN ringbuffer size is 1024 bytes;
			COM_to_LAN_ringbuffer_TxRdy = (!RingBufFull(&Data_From_COM_To_LAN)) && SerialReceiveAvailable(COM_Port);

			//	  UARTprintf("CL_Qty2:%d\n",RingBufUsed(&Data_From_COM_To_LAN));
			//	  UARTprintf("CR_Qty2:%d\n",SerialReceiveAvailable(COM_Port));

			// make some delays according to different COM baudrate;
			if(COM_to_LAN_ringbuffer_TxRdy)
				Make_Delay(COM_Port);

			while(COM_to_LAN_ringbuffer_TxRdy)
			{
				u8ichar = (uint8_t)SerialReceive(COM_Port);
				RingBufWriteOne(&Data_From_COM_To_LAN, u8ichar);


				COM_to_LAN_ringbuffer_TxRdy = (!RingBufFull(&Data_From_COM_To_LAN)) && SerialReceiveAvailable(COM_Port);

			}

		}
		else  if((Switch_WiFi_to_COM ==1)&&(Switch_LAN_to_COM == 1) )
		{

			// check com1 rx ringbuffer data available, and wifi tx ringbuffer have space.
			// if ok, com1 rx ringbuffer data  --->  wifi tx ringbuffer
			COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port)&&(!RingBufFull(&Data_From_COM_To_LAN));
			WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port))&&LAN_to_COM_TX_Finish;
			//	  UARTprintf("CL_Qty2:%d\n",RingBufUsed(&Data_From_COM_To_LAN));
			//	  UARTprintf("CR_Qty2:%d\n",SerialReceiveAvailable(COM_Port));

			// make some delays according to different COM baudrate;
			if(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
				Make_Delay(COM_Port);

			while(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
			{
				if(COMtoWiFiTxRdy)
				{
					u8ichar = (uint8_t)SerialReceive(COM_Port);
					RingBufWriteOne(&Data_From_COM_To_LAN, u8ichar);
					SerialSend(WiFi_Port, u8ichar);
				}
				if(WiFitoCOMRxRdy)
				{
					// lan to com TX finish, and wifi port has data coming, and com port has space;
					WiFi_to_COM_TX_Finish = false; // set flag for wifi tx data to com; avoid the disturb of LAN;
					u8ichar_WiFi_In = (uint8_t)SerialReceive(WiFi_Port);
					SerialSend(COM_Port,u8ichar_WiFi_In );
				}
				COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port)&&(!RingBufFull(&Data_From_COM_To_LAN));
				WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port))&&LAN_to_COM_TX_Finish;

			}
			//  UARTprintf("CL_Qty2:%d\n",RingBufUsed(&Data_From_COM_To_LAN));
			//  UARTprintf("CR_Qty2:%d\n",SerialReceiveAvailable(COM_Port));
			if(SerialReceiveAvailable(WiFi_Port)==0)
				WiFi_to_COM_TX_Finish = true; // set flag to indicate to LAN, you can go;

		}

		//	  else  if((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 0)&&(g_sParameters.ui32WiFiModuleBaudRate!=9600)&&(wifiCMD_Executing==false) )
		//	  UARTprintf("wifi task loop end \n" );
		//	  com_wifi_stack_remaining = uxTaskGetStackHighWaterMark(NULL);
		//	  UARTprintf("\ncom_wifi_stack_remaining : %d \n",com_wifi_stack_remaining );
		vTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_RATE_MS ) );
	} //for while(1);

}
//*****************************************************************************
//
// swap data between com1 and wifi uart;
//!
//
//*****************************************************************************
static void
COM1_WiFiTask(void *pvParameters)
{

	uint8_t u8ichar,u8ichar_WiFi_In,MAC_Addr_Temp[6],*pui8Data;//UARTRxBuf[20];
	//	uint8_t data_rev_qty;
	uint8_t rec_char_status,len_counter,len_MAC_counter,char_temp;
	uint8_t UserAccount_Temp[12],len_useraccout_counter,Sub_DATA_Temp[20],sub_data_counter,DATA_len;
	uint8_t *command,len_frame,received_frame[512],*response_data;
	portTickType xLastWakeTime;
	uint8_t count_Tag;

	//	Switch_WiFi_to_COM = 0;
	//	_Bool CMDModeOK_Flag = 0;
	_Bool COMtoWiFiTxRdy =0;
	_Bool COM_to_LAN_ringbuffer_TxRdy =0;
	//	_Bool COM_to_LAN_ringbuffer_full =0;
	_Bool WiFitoCOMRxRdy =0;
	slave_data_rdy_syn = 0;
	int retval;
	//	uint32_t com_wifi_stack_remaining;
	//	_Bool WiFiModuleOK =0;
	//	tWiFiCMD sWiFiCMD;
	//	int retval;
	/*
	if(g_sParameters.ui32WiFiModuleBaudRate==9600)
	{
		_UR4_SerialSetConfig(9600);
	    if(SetWiFi_UR_Bardrate(115200) == 0) //set wifi module baudrate to 115200;
	    {
	    	UARTprintf("Succed to set WiFi Baudrate to 115200! \n");
	        _UR4_SerialSetConfig(115200);// set mcu wifi ur port to 115200 also;
	    }
	}

	 */
	xLastWakeTime = xTaskGetTickCount();
	while(1)
	{

		//add sending poweronok string to COM port each startup;

		/*
	   if((PWR_OK_Flag)&&(wifi_restoring == false))
	   {
		   ConsolePutString_to_COM((char *)PWR_OK_String);

		   vTaskDelay( 4000/portTICK_RATE_MS );
		   PWR_OK_Flag = false;
		//   g_sParameters.ui8Reserved1[0]++;
		//   UARTprintf("\n\npower on times: %d \n", g_sParameters.ui8Reserved1[0]);
		//	g_sWorkingDefaultParameters= g_sParameters;
		//	ConfigSave();
		//	vTaskDelay( 2000/portTICK_RATE_MS );
	//	   SysCtlReset();

	   }
		 */

#if watchdog_EN
		ROM_WatchdogReloadSet(WATCHDOG0_BASE, g_ui32SysClock*300);
#endif

		if(g_bFirmwareUpdate)
		{
			//
			// Transfer control to the bootloader.
			//
			SoftwareUpdateBegin(g_ui32SysClock);
		}
		// User_HWrestore();
		/*
	   wifi_app_CMD_processing = false;
		  Switch_LAN_to_COM = 1;
		  Switch_WiFi_to_COM = 1;
		 */


		// data transmit only;

		//resume lan-com setting;
		//data transfer mode;
		//		  if(g_sParameters.datachansel ==1 || g_sParameters.datachansel==3) // lan to com, or both;
		//		  Switch_LAN_to_COM = 1;
		if(!(WiFi_Setting))
			resume_channel_connection();







		//	     UARTprintf("Switch_LAN_to_COM =%d, Switch_WiFi_to_COM = %d",Switch_LAN_to_COM,Switch_WiFi_to_COM);
		if((Switch_WiFi_to_COM ==1)&&(Switch_LAN_to_COM == 0) ) // enable wifi to com channel;
		{
			//		  UARTprintf("\n Switch_WiFi_to_COM = %d\n",Switch_WiFi_to_COM);
			//		 UARTprintf("\n current data channel is WiFi to COM\n");
			// check com1 rx ringbuffer data available, and wifi tx ringbuffer have space.
			// if ok, com1 rx ringbuffer data  --->  wifi tx ringbuffer
			COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port);
			WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port));

			// make some delays according to different COM baudrate;
			if(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
				Make_Delay(COM_Port);

			while(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
			{
				if(COMtoWiFiTxRdy)
				{
					u8ichar = (uint8_t)SerialReceive(COM_Port);
					SerialSend(WiFi_Port, u8ichar);
					//        WiFi_Status_Retval = WiFi_Datain;
					//        WiFiLEDflashtime = 1;
				}
				if(WiFitoCOMRxRdy)
				{
					u8ichar_WiFi_In = (uint8_t)SerialReceive(WiFi_Port);
					SerialSend(COM_Port,u8ichar_WiFi_In );
					// wifi status led falsh 4 times;
					//        WiFi_Status_Retval = WiFi_Datain;
					//       WiFiLEDflashtime = 1;
					//		            WiFiModuleOK = true;
				}

				COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port);
				WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port));

			}

		}
		else  if((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 1) )
		{

			// check com1 rx ringbuffer data available, and com to lan ringbuffer, not full, have space.

			// Data_From_COM_To_LAN ringbuffer size is 1024 bytes;
			COM_to_LAN_ringbuffer_TxRdy = (!RingBufFull(&Data_From_COM1_To_LAN)) && SerialReceiveAvailable(COM1_Port);

			//	  UARTprintf("CL_Qty2:%d\n",RingBufUsed(&Data_From_COM_To_LAN));
			//	  UARTprintf("CR_Qty2:%d\n",SerialReceiveAvailable(COM_Port));

			// make some delays according to different COM baudrate;
			if(COM_to_LAN_ringbuffer_TxRdy)
				Make_Delay(COM1_Port);

			while(COM_to_LAN_ringbuffer_TxRdy)
			{
				u8ichar = (uint8_t)SerialReceive(COM1_Port);
				RingBufWriteOne(&Data_From_COM1_To_LAN, u8ichar);


				COM_to_LAN_ringbuffer_TxRdy = (!RingBufFull(&Data_From_COM1_To_LAN)) && SerialReceiveAvailable(COM1_Port);

			}

		}
		else  if((Switch_WiFi_to_COM ==1)&&(Switch_LAN_to_COM == 1) )
		{

			// check com1 rx ringbuffer data available, and wifi tx ringbuffer have space.
			// if ok, com1 rx ringbuffer data  --->  wifi tx ringbuffer
			COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port)&&(!RingBufFull(&Data_From_COM_To_LAN));
			WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port))&&LAN_to_COM_TX_Finish;
			//	  UARTprintf("CL_Qty2:%d\n",RingBufUsed(&Data_From_COM_To_LAN));
			//	  UARTprintf("CR_Qty2:%d\n",SerialReceiveAvailable(COM_Port));

			// make some delays according to different COM baudrate;
			if(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
				Make_Delay(COM_Port);

			while(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
			{
				if(COMtoWiFiTxRdy)
				{
					u8ichar = (uint8_t)SerialReceive(COM_Port);
					RingBufWriteOne(&Data_From_COM_To_LAN, u8ichar);
					SerialSend(WiFi_Port, u8ichar);
				}
				if(WiFitoCOMRxRdy)
				{
					// lan to com TX finish, and wifi port has data coming, and com port has space;
					WiFi_to_COM_TX_Finish = false; // set flag for wifi tx data to com; avoid the disturb of LAN;
					u8ichar_WiFi_In = (uint8_t)SerialReceive(WiFi_Port);
					SerialSend(COM_Port,u8ichar_WiFi_In );
				}
				COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port)&&(!RingBufFull(&Data_From_COM_To_LAN));
				WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port))&&LAN_to_COM_TX_Finish;

			}
			//  UARTprintf("CL_Qty2:%d\n",RingBufUsed(&Data_From_COM_To_LAN));
			//  UARTprintf("CR_Qty2:%d\n",SerialReceiveAvailable(COM_Port));
			if(SerialReceiveAvailable(WiFi_Port)==0)
				WiFi_to_COM_TX_Finish = true; // set flag to indicate to LAN, you can go;

		}

		//	  else  if((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 0)&&(g_sParameters.ui32WiFiModuleBaudRate!=9600)&&(wifiCMD_Executing==false) )
		//	  UARTprintf("wifi task loop end \n" );
		//	  com_wifi_stack_remaining = uxTaskGetStackHighWaterMark(NULL);
		//	  UARTprintf("\ncom_wifi_stack_remaining : %d \n",com_wifi_stack_remaining );
		vTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_RATE_MS ) );
	} //for while(1);

}

//*****************************************************************************
//
// swap data between com2 and wifi uart;
//!
//
//*****************************************************************************
static void
COM2_WiFiTask(void *pvParameters)
{

	uint8_t u8ichar,u8ichar_WiFi_In,MAC_Addr_Temp[6],*pui8Data;//UARTRxBuf[20];
	//	uint8_t data_rev_qty;
	uint8_t rec_char_status,len_counter,len_MAC_counter,char_temp;
	uint8_t UserAccount_Temp[12],len_useraccout_counter,Sub_DATA_Temp[20],sub_data_counter,DATA_len;
	uint8_t *command,len_frame,received_frame[512],*response_data;
	portTickType xLastWakeTime;
	uint8_t count_Tag;

	//	Switch_WiFi_to_COM = 0;
	//	_Bool CMDModeOK_Flag = 0;
	_Bool COMtoWiFiTxRdy =0;
	_Bool COM_to_LAN_ringbuffer_TxRdy =0;
	//	_Bool COM_to_LAN_ringbuffer_full =0;
	_Bool WiFitoCOMRxRdy =0;
	slave_data_rdy_syn = 0;
	int retval;
	//	uint32_t com_wifi_stack_remaining;
	//	_Bool WiFiModuleOK =0;
	//	tWiFiCMD sWiFiCMD;
	//	int retval;
	/*
	if(g_sParameters.ui32WiFiModuleBaudRate==9600)
	{
		_UR4_SerialSetConfig(9600);
	    if(SetWiFi_UR_Bardrate(115200) == 0) //set wifi module baudrate to 115200;
	    {
	    	UARTprintf("Succed to set WiFi Baudrate to 115200! \n");
	        _UR4_SerialSetConfig(115200);// set mcu wifi ur port to 115200 also;
	    }
	}

	 */
	xLastWakeTime = xTaskGetTickCount();
	while(1)
	{

		//add sending poweronok string to COM port each startup;

		/*
	   if((PWR_OK_Flag)&&(wifi_restoring == false))
	   {
		   ConsolePutString_to_COM((char *)PWR_OK_String);

		   vTaskDelay( 4000/portTICK_RATE_MS );
		   PWR_OK_Flag = false;
		//   g_sParameters.ui8Reserved1[0]++;
		//   UARTprintf("\n\npower on times: %d \n", g_sParameters.ui8Reserved1[0]);
		//	g_sWorkingDefaultParameters= g_sParameters;
		//	ConfigSave();
		//	vTaskDelay( 2000/portTICK_RATE_MS );
	//	   SysCtlReset();

	   }
		 */

#if watchdog_EN
		ROM_WatchdogReloadSet(WATCHDOG0_BASE, g_ui32SysClock*300);
#endif

		if(g_bFirmwareUpdate)
		{
			//
			// Transfer control to the bootloader.
			//
			SoftwareUpdateBegin(g_ui32SysClock);
		}
		// User_HWrestore();
		/*
	   wifi_app_CMD_processing = false;
		  Switch_LAN_to_COM = 1;
		  Switch_WiFi_to_COM = 1;
		 */


		// data transmit only;

		//resume lan-com setting;
		//data transfer mode;
		//		  if(g_sParameters.datachansel ==1 || g_sParameters.datachansel==3) // lan to com, or both;
		//		  Switch_LAN_to_COM = 1;
		if(!(WiFi_Setting))
			resume_channel_connection();







		//	     UARTprintf("Switch_LAN_to_COM =%d, Switch_WiFi_to_COM = %d",Switch_LAN_to_COM,Switch_WiFi_to_COM);
		if((Switch_WiFi_to_COM ==1)&&(Switch_LAN_to_COM == 0) ) // enable wifi to com channel;
		{
			//		  UARTprintf("\n Switch_WiFi_to_COM = %d\n",Switch_WiFi_to_COM);
			//		 UARTprintf("\n current data channel is WiFi to COM\n");
			// check com1 rx ringbuffer data available, and wifi tx ringbuffer have space.
			// if ok, com1 rx ringbuffer data  --->  wifi tx ringbuffer
			COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port);
			WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port));

			// make some delays according to different COM baudrate;
			if(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
				Make_Delay(COM_Port);

			while(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
			{
				if(COMtoWiFiTxRdy)
				{
					u8ichar = (uint8_t)SerialReceive(COM_Port);
					SerialSend(WiFi_Port, u8ichar);
					//        WiFi_Status_Retval = WiFi_Datain;
					//        WiFiLEDflashtime = 1;
				}
				if(WiFitoCOMRxRdy)
				{
					u8ichar_WiFi_In = (uint8_t)SerialReceive(WiFi_Port);
					SerialSend(COM_Port,u8ichar_WiFi_In );
					// wifi status led falsh 4 times;
					//        WiFi_Status_Retval = WiFi_Datain;
					//       WiFiLEDflashtime = 1;
					//		            WiFiModuleOK = true;
				}

				COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port);
				WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port));

			}

		}
		else  if((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 1) )
		{

			// check com1 rx ringbuffer data available, and com to lan ringbuffer, not full, have space.

			// Data_From_COM_To_LAN ringbuffer size is 1024 bytes;
			COM_to_LAN_ringbuffer_TxRdy = (!RingBufFull(&Data_From_COM2_To_LAN)) && SerialReceiveAvailable(COM2_Port);

			//	  UARTprintf("CL_Qty2:%d\n",RingBufUsed(&Data_From_COM_To_LAN));
			//	  UARTprintf("CR_Qty2:%d\n",SerialReceiveAvailable(COM_Port));

			// make some delays according to different COM baudrate;
			if(COM_to_LAN_ringbuffer_TxRdy)
				Make_Delay(COM2_Port);

			while(COM_to_LAN_ringbuffer_TxRdy)
			{
				u8ichar = (uint8_t)SerialReceive(COM2_Port);
				RingBufWriteOne(&Data_From_COM2_To_LAN, u8ichar);

				COM_to_LAN_ringbuffer_TxRdy = (!RingBufFull(&Data_From_COM2_To_LAN)) && SerialReceiveAvailable(COM2_Port);

			}

		}
		else  if((Switch_WiFi_to_COM ==1)&&(Switch_LAN_to_COM == 1) )
		{

			// check com1 rx ringbuffer data available, and wifi tx ringbuffer have space.
			// if ok, com1 rx ringbuffer data  --->  wifi tx ringbuffer
			COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port)&&(!RingBufFull(&Data_From_COM_To_LAN));
			WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port))&&LAN_to_COM_TX_Finish;
			//	  UARTprintf("CL_Qty2:%d\n",RingBufUsed(&Data_From_COM_To_LAN));
			//	  UARTprintf("CR_Qty2:%d\n",SerialReceiveAvailable(COM_Port));

			// make some delays according to different COM baudrate;
			if(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
				Make_Delay(COM_Port);

			while(COMtoWiFiTxRdy||WiFitoCOMRxRdy)
			{
				if(COMtoWiFiTxRdy)
				{
					u8ichar = (uint8_t)SerialReceive(COM_Port);
					RingBufWriteOne(&Data_From_COM_To_LAN, u8ichar);
					SerialSend(WiFi_Port, u8ichar);
				}
				if(WiFitoCOMRxRdy)
				{
					// lan to com TX finish, and wifi port has data coming, and com port has space;
					WiFi_to_COM_TX_Finish = false; // set flag for wifi tx data to com; avoid the disturb of LAN;
					u8ichar_WiFi_In = (uint8_t)SerialReceive(WiFi_Port);
					SerialSend(COM_Port,u8ichar_WiFi_In );
				}
				COMtoWiFiTxRdy = (!SerialSendFull(WiFi_Port)) && SerialReceiveAvailable(COM_Port)&&(!RingBufFull(&Data_From_COM_To_LAN));
				WiFitoCOMRxRdy = SerialReceiveAvailable(WiFi_Port) && (!SerialSendFull(COM_Port))&&LAN_to_COM_TX_Finish;

			}
			//  UARTprintf("CL_Qty2:%d\n",RingBufUsed(&Data_From_COM_To_LAN));
			//  UARTprintf("CR_Qty2:%d\n",SerialReceiveAvailable(COM_Port));
			if(SerialReceiveAvailable(WiFi_Port)==0)
				WiFi_to_COM_TX_Finish = true; // set flag to indicate to LAN, you can go;

		}

		//	  else  if((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 0)&&(g_sParameters.ui32WiFiModuleBaudRate!=9600)&&(wifiCMD_Executing==false) )
		//	  UARTprintf("wifi task loop end \n" );
		//	  com_wifi_stack_remaining = uxTaskGetStackHighWaterMark(NULL);
		//	  UARTprintf("\ncom_wifi_stack_remaining : %d \n",com_wifi_stack_remaining );
		vTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_RATE_MS ) );
	} //for while(1);

}

//*****************************************************************************
//
// Initialize the serial peripherals and the serial task.
//
//*****************************************************************************
uint32_t
COM_WiFiTaskInit(void)
{/*
	g_Que_COM_LAN_WiFi = xQueueCreate(128, sizeof(uint8_t)); // change from 64 to 128


    if(g_Que_COM_LAN_WiFi == 0)
    {
        return(1);
    }
 */


	g_Que_TI_Master_to_Slave = xQueueCreate(1, sizeof(uint8_t)); // change from 64 to 128
	if(g_Que_TI_Master_to_Slave == 0)
	{
		return(1);
	}


	//
	// Create the COM_WiFiTask task.
	//
	if(xTaskCreate(COM_WiFiTask, (signed portCHAR *)"COM_WiFiTask",
			STACKSIZE_COM_WiFiTASK, NULL, tskIDLE_PRIORITY +
			PRIORITY_WiFi_TASK, NULL) != pdTRUE)
	{
		return(1);
	}

	//
	// Success.
	//
	return(0);
}

/**
 * Initialize the serial wifi task.
 * Chuck
 */
uint32_t
COM_1_WiFiTaskInit(void)
{/*
	g_Que_COM_LAN_WiFi = xQueueCreate(128, sizeof(uint8_t)); // change from 64 to 128


    if(g_Que_COM_LAN_WiFi == 0)
    {
        return(1);
    }
 */


	g_Que_TI_Master_to_Slave1 = xQueueCreate(1, sizeof(uint8_t)); // change from 64 to 128
	if(g_Que_TI_Master_to_Slave1 == 0)
	{
		return(1);
	}


	//
	// Create the COM_WiFiTask task.
	//
	if(xTaskCreate(COM1_WiFiTask, (signed portCHAR *)"COM1_WiFiTask",
			STACKSIZE_COM_WiFiTASK, NULL, tskIDLE_PRIORITY +
			PRIORITY_WiFi_TASK, NULL) != pdTRUE)
	{
		return(1);
	}

	//
	// Success.
	//
	return(0);
}

/**
 * Initialize the serial wifi task.
 * Chuck
 */
uint32_t
COM_2_WiFiTaskInit(void)
{/*
	g_Que_COM_LAN_WiFi = xQueueCreate(128, sizeof(uint8_t)); // change from 64 to 128


    if(g_Que_COM_LAN_WiFi == 0)
    {
        return(1);
    }
 */


	g_Que_TI_Master_to_Slave2 = xQueueCreate(1, sizeof(uint8_t)); // change from 64 to 128
	if(g_Que_TI_Master_to_Slave2 == 0)
	{
		return(1);
	}


	//
	// Create the COM_WiFiTask task.
	//
	if(xTaskCreate(COM2_WiFiTask, (signed portCHAR *)"COM2_WiFiTask",
			STACKSIZE_COM_WiFiTASK, NULL, tskIDLE_PRIORITY +
			PRIORITY_WiFi_TASK, NULL) != pdTRUE)
	{
		return(1);
	}

	//
	// Success.
	//
	return(0);
}





