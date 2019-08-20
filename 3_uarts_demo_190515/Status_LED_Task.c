/*
 * Status_LED_Task.c
 *
 *Display LED status information; Including WiFi returning infromation, and locator scan status indicator;
 *
 *  Created on: 2015Äê9ÔÂ15ÈÕ
 *      Author: pony
 */




#include "user_define.h"

#define Swap_Ready_LED_with_COM_TX_RX 1
#if Swap_Ready_LED_with_COM_TX_RX

#define Ready_LED_plus GPIO_PIN_4
#define Ready_LED_minus GPIO_PIN_5
#define Wifi_LED_plus GPIO_PIN_2
#define Wifi_LED_minus GPIO_PIN_3
#define COM_TX_RX_LED_plus GPIO_PIN_0
#define COM_TX_RX_LED_minus GPIO_PIN_1

#else

#define Ready_LED_plus GPIO_PIN_0
#define Ready_LED_minus GPIO_PIN_1
#define Wifi_LED_plus GPIO_PIN_2
#define Wifi_LED_minus GPIO_PIN_3
#define COM_TX_RX_LED_plus GPIO_PIN_4
#define COM_TX_RX_LED_minus GPIO_PIN_5

#endif
// Swap_Ready_LED_with_COM_TX_RX


void Locator_LED(void)
{
	    //Ready LED: Green blinking, located by administrator's location function;
	    //red: PD1 1, PD0 0;  Green PD1 0, PD0 1;
	    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Ready_LED_plus | Ready_LED_minus), (Ready_LED_plus | Ready_LED_minus)); //led light out;
	    		vTaskDelay( 150/portTICK_RATE_MS );
	    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Ready_LED_plus | Ready_LED_minus), Ready_LED_plus); //green on;
	    		locator_scan =locator_scan -1;
}
void WiFiStatus_LED_Green(void)
{
	    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), Wifi_LED_plus); //green on;
}

void WiFiStatus_LED_Green_Flash(void)
{
	if(WiFiLEDflashtime !=0)
	{
		if(locator_scan==true)
		{
		    //Ready LED: Green blinking, located by administrator's location function;
		    //red: PD3 1, PD2 0;  Green PD3 0, PD2 1;
		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), (Wifi_LED_plus | Wifi_LED_minus)); //led light out;
		    		vTaskDelay( 150/portTICK_RATE_MS );
		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), Wifi_LED_plus); //green on;
		    		vTaskDelay( 150/portTICK_RATE_MS );
		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), (Wifi_LED_plus | Wifi_LED_minus)); //led light out;
		    		vTaskDelay( 150/portTICK_RATE_MS );
		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), Wifi_LED_plus); //green on;
		    		WiFiLEDflashtime = WiFiLEDflashtime-1;
		}
		else if(locator_scan==false)
		{
    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), (Wifi_LED_plus | Wifi_LED_minus)); //led light out;
    		vTaskDelay( 150/portTICK_RATE_MS );
    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), Wifi_LED_plus); //green on;
    		vTaskDelay( 150/portTICK_RATE_MS );
    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), (Wifi_LED_plus | Wifi_LED_minus)); //led light out;
    		vTaskDelay( 150/portTICK_RATE_MS );
    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), Wifi_LED_plus); //green on;
    		vTaskDelay( 150/portTICK_RATE_MS );
    		WiFiLEDflashtime = WiFiLEDflashtime-1;
		}
	}
	else WiFi_Status_Retval = WiFi_LED_Off;

}
void WiFiStatus_LED_Off(void)
{
    //wifistatus led out;
    //red: PD3 1, PD2 0;  Green PD3 0, PD2 1;
    ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), 0); //led light out;
}

void AllStatusLEDoff(void)
{
	//COM RX/TX status led light out; red: PD5 1, PD4 0;  Green PD5 0, PD4 1;
	ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), 0); //led light out;
	//locator search led /power ready led ;    //red: PD1 1, PD0 0;  Green PD1 0, PD0 1;
    ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Ready_LED_plus | Ready_LED_minus), 0); //led light out;
    //wifistatus led out;
    //red: PD3 1, PD2 0;  Green PD3 0, PD2 1;
    ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), 0); //led light out;

}
void WiFi_RX_TX_LED_Defaultoff(void)
{
	//COM RX/TX status led light out; red: PD5 1, PD4 0;  Green PD5 0, PD4 1;
	ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), 0); //led light out;
    //wifistatus led out;
    //red: PD3 1, PD2 0;  Green PD3 0, PD2 1;
    ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), 0); //led light out;
}
void SysreadyLED_Green(void)
{

	//red: PD1 1, PD0 0;  Green PD1 0, PD0 1;
    //Ready LED: Green, power is on, and function is normally;
    ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Ready_LED_plus | Ready_LED_minus), Ready_LED_plus); //+  light green led;

}
void SysHaltLED_RED(void)
{

	//red: PD1 1, PD0 0;  Green PD1 0, PD0 1;
    //Ready LED: Green, power is on, and function is normally;
    ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Ready_LED_plus | Ready_LED_minus), Ready_LED_minus); //+  light green led;

}
   // User LED;
