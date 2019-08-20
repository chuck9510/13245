/*
 * microchipWiFi.c
 *
 *  Created on: 2015年8月17日
 *      Author: pony
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "user_define.h"
#include "config.h"
#include  <task.h>
#include  <math.h>
#include "serial.h"
static uint8_t appendCRLF = 1;

uint8_t RSSI[wifi_scan_qty],Scaned_AP_Num;
char WiFi_SSID_temp[wifi_scan_qty][NET_NAME_LEN];
uint8_t securitymode[wifi_scan_qty];
char wifiini_data_response[100];
extern bool Wifly_Acc;
bool wifiCMD_Executing;
void ConsolePut(char c)
{
   	 SerialSend(WiFi_Port, c);

}
void ConsolePut_to_COM(char c)
{
   	 SerialSend(COM_Port, c);

}
void ConsolePutString(char* str)
{
    char c;
    while( c = *str++ )
        ConsolePut(c);
}

void ConsolePutString_to_COM(char* str)
{
    char c;
    while( c = *str++ )
        ConsolePut_to_COM(c);
}

 /*Converts an integer to a string. */
char *my_itoa(char *str,int value,int radix)
{
	int i,j,sign;
	char ps[256];
	memset(ps,0,256);
	if((radix>36) || (radix<2)) return 0;   sign = 0; if(value<0)
	{
		sign=-1;   value=-value;
	}
	i=0;
	do
	{
		if((value % radix) > 9)
			ps[i] = value % radix + '0' + 39;
		else
			ps[i] = value % radix + '0' ;
		i++;
	}
	while((value /= radix) > 0);
	if(sign<0)
		ps[i]='-';
	else i--;
	for(j=i;j>=0;j--)
		str[i-j]=ps[j];
	str[i+1] = '\0'; //added by pony;
	return str;
}
/*
#include <memory.h>
void myitoa(char *tmp,)
{ char tmp[30];
//init
memset(tmp, 0x00, 30);
sprintf(tmp, "%d", 34);//

}
*/
void WiFiReset(void)
{
//	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_PIN_7);// set wifi gpio9 high;
 //   delayms(100);
    ROM_GPIOPinWrite(WiFi_Reset_Port, WiFi_Reset_Pin, WiFi_Reset_Pin);// set wifi reset high;
    delayms(10);
    ROM_GPIOPinWrite(WiFi_Reset_Port, WiFi_Reset_Pin, 0);// set wifi reset low;
    delayms(10);
    ROM_GPIOPinWrite(WiFi_Reset_Port, WiFi_Reset_Pin, WiFi_Reset_Pin);// set wifi reset high;
    delayms(10);
  //  ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0);// set wifi gpio9 high;
}

void receivewifi_ini_data(void)
{
	int i=0;
	//Wait until send condition ok;
	vTaskDelay( 1000/portTICK_RATE_MS );

//	while(RingBufUsed(&g_sRxBuf[WiFi_Port]) == 0){};
	//Actively read rx ringbuffer until it's empty; and wait tx ringbuffer empty;
  	 while((RingBufUsed(&g_sRxBuf[WiFi_Port]) != 0) || (RingBufUsed(&g_sTxBuf[WiFi_Port])  != 0) )
  			 {
  		wifiini_data_response[i] =	 SerialReceive(WiFi_Port);
  		 	 	 i++;
  			 }
  	wifiini_data_response[i]='\0';
//  	UARTprintf("wifiini_data_response:\n%s\n",wifiini_data_response);
  	 i=0;

		if(g_sParameters.datachansel==2)//1, lan-com;2; wilfi-com;
		{
			Switch_WiFi_to_COM = 1;
			Switch_LAN_to_COM = 0;
		}
		else if(g_sParameters.datachansel==1)//1, lan-com;2; wilfi-com;
		{
			Switch_WiFi_to_COM = 0;
			Switch_LAN_to_COM = 1;
		}

	}
char *ipconvtosring(uint32_t ui32IPAddr) //fail to set ip address;
{
    usnprintf(wifi_ip_temp, 16, "%d.%d.%d.%d",
                     ((ui32IPAddr >>  24) & 0xFF),
                     ((ui32IPAddr >>  16) & 0xFF),
                     ((ui32IPAddr >> 8) & 0xFF),
                     ((ui32IPAddr >> 0) & 0xFF));
	//UARTprintf("ip converted to string, and to be sent:%s!\n",wifi_ip_temp);
	return(wifi_ip_temp);
}

void wifi_cmd_err_exit(void)
{
	int retval;
	if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
	UARTprintf("Fail to exit wifi cmd err mode! \n");
}

int SetWiFi_UR_Baudrate(uint32_t baudrate)
{
	uint8_t timeout_number = 0;
	int retval;
	receivewifi_ini_data();
	ExitCMD();
	/*
	 EnterCMD();
	 WiFiModulereboot();
	 ExitCMD();
*/
	vTaskDelay( 100/portTICK_RATE_MS );


					Switch_WiFi_to_COM = 0;
					Switch_LAN_to_COM = 0;


	//need to assure enter CMD mode;


	retval =RN_NOT_ERR_OK(retval, module_send_cmd("CMD", "$$$")); // if retval is 0, cmd ok, else if  1, cmd error;


	while(retval&&(timeout_number < CMD_retry_time_external))
	{
		wifimoduledefault();

//		ExitCMD();
//		vTaskDelay( 500/portTICK_RATE_MS );
		retval =RN_NOT_ERR_OK(retval, module_send_cmd("CMD", "$$$"));

		timeout_number++;
	}
	//if still error;



//	if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
//	{
		//RN_RED_LED = 1;
//		UARTprintf("Fail to enter CMD mode! \n");
//		retval = WiFi_ExeCMD_Error;


//	}
//	else //set uart flow 0x21:  Even parity with flow control;
//	{

		//1. "set uart baudrate 115200";
		if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set u b %d",baudrate))) //exit ap mode:
		{
                 //   RN_RED_LED = 1;
			UARTprintf("Fail to set baudrate! \n");
			wifi_cmd_err_exit();
			retval = WiFi_ExeCMD_Error;
		}
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set uart flow 0x21")))
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set uart flow 1")))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set u f 1")))
		{
		                   // RN_RED_LED = 1;
			retval = WiFi_ExeCMD_Error;
			UARTprintf("Fail to exit station mode! \n");
			wifi_cmd_err_exit();
		}
		//UDP broadcase time interval;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set b i 1" )))
		{
	        UARTprintf("Fail to set b i 1\n");
	        wifi_cmd_err_exit();
		}
		// change modenaem;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set o d %s",g_sParameters.ui8ModName )))
		{
		    UARTprintf("Fail to set deviceid\n");
		    wifi_cmd_err_exit();
		}
		// don't send to clinet 'hello' when tcp connect,
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm remote 0" )))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set c r 0" )))
		{
		    UARTprintf("Fail to set comm remote\n");
		    wifi_cmd_err_exit();
		}
		// don't print wifi debug information through wifi-UR port,
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys printlvl 0" )))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set s p 0" )))
		{
		    UARTprintf("Fail to set sys print 1\n");
		    wifi_cmd_err_exit();
		}
		// set ip localport 80, default is 2000;
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip localport 80" )))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set i l %d",g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort )))
		{
		    UARTprintf("Fail to set ip localport 2000\n");
		    wifi_cmd_err_exit();
		}

		//11.set comm open ~o~
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm open ~o~" )))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set c o ~o~" )))
						{
				            UARTprintf("Fail to set comm open ~o~\n");
				            wifi_cmd_err_exit();
						}
				//12.set comm close ~c~
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm close ~c~" )))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set c c ~c~" )))
						{
					        UARTprintf("Fail to set comm close ~c~\n");
					        wifi_cmd_err_exit();
						}
		//set sys iofunc 0x10, to enable wifi gpio4;
//		This pin goes high after the module has associated/authenticated and has an IP address.
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys iofunc 0x10" )))
						{
							 UARTprintf("fail set sys iofunc 0x10\n");
							 wifi_cmd_err_exit();
						}

		//set secondary broadcast frame, 120 bytes, including IP address and MAC;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set broadcast address 0.0.0.0" )))
						{
							 UARTprintf("fail to set broadcast address to 0.0.0.0\n");
							 wifi_cmd_err_exit();
						}
		//set secondary broadcast frame, 120 bytes, including IP address and MAC;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set broadcast backup 255.255.255.255" )))
						{
							 UARTprintf("fail to set broadcast backup to 255.255.255.255\n");
							 wifi_cmd_err_exit();
						} //set broadcast remote
		//set secondary broadcast frame, 120 bytes, including IP address and MAC;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set broadcast remote 55554" )))
						{
							 UARTprintf("fail to set broadcast remote 55554\n");
							 wifi_cmd_err_exit();
						} //set broadcast remote


		/*
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys iofunc 0x70" )))
						{
							 UARTprintf("Fail to set GIOP5\n");
						}
		//set sys iofunc 0x20, to enable wifi gpio5;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys iofunc 0x20" )))
						{
							 UARTprintf("enable wifi GIOP5\n");
						}
		//set sys iofunc 0x40, to enable wifi gpio6;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys iofunc 0x40" )))
						{
							 UARTprintf("enable wifi GIOP6\n");
						}
		*/
#if HW_Triger_TCP
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys iofunc 0x60" )))
						{
							 UARTprintf("enable wifi GIOP6 and gpio5\n");
						}
#else

		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm idle 0" )))
{
    UARTprintf("Fail to set comm idle 0\n");
}

#endif
		// restore WiFi module to AP  DHCP server mode:
		// ssid: mima12345679;              pass:12345678;
		// ip address:   192.168.1.106
		// subnet mask:  255.255.255.0;
		// gatway:       192.168.1.106;

		// 1. enable ap mode:"set wlan join 7"
	else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan join 7")))
		{
                 //   RN_RED_LED = 1;
			UARTprintf("Fail to ap mode! \n");
			wifi_cmd_err_exit();
		}
		// 3.set frequency:"set w c %d";  need to use paras;    											 v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w c %d",g_sParameters.sWiFiModule.sWiFiAPSet.ucFrequency_Channel )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless frequency! \n");
					wifi_cmd_err_exit();
				}
		//5.set ap's ssid:"set apmode ssid %s"; need to use one para; ssid network name;           			  v
//				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set apmode ssid %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName )))
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set a s %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's SSID! \n");
					wifi_cmd_err_exit();
				}
		//4.set security mode:"set apmode passphrase %s"; need to use one para; wep-128 type; security; 	 v
				//This command sets the soft AP mode passphrase to be used for WPA2-AES encryp-
				//tion. When set, the module will broadcast a network in soft AP mode with WAP2-AES
				//encryption enabled. The allowable length of <string> is between 8 and 64 characters;
//				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set apmode passphrase %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords )))// need check;g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set a p %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless security type! \n");
					wifi_cmd_err_exit();
				}

		// 2. enable AP's dhcp server:"set ip dhcp 4"
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip dhcp 4")))
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to set wireless DHCP mode! \n");
					wifi_cmd_err_exit();
				}
		/*
		// 2. enable AP's dhcp server:"set wlan auth 5"
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan auth 5")))
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to set wireless security Auth mode! \n");
				}
		// 2. enable AP's dhcp server:"set dhcp lease 864000" 10 days;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set dhcp lease 864000")))
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to set wireless lease time! \n");
				}

*/
		//5.set ip address:"set ip address %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip address %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's IP address! \n");
					wifi_cmd_err_exit();
				}

		//5.set ap's ssid:"set ip net %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip net %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's IP address! \n");
					wifi_cmd_err_exit();
				}
		//5.set ap's gateway:"set ip gateway %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip gateway %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's gateway address! \n");
					wifi_cmd_err_exit();
				}
		// set remote host ip 192.168.0.11 for wifi web connect;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip host %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr) )))
					{
					        //RN_RED_LED = 1;
						UARTprintf("set host ip! \n");
						wifi_cmd_err_exit();
					}
/*
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm idle 5" )))
		{
		    UARTprintf("Fail to set comm idle 2\n");
		}
*/
		//get mac;
		//7.get mac address: "get mac";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Mac Addr=", "get mac"))) // get the MAC address, and send to queue, and feedback to web page;
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to get MAC address! \n");
					wifi_cmd_err_exit();
				}

		else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
		{
		                   // RN_RED_LED = 1;
			retval = WiFi_ExeCMD_Error;
			UARTprintf("Fail to exit station mode! \n");
			wifi_cmd_err_exit();
		}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
		{
          //  RN_RED_LED = 1;
			UARTprintf("Reboot fail! \n");
			retval = WiFi_ExeCMD_Error;
			wifi_cmd_err_exit();
		}

		else{
			retval = RN_ERR_OK;
			if(!Wifly_Acc)
			GetWiFiMACAddress();
#if HW_Triger_TCP
        	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);
        	while(GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)&GPIO_PIN_6 != 0) {
        		UARTprintf("\n wait gpio6: 0\n");
        	}
