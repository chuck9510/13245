//*****************************************************************************
//
// config.c - Configuration of the serial to Ethernet converter.
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
#include <string.h>
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/uart.h"
#include "utils/eeprom_pb.h"
#include "utils/locator.h"
#include "utils/lwiplib.h"
#include "utils/ustdlib.h"
#include "httpserver_raw/httpd.h"
#include "config.h"
#include "serial.h"
#include "telnet.h"
#include "user_define.h"
#include "driverlib/sysctl.h"
#include "stdio.h"

#include "driverlib/eeprom.h"

#include "WiFly_Webpage.h"
//*****************************************************************************
//
//! \addtogroup config_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! Flags sent to the main loop indicating that it should update the IP address
//! after a short delay (to allow us to send a suitable page back to the web
//! browser telling it the address has changed).
//
//*****************************************************************************
uint8_t g_ui8UpdateRequired;
bool if_ap = false;
uint8_t permit = 0;
uint8_t login_counter = 0;
bool logoff_htm = true; //for index.htm
bool logoff_shtm = true; //for .shtm
bool APPW_NULL = false;
bool StaPW_NULL = false;
bool Wifly_AP_STA = false;
extern bool WiFly_Webaccess_Ready;
extern bool Wifly_Acc;
extern bool Wifly_modname_modify;
uint8_t join_AP_pw_needed = 0;
//uint8_t Fw_Rev = 5;

uint8_t feedback = 0;
extern uint32_t g_ui32IPAddress;
//uint8_t modbus = 1;
//0822
//uint8_t RFC2217_EN=1;
typedef struct {
	//security index;
	uint8_t ui8index;

	//security mode name
	char *SecModeName;
} Sec_Mode;

Sec_Mode Sec_Mode_Tab[10] = { { 0, "OPEN" }, { 1, "WEP" }, { 2, "WPA1" }, { 3,
		"MIXED" }, { 4, "WPA2" }, { 5, "Enterprice WEP" }, { 6,
		"Enterprice WPA1" }, { 7, "Enterprice WPA mixed" }, { 8,
		"Enterprice WPA2" }, { 9, "Enterprice NO security" } };
//*****************************************************************************
//
//! The maximum length of any HTML form variable name used in this application.
//
//*****************************************************************************
#define MAX_VARIABLE_NAME_LEN   16

//*****************************************************************************
//
// SSI tag indices for each entry in the g_pcSSITags array.
//0823
//*****************************************************************************
#define SSI_INDEX_IPADDR        0
#define SSI_INDEX_MACADDR       1
#define SSI_INDEX_P0BR          2
#define SSI_INDEX_P0SB          3
#define SSI_INDEX_P0P           4
#define SSI_INDEX_P0BC          5
#define SSI_INDEX_P0FC          6
#define SSI_INDEX_P0TT          7
#define SSI_INDEX_P0TLP         8
#define SSI_INDEX_P0TRP         9
#define SSI_INDEX_P0TIP         10
#define SSI_INDEX_P0TIP1        11
#define SSI_INDEX_P0TIP2        12
#define SSI_INDEX_P0TIP3        13
#define SSI_INDEX_P0TIP4        14
//#define SSI_INDEX_P0TNM         15
#define SSI_INDEX_MODNAME       15
#define SSI_INDEX_PNPPORT       16
#define SSI_INDEX_DISABLE       17
#define SSI_INDEX_DVARS         18
#define SSI_INDEX_P0VARS        19
#define SSI_INDEX_MODNINP       20
#define SSI_INDEX_PNPINP        21
//#define SSI_INDEX_P0TVARS       23
#define SSI_INDEX_P0IPVAR       22
#define SSI_INDEX_IPVARS        23
#define SSI_INDEX_SNVARS        24
#define SSI_INDEX_GWVARS        25
#define SSI_INDEX_REVISION      26
//#define SSI_INDEX_P0PROT        29
#define SSI_INDEX_P0TM          27
#define SSI_INDEX_P0URVARS      28//chris_web
#define SSI_INDEX_P0NETVARS     29
#define SSI_INDEX_P0WS	        30
#define SSI_INDEX_P0MX			31
//****WIFI tag ***********************//
#define SSI_INDEX_WIFIMD			32
#define SSI_INDEX_WIFINN			33
#define SSI_INDEX_WIFIMAC			34
#define SSI_INDEX_WIFISM			35
#define SSI_INDEX_WIFINM			36
#define SSI_INDEX_WIFIFRE			37
#define SSI_INDEX_WIFIIP			38
#define SSI_INDEX_WIFIMK			39
#define SSI_INDEX_WIFIDT			40
#define SSI_INDEX_WIFIAPID			41
#define SSI_INDEX_WIFISTASM			42
#define SSI_INDEX_WIFISTAET			43
#define SSI_INDEX_WIFICT			44
#define SSI_INDEX_WIFIPW			45//chris_0820
#define SSI_INDEX_RFC				46//0822
#define SSI_INDEX_APPW				47//0907
#define SSI_INDEX_CHSW				48//0907
#define SSI_INDEX_STAIP				49//0907
#define SSI_INDEX_STAMK				50//0907
#define SSI_INDEX_STAGW				51//0907
#define SSI_INDEX_SWITCH_JS			52//.JS
#define SSI_INDEX_WIFIMD_JS			53//.JS
#define SSI_INDEX_APSET_JS			54//.JS
#define SSI_INDEX_APIP_JS			55//.JS
#define SSI_INDEX_APMS_JS			56//.JS
#define SSI_INDEX_STASET_JS			57//.JS
#define SSI_INDEX_STCIP_JS			58//.JS
#define SSI_INDEX_STCMS_JS			59//.JS
#define SSI_INDEX_STCGW_JS			60//.JS
#define SSI_INDEX_LOGIN_JS			61//.JS
#define SSI_INDEX_SCAN_JS			62//.js
#define SSI_INDEX_GTIP_JS			63//.js
#define SSI_INDEX_GTIP				64
#define SSI_INDEX_FEEDBACK_JS		65//.JS
#define SSI_INDEX_MODBUS			66
#define SSI_INDEX_UDP_IP			67
#define SSI_INDEX_UDP_IP_JS			68
#define SSI_INDEX_P0PROT			69
#define SSI_INDEX_WIFIPROL			70
#define SSI_INDEX_WFLP				71
#define SSI_INDEX_WFRP				72
#define SSI_INDEX_WFIP				73
#define SSI_INDEX_WFDAJS			74
#define SSI_INDEX_JOIN_AP			75
#define SSI_INDEX_P0PKT				76
#define SSI_INDEX_P0PKLEN			77
/*****************************Add by Chuck************************************************/
//com1
#define SSI_INDEX_P1BR          78
#define SSI_INDEX_P1SB          79
#define SSI_INDEX_P1P           80
#define SSI_INDEX_P1BC          81
#define SSI_INDEX_P1FC          82
#define SSI_INDEX_P1TT          83
#define SSI_INDEX_P1TLP         84
#define SSI_INDEX_P1TRP         85
#define SSI_INDEX_P1TIP         86
#define SSI_INDEX_P1TIP1        87
#define SSI_INDEX_P1TIP2        88
#define SSI_INDEX_P1TIP3        89
#define SSI_INDEX_P1TIP4        90
#define SSI_INDEX_P1VARS        91
#define SSI_INDEX_P1IPVAR       92
#define SSI_INDEX_P1TM          93
#define SSI_INDEX_P1URVARS      94//chris_web
#define SSI_INDEX_P1NETVARS     95
#define SSI_INDEX_P1WS	        96
#define SSI_INDEX_P1MX			97
#define SSI_INDEX_P1RFC				98//0822
#define SSI_INDEX_P1PROT			99
#define SSI_INDEX_P1PKT				100
#define SSI_INDEX_P1PKLEN			101
//com2
#define SSI_INDEX_P2BR          102
#define SSI_INDEX_P2SB          103
#define SSI_INDEX_P2P           104
#define SSI_INDEX_P2BC          105
#define SSI_INDEX_P2FC          106
#define SSI_INDEX_P2TT          107
#define SSI_INDEX_P2TLP         108
#define SSI_INDEX_P2TRP         109
#define SSI_INDEX_P2TIP         110
#define SSI_INDEX_P2TIP1        111
#define SSI_INDEX_P2TIP2        112
#define SSI_INDEX_P2TIP3        113
#define SSI_INDEX_P2TIP4        114
#define SSI_INDEX_P2VARS        115
#define SSI_INDEX_P2IPVAR       116
#define SSI_INDEX_P2TM          117
#define SSI_INDEX_P2URVARS      118//chris_web
#define SSI_INDEX_P2NETVARS     119
#define SSI_INDEX_P2WS	        120
#define SSI_INDEX_P2MX			121
#define SSI_INDEX_P2RFC				122//0822
#define SSI_INDEX_P2PROT			123
#define SSI_INDEX_P2PKT				124
#define SSI_INDEX_P2PKLEN			125

//*****************************************************************************
//
//! This array holds all the strings that are to be recognized as SSI tag
//! names by the HTTPD server.  The server will call ConfigSSIHandler to
//! request a replacement string whenever the pattern <!--#tagname--> (where
//! tagname appears in the following array) is found in ``.ssi'' or ``.shtm''
//! files that it serves.
//0823
//*****************************************************************************
const char *g_pcConfigSSITags[] = { "ipaddr",     // SSI_INDEX_IPADDR
		"macaddr",    // SSI_INDEX_MACADDR
		"p0br",       // SSI_INDEX_P0BR
		"p0sb",       // SSI_INDEX_P0SB
		"p0p",        // SSI_INDEX_P0P
		"p0bc",       // SSI_INDEX_P0BC
		"p0fc",       // SSI_INDEX_P0FC
		"p0tt",       // SSI_INDEX_P0TT
		"p0tlp",      // SSI_INDEX_P0TLP
		"p0trp",      // SSI_INDEX_P0TRP
		"p0tip",      // SSI_INDEX_P0TIP
		"p0tip1",     // SSI_INDEX_P0TIP1
		"p0tip2",     // SSI_INDEX_P0TIP2
		"p0tip3",     // SSI_INDEX_P0TIP3
		"p0tip4",     // SSI_INDEX_P0TIP4 "p0tnm",       SSI_INDEX_P0TNM
		"modname",    // SSI_INDEX_MODNAME
		"pnpport",    // SSI_INDEX_PNPPORT
		"disable",    // SSI_INDEX_DISABLE
		"dvars",      // SSI_INDEX_DVARS
		"p0vars",     // SSI_INDEX_P0VARS
		"modninp",    // SSI_INDEX_MODNINP
		"pnpinp",     // SSI_INDEX_PNPINP    "p0tvars",    // SSI_INDEX_P0TVARS
		"p0ipvar",    // SSI_INDEX_P0IPVAR
		"ipvars",     // SSI_INDEX_IPVARS
		"snvars",     // SSI_INDEX_SNVARS
		"gwvars",     // SSI_INDEX_GWVARS
		"revision",   // SSI_INDEX_REVISION
		"p0tm",      // SSI_INDEX_P0TM
		"p0urvars", //SSI_INDEX_P0URVARS  chris_web
		"p0netvars",	//SSI_INDEX_P0NETVARS
		"p0ws",		//SSI_INDEX_P0WS
		"p0mx",		//SSI_INDEX_P0MX
		"wifimd",		//SSI_INDEX_WIFIMD
		"wifinn",		//SSI_INDEX_WIFINN
		"wifimac",		//SSI_INDEX_WIFIMAC
		"wifism",		//SSI_INDEX_WIFISM
		"wifinm",		//SSI_INDEX_WIFINM
		"wififre",		//SSI_INDEX_WIFIFRE
		"wifiip",		//SSI_INDEX_WIFIIP
		"wifimk",		//SSI_INDEX_WIFIMK
		"wifidt",		//SSI_INDEX_WIFIDT
		"wifiapid",		//SSI_INDEX_WIFIAPID
		"wifistasm",		//SSI_INDEX_WIFISTASM
		"wifistaet",		//SSI_INDEX_WIFISTAET
		"wifict",		//SSI_INDEX_WIFICT
		"wifipw",		//SSI_INDEX_WIFIPW
		"p02217",		//SSI_INDEX_RFC 0822
		"appw",          //SSI_INDEX_APPW 0907
		"chsw",          //SSI_INDEX_CHSW
		"staip",        //SSI_INDEX_STAIP
		"stamk",         //SSI_INDEX_STAMK
		"stagw",         //SSI_INDEX_STAGW
		"CH_switch",		//SSI_INDEX_SWITCH_JS
		"wifimdjs",		//SSI_INDEX_WIFIMD_JS
		"apset",			//SSI_INDEX_APSET_JS
		"apip",			//SSI_INDEX_APIP_JS
		"apms",			//SSI_INDEX_APMS_JS
		"staset",		//SSI_INDEX_STASET_JS
		"stcip",		//SSI_INDEX_STCIP_JS
		"stcms",		//SSI_INDEX_STCMS_JS
		"stcgw",			//SSI_INDEX_STCGW_JS
		"Login",			//SSI_INDEX_LOGIN_JS
		"scan",				//SSI_INDEX_SCAN_JS
		"gtipjs",			//SSI_INDEX_GTIP_JS
		"gtip",			//SSI_INDEX_GTIP
		"feedback",			//SSI_INDEX_FEEDBACK_JS
		"modbus",		//SSI_INDEX_MODBUS
		"udpip",			//SSI_INDEX_UDP_IP
		"udpipvars",			//SSI_INDEX_UDP_IP_JS
		"p0prot",     //SSI_INDEX_P0PROT
		"wifiprol",     //SSI_INDEX_WIFIPROL
		"wflp",     //SSI_INDEX_WFLP
		"wfrp",     //SSI_INDEX_WFRP
		"wfip",     //SSI_INDEX_WFIP
		"wfdajs",	//SSI_INDEX_WFDAJS
		"joinAP",	//SSI_INDEX_JOIN_AP
		"p0pkt",	//SSI_INDEX_P0PKT
		"p0pklen",	//SSI_INDEX_P0PKLEN
		/************************Add by Chuck ********************************************/
		//COM1
		"p1br",// SSI_INDEX_P1BR
		"p1sb",       // SSI_INDEX_P1SB
		"p1p",        // SSI_INDEX_P1P
		"p1bc",       // SSI_INDEX_P1BC
		"p1fc",       // SSI_INDEX_P1FC
		"p1tt",       // SSI_INDEX_P1TT
		"p1tlp",      // SSI_INDEX_P1TLP
		"p1trp",      // SSI_INDEX_P1TRP
		"p1tip",      // SSI_INDEX_P1TIP
		"p1tip1",     // SSI_INDEX_P1TIP1
		"p1tip2",     // SSI_INDEX_P1TIP2
		"p1tip3",     // SSI_INDEX_P1TIP3
		"p1tip4",     // SSI_INDEX_P1TIP4 "p1tnm",       SSI_INDEX_P1TNM
		"p1vars",     // SSI_INDEX_P1VARS
		"p1ipvar",    // SSI_INDEX_P1IPVAR
		"p1tm",      // SSI_INDEX_P1TM
		"p1urvars", //SSI_INDEX_P1URVARS  chris_web
		"p1netvars",	//SSI_INDEX_P1NETVARS
		"p1ws",		//SSI_INDEX_P1WS
		"p1mx",		//SSI_INDEX_P1MX
		"p12217",		//SSI_INDEX_RFC 0822
		"p1prot",     //SSI_INDEX_P1PROT
		"p1pkt",	//SSI_INDEX_P1PKT
		"p1pklen",	//SSI_INDEX_P1PKLEN
		//COM2
		"p2br",// SSI_INDEX_P2BR
		"p2sb",       // SSI_INDEX_P2SB
		"p2p",        // SSI_INDEX_P2P
		"p2bc",       // SSI_INDEX_P2BC
		"p2fc",       // SSI_INDEX_P2FC
		"p2tt",       // SSI_INDEX_P2TT
		"p2tlp",      // SSI_INDEX_P2TLP
		"p2trp",      // SSI_INDEX_P2TRP
		"p2tip",      // SSI_INDEX_P2TIP
		"p2tip1",     // SSI_INDEX_P2TIP1
		"p2tip2",     // SSI_INDEX_P2TIP2
		"p2tip3",     // SSI_INDEX_P2TIP3
		"p2tip4",     // SSI_INDEX_P2TIP4 "p1tnm",       SSI_INDEX_P2TNM
		"p2vars",     // SSI_INDEX_P2VARS
		"p2ipvar",    // SSI_INDEX_P2IPVAR
		"p2tm",      // SSI_INDEX_P2TM
		"p2urvars", //SSI_INDEX_P2URVARS  chris_web
		"p2netvars",	//SSI_INDEX_P2NETVARS
		"p2ws",		//SSI_INDEX_P2WS
		"p2mx",		//SSI_INDEX_P2MX
		"p22217",		//SSI_INDEX_RFC 0822
		"p2prot",     //SSI_INDEX_P2PROT
		"p2pkt",	//SSI_INDEX_P2PKT
		"p2pklen"	//SSI_INDEX_P2PKLEN
		};
//*****************************************************************************
//
//! The number of individual SSI tags that the HTTPD server can expect to
//! find in our configuration pages.
//
//*****************************************************************************
//#define NUM_CONFIG_SSI_TAGS     (sizeof(g_pcConfigSSITags) / sizeof (char *))
int NUM_CONFIG_SSI_TAGS = sizeof(g_pcConfigSSITags) / sizeof(char *);
//*****************************************************************************
//
//! Prototype for the function which handles requests for config.cgi.
//
//*****************************************************************************
static const char *ConfigCGIHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for login.cgi.
//
//*****************************************************************************
static const char *LoginHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for logout.cgi.
//
//*****************************************************************************
static const char *LogoutHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]);
//*****************************************************************************
//! Prototype for the function which handles requests for search.cgi.
//
//*****************************************************************************
static const char *WiFiSearchHandler(int iIndex, int iNumParams,
		char *pcParam[], char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for searchWifi.cgi.
//
//*****************************************************************************
static const char *SelectAPHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for misc.cgi.
//0820
//*****************************************************************************
static const char *ConfigMiscCGIHandler(int iIndex, int iNumParams,
		char *pcParam[], char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for chswitch.cgi.
//
//*****************************************************************************
static const char *DataCHSelectHandler(int iIndex, int iNumParams,
		char *pcParam[], char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for ip.cgi.
//
//*****************************************************************************
static const char *ConfigIPCGIHandler(int iIndex, int iNumParams,
		char *pcParam[], char *pcValue[]);

//*****************************************************************************
//
//! Prototype for the function which handles requests for defaults.cgi.
//
//*****************************************************************************
static const char *ConfigDefaultsCGIHandler(int iIndex, int iNumParams,
		char *pcParam[], char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for pw.cgi.
//
//*****************************************************************************
static const char *
ConfigLoginCGIHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for wifiap.cgi.
//
//*****************************************************************************
static const char *
ConfigWifiAPHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]);

//*****************************************************************************
//
//! Prototype for the function which handles requests for wifista.cgi.
//
//*****************************************************************************
static const char *
ConfigWifiStaHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for wifimode.cgi.
//
//*****************************************************************************
static const char *
ConfigWifiModeHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for restart.cgi.
//
//*****************************************************************************
static const char *
ConfigRestartHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]);
//*****************************************************************************
//
//! Prototype for the function which handles requests for restart.cgi.
//
//*****************************************************************************
static const char *
WiFiTransModeHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]);

//*****************************************************************************
//
//! Prototype for the main handler used to process server-side-includes for the
//! application's web-based configuration screens.
//
//*****************************************************************************
uint16_t ConfigSSIHandler(int iIndex, char *pcInsert, int iInsertLen);

//*****************************************************************************
//
// CGI URI indices for each entry in the g_psConfigCGIURIs array.
//
//*****************************************************************************
#define CGI_INDEX_CONFIG        0
#define CGI_INDEX_MISC          1
#define CGI_INDEX_DEFAULTS      2
#define CGI_INDEX_IP            3

//*****************************************************************************
//
//! This array is passed to the HTTPD server to inform it of special URIs
//! that are treated as common gateway interface (CGI) scripts.  Each URI name
//! is defined along with a pointer to the function which is to be called to
//! process it.
//0820
//*****************************************************************************
const tCGI g_psConfigCGIURIs[] = { { "/login.cgi", LoginHandler }, {
		"/logout.cgi", LogoutHandler }, { "/config.cgi", ConfigCGIHandler }, // CGI_INDEX_CONFIG
		{ "/misc.cgi", ConfigMiscCGIHandler },          // CGI_INDEX_MISC
		{ "/defaults.cgi", ConfigDefaultsCGIHandler },  // CGI_INDEX_DEFAULTS
		{ "/ip.cgi", ConfigIPCGIHandler },              // CGI_INDEX_IP
		{ "/pw.cgi", ConfigLoginCGIHandler }, //chris_web
		{ "/restart.cgi", ConfigRestartHandler }, //chris_web
		{ "/wifiap.cgi", ConfigWifiAPHandler }, //chris_web
		{ "/wifista.cgi", ConfigWifiStaHandler }, //chris_web
		{ "/wifimode.cgi", ConfigWifiModeHandler }, //chris_web
		{ "/chswitch.cgi", DataCHSelectHandler }, //0907
		{ "/search.cgi", WiFiSearchHandler }, { "/searchWifi.cgi",
				SelectAPHandler }, { "/wifitm.cgi", WiFiTransModeHandler }, };

//*****************************************************************************
//
//! The number of individual CGI URIs that are used by our configuration
//! web pages.
//
//*****************************************************************************
//#define NUM_CONFIG_CGI_URIS     (sizeof(g_psConfigCGIURIs) / sizeof(tCGI))
int NUM_CONFIG_CGI_URIS = (sizeof(g_psConfigCGIURIs) / sizeof(tCGI));
//*****************************************************************************
//
//! The file sent back to the browser by default following completion of any
//! of our CGI handlers.
//
//*****************************************************************************
#define DEFAULT_CGI_RESPONSE    "/s2e.shtm"

//#define CONFIG_CGI_WLAN2UR    "/wlan2ur.shtm"//chris
#define CONFIG_CGI_WLAN2UR_COM0    "/wlan2ur_com0.shtm"//chuck
#define CONFIG_CGI_WLAN2UR_COM1    "/wlan2ur_com1.shtm"//chuck
#define CONFIG_CGI_WLAN2UR_COM2    "/wlan2ur_com2.shtm"//chuck
#define CONFIG_CGI_WLANAP    "/wlanap.shtm"//chris
#define CONFIG_CGI_QC    "/qc.shtm"//chris
#define CONFIG_CGI_WLANSTA    "/wlansta.shtm"//chris
#define CONFIG_CGI_MS    "/ms.shtm"//chris_0820
#define CONFIG_CGI_Eth    "/lan.shtm"//0907
#define CONFIG_CGI_Index    "/index.htm"
#define CONFIG_CGI_Login    "/login.shtm"
#define CONFIG_CGI_APTable   "/searchWifi.shtm"
//*****************************************************************************
//
//! The file sent back to the browser in cases where a parameter error is
//! detected by one of the CGI handlers.  This should only happen if someone
//! tries to access the CGI directly via the browser command line and doesn't
//! enter all the required parameters alongside the URI.
//
//*****************************************************************************
#define PARAM_ERROR_RESPONSE    "/perror.htm"

//*****************************************************************************
//
//! The file sent back to the browser to signal that the IP address of the
//! device is about to change and that the web server is no longer operating.
//
//*****************************************************************************
#define IP_UPDATE_RESPONSE      "/ipchg.shtm"

//*****************************************************************************
//
//! The URI of the ``Miscellaneous Settings'' page offered by the web server.
//
//*****************************************************************************
#define MISC_PAGE_URI           "/misc.shtm"
#define DEV_PAGE_URI           "/dev.shtm"
//*****************************************************************************
//
// Strings used for format JavaScript parameters for use by the configuration
// web pages.
//
//*****************************************************************************
#define JAVASCRIPT_HEADER                                                     \
		"<script type='text/javascript' language='JavaScript'><!--\n"
#define UR_JAVASCRIPT_VARS                                                   		\
		"var tm = %d;\n"                                                          	\
		"var br= %d;\n"                                                          	\
		"var sb = %d;\n"                                                          	\
		"var bc = %d;\n"                                                          	\
		"var fc = %d;\n"                                                          	\
		"var par = %d;\n"

//"var rfc = %d;\n"
#define NET_JAVASCRIPT_VARS                                                   \
		"var ws = %d;\n"                                                          \
		"var tlp= %d;\n"                                                          \
		"var trp = %d;\n"                                                          \
		"var tt = %d;\n"															\
		"var pt = %d;\n"															\
		"var pl = %d;\n"															\
		"var mb= %d;\n"																\
		"var tnp = %d;\n"

//"var mx = %d;\n"                                                          \   max client number
#define TIP_JAVASCRIPT_VARS                                                   \
		"var tip1 = %d;\n"                                                        \
		"var tip2 = %d;\n"                                                        \
		"var tip3 = %d;\n"                                                        \
		"var tip4 = %d;\n"
#define IP_JAVASCRIPT_VARS                                                    \
		"var staticip = %d;\n"                                                    \
		"var sip1 = %d;\n"                                                        \
		"var sip2 = %d;\n"                                                        \
		"var sip3 = %d;\n"                                                        \
		"var sip4 = %d;\n"
#define SUBNET_JAVASCRIPT_VARS                                                \
		"var mip1 = %d;\n"                                                        \
		"var mip2 = %d;\n"                                                        \
		"var mip3 = %d;\n"                                                        \
		"var mip4 = %d;\n"
