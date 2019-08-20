/*
 * WiFiCMD.c
 *
 *  Created on: 2015Äê8ÔÂ17ÈÕ
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
#include "user_define.h"

extern _Bool WiFiCMD_Mode_FLag;
extern bool Wifi_Set_OK;
extern bool WiFly_Webaccess;
_Bool WiFi_Assistant_Set_flag;
//Set for 2 assitant wireless functions:
//1¡¢statemachine communication strings
//2¡¢wireless device search
void WiFi_Assistant_Set_old(void)

	/*1.$$$
	**2.set comm open ~o~
	**3.set comm close ~c~
	**4.set set comm remote 0
	**5.set b i 1 				//set board interval as 2s
	**6.set o d SerialToEthernet
	**.save
	**.reboot
	*/
{
		int retval;
		Switch_WiFi_to_COM = 0;
		WiFi_Assistant_Set_flag =true;
		vTaskDelay( 200/portTICK_RATE_MS );
		RingBufFlush(&g_sRxBuf[WiFi_Port]);
		//reboot wifi for each restart;
		send_reboot_cmd();
//		vTaskDelay(1000/portTICK_RATE_MS );
		delayms(100);
		if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode==2)
		WiFiSTAJoin();  // if user changed wifi AP's password, or the ssid is not existed, how to deal with this abnormal;

//		vTaskDelay( 200/portTICK_RATE_MS );
		// 1, sucess to enter cmd mode;
				if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
				{
					UARTprintf("Fail to enter CMD mode! \n");
				}
/*
				//11.set comm open ~o~
				if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm open ~o~" )))
				{
		            UARTprintf("Fail to set comm open ~o~\n");
				}
		//12.set comm close ~c~
				if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm close ~c~" )))
				{
			        UARTprintf("Fail to set comm close ~c~\n");
				}
				// don't send to clinet 'hello' when tcp connect,
				if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm remote 0" )))
				{
				    UARTprintf("Fail to set comm remote\n");
				}
				//Sets the interval (in seconds) at which the hello/heartbeat UDP
				//message is sent.
				if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set b i 1" )))
				{
			        UARTprintf("Fail to set b i 1\n");
				}
				//Sets the configurable device ID, where XXX is GSX for the
				//RN131 and EZX for the RN171.
				if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set o d %s",g_sParameters.ui8ModName )))
				{
				    UARTprintf("Fail to set deviceid\n");
				}
*/

				/*
				// set comm idle 2; close TCP connection when timer timerout, set to 2 seconds;
				if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm idle 5" )))
				{
				    UARTprintf("Fail to set comm idle 2\n");
				}
				*/
/*

#if HW_Triger_TCP
				//set sys iofunc 0x40, to enable wifi gpio6 and gpio5;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys iofunc 0x60" )))
								{
									 UARTprintf("enable wifi GIOP6 and gpio5\n");
								}

#else
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm idle 10" )))
{
    UARTprintf("Fail to set comm idle 5\n");
}

#endif


				//7.get mac address: "get mac";
						else if(RN_NOT_ERR_OK(retval, module_send_cmd("Mac Addr=", "get mac"))) // get the MAC address, and send to queue, and feedback to web page;
						{
		                  //  RN_RED_LED = 1;
							UARTprintf("Fail to get MAC address! \n");
						}

				*/
				//8.get ip address: "get ip";
			else if(RN_NOT_ERR_OK(retval, module_send_cmd("IP=", "get ip"))) // get the IP address, and send to queue, and feedback to web page;
			{
		                  //  RN_RED_LED = 1;
				UARTprintf("Fail to get IP address! \n");
				wifi_cmd_err_exit();

			}
				//11.exit command;
			else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
			{
				// RN_RED_LED = 1;
				UARTprintf("Fail to exit CMD  mode! \n");
			}
				/*
		//15.save ;
				if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
				{
                  UARTprintf("Fail to save ssid! \n");
				}
		//reboot
				if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
				{
                  UARTprintf("Reboot fail! \n");
				}

				*/

				if((g_sParameters.datachansel==2)||(g_sParameters.datachansel==3))//1, lan-com;2; wilfi-com;
				 Switch_WiFi_to_COM = 1;
				WiFiDynamicIP = WiFiIPGetandConvert();
	//			GetWiFiMACAddress();

/*	        	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);
	        	while(GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)&GPIO_PIN_6 != 0) {
	        		UARTprintf("\n wait gpio6: 0\n");
	        	}
*/
			    wifiCMD_Executing = false;

			    // clear all data;
			    SerialPurgeData(WiFi_Port, 0x03);
			    WiFi_Assistant_Set_flag =false;
}