#endif

		}

//	}
		resume_channel_connection();
	return retval;
}

int SetWiFi_UR_Baudrate_(uint32_t baudrate)
{
	uint8_t timeout_number = 0;
	int retval;
	receivewifi_ini_data();
	ExitCMD();
	/*
	 EnterCMD();
	 WiFiModulereboot();
	 ExitCMD();
*/
	vTaskDelay( 100/portTICK_RATE_MS );


					Switch_WiFi_to_COM = 0;
					Switch_LAN_to_COM = 0;


	//need to assure enter CMD mode;


	retval =RN_NOT_ERR_OK(retval, module_send_cmd("CMD", "$$$")); // if retval is 0, cmd ok, else if  1, cmd error;


	while(retval&&(timeout_number < CMD_retry_time_external))
	{
		wifimoduledefault();

//		ExitCMD();
//		vTaskDelay( 500/portTICK_RATE_MS );
		retval =RN_NOT_ERR_OK(retval, module_send_cmd("CMD", "$$$"));

		timeout_number++;
	}
	//if still error;



//	if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
//	{
		//RN_RED_LED = 1;
//		UARTprintf("Fail to enter CMD mode! \n");
//		retval = WiFi_ExeCMD_Error;


//	}
//	else //set uart flow 0x21:  Even parity with flow control;
//	{

		//1. "set uart baudrate 115200";
		if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set u b %d",baudrate))) //exit ap mode:
		{
                 //   RN_RED_LED = 1;
			UARTprintf("Fail to set baudrate! \n");
			retval = WiFi_ExeCMD_Error;
			wifi_cmd_err_exit();
		}
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set uart flow 0x21")))
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set uart flow 1")))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set u f 1")))
		{
		                   // RN_RED_LED = 1;
			retval = WiFi_ExeCMD_Error;
			UARTprintf("Fail to exit station mode! \n");
			wifi_cmd_err_exit();
		}
		//UDP broadcase time interval;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set b i 1" )))
		{
	        UARTprintf("Fail to set b i 1\n");
	        wifi_cmd_err_exit();
		}
		// change modenaem;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set o d %s",g_sParameters.ui8ModName )))
		{
		    UARTprintf("Fail to set deviceid\n");
		    wifi_cmd_err_exit();
		}
		// don't send to clinet 'hello' when tcp connect,
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm remote 0" )))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set c r 0" )))
		{
		    UARTprintf("Fail to set comm remote\n");
		    wifi_cmd_err_exit();
		}
		// don't print wifi debug information through wifi-UR port,
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys printlvl 0" )))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set s p 0" )))
		{
		    UARTprintf("Fail to set sys print 1\n");
		    wifi_cmd_err_exit();
		}
		// set ip localport 80, default is 2000;
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip localport 80" )))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set i l %d",g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort )))
		{
		    UARTprintf("Fail to set ip localport 2000\n");
		    wifi_cmd_err_exit();
		}

		//11.set comm open ~o~
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm open ~o~" )))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set c o ~o~" )))
						{
				            UARTprintf("Fail to set comm open ~o~\n");
				            wifi_cmd_err_exit();
						}
				//12.set comm close ~c~
//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm close ~c~" )))
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set c c ~c~" )))
						{
					        UARTprintf("Fail to set comm close ~c~\n");
					        wifi_cmd_err_exit();
						}
		/*
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys iofunc 0x70" )))
						{
							 UARTprintf("Fail to set GIOP5\n");
						}
		//set sys iofunc 0x20, to enable wifi gpio5;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys iofunc 0x20" )))
						{
							 UARTprintf("enable wifi GIOP5\n");
						}
		//set sys iofunc 0x40, to enable wifi gpio6;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys iofunc 0x40" )))
						{
							 UARTprintf("enable wifi GIOP6\n");
						}
		*/
#if HW_Triger_TCP
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set sys iofunc 0x60" )))
						{
							 UARTprintf("enable wifi GIOP6 and gpio5\n");
							 wifi_cmd_err_exit();
						}
#else
/*
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm idle 5" )))
{
    UARTprintf("Fail to set comm idle 3\n");
}
*/
#endif
		// restore WiFi module to AP  DHCP server mode:
		// ssid: mima12345679;              pass:12345678;
		// ip address:   192.168.1.106
		// subnet mask:  255.255.255.0;
		// gatway:       192.168.1.106;


/*
		// 1. enable ap mode:"set wlan join 7"
	else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan join 7")))
		{
                 //   RN_RED_LED = 1;
			UARTprintf("Fail to ap mode! \n");
		}
		// 3.set frequency:"set w c %d";  need to use paras;    											 v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w c %d",g_sParameters.sWiFiModule.sWiFiAPSet.ucFrequency_Channel )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless frequency! \n");
				}
		//5.set ap's ssid:"set apmode ssid %s"; need to use one para; ssid network name;           			  v
//				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set apmode ssid %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName )))
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set a s %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's SSID! \n");
				}
		//4.set security mode:"set apmode passphrase %s"; need to use one para; wep-128 type; security; 	 v
				//This command sets the soft AP mode passphrase to be used for WPA2-AES encryp-
				//tion. When set, the module will broadcast a network in soft AP mode with WAP2-AES
				//encryption enabled. The allowable length of <string> is between 8 and 64 characters;
//				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set apmode passphrase %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords )))// need check;g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set a p %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless security type! \n");
				}

		// 2. enable AP's dhcp server:"set ip dhcp 4"
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip dhcp 4")))
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to set wireless DHCP mode! \n");
				}

		// 2. enable AP's dhcp server:"set wlan auth 5"
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan auth 5")))
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to set wireless security Auth mode! \n");
				}
		// 2. enable AP's dhcp server:"set dhcp lease 864000" 10 days;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set dhcp lease 864000")))
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to set wireless lease time! \n");
				}


		//5.set ip address:"set ip address %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip address %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's IP address! \n");
				}

		//5.set ap's ssid:"set ip net %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip net %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's IP address! \n");
				}
		//5.set ap's gateway:"set ip gateway %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip gateway %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's gateway address! \n");
				}
		// set remote host ip 192.168.0.11 for wifi web connect;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip host %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr) )))
					{
					        //RN_RED_LED = 1;
						UARTprintf("set host ip! \n");
					}

				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set comm idle 5" )))
		{
		    UARTprintf("Fail to set comm idle 2\n");
		}
*/
		//get mac;
		//7.get mac address: "get mac";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Mac Addr=", "get mac"))) // get the MAC address, and send to queue, and feedback to web page;
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to get MAC address! \n");
					wifi_cmd_err_exit();
				}

		else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
		{
		                   // RN_RED_LED = 1;
			retval = WiFi_ExeCMD_Error;
			UARTprintf("Fail to exit station mode! \n");
			wifi_cmd_err_exit();
		}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
		{
          //  RN_RED_LED = 1;
			UARTprintf("Reboot fail! \n");
			retval = WiFi_ExeCMD_Error;
			wifi_cmd_err_exit();
		}


		else{
			retval = RN_ERR_OK;
			if(!Wifly_Acc)
			GetWiFiMACAddress();
#if HW_Triger_TCP
        	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);
        	while(GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)&GPIO_PIN_6 != 0) {
        		UARTprintf("\n wait gpio6: 0\n");
        	}
#endif

		}

//	}
		resume_channel_connection();
	return retval;
}


int GetWiFiMACAddress(void)
{
	uint32_t digit_num,i,j,k;
	uint8_t mac_digit;
	int retval;
	char *p;
	char *pIndex;
	char wifi_mac_temp[3];
	uint8_t wifi_mac[6];
	if((pIndex = strstr((const char *)ResponseFromWiFi_Mac, "Mac Addr=")) !=NULL ) // find what we want;
	{
		pIndex +=9;
		for(j=0,digit_num=0;j<6;j++)
		{
		while((*pIndex <= '9')&&(*pIndex >= '0')||(*pIndex <= 'f')&&(*pIndex >= 'a'))
			{
			wifi_mac_temp[digit_num] = *pIndex; //wifi_mac_temp[0] is weight bit;
				pIndex++;
				digit_num++; //digit_num should be 2;
			}
		p= wifi_mac_temp;
		p[2] = '\0';

			for(i=0,mac_digit=0;i<digit_num;i++)  // add "="
				{
				if((p[i]  <= '9')&&(p[i]  >= '0'))
					mac_digit = mac_digit+(int)(wifi_mac_temp[i]-'0')*pow(16,digit_num-1-i); // remove "-1";
				else if((p[i] <= 'f')&&(p[i] >= 'a'))
				{
					mac_digit = mac_digit+(wifi_mac_temp[i]-'a'+10)*pow(16,digit_num-1-i);
				}
//				UARTprintf("loop: %d mac_digit: %X \n",i,mac_digit);
				}

		//get one of the 6 mac digit;
		wifi_mac[j] = mac_digit;

//		UARTprintf("loop 6 of %d: 0x%X.",j,mac_digit);
		pIndex++;
		digit_num=0;
		}

//		wifi_mac[6] = '\0';
//		strcpy((char *)g_sParameters.sWiFiModule.ucWiFiMAC,(const char *)wifi_mac);
//		UARTprintf("\n");
//		UARTprintf("WiFi mac :\n");
/*
		for(k=0;k<6;k++)
		{
			UARTprintf("%X:",wifi_mac[k]);

		}
		UARTprintf("\n");
		*/
//		UARTprintf("WiFi mac in gsparas :\n");
		// copy mac;
		for(k=0;k<6;k++)
		{
			g_sParameters.sWiFiModule.ucWiFiMAC[k] = wifi_mac[k];

		}
/*
		for(k=0;k<6;k++)
		{
			UARTprintf("%X:",g_sParameters.sWiFiModule.ucWiFiMAC[k]);

		}
		UARTprintf("\n");
		*/
		retval = 0;

	}
	else{

		UARTprintf("WiFi MAC not found!\n");
		retval = 1;
	}
	return(retval);
}

void powtest(void)
{
int i,j;
for(i=0;i<3;i++)
{
	j=pow(10,i);
	UARTprintf("test pow loop: %d; pow value %d\n",i,j);
}

}
uint32_t WiFiIPGetandConvert(void)
{

	uint32_t ui32IPAddr,j,temp;
	int ip_digit,digit_num,i;
//	double ip_digit;
	char *pIndex;
	char wifi_ip_temp[4];
	ui32IPAddr = 0x12345678;
	if((pIndex = strstr((const char *)ResponseFromWiFi_IP, "IP=")) !=NULL ) // find what we want;
	{
		pIndex = pIndex+3;
		digit_num=0;
//		UARTprintf("\n WiFi IP :");
		for(j=0,digit_num=0;j<4;j++)
		{
			while((*pIndex <= '9')&&(*pIndex >= '0') )
					{
						wifi_ip_temp[digit_num] = *pIndex; // [0] is MSB;
						pIndex++;
						digit_num++;  // for the last ok conditon, pIndex+1,digit_num+1;
					}
					//convert string to value;
				for(i=0,ip_digit=0;i<digit_num;i++) // digit_num should be "3";
				{
					ip_digit = ip_digit+(int)(wifi_ip_temp[i]-'0')*pow(10,digit_num-1-i);
				}
				temp = ui32IPAddr<<8;
				ui32IPAddr = temp + (uint8_t)ip_digit;
				pIndex++; // point to the next,current is not ip char;
				digit_num=0;
		}
//		UARTprintf("WiFi IP : 0x%X !\n",ui32IPAddr);
		return(ui32IPAddr);
	}
	else{
		UARTprintf("WiFi IP not found!\n");
	}
	return(0);
}



uint32_t WiFiGetLocalPort(void)
{

	uint32_t ui32LocalPort=0;
	int length,digit_num,i;
	char *pIndex;
	char wifi_ip_temp[6];
	if((pIndex = strstr((const char *)ResponseFromWiFi_IP, ":")) !=NULL ) // find what we want;
	{
		pIndex = pIndex+1;
		digit_num=0;

			while((*pIndex <= '9')&&(*pIndex >= '0') )
					{
						wifi_ip_temp[digit_num] = *pIndex; // [0] is MSB;
						pIndex++;
						digit_num++;  // for the last ok conditon, pIndex+1,digit_num+1;
					}
			wifi_ip_temp[digit_num] = '\0';
			length = strlen((const char *)wifi_ip_temp);
			for(i=0;i<length;i++)
			{
				ui32LocalPort += (int)(wifi_ip_temp[i]-'0')*pow(10,length-1-i);
			}
			g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort = ui32LocalPort;
			return(ui32LocalPort);
	}
		//UARTprintf("WiFi IP : 0x%X !\n",ui32IPAddr);
	else
	{
		g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort = 0;
		UARTprintf("WiFi IP not found!\n");
	}
	return(0);
}