#define GW_JAVASCRIPT_VARS                                                    \
		"var gip1 = %d;\n"                                                        \
		"var gip2 = %d;\n"                                                        \
		"var gip3 = %d;\n"                                                        \
		"var gip4 = %d;\n"
#define UDPIP_JAVASCRIPT_VARS												\
		"var uip1 = %d;\n"                                                        \
		"var uip2 = %d;\n"                                                        \
		"var uip3 = %d;\n"                                                        \
		"var uip4 = %d;\n"
#define SW_JAVASCRIPT_VARS                                                   \
		"var sw = %d;\n"
#define WM_JAVASCRIPT_VARS                                                   \
		"var wm = %d;\n"												\
		"var tprol = %d;\n"												\
		"var lp = %d;\n"												\
		"var rp = %d;\n"												\
		"var wfip1 = %d;\n"                                                        \
		"var wfip2 = %d;\n"                                                        \
		"var wfip3 = %d;\n"                                                        \
		"var wfip4 = %d;\n"
#define APSET_JAVASCRIPT_VARS                                                   \
		"var apf = %d;\n"                                                        \
		"var apdh = %d;\n"                                                        \
		"var apsid = \"%s\";\n"                                                        \
		"var apw = \"%s\";\n"
#define APIP_JAVASCRIPT_VARS                                                    \
		"var apip1 = %d;\n"                                                        \
		"var apip2 = %d;\n"                                                        \
		"var apip3 = %d;\n"                                                        \
		"var apip4 = %d;\n"
#define APMK_JAVASCRIPT_VARS                                                    \
		"var apmk1 = %d;\n"                                                        \
		"var apmk2 = %d;\n"                                                        \
		"var apmk3 = %d;\n"                                                        \
		"var apmk4 = %d;\n"
#define APGW_JAVASCRIPT_VARS                                                    \
		"var apgip1 = %d;\n"                                                        \
		"var apgip2 = %d;\n"                                                        \
		"var apgip3 = %d;\n"                                                        \
		"var apgip4 = %d;\n"
#define STASET_JAVASCRIPT_VARS                                                   \
		"var stacon = %d;\n"														\
		"var stapw = '%s';\n"														\
		"var apssid = '%s';\n"														\
		"var join_AP = %d;"
#define STCIP_JAVASCRIPT_VARS                                                    \
		"var stcip1 = %d;\n"                                                        \
		"var stcip2 = %d;\n"                                                        \
		"var stcip3 = %d;\n"                                                        \
		"var stcip4 = %d;\n"
#define STCMS_JAVASCRIPT_VARS                                                    \
		"var stcms1 = %d;\n"                                                        \
		"var stcms2 = %d;\n"                                                        \
		"var stcms3 = %d;\n"                                                        \
		"var stcms4 = %d;\n"
#define STCGW_JAVASCRIPT_VARS                                                    \
		"var stcgw1 = %d;\n"                                                        \
		"var stcgw2 = %d;\n"                                                        \
		"var stcgw3 = %d;\n"                                                        \
		"var stcgw4 = %d;\n"
#define LOGIN_JAVASCRIPT_VARS                                                    \
		"var permit = %d;\n"														\
		"var counter = %d;\n"
#define SCAN_JAVASCRIPT_VARS    "<tr><td><input type=radio name=ssid value='%s' onClick=\"sel(this.value,'%s','-%d','%s')\"></td><td>%s</td><td>-%d</td><td>%s</td></tr>\n"

#define FEEDBACK_JAVASCRIPT_VARS                                                   \
		"var i = %d;\n"
#define JAVASCRIPT_FOOTER                                                     \
		"//--></script>\n"

//*****************************************************************************
//
//! Structure used in mapping numeric IDs to human-readable strings.
//
//*****************************************************************************
typedef struct {
	//
	//! A human readable string related to the identifier found in the ui8Id
	//! field.
	//
	const char *pcString;

	//
	//! An identifier value associated with the string held in the pcString
	//! field.
	//
	uint8_t ui8Id;
} tStringMap;

//*****************************************************************************
//
//! The array used to map between parity identifiers and their human-readable
//! equivalents.
//
//*****************************************************************************
static const tStringMap g_psParityMap[] = { { "None", SERIAL_PARITY_NONE }, {
		"Odd", SERIAL_PARITY_ODD }, { "Even", SERIAL_PARITY_EVEN }, { "Mark",
		SERIAL_PARITY_MARK }, { "Space", SERIAL_PARITY_SPACE } };

#define NUM_PARITY_MAPS         (sizeof(g_psParityMap) / sizeof(tStringMap))

//*****************************************************************************
//
//! The array used to map between wifi and identifiers and their human-readable
//! equivalents.
//
//*****************************************************************************
/*
 static const tStringMap g_psWiFiCMD_Map[] =
 {
 { "enter_CMD", enter_CMD },
 { "exit_CMD", exit_CMD }

 };
 */

#define NUM_WiFiCMD_MAPS         (sizeof(g_psWiFiCMD_Map) / sizeof(tStringMap))

_Bool WiFiCMD_Mode_FLag;  //1, wificmd mode; 0, normal mode;

uint8_t ResponseFromWiFi[ResponseFromWiFi_size]; // save the response data from wifi module;
uint8_t SendCMDBufferToWiFi[20]; // save the command to be sent to wifi module;
uint8_t ResponseFromWiFi_IP[110];
uint8_t ResponseFromWiFi_Mac[40];
uint8_t ResponseFromWiFi_SCAN[ResponseFromWiFi_size]; //500
/*
 tWiFi_CommandSet sWiFi_CommandSet =
 {
 //scan command;
 {{'s','c','a','n','\r','\n'},6},
 //enter CMD mode;
 {{'$','$','$','\0'},4},
 //exit CMD mode;
 {{'e','x','i','t','\r','\n'},6}

 };
 */

//*****************************************************************************
//
//! The array used to map between parity identifiers and their human-readable
//! equivalents.
//
//*****************************************************************************
static const tStringMap g_psContactModeMap[] = { { "TCP Server",
		PORT_TELNET_SERVER }, { "TCP Client", PORT_TELNET_CLIENT }, { "UDP",
		PORT_UDP } };

#define NUM_ContactMode_MAPS         (sizeof(g_psContactModeMap) / sizeof(tStringMap))
static const tStringMap g_psTransitModeMap[] = { { "RS232", RS232 }, {
		"RS485_2_wires", RS485_2_wires }, { "RS422_RS485_4_wires",
		RS422_RS485_4_wires } };

#define NUM_TransitMode_MAPS         (sizeof(g_psTransitModeMap) / sizeof(tStringMap))

//*****************************************************************************
//0820
//!WIFI MODE
//*****************************************************************************
static const tStringMap g_psWifiModeMap[] = { { "AP Mode", AP_Mode }, {
		"STA Mode", STA_Mode } };

#define NUM_WifiMode_MAPS         (sizeof(g_psWifiModeMap) / sizeof(tStringMap))
//*****************************************************************************
//0907
//!data channel select
//*****************************************************************************
static const tStringMap g_psNetCHMap[] = { { "LAN - COM", LAN_COM }, {
		"WLAN - COM", WLAN_COM }, { "Both - COM", Both_COM } };

#define NUM_NetCH_MAPS         (sizeof(g_psNetCHMap) / sizeof(tStringMap))
//*****************************************************************************
//0820
//!security mode
//*****************************************************************************
/*static const tStringMap g_psAP_SecModeMap[] =
 {
 { "Disable", Disable },
 { "WEP 128", WEP_128 },
 { "WPA PSK", WPA_PSK },
 { "WPA2 PSK", WPA2_PSK }
 };

 #define NUM_AP_SecMode_MAPS         (sizeof(g_psAP_SecModeMap) / sizeof(tStringMap))*/
//*****************************************************************************
//0820
//!AP network mode
//*****************************************************************************
/*static const tStringMap g_psAP_NetworkModeMap[] =
 {
 { "11b/g mixed mode", NM11b_g_mixed_mode },
 { "11b only", NM11b_only },
 { "11g only", NM11g_only }
 };

 #define NUM_AP_NetworkMode_MAPS         (sizeof(g_psAP_NetworkModeMap) / sizeof(tStringMap))
 */
//*****************************************************************************
//0820
//!AP channel
//*****************************************************************************
static const tStringMap g_psWifiChannelModeMap[] = {
		{ "AutoSelect", AutoSelect }, { "2.412GHz", FREQ2_412GHz }, {
				"2.417GHz", FREQ2_417GHz }, { "2.422GHz", FREQ2_422GHz }, {
				"2.427GHz", FREQ2_427GHz }, { "2.432GHz", FREQ2_432GHz }, {
				"2.437GHz", FREQ2_437GHz }, { "2.442GHz", FREQ2_442GHz }, {
				"2.447GHz", FREQ2_447GHz }, { "2.452GHz", FREQ2_452GHz }, {
				"2.457GHz", FREQ2_457GHz }, { "2.462GHz", FREQ2_462GHz } };

#define NUM_WifiChannelMode_MAPS         (sizeof(g_psWifiChannelModeMap) / sizeof(tStringMap))
//*****************************************************************************
//chris_0820
//!AP DHCP Type
//*****************************************************************************
static const tStringMap g_psWifiDHCPTypeMap[] = { { "Server", Server }, //0821
		{ "Disable", DHCP_Disable }

};

#define NUM_WifiDHCPType_MAPS         (sizeof(g_psWifiDHCPTypeMap) / sizeof(tStringMap))
//*****************************************************************************
//chris_0820
//!STA security mode
//*****************************************************************************
/*static const tStringMap g_psWifiStaSecModeMap[] =
 {
 { "WEP", STA_WEP },
 { "WPA", STA_WPA_PSK },
 { "WPA2", STA_WPA2_PSK },
 { "OPEN", OPEN }
 };

 #define NUM_WifiStaSecMode_MAPS         (sizeof(g_psWifiStaSecModeMap) / sizeof(tStringMap))
 */
//*****************************************************************************
//chris_0820
//!STA Encryption Type
//*****************************************************************************

//static const tStringMap g_psWifiStaETMap[] =
//{
//   { "TKIP", TKIP },
//   { "AES", AES }
//};
//#define NUM_WifiStaET_MAPS         (sizeof(g_psWifiStaETMap) / sizeof(tStringMap))
//*****************************************************************************
//chris_0820
//!STA WLAN Connection Type
//*****************************************************************************
static const tStringMap g_psWifiStaCTMap[] = { { "DHCP", DHCP }, { "Static IP",
		Static_IP } };

#define NUM_WifiStaCT_MAPS         (sizeof(g_psWifiStaCTMap) / sizeof(tStringMap))
//*****************************************************************************
//0822
//RFC2217
//*****************************************************************************
static const tStringMap g_psRFC2217Map[] = { { "Disable", RFC2217_dis }, {
		"Enable", RFC2217_en } };

#define NUM_RFC2217_MAPS         (sizeof(g_psRFC2217Map) / sizeof(tStringMap))
//*****************************************************************************
//modbus
//*****************************************************************************
static const tStringMap g_psModbusMap[] = { { "Disable", Modbus_dis }, {
		"modbusRTU - modbusTCP", Modbus_en }, { "modbusRTU - modbusRTU",
		Modbus_RTU_RTU } };

#define NUM_Modbus_MAPS         (sizeof(g_psModbusMap) / sizeof(tStringMap))
//*****************************************************************************
//
//! The array used to map between flow control identifiers and their
//! human-readable equivalents.
//
//*****************************************************************************
static const tStringMap g_psFlowControlMap[] = { { "None",
		SERIAL_FLOW_CONTROL_NONE }, { "Hardware", SERIAL_FLOW_CONTROL_HW } };

#define NUM_FLOW_CONTROL_MAPS   (sizeof(g_psFlowControlMap) / \
		sizeof(tStringMap))

//*****************************************************************************
//
//! This structure instance contains the factory-default set of configuration
//! parameters for S2E module.
//
//*****************************************************************************
//static const tConfigParameters g_sParametersFactory =0822
const tConfigParameters g_sParametersFactory = {
//
// The sequence number (ui8SequenceNum); this value is not important for
// the copy in SRAM.
//
		0,

		//
		// The CRC (ui8CRC); this value is not important for the copy in SRAM.
		//
		0,

		//
		// The parameter block version number (ui8Version).
		//
		0,

		//
		// Flags (ui8Flags),bit7 0, DHCP;bit7 1, Static IP;
		//
		0x80,

		//
		// Reserved0 (ui8Reserved0).
		//
		//    {
		//     0xFF, 0xFF, 0xFF, 0x0
		//    		'R','e','s',0
		//    },
		// Reserved1 (ui8Reserved1).
		//
		//    {
		//     0xFF, 0xFF, 0xFF, 0x0
		//    		'R','e','s',0
		//    },
		{
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		//
		// Parameters for Port 0 (sPort[0]).
		//

				{

				8, // The number of data bits.

						SERIAL_PARITY_NONE, // The parity (NONE).

						1, // The number of stop bits.

						SERIAL_FLOW_CONTROL_NONE, // The flow control (NONE).

						115200, // The baud rate (ui32BaudRate).

						0, // The telnet session timeout (ui32TelnetTimeout).

						//UDP port no. from 49152 through 65535" */
						2000,// The telnet session listen or local port number
						// (ui16TelnetLocalPort).

						2000,// The telnet session remote port number (when in client mode).
						//
						// The IP address to which a connection will be made when in telnet
						// client mode.  This defaults to an invalid address since it's not
						// sensible to have a default value here.
						//
						0xc0a80164,//192.168.1.100 in the backward order;

						PORT_TELNET_SERVER, //TCP SEVER Flags indicating the operating mode for this port.

						// uiTelnetURL[30]; 192.168.0.1
						{ 0x31, 0x39, 0x32, 0x2E, 0x31, 0x36, 0x38, 0x2E, 0x30,
								0x2E, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00
						//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

						},
						//! Work mode
						0x03,//youren app: 0,UDP; 01,TCP Client;02,UDP Server; 03, TCP Server;

						//			uint8_t uiPackLen[4], changed to uint32_t uiPackLen;
						{ 0x00, 0x00, 0x00, 0x00 }, // 40 bytes;
						//			40,  //change to byte;

						//! Series package time;
						0x00,

						// uint8_t ucTimeCount
						0x00,

						//! 	TCP server type
						0x11,

						RS232, // Transit mode

						RFC2217_dis//	default 1,disable RFC2217 function; 2, enable RFC2217 function;

				},
				//////////////////////////////////////////////////////////////////////////////////////////////////////
				//
				// Parameters for Port 1 (sPort[1]).
				//

				{

				8, // The number of data bits.

						SERIAL_PARITY_NONE, // The parity (NONE).

						1, // The number of stop bits.

						SERIAL_FLOW_CONTROL_NONE, // The flow control (NONE).

						115200, // The baud rate (ui32BaudRate).

						0, // The telnet session timeout (ui32TelnetTimeout).

						//UDP port no. from 49152 through 65535" */
						2001,// The telnet session listen or local port number
						// (ui16TelnetLocalPort).

						2001,// The telnet session remote port number (when in client mode).
						//
						// The IP address to which a connection will be made when in telnet
						// client mode.  This defaults to an invalid address since it's not
						// sensible to have a default value here.
						//
						0xc0a80164,//192.168.1.100 in the backward order;

						PORT_TELNET_SERVER, //TCP SEVER Flags indicating the operating mode for this port.

						// uiTelnetURL[30]; 192.168.0.1
						{ 0x31, 0x39, 0x32, 0x2E, 0x31, 0x36, 0x38, 0x2E, 0x30,
								0x2E, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00
						//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

						},
						//! Work mode
						0x03,//youren app: 0,UDP; 01,TCP Client;02,UDP Server; 03, TCP Server;

						//			uint8_t uiPackLen[4], changed to uint32_t uiPackLen;
						{ 0x00, 0x00, 0x00, 0x00 }, // 40 bytes;
						//			40,  //change to byte;

						//! Series package time;
						0x00,

						// uint8_t ucTimeCount
						0x00,

						//! 	TCP server type
						0x11,

						RS232, // Transit mode

						RFC2217_en	//	default 1,disable RFC2217 function; 2, enable RFC2217 function;

				},
				//////////////////////////////////////////////////////////////////////////////////////////////////////
				//
				// Parameters for Port 2 (sPort[2]).
				//
				{

				8, // The number of data bits.

						SERIAL_PARITY_NONE, // The parity (NONE).

						1, // The number of stop bits.

						SERIAL_FLOW_CONTROL_NONE, // The flow control (NONE).

						115200, // The baud rate (ui32BaudRate).

						0, // The telnet session timeout (ui32TelnetTimeout).

						//UDP port no. from 49152 through 65535" */
						2002,// The telnet session listen or local port number
						// (ui16TelnetLocalPort).

						2002,// The telnet session remote port number (when in client mode).
						//
						// The IP address to which a connection will be made when in telnet
						// client mode.  This defaults to an invalid address since it's not
						// sensible to have a default value here.
						//
						0xc0a80164,//192.168.1.100 in the backward order;

						PORT_TELNET_SERVER, //TCP SEVER Flags indicating the operating mode for this port.

						// uiTelnetURL[30]; 192.168.0.1
						{ 0x31, 0x39, 0x32, 0x2E, 0x31, 0x36, 0x38, 0x2E, 0x30,
								0x2E, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00
						//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

						},
						//! Work mode
						0x03,//youren app: 0,UDP; 01,TCP Client;02,UDP Server; 03, TCP Server;

						//			uint8_t uiPackLen[4], changed to uint32_t uiPackLen;
						{ 0x00, 0x00, 0x00, 0x00 }, // 40 bytes;
						//			40,  //change to byte;

						//! Series package time;
						0x00,

						// uint8_t ucTimeCount
						0x00,

						//! 	TCP server type
						0x11,

						RS232, // Transit mode

						RFC2217_en	//	default 1,disable RFC2217 function; 2, enable RFC2217 function;

				} },

		//
		// Parameters for WiFi module.
		//
		{

		//! MAC Adress
				{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
				//Mode selection;
				{
				AP_Mode, 			// Station mode;
						0, // WiFi Transmit mode：			1 ： transparent, 2 ： ???
						0x04,				//bit6=bit2=1,STA SERVER;AP SERVER
						0xC0A8000B,					// GateWay; 192.168.0.11;
						2000,							//remote port
						2000							//local port
				},
				//AP Setting;
				{
				NM11b_g_mixed_mode,	// AP network mode：				1 ： 11b/g mixed mode, 2 ： 11b only, 3:11g only;
						{ 'm', 'i', 'm', 'a', '1', '2', '3', '4', '5', '6', '7',
								'9', '\0' }, //  uint8_t ui8NetName[MOD_NAME_LEN];
						AutoSelect,	// Frequency/Channel Select：	1 ：AutoSelect, 2 ：2.412GHz(CH1), 3: 2.417GHz(CH1)  … 12;
						Disable,// Security Mode				1 ：Disable, 2：WEP, 3: WPA-PSK  4: WPA2-PSK;
						0xC0A80001,	// Static IP address to use if DHCP is not in use.	192.168.0.1
						0xFFFFFF00,	// The subnet mask to use if DHCP is not in use.							255.255.255.0
						Server,	// DHCP type					2 ：Disable, 1 ：Sever;
						0xC0A80001,						// GateWay; 192.168.0.1;
						{ '1', '2', '3', '4', '5', '6', '7', '8', '\0' }//! Password		rubygirl;
				},

				//Station Setting;
				{ { 'm', 'i', 'm', 'a', '1', '2', '3', '4', '5', '6', '7', '8',
						'\0' }, //  uint8_t ui8NetName[MOD_NAME_LEN];21
						0,// Security Mode				1 ：WEP, 2: WPA-PSK  3: WPA2-PSK, 4: OPEN;
						TKIP,	// Encription type:				1 ：TKIP, 2 ：AES;
						{ '1', '2', '3', '4', '5', '6', '7', '8', '\0' },//! Password		rubygirl; total 15 byte;

						/*NO tWiFiStaionSet.ui32APSubnetMask here？？？？？？？？？？？？？？？？？？？？？*/
						0xFFFFFF00,
						DHCP,// WLAN Station connection type;1 ：DHCP, 2 ：StationIP;
						0x00000000,						// GateWay;
						0x00000000						// Static IP;
				}

		},

		//
		// Module Name (ui8ModName).
		//
		{ 'S', 'e', 'r', 'i', 'e', 's', 'T', 'o', 'E', 't', 'h', 'e', 'r', 'n',
				'e', 't', '\0'

		}, //youren has 16byte for modname;

		//
		// Static IP address (used only if indicated in ui8Flags).
		//
		0xC0A80008,//192.168.1.106 , change to 192.168.0.8

		//
		// Default gateway IP address (used only if static IP is in use).
		//
		0xC0A80001,

		//
		// Subnet mask (used only if static IP is in use).
		//
		0xFFFFFF00,

		//
		// ui8Reserved2 (compiler will pad to the full length)
		//
		//    {
		//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		//    },

		//! Username
		{ 0x61, 0x64, 0x6d, 0x69, 0x6e, 0x00 },		//	6

		//! Password
		{ 0x61, 0x64, 0x6d, 0x69, 0x6e, 0x00 },	//	6			MAC Adress 不用放这里;

		//! UPNP port
		{ 0x20, 0x19 },			//	2		2019

		//! HTTP service port
		{ 0x50, 0x00 },		//	2		5000

		//! Write back what you read;
		0x00,//	1

		//! Device ID
		{ 0x01, 0x00 },			//	2		0100

		//! Device Type
		0x00,//	1

		//! MAC Adress
		{ 0x00, 0x1a, 0xb6, 0x02, 0xb6, 0xdd },	//		6		FF FF FF FF FF FF

		//! Data channel select
		0x01,//  1       1:LAN_COM     2:WLAN_COM   3:both to com enabled;
		// youren debug;
		//   {0x00,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31,0x20,0x00,0x00,0x05,0x01,0x01},

		9600,//wifi module baudrate;
		Modbus_dis, //modbus function options
		0, // counter for power on/off reset; 32bit, 4 byte;
		   //
		   //! Padding to ensure the whole structure is 256 bytes long.
		   //
		   //    {
		   //    		0,0,0,0,0,0,0,0,0,0,
		   //	0,0,0  //13;
		   //	0,0,0,0,0

		//    		'a','a',0
		//    },

		//    {
		//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		//		0x00, 0x00  //, 0x00, 0x00, 0x00, 0x00
		//    }
		{ 0x00, 0x00		//, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		//		0x00, 0x00  //, 0x00, 0x00, 0x00, 0x00
		} };

//*****************************************************************************
//
//! This structure instance contains the run-time set of configuration
//! parameters for S2E module.  This is the active parameter set and may
//! contain changes that are not to be committed to flash.
//
//*****************************************************************************
tConfigParameters g_sParameters;

//*****************************************************************************
//
//! This structure instance points to the most recently saved parameter block
//! in flash.  It can be considered the default set of parameters.
//
//*****************************************************************************
const tConfigParameters *g_psDefaultParameters;

const uint8_t Firmware_Rev[4] = { 0x04, 0x00, 0x0, 0x0 };//0xD0,0x0E,0x12,0x34
//const uint8_t Firmware_Rev[4] = {0xD0,0x0E,0x12,0x34};//0xD0,0x0E,0x12,0x34
//*****************************************************************************
//
//! This structure contains the latest set of parameter committed to flash
//! and is used by the configuration pages to store changes that are to be
//! written back to flash.  Note that g_sParameters may contain other changes
//! which are not to be written so we can't merely save the contents of the
//! active parameter block if the user requests some change to the defaults.
//
//*****************************************************************************
//static tConfigParameters g_sWorkingDefaultParameters;
tConfigParameters g_sWorkingDefaultParameters;

//*****************************************************************************
//
//! This structure instance points to the factory default set of parameters in
//! flash memory.
//
//*****************************************************************************
const tConfigParameters * const g_psFactoryParameters = &g_sParametersFactory;

//*****************************************************************************
//
//! Loads the S2E parameter block from factory-default table.
//!
//! This function is called to load the factory default parameter block.
//!
//! \return None.
//
//*****************************************************************************
void ConfigLoadFactory(void) {
	//
	// Copy the factory default parameter set to the active and working
	// parameter blocks.
	//

	g_sParameters = g_sParametersFactory;
	g_sWorkingDefaultParameters = g_sParametersFactory;

	//   wifimoduledefault();
}

//*****************************************************************************
//
//! Loads the S2E parameter block from flash.
//!
//! This function is called to load the most recently saved parameter block
//! from flash.
//!
//! \return None.
//
//*****************************************************************************
void ConfigLoad(void) {
	uint8_t *pui8Buffer;

	//
	// Get a pointer to the latest parameter block in flash.
	//
	pui8Buffer = EEPROMPBGet();

	//
	// See if a parameter block was found in flash.
	//
	if (pui8Buffer) {
		// 	UARTprintf("gsparameterererere.\n");
		//
		// A parameter block was found so copy the contents to both our
		// active parameter set and the working default set.
		//
		g_sParameters = *(tConfigParameters *) pui8Buffer;
		g_sWorkingDefaultParameters = g_sParameters;

	}
	/*
	 if(g_sParameters.ui32WiFiModuleBaudRate==9600)
	 {
	 // hadware timing for wifi factory resuming;
	 //   	_UR4_SerialSetConfig(9600);
	 wifimoduledefault();
	 } */

}
void delay1ms(void) {
	uint32_t i, j, k;
	for (i = 0; i < 12; i++) {
		for (j = 0; j < 11; j++) {
			for (k = 0; k < 100; k++) {
				SysCtlDelay(1);
			}
		}
	}
}
void delay1us(void) {
	uint32_t i;
	for (i = 0; i < 13; i++)
		SysCtlDelay(1);

}
void delayuS(uint32_t count) {
	uint32_t i;
	for (i = 0; i < count; i++)
		delay1us();
}

void delayms(uint32_t count) {
	uint32_t i;
	for (i = 0; i < count; i++)
		delay1ms();
}

int reveive_ini_wifi_data(char *response) {
	int i, ret, retval;

	for (i = 0; i < MODULE_RX_CMD_RESPONSE_TIMEOUT_INI; i++) // wait for the second response data;
			{
		ret = ReceiveWifiResponse_INI();
		//		ret =  ReceiveWifiResponse();
		if (ret == RN_ERR_OK)	//response has data;
				{
			//	UARTprintf("\nWait for wifi ini data time %dms!\n",i*3+200);
			if (strstr((const char *) ResponseFromWiFi, response) != NULL) // find string ok;
			{
				//special deal with pin9=5 factory reset;
				if (strstr((const char *) ResponseFromWiFi, "PIN9=5") != NULL) // find the last pulse;
				{
					UARTprintf("\nfind p9=5\n");
					if (strstr((const char *) ResponseFromWiFi, "reset") != NULL) {
						//wifi Factory-reset is really ok;
						retval = RN_ERR_OK;
						WiFi_Status_Retval = WiFi_Normal;
						UARTprintf("\nfrst ok\n");
						return (retval);
					} else {
						retval = RN_ERR_Fault;
						//		WiFi_Status_Retval = WiFi_EnterCMD_Error;
						return (retval);
					}

				}

				//UARTprintf("\n/:  %s \n",ResponseFromWiFi);
				retval = RN_ERR_OK;

				WiFi_Status_Retval = WiFi_Normal;
				return (retval);
			} else { // have data, but don't find what we need;
					 //		UARTprintf("Received data, but it's not expected wifi ini data!\n");
					 //		UARTprintf("\nReceived Data:  %s Hex: %X %X %X \n",ResponseFromWiFi,ResponseFromWiFi[0],ResponseFromWiFi[1],ResponseFromWiFi[2]);
				retval = RN_ERR_Fault;
				//		WiFi_Status_Retval = WiFi_EnterCMD_Error;
				return (retval);
				//		continue;
			}

		} else {
			// have no data;
			//		UARTprintf("\nWait for the second response data, still no data!\n");
			vTaskDelay(2 / portTICK_RATE_MS);
			//		retval = ret;
		}

	}
	// timeout, led red flash;
	UARTprintf("\n/time: %d ms \n", i * 3);
	retval = RN_ERR_Fault;
	WiFi_Status_Retval = WiFi_EnterCMD_Error;
	return retval;
}

char *wifi_ini_response_para(uint32_t i) //
{
	//  usnprintf(wifi_ini_response_data, 10, "PIN9=%d",
	//                   i);
	usnprintf(wifi_ini_response_data, 10, "PIN9=");
	wifi_ini_response_data[6] = '\0';
	//	UARTprintf("wifi ini data to be received:%s!\n",wifi_ini_response_data);

	return (wifi_ini_response_data);
}
void EN_AP_Mode(void) {
	//To enable AP mode in hardware, hold GPIO9 high at 3.3V and then Reset (or power
	//  	cycle) the module. The module will then boot up in AP mode with the DHCP server
	//  	enabled.

	//	delayms(100);
	vTaskDelay(100 / portTICK_RATE_MS);
	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_PIN_7);// set wifi gpio9 high;
	//    ROM_GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_0, GPIO_PIN_0);// set wifi reset high;
	vTaskDelay(100 / portTICK_RATE_MS);
	ROM_GPIOPinWrite(WiFi_Reset_Port, WiFi_Reset_Pin, 0);// set wifi reset low;
	vTaskDelay(100 / portTICK_RATE_MS);
	ROM_GPIOPinWrite(WiFi_Reset_Port, WiFi_Reset_Pin, WiFi_Reset_Pin);// set wifi reset high;
	vTaskDelay(100 / portTICK_RATE_MS);
}
void Factory_Reset_WiFi_HW(void) {
	int i, k, j = 0;

	k = 1;
	for (i = 0; i < 2; i++) {
		ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0);// set wifi gpio9 low;
		while (GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) & GPIO_PIN_7 != 0) {
			UARTprintf("\n wait gpio7: 0\n");
		}
		UARTprintf("%d: 0\n", k++);
		// 	delayms(500);
		vTaskDelay(1000 / portTICK_RATE_MS);
		reveive_ini_wifi_data(wifi_ini_response_para(++j));
		ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_PIN_7);// set wifi gpio9 high;
		while (GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7)
				& GPIO_PIN_7 != GPIO_PIN_7) {
			UARTprintf("\n wait gpio7: 1\n");
		}
		UARTprintf("%d: 1\n", k++);
		vTaskDelay(1000 / portTICK_RATE_MS);
		reveive_ini_wifi_data(wifi_ini_response_para(++j));

	}
	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0);	// set wifi gpio9 low;
	while (GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7) & GPIO_PIN_7 != 0) {
		UARTprintf("\n wait gpio7: 0\n");
	}
	UARTprintf("%d: 0\n", k++);
	vTaskDelay(1000 / portTICK_RATE_MS);
}