void WiFi_Assistant_Set(void)

	/*1.$$$
	**2.set comm open ~o~
	**3.set comm close ~c~
	**4.set set comm remote 0
	**5.set b i 1 				//set board interval as 2s
	**6.set o d SerialToEthernet
	**.save
	**.reboot
	*/
{
		int retval;
		Switch_WiFi_to_COM = 0;
		WiFi_Assistant_Set_flag =true;
		vTaskDelay( 200/portTICK_RATE_MS );
		RingBufFlush(&g_sRxBuf[WiFi_Port]);
		//reboot wifi for each restart;

		WiFiGetIP_SysRST();

				if((g_sParameters.datachansel==2)||(g_sParameters.datachansel==3))//1, lan-com;2; wilfi-com;
				 Switch_WiFi_to_COM = 1;
//				WiFiDynamicIP = WiFiIPGetandConvert();

			    wifiCMD_Executing = false;

			    // clear all data;
			    SerialPurgeData(WiFi_Port, 0x03);
			    WiFi_Assistant_Set_flag =false;
}

bool wificmd_processing_flag;
//*****************************************************************************
//
// swap data between com and wifi uart;
//!
//
//*****************************************************************************
static void
WiFiCMDTask(void *pvParameters)
{
	int retval;
	//_Bool CMDModeOK_Flag = 0;

	tWiFiCMD sWiFiCMD;
//	portTickType xLastWakeTime;
//	 portBASE_TYPE xStatus;
//	 const portTickType xTicksToWait = 100 / portTICK_RATE_MS;

#if  Have_WiFi_Module_SKU
	//Set for assitant wireless functions
	if(g_sParameters.ui32WiFiModuleBaudRate != 9600)
	{
#if HW_Triger_TCP
    	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);
    	while(GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)&GPIO_PIN_6 != 0) {
    		UARTprintf("\n wait gpio6: 0\n");
    	}
#endif
		Wifi_Set_OK = false;
//		UARTprintf("\nui32WiFiModuleBaudRate : %d \n",g_sParameters.ui32WiFiModuleBaudRate );

		WiFi_Assistant_Set();
		Wifi_Set_OK = true;
	}
#endif
	 for( ;; )
	 {
		 // wifiCMD_stack_remaining = uxTaskGetStackHighWaterMark(NULL);
		  //UARTprintf("\nwifiCMD_stack_remaining_start : %d \n",wifiCMD_stack_remaining );

#if  Have_WiFi_Module_SKU
		 if((g_sParameters.ui32WiFiModuleBaudRate==9600)&&(wifi_restoring == true))
		 {
			 WiFiINI();
			 wifi_restoring = false;
		 }
#endif

		 xQueueReceive( g_QueWiFiCMD, &sWiFiCMD, portMAX_DELAY );

		 wificmd_processing_flag = true;
		 switch (sWiFiCMD.ui8Name)
		 {

		 case APMode_Non_DHCP_Mode_Setting: // static ip mode;
//			 ExitCMD();
//			 EnterCMD();
//			 WiFiModulereboot();
//			 ExitCMD();
			 APMode_Disable_DHCP_Server_Set();
			 //WiFly_Webaccess_Ready = true;
//			  wifiCMD_stack_remaining = uxTaskGetStackHighWaterMark(NULL);
//			  UARTprintf("\nwifiCMD_stack_remaining_AP_Non_DHCP : %d \n",wifiCMD_stack_remaining );
			// ExitCMD();
			 break;
		 case APMode_DHCP_Mode_Setting: // DHCP mode; Dynamic ip;
//			 ExitCMD();
//			 EnterCMD();
//			 WiFiModulereboot();
//			 ExitCMD();
			 APMode_Enable_DHCP_Server_Set();
			 //WiFly_Webaccess_Ready = true;
//			  wifiCMD_stack_remaining = uxTaskGetStackHighWaterMark(NULL);
//			  UARTprintf("\nwifiCMD_stack_remaining_AP_DHCP : %d \n",wifiCMD_stack_remaining );
			// ExitCMD();
			 break;

		 case STAMode_Non_DHCP_Mode_Setting: // static ip mode;
//			 ExitCMD();
//			 EnterCMD();
//			 WiFiModulereboot();
//			 ExitCMD();
			 STAMode_Non_DHCP_Mode_Set();
			 //WiFly_Webaccess_Ready = true;
//			  wifiCMD_stack_remaining = uxTaskGetStackHighWaterMark(NULL);
//			  UARTprintf("\nwifiCMD_stack_remaining_STA_Non_DHCP : %d \n",wifiCMD_stack_remaining );
//			 EnterCMD();
//			 WiFiModulereboot();
			// ExitCMD();
			 break;
		 case STAMode_DHCP_Mode_Setting: // DHCP mode; Dynamic ip;
//			 ExitCMD();
//			 EnterCMD();
//			 WiFiModulereboot();
//			 ExitCMD();
			 STAMode_DHCP_Mode_Set();
			 //WiFly_Webaccess_Ready = true;
//			  wifiCMD_stack_remaining = uxTaskGetStackHighWaterMark(NULL);
//			  UARTprintf("\nwifiCMD_stack_remaining_STA_DHCP : %d \n",wifiCMD_stack_remaining );
//			 EnterCMD();
//			 WiFiModulereboot();
			// ExitCMD();
			 break;

		 case WiFiSCAN: // DHCP mode; Dynamic ip;
//			 ExitCMD();
			 STAMode_Scan();
			// ExitCMD();
			 break;


		 case WiFi_DataTrans_TCP_Server: // Data wifi TCP Server
//		 	  ExitCMD();
		 	  WiFiData_TCP_S();
 			 break;
		 case WiFi_DataTrans_TCP_Client: // Data wifi TCP Client
//		 	  ExitCMD();
		 	 WiFiData_TCP_C(); // has get ip cmd;
 			 break;
		 case WiFi_DataTrans_UDP: // UDP
//		 	  ExitCMD();
		 	 WiFiData_UDP();
 			 break;
		 case WiFi_DeviceID_Altered: // Synchronize modname
//		 	  ExitCMD();
		 	 Syn_Modname();
 			 break;
 		case WiFi_DataTrans_State:
// 			 ExitCMD();
 			WiFiGet_DataTrans_State();
 			 break;
		 case GetWiFi_IPAdress: // DHCP mode; Dynamic ip;
			 GetWiFi_IP_CMD_Executing = true;
//			 ExitCMD();
//			 EnterCMD();
//			 WiFiModulereboot();
			 WiFiGetIP();
			// ExitCMD();
			 GetWiFi_IP_CMD_Executing = false;
			 break;
		 case GetWiFiMAC: // DHCP mode; Dynamic ip;
			 GetWiFi_IP_CMD_Executing = true;
//			 ExitCMD();
//			 EnterCMD();
//			 WiFiModulereboot();
			 WiFiGetIP();
			// ExitCMD();
			 GetWiFi_IP_CMD_Executing = false;
			 break;
		 case WiFi_Rejoin: // DHCP mode; Dynamic ip;

//			 ExitCMD();
//			 EnterCMD();
//			 WiFiModulereboot();
			 wifi_disconnect_rejoin();
			// ExitCMD();

			 break;

/*
	 		case WiFlyPage_TCP_Close:
	 			 ExitCMD();
	 			TCP_CLOSE();
	 			 break;
*/
		 default:UARTprintf("Invalid WiFi Command! \n");break;
		 }
		 wificmd_processing_flag = false;
//	 }
//	 else
//	 {
//		 UARTprintf("No commands in the xQueue \n");
//	 }
		// WiFiCMD_Mode_FLag = 0;
//		 vTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_RATE_MS ) );
		  //wifiCMD_stack_remaining = uxTaskGetStackHighWaterMark(NULL);
	//	  UARTprintf("\nwifiCMD_stack_remaining : %d \n",wifiCMD_stack_remaining );
	 }

}