uint32_t WiFiGetRemoteIP(void)
{

	uint32_t ui32IPAddr,j,temp;
	int ip_digit,digit_num,i;
	char *pIndex;
	char wifi_ip_temp[4];
	if((pIndex = strstr((const char *)ResponseFromWiFi_IP, "HOST=")) !=NULL ) // find what we want;
	{
		pIndex = pIndex+5;
		digit_num=0;
		//UARTprintf("\n WiFi IP :");
		for(j=0;j<4;j++)
		{
			while((*pIndex <= '9')&&(*pIndex >= '0') )
					{
						wifi_ip_temp[digit_num] = *pIndex; // [0] is MSB;
						pIndex++;
						digit_num++;  // for the last ok conditon, pIndex+1,digit_num+1;
					}
					//convert string to value;
				for(i=0,ip_digit=0;i<digit_num;i++) // digit_num should be "3";
				{
					ip_digit = ip_digit+(int)(wifi_ip_temp[i]-'0')*pow(10,digit_num-1-i);

				}

				temp = ui32IPAddr<<8;
				ui32IPAddr = temp + (uint8_t)ip_digit;


				pIndex++; // point to the next,current is not ip char;
				digit_num=0;

		}
		//UARTprintf("WiFi IP : 0x%X !\n",ui32IPAddr);
		g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr = ui32IPAddr;
		return(ui32IPAddr);
	}
	else{
		g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr = 0;
		UARTprintf("Remote IP not found!\n");
	}
	return(0);
}

uint32_t WiFiGetRemotePort(void)
{

	uint32_t ui32RemotePort=0;
	int length,digit_num,i;
	char *pIndex1,*pIndex2;
	char wifi_ip_temp[6];
	if((pIndex1 = strstr((const char *)ResponseFromWiFi_IP, "HOST=")) !=NULL ) // find what we want;
		if((pIndex2 = strstr((const char *)pIndex1, ":")) !=NULL )
	{
		pIndex2 +=1;
		digit_num=0;

		while((*pIndex2 <= '9')&&(*pIndex2 >= '0') )
				{
					wifi_ip_temp[digit_num] = *pIndex2; // [0] is MSB;
					pIndex2++;
					digit_num++;  // for the last ok conditon, pIndex+1,digit_num+1;
				}
		wifi_ip_temp[digit_num] = '\0';
		length = strlen((const char *)wifi_ip_temp);
		for(i=0;i<length;i++)
		{
			ui32RemotePort += (int)(wifi_ip_temp[i]-'0')*pow(10,length-1-i);
		}
		g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort = ui32RemotePort;
		return(ui32RemotePort);
	}
		//UARTprintf("WiFi IP : 0x%X !\n",ui32IPAddr);
	else
	{
		g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort = 0;
		UARTprintf("Remote Port not found!\n");
	}
	return(0);
}
/*
uint8_t RSSI[wifi_scan_qty],Scaned_AP_Num;
char WiFi_SSID_temp[wifi_scan_qty][NET_NAME_LEN];
uint8_t securitymode[wifi_scan_qty]; */

void WiFiScan_SSID_sorting(uint8_t *arr_RSSI,uint8_t n_Scaned_AP_Num,uint8_t *arry_securitymode)
{
	//n为数组长度
	int i,j,temp_RSSI,temp_securitymode;
	char temp_WiFi_SSID_temp[NET_NAME_LEN];
/*
	for(i=0;i<n_Scaned_AP_Num;i++)
	{
		UARTprintf("\nno:%d,%d,%d",i,*(arr_RSSI+i),*(arry_securitymode+i));
		UARTprintf("%s\n",WiFi_SSID_temp[i]);
	}
*/
	for(i=0;i<n_Scaned_AP_Num;i++)
	{
	for(j=0;j<n_Scaned_AP_Num-i-1;j++)
	{
	if(*(arr_RSSI+j)>*(arr_RSSI+j+1))
	{
		temp_RSSI=*(arr_RSSI+j);
		temp_securitymode=*(arry_securitymode+j);
		strcpy((char *)temp_WiFi_SSID_temp,(char *)WiFi_SSID_temp[j]);

	*(arr_RSSI+j)=*(arr_RSSI+j+1);
	*(arry_securitymode+j)=*(arry_securitymode+j+1);
	strcpy((char *)WiFi_SSID_temp[j],(char *)WiFi_SSID_temp[j+1]);

	*(arr_RSSI+j+1)=temp_RSSI;
	*(arry_securitymode+j+1) = temp_securitymode;
	strcpy((char *)WiFi_SSID_temp[j+1],(char *)temp_WiFi_SSID_temp);

	}
	}
	}
//	if(Scaned_AP_Num>SSID_absent_qty)
//	Scaned_AP_Num = Scaned_AP_Num - SSID_absent_qty;
/*
	for(i=0;i<n_Scaned_AP_Num;i++)
	{
		UARTprintf("\nno:%d,%d,%d",i,*(arr_RSSI+i),*(arry_securitymode+i));
		UARTprintf("%s\n",WiFi_SSID_temp[i]);
	}
*/

}

//need to initialize array;
void WiFiSCAN_data_array_INI(void)
{
//	uint8_t RSSI[wifi_scan_qty],Scaned_AP_Num;
//	char WiFi_SSID_temp[wifi_scan_qty][NET_NAME_LEN];
//	uint8_t securitymode[wifi_scan_qty];
	uint8_t i;
	Scaned_AP_Num = 0;
	for(i=0;i<wifi_scan_qty;i++)
	{
		WiFi_SSID_temp[i][0] = 0;
		securitymode[i] = 0;
		RSSI[i] = 0;
	}

}

uint8_t SSID_absent_qty = 0;
void WiFiSCAN_get_SSID_SecurityData(void)
{
	uint8_t digit_num,i,j,k,RSSI_digit_num,RSSI_digit,digit_num_gewei,i_valid;
	char *pIndex,*p;
	uint8_t RSSI_temp[4];
	RSSI_digit_num =0;
	SSID_absent_qty =0;
	WiFiSCAN_data_array_INI();
	//1, find the number of AP;  should be less than 13; scan time: 200ms/channel;
	//2, loop to find each SSID and security value;
//	UARTprintf("The string to be scanned:%s!\n",ResponseFromWiFi_SCAN);
//	strstr(str1,str2) 函数用于判断字符串str2是否是str1的子串。
//	如果是，则该函数返回str2在str1中首次出现的地址；否则，返回NULL。
	if((pIndex = strstr((char *)ResponseFromWiFi_SCAN, "Found")) !=NULL ) // find what we want; //scan output format: 13 + 65 +65 ++65;
	{
		UARTprintf("WiFi AP Finded!\n");
		pIndex +=6;
		// find the number and convert to digit; only one time;
		//if over 10 access points, should consider 2 digits;
		digit_num=0;
		digit_num_gewei =0;
		Scaned_AP_Num =0;
		for(k=0;k<2;k++)
		{
			if((*pIndex <= '9')&&(*pIndex >= '0'))
			{
				if(k==0)
				{
					digit_num = *pIndex - '0';
					Scaned_AP_Num = digit_num;
//					UARTprintf(" 1:%c\n",*pIndex);
					pIndex++;
//					UARTprintf(" 2:%c\n",*pIndex);
				}
				else if(k==1)
				{
					digit_num_gewei = *pIndex - '0';
					Scaned_AP_Num = Scaned_AP_Num * 10 + digit_num_gewei;
					pIndex++;
				}
			}
		}

		if(Scaned_AP_Num>0)
		{
			UARTprintf("\n Scaned WiFI AP Quantities:%d\n",Scaned_AP_Num);
		}
		else {

			UARTprintf("WiFi Scan AP number is not found!\n");
		}

//		UARTprintf(" 31:%d%d%d%d%d%d%d%d%d\n",pIndex[0],pIndex[1],pIndex[2],pIndex[3],pIndex[4],pIndex[5],pIndex[6],pIndex[7],pIndex[8]);
//		UARTprintf(" 31:%c%c%c%c%c%c%c%c%c\n",pIndex[0],pIndex[1],pIndex[2],pIndex[3],pIndex[4],pIndex[5],pIndex[6],pIndex[7],pIndex[8]);
		pIndex +=2;
		if(Scaned_AP_Num>wifi_scan_qty)
		{
//			Scaned_AP_Num = wifi_scan_qty;
			digit_num = wifi_scan_qty;
		}

		else digit_num = Scaned_AP_Num;
		for(j=0,i_valid=0;j<digit_num;j++)
		{
			// find RSSI data;
				pIndex +=7;
//				UARTprintf("print RSSI char p[0]:%c p[1]:%c p[2]:%c p[3]:%c p[4]:%c p[5]:%c!\n",pIndex[0],pIndex[1],pIndex[2],pIndex[3],pIndex[4],pIndex[5]);
				p = pIndex;
					while((*p <= '9')&&(*p >= '0') )
							{
								RSSI_temp[RSSI_digit_num] = *p; // [0] is MSB;
								p++;
								RSSI_digit_num++;  // for the last ok conditon, pIndex+1,digit_num+1;
							}
							//convert string to value;
						for(i=0,RSSI_digit=0;i<RSSI_digit_num;i++) // digit_num should be "3";
						{
							RSSI_digit = RSSI_digit+(int)(RSSI_temp[i]-'0')*pow(10,RSSI_digit_num-1-i);
						}
						RSSI[i_valid] = RSSI_digit;
//						UARTprintf("RSSI %d:%d\n",j,RSSI[j]);
						RSSI_digit_num=0;

			// find security type and ssid; // RSSI can be 1 digit, or 2 digits; so,
						if((RSSI_digit>0)&&(RSSI_digit<=9))
							pIndex +=3;
						else if(RSSI[i_valid]>=10)
							pIndex +=4;
//				UARTprintf("security char:%c\n",*pIndex);
				if((*pIndex <= '9')&&(*pIndex >= '0')) //secrity mode from 0 to 9; only one byte is ok;
				{
					securitymode[i_valid] = *pIndex - '0';
				}
				else {

			//		UARTprintf("WiFi Scan security mode is not found!\n");
				}
			// find ssid;
				pIndex +=30;
				if(*pIndex == ',')
				{
					// get ssid string;
					pIndex++;

					// if number, not right, if ='\r\n' 13 CR, carriage return'\r', 10 LF line feed'\n', 1310;
//					if(*pIndex == '\r')&&(*(pIndex+1) == '\n')
					if((*pIndex == 13)&&(*(pIndex+1) == 10))
					{
						UARTprintf("SSID absent\n");
						SSID_absent_qty++;
						WiFi_SSID_temp[i_valid][0] = '\0';
						pIndex+=2;
//						UARTprintf(" 22:%c%c%c%c\n",pIndex[0],pIndex[1],pIndex[2],pIndex[3]);

					}
					else // have valid ssid string; //21 char;NET_NAME_LEN
					{
						i = 0;
						p = WiFi_SSID_temp[i_valid];
						while((*pIndex != '\0')&&(*pIndex != ',')&&(*pIndex != ':'))
//						while((*pIndex != 13)&&(*(pIndex+1) != 10))
						{
							if(i < NET_NAME_LEN)
							p[i] = *pIndex;

							i++;
							pIndex++;
						}
						//if i+1 > NET_NAME_LEN, ignore the remaining;
						if(i+1 > NET_NAME_LEN)
						{
							p[NET_NAME_LEN-1] = '\0';
						}

						// get one ssid;
						if(j != Scaned_AP_Num-1) // not the last one;
						{
							p[i-4] = '\0'; // 4 is ok;
							pIndex = pIndex-2;
//							UARTprintf("SSID not last one, p[0]:%c p[1]:%c p[2]:%c p[3]:%c p[4]:%c p[5]:%c\n",pIndex[0-3],pIndex[1-3],pIndex[2-3],pIndex[3-3],pIndex[4-3],pIndex[5-3]);

						}
						else if(j == Scaned_AP_Num-1) // only one or the last one;
						{
							p[i-5] = '\0';
							pIndex = pIndex-3;
//							UARTprintf("SSID last one, p[0]:%c p[1]:%c p[2]:%c p[3]:%c p[4]:%c p[5]:%c\n",pIndex[0-3],pIndex[1-3],pIndex[2-3],pIndex[3-3],pIndex[4-3],pIndex[5-3]);

						}
						i_valid++;
					}



				}
				// print ssid;
//				UARTprintf("WiFi Scan SSID %d:%s\n",j,WiFi_SSID_temp[j]);
		}
		Scaned_AP_Num = i_valid;
	}
	else{
		UARTprintf("WiFi Found string is not found!\n");
	}

	// sorting array;
	WiFiScan_SSID_sorting(RSSI,Scaned_AP_Num,securitymode);

}
void send_reboot_cmd(void)
{
	int retval;
	 if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("reboot to enter CMD mode! \n");
	}	// 4.set STA mode ap's ssid:						"set wlan ssid %s";
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan ssid %s",g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName )))
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
		//8.get ip address: "reboot"; //"IP="
					else if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot"))) // get the IP address, and send to queue, and feedback to web page;
				{
	              //  RN_RED_LED = 1;
					UARTprintf("Fail to reboot! \n");
					wifi_cmd_err_exit();
				}
					else UARTprintf("reboot ok \n");
		//11.exit command;
	 /*
					else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
				{
				// RN_RED_LED = 1;
					UARTprintf("Fail to exit reboot mode! \n");
//					wifi_cmd_err_exit();
				}*/