void wifimoduledefult_func(void) {
	char temp;
	//	uint32_t INT_GPIOC_priority_status;
	//	INT_GPIOC_priority_status = IntPriorityGet(INT_GPIOC);
	//	MAP_IntPrioritySet(INT_GPIOC, 0);
	//wifi gpio9 default high;
	//	IntMasterDisable();
	//	  wifiini_flag = 1;
	Switch_WiFi_to_COM = 0;

	//Wait until send condition ok;
	//Actively read rx ringbuffer until it's empty; and wait tx ringbuffer empty;
	while ((RingBufUsed(&g_sRxBuf[WiFi_Port ]) != 0)
			|| (RingBufUsed(&g_sTxBuf[WiFi_Port ]) != 0)) {
		SerialReceive(WiFi_Port);
	};

	// enable ap mode;
	EN_AP_Mode();

	//need to clear the wifi UR buffer;

	//Wait until send condition ok;
	//Actively read rx ringbuffer until it's empty; and wait tx ringbuffer empty;
	while ((RingBufUsed(&g_sRxBuf[WiFi_Port ]) != 0)
			|| (RingBufUsed(&g_sTxBuf[WiFi_Port ]) != 0)
			|| (wifi_app_CMD_processing != false)) {
		temp = SerialReceive(WiFi_Port);
		//	 	UARTprintf("%c\n",temp);
	};
	Factory_Reset_WiFi_HW();

}

void wifimoduledefault(void) {
	uint8_t j, ret;
	j = 0;
	ret = 1;
	wifimoduledefult_func();
	ret = reveive_ini_wifi_data(wifi_ini_response_para(++j)); //receive pin9=5 inf;
	while (ret != RN_ERR_OK) {
		_UR4_SerialSetConfig(9600);
		vTaskDelay(60 / portTICK_RATE_MS);
		wifimoduledefult_func();
		ret = reveive_ini_wifi_data(wifi_ini_response_para(++j)); //receive pin9=5 inf;
	}
	_UR4_SerialSetConfig(9600);
	// receive "factory reset";
	//    reveive_ini_wifi_data("Factory-reset");
	WiFiReset();
	vTaskDelay(100 / portTICK_RATE_MS); // increase the time after wifireset from 100 to 1000ms;

}

//*****************************************************************************
//
//! Saves the S2E parameter block to flash.
//!
//! This function is called to save the current S2E configuration parameter
//! block to flash memory.
//!
//! \return None.
//
//*****************************************************************************
void ConfigSave(void) {
	uint8_t *pui8Buffer;

	//
	// Save the working defaults parameter block to flash.
	//
	EEPROMPBSave((uint8_t *) &g_sWorkingDefaultParameters);

	//
	// Get the pointer to the most recenly saved buffer.
	// (should be the one we just saved).
	//
	pui8Buffer = EEPROMPBGet();

	//
	// Update the default parameter pointer.
	//
	if (pui8Buffer) {
		g_psDefaultParameters = (tConfigParameters *) pui8Buffer;
		//  UARTprintf("\n have valid flash\n");
	} else {
		g_psDefaultParameters = (tConfigParameters *) g_psFactoryParameters;
	}
}

//*****************************************************************************
//
//! Initializes the configuration parameter block.
//!
//! This function initializes the configuration parameter block.  If the
//! version number of the parameter block stored in flash is older than
//! the current revision, new parameters will be set to default values as
//! needed.
//!
//! \return None.
//
//*****************************************************************************
void ConfigInit(void) {
	uint8_t *pui8Buffer;
	//
	// Verify that the parameter block structure matches the FLASH parameter
	// block size.
	//
	ASSERT(sizeof(tConfigParameters) == FLASH_PB_SIZE)

	//
	// Initialize the flash parameter block driver.
	//
	if (EEPROMPBInit(EEPROM_PB_START, EEPROM_PB_SIZE)) {
		DEBUG_MSG("EEPROM Parameter Block Initalization Failed!");
		while (true) {
		}
	}

	//clear EEPROM manual
//	if (EEPROMMassErase() == 0)
//		UARTprintf("Erase Success\n");
//	if (EEPROMMassErase() == 0)
//		UARTprintf("Erase Success\n");
//	if (EEPROMMassErase() == 0)
//		UARTprintf("Erase Success\n");

	//
	// First, load the factory default values.
	//
	ConfigLoadFactory();
	//save revised default parameters;

	//0822
	//#if revise_code
	//   ConfigSave();
	//#endif

	//
	// Then, if available, load the latest non-volatile set of values.
	//
	ConfigLoad();

	//
	// Get the pointer to the most recently saved buffer.
	//
	pui8Buffer = EEPROMPBGet();

	//
	// Update the default parameter pointer.
	//
	if (pui8Buffer) {
		UARTprintf("DefaultParameters\n");
		g_psDefaultParameters = (tConfigParameters *) pui8Buffer;
	} else {
		UARTprintf("FactoryParameters\n");
		g_psDefaultParameters = (tConfigParameters *) g_psFactoryParameters;
	}

}

//*****************************************************************************
//
//! Configures HTTPD server SSI and CGI capabilities for our configuration
//! forms.
//!
//! This function informs the HTTPD server of the server-side-include tags
//! that we will be processing and the special URLs that are used for
//! CGI processing for the web-based configuration forms.
//!
//! \return None.
//
//*****************************************************************************
void ConfigWebInit(void) {
	//
	// Pass our tag information to the HTTP server.
	//
	http_set_ssi_handler(ConfigSSIHandler, g_pcConfigSSITags,
			NUM_CONFIG_SSI_TAGS);

	//
	// Pass our CGI handlers to the HTTP server.
	//
	http_set_cgi_handlers(g_psConfigCGIURIs, NUM_CONFIG_CGI_URIS);
}

//*****************************************************************************
//
//! \internal
//!
//! Searches a mapping array to find a human-readable description for a
//! given identifier.
//!
//! \param psMap points to an array of \e tStringMap structures which contain
//! the mappings to be searched for the provided identifier.
//! \param ui32Entries contains the number of map entries in the \e psMap
//! array.
//! \param ui8Id is the identifier whose description is to be returned.
//!
//! This function scans the given array of ID/string maps and returns a pointer
//! to the string associated with the /e ui8Id parameter passed.  If the
//! identifier is not found in the map array, a pointer to ``**UNKNOWN**'' is
//! returned.
//!
//! \return Returns a pointer to an ASCII string describing the identifier
//! passed, if found, or ``**UNKNOWN**'' if not found.
//
//*****************************************************************************
static const char *
ConfigMapIdToString(const tStringMap *psMap, uint32_t ui32Entries,
		uint8_t ui8Id) {
	uint32_t ui32Loop;

	//
	// Check each entry in the map array looking for the ID number we were
	// passed.
	//
	for (ui32Loop = 0; ui32Loop < ui32Entries; ui32Loop++) {
		//
		// Does this map entry match?
		//

		if (psMap[ui32Loop].ui8Id == ui8Id) {
			//
			// Yes - return the IDs description string.
			//
			return (psMap[ui32Loop].pcString);
		}
	}

	//
	// If we drop out, the ID passed was not found in the map array so return
	// a default "**UNKNOWN**" string.
	//
	return ("**UNKNOWN**");
}

void SetDataTransCMDtoWiFi(int32_t WFData_Prol) {
	tWiFiCMD pxMessage;
	tWiFiCMD sWiFiCMD;
	if (g_sParameters.datachansel == 2)	//1, lan-com;2; wilfi-com;
		Switch_WiFi_to_COM = 0;
	else if (g_sParameters.datachansel == 3) // both enable;
			{
		Switch_WiFi_to_COM = 0;
		Switch_LAN_to_COM = 0;
	}
	switch (WFData_Prol) {
	case PORT_TELNET_S_C:
		pxMessage.ui8Name = WiFi_DataTrans_TCP_Server;
		break;
	case PORT_TELNET_CLIENT:
		pxMessage.ui8Name = WiFi_DataTrans_TCP_Client;
		break;
	case PORT_UDP:
		pxMessage.ui8Name = WiFi_DataTrans_UDP;
		break;
		//case PORT_TELNET_CLIENT_OLD : pxMessage.ui8Name = WiFi_DataTrans_TCP_Client_OLD;
		//break;
	default:
		break;
	}
	xQueueSendToBack(g_QueWiFiCMD, (void * ) &pxMessage, (portTickType ) 0);
	UARTprintf("send wifi data transmission command to wifi: 0x%x \n",
			pxMessage.ui8Name);
	xQueueReceive(g_QueWiFiDataBack, &sWiFiCMD, portMAX_DELAY);
	if (g_sParameters.datachansel == 2)	//1, lan-com;2; wilfi-com;
		Switch_WiFi_to_COM = 1;
	else if (g_sParameters.datachansel == 3) // both enable;
			{
		Switch_WiFi_to_COM = 1;
		Switch_LAN_to_COM = 1;
	}
}
void SetModnameCMDtoWiFi(uint8_t *Modname) {
	tWiFiCMD pxMessage;
	tWiFiCMD sWiFiCMD;
	if (g_sParameters.datachansel == 2) //1, lan-com;2; wilfi-com;
		Switch_WiFi_to_COM = 0;
	else if (g_sParameters.datachansel == 3) // both enable;
			{
		Switch_WiFi_to_COM = 0;
		Switch_LAN_to_COM = 0;
	}

	pxMessage.ui8Name = WiFi_DeviceID_Altered;

	xQueueSendToBack(g_QueWiFiCMD, (void * ) &pxMessage, (portTickType ) 0);
	//UARTprintf("wifi: %s \n", pxMessage.ui8Name);
	xQueueReceive(g_QueWiFiDataBack, &sWiFiCMD, portMAX_DELAY);
	if (g_sParameters.datachansel == 2) //1, lan-com;2; wilfi-com;
		Switch_WiFi_to_COM = 1;
	else if (g_sParameters.datachansel == 3) // both enable;
			{
		Switch_WiFi_to_COM = 1;
		Switch_LAN_to_COM = 1;
	}
}
//*****************************************************************************
//
//! \internal
//!
//! Updates WiFI parameters.
//!
//! This function changes the WiFi configuration to match the
//! values stored in g_sParameters.sWiFiModule.  On exit, the
//! new parameters will be in effect and g_sParameters.sWiFiModule will have been
//! updated to show the actual parameters in effect (in case any of the
//! supplied parameters are not valid or the actual hardware values differ
//! slightly from the requested value).
//!
//! \return None.
//
//*****************************************************************************
void ConfigUpdateWiFiParameters(bool bWiFi) {
	/*
	 if((g_sParameters.sWiFiModule.sWiFiAPSet.ucAPNetMode 		 != 	sWiFiParams.sWiFiAPSet.ucAPNetMode) ||
	 (g_sParameters.sWiFiModule.sWiFiAPSet.ucDHCPType 		 !=    	sWiFiParams.sWiFiAPSet.ucDHCPType) ||
	 (g_sParameters.sWiFiModule.sWiFiAPSet.ucFrequency_Channel !=    	sWiFiParams.sWiFiAPSet.ucFrequency_Channel) ||
	 (g_sParameters.sWiFiModule.sWiFiAPSet.ucSecurityMode 	 !=    	sWiFiParams.sWiFiAPSet.ucSecurityMode) ||
	 (g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask 	 !=    	sWiFiParams.sWiFiAPSet.ui32APSubnetMask) ||
	 (g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP 			 !=    	sWiFiParams.sWiFiAPSet.ui32AP_IP)||

	 (g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode 		 !=    	sWiFiParams.sWiFiModSel.ucWiFiMode)||
	 (g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiTransMode	 !=    	sWiFiParams.sWiFiModSel.ucWiFiTransMode)||

	 (g_sParameters.sWiFiModule.sWiFiStaionSet.ucEncryptionType	!=    	sWiFiParams.sWiFiStaionSet.ucEncryptionType)||
	 (g_sParameters.sWiFiModule.sWiFiStaionSet.ucSecurityMode  	!=    	sWiFiParams.sWiFiStaionSet.ucSecurityMode)||
	 (g_sParameters.sWiFiModule.sWiFiStaionSet.ucStationConnType 	!=    	sWiFiParams.sWiFiStaionSet.ucStationConnType)||
	 (g_sParameters.sWiFiModule.sWiFiStaionSet.ui32APSubnetMask 	!=    	sWiFiParams.sWiFiStaionSet.ui32APSubnetMask)||
	 (g_sParameters.sWiFiModule.sWiFiStaionSet.ui32AP_IP 			!=    	sWiFiParams.sWiFiStaionSet.ui32AP_IP)||
	 (g_sParameters.sWiFiModule.sWiFiStaionSet.ui8Passwords 		!=    	sWiFiParams.sWiFiStaionSet.ui8Passwords)||

	 (g_sParameters.sWiFiModule.ucWiFiMAC 		!=    	sWiFiParams.ucWiFiMAC)||
	 (g_sParameters.sWiFiModule.ui8NetName 		!=    	sWiFiParams.ui8NetName)
	 )
	 if(bWiFi)
	 {



	 //
	 // Configure WiFi Module with current parameters.

	 // Set the baud rate.
	 //
	 //     SerialSetBaudRate(ui32Port, psPort->ui32BaudRate);

	 //
	 // Set wificmd mode.
	 //
	 //      SerialSetWiFiCMDMode(ui32Port, psPort->uiWiFiCMDMode);  //pony

	 //
	 // Set transit mode.
	 //
	 //      SerialSetTransitMode(ui32Port, psPort->uiTransitMode);  //pony

	 //
	 // Get the current settings.
	 //
	 g_sParameters.sPort[ui32Port].ui32BaudRate = SerialGetBaudRate(ui32Port);
	 g_sParameters.sPort[ui32Port].ui8DataSize = SerialGetDataSize(ui32Port);
	 g_sParameters.sPort[ui32Port].ui8Parity = SerialGetParity(ui32Port);
	 g_sParameters.sPort[ui32Port].ui8StopBits = SerialGetStopBits(ui32Port);
	 g_sParameters.sPort[ui32Port].ui8FlowControl = SerialGetFlowControl(ui32Port);
	 g_sParameters.sPort[ui32Port].uiTransitMode = SerialGetTransitMode(ui32Port); //pony
	 g_sParameters.sPort[ui32Port].uiWiFiCMDMode = SerialGetWiFiCMDMode(ui32Port); //pony
	 }
	 */
}

//*****************************************************************************
//
//! \internal
//!
//! Updates all parameters associated with a single port.
//!
//! \param ui32Port specifies which of the two supported ports to update.
//! Valid values are 0 and 1.
//!
//! This function changes the serial and telnet configuration to match the
//! values stored in g_sParameters.sPort for the supplied port.  On exit, the
//! new parameters will be in effect and g_sParameters.sPort will have been
//! updated to show the actual parameters in effect (in case any of the
//! supplied parameters are not valid or the actual hardware values differ
//! slightly from the requested value).
//!
//! \return None.
//
//*****************************************************************************
void ConfigUpdatePortParameters(uint32_t ui32Port, bool bSerial, bool bTelnet) {
	//
	// Do we have to update the telnet settings?  Note that we need to do this
	// first since the act of initiating a telnet connection resets the serial
	// port settings to defaults.
	//
	if (bTelnet && ((g_sParameters.sPort[ui32Port].ucWorkMode) != PORT_UDP)) {
		//
		// Close any existing connection and shut down the server if required.
		//
		TelnetClose(ui32Port);
		//max client number
		//SetClientNum();//0823
		//
		// Are we to operate as a telnet server?
		//
		if ((g_sParameters.sPort[ui32Port].ucWorkMode == PORT_TELNET_SERVER)) {
			//
			// Yes - start listening on the required port.
			//
			TelnetListen(g_sParameters.sPort[ui32Port].ui16TelnetLocalPort,
					ui32Port);
		} else {
			//
			// No - we are a client so initiate a connection to the desired
			// IP address using the configured ports.
			//
			TelnetOpen(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr,
					g_sParameters.sPort[ui32Port].ui16TelnetRemotePort,
					g_sParameters.sPort[ui32Port].ui16TelnetLocalPort,
					ui32Port);
		}
	}

	//
	// Do we need to update the serial port settings?  We do this if we are
	// told that the serial settings changed or if we just reconfigured the
	// telnet settings (which resets the serial port parameters to defaults as
	// a side effect).
	//
	//   if(bSerial || bTelnet)
	if (bSerial || (bTelnet && ((g_sParameters.sPort[ui32Port].ucWorkMode) !=
	PORT_UDP))) {
		SerialSetCurrent(ui32Port);
	}
}

//*****************************************************************************
//
//! \internal
//!
//! Performs any actions necessary in preparation for a change of IP address.
//!
//! This function is called before ConfigUpdateIPAddress in preparation for a
//! change of IP address or switch between DHCP and StaticIP.
//!
//! \return None.
//
//*****************************************************************************
void ConfigPreUpdateIPAddress(void) {
}

//*****************************************************************************
//
//! \internal
//!
//! Sets the IP address selection mode and associated parameters.
//!
//! This function ensures that the IP address selection mode (static IP or
//! DHCP/AutoIP) is set according to the parameters stored in g_sParameters.
//!
//! \return None.
//
//*****************************************************************************
void ConfigUpdateIPAddress(void) {
	//
	// Change to static/dynamic based on the current settings in the
	// global parameter block.
	//
	if ((g_sParameters.ui8Flags & CONFIG_FLAG_STATICIP) == CONFIG_FLAG_STATICIP) {
		lwIPNetworkConfigChange(g_sParameters.ui32StaticIP,
				g_sParameters.ui32SubnetMask, g_sParameters.ui32GatewayIP,
				IPADDR_USE_STATIC);
	} else {
		lwIPNetworkConfigChange(0, 0, 0, IPADDR_USE_DHCP);
	}
}

//*****************************************************************************
//
//! \internal
//!
//! Performs changes as required to apply all active parameters to the system.
//!
//! \param bUpdateIP is set to \e true to update parameters related to the
//! S2E board's IP address. If \e false, the IP address will remain unchanged.
//!
//! This function ensures that the system configuration matches the values in
//! the current, active parameter block.  It is called after the parameter
//! block has been reset to factory defaults.
//!
//! \return None.
//
//*****************************************************************************
void ConfigUpdateAllParameters(bool bUpdateIP) {
	//
	// Have we been asked to update the IP address along with the other
	// parameters?
	//
	if (bUpdateIP) {
		//
		// Yes - update the IP address selection parameters.
		//
		ConfigPreUpdateIPAddress();
		ConfigUpdateIPAddress();
	}

	//
	// Update the parameters for each of the individual ports.
	//
	ConfigUpdatePortParameters(0, true, true);
	//    ConfigUpdatePortParameters(1, true, true);

	//
	// Update the name that the board publishes to the locator/finder app.
	//
	LocatorAppTitleSet((char *) g_sParameters.ui8ModName);
}

//*****************************************************************************
//
//! \internal
//!
//! Searches the list of parameters passed to a CGI handler and returns the
//! index of a given parameter within that list.
//!
//! \param pcToFind is a pointer to a string containing the name of the
//! parameter that is to be found.
//! \param pcParam is an array of character pointers, each containing the name
//! of a single parameter as encoded in the URI requesting the CGI.
//! \param iNumParams is the number of elements in the pcParam array.
//!
//! This function searches an array of parameters to find the string passed in
//! \e pcToFind.  If the string is found, the index of that string within the
//! \e pcParam array is returned, otherwise -1 is returned.
//!
//! \return Returns the index of string \e pcToFind within array \e pcParam
//! or -1 if the string does not exist in the array.
//
//*****************************************************************************
static int ConfigFindCGIParameter(const char *pcToFind, char *pcParam[],
		int iNumParams) {
	int iLoop;

	//
	// Scan through all the parameters in the array.
	//
	for (iLoop = 0; iLoop < iNumParams; iLoop++) {
		//
		// Does the parameter name match the provided string?
		//
		//UARTprintf("--%s--",pcParam[iLoop]);
		if (strcmp(pcToFind, pcParam[iLoop]) == 0) {
			//
			// We found a match - return the index.
			//
			return (iLoop);
		}
	}

	//
	// If we drop out, the parameter was not found.
	//
	return (-1);
}

static bool ConfigIsValidHexDigit(const char cDigit) {
	if (((cDigit >= '0') && (cDigit <= '9'))
			|| ((cDigit >= 'a') && (cDigit <= 'f'))
			|| ((cDigit >= 'A') && (cDigit <= 'F'))) {
		return (true);
	} else {
		return (false);
	}
}