// PD0/PD1 ready+/-; PD2/PD3 link+/-; PD4/PD5 tx/rx  +/-;
//red: PD1 1, PD0 0;  Green PD1 0, PD0 1;
void COM_TX_LED_Green(void)
{
	    		if(locator_scan!=0)
	    		{
	    		    //Ready LED: Green blinking, located by administrator's location function;
	    		    //red: PD5 1, PD4 0;  Green PD5 0, PD4 1;
	    		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus)); //led light out;
	    		    		vTaskDelay( 150/portTICK_RATE_MS );
	    		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), COM_TX_RX_LED_plus); //green on;
	    		    		txdata_sending = txdata_sending-1;
	    		    		if(txdata_sending ==0)
	    		    			ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus)); //led light out;
	    		}
	    		else if(locator_scan==0)
	    		{
		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus)); //led light out;
		    		vTaskDelay( 150/portTICK_RATE_MS );
		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), COM_TX_RX_LED_plus); //green on;
		    		vTaskDelay( 150/portTICK_RATE_MS );
		    		txdata_sending = txdata_sending-1;
		    		if(txdata_sending ==0)
		    			ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus)); //led light out;
	    		}
}

void COM_RX_LED_Red(void)
{
	    		if(locator_scan!=0)
	    		{
	    		    //Ready LED: Green blinking, located by administrator's location function;
	    		    //red: PD5 1, PD4 0;  Green PD5 0, PD4 1;
	    		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus)); //led light out;
	    		    		vTaskDelay( 150/portTICK_RATE_MS );
	    		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), COM_TX_RX_LED_minus); //red on;
	    		    		rxdata_receiving = rxdata_receiving -1;
	    		    		if(rxdata_receiving ==0)
	    		    			ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus)); //led light out;
	    		}
	    		else if(locator_scan==0)
	    		{
		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus)); //led light out;
		    		vTaskDelay( 150/portTICK_RATE_MS );
		    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), COM_TX_RX_LED_minus); //red on;
		    		vTaskDelay( 150/portTICK_RATE_MS );
		    		rxdata_receiving = rxdata_receiving -1;
		    		if(rxdata_receiving ==0)
		    			ROM_GPIOPinWrite(GPIO_PORTD_BASE, (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus), (COM_TX_RX_LED_plus | COM_TX_RX_LED_minus)); //led light out;
	    		}
}

void WiFiStatus_LED_Red(void)
{
	if(locator_scan!=0)
	{
	    //Ready LED: Green blinking, located by administrator's location function;
	    //red: PD3 1, PD2 0;  Green PD3 0, PD2 1;
	    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), (Wifi_LED_plus | Wifi_LED_minus)); //led light out;
	    		vTaskDelay( 150/portTICK_RATE_MS );
	    		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), Wifi_LED_minus); //red on;
	}
	else if(locator_scan==0)
	{
		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), (Wifi_LED_plus | Wifi_LED_minus)); //led light out;
		vTaskDelay( 150/portTICK_RATE_MS );
		ROM_GPIOPinWrite(GPIO_PORTD_BASE, (Wifi_LED_plus | Wifi_LED_minus), Wifi_LED_minus); //red on;
		vTaskDelay( 150/portTICK_RATE_MS );
	}
}