//	 UARTprintf("rev:%s\n",ResponseFromWiFi_IP);
}
void send_join_cmd(void)
{
	int retval;
	 if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
		//8.get ip address: "join"; //"IP="
					else if(RN_NOT_ERR_OK(retval, module_send_cmd("SCAN OK", "join"))) // get the IP address, and send to queue, and feedback to web page;
				{
	              //  RN_RED_LED = 1;
					UARTprintf("Fail to join! \n");
					wifi_cmd_err_exit();
				}
		//11.exit command;
					else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
				{
				// RN_RED_LED = 1;
					UARTprintf("Fail to exit join mode! \n");
				}
//	 UARTprintf("rev:%s\n",ResponseFromWiFi_IP);
}

void send_get_ip_cmd(void)
{
	int retval;
	 if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("get ip Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
		//8.get ip address: "get ip"; //"IP="
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
					UARTprintf("Fail to exit get ip mode! \n");
				}
//	 UARTprintf("rev:%s\n",ResponseFromWiFi_IP);
}

int  WiFiGetIP_func(void)
{
	int retval;

    uint8_t timeout_number = 0;
	while(((strstr((const char *)ResponseFromWiFi_IP, "IF=UP")) ==NULL )&&(timeout_number < retry_time_for_IP_and_Join_CMD))
	{
		//delay some time;1
		vTaskDelay( 1000/portTICK_RATE_MS ); //from 100 to 500;
		send_get_ip_cmd();
		timeout_number++;
	}
	if(timeout_number == retry_time_for_IP_and_Join_CMD)
	{
		retval = retry_time_for_IP_and_Join_CMD;
		UARTprintf("retry 1 \n");
	}
	else {
			retval = 0;
			UARTprintf("IF UP \n");

		}

	return(retval);
}
/*
void reset_ssidName(void)
{
	int retval;
	 if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
	// 4.set STA mode ap's ssid:						"set wlan ssid %s";
			else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan ssid %s",g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName )))
			{
	                    //RN_RED_LED = 1;
				UARTprintf("Fail to set STA mode ap's ssid! \n");
				wifi_cmd_err_exit();
			}
}
*/
int WiFiGetIP_SysRST(void)
{
	int retval;
	tWiFiCMD pxMessage;
	uint8_t timeout_number = 0;
	send_reboot_cmd();
	vTaskDelay(1000/portTICK_RATE_MS );

	if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == STA_Mode)
	 WiFiSTAJoin();
	send_get_ip_cmd();
	retval= WiFiGetIP_func();
	while ((retval!= RN_ERR_OK)&&(timeout_number < retry_time_for_IP_and_Join_CMD))  //if !=0, not ok, send join cmd again;
	{
		send_reboot_cmd();
		vTaskDelay(1000/portTICK_RATE_MS );
		//send join cmd again;
//		send_join_cmd();
		// send get ip cmd agin;
//		 ExitCMD();
//		 STAMode_DHCP_Mode_Set();
		if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == STA_Mode)
		 WiFiSTAJoin();
		send_get_ip_cmd();
		retval= WiFiGetIP_func();
		timeout_number++;
	}

						WiFiDynamicIP = WiFiIPGetandConvert();

		return retval;
}

int WiFiGetIP(void)
{
	int retval;
	tWiFiCMD pxMessage;
	uint8_t timeout_number = 0;
	send_get_ip_cmd();
	retval= WiFiGetIP_func();
	while ((retval!= RN_ERR_OK)&&(timeout_number < retry_time_for_IP_and_Join_CMD))  //if !=0, not ok, send join cmd again;
	{
		send_reboot_cmd();
		vTaskDelay(1000/portTICK_RATE_MS );
		//send join cmd again;
//		send_join_cmd();
		// send get ip cmd agin;
//		 ExitCMD();
//		 STAMode_DHCP_Mode_Set();
		if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == STA_Mode)
		 WiFiSTAJoin();
		send_get_ip_cmd();
		retval= WiFiGetIP_func();
		timeout_number++;
	}
	//if timeout_number ==3, please check passwords; how to let user see this, TBD!

//						UARTprintf("Succed to handle STA static ip mode page function! \n");
						// get mac;
//						GetWiFiMACAddress();
						WiFiDynamicIP = WiFiIPGetandConvert();

								// get ip address from array;
								//g_QueWiFiDataBack
								pxMessage.ui8Name = GetWiFi_IPAdress;
								xQueueSendToBack( g_QueWiFi_IPDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;

		return retval;
}

int WiFiGetMAC(void)
{
	int retval;
	tWiFiCMD pxMessage;
	 if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("get mac Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
		//8.get ip address: "get mac"; //"IP="
					else if(RN_NOT_ERR_OK(retval, module_send_cmd("Mac Addr=", "get mac"))) // get the IP address, and send to queue, and feedback to web page;
				{
	              //  RN_RED_LED = 1;
					UARTprintf("Fail to get mac address! \n");
					wifi_cmd_err_exit();
				}
		//11.exit command;
					else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
				{
				// RN_RED_LED = 1;
					UARTprintf("Fail to exit get mac mode! \n");
					wifi_cmd_err_exit();
				}

					else{
//						UARTprintf("Succed to handle STA static ip mode page function! \n");
						// get mac;
//						GetWiFiMACAddress();
						GetWiFiMACAddress();
					}
								// get ip address from array;
								//g_QueWiFiDataBack
								pxMessage.ui8Name = GetWiFiMAC;
								xQueueSendToBack( g_QueWiFiDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;

		return retval;
}

int WiFiGet_DataTrans_State(void)
{
	int retval;
	tWiFiCMD pxMessage;
	 if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
		//8.get ip address: "get ip"; //"IP="
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
					UARTprintf("Fail to exit STA static ip mode! \n");

				}

				else{
						g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr = WiFiGetRemoteIP();
						g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort = WiFiGetLocalPort();
						g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort = WiFiGetRemotePort();
					}
								// get ip address from array;

								pxMessage.ui8Name = WiFi_DataTrans_State;
								xQueueSendToBack( g_QueWiFiDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;

		return retval;
}
int STAMode_Scan(void)
{
	// cd command mod:									$$$
	// 1. search network access points:					"scan";
	// exit command:  									"exit";
	int retval;
	tWiFiCMD pxMessage;

	if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
	 // 0, sucess to enter cmd mode;
	// 1. search network access points:					"scan";
	else if(RN_NOT_ERR_OK(retval, module_send_cmd("Found", "scan")))
	{
		                    //RN_RED_LED = 1;
		UARTprintf("Fail to search network access points! \n");
		wifi_cmd_err_exit();
	}
	//11.exit command;
	else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
			{
			// RN_RED_LED = 1;
				UARTprintf("Fail to exit STA static ip mode! \n");
				wifi_cmd_err_exit();
			}
	else {
		UARTprintf("precess scan data!\n");
		// receive and get ssid and security mode data;
		WiFiSCAN_get_SSID_SecurityData();
	}
	//g_QueWiFiDataBack
	pxMessage.ui8Name = WiFiSCAN;
	xQueueSendToBack( g_QueWiFiScan_DataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;

	if(retval !=0)
	{
			// RN_RED_LED = 1;
		UARTprintf("Fail to exit scan! \n");
	}
	else
	{
		UARTprintf("Succed; \n");
	}
return retval;
}

void WiFiSTAJoin(void)
{
	int retval;
	uint8_t timeout_number = 0;
	 if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}/*
	else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan join 0"))) //exit ap mode:
	{
	                 //   RN_RED_LED = 1;
		UARTprintf("Fail to exit ap mode! \n");
	}*/
	 else if(RN_NOT_ERR_OK(retval, module_send_cmd("Associ", "join")))  // return 0, means ok, 1, menas fail;
	{
               // RN_RED_LED = 1;
//		retval = RN_ERR_CMDFault;
		UARTprintf("Fail to join wirless net,try again! \n");
		/*
		retval = RN_NOT_ERR_OK(retval, module_send_cmd("Associated", "join"));
		while((retval)&& (timeout_number < retry_time_for_IP_and_Join_CMD))
		{
			vTaskDelay( 200/portTICK_RATE_MS );
			retval = RN_NOT_ERR_OK(retval, module_send_cmd("Associated", "join"));

			timeout_number++;
		}
*/
		if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
			{
				// RN_RED_LED = 1;
				UARTprintf("Fail to exit CMD  mode! \n");
				module_send_cmd("EXIT", "exit");
			}

	}
		//11.exit command;
	else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
	{
		// RN_RED_LED = 1;
		UARTprintf("Fail to exit CMD  mode! \n");
		// send exit command again;
		 module_send_cmd("EXIT", "exit");
	}
}

int STAMode_DHCP_Mode_Set(void) // Station, Dynamic ip mode;
{
	// cd command mod:									$$$
	// 1. exit ap mode:								"set wlan join 0"
	// 2. trun on Station's dhcp :						"set ip dhcp 1" ;0 off; 1 on;
				// 3. search network access points:					"scan";
	// 4.set STA mode ap's ssid:						"set wlan ssid %s";
	// 5.set STA mode ap's pass:						"set wlan phrase %s";
	// 6.save command:									"save";
	// 7.get ip address: 								"get ip";
	// 8.get mac address: 								"get mac";
	// exit command:  									"exit";
	int retval;
	//uint32_t	wifiCMD_stack_remaining_join;
	tWiFiCMD pxMessage;
//	retval = STAMode_Scan();
//	if(!retval)
//	{
//		UARTprintf("Fail to scan access points! \n");
//	}

	 if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
	 // 0, sucess to enter cmd mode;
		//1. exit ap mode:"set wlan join 0"
//	else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan join 1"))) //exit ap mode:
	else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w j 0"))) //exit ap mode:
		{
                 //   RN_RED_LED = 1;
			UARTprintf("Fail to exit ap mode! \n");
			wifi_cmd_err_exit();
		}
		// 2. trun on Station's dhcp :						"set ip dhcp 1" ;0 off; 1 on;
//	else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip dhcp 1")))  // trun on Station's dhcp;
	else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set i d 1")))  // trun on Station's dhcp;
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to trun on Station's dhcp! \n");
					wifi_cmd_err_exit();
				}
		// 4.set STA mode ap's ssid:						"set wlan ssid %s";
//				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan ssid %s",g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName )))
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w s %s",g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set STA mode ap's ssid! \n");
					wifi_cmd_err_exit();
				}
		// 5.set STA mode ap's pass:						"set wlan phrase %s";    			  v
//				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan phrase %s",g_sParameters.sWiFiModule.sWiFiStaionSet.ui8InputPasswords )))
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w p %s",g_sParameters.sWiFiModule.sWiFiStaionSet.ui8InputPasswords )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set STA mode ap's pass! \n");
					wifi_cmd_err_exit();
				}

/*
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Associated", "join")))
				{
		                   // RN_RED_LED = 1;
					retval = RN_ERR_CMDFault;
					UARTprintf("Fail to jion wirless net! \n");
				} */

/*
		//10.get mac address: "get mac";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Mac Addr=", "get mac"))) // get the MAC address, and send to queue, and feedback to web page;
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to save ssid! \n");
				}
	 */
		//9.save command;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
				{
               //  RN_RED_LED = 1;
					UARTprintf("Fail to save ssid! \n");
					wifi_cmd_err_exit();
				}
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Reboot fail! \n");
					wifi_cmd_err_exit();
				}
	 /*
	//8.get ip address: "get ip";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("IP=", "get ip"))) // get the IP address, and send to queue, and feedback to web page;
			{
              //  RN_RED_LED = 1;
				UARTprintf("Fail to get IP address! \n");
			}
	 */
	//11.exit command;
			/*	else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
			{
			// RN_RED_LED = 1;
				UARTprintf("Fail to exit STA static ip mode! \n");
			} */

				else{
					vTaskDelay( 100/portTICK_RATE_MS );
//					  wifiCMD_stack_remaining_join = uxTaskGetStackHighWaterMark(NULL);
					  //UARTprintf("\nwifiCMD_stack_remaining_STA_DHCP_join_before : %d \n",wifiCMD_stack_remaining_join );
					WiFiSTAJoin();
					//UARTprintf("Succed to handle STA DHCP mode page function! \n");
					/*
					// get mac;
					if(!Wifly_Acc)
					GetWiFiMACAddress();
					*/
//					WiFiDynamicIP = WiFiIPGetandConvert();
				}
							// get ip address from array;
							//g_QueWiFiDataBack
							pxMessage.ui8Name = STAMode_DHCP_Mode_Setting;
							xQueueSendToBack( g_QueWiFi_STA_DataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;

	return retval;

}