//*****************************************************************************
//
// Initialize the serial peripherals and the serial task.
//
//*****************************************************************************
uint32_t
WiFiCMDTaskInit(void)
{


	g_QueWiFiCMD = xQueueCreate(1, sizeof(tWiFiCMD));

    if(g_QueWiFiCMD == 0)
    {
        return(1);
    }
	g_QueWiFiDataBack = xQueueCreate(1, sizeof(tWiFiCMD));
    if(g_QueWiFiDataBack == 0)
    {
        return(1);
    }
    g_QueWiFi_STA_DataBack = xQueueCreate(1, sizeof(tWiFiCMD));
    if(g_QueWiFi_STA_DataBack == 0)
    {
        return(1);
    }
    g_QueWiFi_IPDataBack = xQueueCreate(1, sizeof(tWiFiCMD));
    if(g_QueWiFi_IPDataBack == 0)
    {
        return(1);
    }

    g_QueWiFiScan_DataBack = xQueueCreate(1, sizeof(tWiFiCMD));
    if(g_QueWiFiScan_DataBack == 0)
    {
        return(1);
    }
    g_QueWiFi_APDataBack = xQueueCreate(1, sizeof(tWiFiCMD));
    if(g_QueWiFi_APDataBack == 0)
    {
        return(1);
    }
    g_QueWiFi_AP_NoDHCP_DataBack = xQueueCreate(1, sizeof(tWiFiCMD));
    if(g_QueWiFi_AP_NoDHCP_DataBack == 0)
    {
        return(1);
    }
    g_QueWiFi_STA_Rejoin_DataBack = xQueueCreate(1, sizeof(tWiFiCMD));
    if(g_QueWiFi_STA_Rejoin_DataBack == 0)
    {
        return(1);
    }
    //
    // Create the Serial task.
    //
    if(xTaskCreate(WiFiCMDTask, (signed portCHAR *)"WiFiCMD", STACKSIZE_WiFiCMDTASK, NULL, tskIDLE_PRIORITY + PRIORITY_WiFiCMD_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}