static uint8_t ConfigHexDigit(const char cDigit) {
	if ((cDigit >= '0') && (cDigit <= '9')) {
		return (cDigit - '0');
	} else {
		if ((cDigit >= 'a') && (cDigit <= 'f')) {
			return ((cDigit - 'a') + 10);
		} else {
			if ((cDigit >= 'A') && (cDigit <= 'F')) {
				return ((cDigit - 'A') + 10);
			}
		}
	}

	//
	// If we get here, we were passed an invalid hex digit so return 0xFF.
	//
	return (0xFF);
}

//*****************************************************************************
//
//! \internal
//!
//! Decodes a single %xx escape sequence as an ASCII character.
//!
//! \param pcEncoded points to the ``%'' character at the start of a three
//! character escape sequence which represents a single ASCII character.
//! \param pcDecoded points to a byte which will be written with the decoded
//! character assuming the escape sequence is valid.
//!
//! This function decodes a single escape sequence of the form ``%xy'' where
//! x and y represent hexadecimal digits.  If each digit is a valid hex digit,
//! the function writes the decoded character to the pcDecoded buffer and
//! returns true, else it returns false.
//!
//! \return Returns \b true on success or \b false if pcEncoded does not point
//! to a valid escape sequence.
//
//*****************************************************************************
static bool ConfigDecodeHexEscape(const char *pcEncoded, char *pcDecoded) {
	if ((pcEncoded[0] != '%') || !ConfigIsValidHexDigit(pcEncoded[1])
			|| !ConfigIsValidHexDigit(pcEncoded[2])) {
		return (false);
	} else {
		*pcDecoded = ((ConfigHexDigit(pcEncoded[1]) * 16)
				+ ConfigHexDigit(pcEncoded[2]));
		return (true);
	}
}

//*****************************************************************************
//
//! \internal
//!
//! Encodes a string for use within an HTML tag, escaping non alphanumeric
//! characters.
//!
//! \param pcDecoded is a pointer to a null terminated ASCII string.
//! \param pcEncoded is a pointer to a storage for the encoded string.
//! \param ui32Len is the number of bytes of storage pointed to by pcEncoded.
//!
//! This function encodes a string, adding escapes in place of any special,
//! non-alphanumeric characters.  If the encoded string is too long for the
//! provided output buffer, the output will be truncated.
//!
//! \return Returns the number of characters written to the output buffer
//! not including the terminating NULL.
//
//*****************************************************************************
static uint32_t ConfigEncodeFormString(const char *pcDecoded, char *pcEncoded,
		uint32_t ui32Len) {
	uint32_t ui32Loop;
	uint32_t ui32Count;

	//
	// Make sure we were not passed a tiny buffer.
	//
	if (ui32Len <= 1) {
		return (0);
	}

	//
	// Initialize our output character counter.
	//
	ui32Count = 0;

	//
	// Step through each character of the input until we run out of data or
	// space to put our output in.
	//
	for (ui32Loop = 0; pcDecoded[ui32Loop] && (ui32Count < (ui32Len - 1));
			ui32Loop++) {
		switch (pcDecoded[ui32Loop]) {
		//
		// Pass most characters without modification.
		//
		default: {
			pcEncoded[ui32Count++] = pcDecoded[ui32Loop];
			break;
		}

		case '\'': {
			ui32Count += usnprintf(&pcEncoded[ui32Count], (ui32Len - ui32Count),
					"&#39;");
			break;
		}
		}
	}

	return (ui32Count);
}

//*****************************************************************************
//
//! \internal
//!
//! Decodes a string encoded as part of an HTTP URI.
//!
//! \param pcEncoded is a pointer to a null terminated string encoded as per
//! RFC1738, section 2.2.
//! \param pcDecoded is a pointer to storage for the decoded, null terminated
//! string.
//! \param ui32Len is the number of bytes of storage pointed to by pcDecoded.
//!
//! This function decodes a string which has been encoded using the method
//! described in RFC1738, section 2.2 for URLs.  If the decoded string is too
//! long for the provided output buffer, the output will be truncated.
//!
//! \return Returns the number of character written to the output buffer, not
//! including the terminating NULL.
//
//*****************************************************************************
static uint32_t ConfigDecodeFormString(const char *pcEncoded, char *pcDecoded,
		uint32_t ui32Len) {
	uint32_t ui32Loop;
	uint32_t ui32Count;
	bool bValid;

	ui32Count = 0;
	ui32Loop = 0;

	//
	// Keep going until we run out of input or fill the output buffer.
	//
	while (pcEncoded[ui32Loop] && (ui32Count < (ui32Len - 1))) {
		switch (pcEncoded[ui32Loop]) {
		//
		// '+' in the encoded data is decoded as a space.
		//
		case '+': {
			pcDecoded[ui32Count++] = ' ';
			ui32Loop++;
			break;
		}

			//
			// '%' in the encoded data indicates that the following 2
			// characters indicate the hex ASCII code of the decoded character.
			//
		case '%': {
			if (pcEncoded[ui32Loop + 1] && pcEncoded[ui32Loop + 2]) {
				//
				// Decode the escape sequence.
				//
				bValid = ConfigDecodeHexEscape(&pcEncoded[ui32Loop],
						&pcDecoded[ui32Count]);

				//
				// If the escape sequence was valid, move to the next
				// output character.
				//
				if (bValid) {
					ui32Count++;
				}

				//
				// Skip past the escape sequence in the encoded string.
				//
				ui32Loop += 3;
			} else {
				//
				// We reached the end of the string partway through an
				// escape sequence so just ignore it and return the number
				// of decoded characters found so far.
				//
				pcDecoded[ui32Count] = '\0';
				return (ui32Count);
			}
			break;
		}

			//
			// For all other characters, just copy the input to the output.
			//
		default: {
			pcDecoded[ui32Count++] = pcEncoded[ui32Loop++];
			break;
		}
		}
	}

	//
	// Terminate the string and return the number of characters we decoded.
	//
	pcDecoded[ui32Count] = '\0';
	return (ui32Count);
}

//*****************************************************************************
//
//! \internal
//!
//! Ensures that a string passed represents a valid decimal number and,
//! if so, converts that number to a long.
//!
//! \param pcValue points to a null terminated string which should contain an
//! ASCII representation of a decimal number.
//! \param pi32Value points to storage which will receive the number
//! represented by pcValue assuming the string is a valid decimal number.
//!
//! This function determines whether or not a given string represents a valid
//! decimal number and, if it does, converts the string into a decimal number
//! which is returned to the caller.
//!
//! \return Returns \b true if the string is a valid representation of a
//! decimal number or \b false if not.

//*****************************************************************************
static bool ConfigCheckDecimalParam(const char *pcValue, int32_t *pi32Value) {
	uint32_t ui32Loop;
	bool bStarted;
	bool bFinished;
	bool bNeg;
	int32_t i32Accum;

	//
	// Check that the string is a valid decimal number.
	//
	bStarted = false;
	bFinished = false;
	bNeg = false;
	ui32Loop = 0;
	i32Accum = 0;

	while (pcValue[ui32Loop]) {
		//
		// Ignore whitespace before the string.
		//
		if (!bStarted) {
			if ((pcValue[ui32Loop] == ' ') || (pcValue[ui32Loop] == '\t')) {
				ui32Loop++;
				continue;
			}

			//
			// Ignore a + or - character as long as we have not started.
			//
			if ((pcValue[ui32Loop] == '+') || (pcValue[ui32Loop] == '-')) {
				//
				// If the string starts with a '-', remember to negate the
				// result.
				//
				bNeg = (pcValue[ui32Loop] == '-') ? true : false;
				bStarted = true;
				ui32Loop++;
			} else {
				//
				// We found something other than whitespace or a sign character
				// so we start looking for numerals now.
				//
				bStarted = true;
			}
		}

		if (bStarted) {
			if (!bFinished) {
				//
				// We expect to see nothing other than valid digit characters
				// here.
				//
				if ((pcValue[ui32Loop] >= '0') && (pcValue[ui32Loop] <= '9')) {
					i32Accum = (i32Accum * 10) + (pcValue[ui32Loop] - '0');
				} else {
					//
					// Have we hit whitespace?  If so, check for no more
					// characters until the end of the string.
					//
					if ((pcValue[ui32Loop] == ' ')
							|| (pcValue[ui32Loop] == '\t')) {
						bFinished = true;
					} else {
						//
						// We got something other than a digit or whitespace so
						// this makes the string invalid as a decimal number.
						//
						return (false);
					}
				}
			} else {
				//
				// We are scanning for whitespace until the end of the string.
				//
				if ((pcValue[ui32Loop] != ' ') && (pcValue[ui32Loop] != '\t')) {
					//
					// We found something other than whitespace so the string
					// is not valid.
					//
					return (false);
				}
			}

			//
			// Move on to the next character in the string.
			//
			ui32Loop++;
		}
	}

	//
	// If we drop out of the loop, the string must be valid.  All we need to do
	// now is negate the accumulated value if the string started with a '-'.
	//
	*pi32Value = bNeg ? -i32Accum : i32Accum;
	return (true);
}

//*****************************************************************************
//
//! \internal
//!
//! Searches the list of parameters passed to a CGI handler for a parameter
//! with the given name and, if found, reads the parameter value as a decimal
//! number.
//!
//! \param pcName is a pointer to a string containing the name of the
//! parameter that is to be found.
//! \param pcParam is an array of character pointers, each containing the name
//! of a single parameter as encoded in the URI requesting the CGI.
//! \param iNumParams is the number of elements in the pcParam array.
//! \param pcValues is an array of values associated with each parameter from
//! the pcParam array.
//! \param pbError is a pointer that will be written to \b true if there is any
//! error during the parameter parsing process (parameter not found, value is
//! not a valid decimal number).
//!
//! This function searches an array of parameters to find the string passed in
//! \e pcName.  If the string is found, the corresponding parameter value is
//! read from array pcValues and checked to make sure that it is a valid
//! decimal number.  If so, the number is returned.  If any error is detected,
//! parameter \e pbError is written to \b true.  Note that \e pbError is NOT
//! written if the parameter is successfully found and validated.  This is to
//! allow multiple parameters to be parsed without the caller needing to check
//! return codes after each individual call.
//!
//! \return Returns the value of the parameter or 0 if an error is detected (in
//! which case \e *pbError will be \b true).
//
//*****************************************************************************
static int32_t ConfigGetCGIParam(const char *pcName, char *pcParams[],
		char *pcValue[], int iNumParams, bool *pbError) {
	int iParam;
	int32_t i32Value;
	bool bRetcode;
	//UARTprintf("iNumParams:%d",iNumParams);
	//
	// Is the parameter we are looking for in the list?
	//
	i32Value = 0;
	iParam = ConfigFindCGIParameter(pcName, pcParams, iNumParams);
	//    UARTprintf("parameters number is %d\n",iNumParams);
	if (iParam != -1) {
		//    	UARTprintf("find the para %s\n\n",pcParams[iParam]);
		//    	UARTprintf("cgi para no. is %d\n",iParam);
		//
		// We found the parameter so now get its value.
		//
		bRetcode = ConfigCheckDecimalParam(pcValue[iParam], &i32Value);
		if (bRetcode) {
			//
			// All is well - return the parameter value.
			//
			//        	UARTprintf("cgi para value is %d\n",i32Value);
			return (i32Value);
		}
	}

	//
	// If we reach here, there was a problem so return 0 and set the error
	// flag.
	//
	int i = 0;
	for (i = 0; i < 20; i++) {

		UARTprintf("%c", *pcName);
		pcName++;
	}
	*pbError = true;
	UARTprintf("can't find the para %s\n\n", pcName);
	return (0);
}

//*****************************************************************************
//
//! \internal
//!
//! Searches the list of parameters passed to a CGI handler for 4 parameters
//! representing an IP address and extracts the IP address defined by them.
//!
//! \param pcName is a pointer to a string containing the base name of the IP
//! address parameters.
//! \param pcParam is an array of character pointers, each containing the name
//! of a single parameter as encoded in the URI requesting the CGI.
//! \param iNumParams is the number of elements in the pcParam array.
//! \param pcValues is an array of values associated with each parameter from
//! the pcParam array.
//! \param pbError is a pointer that will be written to \b true if there is any
//! error during the parameter parsing process (parameter not found, value is
//! not a valid decimal number).
//!
//! This function searches an array of parameters to find four parameters
//! whose names are \e pcName appended with digits 1 - 4.  Each of these
//! parameters is expected to have a value which is a decimal number between
//! 0 and 255.  The parameter values are read and concatenated into an unsigned
//! long representing an IP address with parameter 1 in the leftmost postion.
//!
//! For example, if \e pcName points to string ``ip'', the function will look
//! for 4 CGI parameters named ``ip1'', ``ip2'', ``ip3'' and ``ip4'' and read
//! their values to generate an IP address of the form 0xAABBCCDD where ``AA''
//! is the value of parameter ``ip1'', ``BB'' is the value of ``p2'', ``CC''
//! is the value of ``ip3'' and ``DD'' is the value of ``ip4''.
//!
//! \return Returns the IP address read or 0 if an error is detected (in
//! which case \e *pbError will be \b true).
//
//*****************************************************************************
uint32_t ConfigGetCGIIPAddr(const char *pcName, char *pcParam[],
		char *pcValue[], int iNumParams, bool *pbError) {
	uint32_t ui32IPAddr;
	uint32_t ui32Loop;
	long i32Value;
	char pcVariable[MAX_VARIABLE_NAME_LEN];
	bool bError;

	//
	// Set up for the loop which reads each address element.
	//
	ui32IPAddr = 0;
	bError = false;

	//
	// Look for each of the four variables in turn.
	//
	for (ui32Loop = 1; ui32Loop <= 4; ui32Loop++) {
		//
		// Generate the name of the variable we are looking for next.
		//
		usnprintf(pcVariable, MAX_VARIABLE_NAME_LEN, "%s%d", pcName, ui32Loop);

		//
		// Shift our existing IP address to the left prior to reading the next
		// byte.
		//
		ui32IPAddr <<= 8;

		//
		// Get the next variable and mask it into the IP address.
		//
		i32Value = ConfigGetCGIParam(pcVariable, pcParam, pcValue, iNumParams,
				&bError);
		ui32IPAddr |= ((uint32_t) i32Value & 0xFF);
	}

	//
	// Did we encounter any error while reading the parameters?
	//
	if (bError) {
		//
		// Yes - mark the clients error flag and return 0.
		//
		*pbError = true;
		return (0);
	} else {
		//
		// No - all is well so return the IP address.
		//
		return (ui32IPAddr);
	}
}

//*****************************************************************************
//
//! \internal
//!
//! Performs processing for the URI ``/restart.cgi''.
//chris_web
//*****************************************************************************
static const char *
ConfigRestartHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]) {
	AllStatusLEDoff();
	//	WiFiReset();
	SysCtlReset();

	/*
	 //
	 // Disable all processor interrupts.  Instead of disabling them one at a
	 // time, a direct write to NVIC is done to disable all peripheral
	 // interrupts.
	 //

	 HWREG(NVIC_DIS0) = 0xffffffff;
	 HWREG(NVIC_DIS1) = 0xffffffff;
	 HWREG(NVIC_DIS2) = 0xffffffff;
	 HWREG(NVIC_DIS3) = 0xffffffff;
	 HWREG(NVIC_DIS4) = 0xffffffff;

	 //
	 // Call the ROM UART boot loader.
	 //
	 ROM_UpdateUART(); */

	return (DEV_PAGE_URI);
}

static void SendSTAWebpageCMDtoWiFi(bool WiFiSTASetPage_changed) {
	tWiFiCMD pxMessage;
	tWiFiCMD sWiFiCMD;
	if (WiFiSTASetPage_changed) // has changes for all STA parameters;
	{
		//		if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == 2) // STA Mode;
		//					        {
		if (g_sParameters.sWiFiModule.sWiFiStaionSet.ucStationConnType == 2) // DHCP type is static ip;
				{
			// get wlan; /get ssid,
			// get ip; / ip address /sm address :
			// get mac;
			// "set ip dhcp 4"; /set dhcp;
			// set frequency and security mode ;
			//APMode_Non_DHCP_Mode_Setting
			if (g_sParameters.datachansel == 2)		//1, lan-com;2; wilfi-com;
				Switch_WiFi_to_COM = 0;
			else if (g_sParameters.datachansel == 3) // both enable;
					{
				Switch_WiFi_to_COM = 0;
				Switch_LAN_to_COM = 0;
			}
			vTaskDelay(60 / portTICK_RATE_MS);
#if  Have_WiFi_Module_SKU
			pxMessage.ui8Name = STAMode_Non_DHCP_Mode_Setting;
			//   		xMessage.WiFiPara0.StringParas = vNETWORK_SSID;
			//   		xMessage.WiFiPara1.StringParas = vNETWORK_PASS;
			//			pxMessage = xMessage;
			xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
			UARTprintf("send STAMode_Non_DHCP_Mode_Setting command to wifi: 0x%x \n", pxMessage.ui8Name);
			//get feedback data and send to web;
			xQueueReceive( g_QueWiFi_STA_DataBack, &sWiFiCMD, portMAX_DELAY );
#endif
			if (g_sParameters.datachansel == 2)		//1, lan-com;2; wilfi-com;
				Switch_WiFi_to_COM = 1;
			else if (g_sParameters.datachansel == 3) // both enable;
					{
				Switch_WiFi_to_COM = 1;
				Switch_LAN_to_COM = 1;
			}
		} else if (g_sParameters.sWiFiModule.sWiFiStaionSet.ucStationConnType
				== 1)  // DHCP type is DHCP ip;
				{
			if (g_sParameters.datachansel == 2)  //1, lan-com;2; wilfi-com;
				Switch_WiFi_to_COM = 0;
			else if (g_sParameters.datachansel == 3) // both enable;
					{
				Switch_WiFi_to_COM = 0;
				Switch_LAN_to_COM = 0;
			}
#if  Have_WiFi_Module_SKU
			vTaskDelay( 60/portTICK_RATE_MS );
			// get mac address;
			// get ssid;
			// set frequency and security mode ;
			pxMessage.ui8Name = STAMode_DHCP_Mode_Setting;
			//   		xMessage.WiFiPara0.StringParas = vNETWORK_SSID;
			//    		xMessage.WiFiPara1.StringParas = vNETWORK_PASS;
			//			pxMessage = xMessage;
			xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
			UARTprintf("send STAMode_DHCP_Mode_Setting command to wifi: 0x%x \n", pxMessage.ui8Name);
			//get feedback data and send to web;
			xQueueReceive( g_QueWiFi_STA_DataBack, &sWiFiCMD, portMAX_DELAY );
#endif
			if (g_sParameters.datachansel == 2)		//1, lan-com;2; wilfi-com;
				Switch_WiFi_to_COM = 1;
			else if (g_sParameters.datachansel == 3) // both enable;
					{
				Switch_WiFi_to_COM = 1;
				Switch_LAN_to_COM = 1;
			}
		} else {
			UARTprintf("DHCP type paras error!\n");
		}

		//					        }
		/*
		 else if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == 2) // station mode;
		 {
		 //this cgi make response to ap setting page; so here do nothing;
		 UARTprintf("It should be AP mode, but select Station mode! Nothing is to do!\n");
		 }
		 else {
		 UARTprintf("WiFiMode paras error!\n");
		 }
		 */
	} else // no changes; do nothing;
	{
		UARTprintf("No changes for AP mode!");
	}
	//WiFly_Webaccess_Ready = true;
}

//*****************************************************************************
//
//! \internal
//!
//! Per WiFi AP webpage setting, end commands to WiFi module;
//!
//*****************************************************************************
static void SendAPWebpageCMDtoWiFi(bool WiFiAPSetPage_changed) {
	tWiFiCMD pxMessage;
	tWiFiCMD sWiFiCMD;
	if (WiFiAPSetPage_changed) // has changes for all AP parameters;
	{
		if (g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == 1) // AP Mode;
				{
			if (g_sParameters.sWiFiModule.sWiFiAPSet.ucDHCPType == 2) // DHCP type is static ip;
					{
				// get wlan; /get ssid,
				// get ip; / ip address /sm address :
				// get mac;
				// "set ip dhcp 4"; /set dhcp;
				// set frequency and security mode ;
				//APMode_Non_DHCP_Mode_Setting
				if (g_sParameters.datachansel == 2)	//1, lan-com;2; wilfi-com;
					Switch_WiFi_to_COM = 0;
				else if (g_sParameters.datachansel == 3) // both enable;
						{
					Switch_WiFi_to_COM = 0;
					Switch_LAN_to_COM = 0;
				}
				vTaskDelay(60 / portTICK_RATE_MS);
#if  Have_WiFi_Module_SKU
				pxMessage.ui8Name = APMode_Non_DHCP_Mode_Setting;
				//  		xMessage.WiFiPara0.StringParas = vNETWORK_SSID;
				//   		xMessage.WiFiPara1.StringParas = vNETWORK_PASS;
				//			pxMessage = xMessage;
				xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
				UARTprintf("send APMode_Non_DHCP_Mode_Setting command to wifi: 0x%x \n", pxMessage.ui8Name);
				//get feedback data and send to web;
				xQueueReceive( g_QueWiFi_AP_NoDHCP_DataBack, &sWiFiCMD, portMAX_DELAY );
				//									 xQueueReceive( g_QueWiFi_AP_NoDHCP_DataBack, &sWiFiCMD, 4000 );
#endif
				if (g_sParameters.datachansel == 2)	//1, lan-com;2; wilfi-com;
					Switch_WiFi_to_COM = 1;
				else if (g_sParameters.datachansel == 3) // both enable;
						{
					Switch_WiFi_to_COM = 1;
					Switch_LAN_to_COM = 1;
				}
			} else if (g_sParameters.sWiFiModule.sWiFiAPSet.ucDHCPType == 1) // DHCP type is DHCP ip;
					{
				if (g_sParameters.datachansel == 2)  //1, lan-com;2; wilfi-com;
					Switch_WiFi_to_COM = 0;
				else if (g_sParameters.datachansel == 3) // both enable;
						{
					Switch_WiFi_to_COM = 0;
					Switch_LAN_to_COM = 0;
				}
				vTaskDelay(60 / portTICK_RATE_MS);
				// get mac address;
				// get ssid;
				// set frequency and security mode ;
#if  Have_WiFi_Module_SKU
				pxMessage.ui8Name = APMode_DHCP_Mode_Setting;
				//		xMessage.WiFiPara0.StringParas = vNETWORK_SSID;
				//		xMessage.WiFiPara1.StringParas = vNETWORK_PASS;
				//		pxMessage = xMessage;
				xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
				UARTprintf("send APMode_DHCP_Mode_Setting command to wifi: 0x%x \n", pxMessage.ui8Name);
				//get feedback data and send to web;
				xQueueReceive( g_QueWiFi_APDataBack, &sWiFiCMD, portMAX_DELAY );
				//									 xQueueReceive( g_QueWiFi_APDataBack, &sWiFiCMD, 4000 );
#endif
				if (g_sParameters.datachansel == 2)	//1, lan-com;2; wilfi-com;
					Switch_WiFi_to_COM = 1;
				else if (g_sParameters.datachansel == 3) // both enable;
						{
					Switch_WiFi_to_COM = 1;
					Switch_LAN_to_COM = 1;
				}
			} else {
				UARTprintf("DHCP type paras error!\n");
			}

		} else if (g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == 2) // station mode;
				{
			//this cgi make response to ap setting page; so here do nothing;
			UARTprintf(
					"It should be AP mode, but select Station mode! Nothing is to do!\n");
		} else {
			UARTprintf("WiFiMode paras error!\n");
		}

	} else // no changes; do nothing;
	{
		UARTprintf("No changes for AP mode!");
	}
	//WiFly_Webaccess_Ready = true;
}