int STAMode_Non_DHCP_Mode_Set(void) // Station, Static ip mode;
{
	// cd command mod:									$$$
	// 1. exit ap mode:								"set wlan join 0"
	// 2. trun on Station's dhcp :						"set ip dhcp 0" ;0 off; 1 on;
		// 3. search network access points:					"scan";
	// 4.set STA mode ap's ssid:						"set wlan ssid %s";
	// 5.set STA mode ap's pass:						"set wlan phrase %s";
	// 6.set ip address:								"set ip address %d";
	// 7.set sm address: 								"set ip net %d";
	// 8.set gateway address:							"set ip gateway %d";
	// 9.save command:									"save";
	// 10.get mac address: 								"get mac";
	// exit command:  									"exit";

	int retval;
	tWiFiCMD pxMessage;
//	retval = STAMode_Scan();
//	if(!retval)
//	{
//		UARTprintf("Fail to scan access points! \n");
//	}

	if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
	 // 0, sucess to enter cmd mode;
			//1. exit ap mode:"set wlan join 0"
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan join 0"))) //exit ap mode:
		{
                 //   RN_RED_LED = 1;
			UARTprintf("Fail to exit ap mode! \n");
			wifi_cmd_err_exit();
		}
		// 2. trun off Station's dhcp :						"set ip dhcp 0" ;0 off; 1 on;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip dhcp 0")))  // trun off Station's dhcp;
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to trun off Station's dhcp! \n");
					wifi_cmd_err_exit();
				}

		// 4.set STA mode ap's ssid:						"set wlan ssid %s";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan ssid %s",g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set STA mode ap's ssid! \n");
					wifi_cmd_err_exit();
				}
		// 5.set STA mode ap's pass:						"set wlan phrase %s";    			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan phrase %s",g_sParameters.sWiFiModule.sWiFiStaionSet.ui8InputPasswords )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set STA mode ap's pass! \n");
					wifi_cmd_err_exit();
				}
		// 6.set ip address:								"set ip address %s";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip address %s", ipconvtosring(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticIP)))) //
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless STA static ip address! \n");
					wifi_cmd_err_exit();
				}
		// 7.set sm address: 								"set ip net %s";       					    v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip net %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiStaionSet.ui32STASubnetMask) ))) //change, add;
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless STA subnet address! \n");
					wifi_cmd_err_exit();
				}
		// 8.set gateway address:							"set ip gateway %s";     		    v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip gateway %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticGateway) ))) //? add gateway address in database;
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless STA gateway address! \n");
					wifi_cmd_err_exit();
				}
/*
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "join")))
				{
		                   // RN_RED_LED = 1;
					retval = RN_ERR_CMDFault;
					UARTprintf("Fail to jion wirless net! \n");
				}
	*/
		//9.save command;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to save ssid! \n");
					wifi_cmd_err_exit();
				}
	/*
		//10.get mac address: "get mac";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Mac Addr=", "get mac"))) // get the MAC address, and send to queue, and feedback to web page;
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to get mac! \n");
				}*/
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Reboot fail! \n");
				}
	/*
	//11.exit command;
			else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
			{
			// RN_RED_LED = 1;
				UARTprintf("Fail to exit STA static ip mode! \n");
			}
	*/
			else
			{
				vTaskDelay( 100/portTICK_RATE_MS );
				WiFiSTAJoin();
				/*
				// get mac;
				if(!Wifly_Acc)
				GetWiFiMACAddress(); // 0, get mac ok;
				*/
				UARTprintf("Succed STA static ip func! \n");
			}
pxMessage.ui8Name = STAMode_Non_DHCP_Mode_Setting;
xQueueSendToBack( g_QueWiFi_STA_DataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;
	return retval;
}


int APMode_Disable_DHCP_Server_Set(void) // Disable DHCP server for AP mode;

{
	tWiFiCMD pxMessage;

	// cd command mod:									$$$
	// 1. enable ap mode:								"set wlan join 7"

	// 2.set frequency:									"set w c 1";
	// 3.set ap's ssid:									"set apmode ssid %s";
	// 4.set security mode:								"set apmode passphrase %s";
	// 5. enable AP's dhcp server:						"set ip dhcp 0"


	//6.set ip address:"set ip address %s";
	//7.set ap's subnet:"set ip net %s";
	//8.set ap's gateway:"set ip gateway %s";

							//  ip/gw/subnet;
	// 9.save command:									"save";
	// 10.	reboot
	// 11.get mac address: 								"get mac";
	// 12.get ip address: 								"get ip";
	// 13.exit command:  									"exit";

	int retval;
	if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
	 // 0, sucess to enter cmd mode;


		// 1. enable ap mode:"set wlan join 7"
	else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan join 7")))
		{
                 //   RN_RED_LED = 1;
			UARTprintf("Fail to ap mode! \n");
			wifi_cmd_err_exit();
		}
		// 3.set frequency:"set w c %d";  need to use paras;    											 v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w c %d",g_sParameters.sWiFiModule.sWiFiAPSet.ucFrequency_Channel )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless frequency! \n");
					wifi_cmd_err_exit();
				}
		//5.set ap's ssid:"set apmode ssid %s"; need to use one para; ssid network name;           			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set apmode ssid %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's SSID! \n");
					wifi_cmd_err_exit();
				}
		//4.set security mode:"set apmode passphrase %s"; need to use one para; wep-128 type; security; 	 v
				//This command sets the soft AP mode passphrase to be used for WPA2-AES encryp-
				//tion. When set, the module will broadcast a network in soft AP mode with WAP2-AES
				//encryption enabled. The allowable length of <string> is between 8 and 64 characters;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set apmode passphrase %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords )))// need check;g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless security type! \n");
					wifi_cmd_err_exit();
				}

		// 2. disable AP's dhcp server:"set ip dhcp 0"
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip dhcp 0")))
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to set wireless DHCP mode! \n");
					wifi_cmd_err_exit();
				}

		//5.set ip address:"set ip address %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip address %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's IP address! \n");
					wifi_cmd_err_exit();
				}

		//5.set ap's ssid:"set ip net %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip net %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's IP address! \n");
					wifi_cmd_err_exit();
				}
		//5.set ap's gateway:"set ip gateway %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip gateway %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's gateway address! \n");
					wifi_cmd_err_exit();
				}

		//6.save command;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to save ssid! \n");
					wifi_cmd_err_exit();
				}
/*
		//7.get mac address: "get mac";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Mac Addr=", "get mac"))) // get the MAC address, and send to queue, and feedback to web page;
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to get MAC address! \n");
				}
	*/
/*
		//8.get ip address: "get ip";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("IP=", "get ip"))) // get the IP address, and send to queue, and feedback to web page;
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to get IP address! \n");
				}
	*/
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Reboot fail! \n");
					wifi_cmd_err_exit();
				}
				else{
					// get ip address from array;
//					WiFiDynamicIP = WiFiIPGetandConvert();
					/*
					// get mac address from array;
					if(!Wifly_Acc)
					GetWiFiMACAddress();
					*/
					UARTprintf("Succed to set AP none DHCP mode! \n");
				}

				pxMessage.ui8Name = APMode_Non_DHCP_Mode_Setting;

				xQueueSendToBack( g_QueWiFi_AP_NoDHCP_DataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;
				return retval;
}

int APMode_Enable_DHCP_Server_Set(void) // // Enable DHCP server for AP mode;
// Dynamic ip mode;
{
	tWiFiCMD pxMessage;

	// cd command mod:									$$$
	// 1. enable ap mode:								"set wlan join 7"
	// 2. enable AP's dhcp server:						"set ip dhcp 4"
	// 3.set frequency:									"set w c %d";
	// 4.set security mode:								"set apmode passphrase %s";
	// 5.set ap's ssid:									"set apmode ssid %s";
							// no ip/gw/subnet;
	// 6.save command:									"save";
	// 7.get mac address: 								"get mac";
	// 8.get ip address: 								"get ip";
	// exit command:  									"exit";

	int retval;
	if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
	 // 0, sucess to enter cmd mode;


		// 1. enable ap mode:"set wlan join 7"
	else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan join 7")))
		{
                 //   RN_RED_LED = 1;
			UARTprintf("Fail to ap mode! \n");
			wifi_cmd_err_exit();
		}
		// 3.set frequency:"set w c %d";  need to use paras;    											 v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w c %d",g_sParameters.sWiFiModule.sWiFiAPSet.ucFrequency_Channel )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless frequency! \n");
					wifi_cmd_err_exit();
				}
		//5.set ap's ssid:"set apmode ssid %s"; need to use one para; ssid network name;           			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set a ssid %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's SSID! \n");
					wifi_cmd_err_exit();
				}
		//4.set security mode:"set apmode passphrase %s"; need to use one para; wep-128 type; security; 	 v
				//This command sets the soft AP mode passphrase to be used for WPA2-AES encryp-
				//tion. When set, the module will broadcast a network in soft AP mode with WAP2-AES
				//encryption enabled. The allowable length of <string> is between 8 and 64 characters;
		//		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set apmode passphrase %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords )))// need check;g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set a p %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords )))// need check;g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless security type! \n");
					wifi_cmd_err_exit();
				}

		// 2. enable AP's dhcp server:"set ip dhcp 4"
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip dhcp 4")))
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to set wireless DHCP mode! \n");
					wifi_cmd_err_exit();
				}

		//5.set ip address:"set ip address %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip address %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's IP address! \n");
					wifi_cmd_err_exit();
				}

		//5.set ap's ssid:"set ip net %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip net %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's IP address! \n");
					wifi_cmd_err_exit();
				}
		//5.set ap's gateway:"set ip gateway %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip gateway %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's gateway address! \n");
					wifi_cmd_err_exit();
				}

		//6.save command;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to save ssid! \n");
					wifi_cmd_err_exit();
				}
/*
		//7.get mac address: "get mac";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Mac Addr=", "get mac"))) // get the MAC address, and send to queue, and feedback to web page;
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to get MAC address! \n");
				}*/
/*
		//8.get ip address: "get ip";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("IP=", "get ip"))) // get the IP address, and send to queue, and feedback to web page;
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to get IP address! \n");
				}
	*/
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Reboot fail! \n");
					wifi_cmd_err_exit();
				}
				else{
					// get ip address from array;
//					WiFiDynamicIP = WiFiIPGetandConvert();
					/*
					// get mac address from array;
					if(!Wifly_Acc)
					GetWiFiMACAddress();
					*/
					UARTprintf("Succed to set AP DHCP mode! \n");
				}

				pxMessage.ui8Name = APMode_DHCP_Mode_Setting;

				xQueueSendToBack( g_QueWiFi_APDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;
				return retval;
}

void WiFiModulereboot(void)
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
	UARTprintf("prepare to reboot:\n");
	SerialSend(WiFi_Port, 'r');
	SerialSend(WiFi_Port, 'e');
	SerialSend(WiFi_Port, 'b');
	SerialSend(WiFi_Port, 'o');
	SerialSend(WiFi_Port, 'o');
	SerialSend(WiFi_Port, 't');
	SerialSend(WiFi_Port, '\r');
	SerialSend(WiFi_Port, '\n');
	vTaskDelay( 100/portTICK_RATE_MS );
//	UARTprintf("\n response from wifi :" );
	while(SerialReceiveAvailable(WiFi_Port)) //when wifi rx ringbuffer has data, receive it;
	   {
		 	 ResponseFromWiFi[index] = (uint8_t)SerialReceive(WiFi_Port);
		// 	 UARTprintf("%c",ResponseFromWiFi[index] );
		 	 index++;
	   }