void wifi_disconnect_rejoin(void)
{
	// rejoin;
//	module_cmd_mode();
	//delayms(1000);
	// join
	tWiFiCMD pxMessage;
	int retval;
	if(RN_NOT_ERR_OK(retval, module_send_cmd("CMD", "$$$")))
			{
			//if(RN_NOT_ERR_OK(retval, module_send_cmd("lease=", "join")))
			//{
				UARTprintf("Fail to cmd! \n");
				wifi_cmd_err_exit();
			//}
			}
	// 4.set STA mode ap's ssid:						"set wlan ssid %s";
			else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w s %s",g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName )))
			{
	                    //RN_RED_LED = 1;
				UARTprintf("Fail to set STA mode ap's ssid! \n");
				wifi_cmd_err_exit();
			}
	//6.save command;
			else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
			{
              //  RN_RED_LED = 1;
				UARTprintf("Fail to save ssid! \n");
				wifi_cmd_err_exit();
			}
	else if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
			{
			//if(RN_NOT_ERR_OK(retval, module_send_cmd("lease=", "join")))
			//{
				UARTprintf("Fail to reboot! \n");
				wifi_cmd_err_exit();
			//}
			}
	else  	UARTprintf("reboot ok! \n");

	vTaskDelay(1000/portTICK_RATE_MS );
//	module_cmd_mode();
	if(RN_NOT_ERR_OK(retval, module_send_cmd("CMD", "$$$")))
			{
			//if(RN_NOT_ERR_OK(retval, module_send_cmd("lease=", "join")))
			//{
				UARTprintf("Fail to cmd! \n");
				wifi_cmd_err_exit();
			//}
			}
	else if(RN_NOT_ERR_OK(retval, module_send_cmd("Associ", "join")))
	{
	//if(RN_NOT_ERR_OK(retval, module_send_cmd("lease=", "join")))
	//{
		UARTprintf("Fail to join! \n");
		wifi_cmd_err_exit();
	//}
	}
	else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
	{
	//if(RN_NOT_ERR_OK(retval, module_send_cmd("lease=", "join")))
	//{
		UARTprintf("Fail to exit! \n");
//		wifi_cmd_err_exit();
	//}
	}
	else UARTprintf("dis rejoin ok \n");

	pxMessage.ui8Name = WiFi_Rejoin;
	xQueueSendToBack( g_QueWiFi_STA_Rejoin_DataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;


//							GPIO_ReadBack = GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_4);
}

static void
WiFi_StatusLED_Task(void *pvParameters)
{

//	uint8_t ;
	portTickType xLastWakeTime;
//	WiFi_Status_Retval = 0;
	uint8_t GPIO_ReadBack;
	tWiFiCMD pxMessage,sWiFiCMD;
	uint8_t counter_reconn_wifi_STA = 0;
   while(1)
   {
	xLastWakeTime = xTaskGetTickCount();

	  //monitor wifi connection by reading gpio4(MCU site PC4);
	  //This pin goes high after the module has associated/authenticated and has an IP address.

		if((g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == STA_Mode)&&(!(WiFi_Setting))&&(counter_reconn_wifi_STA == 60))
		{
			GPIO_ReadBack = GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_4);
//				UARTprintf("rbback:0x%02X\n",GPIO_ReadBack);

			if((GPIO_ReadBack & GPIO_PIN_4) == 0) // 1,associated, 0, disconnected;
			{
				UARTprintf("rejion! \n");
#if  Have_WiFi_Module_SKU
		       pxMessage.ui8Name = WiFi_Rejoin;
		       xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
		       UARTprintf("STA rejoin cmd to Que: 0x%x \n", pxMessage.ui8Name);
					//get feedback data and send to web;
					 xQueueReceive( g_QueWiFi_STA_Rejoin_DataBack, &sWiFiCMD, portMAX_DELAY );
#endif


			}
			counter_reconn_wifi_STA = 0;

		}
		counter_reconn_wifi_STA++;

	if((locator_scan!=0))
	{
		// led flash 1 times; on/off;
		Locator_LED();

	}
	if(txdata_sending !=0) //send data;
	{
		// flash green;
		COM_TX_LED_Green();
	}
	/*
	else if((rxdata_receiving !=0)&&(txdata_sending ==0)) //receiving data;
	{
		// flash red light;
		COM_RX_LED_Red(); // changed to orange LED;
	}
	*/
	if(rxdata_receiving !=0) //receiving data;
	{
		// flash red light;
		COM_RX_LED_Red();
	}

	// WiFi error information;
		switch(WiFi_Status_Retval)
		{
//		case WiFi_LED_Off:WiFiStatus_LED_Off();break;
//		case WiFi_Normal: WiFiStatus_LED_Green();break;
		case WiFi_Normal: WiFiStatus_LED_Off();break;
		case WiFi_Datain: WiFiStatus_LED_Green_Flash();break;
		case WiFi_URBUS_Error:;break;
		case WiFi_INI_Error:;break;
//		case WiFi_EnterCMD_Error:WiFiStatus_LED_Red();break;
		case WiFi_LED_Off:WiFiStatus_LED_Off();break;
		default:;break;
		}
	vTaskDelayUntil( &xLastWakeTime, ( 300 / portTICK_RATE_MS ) );
   }
}


//*****************************************************************************
//
// Initialize the serial peripherals and the serial task.
//
//*****************************************************************************
uint32_t
WiFi_StatusLED_TaskInit(void)
{

    //
    // Create the Serial task.
    //
    if(xTaskCreate(WiFi_StatusLED_Task, (signed portCHAR *)"WiFi_StatusLED_Task", STACKSIZE_SerialTASK, NULL, tskIDLE_PRIORITY + PRIORITY_WiFiCMD_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}