//*****************************************************************************
//
//! \internal
//!
//! Performs processing for the URI ``/wifiap.cgi''.
//chris_0820
//*****************************************************************************
static const char *
ConfigWifiAPHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]) {
	//	 int32_t i32Value;
	int iParam;
	bool bParamError, WiFiAPSetPage_changed;
	tWiFiParameters sWiFiParams;
	APPW_NULL = false;

	sWiFiParams.sWiFiAPSet = g_sParameters.sWiFiModule.sWiFiAPSet;
	/*	    // Network Mode
	 //
	 sWiFiParams.sWiFiAPSet.ucAPNetMode = (uint32_t)ConfigGetCGIParam("wifinm", pcParam,
	 pcValue,
	 iNumParams,
	 &bParamError);*/
	// Frequency(Channel)
	//
	sWiFiParams.sWiFiAPSet.ucFrequency_Channel = (uint32_t) ConfigGetCGIParam(
			"wififre", pcParam, pcValue, iNumParams, &bParamError);
	/*	    // AP Security Mode
	 //
	 sWiFiParams.sWiFiAPSet.ucSecurityMode = (uint32_t)ConfigGetCGIParam("wifism", pcParam,
	 pcValue,
	 iNumParams,
	 &bParamError);*/
	// AP DHCP Type
	//
	sWiFiParams.sWiFiAPSet.ucDHCPType = (uint32_t) ConfigGetCGIParam("wifidt",
			pcParam, pcValue, iNumParams, &bParamError);
	//	    if(sWiFiParams.sWiFiAPSet.ucDHCPType == 2)//0822 // DHCP type is static ip;					2 ：Disable, 1 ：Sever;
	//	    {
	// AP IP address
	//
	sWiFiParams.sWiFiAPSet.ui32AP_IP = ConfigGetCGIIPAddr("wip", pcParam,
			pcValue, iNumParams, &bParamError);
	//  UARTprintf("AP IP from web:%s\n",pcValue[iParam]);
	// UARTprintf("AP IP from web:%d\n",sWiFiParams.sWiFiAPSet.ui32AP_IP);
	// AP mask
	//
	sWiFiParams.sWiFiAPSet.ui32APSubnetMask = ConfigGetCGIIPAddr("wmip",
			pcParam, pcValue, iNumParams, &bParamError);
	//	    }
	// AP Gateway
	//
	sWiFiParams.sWiFiAPSet.ui32APGateWay = ConfigGetCGIIPAddr("gtip", pcParam,
			pcValue, iNumParams, &bParamError);
	//	    }
	//netname SSID
	iParam = ConfigFindCGIParameter("wifinn", pcParam, iNumParams);
	if (iParam != -1) {
		ConfigDecodeFormString(pcValue[iParam],
				(char *) sWiFiParams.sWiFiAPSet.ui8NetName,
				NET_NAME_LEN);
	}
	//LocatorAppTitleSet((char *)g_sParameters.ui8ModName);
	//AP password
	//0907
	iParam = ConfigFindCGIParameter("appw", pcParam, iNumParams);
	if (*(pcValue[iParam]) < ' ') {
		APPW_NULL = true;
		strncpy(pcValue[iParam], "0", 2);
		//UARTprintf("\npassword is %s\n",pcValue[iParam]);
	}

	/*for( ;*(pcValue[iParam])!='\0';pcValue[iParam]++)
	 {
	 UARTprintf("password is %x\n",*(pcValue[iParam]));
	 }*/

	if (iParam != -1) {
		ConfigDecodeFormString(pcValue[iParam],
				(char *) sWiFiParams.sWiFiAPSet.ui8SetPasswords, 9);
	}

	if ((sWiFiParams.sWiFiAPSet.ucDHCPType
			!= g_sParameters.sWiFiModule.sWiFiAPSet.ucDHCPType)
			|| (sWiFiParams.sWiFiAPSet.ucFrequency_Channel
					!= g_sParameters.sWiFiModule.sWiFiAPSet.ucFrequency_Channel)
			|| (sWiFiParams.sWiFiAPSet.ucSecurityMode
					!= g_sParameters.sWiFiModule.sWiFiAPSet.ucSecurityMode)
			|| (sWiFiParams.sWiFiAPSet.ui32APSubnetMask
					!= g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask)
			|| (sWiFiParams.sWiFiAPSet.ui32APGateWay
					!= g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay)
			|| (sWiFiParams.sWiFiAPSet.ui32AP_IP
					!= g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP)
			|| (sWiFiParams.sWiFiAPSet.ui8NetName
					!= g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName) || // mac address, no need to change;
			(sWiFiParams.sWiFiAPSet.ui8SetPasswords
					!= g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords) //0907
			)// AP web page has changes;
			{
		WiFiAPSetPage_changed = true;

	} else {

		WiFiAPSetPage_changed = false;
	}

	g_sParameters.sWiFiModule.sWiFiAPSet = sWiFiParams.sWiFiAPSet; // has known the changes; save them;
	g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode = AP_Mode;

	SendAPWebpageCMDtoWiFi(WiFiAPSetPage_changed);
	/*
	 //handle the "default" button
	 i32Value = (uint8_t)ConfigGetCGIParam("default", pcParam, pcValue,
	 iNumParams, &bParamError);

	 if(!bParamError && (i32Value == 1))
	 {
	 */
	//
	// Yes - save these settings as the defaults.
	//
	g_sWorkingDefaultParameters.sWiFiModule.sWiFiAPSet =
			g_sParameters.sWiFiModule.sWiFiAPSet;
	g_sWorkingDefaultParameters.sWiFiModule.sWiFiModSel.ucWiFiMode = AP_Mode;
	//g_sWorkingDefaultParameters.sWiFiModule.sWiFiModSel.ucWiFiMode=g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode;
	ConfigSave();
	//	     }
	if_ap = true;
	if (Wifly_Acc)
		Wifly_AP_STA = true;
	return (CONFIG_CGI_WLANAP);
}

//*****************************************************************************
//
//! \internal
//! Performs processing for the URI ``/chswitch.cgi''.
//0907
//*****************************************************************************
static const char * DataCHSelectHandler(int iIndex, int iNumParams,
		char *pcParam[], char *pcValue[]) {
	int32_t i32Value;
	bool bParamError;
	//tWiFiParameters sWiFiParams;
	uint8_t ch_select;

	//ch_select = g_sParameters.datachansel;

	ch_select = (uint32_t) ConfigGetCGIParam("chsw", pcParam, pcValue,
			iNumParams, &bParamError);
	//        UARTprintf("\n ch_select is: %d \n", ch_select);
	if (g_sParameters.datachansel != ch_select) {
		if (ch_select == 1) //1, lan-com; 2, wifi-com;
				{
			Switch_WiFi_to_COM = 0;
			Switch_LAN_to_COM = 1;
			//UARTprintf("\n current data channel is LAN to COM\n");
		} else if (ch_select == 2) {
			Switch_LAN_to_COM = 0;
			Switch_WiFi_to_COM = 1;
			//UARTprintf("\n current data channel is WiFi to COM\n");
		} else {
			Switch_LAN_to_COM = 1;
			Switch_WiFi_to_COM = 1;

		}

	} else {
		//UARTprintf("\n switch no change\n");
	}
	g_sParameters.datachansel = ch_select;

	i32Value = (uint8_t) ConfigGetCGIParam("default", pcParam, pcValue,
			iNumParams, &bParamError);
	if (!bParamError && (i32Value == 1)) {
		// Yes - save these settings as the defaults.
		g_sWorkingDefaultParameters.datachansel = g_sParameters.datachansel;
		ConfigSave();
	}

	return (CONFIG_CGI_Eth);
}
//*****************************************************************************
//
//! \internal
//!
//! Performs processing for the URI ``/wifimode.cgi''.
//chris_web
//*****************************************************************************
static const char *
ConfigWifiModeHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]) {
	//	int32_t i32Value;
	bool bParamError;
	tWiFiParameters sWiFiParams;
	//	tWiFiCMD pxMessage,sWiFiCMD;
	//Make a copy of the origin, and receive the new web value;
	//In the end, compare with the origin, judge whether has any changes;
	sWiFiParams.sWiFiModSel = g_sParameters.sWiFiModule.sWiFiModSel;
	//WIFI Mode 1.AP mode; 2: station mode;
	sWiFiParams.sWiFiModSel.ucWiFiMode = (uint32_t) ConfigGetCGIParam("wifimd",
			pcParam, pcValue, iNumParams, &bParamError);

	/*	    g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode = sWiFiParams.sWiFiModSel.ucWiFiMode;

	 i32Value = (uint8_t)ConfigGetCGIParam("default", pcParam, pcValue,
	 iNumParams, &bParamError);
	 if(!bParamError && (i32Value == 1))
	 {

	 // Yes - save these settings as the defaults.
	 g_sWorkingDefaultParameters.sWiFiModule.sWiFiModSel.ucWiFiMode =
	 g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode;
	 ConfigSave();
	 return(CONFIG_CGI_MS);
	 }
	 */

	if (sWiFiParams.sWiFiModSel.ucWiFiMode == 1) {
		if_ap = true;
		UARTprintf("\n ws apmode\n");
		return (CONFIG_CGI_WLANAP);
	} else
		return (CONFIG_CGI_WLANSTA);

}

//*****************************************************************************
//
//! \internal
//!
//! Performs processing for the URI ``/wifitm.cgi''.
//*****************************************************************************
static const char *
WiFiTransModeHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]) {
	int32_t iParam;
	//	int32_t i32Value;
	bool bParamError;
	tWiFiModSel sWiFiParams;
	//tWiFiCMD pxMessage;

	sWiFiParams = g_sParameters.sWiFiModule.sWiFiModSel;
	//UARTprintf("**1** %d\n",(sWiFiParams.WiFiTransPcolFlag &0x07));

	// AP Transmission Procotol
	//
	iParam = (uint32_t) ConfigGetCGIParam("wifiprol", pcParam, pcValue,
			iNumParams, &bParamError);
	sWiFiParams.WiFiTransPcolFlag &= ~0x07;	//flush AP flag
	//UARTprintf("**2** %d\n",(sWiFiParams.WiFiTransPcolFlag));
	//for TCP Server mode
	if (iParam == PORT_TELNET_S_C) {
		WiFly_Webaccess_Ready = true;
		//UARTprintf("get server!\n");

		sWiFiParams.WiFiTransPcolFlag |= WiFi_SERVER;
		//
		// Telnet local port
		//
		sWiFiParams.ui16DataLocalPort = (uint16_t) ConfigGetCGIParam("wflp",
				pcParam, pcValue, iNumParams, &bParamError);
		//pxMessage.ui8Name = WiFi_DataTrans_TCP_Server;
	}
	//for TCP Client mode
	else if (iParam == PORT_TELNET_CLIENT) {
		WiFly_Webaccess_Ready = false;
		//UARTprintf("get client!\n");
		sWiFiParams.WiFiTransPcolFlag |= WiFi_CLIENT;
		//
		// Telnet local port
		//
		sWiFiParams.ui16DataLocalPort = (uint16_t) ConfigGetCGIParam("wflp",
				pcParam, pcValue, iNumParams, &bParamError);
		//
		// Telnet remote port
		//
		sWiFiParams.ui16DataRemotePort = (uint16_t) ConfigGetCGIParam("wfrp",
				pcParam, pcValue, iNumParams, &bParamError);
		//
		// Telnet IP address
		//
		sWiFiParams.ui32DataIPAddr = ConfigGetCGIIPAddr("wfip", pcParam,
				pcValue, iNumParams, &bParamError);
		//if client setting changed?
		//1127
		/*	    	 if((sWiFiParams.ui16DataLocalPort == g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort )&&
		 (sWiFiParams.ui16DataRemotePort == g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort )&&
		 (sWiFiParams.ui32DataIPAddr == g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr ))

		 iParam = PORT_TELNET_CLIENT_OLD;*/
	}
	//for UDP
	else if (iParam == PORT_UDP) {
		WiFly_Webaccess_Ready = false;
		//UARTprintf("get udp!\n");
		sWiFiParams.WiFiTransPcolFlag |= WiFi_UDP;
		//
		// Telnet local port
		//
		sWiFiParams.ui16DataLocalPort = (uint16_t) ConfigGetCGIParam("wflp",
				pcParam, pcValue, iNumParams, &bParamError);

		//
		// Telnet remote port
		//
		sWiFiParams.ui16DataRemotePort = (uint16_t) ConfigGetCGIParam("wfrp",
				pcParam, pcValue, iNumParams, &bParamError);

		//
		// Remote IP for UDP
		//
		sWiFiParams.ui32DataIPAddr = ConfigGetCGIIPAddr("wfip", pcParam,
				pcValue, iNumParams, &bParamError);
		//pxMessage.ui8Name = WiFi_DataTrans_UDP;
	}

	//UARTprintf("**4** %d\n",(g_sParameters.sWiFiModule.sWiFiModSel.WiFiTransPcolFlag &0x07));

	/*  if(((sWiFiParams.ui16DataLocalPort != g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort )||
	 (sWiFiParams.ui16DataRemotePort != g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort )||
	 (sWiFiParams.ui32DataIPAddr != g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr )||
	 ((sWiFiParams.WiFiTransPcolFlag & 0x07) != (g_sParameters.sWiFiModule.sWiFiModSel.WiFiTransPcolFlag & 0x07) )
	 ) || (iParam == PORT_TELNET_CLIENT))
	 {*/
	//WiFiDataSetPage_changed = true;
	g_sParameters.sWiFiModule.sWiFiModSel = sWiFiParams;
	SetDataTransCMDtoWiFi(iParam);
	//}
	/*else
	 {
	 WiFiDataSetPage_changed = false;
	 }*/

	//xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
	/*
	 i32Value = (uint8_t)ConfigGetCGIParam("default", pcParam, pcValue,
	 iNumParams, &bParamError);
	 if(!bParamError && (i32Value == 1))
	 {
	 */
	// Yes - save these settings as the defaults.
	g_sWorkingDefaultParameters.sWiFiModule.sWiFiModSel = sWiFiParams;
	ConfigSave();
	//	        }

	//SendAPWebpageCMDtoWiFi(WiFiAPSetPage_changed);
	if (Wifly_Acc)
		Wifly_AP_STA = true;
	return (CONFIG_CGI_MS);
}
//*****************************************************************************
//
//! \internal
//!
//! Performs processing for the URI ``/wifista.cgi''.
//chris_0820
//*****************************************************************************
static const char *
ConfigWifiStaHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]) {
	//	int32_t i32Value;
	bool bParamError, WiFiSTASetPage_changed;
	int iParam;
	tWiFiParameters sWiFiParams;
	sWiFiParams.sWiFiStaionSet = g_sParameters.sWiFiModule.sWiFiStaionSet;
	StaPW_NULL = false;
	/*				//
	 //STA Security Mode
	 //
	 sWiFiParams.sWiFiStaionSet.ucSecurityMode = (uint32_t)ConfigGetCGIParam("wifistasm", pcParam,
	 pcValue,
	 iNumParams,
	 &bParamError);*/

	/*	// Encryption Type
	 //
	 sWiFiParams.sWiFiStaionSet.ucEncryptionType = (uint32_t)ConfigGetCGIParam("wifistaet", pcParam,
	 pcValue,
	 iNumParams,
	 &bParamError);*/

	// WLAN Connection Type
	//
	sWiFiParams.sWiFiStaionSet.ucStationConnType = (uint32_t) ConfigGetCGIParam(
			"wifict", pcParam, pcValue, iNumParams, &bParamError);
	//static mode ,to get IP/MASK/Gateway value
	//0907
	if (sWiFiParams.sWiFiStaionSet.ucStationConnType == 2)// 1 ：DHCP, 2 ：Static;
			{
		// STA IP address
		//
		sWiFiParams.sWiFiStaionSet.sWiFiSTAstaticIP = ConfigGetCGIIPAddr("ip",
				pcParam, pcValue, iNumParams, &bParamError);
		// STA mask
		//
		sWiFiParams.sWiFiStaionSet.ui32STASubnetMask = ConfigGetCGIIPAddr(
				"mkip", pcParam, pcValue, iNumParams, &bParamError);
		// STA gateway
		//
		sWiFiParams.sWiFiStaionSet.sWiFiSTAstaticGateway = ConfigGetCGIIPAddr(
				"gwip", pcParam, pcValue, iNumParams, &bParamError);
	}

	// get user-selected AP SSID
	//
	iParam = ConfigFindCGIParameter("wifiapid", pcParam, iNumParams);
	if (iParam != -1)

	{
		ConfigDecodeFormString(pcValue[iParam],
				(char *) sWiFiParams.sWiFiStaionSet.ui8NetName,
				NET_NAME_LEN);

		//LocatorAppTitleSet((char *)g_sParameters.ui8ModName);
		//bChanged = true;
	}

	// Passwords
	//
	iParam = ConfigFindCGIParameter("wifipw", pcParam, iNumParams);
	if (iParam != -1)

	{
		if (*(pcValue[iParam]) != '\0')
			ConfigDecodeFormString(pcValue[iParam],
					(char *) sWiFiParams.sWiFiStaionSet.ui8InputPasswords,
					WIFISTA_PASSWORD_LEN);
		else {
			strncpy((char *) sWiFiParams.sWiFiStaionSet.ui8InputPasswords, "0",
					2);
			StaPW_NULL = true;
		}

		//LocatorAppTitleSet((char *)g_sParameters.ui8ModName);
		//bChanged = true;
	} else {
		strncpy((char *) sWiFiParams.sWiFiStaionSet.ui8InputPasswords, "0", 2);
		StaPW_NULL = true;
	}

	/************add***************************/
	if ((sWiFiParams.sWiFiStaionSet.sWiFiSTAstaticGateway
			!= g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticGateway)
			|| (sWiFiParams.sWiFiStaionSet.sWiFiSTAstaticIP
					!= g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticIP)
			||
			//(sWiFiParams.sWiFiStaionSet.ucEncryptionType != g_sParameters.sWiFiModule.sWiFiStaionSet.ucEncryptionType )||
			(sWiFiParams.sWiFiStaionSet.ucSecurityMode
					!= g_sParameters.sWiFiModule.sWiFiStaionSet.ucSecurityMode)
			|| (sWiFiParams.sWiFiStaionSet.ucStationConnType
					!= g_sParameters.sWiFiModule.sWiFiStaionSet.ucStationConnType)
			|| (sWiFiParams.sWiFiStaionSet.ui32STASubnetMask
					!= g_sParameters.sWiFiModule.sWiFiStaionSet.ui32STASubnetMask)
			|| // mac address, no need to change;
			(sWiFiParams.sWiFiStaionSet.ui8NetName
					!= g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName)
			|| (sWiFiParams.sWiFiStaionSet.ui8InputPasswords
					!= g_sParameters.sWiFiModule.sWiFiStaionSet.ui8InputPasswords)) // AP web page has changes;
			{
		WiFiSTASetPage_changed = true;

	} else {

		WiFiSTASetPage_changed = false;
	}

	g_sParameters.sWiFiModule.sWiFiStaionSet = sWiFiParams.sWiFiStaionSet;
	SendSTAWebpageCMDtoWiFi(WiFiSTASetPage_changed);
	/*
	 //handle the "default" button  chris_0820
	 i32Value = (uint8_t)ConfigGetCGIParam("default", pcParam, pcValue,
	 iNumParams, &bParamError);
	 if(!bParamError && (i32Value == 1))
	 {
	 */
	//
	// Yes - save these settings as the defaults.
	//
	g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode = STA_Mode;
	g_sWorkingDefaultParameters.sWiFiModule.sWiFiStaionSet =
			g_sParameters.sWiFiModule.sWiFiStaionSet;
	g_sWorkingDefaultParameters.sWiFiModule.sWiFiModSel.ucWiFiMode = STA_Mode;
	//g_sWorkingDefaultParameters.sWiFiModule.sWiFiModSel.ucWiFiMode=g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode;

	ConfigSave();
	//	     	        }
	if (Wifly_Acc)
		Wifly_AP_STA = true;

	return (CONFIG_CGI_WLANSTA);
}
//*****************************************************************************
//
//! \internal
//!
//! Performs processing for the URI ``/pw.cgi''.
//chris_web
//*****************************************************************************
static const char *
ConfigLoginCGIHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]) {
	int iParam;
	//bool bChanged;
	uint8_t userName[6];
	uint8_t Password[6];
	uint8_t Old_pw[6];
	uint8_t Confirm_pw[6];
	//
	// We have not made any changes that need written to flash yet.
	feedback = 0;
	//bChanged = false;

	//
	// Find the "account" parameter.
	//
	iParam = ConfigFindCGIParameter("account", pcParam, iNumParams);
	if (iParam != -1)

	{
		ConfigDecodeFormString(pcValue[iParam], (char *) userName,
		ACCOUNT_NAME_LEN);
		//strncpy((char *)g_sParameters.ui8Username,
		//      (char *)g_sWorkingDefaultParameters.ui8Username, ACCOUNT_NAME_LEN);
		// LocatorAppTitleSet((char *)g_sParameters.ui8ModName);
		//bChanged = true;
	}

	//for v1 debug #8
	// Find the "opw" parameter.
	//old password
	iParam = ConfigFindCGIParameter("opw", pcParam, iNumParams);
	if (iParam != -1) {
		ConfigDecodeFormString(pcValue[iParam], (char *) Old_pw,
		ACCOUNT_PASSWORD_LEN);
	}

	//new password
	// Find the "pw" parameter.
	//
	iParam = ConfigFindCGIParameter("pw", pcParam, iNumParams);
	if (iParam != -1) {
		ConfigDecodeFormString(pcValue[iParam], (char *) Password,
		ACCOUNT_PASSWORD_LEN);
	}
	//confirm new password
	// Find the "cpw" parameter.
	//
	iParam = ConfigFindCGIParameter("cpw", pcParam, iNumParams);
	if (iParam != -1) {
		ConfigDecodeFormString(pcValue[iParam], (char *) Confirm_pw,
		ACCOUNT_PASSWORD_LEN);
	}

	if (strncmp((char *) g_sParameters.ui8Username, (char *) userName,
			ACCOUNT_PASSWORD_LEN) == 0)	//judge for username
			{
		if (strncmp((char *) g_sParameters.ui8Passwords, (char *) Old_pw,
				ACCOUNT_PASSWORD_LEN) == 0)	//judge for old password
				{
			if (strncmp((char *) Password, (char *) Confirm_pw,
					ACCOUNT_PASSWORD_LEN) == 0)	//judge for twice new password
					{
				//UARTprintf("matched\n");

				//UARTprintf("new password is %s\n",Password);

				strncpy((char *) g_sParameters.ui8Passwords, (char *) Password,
						ACCOUNT_PASSWORD_LEN);
				strncpy((char *) g_sWorkingDefaultParameters.ui8Passwords,
						(char *) Password, ACCOUNT_PASSWORD_LEN);
				// LocatorAppTitleSet((char *)g_sParameters.ui8ModName);
				//bChanged = true;
				//EEPROMPBSave((uint8_t *)&g_sWorkingDefaultParameters);
				ConfigSave();
				//UARTprintf("new password is %s\n",g_psDefaultParameters->ui8Passwords);
				feedback = 4;
			} else
				feedback = 3;
		} else
			feedback = 2;
	} else
		feedback = 1;

	//
	// Did anything change?
	//
	//if(bChanged)
	// {
	//
	// Yes - save the latest parameters to flash.
	//
	//
	// }

	return (DEV_PAGE_URI);
}

//*****************************************************************************
//!
//! Performs processing for the URI ``/login.cgi''.
//*****************************************************************************
static const char *
LoginHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
	int iParam;
	//bool bChanged;
	uint8_t userName[6];
	uint8_t Password[6];
	login_counter = 1;

	permit = 0;
	//
	// Find the "username" parameter.
	//
	iParam = ConfigFindCGIParameter("userName", pcParam, iNumParams);
	if (iParam != -1)

	{
		ConfigDecodeFormString(pcValue[iParam], (char *) userName, 6);
		//UARTprintf("entered username is %s\n",pcValue[iParam]);
		//UARTprintf("fetched username is %s\n",userName);
	}

	//
	// Find the "password" parameter.
	//
	iParam = ConfigFindCGIParameter("password", pcParam, iNumParams);
	if (iParam != -1) {
		ConfigDecodeFormString(pcValue[iParam], (char *) Password, 6);
		/*UARTprintf("entered password is %s\n",pcValue[iParam]);
		 UARTprintf("fetched password is %s\n",Password);
		 UARTprintf("restored password is %s\n",g_sParameters.ui8Passwords);*/
	}

	//compare
	if (strncmp((char *) userName, (char *) g_sParameters.ui8Username, 6)
			== 0) {
		if (strncmp((char *) Password, (char *) g_sParameters.ui8Passwords, 6)
				== 0) {
			permit = 1;
			logoff_htm = false;
			logoff_shtm = false;
			//UARTprintf("cgi permit is: %d !\n",permit);
			return (CONFIG_CGI_Index);
		} else
			permit = 0;
		logoff_htm = true;
		logoff_shtm = true;
		//UARTprintf("cgi permit is: %d !\n",permit);
		return (CONFIG_CGI_Login);
	}

	permit = 0;
	logoff_htm = true;
	logoff_shtm = true;
	return (CONFIG_CGI_Login);

}
//*****************************************************************************
//!
//! Performs processing for the URI ``/logout.cgi''.
//*****************************************************************************
static const char *
LogoutHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
	/*int iParam;
	 //bool bChanged;
	 uint8_t userName[6];
	 uint8_t Password[6];
	 login_counter = 1;*/
	login_counter = 0;
	permit = 0;
	logoff_htm = true;
	UARTprintf("cgi permit is out: %d !\n", permit);

	return (CONFIG_CGI_Login);
}
//*****************************************************************************
//!
//! Performs processing for the URI ``/search.cgi''.
//*****************************************************************************
static const char *
WiFiSearchHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
	tWiFiCMD pxMessage, sWiFiCMD;
	UARTprintf("begin to search\n\n");

	//APMode_Non_DHCP_Mode_Setting
	if (g_sParameters.datachansel == 2)	//1, lan-com;2; wilfi-com;
		Switch_WiFi_to_COM = 0;
	else if (g_sParameters.datachansel == 3) // both enable;
			{
		Switch_WiFi_to_COM = 0;
		Switch_LAN_to_COM = 0;
	}
	vTaskDelay(60 / portTICK_RATE_MS);
#if  Have_WiFi_Module_SKU
	pxMessage.ui8Name = WiFiSCAN;
	xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
	UARTprintf("send WiFiSCAN command to wifi: 0x%x \n", pxMessage.ui8Name);
	//get feedback data and send to web;
	xQueueReceive( g_QueWiFiScan_DataBack, &sWiFiCMD, portMAX_DELAY );