//	SerialSend(WiFi_Port, '/r');	 // send enter plus return '/n';
//	UARTprintf("\n" );
//	vTaskDelay( 1000/portTICK_RATE_MS );
}
/*
int WiFi_STA_Network_Connect()
{
	int retval;
	tWiFiCMD pxMessage;
	if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		retval = RN_ERR_CMDFault;
		UARTprintf("Fail to enter CMD mode! \n");
	}
	 // 0, sucess to enter cmd mode;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w s %s", NETWORK_SSID)))
		{
                 //   RN_RED_LED = 1;
			retval = RN_ERR_CMDFault;
			UARTprintf("Fail to set wirless SSID! \n");
		}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w p %s", NETWORK_PASS)))
		{
                    //RN_RED_LED = 1;
			retval = RN_ERR_CMDFault;
			UARTprintf("Fail to set wirless passwords! \n");
		}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
		{
                  //  RN_RED_LED = 1;
			retval = RN_ERR_CMDFault;
			UARTprintf("Fail to save ssid! \n");
		}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd("Joining", "join")))
		{
                   // RN_RED_LED = 1;
			retval = RN_ERR_CMDFault;
			UARTprintf("Fail to jion wirless net! \n");
		}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
		{
		                   // RN_RED_LED = 1;
			retval = RN_ERR_CMDFault;
			UARTprintf("Fail to exit station mode! \n");
		}
		else
		{
			retval = RN_ERR_OK;
			UARTprintf("Succed to jion wireless net! \n");
		}

	pxMessage.ui8Name = WiFiSTA_Net_Connection;
	pxMessage.CMDExecute_status = retval;
	xQueueSendToBack( g_QueWiFiDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;
	return retval;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
int WiFi_AP_Network_Connect(void)
// Dynamic ip mode;
{
	tWiFiCMD pxMessage;

	// cd command mod:									$$$
	// 1. enable ap mode:								"set wlan join 7"
	// 2. enable AP's dhcp server:						"set ip dhcp 4"
	// 3.set frequency:									"set w c %d";
	// 4.set security mode:								"set apmode passphrase %s";
	// 5.set ap's ssid:									"set apmode ssid %s";
							// no ip/gw/subnet;
	// 6.save command:									"save";
	// 7.get mac address: 								"get mac";
	// 8.get ip address: 								"get ip";
	// exit command:  									"exit";


	int retval;
	if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		WiFi_Status_Retval = retval;
		UARTprintf("Fail to enter CMD mode! \n");
	}
	 // 0, sucess to enter cmd mode;

		// 1. enable ap mode:"set wlan join 7"
	else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan join 7")))
		{
                 //   RN_RED_LED = 1;
			UARTprintf("Fail to ap mode! \n");
		}
		// 2. enable AP's dhcp server:"set ip dhcp 4"
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip dhcp 4")))
			{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to set wireless DHCP mode! \n");
				}
		// 3.set frequency:"set w c %d";  need to use paras;    											 v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set w c %d",g_sParameters.sWiFiModule.sWiFiAPSet.ucFrequency_Channel )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless frequency! \n");
				}
		//4.set security mode:"set apmode passphrase %s"; need to use one para; wep-128 type; security; 	 v
				//This command sets the soft AP mode passphrase to be used for WPA2-AES encryp-
				//tion. When set, the module will broadcast a network in soft AP mode with WAP2-AES
				//encryption enabled. The allowable length of <string> is between 8 and 64 characters;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set apmode passphrase %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords )))// need check;g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless security type! \n");
				}
		//5.set ap's ssid:"set apmode ssid %s"; need to use one para; ssid network name;           			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set apmode ssid %s",g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's SSID! \n");
				}
		//5.set ap's ssid:"set ip address %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip address %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's IP address! \n");
				}

		//5.set ap's ssid:"set ip net %s";          			  v
				else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip net %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask) )))
				{
		                    //RN_RED_LED = 1;
					UARTprintf("Fail to set wirless AP's IP address! \n");
				}


		//6.save command;
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to save ssid! \n");
				}

		//7.get mac address: "get mac";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Mac Addr=", "get mac"))) // get the MAC address, and send to queue, and feedback to web page;
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to get MAC address! \n");
				}

		//8.get ip address: "get ip";
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("IP=", "get ip"))) // get the IP address, and send to queue, and feedback to web page;
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Fail to get IP address! \n");
				}
				else if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
				{
                  //  RN_RED_LED = 1;
					UARTprintf("Reboot fail! \n");
				}

				else{
					// get mac address from array;
					GetWiFiMACAddress();
					// get ip address from array;
					WiFiDynamicIP = WiFiIPGetandConvert();

					retval = RN_ERR_OK;
					UARTprintf("Succed to set AP DHCP mode! \n");
					WiFi_Status_Retval = RN_ERR_OK;
				}
	pxMessage.ui8Name = APMode_DHCP_Mode_Setting;
	xQueueSendToBack( g_QueWiFiDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;
	UARTprintf("APModeDHCP_Mode_Setting Command End!\n");
	UARTprintf("Tell CGI that WiFi IP address is already in response array now! \n");
	return retval;
}
*/
/**************************************************************************************************/

int module_cmd_mode()
{
	int retval;
//	int i;
//	char *p;
    appendCRLF = 0;
//	#define RN_NOT_ERR_OK(_retval, _function) ((_retval = _function) != RN_ERR_OK); RN_ERR_OK = 0
//  if retval == 0, (0 != 0) is false, so RN_NOT_ERR_OK() is false, execute ok;
//  if retval == 1, (1 != 0) is true,  so print error information;
	if(RN_NOT_ERR_OK(retval, module_send_cmd("CMD", "$$$"))) // if can't enter cmd mode, retry 5 times;
	{
		UARTprintf("\nCan't enter cmd mode, retry 0 times; \n");
		wifi_cmd_err_exit();
		/*
		for(i = 0; i < MODULE_ENTER_CMD_MODE_RETRIES; ++i)
		{
			if(!RN_NOT_ERR_OK(retval, module_send_cmd("$$$", "$$$"))) // if ok,execute;
			{
				retval = WiFi_EnterCMD_Error;
				break;
			}

                       // __delay_ms(25);
			vTaskDelay( 25/portTICK_RATE_MS );
		}*/
	}
	else
	{
	//	UARTprintf("\nSuccess to enter command mode!\n\n");
		retval = RN_ERR_OK; //return ok;
	}

        appendCRLF = 1;
        WiFi_Status_Retval = retval;
	return retval;
}

// Receive wifi response data to verify wifi data
int ReceiveWifiResponse(void)
{
	 uint32_t index = 0;
	 //if rx ringbuffer has no data, wait...........
	 if(SerialReceiveAvailable(WiFi_Port)) {vTaskDelay( 300/portTICK_RATE_MS );}//from 200 to 500
//	 UARTprintf("\nrecieved qty:%d\n",SerialReceiveAvailable(WiFi_Port));
//	 UARTprintf("\n");
	 while(SerialReceiveAvailable(WiFi_Port)&&(index<(ResponseFromWiFi_size-1))) //when wifi rx ringbuffer has data, receive it;
	 {
		 ResponseFromWiFi[index++] = (uint8_t)SerialReceive(WiFi_Port);
//		 UARTprintf("%c",ResponseFromWiFi[index-1]);
	 	 	vTaskDelay( 1/portTICK_RATE_MS ); // from reboot to receive cmd 57ms;

	 //	 	WiFi_Status_Retval = WiFi_Datain;
      //      WiFiLEDflashtime = 1;
	 }
//	 vTaskDelay( 1/portTICK_RATE_MS );
//	 UARTprintf(" n:%d ",index);

	 ResponseFromWiFi[index] = '\0';
//	 WiFi_Status_Retval = WiFi_Normal;
	 // finish the receiving; judge the response data;
	 if(index > 0)
	 return RN_ERR_OK;
	 else return RN_ERR_EMPTY;
}

// Receive wifi response data to verify wifi data
int ReceiveWifiResponse_INI(void)
{
	 uint32_t index = 0;
	 //if rx ringbuffer has no data, wait...........
	 if(SerialReceiveAvailable(WiFi_Port)) {vTaskDelay( 600/portTICK_RATE_MS );}//from 200 to 500
//	 UARTprintf("\nrecieved qty:%d\n",SerialReceiveAvailable(WiFi_Port));
//	 UARTprintf("\n");
	 while(SerialReceiveAvailable(WiFi_Port)&&(index<(ResponseFromWiFi_size-1))) //when wifi rx ringbuffer has data, receive it;
	 {
		 ResponseFromWiFi[index++] = (uint8_t)SerialReceive(WiFi_Port);
//		 UARTprintf("%c",ResponseFromWiFi[index-1]);
	 	 	vTaskDelay( 1/portTICK_RATE_MS ); // from reboot to receive cmd 57ms;

	 //	 	WiFi_Status_Retval = WiFi_Datain;
      //      WiFiLEDflashtime = 1;
	 }
//	 vTaskDelay( 1/portTICK_RATE_MS );
//	 UARTprintf(" n:%d ",index);

	 ResponseFromWiFi[index] = '\0';
//	 WiFi_Status_Retval = WiFi_Normal;
	 // finish the receiving; judge the response data;
	 if(index > 0)
	 return RN_ERR_OK;
	 else return RN_ERR_EMPTY;
}

/**************************************************************************************************/
int module_send_cmd(const char *response, const char *fmt, ...)
{
	int retval = RN_ERR_OK;
	uint32_t CMD_response_timeout;
//	const char *p;
	va_list argp;
	int CMD_retry_count =0;
	int ret;
	char *IPIndex;
	bool cmd_resend_flag = false;
	bool cmd_aok_cpy_check_flag = false;
/*
	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);//set wifi gpio5 0;
	while(GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)&GPIO_PIN_6 != 0) {
		UARTprintf("\n wait gpio6: 0\n");
	};
*/
	const char *p;
//	va_list argp;
	int i;
	char *s;
	static char fmtbuf[64]; //change parameter char length from 256 to 64;
	char chartemp[4];
	bool CMD_flag = false;
	bool joinCMD_flag = false;
	bool joinCMD_findOK_flag = false;
	appendCRLF =1;
	CMD_response_timeout = CMD_RESPONSE_TIMEOUT_general;
	if(response != (const char *)NULL)
	{
		ResponseFromWiFi[0] = '\0';

 	}
	//Wait until send condition ok;
	//Actively read rx ringbuffer until it's empty; and wait tx ringbuffer empty;
  	 while((RingBufUsed(&g_sRxBuf[WiFi_Port]) != 0) || (RingBufUsed(&g_sTxBuf[WiFi_Port])  != 0) ||(wifi_app_CMD_processing != false))
  			 {
  		 	 	 SerialReceive(WiFi_Port);
  			 };
	wifiCMD_Executing = true;
    Switch_WiFi_to_COM = 0;


	va_start(argp, fmt);
//	UARTprintf("\nPrint command to send:\n");
	for(p = fmt; *p != '\0'; p++)
	{
		if(*p != '%')
		{
//			UARTprintf("%c",*p);
                    ConsolePut(*p);
                    continue;
		}
		switch(*++p)
		{
		case 'c':
			i = va_arg(argp, int);
                        ConsolePut(i);
			break;

		case 'd':
			i = va_arg(argp, int);
//			UARTprintf("%d",i);
			s = my_itoa(fmtbuf, i, 10);
                        ConsolePutString(s);
                     //   UARTprintf("%s",s);
			break;

		case 's':
			s = va_arg(argp, char *);
		//	pcStr = va_arg(vaArgP, char *);
                        ConsolePutString(s);
//                        UARTprintf("%s",s);
			break;

		case 'x':
		case 'X':
			i = va_arg(argp, int);
			s = my_itoa(fmtbuf, i, 16);
                        ConsolePutString(s);
			break;

		case '%':
                        ConsolePut('%');
			break;
		}
		vTaskDelay( 1/portTICK_RATE_MS );
	}
//	UARTprintf("\n");
	strncpy(chartemp,fmt,3);
	chartemp[3] = '\0';
	if( strstr((const char *)chartemp, "$$$") !=NULL )
	{
		// mark $$$ cmd;
		appendCRLF = 0;
	}


        if(appendCRLF)
        {
            ConsolePut('\r');
            ConsolePut('\n');
        }

//	UARTprintf("\n");
	strncpy(chartemp,fmt,3);
	chartemp[3] = '\0';
	if( strstr((const char *)chartemp, "$$$") !=NULL )
	{
		// mark $$$ cmd;
		CMD_flag = true;
		UARTprintf("cmdstart\n");
	}
	else 	if( strstr((const char *)chartemp, "sca") !=NULL ) // sacn cmd;
	{
		// mark $$$ cmd;
		CMD_response_timeout = RX_CMD_RESPONSE_TIMEOUT_scan ;
//		UARTprintf("scan_start\n");
	}

	else 	if( strstr((const char *)chartemp, "joi") !=NULL ) // sacn cmd;
	{
		// mark $$$ cmd;
		CMD_response_timeout = RX_CMD_RESPONSE_TIMEOUT_scan ;
		joinCMD_flag = true;
		UARTprintf("join_start\n");
	}




   // if expect response data, receive response data;
	if(response != (const char *)NULL)
	{
		uint32_t r,i;
		bool CMDCopy_flag,AOK_flag;
		retval = RN_ERR_READ;

//		vTaskDelay( 20/portTICK_RATE_MS );
		for(r = 0,i=0; r < CMD_response_timeout; ++r)
			{

			ret = ReceiveWifiResponse();

			if( ret == RN_ERR_OK  )	//response has data;
			{
//				UARTprintf("\nReceive WiFi first Response Data:  %s \n",ResponseFromWiFi);
//				UARTprintf("\nfmt data:  %s \n",fmt);

//				UARTprintf("\nchartemp data:  %s \n",chartemp);
				if((!cmd_resend_flag)&&(!cmd_aok_cpy_check_flag))
				{
					if(strstr((const char *)ResponseFromWiFi,chartemp) !=NULL)
						CMDCopy_flag = true;
					else CMDCopy_flag = false;
					if( strstr((const char *)ResponseFromWiFi, response) !=NULL ) // find string ok;
						AOK_flag = true;
					else AOK_flag = false;
					cmd_aok_cpy_check_flag = true;
				}


				//deal with ERR: information; when module error, its echo is wrong, and send with "ERR: ?-Cmd";
				if( strstr((const char *)ResponseFromWiFi, "ERR") !=NULL ) // cmd error;
				{
					UARTprintf("\nfind ERR:  %s \n",ResponseFromWiFi);
					//need to send command again; timeout 3 times;
					if(response != (const char *)NULL)
					{
						ResponseFromWiFi[0] = '\0';

				 	}
					//Wait until send condition ok;
					//Actively read rx ringbuffer until it's empty; and wait tx ringbuffer empty;
				  	 while((RingBufUsed(&g_sRxBuf[WiFi_Port]) != 0) || (RingBufUsed(&g_sTxBuf[WiFi_Port])  != 0) ||(wifi_app_CMD_processing != false))
				  			 {
				  		 	 	 SerialReceive(WiFi_Port);
				  			 };
					wifiCMD_Executing = true;
					va_start(argp, fmt);
					UARTprintf("\nfind ERR,resend\n");
					for(p = fmt; *p != '\0'; p++)
					{
						if(*p != '%')
						{
//							UARTprintf("%c",*p);
				                    ConsolePut(*p);
				                    continue;
						}
						switch(*++p)
						{
						case 'c':
							i = va_arg(argp, int);
				                        ConsolePut(i);
							break;

						case 'd':
							i = va_arg(argp, int);
//							UARTprintf("%d",i);
							s = my_itoa(fmtbuf, i, 10);
				                        ConsolePutString(s);
				                     //   UARTprintf("%s",s);
							break;

						case 's':
							s = va_arg(argp, char *);
						//	pcStr = va_arg(vaArgP, char *);
				                        ConsolePutString(s);
//				                        UARTprintf("%s",s);
							break;

						case 'x':
						case 'X':
							i = va_arg(argp, int);
							s = my_itoa(fmtbuf, i, 16);
				                        ConsolePutString(s);
							break;

						case '%':
				                        ConsolePut('%');
							break;
						}
						vTaskDelay( 1/portTICK_RATE_MS );
					}
//					UARTprintf("\n");
					strncpy(chartemp,fmt,3);
					chartemp[3] = '\0';
					if( strstr((const char *)chartemp, "$$$") !=NULL )
					{
						// mark $$$ cmd;
						appendCRLF = 0;
					}


				        if(appendCRLF)
				        {
				            ConsolePut('\r');
				            ConsolePut('\n');
				        }



					if(response != (const char *)NULL) // if expect response data, receive response data;
					{
						while(!SerialReceiveAvailable(WiFi_Port))
						{
							 vTaskDelay(20/portTICK_RATE_MS);
						}
						ret = ReceiveWifiResponse();
					}
					cmd_aok_cpy_check_flag =false;
					if(strstr((const char *)ResponseFromWiFi,chartemp) !=NULL)
						CMDCopy_flag = true;
					else CMDCopy_flag = false;
					if( strstr((const char *)ResponseFromWiFi, response) !=NULL ) // find string ok;
						AOK_flag = true;
					else AOK_flag = false;
					cmd_resend_flag = true;
					cmd_aok_cpy_check_flag = true;

				}

				if(CMDCopy_flag &&!AOK_flag) //only find command copy; need to wait for the second response data;
				{

					 // other commands except "save "command; scard the first invalid data, wait for the next..
//					if(joinCMD_flag==true)
//						UARTprintf("\n10command:  %s \n",ResponseFromWiFi);
//						UARTprintf("\n 10 Wait for the second response data!\n");
						for(i=0;i < CMD_response_timeout;i++) // wait for the second response data;
						{
							ret = ReceiveWifiResponse();
							if( ret == RN_ERR_OK  )	//response has data;
							{
//								if(joinCMD_flag==true)
//								UARTprintf("\n10ok 2nd response ok: %d\n",ret);
							//	UARTprintf("\nWait for sec data time %dms!\n",r*3+i*3);
								if( strstr((const char *)ResponseFromWiFi, response) !=NULL ) // find string ok;
								{
//									UARTprintf("\n10ok find str\n");
//									if(joinCMD_flag==true)
//								UARTprintf("\n10ok 2nd:  %s \n",ResponseFromWiFi);
//										UARTprintf("\n10 join ok\n");
								retval = RN_ERR_OK;
								// save response data for scan command;
									if(strcmp(response,"Found")==0)
									{
//										UARTprintf("\n10ok find Found\n");
										ResponseFromWiFi_SCAN[0] = '\0';
										strcpy((char *)ResponseFromWiFi_SCAN,(char *)ResponseFromWiFi);
						//				UARTprintf("\nReceive second response Data:  %s \n",ResponseFromWiFi);
//										UARTprintf("\nRevd Data: %s \n",ResponseFromWiFi_SCAN);
									}
									WiFi_Status_Retval = WiFi_Normal;
									wifiCMD_Executing=false;
//									UARTprintf("\n10ok return %d\n",retval);
								    resume_channel_connection();
										return(retval);
								}
								else
								{ // have data, but don't find what we need;
							//		UARTprintf("Received second response data, but it's not expected!\n");
//									if(joinCMD_flag==true)
//									UARTprintf("\n10 not ok:  %s \n",ResponseFromWiFi);

									// find string ok;
									if((joinCMD_flag)&&(!joinCMD_findOK_flag)&&( strstr((const char *)ResponseFromWiFi, "OK") !=NULL ))
									{
//											UARTprintf("\n join fb ok\n");
											//clear response;
											ResponseFromWiFi[0] = '\0';
											joinCMD_findOK_flag = true;
									}
									else if((joinCMD_flag)&&(!joinCMD_findOK_flag)&&( strstr((const char *)ResponseFromWiFi, "FAIL") !=NULL ))
									{
										retval = RN_ERR_Fault;
										WiFi_Status_Retval = WiFi_EnterCMD_Error;
										wifiCMD_Executing=false;
									    resume_channel_connection();
										return(retval);
									}
									/*
									if((joinCMD_flag)&&(!joinCMD_findOK_flag))// if find SCA, wait Asso again;
									{
//										UARTprintf("\n join cmd11\n");
										//clear response;
										ResponseFromWiFi[0] = '\0';
										joinCMD_findOK_flag = true;

									}
									*/
									if(joinCMD_flag)
									{
										if((joinCMD_findOK_flag)&&( strstr((const char *)ResponseFromWiFi, "Associ") !=NULL )) //response =="Associated"; join ok;
										{
											retval = RN_ERR_OK;
											WiFi_Status_Retval = WiFi_Normal;
											wifiCMD_Executing=false;
											UARTprintf("\n10 join ok\n");
										    resume_channel_connection();
											return(retval);
										}
										else if((joinCMD_findOK_flag)&&( strstr((const char *)ResponseFromWiFi, "Disc") !=NULL ))
										{
											retval = WiFi_STA_CNN_Pass_ERR;
											WiFi_Status_Retval = WiFi_EnterCMD_Error;
											wifiCMD_Executing=false;
										    resume_channel_connection();
											return(retval);
										}
										else{
											vTaskDelay( 2/portTICK_RATE_MS );
										}
									}
/*
									else{
										retval = RN_ERR_Fault;
										WiFi_Status_Retval = WiFi_EnterCMD_Error;
										wifiCMD_Executing=false;
									    resume_channel_connection();
										return(retval);
									}
*/



								}

							}
							else{
								// still have no second data;
								vTaskDelay( 2/portTICK_RATE_MS );
//								UARTprintf("\n10ok 2nd response nok: %d\n",ret);
						//		retval = ret;
							}

						}
//						UARTprintf("\n 10 for loop timeout!\n");

						retval = RN_ERR_Fault;
						WiFi_Status_Retval = WiFi_EnterCMD_Error;
						wifiCMD_Executing=false;
					    resume_channel_connection();
						return(retval);

//						UARTprintf("\n 10 Wait 2nd data timeout!\n");
				}
				else if(CMDCopy_flag &&AOK_flag)//find both command copy and AOK response; command completed!
				{
					// find string ok;
					//UARTprintf("\n wait time:  %d ms \n",r*3);
//					if(joinCMD_flag==true)
//					UARTprintf("\n 11:  %s \n",ResponseFromWiFi);
					retval = RN_ERR_OK;
					// save response infomation to the special buffer for use later;
					if(strcmp(response,"IP=")==0) //response =="IP=";
					{
						//strncpy((char *)ResponseFromWiFi_IP,(char *)ResponseFromWiFi,60);
						//IP = (char *)ResponseFromWiFi_IP;
						if((IPIndex = strstr((const char *)ResponseFromWiFi, "IF=")) !=NULL )
							strncpy((char *)ResponseFromWiFi_IP,(char *)IPIndex,110);
						ResponseFromWiFi_IP[109] = '\0';
						//UARTprintf("\n ResponseFromWiFi_IP is :  %s \n",ResponseFromWiFi_IP);
					}
					else if(strcmp(response,"Mac Addr=")==0)
					{
						strncpy((char *)ResponseFromWiFi_Mac,(char *)ResponseFromWiFi,40);
						ResponseFromWiFi_Mac[39] = '\0';
					}
					else if(strcmp(response,"ERR")==0)
					{
						retval = RN_ERR_Fault;
					}
					else if(strcmp(response,"~c~")==0) //response =="IP=";
					{
						char *open_pointer;
						open_pointer = strstr((const char *)ResponseFromWiFi, "~o~");
						if(open_pointer !=NULL )
						{
							Find_TCP_open_in_Command = true;
						//	UARTprintf("+%s+\n",open_pointer);
							RingBufFlush(&wifi_pageRxBuf);
							while(*open_pointer != '\0')
							{
								RingBufWriteOne(&wifi_pageRxBuf, *open_pointer);
								open_pointer++;
							}
						}
											}

					WiFi_Status_Retval = WiFi_Normal;
					wifiCMD_Executing=false;
				    resume_channel_connection();
					return(retval);

				}
				else if(!CMDCopy_flag &&AOK_flag)//find AOK response only; command completed!
				{
					// find string ok;
			//		UARTprintf("\n  wait time:  %d ms \n",r*3);
//					if(joinCMD_flag==true)
//					UARTprintf("\n 01:  %s \n",ResponseFromWiFi);


						retval = RN_ERR_OK;
						WiFi_Status_Retval = WiFi_Normal;
						wifiCMD_Executing=false;
					    resume_channel_connection();
						return(retval);



				}
				else if((!CMDCopy_flag) &&(!AOK_flag))// such as join command; no copy; no aok, only judge associated.
				{
					// find string ok;
			//		UARTprintf("\n  wait time:  %d ms \n",r*3);
//					if(joinCMD_flag==true)
//					UARTprintf("\n 00:  %s \n",ResponseFromWiFi);




//					else // received invalid string; warning;
//					{
			//		UARTprintf("Received second response data, but it's not expected!\n");
			//		UARTprintf("\n00:  %s \n",ResponseFromWiFi);
					retval = RN_ERR_Fault;
					WiFi_Status_Retval = WiFi_EnterCMD_Error;
					wifiCMD_Executing=false;
				    resume_channel_connection();
					return(retval);
//					}

				}

					}

					else if( ret == RN_ERR_EMPTY  ) //response has no data;
					{
						// if cmd fail, need deal with it here; such as input $$$, but no response CMD string back;
						// one method is to send $$$ again in external; but here need to short the waiting time for CMD;
						vTaskDelay( 2/portTICK_RATE_MS );
						retval = ret;

						if(CMD_flag&&(CMD_retry_count>CMD_retry_time)) // find $$$ CMD; and wait time over 100;

							{
							//resend $$$;

							// return;
							wifiCMD_Executing=false;
						    resume_channel_connection();
							return(retval);

							}
						else if(CMD_flag&&(CMD_retry_count<CMD_retry_time))
						CMD_retry_count++;

					}

			}
		// first timeout, led red flash;
//			UARTprintf("\n elapse time:  %d ms \n",r*3+i*3);
			retval = RN_ERR_Fault;
			WiFi_Status_Retval = WiFi_EnterCMD_Error;
		}
//	UARTprintf("\n return value:  %d \n",retval);
	if( ret == RN_ERR_EMPTY  )
		retval = ret;
	wifiCMD_Executing=false;
    resume_channel_connection();
	return retval;
}