#endif
	// send data to webpage;

	if (g_sParameters.datachansel == 2) //1, lan-com;2; wilfi-com;
		Switch_WiFi_to_COM = 1;
	else if (g_sParameters.datachansel == 3) // both enable;
			{
		Switch_WiFi_to_COM = 1;
		Switch_LAN_to_COM = 1;
	}
	UARTprintf("scan cmd execute ok.\n");

	return (CONFIG_CGI_APTable);
}
//*****************************************************************************
//!
//! Performs processing for the URI ``/searchWifi.cgi''.
//get user-selected AP
//*****************************************************************************
static const char *
SelectAPHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
	char *ssid_get;
	int i;
	int iParam;
	int length;
	bool find = false;

	UARTprintf("1d\n\n");

	iParam = ConfigFindCGIParameter("ssid", pcParam, iNumParams);
	if (iParam != -1) {
		ConfigDecodeFormString(pcValue[iParam], ssid_get,
		NET_NAME_LEN);
		//		            UARTprintf("get the ssid is %s\n\n",ssid_get);
	}
	//		        UARTprintf("get the ssid is %s\n\n",ssid_get);
	for (i = 0; i < Scaned_AP_Num; i++) {
		if (strcmp(ssid_get, (char *) WiFi_SSID_temp[i]) == 0) {

			//UARTprintf("array index is %d\n\n",i);
			length = strlen(ssid_get);

			//			UARTprintf("length is %d\n\n",length);
			//			*(ssid_get+length-2) ='\0';
			//			UARTprintf("modified length is %d\n\n",length);

			strcpy((char *) g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName,
					ssid_get);

			//get security mode
			g_sParameters.sWiFiModule.sWiFiStaionSet.ucSecurityMode =
					securitymode[i];
			if (securitymode[i] == 0) {
				join_AP_pw_needed = 1;
			} else
				join_AP_pw_needed = 0;

			find = true;
			break;
		}
	}
	if (find)
		return (CONFIG_CGI_WLANSTA);
	else
		return (CONFIG_CGI_APTable);

}



//*****************************************************************************
//
//! \internal
//!
//! Performs processing for the URI ``/config.cgi''.
//!
//! \param iIndex is an index into the g_psConfigCGIURIs array indicating which
//! CGI URI has been requested.
//! \param uNumParams is the number of entries in the pcParam and pcValue
//! arrays.
//! \param pcParam is an array of character pointers, each containing the name
//! of a single parameter as encoded in the URI requesting this CGI.
//! \param pcValue is an array of character pointers, each containing the value
//! of a parameter as encoded in the URI requesting this CGI.
//!
//! This function is called whenever the HTTPD server receives a request for
//! URI ``/config.cgi''.  Parameters from the request are parsed into the
//! \e pcParam and \e pcValue arrays such that the parameter name and value
//! are contained in elements with the same index.  The strings contained in
//! \e pcParam and \e pcValue contain all replacements and encodings performed
//! by the browser so the CGI function is responsible for reversing these if
//! required.
//!
//! After processing the parameters, the function returns a fully qualified
//! filename to the HTTPD server which will then open this file and send the
//! contents back to the client in response to the CGI.
//!
//! This specific CGI expects the following parameters:
//!
//! - ``port'' indicates which connection's settings to update.  Valid
//!   values are ``0'' or ``1''.
//! - ``br'' supplies the baud rate.
//! - ``bc'' supplies the number of bits per character.
//! - ``parity'' supplies the parity.  Valid values are ``0'', ``1'', ``2'',
//!   ``3'' or ``4'' with meanings as defined by \b SERIAL_PARITY_xxx in
//!   serial.h.
//! - ``stop'' supplies the number of stop bits.
//! - ``flow'' supplies the flow control setting.  Valid values are ``1'' or
//!   ``3'' with meanings as defined by the \b SERIAL_FLOW_CONTROL_xxx in
//!   serial.h.
//! - ``telnetlp'' supplies the local port number for use by the telnet server.
//! - ``telnetrp'' supplies the remote port number for use by the telnet
//!   client.
//! - ``telnett'' supplies the telnet timeout in seconds.
//! - ``telnetip1'' supplies the first digit of the telnet server IP address.
//! - ``telnetip2'' supplies the second digit of the telnet server IP address.
//! - ``telnetip3'' supplies the third digit of the telnet server IP address.
//! - ``telnetip4'' supplies the fourth digit of the telnet server IP address.
//! - ``tnmode'' supplies the telnet mode, ``0'' for server, ``1'' for client.
//! - ``tnprot'' supplies the telnet protocol, ``0'' for telnet, ``1'' for raw.
//! - ``default'' will be defined with value ``1'' if the settings supplied are
//!   to be saved to flash as the defaults for this port.
//!
//! \return Returns a pointer to a string containing the file which is to be
//! sent back to the HTTPD client in response to this request.
//0823
//*****************************************************************************
static const char *
ConfigCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
	int32_t i32Port;
	//	    int32_t i32Value;
	//	    int8_t rfc2217;
	//int32_t i32TelnetMode;
	int32_t i32TelnetProtocol;
	bool bParamError;
	bool bSerialChanged;
	bool bTelnetChanged;
	tPortParameters sPortParams;
	uint8_t maxclint;
	uint8_t modbus;
	uint8_t UDP_IP[30];
	uint32_t packlen_temp;
	//char *para;
	int i = 0, j, iParam;
	//
	// We have not encountered any parameter errors yet.
	//
	bParamError = false;

	//
	// Get the port number.
	//
	i32Port = ConfigGetCGIParam("port", pcParam, pcValue, iNumParams,
			&bParamError);
	UARTprintf("port:%d\n:",i32Port);

	if (bParamError || ((i32Port != 0) && (i32Port != 1) && (i32Port != 2))) {
		//
		// The connection parameter was invalid.
		//
		return (PARAM_ERROR_RESPONSE);
	}

	//
	// Take a local copy of the current parameter set for this connection
	//
	sPortParams = g_sParameters.sPort[i32Port];
	strcpy((char *) UDP_IP, (char *) g_sParameters.sPort[i32Port].uiTelnetURL);

	//
	// Baud rate
	//
	sPortParams.ui32BaudRate = (uint32_t) ConfigGetCGIParam("br", pcParam,
			pcValue, iNumParams, &bParamError);
	//
	// Transit mode by pony
	//
	sPortParams.uiTransitMode = (uint32_t) ConfigGetCGIParam("tm", pcParam,
			pcValue, iNumParams, &bParamError);

	//RTprintf("Transit Mode received from Web: %d !\n", sPortParams.uiTransitMode);

	//
	// Parity
	//
	sPortParams.ui8Parity = (uint8_t) ConfigGetCGIParam("parity", pcParam,
			pcValue, iNumParams, &bParamError);

	//
	// Stop bits
	//
	sPortParams.ui8StopBits = (uint8_t) ConfigGetCGIParam("stop", pcParam,
			pcValue, iNumParams, &bParamError);

	//
	// Data Size
	//
	sPortParams.ui8DataSize = (uint8_t) ConfigGetCGIParam("bc", pcParam,
			pcValue, iNumParams, &bParamError);
	if (sPortParams.uiTransitMode == 1)
		//
		// Flow control
		//
		sPortParams.ui8FlowControl = (uint8_t) ConfigGetCGIParam("flow",
				pcParam, pcValue, iNumParams, &bParamError);

	//
	// Work mode select
	//
	sPortParams.ucWorkMode = ConfigGetCGIParam("ws", pcParam, pcValue,
			iNumParams, &bParamError);
	/***
	 //
	 // RFC2217
	 //0822
	 rfc2217 =   (uint8_t)ConfigGetCGIParam("RFCEN", pcParam, pcValue,
	 iNumParams, &bParamError);

	 sPortParams.ui8Flags &= ~RFC2217_SET;//0111
	 if(rfc2217 == RFC2217_dis)//RFC2217 disable
	 {
	 sPortParams.ui8Flags &= ~RFC2217_SET;//0111
	 }
	 else if(rfc2217 == RFC2217_en)//RFC2217 Enable
	 {
	 sPortParams.ui8Flags |= RFC2217_SET;//1000
	 }
	 ***/

	/*  if(sPortParams.ucRFC2217 != g_sParameters.sPort[i32Port].ucRFC2217)
	 {
	 g_sParameters.sPort[i32Port].ucRFC2217= sPortParams.ucRFC2217;
	 }*/
	//
	// If we are in telnet client mode, get the additional parameters required.
	//
	if (sPortParams.ucWorkMode == PORT_TELNET_CLIENT) {

		//
		// Telnet timeout
		//
		sPortParams.ui32TelnetTimeout = (uint8_t) ConfigGetCGIParam("telnett",
				pcParam, pcValue, iNumParams, &bParamError);
		//
		// Telnet packing time
		//
		sPortParams.uiPackTime = (uint8_t) ConfigGetCGIParam("packtime",
				pcParam, pcValue, iNumParams, &bParamError);
		//
		// Telnet packing length
		//
		//    	   		    sPortParams.uiPackLen[0] =
		packlen_temp = (uint32_t) ConfigGetCGIParam("packlen", pcParam, pcValue, //change from 8 to 16
				iNumParams, &bParamError);
		if (packlen_temp >= 1024)
			packlen_temp = 1023;
		sPortParams.uiPackLen[3] = packlen_temp & 0xFF;
		sPortParams.uiPackLen[2] = (packlen_temp & 0xFF00) >> 8;
		sPortParams.uiPackLen[1] = (packlen_temp & 0xFF0000) >> 16;
		sPortParams.uiPackLen[0] = (packlen_temp & 0xFF000000) >> 24;

		//
		// Telnet local port
		//
		sPortParams.ui16TelnetLocalPort = (uint16_t) ConfigGetCGIParam(
				"telnetlp", pcParam, pcValue, iNumParams, &bParamError);

		//
		// Telnet remote port
		//
		sPortParams.ui16TelnetRemotePort = (uint16_t) ConfigGetCGIParam(
				"telnetrp", pcParam, pcValue, iNumParams, &bParamError);

		//
		// Telnet IP address
		//
		sPortParams.ui32TelnetIPAddr = ConfigGetCGIIPAddr("telnetip", pcParam,
				pcValue, iNumParams, &bParamError);

		//
		// Telnet protocol
		//
		i32TelnetProtocol = ConfigGetCGIParam("tnprot", pcParam, pcValue,
				iNumParams, &bParamError);
		//
		// Modbus enable
		//
		if (i32TelnetProtocol == 2)		//select Raw
				{
			modbus = (uint32_t) ConfigGetCGIParam("modbus", pcParam, pcValue,
					iNumParams, &bParamError);
			// UARTprintf("get mb is %d\n",modbus);

			if (modbus == Modbus_dis)		//modbus disable
			{
				sPortParams.ui8Flags &= ~Modbus_SET;
			} else if (modbus == Modbus_en)		//modbus Enable
			{
				sPortParams.ui8Flags &= ~Modbus_SET;
				sPortParams.ui8Flags |= Modbus_EN;
				ResetModbusTCPStatemachine(); //reset modbusTCP statemachine;
				modbusRTUState = MODBUS_RTU_STATE_IDLE;
				//		    	    	modbusRTUState2 = MODBUS_RTU_STATE_INIT; // com UR in statemachine;
			} else if (modbus == Modbus_RTU_RTU) {
				sPortParams.ui8Flags &= ~Modbus_SET;
				sPortParams.ui8Flags |= ModbusRTU_RTU;
				ResetModbusTCPStatemachine(); //reset modbusTCP statemachine;
				//		    	    	modbusRTUState3 = MODBUS_RTU_STATE_INIT; // wifi UR in statemachine;
				//		    	    	modbusRTUState2 = MODBUS_RTU_STATE_INIT; // com UR in statemachine;
			}
		} else				//select Telnet,disable the modbus
		{
			sPortParams.ui8Flags &= ~Modbus_SET;
		}

	}
	if (sPortParams.ucWorkMode == PORT_TELNET_SERVER) {

		//
		// Telnet timeout
		//
		sPortParams.ui32TelnetTimeout = (uint8_t) ConfigGetCGIParam("telnett",
				pcParam, pcValue, iNumParams, &bParamError);
		//
		// Telnet packing time
		//
		sPortParams.uiPackTime = (uint8_t) ConfigGetCGIParam("packtime",
				pcParam, pcValue, iNumParams, &bParamError);
		//
		// Telnet packing length
		//
		//	    	   		    sPortParams.uiPackLen =

		packlen_temp = (uint32_t) ConfigGetCGIParam("packlen", pcParam, pcValue,
				iNumParams, &bParamError);
		if (packlen_temp >= 1024)
			packlen_temp = 1023;
		//	    	   		UARTprintf("packlen_temp:%d\n",packlen_temp);
		sPortParams.uiPackLen[3] = packlen_temp & 0xFF;
		sPortParams.uiPackLen[2] = (packlen_temp & 0xFF00) >> 8;
		sPortParams.uiPackLen[1] = (packlen_temp & 0xFF0000) >> 16;
		sPortParams.uiPackLen[0] = (packlen_temp & 0xFF000000) >> 24;
		//	    	   		UARTprintf("uiPackLen0~3:0x%02X,0x%02X,0x%02X,0x%02X,\n",sPortParams.uiPackLen[0],sPortParams.uiPackLen[1],sPortParams.uiPackLen[2],sPortParams.uiPackLen[3]);
		//
		// Telnet local port
		//
		sPortParams.ui16TelnetLocalPort = (uint16_t) ConfigGetCGIParam(
				"telnetlp", pcParam, pcValue, iNumParams, &bParamError);
		/*
		 //
		 // MAX number of clients when as sever
		 //
		 maxclint = (uint8_t)ConfigGetCGIParam("mx", pcParam,pcValue,
		 iNumParams,&bParamError);
		 sPortParams.uiTCPServerType &= 0x0f;
		 sPortParams.uiTCPServerType |= (maxclint << 4);
		 */
		//
		// Telnet protocol
		//
		i32TelnetProtocol = ConfigGetCGIParam("tnprot", pcParam, pcValue,
				iNumParams, &bParamError);
		//
		// Modbus enable
		//
		if (i32TelnetProtocol == 2)		//select Raw
				{
			modbus = (uint32_t) ConfigGetCGIParam("modbus", pcParam, pcValue,
					iNumParams, &bParamError);
			if (modbus == Modbus_dis)		//modbus disable
			{
				sPortParams.ui8Flags &= ~Modbus_SET;
			} else if (modbus == Modbus_en)		//modbus Enable
			{
				sPortParams.ui8Flags &= ~Modbus_SET;
				sPortParams.ui8Flags |= PORT_MODBUS_RTU_TCP;
				ResetModbusTCPStatemachine(); //reset modbusTCP statemachine;
			} else if (modbus == Modbus_RTU_RTU) {
				sPortParams.ui8Flags &= ~Modbus_SET;
				sPortParams.ui8Flags |= ModbusRTU_RTU;
			}
		} else //select Telnet,disable the modbus
		{
			sPortParams.ui8Flags &= ~Modbus_SET;
		}
	}
	if (sPortParams.ucWorkMode == PORT_UDP) {
		//UARTprintf("get udp\n");
		//
		// Telnet local port
		//
		sPortParams.ui16TelnetLocalPort = (uint16_t) ConfigGetCGIParam(
				"telnetlp", pcParam, pcValue, iNumParams, &bParamError);

		//
		// Telnet remote port
		//
		sPortParams.ui16TelnetRemotePort = (uint16_t) ConfigGetCGIParam(
				"telnetrp", pcParam, pcValue, iNumParams, &bParamError);
		//
		//disable modbus
		//
		sPortParams.ui8Flags &= ~Modbus_SET;		//1111 1011

		//
		// Remote IP for UDP
		//
		for (j = 0; j < 4; j++) {
			char para[7] = { "udpip" };
			para[5] = (char) (0x31 + j);
			iParam = ConfigFindCGIParameter(para, pcParam, iNumParams);
			while (*(pcValue[iParam]++)) {
				// 	    UARTprintf("%c\n",*(pcValue[iParam]-1));

				UDP_IP[i++] = *(pcValue[iParam] - 1);

				//UARTprintf("** %c\n",UDP_IP[i-1]);
			}
			UDP_IP[i++] = '.';
		}
		UDP_IP[i - 1] = '\0';
	}

	//
	// We have now read all the parameters and made sure that they are valid
	// decimal numbers.  Did we see any errors during this process?
	//
	if (bParamError) {
		//
		// Yes - tell the user there was an error.
		//
		return (PARAM_ERROR_RESPONSE);
	}

	//
	// Update the telnet mode from the parameter we read.
	//
	sPortParams.ui8Flags &= ~PORT_FLAG_PROTOCOL;

	if ((i32TelnetProtocol == 0) && (sPortParams.ucWorkMode != PORT_UDP)) // solve UDP mode, default is telnet mode issue;
		sPortParams.ui8Flags |= PORT_PROTOCOL_TELNET;
	//else if(i32TelnetProtocol == 1)
	//sPortParams.ui8Flags |= PORT_TELNET_CLIENT;
	else
		sPortParams.ui8Flags |= PORT_PROTOCOL_RAW;

	//
	// Update the telnet protocol from the parameter we read.
	//
	//sPortParams.ui8Flags &= ~PORT_FLAG_PROTOCOL;
	//sPortParams.ui8Flags |= (i32TelnetProtocol ? PORT_PROTOCOL_RAW :
	//                       PORT_PROTOCOL_TELNET);

	//
	// Did any of the serial parameters change?
	//
	if ((g_sParameters.sPort[i32Port].ui8DataSize != sPortParams.ui8DataSize)
			|| (g_sParameters.sPort[i32Port].ui8FlowControl
					!= sPortParams.ui8FlowControl)
			|| (g_sParameters.sPort[i32Port].ui8Parity != sPortParams.ui8Parity)
			|| (g_sParameters.sPort[i32Port].ui8StopBits
					!= sPortParams.ui8StopBits)
			|| (g_sParameters.sPort[i32Port].ui32BaudRate
					!= sPortParams.ui32BaudRate)
			|| (g_sParameters.sPort[i32Port].uiTransitMode
					!= sPortParams.uiTransitMode)
			//||   ((g_sParameters.sPort[i32Port].ui8Flags & RFC2217_SET) != (sPortParams.ui8Flags & RFC2217_SET))
			) {
		bSerialChanged = true;
		//	        UARTprintf("Transit Mode received from Web: %d, !\n", sPortParams.uiTransitMode);
		//	        UARTprintf("Transit Mode from current setting: %d, !\n", g_sParameters.sPort[i32Port].uiTransitMode);
	} else {
		bSerialChanged = false;
	}

	//
	// Did any of the telnet parameters change?
	//
	if ((g_sParameters.sPort[i32Port].ui32TelnetIPAddr
			!= sPortParams.ui32TelnetIPAddr)
			|| (g_sParameters.sPort[i32Port].ui32TelnetTimeout
					!= sPortParams.ui32TelnetTimeout)
			|| (g_sParameters.sPort[i32Port].ui16TelnetLocalPort
					!= sPortParams.ui16TelnetLocalPort)
			|| (g_sParameters.sPort[i32Port].ui16TelnetRemotePort
					!= sPortParams.ui16TelnetRemotePort)
			//||(g_sParameters.sPort[i32Port].ui8Flags & PORT_FLAG_CONTACT_MODE) !=
			// (sPortParams.ui8Flags & PORT_FLAG_CONTACT_MODE)
			|| ((g_sParameters.sPort[i32Port].uiTCPServerType & 0xf0)
					!= (sPortParams.uiTCPServerType & 0xf0)
					|| (strcmp((char *) UDP_IP,
							(char *) g_sParameters.sPort[i32Port].uiTelnetURL)
							!= 0)
					|| (g_sParameters.sPort[i32Port].ucWorkMode)
							!= (sPortParams.ucWorkMode))
			|| ((g_sParameters.sPort[i32Port].ui8Flags & PORT_FLAG_PROTOCOL)
					!= (sPortParams.ui8Flags & PORT_FLAG_PROTOCOL))
			|| ((g_sParameters.sPort[i32Port].ui8Flags & Modbus_SET)
					!= (sPortParams.ui8Flags & Modbus_SET))
			|| ((g_sParameters.sPort[i32Port].uiPackTime)
					!= (sPortParams.uiPackTime))
			|| ((g_sParameters.sPort[i32Port].uiPackLen)
					!= (sPortParams.uiPackLen))

			) {
		bTelnetChanged = true;

	} else {
		bTelnetChanged = false;
	}

	//
	// Update the current parameters with the new settings.
	//
	g_sParameters.sPort[i32Port] = sPortParams;
	strcpy((char *) g_sParameters.sPort[i32Port].uiTelnetURL, (char *) UDP_IP);
	//UARTprintf("Transit Mode from current setting: %d, !\n", g_sParameters.sPort[i32Port].uiTransitMode);
	/*
	 //
	 // Were we asked to save this parameter set as the new default?
	 //
	 i32Value = (uint8_t)ConfigGetCGIParam("default", pcParam, pcValue,
	 iNumParams, &bParamError);
	 if(!bParamError && (i32Value == 1))
	 { }
	 */
	//
	// Yes - save these settings as the defaults.
	//
	g_sWorkingDefaultParameters.sPort[i32Port] = g_sParameters.sPort[i32Port];
	// g_sWorkingDefaultParameters.modbus =
	//   g_sParameters.modbus;
	ConfigSave();

	//
	// Apply all the changes to the working parameter set.
	//
	ConfigUpdatePortParameters(i32Port, bSerialChanged, bTelnetChanged);

	//
	// Send the user back to the main status page.
	//

	if (i32Port == 0)
		return (CONFIG_CGI_WLAN2UR_COM0);
	else if (i32Port == 1)
		return (CONFIG_CGI_WLAN2UR_COM1);
	else
		return (CONFIG_CGI_WLAN2UR_COM2);
}

//*****************************************************************************
//
//! \internal
//!
//! Performs processing for the URI ``/ip.cgi''.
//!
//! \param iIndex is an index into the g_psConfigCGIURIs array indicating which
//! CGI URI has been requested.
//! \param uNumParams is the number of entries in the pcParam and pcValue
//! arrays.
//! \param pcParam is an array of character pointers, each containing the name
//! of a single parameter as encoded in the URI requesting this CGI.
//! \param pcValue is an array of character pointers, each containing the value
//! of a parameter as encoded in the URI requesting this CGI.
//!
//! This function is called whenever the HTTPD server receives a request for
//! URI ``/ip.cgi''.  Parameters from the request are parsed into the
//! \e pcParam and \e pcValue arrays such that the parameter name and value
//! are contained in elements with the same index.  The strings contained in
//! \e pcParam and \e pcValue contain all replacements and encodings performed
//! by the browser so the CGI function is responsible for reversing these if
//! required.
//!
//! After processing the parameters, the function returns a fully qualified
//! filename to the HTTPD server which will then open this file and send the
//! contents back to the client in response to the CGI.
//!
//! This specific CGI expects the following parameters:
//!
//! - ``staticip'' contains ``1'' to use a static IP address or ``0'' to use
//!   DHCP/AutoIP.
//! - ``sip1'' contains the first digit of the static IP address.
//! - ``sip2'' contains the second digit of the static IP address.
//! - ``sip3'' contains the third digit of the static IP address.
//! - ``sip4'' contains the fourth digit of the static IP address.
//! - ``gip1'' contains the first digit of the gateway IP address.
//! - ``gip2'' contains the second digit of the gateway IP address.
//! - ``gip3'' contains the third digit of the gateway IP address.
//! - ``gip4'' contains the fourth digit of the gateway IP address.
//! - ``mip1'' contains the first digit of the subnet mask.
//! - ``mip2'' contains the second digit of the subnet mask.
//! - ``mip3'' contains the third digit of the subnet mask.
//! - ``mip4'' contains the fourth digit of the subnet mask.
//!
//! \return Returns a pointer to a string containing the file which is to be
//! sent back to the HTTPD client in response to this request.
//
//*****************************************************************************
static const char *
ConfigIPCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
	//    bool bChanged;
	bool bParamError;
	int32_t i32Mode;
	uint32_t ui32IPAddr;
	uint32_t ui32GatewayAddr;
	uint32_t ui32SubnetMask;
	int32_t i32Value;
	//
	// Nothing has changed and we have seen no errors so far.
	//
	//bChanged = false;
	bParamError = false;
	ui32IPAddr = 0;
	ui32GatewayAddr = 0;
	ui32SubnetMask = 0;

	//
	// Get the IP selection mode.
	//
	i32Mode = ConfigGetCGIParam("staticip", pcParam, pcValue, iNumParams,
			&bParamError);

	//
	// This parameter is required so tell the user there has been a problem if
	// it is not found or is invalid.
	//
	if (bParamError) {
		return (PARAM_ERROR_RESPONSE);
	}

	//
	// If we are being told to use a static IP, read the remaining information.
	//
	if (i32Mode) {
		//
		// Get the static IP address to use.
		//
		ui32IPAddr = ConfigGetCGIIPAddr("sip", pcParam, pcValue, iNumParams,
				&bParamError);
		//
		// Get the gateway IP address to use.
		//
		ui32GatewayAddr = ConfigGetCGIIPAddr("gip", pcParam, pcValue,
				iNumParams, &bParamError);

		ui32SubnetMask = ConfigGetCGIIPAddr("mip", pcParam, pcValue, iNumParams,
				&bParamError);
	}

	//
	// Make sure we read all the required parameters correctly.
	//
	if (bParamError) {
		//
		// Oops - some parameter was invalid.
		//
		return (PARAM_ERROR_RESPONSE);
	}

	//
	// We have all the parameters so determine if anything changed.
	//

	//
	// Did the basic mode change?
	//
	if ((i32Mode && !(g_sParameters.ui8Flags & CONFIG_FLAG_STATICIP))
			|| (!i32Mode && (g_sParameters.ui8Flags & CONFIG_FLAG_STATICIP))) {
		//
		// The mode changed so set the new mode in the parameter block.
		//
		if (!i32Mode) {
			g_sParameters.ui8Flags &= ~CONFIG_FLAG_STATICIP;
		} else {
			g_sParameters.ui8Flags |= CONFIG_FLAG_STATICIP;
		}

		//
		// Remember that something changed.
		//
		//bChanged = true;
	}

	//
	// If we are now using static IP, check for modifications to the IP
	// addresses and mask.
	//
	if (i32Mode) {
		if ((g_sParameters.ui32StaticIP != ui32IPAddr)
				|| (g_sParameters.ui32GatewayIP != ui32GatewayAddr)
				|| (g_sParameters.ui32SubnetMask != ui32SubnetMask)) {
			//
			// Something changed so update the parameter block.
			//
			//bChanged = true;
			g_sParameters.ui32StaticIP = ui32IPAddr;
			g_sParameters.ui32GatewayIP = ui32GatewayAddr;
			g_sParameters.ui32SubnetMask = ui32SubnetMask;
			g_ui32IPAddress = ui32IPAddr;			//**
		}
	}

	// Were we asked to save this parameter set as the new default?
	//
	i32Value = (uint8_t) ConfigGetCGIParam("default", pcParam, pcValue,
			iNumParams, &bParamError);

	if (!bParamError && (i32Value == 1)) {
		//
		// Yes - save these settings as the defaults.
		//
		//UARTprintf("get default\n");
		g_sWorkingDefaultParameters = g_sParameters;
		ConfigSave();
	} else
		g_sWorkingDefaultParameters = g_sParameters;

	ConfigUpdateIPAddress();
	//
	// If anything changed, we need to resave the parameter block.
	//
	/*if(bChanged)
	 {
	 //
	 // Shut down connections in preparation for the IP address change.
	 //
	 ConfigPreUpdateIPAddress();

	 //
	 // Update the working default set and save the parameter block.
	 //
	 g_sWorkingDefaultParameters = g_sParameters;
	 ConfigSave();

	 //
	 // Tell the main loop that a IP address update has been requested.
	 //
	 UARTprintf("old g_ui8UpdateRequired is %d\n",g_ui8UpdateRequired);

	 g_ui8UpdateRequired |= UPDATE_IP_ADDR;

	 UARTprintf("newly g_ui8UpdateRequired is %d\n",g_ui8UpdateRequired);

	 //
	 // Direct the browser to a page warning about the impending IP
	 // address change.
	 //
	 return(IP_UPDATE_RESPONSE);
	 }*/

	//
	// Direct the user back to our miscellaneous settings page.
	//
	return (IP_UPDATE_RESPONSE);
}