void WiFiData_TCP_S(void)
{
	/*	1.$$$
	//	2.set ip protocol 2
	//	3.set ip localport %d
	//	4.save
	//	5.reboot(if AP mode)
	 *    join(if STA mode)
	//  //6.get ip
	*/
	int retval;
	tWiFiCMD pxMessage;
		if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
		{
			//RN_RED_LED = 1;
			WiFi_Status_Retval = retval;
			UARTprintf("Fail to enter CMD mode! \n");
			wifi_cmd_err_exit();
		}
			// 1. enable ap mode:"set wlan join 7"
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip protocol 2")))
			{
	                 //   RN_RED_LED = 1;
				UARTprintf("Fail to set ip protocol 2! \n");
				wifi_cmd_err_exit();
			}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip localport %d",g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort )))
			{
			        //RN_RED_LED = 1;
				UARTprintf("set ip localport! \n");
				wifi_cmd_err_exit();
			}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save" )))
			{
			        //RN_RED_LED = 1;
				UARTprintf("fail to save \n");
				wifi_cmd_err_exit();
			}
//		else module_send_cmd("Storing", "save" );
			/*{
			        //RN_RED_LED = 1;
				UARTprintf("save! \n");
			}*/
/*		if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == STA_Mode)
				//module_send_cmd("Associated", "join");
			if(RN_NOT_ERR_OK(retval, module_send_cmd("Associated", "join")))
			{
			//   RN_RED_LED = 1;
			UARTprintf("Fail to join! \n");
			}*/
	//	if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == AP_Mode)
						//module_send_cmd("Associated", "join");
		else if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
			{
				//   RN_RED_LED = 1;
				UARTprintf("Fail to reboot! \n");
				wifi_cmd_err_exit();

			}
			delayms(1);
			if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == STA_Mode)
				WiFiSTAJoin();
		/*else if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
				{
					//RN_RED_LED = 1;
					WiFi_Status_Retval = retval;
					UARTprintf("Fail to enter CMD mode! \n");
				}*/
		/*if(RN_NOT_ERR_OK(retval, module_send_cmd("IP=", "get ip"))) // get the IP address, and send to queue, and feedback to web page;
						{
			              //  RN_RED_LED = 1;
							UARTprintf("Fail to get IP address! \n");
						}*/
		pxMessage.ui8Name = PORT_TELNET_S_C;
		xQueueSendToBack( g_QueWiFiDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;
}
void WiFiData_TCP_C(void)
{
	/*	1.$$$
	//	2.set ip protocol 8
	//	3.set ip localport %d
	//	4.set ip remote %d
	//  5.set ip host %s
	//  7.save
	//  8.reboot
	//  9.$$$
	//  2.join(if STA mode)
	//  6.open %s %d
	*/
	int retval;
	tWiFiCMD pxMessage;
		if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
		{
			//RN_RED_LED = 1;
			WiFi_Status_Retval = retval;
			UARTprintf("Fail to enter CMD mode! \n");
			wifi_cmd_err_exit();
		}

			// 1. enable ap mode:"set wlan join 7"
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip protocol 8")))
			{
	                 //   RN_RED_LED = 1;
				UARTprintf("Fail to set ip protocol 2! \n");
				wifi_cmd_err_exit();
			}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip localport %d",g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort )))
			{
			        //RN_RED_LED = 1;
				UARTprintf("set ip localport! \n");
				wifi_cmd_err_exit();
			}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip remote %d",g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort )))
			{
			        //RN_RED_LED = 1;
				UARTprintf("set ip remote! \n");
				wifi_cmd_err_exit();
			}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip host %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr) )))
			{
			        //RN_RED_LED = 1;
				UARTprintf("set host ip! \n");
				wifi_cmd_err_exit();
			}
		else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save" )))
			{
			        //RN_RED_LED = 1;
				UARTprintf("save! \n");
				wifi_cmd_err_exit();
			}
		else module_send_cmd("Reboot", "reboot");
/*
			if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot"))) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
			{
				//RN_RED_LED = 1;
				UARTprintf("Fail to reboot! \n");
			}
		delayms(1);
		else
*/
		module_cmd_mode();
		//delayms(1000);
		// join
		if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == STA_Mode)
		//
/*			if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set wlan pass %s",g_sParameters.sWiFiModule.sWiFiStaionSet.ui8InputPasswords)))
			{
				//   RN_RED_LED = 1;
				UARTprintf("Fail to join! \n");
			}*/
			if(RN_NOT_ERR_OK(retval, module_send_cmd("Associated", "join")))
			{
				//if(RN_NOT_ERR_OK(retval, module_send_cmd("lease=", "join")))
				//{
					UARTprintf("Fail to join! \n");
					wifi_cmd_err_exit();
				//}
			}
		//delayms(1500);
//		vTaskDelay( 1800/portTICK_RATE_MS );
			else if(RN_NOT_ERR_OK(retval, module_send_cmd("IP=", "get ip")))
			{
				UARTprintf("Fail to get ip! \n");
				wifi_cmd_err_exit();
			}
		//delayms(1);
			else if(RN_NOT_ERR_OK(retval, module_send_cmd("Connect","open %s %d",ipconvtosring(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr),
																						g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort )))
			{
			        //RN_RED_LED = 1;
				UARTprintf("Fail to open remote ip! \n");
				wifi_cmd_err_exit();
			}

		/*if(RN_NOT_ERR_OK(retval, module_send_cmd("IP=", "get ip"))) // get the IP address, and send to queue, and feedback to web page;
			{
			              //  RN_RED_LED = 1;
				UARTprintf("Fail to get IP address! \n");
			}*/
		//else module_send_cmd("IP=", "get ip");
		pxMessage.ui8Name = WiFi_DataTrans_TCP_Client;
		xQueueSendToBack( g_QueWiFiDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;
}

void WiFiData_UDP(void)
{
		/*	1.$$$
		//	2.join(if STA mode)
		//  3.set ip protocol 1
		//	3.set ip localport %d
		//	4.set ip remote %d
		//  5.set ip host %s
		//  6.save
		//	7.reboot(if AP mode)
		 *    exit(if STA mode)
		//	//8.$$$(if AP mode)
		 *    //get ip
		//  //9.exit(if AP mode)
		*/
		int retval;
		tWiFiCMD pxMessage;
			if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
			{
				//RN_RED_LED = 1;
				WiFi_Status_Retval = retval;
				UARTprintf("Fail to enter CMD mode! \n");
				wifi_cmd_err_exit();
			}
/*			//join
			if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == STA_Mode)
							//module_send_cmd("Associated", "join");
				if(RN_NOT_ERR_OK(retval, module_send_cmd("Associated", "join")))
					{
					 //   RN_RED_LED = 1;
					UARTprintf("Fail to join! \n");
					}
			*/
				// 1. enable ap mode:"set wlan join 7"
			else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip protocol 1")))
				{
		                 //   RN_RED_LED = 1;
					UARTprintf("Fail to set ip protocol 1! \n");
					wifi_cmd_err_exit();
				}
			else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip localport %d",g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort )))
				{
				        //RN_RED_LED = 1;
					UARTprintf("set ip localport! \n");
					wifi_cmd_err_exit();
				}
			else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip remote %d",g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort )))
				{
				        //RN_RED_LED = 1;
					UARTprintf("set ip remote! \n");
					wifi_cmd_err_exit();
				}
			else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set ip host %s",ipconvtosring(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr) )))
				{
				        //RN_RED_LED = 1;
					UARTprintf("set host ip! \n");
					wifi_cmd_err_exit();
				}
			else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
				{
				UARTprintf("Fail to save!\n");
				wifi_cmd_err_exit();
				}
			else module_send_cmd("EXIT", "exit");

			//judge of wifi mode
/*
			if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == AP_Mode)
				//module_send_cmd("Associated", "join");
				if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
			{
				UARTprintf("Fail to reboot!\n");
			}
*/
				/*else module_cmd_mode();
			module_send_cmd("IP=", "get ip");
			if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == AP_Mode)
				//module_send_cmd("Associated", "join");
				if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
				{
					UARTprintf("Fail to exit!\n");
				}*/
			pxMessage.ui8Name = WiFi_DataTrans_UDP;
			xQueueSendToBack( g_QueWiFiDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;
}
void Syn_Modname(void)
{

	/*	1.$$$
	//	2.set opt deviceid **
	//  6.save
	//	7.reboot(if AP mode)
	*/
	int retval;
	tWiFiCMD pxMessage;
		if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
		{
			//RN_RED_LED = 1;
			WiFi_Status_Retval = retval;
			UARTprintf("Fail to enter CMD mode! \n");
			wifi_cmd_err_exit();
		}

		else if(RN_NOT_ERR_OK(retval, module_send_cmd(CMD_AOK, "set o d %s",g_sParameters.ui8ModName )))
		{
		    UARTprintf("Fail to set opt deviceid\n");
		    wifi_cmd_err_exit();
		}
		//15.save ;
		else if(RN_NOT_ERR_OK(retval, module_send_cmd("Storing", "save")))
		{
            UARTprintf("Fail to save ssid! \n");
            wifi_cmd_err_exit();
		}
/*		//reboot
		if(RN_NOT_ERR_OK(retval, module_send_cmd("Reboot", "reboot")))
		{
            UARTprintf("Reboot fail! \n");
		}

		if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == STA_Mode)
			if(RN_NOT_ERR_OK(retval, module_send_cmd("SCAN OK", "join")))
				{
					UARTprintf("Fail to join wirless net! \n");
				}
*/
		else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
		{
			UARTprintf("Fail to exit! \n");

		}

		pxMessage.ui8Name = WiFi_DeviceID_Altered;
		xQueueSendToBack( g_QueWiFiDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;
}


void TCP_CLOSE(void)
{
	int retval;
//	tWiFiCMD pxMessage;
	UARTprintf("\ncl_cmd_s\n");
	//setup data path;
	TCP_close_processing = true;
	Switch_WiFi_to_COM = 0;
	Switch_LAN_to_COM = 0;
//	save_WiFi_TCP_connected= WiFi_TCP_connected;
//	save_Webpage_Visit_Req= Webpage_Visit_Req;
	WiFi_TCP_connected = false;
	Webpage_Visit_Req = false;
//	vTaskDelay( 100/portTICK_RATE_MS );

	 if(RN_NOT_ERR_OK(retval, module_cmd_mode())) // if RN_NOT_ERR_OK() is 1,  fail to enter cmd mode;
	{
		//RN_RED_LED = 1;
		UARTprintf("Fail to enter CMD mode! \n");
		wifi_cmd_err_exit();
	}
	 else if(RN_NOT_ERR_OK(retval, module_send_cmd("~c~", "close")))
	{
		UARTprintf("Fail to close TCP \n");
		wifi_cmd_err_exit();
	}
		//11.exit command;
//	 else if(RN_NOT_ERR_OK(retval, module_send_cmd(NULL, "exit")))
	 else if(RN_NOT_ERR_OK(retval, module_send_cmd("EXIT", "exit")))
	{
		// RN_RED_LED = 1;
		UARTprintf("Fail to exit CMD  mode! \n");
	}
//		pxMessage.ui8Name = WiFlyPage_TCP_Close;
//		xQueueSendToBack( g_QueWiFiDataBack, ( void * ) &pxMessage, ( portTickType ) 0 ); // sync;

		//resume breakpoint;
//		WiFi_TCP_connected= save_WiFi_TCP_connected;
//		Webpage_Visit_Req= Webpage_Visit_Req;

		if(Find_TCP_open_in_Command)
		{
			WiFi_TCP_connected = true;
			Webpage_Visit_Req = true;
		}

		//Find_TCP_open_in_Command = false;
		TCP_close_processing = false;

		if(g_sParameters.datachansel==2)//1, lan-com;2; wilfi-com;
		{
			Switch_WiFi_to_COM = 1;
			Switch_LAN_to_COM = 0;
		}
		else if(g_sParameters.datachansel==1)//1, lan-com;2; wilfi-com;
		{
			Switch_WiFi_to_COM = 0;
			Switch_LAN_to_COM = 1;
		}
		else if(g_sParameters.datachansel==3)//1, lan-com;2; wilfi-com;3: both Lan and WiFi to COM enable;
		{
			Switch_WiFi_to_COM = 1;
			Switch_LAN_to_COM = 1;
		}
//		vTaskDelay( 100/portTICK_RATE_MS );

		UARTprintf("\ncl_cmd_d\n");

}