//*****************************************************************************
//
//! \internal
//!
//! Performs processing for the URI ``/misc.cgi''.
//!
//! \param iIndex is an index into the g_psConfigCGIURIs array indicating which
//! CGI URI has been requested.
//! \param uNumParams is the number of entries in the pcParam and pcValue
//! arrays.
//! \param pcParam is an array of character pointers, each containing the name
//! of a single parameter as encoded in the URI requesting this CGI.
//! \param pcValue is an array of character pointers, each containing the value
//! of a parameter as encoded in the URI requesting this CGI.
//!
//! This function is called whenever the HTTPD server receives a request for
//! URI ``/misc.cgi''.  Parameters from the request are parsed into the
//! \e pcParam and \e pcValue arrays such that the parameter name and value
//! are contained in elements with the same index.  The strings contained in
//! \e pcParam and \e pcValue contain all replacements and encodings performed
//! by the browser so the CGI function is responsible for reversing these if
//! required.
//!
//! After processing the parameters, the function returns a fully qualified
//! filename to the HTTPD server which will then open this file and send the
//! contents back to the client in response to the CGI.
//!
//! This specific CGI expects the following parameters:
//!
//! - ``modname'' provides a string to be used as the friendly name for the
//!   module.  This is encoded by the browser and must be decoded here.
//!
//! \return Returns a pointer to a string containing the file which is to be
//! sent back to the HTTPD client in response to this request.
//0820
//*****************************************************************************

static const char *
ConfigMiscCGIHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]) {
	int iParam;
	bool bChanged;

	//
	// We have not made any changes that need written to flash yet.
	//
	bChanged = false;

	//
	// Find the "modname" parameter.
	//
	iParam = ConfigFindCGIParameter("modname", pcParam, iNumParams);
	if (iParam != -1) {
		ConfigDecodeFormString(pcValue[iParam],
				(char *) g_sWorkingDefaultParameters.ui8ModName,
				MOD_NAME_LEN);
		strncpy((char *) g_sParameters.ui8ModName,
				(char *) g_sWorkingDefaultParameters.ui8ModName, MOD_NAME_LEN);
		//LocatorAppTitleSet((char *)g_sParameters.ui8ModName);
		bChanged = true;
	}

	//
	// Did anything change?
	//
	if (bChanged) {
		//
		// Yes - save the latest parameters to flash.
		//
		ConfigSave();
	}
	if (!Wifly_Acc)
		SetModnameCMDtoWiFi(g_sParameters.ui8ModName);
	else {
		Wifly_modname_modify = true;
		UARTprintf("\n****////\\\\***\n");
	}

	return (DEV_PAGE_URI);
}

//*****************************************************************************
//
//! \internal
//!
//! Determines whether the supplied configuration parameter structure indicates
//! that changes are required which are likely to change the board IP address.
//!
//! \param psNow is a pointer to the currently active configuration parameter
//!        structure.
//! \param psNew is a pointer to the configuration parameter structure
//!        containing the latest changes, as yet unapplied.
//!
//! This function is called to determine whether applying a set of
//! configuration parameter changes will required (or likely result in) a
//! change in the local board's IP address.  The function is used to determine
//! whether changes can be made immediately or whether they should be deferred
//! until later, giving the system a chance to send a warning to the user's web
//! browser.
//!
//! \return Returns \e true if an IP address change is likely to occur when
//! the parameters are applied or \e false if the address will not change.
//
//*****************************************************************************
static bool ConfigWillIPAddrChange(tConfigParameters const *psNow,
		tConfigParameters const *psNew) {
	//
	// Did we switch between DHCP/AUTOIP and static IP address?
	//
	if ((psNow->ui8Flags & CONFIG_FLAG_STATICIP)
			!= (psNew->ui8Flags & CONFIG_FLAG_STATICIP)) {
		//
		// Mode change will almost certainly result in an IP change.
		//
		return (true);
	}

	//
	// If we are using a static IP, check the IP address, subnet mask and
	// gateway address for changes.
	//
	if (psNew->ui8Flags & CONFIG_FLAG_STATICIP) {
		//
		// Have any of the addresses changed?
		//
		if ((psNew->ui32StaticIP != psNow->ui32StaticIP)
				|| (psNew->ui32GatewayIP != psNow->ui32GatewayIP)
				|| (psNew->ui32SubnetMask != psNow->ui32SubnetMask)) {
			//
			// Yes - either the local IP address or one of the other important
			// IP parameters changed.
			//
			return (true);
		}
	}

	//
	// If we get this far, the IP address, subnet mask or gateway address are
	// not going to change so return false to tell the caller this.
	//
	return (false);
}

//*****************************************************************************
//
//! \internal
//!
//! Performs processing for the URI ``/defaults.cgi''.
//!
//! \param iIndex is an index into the g_psConfigCGIURIs array indicating which
//! CGI URI has been requested.
//! \param uNumParams is the number of entries in the pcParam and pcValue
//! arrays.
//! \param pcParam is an array of character pointers, each containing the name
//! of a single parameter as encoded in the URI requesting this CGI.
//! \param pcValue is an array of character pointers, each containing the value
//! of a parameter as encoded in the URI requesting this CGI.
//!
//! This function is called whenever the HTTPD server receives a request for
//! URI ``/defaults.cgi''.  Parameters from the request are parsed into the
//! \e pcParam and \e pcValue arrays such that the parameter name and value
//! are contained in elements with the same index.  The strings contained in
//! \e pcParam and \e pcValue contain all replacements and encodings performed
//! by the browser so the CGI function is responsible for reversing these if
//! required.
//!
//! After processing the parameters, the function returns a fully qualified
//! filename to the HTTPD server which will then open this file and send the
//! contents back to the client in response to the CGI.
//!
//! This specific CGI expects no specific parameters and any passed are
//! ignored.
//!
//! \return Returns a pointer to a string containing the file which is to be
//! sent back to the HTTPD client in response to this request.
//0822
//*****************************************************************************
static const char *
ConfigDefaultsCGIHandler(int iIndex, int iNumParams, char *pcParam[],
		char *pcValue[]) {
	bool bAddrChange;

//
// Will this update cause an IP address change?
//
	bAddrChange = ConfigWillIPAddrChange(&g_sParameters, g_psFactoryParameters);

//
// Update the working parameter set with the factory defaults.
//

	/*
	 webrestroe_flag = true;
	 ConfigLoadFactory();

	 #if defined(Have_WiFi_Module_SKU)
	 //
	 // Save the new defaults to flash.
	 //
	 WiFiINI();
	 #endif
	 webrestroe_flag = false;

	 */
	restore_call();
//
// If the IP address won't change, we can apply the other changes
// immediately.
//

	if (!bAddrChange) {
		//
		// Apply the various changes required as a result of changing back to
		// the default settings.
		//
		ConfigUpdateAllParameters(false);

		//
		// In this case,we send the usual page back to the browser.
		//
		return (DEFAULT_CGI_RESPONSE);
	} else {
		//
		// The IP address is likely to change so send the browser a warning
		// message and defer the actual update for a couple of seconds by
		// sending a message to the main loop.
		//
		g_ui8UpdateRequired |= UPDATE_ALL;

		//
		// Send back the warning page.
		//
		return (IP_UPDATE_RESPONSE);
	}
}

//*****************************************************************************
//
//! \internal
//!
//! Provides replacement text for each of our configured SSI tags.
//!
//! \param iIndex is an index into the g_pcConfigSSITags array and indicates
//! which tag we are being passed
//! \param pcInsert points to a buffer into which this function should write
//! the replacement text for the tag.  This should be plain text or valid HTML
//! markup.
//! \param iInsertLen is the number of bytes available in the pcInsert buffer.
//! This function must ensure that it does not write more than this or memory
//! corruption will occur.
//!
//! This function is called by the HTTPD server whenever it is serving a page
//! with a ``.ssi'', ``.shtml'' or ``.shtm'' file extension and encounters a
//! tag of the form <!--#tagname--> where ``tagname'' is found in the
//! g_pcConfigSSITags array.  The function writes suitable replacement text to
//! the \e pcInsert buffer.
//!
//! \return Returns the number of bytes written to the pcInsert buffer, not
//! including any terminating NULL.
//
//*****************************************************************************
uint16_t ConfigSSIHandler(int iIndex, char *pcInsert, int iInsertLen) {
	uint32_t ui32Port, iCountMax = 120;
	int iCount = 0;
	int i;
	const char *pcString;
	tWiFiCMD pxMessage, sWiFiCMD;
	static uint8_t counter = 0;
	uint32_t UDPIP;
	uint8_t *packlen;
	uint32_t frame_len_;
	//
	// Which SSI tag are we being asked to provide content for?
	//
	switch (iIndex) {

	//Data transmission protocol for wifi
	case SSI_INDEX_WIFIPROL: {
		pcString = ConfigMapIdToString(g_psContactModeMap, NUM_ContactMode_MAPS,
				WiFiData_ProtocolGet());

		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}
		//Local port about data transmission for wifi
	case SSI_INDEX_WFLP: {
		return (usnprintf(pcInsert, iInsertLen, "%d",
				g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort));
	}
		//Remote port about data transmission for wifi
	case SSI_INDEX_WFRP: {
		return (usnprintf(pcInsert, iInsertLen, "%d",
				g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort));
	}
		//Remote IP about data transmission for wifi
	case SSI_INDEX_WFIP: {
		return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
				(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr >> 24)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr >> 16)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr >> 8)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr >> 0)
						& 0xFF));
	}
		//firmware version
		//
	case SSI_INDEX_REVISION: {
		return (usnprintf(pcInsert, iInsertLen, "Rev.00%d",
				(Firmware_Rev[1] << 8) + Firmware_Rev[0]));
	}
		//
		// The local IP address tag "ipaddr".
		//
	case SSI_INDEX_IPADDR: {
		uint32_t ui32IPAddr;
		//UARTprintf("IP iInsertLen is: %d !\n",iInsertLen);
		ui32IPAddr = lwIPLocalIPAddrGet();
		return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
				((ui32IPAddr >> 0) & 0xFF), ((ui32IPAddr >> 8) & 0xFF),
				((ui32IPAddr >> 16) & 0xFF), ((ui32IPAddr >> 24) & 0xFF)));
	}
		//chris_0820
		//data channel select
		//0907
	case SSI_INDEX_CHSW: {
		pcString = ConfigMapIdToString(g_psNetCHMap, NUM_NetCH_MAPS,
				GetNetChannel());
		//UARTprintf("switch is: %d !\n",g_sParameters.datachansel);
		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}
		//AP password set
		//0907
	case SSI_INDEX_APPW: {
		//                  UARTprintf("AP network name return value to web is: %s !\n",g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName);
		if (APPW_NULL) {
			return NULL;
		} else {
			return (usnprintf(pcInsert, iInsertLen, "%s",
					g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords));
		}
	}
		//STA IP
		//0907
	case SSI_INDEX_STAIP: {
		return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
				(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticIP >> 24)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticIP >> 16)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticIP >> 8)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticIP >> 0)
						& 0xFF));
	}
		//STA MASK
		//0907
	case SSI_INDEX_STAMK: {
		return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
				(g_sParameters.sWiFiModule.sWiFiStaionSet.ui32STASubnetMask
						>> 24) & 0xFF,
				(g_sParameters.sWiFiModule.sWiFiStaionSet.ui32STASubnetMask
						>> 16) & 0xFF,
				(g_sParameters.sWiFiModule.sWiFiStaionSet.ui32STASubnetMask >> 8)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiStaionSet.ui32STASubnetMask >> 0)
						& 0xFF));
	}
		//STA GATEWAY
		//0907
	case SSI_INDEX_STAGW: {
		return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
				(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticGateway
						>> 24) & 0xFF,
				(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticGateway
						>> 16) & 0xFF,
				(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticGateway
						>> 8) & 0xFF,
				(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticGateway
						>> 0) & 0xFF));
	}
		//wifi mode
	case SSI_INDEX_WIFIMD: {
		pcString = ConfigMapIdToString(g_psWifiModeMap, NUM_WifiMode_MAPS,
				GetWifiMode());
		//                	           UARTprintf("wifimode return value to web is: %s !\n",pcString);
		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}
		//AP security mode
		/*case SSI_INDEX_WIFISM:
		 {
		 pcString = ConfigMapIdToString(g_psAP_SecModeMap, NUM_AP_SecMode_MAPS,
		 WIFIGetAPSecMode());
		 //                	            UARTprintf("AP security mode return value to web is: %s !\n",pcString);
		 return(usnprintf(pcInsert, iInsertLen, "%s", pcString));
		 }*/
		//AP Network Mode
		/*case SSI_INDEX_WIFINM:
		 {
		 pcString = ConfigMapIdToString(g_psAP_NetworkModeMap, NUM_AP_NetworkMode_MAPS,
		 WIFIGetAPNetworkMode());
		 //                	           UARTprintf("AP Network Mode return value to web is: %s !\n",pcString);
		 return(usnprintf(pcInsert, iInsertLen, "%s", pcString));
		 }*/
		//AP channel
	case SSI_INDEX_WIFIFRE: {
		pcString = ConfigMapIdToString(g_psWifiChannelModeMap,
				NUM_WifiChannelMode_MAPS, WIFIGetWIFIChannelMode());
		//                	                	            UARTprintf("channel return value to web is: %s !\n",pcString);
		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}
		//AP IP
	case SSI_INDEX_WIFIIP: {
		if (if_ap) {
			UARTprintf("counter is %d \n", counter);

			if (counter % 2 == 0) {
				//UARTprintf("while excecuting,counter is %d \n", counter);
				//get ip;
				if (g_sParameters.datachansel == 2)	//1, lan-com;2; wilfi-com;
					Switch_WiFi_to_COM = 0;
				else if (g_sParameters.datachansel == 3) // both enable;
						{
					Switch_WiFi_to_COM = 0;
					Switch_LAN_to_COM = 0;
				}
				vTaskDelay(60 / portTICK_RATE_MS);
				//		if(g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode == STA_Mode)
				// 			vTaskDelay( 5000/portTICK_RATE_MS ); //for RN171 wifi module delay;
#if  Have_WiFi_Module_SKU
				pxMessage.ui8Name = GetWiFi_IPAdress;
				xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
				UARTprintf("send getting wifi IP page cmd to Que: 0x%x \n", pxMessage.ui8Name);
				//get feedback data and send to web;
				xQueueReceive( g_QueWiFi_IPDataBack, &sWiFiCMD, portMAX_DELAY );
#endif
				if (g_sParameters.datachansel == 2)	//1, lan-com;2; wilfi-com;
					Switch_WiFi_to_COM = 1;
				else if (g_sParameters.datachansel == 3) // both enable;
						{
					Switch_WiFi_to_COM = 1;
					Switch_LAN_to_COM = 1;
				}
				counter++;

				return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
						(WiFiDynamicIP >> 24) & 0xFF,
						(WiFiDynamicIP >> 16) & 0xFF,
						(WiFiDynamicIP >> 8) & 0xFF,
						(WiFiDynamicIP >> 0) & 0xFF));
			} else {
				//       UARTprintf("while  no excecuting,counter is %d \n", counter);
				counter++;
				if_ap = false;
				return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",

				(WiFiDynamicIP >> 24) & 0xFF, (WiFiDynamicIP >> 16) & 0xFF,
						(WiFiDynamicIP >> 8) & 0xFF,
						(WiFiDynamicIP >> 0) & 0xFF));
			}
		} else {
			UARTprintf("not ap \n");
			//get ip;
			if (g_sParameters.datachansel == 2) //1, lan-com;2; wilfi-com;
				Switch_WiFi_to_COM = 0;
			else if (g_sParameters.datachansel == 3) // both enable;
					{
				Switch_WiFi_to_COM = 0;
				Switch_LAN_to_COM = 0;
			}
			vTaskDelay(60 / portTICK_RATE_MS);
#if  Have_WiFi_Module_SKU
			pxMessage.ui8Name = GetWiFi_IPAdress;
			xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
			UARTprintf("STA send getting wifi IP page cmd to Que: 0x%x \n", pxMessage.ui8Name);
			//get feedback data and send to web;
			xQueueReceive( g_QueWiFi_IPDataBack, &sWiFiCMD, portMAX_DELAY );
#endif

			if (g_sParameters.datachansel == 2) //1, lan-com;2; wilfi-com;
				Switch_WiFi_to_COM = 1;
			else if (g_sParameters.datachansel == 3) // both enable;
					{
				Switch_WiFi_to_COM = 1;
				Switch_LAN_to_COM = 1;
			}
			return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
					(WiFiDynamicIP >> 24) & 0xFF, (WiFiDynamicIP >> 16) & 0xFF,
					(WiFiDynamicIP >> 8) & 0xFF, (WiFiDynamicIP >> 0) & 0xFF));
		}
	}
		//AP MASK
	case SSI_INDEX_WIFIMK: {
		//UARTprintf("AP MASK is:%d.%d.%d.%d!\n",(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask >> 24) & 0xFF,
		//              	                	                    (g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask >> 16) & 0xFF,
		//            	                	                    (g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask >> 8) & 0xFF,
		//          	                	                    (g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask >> 0) & 0xFF));
		return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
				(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask >> 24)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask >> 16)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask >> 8)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask >> 0)
						& 0xFF));
	}
		//AP Gateway
	case SSI_INDEX_GTIP: {
		return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
				(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay >> 24)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay >> 16)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay >> 8)
						& 0xFF,
				(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay >> 0)
						& 0xFF));
	}
		//WIFI MAC
	case SSI_INDEX_WIFIMAC: {
		return (usnprintf(pcInsert, iInsertLen, "%02X-%02X-%02X-%02X-%02X-%02X",
				g_sParameters.sWiFiModule.ucWiFiMAC[0],
				g_sParameters.sWiFiModule.ucWiFiMAC[1],
				g_sParameters.sWiFiModule.ucWiFiMAC[2],
				g_sParameters.sWiFiModule.ucWiFiMAC[3],
				g_sParameters.sWiFiModule.ucWiFiMAC[4],
				g_sParameters.sWiFiModule.ucWiFiMAC[5]));
	}
		//AP network name
	case SSI_INDEX_WIFINN: {
		//					UARTprintf("AP network name return value to web is: %s !\n",g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName);
		return (usnprintf(pcInsert, iInsertLen, "%s",
				g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName));
	}
		//AP DHCP Type
	case SSI_INDEX_WIFIDT: {
		pcString = ConfigMapIdToString(g_psWifiDHCPTypeMap,
				NUM_WifiDHCPType_MAPS, WIFIGetWIFIDHCPType());
		//                	UARTprintf("AP DHCP Type return value to web: %s !\n",pcString);
		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}
		//sta security mode
		/*case SSI_INDEX_WIFISTASM:
		 {
		 pcString = ConfigMapIdToString(g_psWifiStaSecModeMap, NUM_WifiStaSecMode_MAPS,
		 WIFIGetSTASecMode());
		 //                	                UARTprintf("sta security mode return value to web: %s !\n",pcString);
		 return(usnprintf(pcInsert, iInsertLen, "%s", pcString));
		 }*/
		//user-selected AP SSID
	case SSI_INDEX_WIFIAPID: {
		return (usnprintf(pcInsert, iInsertLen, "%s",
				g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName));
	}

		/*       //STA Encryption Type
		 case SSI_INDEX_WIFISTAET:
		 {
		 pcString = ConfigMapIdToString(g_psWifiStaETMap, NUM_WifiStaET_MAPS,
		 WIFIGetSTAEType());
		 //                	UARTprintf("STA Encryption Typ return value to web: %s !\n",pcString);
		 return(usnprintf(pcInsert, iInsertLen, "%s", pcString));
		 }*/
		//STA password
	case SSI_INDEX_WIFIPW: {
		if (StaPW_NULL)
			return (usnprintf(pcInsert, iInsertLen, "%s", "\0"));
		else
			return (usnprintf(pcInsert, iInsertLen, "%s",
					g_sParameters.sWiFiModule.sWiFiStaionSet.ui8InputPasswords));
	}
		//STA WLAN Connection Type
	case SSI_INDEX_WIFICT: {
		pcString = ConfigMapIdToString(g_psWifiStaCTMap, NUM_WifiStaCT_MAPS,
				WIFIGetSTAConnType());
		//                	   UARTprintf("STA WLAN Connection Type return value to web: %s !\n",pcString);
		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}
		//RFC2217
		//0822
	case SSI_INDEX_RFC: {
		pcString = ConfigMapIdToString(g_psRFC2217Map, NUM_RFC2217_MAPS,
				GetRFC2217Value());
		//                 	   UARTprintf("RFC2217 return value to web: %s !\n",pcString);
		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}
		//
		// The local MAC address tag "macaddr".
		//
	case SSI_INDEX_MACADDR: {
		uint8_t pucMACAddr[6];

		lwIPLocalMACGet(pucMACAddr);
		return (usnprintf(pcInsert, iInsertLen, "%02X-%02X-%02X-%02X-%02X-%02X",
				pucMACAddr[0], pucMACAddr[1], pucMACAddr[2], pucMACAddr[3],
				pucMACAddr[4], pucMACAddr[5]));
	}

		//
		// These tag are replaced with the current serial port baud rate for
		// their respective ports.
		//
	case SSI_INDEX_P0BR:
	case SSI_INDEX_P1BR:
	case SSI_INDEX_P2BR: {
		if (iIndex == SSI_INDEX_P0BR) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1BR) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}

		//ui32Port = (iIndex == SSI_INDEX_P0BR) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%d",
				SerialGetBaudRate(ui32Port)));
	}
		//
		// These tag are replaced with the current WiFi command mode or normal mode.
		//
		//       case SSI_INDEX_WICMD: //wicmd
		//
		//        {
		//           ui32Port = (iIndex == SSI_INDEX_WICMD) ? 0 : 1;
		//           pcString = ConfigMapIdToString(g_psWiFiCMD_Map, NUM_WiFiCMD_MAPS,
		//          		SerialGetWiFiCMDMode(ui32Port));
		//      UARTprintf("SSI return value to web: %s !\n",pcString);
		//           return(usnprintf(pcInsert, iInsertLen, "%s", pcString));
		//       }

		//
		// These tag are replaced with the current transit mode for the
		// appropriate serial port.
		//
	case SSI_INDEX_P0TM:
	case SSI_INDEX_P1TM:
	case SSI_INDEX_P2TM: {
		if (iIndex == SSI_INDEX_P0TM) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1TM) {
			ui32Port = 1;
		} else if (iIndex == SSI_INDEX_P2TM) {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0TM) ? 0 : 1;
		pcString = ConfigMapIdToString(g_psTransitModeMap, NUM_TransitMode_MAPS,
				SerialGetTransitMode(ui32Port));
		//UARTprintf("port%d SSI return value to web: %s !\n",ui32Port,pcString);
		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}

		//
		// These tag are replaced with the current number of stop bits for
		// the appropriate serial port.
		//
	case SSI_INDEX_P0SB:
	case SSI_INDEX_P1SB:
	case SSI_INDEX_P2SB: {
		if (iIndex == SSI_INDEX_P0SB) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1SB) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//	ui32Port = (iIndex == SSI_INDEX_P0SB) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%d",
				SerialGetStopBits(ui32Port)));
	}

		//
		// These tag are replaced with the current parity mode for the
		// appropriate serial port.
		//
	case SSI_INDEX_P0P:
	case SSI_INDEX_P1P:
	case SSI_INDEX_P2P: {
		if (iIndex == SSI_INDEX_P0P) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1P) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0P) ? 0 : 1;
		pcString = ConfigMapIdToString(g_psParityMap, NUM_PARITY_MAPS,
				SerialGetParity(ui32Port));
		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}

		//
		// These tag are replaced with the current number of bits per character
		// for the appropriate serial port.
		//
	case SSI_INDEX_P0BC:
	case SSI_INDEX_P1BC:
	case SSI_INDEX_P2BC: {
		if (iIndex == SSI_INDEX_P0BC) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1BC) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0BC) ? 0 : 1;
		//	UARTprintf("%d:%d\n",ui32Port,SerialGetDataSize(ui32Port));
		return (usnprintf(pcInsert, iInsertLen, "%d",
				SerialGetDataSize(ui32Port)));
	}

		//
		// These tag are replaced with the current flow control settings for
		// the appropriate serial port.
		//
	case SSI_INDEX_P0FC:
	case SSI_INDEX_P1FC:
	case SSI_INDEX_P2FC: {
		if (iIndex == SSI_INDEX_P0FC) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1FC) {
			ui32Port = 1;
		} else if (iIndex == SSI_INDEX_P2FC) {
			ui32Port = 2;
		}
		//	ui32Port = (iIndex == SSI_INDEX_P0FC) ? 0 : 1;
		pcString = ConfigMapIdToString(g_psFlowControlMap,
		NUM_FLOW_CONTROL_MAPS, SerialGetFlowControl(ui32Port));
		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}

		//
		// These tag are replaced with the timeout for the appropriate
		// port's telnet session.
		//
	case SSI_INDEX_P0TT:
	case SSI_INDEX_P1TT:
	case SSI_INDEX_P2TT: {
		if (iIndex == SSI_INDEX_P0TT) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1TT) {
			ui32Port = 1;
		} else if (iIndex == SSI_INDEX_P2TT) {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0TT) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%d",
				g_sParameters.sPort[ui32Port].ui32TelnetTimeout));
	}

		//
		// These tag are replaced with the paking time for the appropriate
		// port's telnet session.
		//
	case SSI_INDEX_P0PKT:
	case SSI_INDEX_P1PKT:
	case SSI_INDEX_P2PKT: {
		if (iIndex == SSI_INDEX_P0PKT) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1PKT) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0PKT) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%d",
				g_sParameters.sPort[ui32Port].uiPackTime));
	}
		//
		// These tag are replaced with the paking length for the appropriate
		// port's telnet session.
		//
	case SSI_INDEX_P0PKLEN:
	case SSI_INDEX_P1PKLEN:
	case SSI_INDEX_P2PKLEN: {
		if (iIndex == SSI_INDEX_P0PKLEN) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1PKLEN) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0PKLEN) ? 0 : 1;
		packlen = g_sParameters.sPort[ui32Port].uiPackLen;
		frame_len_ = ((packlen[0]) << 24) + ((packlen[1]) << 16)
				+ ((packlen[2]) << 8) + packlen[3];

		return (usnprintf(pcInsert, iInsertLen, "%d", frame_len_));
	}

		//
		// These tag are replaced with the local TCP port number in use by the
		// appropriate port's telnet session.
		//
	case SSI_INDEX_P0TLP:
	case SSI_INDEX_P1TLP:
	case SSI_INDEX_P2TLP: {
		if (iIndex == SSI_INDEX_P0TLP) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1TLP) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0TLP) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%d",
				g_sParameters.sPort[ui32Port].ui16TelnetLocalPort));
	}

		//
		// These tag are replaced with the remote TCP port number in use by
		// the appropriate port's telnet session when in client mode.
		//
	case SSI_INDEX_P0TRP:
	case SSI_INDEX_P1TRP:
	case SSI_INDEX_P2TRP: {
		if (iIndex == SSI_INDEX_P0TRP) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1TRP) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0TRP) ? 0 : 1;
		if ((g_sParameters.sPort[ui32Port].ucWorkMode) ==
		PORT_TELNET_SERVER) {
			return (usnprintf(pcInsert, iInsertLen, "N/A"));
		} else {
			return (usnprintf(pcInsert, iInsertLen, "%d",
					g_sParameters.sPort[ui32Port].ui16TelnetRemotePort));
		}
	}

		//
		// These tag are replaced with a string describing the port's current
		// telnet mode, client or server.
		//0823
	case SSI_INDEX_P0WS:
	case SSI_INDEX_P1WS:
	case SSI_INDEX_P2WS: {
		if (iIndex == SSI_INDEX_P0WS) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1WS) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0WS) ? 0 : 1;
		/*return(usnprintf(pcInsert, iInsertLen, "%s",
		 ((g_sParameters.sPort[ui32Port].ui8Flags &
		 PORT_FLAG_CONTACT_MODE) == PORT_TELNET_SERVER) ?
		 "TCP Server" : "TCP Client"));*/

		pcString = ConfigMapIdToString(g_psContactModeMap, NUM_ContactMode_MAPS,
				(g_sParameters.sPort[ui32Port].ucWorkMode));

		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}
		// telnet mode, client or server.
		//0823
	case SSI_INDEX_P0MX:
		//case SSI_INDEX_P1MX:
	{
		ui32Port = (iIndex == SSI_INDEX_P0MX) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%d",
				(g_sParameters.sPort[ui32Port].uiTCPServerType >> 4) & 0x0F));
	}
		//
		// These tag are replaced with a string describing the port's current
		// telnet mode, client or server.
		//
	case SSI_INDEX_P0PROT:
	case SSI_INDEX_P1PROT:
	case SSI_INDEX_P2PROT: {
		if (iIndex == SSI_INDEX_P0PROT) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1PROT) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0PROT) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%s",
				((g_sParameters.sPort[ui32Port].ui8Flags &
				PORT_FLAG_PROTOCOL) == PORT_PROTOCOL_TELNET) ? "Telnet" : "Raw"));
	}

		//
		// These tags are replaced with the full destination IP address for
		// the relevant port's telnet connection (which is only valid
		// when operating as a telnet client).
		//
	case SSI_INDEX_P0TIP:
	case SSI_INDEX_P1TIP:
	case SSI_INDEX_P2TIP: {
		if (iIndex == SSI_INDEX_P0TIP) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1TIP) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//ui32Port = (iIndex == SSI_INDEX_P0TIP) ? 0 : 1;
		if ((g_sParameters.sPort[ui32Port].ucWorkMode) == PORT_TELNET_SERVER) {
			return (usnprintf(pcInsert, iInsertLen, "N/A"));
		} else {
			return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
					(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 24)
							& 0xFF,
					(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 16)
							& 0xFF,
					(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 8)
							& 0xFF,
					(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 0)
							& 0xFF));
		}
	}
		// UDP IP
		//
	case SSI_INDEX_UDP_IP: {
		ui32Port = (iIndex == SSI_INDEX_UDP_IP) ? 0 : 1;
		UDPIP = chartodigit((char *) g_sParameters.sPort[ui32Port].uiTelnetURL);
		//UARTprintf("\n ip1 is: %X \n",UDPIP);

		return (usnprintf(pcInsert, iInsertLen, "%d.%d.%d.%d",
				(UDPIP >> 24) & 0xFF, (UDPIP >> 16) & 0xFF, (UDPIP >> 8) & 0xFF,
				(UDPIP >> 0) & 0xFF));
	}
		//
		// These tags are replaced with the first (most significant) number
		// in an aa.bb.cc.dd IP address (aa in this case).
		//
	case SSI_INDEX_P0TIP1:
		//case SSI_INDEX_P1TIP1:
	{
		ui32Port = (iIndex == SSI_INDEX_P0TIP1) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%d",
				(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 24) & 0xFF));
	}

		//
		// These tags are replaced with the first (most significant) number
		// in an aa.bb.cc.dd IP address (aa in this case).
		//
	case SSI_INDEX_P0TIP2:
		//case SSI_INDEX_P1TIP2:
	{
		ui32Port = (iIndex == SSI_INDEX_P0TIP2) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%d",
				(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 16) & 0xFF));
	}

		//
		// These tags are replaced with the first (most significant) number
		// in an aa.bb.cc.dd IP address (aa in this case).
		//
	case SSI_INDEX_P0TIP3:
		//case SSI_INDEX_P1TIP3:
	{
		ui32Port = (iIndex == SSI_INDEX_P0TIP3) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%d",
				(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 8) & 0xFF));
	}

		//
		// These tags are replaced with the first (most significant) number
		// in an aa.bb.cc.dd IP address (aa in this case).
		//
	case SSI_INDEX_P0TIP4:
		//case SSI_INDEX_P1TIP4:
	{
		ui32Port = (iIndex == SSI_INDEX_P0TIP4) ? 0 : 1;
		return (usnprintf(pcInsert, iInsertLen, "%d",
				g_sParameters.sPort[ui32Port].ui32TelnetIPAddr & 0xFF));
	}

		//
		// Generate a block of JavaScript declaring variables which hold the
		// current settings for one of the ports.
		/*
		 case SSI_INDEX_P0VARS://0821
		 //case SSI_INDEX_P1VARS:
		 {
		 ui32Port = (iIndex == SSI_INDEX_P0VARS) ? 0 : 1;
		 iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		 if(iCount < iInsertLen)
		 {
		 iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
		 SER_JAVASCRIPT_VARS,
		 SerialGetBaudRate(ui32Port),
		 SerialGetTransitMode(ui32Port), //pony
		 SerialGetStopBits(ui32Port),
		 SerialGetDataSize(ui32Port),
		 SerialGetFlowControl(ui32Port),
		 SerialGetParity(ui32Port));
		 }
		 if(iCount < iInsertLen)
		 {
		 iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
		 "%s", JAVASCRIPT_FOOTER);
		 }

		 return(iCount);
		 }
		 */
		// Generate a block of JavaScript declaring variables which hold the
		// current settings for one of the ports.
		//*
	case SSI_INDEX_P0URVARS:
	case SSI_INDEX_P1URVARS:
	case SSI_INDEX_P2URVARS: {
//		ui32Port = (iIndex == SSI_INDEX_P0URVARS) ? 0 : 1;
		if (iIndex == SSI_INDEX_P0URVARS) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1URVARS) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		//UARTprintf("\nVARS port:%d\n",ui32Port);
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
					UR_JAVASCRIPT_VARS,
					SerialGetTransitMode(ui32Port),
					SerialGetBaudRate(ui32Port), //pony
					SerialGetStopBits(ui32Port),
					SerialGetDataSize(ui32Port),
					SerialGetFlowControl(ui32Port),
					SerialGetParity(ui32Port)
							);//chuck
			//GetRFC2217Value()
		}
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}

		return (iCount);
	}
		//
		// Return the user-editable friendly name for the module.
		//
	case SSI_INDEX_MODNAME: {
		//            UARTprintf("modname is %s\n",g_sParameters.ui8ModName);
		return (usnprintf(pcInsert, iInsertLen, "%s", g_sParameters.ui8ModName));

	}

		//
		// Initialise JavaScript variables containing the information related
		// to the net settings for COM.
		//chris_web
	case SSI_INDEX_P0NETVARS:
	case SSI_INDEX_P1NETVARS:
	case SSI_INDEX_P2NETVARS: {
//		ui32Port = (iIndex == SSI_INDEX_P0NETVARS) ? 0 : 1;
		if (iIndex == SSI_INDEX_P0NETVARS) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1NETVARS) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		packlen = g_sParameters.sPort[ui32Port].uiPackLen;
		//                	UARTprintf("packlen:0x%02X,0x%02X,0x%02X,0x%02X\n",packlen[0],packlen[1],packlen[2],packlen[3]);
		//                	UARTprintf("packlen:%d,%d,%d,%d\n",packlen[0],packlen[1],packlen[2],packlen[3]);
		//                	UARTprintf("packlen3:%d\n",g_sParameters.sPort[ui32Port].uiPackLen[3]);
		//                	frame_len_ = packlen[0]<<24 + packlen[1]<<16 + packlen[2]<<8 + packlen[3];
		frame_len_ = ((packlen[0]) << 24) + ((packlen[1]) << 16)
				+ ((packlen[2]) << 8) + packlen[3];
		//                 	frame_len_ = packlen[3];
		//                	UARTprintf("packlen<<:0x%08X,0x%06X,0x%04X,0x%02X\n",(packlen[0])<<24,(packlen[1])<<16,(packlen[2])<<8,packlen[3]);
		//                	UARTprintf("frame_len_:%d\n",frame_len_);

		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			NET_JAVASCRIPT_VARS, g_sParameters.sPort[ui32Port].ucWorkMode,
					g_sParameters.sPort[ui32Port].ui16TelnetLocalPort,
					g_sParameters.sPort[ui32Port].ui16TelnetRemotePort,
					g_sParameters.sPort[ui32Port].ui32TelnetTimeout,
					g_sParameters.sPort[ui32Port].uiPackTime, frame_len_,
					GetModbusValue(),
					g_sParameters.sPort[ui32Port].ui8Flags & PORT_FLAG_PROTOCOL
					);
			//(g_sParameters.sPort[ui32Port].uiTCPServerType >> 4) & 0x0F,
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		//
		// Initialise JavaScript variables containing channel switch
		//
	case SSI_INDEX_SWITCH_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			SW_JAVASCRIPT_VARS, g_sParameters.datachansel);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
#if  Have_WiFi_Module_SKU
		//
		// Initialise JavaScript variables containing WIFI mode select
		//
		case SSI_INDEX_WIFIMD_JS:
		{
			if(g_sParameters.datachansel==2)	//1, lan-com;2; wilfi-com;
			Switch_WiFi_to_COM = 0;
			else if(g_sParameters.datachansel==3)// both enable;
			{
				Switch_WiFi_to_COM = 0;
				Switch_LAN_to_COM = 0;
			}
			vTaskDelay( 60/portTICK_RATE_MS );
			pxMessage.ui8Name = WiFi_DataTrans_State;
			xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
			//get feedback data and send to web;
			xQueueReceive( g_QueWiFiDataBack, &sWiFiCMD, portMAX_DELAY );
			if(g_sParameters.datachansel==2)//1, lan-com;2; wilfi-com;
			Switch_WiFi_to_COM = 1;
			else if(g_sParameters.datachansel==3)// both enable;
			{
				Switch_WiFi_to_COM = 1;
				Switch_LAN_to_COM = 1;
			}
			iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
			if(iCount < iInsertLen)
			{
				iCount +=
				usnprintf(pcInsert + iCount, iInsertLen - iCount,
						WM_JAVASCRIPT_VARS,
						g_sParameters.sWiFiModule.sWiFiModSel.ucWiFiMode,
						WiFiData_ProtocolGet(),
						g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort,
						g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort,
						(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr >> 24) & 0xFF,
						(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr >> 16) & 0xFF,
						(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr >> 8) & 0xFF,
						(g_sParameters.sWiFiModule.sWiFiModSel.ui32DataIPAddr >> 0) & 0xFF); //for v1 debug
			}

			if(iCount < iInsertLen)
			{
				iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
						"%s", JAVASCRIPT_FOOTER);
			}
			return(iCount);
		}

#endif
		//
		// Initialise JavaScript variables containing WIFI AP mode set
		//

	case SSI_INDEX_APSET_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			if (APPW_NULL)
				iCount +=
						usnprintf(pcInsert + iCount, iInsertLen - iCount,
						APSET_JAVASCRIPT_VARS,
								g_sParameters.sWiFiModule.sWiFiAPSet.ucFrequency_Channel,
								g_sParameters.sWiFiModule.sWiFiAPSet.ucDHCPType,
								g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName,
								"\0");
			else
				iCount +=
						usnprintf(pcInsert + iCount, iInsertLen - iCount,
						APSET_JAVASCRIPT_VARS,
								g_sParameters.sWiFiModule.sWiFiAPSet.ucFrequency_Channel,
								g_sParameters.sWiFiModule.sWiFiAPSet.ucDHCPType,
								g_sParameters.sWiFiModule.sWiFiAPSet.ui8NetName,
								g_sParameters.sWiFiModule.sWiFiAPSet.ui8SetPasswords);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		// Initialise JavaScript variables containing AP IP
		//
	case SSI_INDEX_APIP_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			APIP_JAVASCRIPT_VARS,
					(g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP >> 24)
							& 0xFF,
					(g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP >> 16)
							& 0xFF,
					(g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP >> 8)
							& 0xFF,
					(g_sParameters.sWiFiModule.sWiFiAPSet.ui32AP_IP >> 0)
							& 0xFF);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		// Initialise JavaScript variables containing AP MASK
		//
	case SSI_INDEX_APMS_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount +=
					usnprintf(pcInsert + iCount, iInsertLen - iCount,
					APMK_JAVASCRIPT_VARS,
							(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask
									>> 24) & 0xFF,
							(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask
									>> 16) & 0xFF,
							(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask
									>> 8) & 0xFF,
							(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APSubnetMask
									>> 0) & 0xFF);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		// Initialise JavaScript variables containing AP Gateway
		//
	case SSI_INDEX_GTIP_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			APGW_JAVASCRIPT_VARS,
					(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay >> 24)
							& 0xFF,
					(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay >> 16)
							& 0xFF,
					(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay >> 8)
							& 0xFF,
					(g_sParameters.sWiFiModule.sWiFiAPSet.ui32APGateWay >> 0)
							& 0xFF);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		// Initialise JavaScript variables containing STA set
		//
	case SSI_INDEX_STASET_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			if (StaPW_NULL)
				iCount +=
						usnprintf(pcInsert + iCount, iInsertLen - iCount,
						STASET_JAVASCRIPT_VARS,
								g_sParameters.sWiFiModule.sWiFiStaionSet.ucStationConnType,
								"\0",
								g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName,
								join_AP_pw_needed);

			else
				iCount +=
						usnprintf(pcInsert + iCount, iInsertLen - iCount,
						STASET_JAVASCRIPT_VARS,
								g_sParameters.sWiFiModule.sWiFiStaionSet.ucStationConnType,
								g_sParameters.sWiFiModule.sWiFiStaionSet.ui8InputPasswords,
								g_sParameters.sWiFiModule.sWiFiStaionSet.ui8NetName,
								join_AP_pw_needed);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		join_AP_pw_needed = 0;
		return (iCount);
	}
		// Initialise JavaScript variables containing STA IP
		//
	case SSI_INDEX_STCIP_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			STCIP_JAVASCRIPT_VARS,
					(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticIP
							>> 24) & 0xFF,
					(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticIP
							>> 16) & 0xFF,
					(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticIP
							>> 8) & 0xFF,
					(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticIP
							>> 0) & 0xFF);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		// Initialise JavaScript variables containing STA MASK
		//
	case SSI_INDEX_STCMS_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			STCMS_JAVASCRIPT_VARS,
					(g_sParameters.sWiFiModule.sWiFiStaionSet.ui32STASubnetMask
							>> 24) & 0xFF,
					(g_sParameters.sWiFiModule.sWiFiStaionSet.ui32STASubnetMask
							>> 16) & 0xFF,
					(g_sParameters.sWiFiModule.sWiFiStaionSet.ui32STASubnetMask
							>> 8) & 0xFF,
					(g_sParameters.sWiFiModule.sWiFiStaionSet.ui32STASubnetMask
							>> 0) & 0xFF);
		}
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		// Initialise JavaScript variables containing STA Gateway
		//
	case SSI_INDEX_STCGW_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount +=
					usnprintf(pcInsert + iCount, iInsertLen - iCount,
					STCGW_JAVASCRIPT_VARS,
							(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticGateway
									>> 24) & 0xFF,
							(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticGateway
									>> 16) & 0xFF,
							(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticGateway
									>> 8) & 0xFF,
							(g_sParameters.sWiFiModule.sWiFiStaionSet.sWiFiSTAstaticGateway
									>> 0) & 0xFF);
		}
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		// Initialise JavaScript variables containing Login
		//
	case SSI_INDEX_LOGIN_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			LOGIN_JAVASCRIPT_VARS, permit, login_counter);
			//UARTprintf("permit is: %d !\n",permit);
			//UARTprintf("Upload password is: %s !\n",g_sParameters.ui8Passwords);
		}
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
	case SSI_INDEX_SCAN_JS: {
		for (i = 0; i < Scaned_AP_Num; i++) {
			if ((iInsertLen - iCount) >= iCountMax) //whether space is enough for the next AP data;
					{
				iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
				SCAN_JAVASCRIPT_VARS,
						(char *) WiFi_SSID_temp[i], //ssid first address;
						((char *) WiFi_SSID_temp[i]), RSSI[i],
						Sec_Mode_Tab[securitymode[i]].SecModeName, //
						((char *) WiFi_SSID_temp[i]), RSSI[i],
						Sec_Mode_Tab[securitymode[i]].SecModeName ////,
						);
				//	UARTprintf("\nAP lengh: %d\n",iCount);
				//	UARTprintf("\nAP SSID%d:%s\n",i,WiFi_SSID_temp[i]);
			} else
				break;

		}
		return (iCount);
	}
		//*
		// Initialise JavaScript variables containing the information related
		// to the telnet settings for a given port.
		//
	case SSI_INDEX_P0IPVAR:
	case SSI_INDEX_P1IPVAR:
	case SSI_INDEX_P2IPVAR: {
//		ui32Port = (iIndex == SSI_INDEX_P0IPVAR) ? 0 : 1;
		if (iIndex == SSI_INDEX_P0IPVAR) {
			ui32Port = 0;
		} else if (iIndex == SSI_INDEX_P1IPVAR) {
			ui32Port = 1;
		} else {
			ui32Port = 2;
		}
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			TIP_JAVASCRIPT_VARS,
					(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 24)
							& 0xFF,
					(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 16)
							& 0xFF,
					(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 8)
							& 0xFF,
					(g_sParameters.sPort[ui32Port].ui32TelnetIPAddr >> 0)
							& 0xFF);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}

		//
		// Generate a block of JavaScript variables containing the current
		// static UP address and static/DHCP setting.
		//
	case SSI_INDEX_IPVARS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			IP_JAVASCRIPT_VARS, (g_sParameters.ui8Flags &
			CONFIG_FLAG_STATICIP) ? 1 : 0,
					(g_sParameters.ui32StaticIP >> 24) & 0xFF,
					(g_sParameters.ui32StaticIP >> 16) & 0xFF,
					(g_sParameters.ui32StaticIP >> 8) & 0xFF,
					(g_sParameters.ui32StaticIP >> 0) & 0xFF);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}

		//
		// Generate a block of JavaScript variables containing the current
		// subnet mask.
		//
	case SSI_INDEX_SNVARS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			SUBNET_JAVASCRIPT_VARS, (g_sParameters.ui32SubnetMask >> 24) & 0xFF,
					(g_sParameters.ui32SubnetMask >> 16) & 0xFF,
					(g_sParameters.ui32SubnetMask >> 8) & 0xFF,
					(g_sParameters.ui32SubnetMask >> 0) & 0xFF);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}

		//
		// Generate a block of JavaScript variables containing the current
		// subnet mask.
		//
	case SSI_INDEX_GWVARS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			GW_JAVASCRIPT_VARS, (g_sParameters.ui32GatewayIP >> 24) & 0xFF,
					(g_sParameters.ui32GatewayIP >> 16) & 0xFF,
					(g_sParameters.ui32GatewayIP >> 8) & 0xFF,
					(g_sParameters.ui32GatewayIP >> 0) & 0xFF);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		//UDP IP JS
		//
	case SSI_INDEX_UDP_IP_JS: {
		ui32Port = (iIndex == SSI_INDEX_UDP_IP_JS) ? 0 : 1;
		UDPIP = chartodigit((char *) g_sParameters.sPort[ui32Port].uiTelnetURL);
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			UDPIP_JAVASCRIPT_VARS, (UDPIP >> 24) & 0xFF, (UDPIP >> 16) & 0xFF,
					(UDPIP >> 8) & 0xFF, (UDPIP >> 0) & 0xFF);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		//
		// Generate an HTML text input field containing the current module
		// name.
		//
	case SSI_INDEX_MODNINP: {
		iCount = usnprintf(pcInsert, iInsertLen, "<input value='");

		if (iCount < iInsertLen) {
			iCount += ConfigEncodeFormString((char *) g_sParameters.ui8ModName,
					pcInsert + iCount, iInsertLen - iCount);
		}

		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
					"' maxlength='%d' size='%d' name='modname'>",
					(MOD_NAME_LEN - 1), MOD_NAME_LEN);
		}
		return (iCount);
	}
		//
		// Initialise JavaScript variables containing login password modification feedback
		//
	case SSI_INDEX_FEEDBACK_JS: {
		iCount = usnprintf(pcInsert, iInsertLen, "%s", JAVASCRIPT_HEADER);
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount,
			FEEDBACK_JAVASCRIPT_VARS, feedback);
		}
		feedback = 0;
		if (iCount < iInsertLen) {
			iCount += usnprintf(pcInsert + iCount, iInsertLen - iCount, "%s",
					JAVASCRIPT_FOOTER);
		}
		return (iCount);
	}
		//Modbus function
		//
	case SSI_INDEX_MODBUS: {
		pcString = ConfigMapIdToString(g_psModbusMap, NUM_Modbus_MAPS,
				GetModbusValue());
		//UARTprintf("switch is: %d !\n",g_sParameters.datachansel);
		return (usnprintf(pcInsert, iInsertLen, "%s", pcString));
	}
		//
		// All other tags are unknown.
		//
	default: {
		return (usnprintf(pcInsert, iInsertLen, "<b><i>Tag %d unknown!</i></b>",
				iIndex));
	}
	}
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
