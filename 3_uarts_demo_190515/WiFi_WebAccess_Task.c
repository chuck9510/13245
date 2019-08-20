/*
 * WiFi_WebAccess_Task.c
 *
 *  Created on: 2015Äê10ÔÂ19ÈÕ
 *      Author: Chris
 */
#include <stdio.h>
#include <string.h>
#include "user_define.h"
#include "WiFly_Webpage.h"
#include "third_party/lwip-1.4.1/apps/httpserver_raw/fs.h"
#include "third_party/lwip-1.4.1/apps/httpserver_raw/httpd_structs.h"
#include "wifi_fsdata.h"

extern bool Wifi_Set_OK;
extern const char * const g_pcTagLeadIn;
extern const char * const g_pcTagLeadOut;
extern uint16_t ConfigSSIHandler(int iIndex, char *pcInsert, int iInsertLen);
extern void SetModnameCMDtoWiFi(uint8_t *Modname);
#define STACKSIZE_WiflypageTASK    1024
bool alloc_mem =false;
bool WiFly_Webaccess_Ready;
bool WiFly_Webaccess = false;
bool StateMac_COMM_OK = true;
bool Wifly_Acc;
bool Wifly_modname_modify;
extern bool Wifly_AP_STA;
char wifiRxBuf[512];//buffer to store characters for each line received by wifly uart
uint8_t httpLine;
bool httpCal = false;
bool Ret_Search_Page = false;
extern uint8_t join_AP_pw_needed;

enum wifi_tag_check_state {
  TAG_NONE,       /* Not processing an SSI tag */
  TAG_LEADIN,     /* Tag lead in "<!--#" being processed */
  TAG_FOUND,      /* Tag name being read, looking for lead-out start */
  TAG_LEADOUT,    /* Tag lead out "-->" being processed */
  TAG_SENDING     /* Sending tag replacement string */
};
#define NUM_HDR_STRINGS 3

struct wifi_http_state
{
  struct fs_file *handle;
  char *file;       /* Pointer to first unsent byte in buf. */

#if LWIP_HTTPD_SUPPORT_REQUESTLIST
  struct pbuf *req;
#endif /* LWIP_HTTPD_SUPPORT_REQUESTLIST */

#if LWIP_HTTPD_SSI || LWIP_HTTPD_DYNAMIC_HEADERS
  char *buf;        /* File read buffer. */
  int buf_len;      /* Size of file read buffer, buf. */
#endif /* LWIP_HTTPD_SSI || LWIP_HTTPD_DYNAMIC_HEADERS */
  u32_t left;       /* Number of unsent bytes in buf. */
  u8_t retries;
#if LWIP_HTTPD_SSI
  const char *parsed;     /* Pointer to the first unparsed byte in buf. */
#if !LWIP_HTTPD_SSI_INCLUDE_TAG
  const char *tag_started;/* Poitner to the first opening '<' of the tag. */
#endif /* !LWIP_HTTPD_SSI_INCLUDE_TAG */
  const char *tag_end;    /* Pointer to char after the closing '>' of the tag. */
  u32_t parse_left; /* Number of unparsed bytes in buf. */
  u16_t tag_index;   /* Counter used by tag parsing state machine */
  u16_t tag_insert_len; /* Length of insert in string tag_insert */
#if LWIP_HTTPD_SSI_MULTIPART
  u16_t tag_part; /* Counter passed to and changed by tag insertion function to insert multiple times */
#endif /* LWIP_HTTPD_SSI_MULTIPART */
  u8_t tag_check;   /* true if we are processing a .shtml file else false */
  u8_t tag_name_len; /* Length of the tag name in string tag_name */
  char tag_name[LWIP_HTTPD_MAX_TAG_NAME_LEN + 1]; /* Last tag name extracted */
  char tag_insert[LWIP_HTTPD_MAX_TAG_INSERT_LEN + 1]; /* Insert string for tag_name */
  enum wifi_tag_check_state tag_state; /* State of the tag processor */
#endif /* LWIP_HTTPD_SSI */
#if LWIP_HTTPD_CGI
  char *params[LWIP_HTTPD_MAX_CGI_PARAMETERS]; /* Params extracted from the request URI */
  char *param_vals[LWIP_HTTPD_MAX_CGI_PARAMETERS]; /* Values for each extracted param */
#endif /* LWIP_HTTPD_CGI */
#if LWIP_HTTPD_DYNAMIC_HEADERS
  const char *hdrs[NUM_HDR_STRINGS]; /* HTTP headers to be sent. */
  u16_t hdr_pos;     /* The position of the first unsent header byte in the
                        current string */
  u16_t hdr_index;   /* The index of the hdr string currently being sent. */
#endif /* LWIP_HTTPD_DYNAMIC_HEADERS */
#if LWIP_HTTPD_TIMING
  u32_t time_started;
#endif /* LWIP_HTTPD_TIMING */
#if LWIP_HTTPD_SUPPORT_POST
  u32_t post_content_len_left;
#if LWIP_HTTPD_POST_MANUAL_WND
  u32_t unrecved_bytes;
  struct tcp_pcb *pcb;
  u8_t no_auto_wnd;
#endif /* LWIP_HTTPD_POST_MANUAL_WND */
#endif /* LWIP_HTTPD_SUPPORT_POST*/
};
/*
typedef struct
{
  const char *name;
  unsigned char shtml;
} dft_filename;

const dft_filename gw_psDefaultFilenames[] = {
  {"/login.shtml", true },
  {"/login.ssi", true },
  {"/login.shtm", true },
  {"/login.html", false },
  {"/login.htm", false }
};

#define NUM_DEFAULT_FILENAMES (sizeof(gw_psDefaultFilenames) /   \
                               sizeof(dft_filename))
*/
typedef struct
{
  const char *name;
  const uint8_t *file_data;
  uint8_t offset;
} wifly_dft_filesys;

const wifly_dft_filesys wifly_psFileSys[] = {
  {"/404.htm", data_404_htm, 9 },
  {"/favicon.ico", data_favicon_ico, 13 },
  {"/foxconn_logo.jpg", data_foxconn_logo_jpg, 18 },
  {"/logo2.jpg", data_logo2_jpg, 11 },
 // {"/product.jpg", data_product_jpg, 13 },
  {"/dev.shtm", data_dev_shtm, 10 },
  {"/index.htm", data_index_htm, 11 },
  {"/ipchg.shtm", data_ipchg_shtm, 12 },
  {"/lan.shtm", data_lan_shtm, 10 },
  {"/login.shtm", data_login_shtm, 12 },
  {"/ms.shtm", data_ms_shtm, 9 },
  {"/overview.htm", data_overview_htm, 14 },
  {"/perror.htm", data_perror_htm, 12 },
  {"/searchWifi.shtm", data_searchWifi_shtm, 17 },
  {"/wlan2ur.shtm", data_wlan2ur_shtm, 14 },
  {"/wlanap.shtm", data_wlanap_shtm, 13 },
  {"/wlansta.shtm", data_wlansta_shtm, 14 },
  {"/s2e.shtm", data_s2e_shtm, 10 },

};

#define NUM_WIFLY_FILENAMES (sizeof(wifly_psFileSys) /   \
                               sizeof(wifly_dft_filesys))


extern void *mem_malloc(unsigned long size);
extern void  mem_free(void *mem);
extern int NUM_CONFIG_CGI_URIS;
extern const tCGI g_psConfigCGIURIs[15];

extern int NUM_CONFIG_SSI_TAGS;
extern const char *g_pcConfigSSITags[75];

//extern bool logoff_htm;//for index.htm
//extern bool logoff_shtm;//for .shtm
//extern int wifly_extract_uri_parameters(struct wifi_http_state *hs, char *params);

struct wifi_http_state*
hs_alloc(void)
{
  struct wifi_http_state *ret;
  ret = (struct wifi_http_state *)mem_malloc(sizeof(struct wifi_http_state));
  if (ret != NULL) {
    /* Initialize the structure. */
    memset(ret, 0, sizeof(struct wifi_http_state));
/* #if LWIP_HTTPD_DYNAMIC_HEADERS
    Indicate that the headers are not yet valid
    ret->hdr_index = NUM_FILE_HDR_STRINGS;*/
/*#endif  LWIP_HTTPD_DYNAMIC_HEADERS */
  }
  return ret;
}

void
hs_free(struct wifi_http_state *hs)
{
  if (hs != NULL) {
    if(hs->handle) {
      //fs_close(hs->handle);
      hs->handle = NULL;
    }
#if LWIP_HTTPD_SSI || LWIP_HTTPD_DYNAMIC_HEADERS
    if (hs->buf != NULL) {
      mem_free(hs->buf);
      hs->buf = NULL;
    }
#else /* HTTPD_USE_MEM_POOL */
    mem_free(hs);
#endif /* HTTPD_USE_MEM_POOL */
  }
  UARTprintf("free mem\n");
}

uint8_t
wifly_extract_uri_parameters(struct wifi_http_state *hs, char *params)
{
  char *pair;
  char *equals;
  uint8_t loop;

  /* If we have no parameters at all, return immediately. */
  if(!params || (params[0] == '\0')) {
      return(0);
  }

  /* Get a pointer to our first parameter */
  pair = params;

  /* Parse up to LWIP_HTTPD_MAX_CGI_PARAMETERS from the passed string and ignore the
   * remainder (if any) */
  for(loop = 0; (loop < LWIP_HTTPD_MAX_CGI_PARAMETERS) && pair; loop++) {

    /* Save the name of the parameter */
    hs->params[loop] = pair;

    /* Remember the start of this name=value pair */
    equals = pair;

    /* Find the start of the next name=value pair and replace the delimiter
     * with a 0 to terminate the previous pair string. */
    pair = strchr(pair, '&');
    if(pair) {
      *pair = '\0';
      pair++;
    } else {
       /* We didn't find a new parameter so find the end of the URI and
        * replace the space with a '\0' */
        pair = strchr(equals, ' ');
        if(pair) {
            *pair = '\0';
        }

        /* Revert to NULL so that we exit the loop as expected. */
        pair = NULL;
    }

    /* Now find the '=' in the previous pair, replace it with '\0' and save
     * the parameter value string. */
    equals = strchr(equals, '=');
    if(equals) {
      *equals = '\0';
      hs->param_vals[loop] = equals + 1;
    } else {
      hs->param_vals[loop] = NULL;
    }
  }

  return loop;
}

void
wifly_get_tag_insert(struct wifi_http_state *hs)
{

	  int loop;
	  size_t len;

//	  if( g_pcConfigSSITags && NUM_CONFIG_SSI_TAGS) {

	    /* Find this tag in the list we have been provided. */
	    for(loop = 0; loop < NUM_CONFIG_SSI_TAGS; loop++) {
	      if(strcmp(hs->tag_name, g_pcConfigSSITags[loop]) == 0) {
	        hs->tag_insert_len = ConfigSSIHandler(loop, hs->tag_insert,
	           LWIP_HTTPD_MAX_TAG_INSERT_LEN);
	        join_AP_pw_needed = 0;
	        return;
	      }
	    }
//	  }

	  /* If we drop out, we were asked to serve a page which contains tags that
	   * we don't have a handler for. Merely echo back the tags with an error
	   * marker. */
	#define UNKNOWN_TAG1_TEXT "<b>***UNKNOWN TAG "
	#define UNKNOWN_TAG1_LEN  18
	#define UNKNOWN_TAG2_TEXT "***</b>"
	#define UNKNOWN_TAG2_LEN  7
	  len = LWIP_MIN(strlen(hs->tag_name),
	    LWIP_HTTPD_MAX_TAG_INSERT_LEN - (UNKNOWN_TAG1_LEN + UNKNOWN_TAG2_LEN));
	  MEMCPY(hs->tag_insert, UNKNOWN_TAG1_TEXT, UNKNOWN_TAG1_LEN);
	  MEMCPY(&hs->tag_insert[UNKNOWN_TAG1_LEN], hs->tag_name, len);
	  MEMCPY(&hs->tag_insert[UNKNOWN_TAG1_LEN + len], UNKNOWN_TAG2_TEXT, UNKNOWN_TAG2_LEN);
	  hs->tag_insert[UNKNOWN_TAG1_LEN + len + UNKNOWN_TAG2_LEN] = 0;

	  len = strlen(hs->tag_insert);
	  //LWIP_ASSERT("len <= 0xffff", len <= 0xffff);
	  hs->tag_insert_len = (u16_t)len;
}

void
Wifly_CharSend(uint8_t ui8Char)
{

//	 UARTprintf("%c",ui8Char);
    //
    // Disable the UART transmit interrupt while determining how to handle this
    // character.  Failure to do so could result in the loss of this character,
    // or stalled output due to this character being placed into the UART
    // transmit buffer but never transferred out into the UART FIFO.
    //
    UARTIntDisable(g_ui32UARTBase[WiFi_Port], UART_INT_TX);
    //
    // See if the transmit buffer is empty and there is space in the FIFO.
    //
    if(RingBufEmpty(&wifi_pageTxBuf) &&
       (UARTSpaceAvail(g_ui32UARTBase[WiFi_Port])))
    {
        //
        // Write this character directly into the FIFO.
        //
        UARTCharPut(g_ui32UARTBase[WiFi_Port], ui8Char);
       //   UARTprintf("%c",ui8Char);
      delayuS(86);
        //vTaskDelay(1);
    }


    //
    // See if there is room in the transmit buffer.
    //
    else if(!RingBufFull(&wifi_pageTxBuf))
    {
        //
        // Put this character into the transmit buffer.
        //
    //	UARTprintf("%c",ui8Char);

    //	  delayuS(150);
    	//vTaskDelay(300);
    	delayuS(86);
    //	vTaskDelay(1/portTICK_RATE_MS);
        RingBufWriteOne(&wifi_pageTxBuf, ui8Char);
    }
    else
    {
    	vTaskDelay(5/portTICK_RATE_MS);
    	RingBufWriteOne(&wifi_pageTxBuf, ui8Char);
    }

 	 	WiFi_Status_Retval = WiFi_Datain;
        WiFiLEDflashtime = 2;
    //
    // Enable the UART transmit interrupt.
    //
    UARTIntEnable(g_ui32UARTBase[WiFi_Port], UART_INT_TX);
}

int32_t
Wifly_CharReceive(void)
{
    uint32_t ui32Data;

    //
    // See if the receive buffer is empty and there is space in the FIFO.
    //
    if(RingBufEmpty(&wifi_pageRxBuf))
    {
        //
        // Return -1 (EOF) to indicate no data available.
        //
        return(-1);
    }

    //
    // Read a single character.
    //
    ui32Data = (long)RingBufReadOne(&wifi_pageRxBuf);

    //
    // Return the data that was read.
    //
 	 	WiFi_Status_Retval = WiFi_Datain;
        WiFiLEDflashtime = 2;

    return(ui32Data);
}

void sendPhoto(uint8_t loop)
{
	uint32_t PhotoLength, i;
	if(loop == 1)
		PhotoLength = sizeof(data_favicon_ico) - (uint32_t)wifly_psFileSys[loop].offset;
	else if(loop == 2)
		PhotoLength = sizeof(data_foxconn_logo_jpg) - (uint32_t)wifly_psFileSys[loop].offset;
	else if(loop == 3)
		PhotoLength = sizeof(data_logo2_jpg) - (uint32_t)wifly_psFileSys[loop].offset;
//	else if(loop == 4)
	//	PhotoLength = sizeof(data_product_jpg) - (uint32_t)wifly_psFileSys[loop].offset;
	UARTprintf("+%u+\n",wifly_psFileSys[loop].offset);
	//UARTprintf("+%u+\n",sizeof(wifly_psFileSys[loop].file_data));
	UARTprintf("+%u+\n",PhotoLength);
	for(i = 0; i < PhotoLength; i++)
	{
		Wifly_CharSend(*(wifly_psFileSys[loop].file_data + wifly_psFileSys[loop].offset + i));
	}

}
void sendPage(const uint8_t *s,struct wifi_http_state *hs)
{
	//const char * const TagLeadIn;
	uint8_t index = 0;
	//const uint8_t *address;
	//char *tag1 = NULL,*tag2 = NULL;
	char tag[10];
	bool Tag_ready = false;
     while(*s)
    {
    	 //find and send "<!--#"
    	if(*s == g_pcTagLeadIn[0])
    		{
    		index = 1;
    		goto SendChar;
    		}
    	else if(index == 1)
    	{
    		if(*s == g_pcTagLeadIn[1])
    			index = 2;
    		else
    		{
    			index =0;
    		}
    	goto SendChar;
    	}
    	else if(index == 2)
    	{
    		if(*s == g_pcTagLeadIn[2])
    			index = 3;
    		else
    		{
    			index =0;
    		}
    	goto SendChar;
    	}
    	else if(index == 3)
    	{
    		if(*s == g_pcTagLeadIn[3])
    			index = 4;
    		else
    		{
    			index =0;
    		}
    	goto SendChar;
    	}
    	else if(index == 4)
    	{
    		if(*s == g_pcTagLeadIn[4])
    			index = 5;
    		else
    		{
    			index =0;
    		}
    	goto SendChar;
    	}

    	//send tag
    	if(index >= 5)
    	{
    		tag[index-5] = *s;
    		//tag1++;
    		index++;//start from 5
    	}


    	if((*s == '-')&&(index > 5))
    	{
    		tag[index-6] = '\0';
    		//address = s;
    		Tag_ready = true;
    		//hs->tag_name = tag2;
    		//UARTprintf("\ntag is %s\n",tag);
    		strcpy(hs->tag_name,tag);
    		wifly_get_tag_insert(hs);
    		index = 0;
    	}

    	if(Tag_ready)
    	{
    		const char *LeadOut = g_pcTagLeadOut;
    		char *tag_in = hs->tag_insert;
    		//send "-->"
    	    	while(*LeadOut)
    	    	{
    	    		Wifly_CharSend(*LeadOut);
    	    		LeadOut++;
    	    	}
    	    //send replacement comment
    	    	while(*tag_in)
    	    	{
    	    		Wifly_CharSend(*tag_in);
    	    		tag_in++;
    	    	}
    	    	s+=2;
    	    	Tag_ready = false;
    	}
    			else
SendChar:		Wifly_CharSend(*s);	// send the character and point to the next one
    	s++;
   }
    Wifly_CharSend('\r');// terminate with a cr / line feed
}


void wifi_http_init_file(uint8_t loop,const char *uri,struct wifi_http_state *hs)
{
	//UARTprintf("\n************%d\n",loop);
	//wifi_get_http_headers(uri);
	char *params_ico, *params_jpg;
	if(!Wifly_AP_STA)
	{
	Switch_WiFi_to_COM = 0;
		delayuS(100);
	wifi_sendpage = true;
	params_ico = (char *)strstr(uri, ".ico");
	params_jpg = (char *)strstr(uri, ".jpg");
    if((params_ico != NULL) || (params_jpg != NULL))
    	sendPhoto(loop);
    else
	sendPage(wifly_psFileSys[loop].file_data + wifly_psFileSys[loop].offset, hs);
		//vTaskDelay( 50/portTICK_RATE_MS );
	if((g_sParameters.datachansel==2)||(g_sParameters.datachansel==3)) //1, lan-com;2; wilfi-com;
		{
			Switch_WiFi_to_COM = 1;
			delayuS(100);
		}
	}
//	else
//		Wifly_AP_STA = false;

	wifi_sendpage = false;
	//sbidx = 0;

	//ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);//set wifi gpio5 low
}

void Login_send(void)
{
	//UARTprintf("\n***initial*****\n");
	uint8_t *page;
	page = (uint8_t *)data_login_shtm + 12;
	//wifi_get_http_headers(uri);
	Switch_WiFi_to_COM = 0;
	delayuS(100);
	wifi_sendpage = true;
	//vTaskDelay( 5/portTICK_RATE_MS );
	//sendPage(data_login_shtm + 12, hs);
	while(*page)
	{
		Wifly_CharSend(*page);	// send the character and point to the next one
		page++;
	}
	    Wifly_CharSend('\r');
	if((g_sParameters.datachansel==2)||(g_sParameters.datachansel==3))//1, lan-com;2; wilfi-com;
	{
	Switch_WiFi_to_COM = 1;
		delayuS(100);
	}

	wifi_sendpage = false;
	//sbidx = 0;
	//httpState = HTTP_CONN_OPEN;
	//ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);//set wifi gpio5 low
}


void
wifi_http_find_file(const char *uri, int is_09)
{
  uint8_t loop;
  //struct fs_file *file = NULL;
  char *params;
  char *params_shtm;

  //char *params_htm;
  char *params_cgi;
  //UARTprintf("uri is %s\n",uri);
  unsigned int i;
  int count;
  //unsigned portBASE_TYPE l;
#if LWIP_HTTPD_SSI
  /*
   * By default, assume we will not be processing server-side-includes
   * tags
   */
  //hs->tag_check = false;
#endif /* LWIP_HTTPD_SSI */

  /* Have we been asked for the default root file? */
  if((uri[0] == '/') &&  (uri[1] == 0))
  {
	//log = true;
	Login_send();
	return ;
  }
  else {
	  struct wifi_http_state *hs;
    /* No - we've been asked for a specific file. */
    /* First, isolate the base URI (without any parameters) */
    params = (char *)strchr(uri, '?');
    if (params != NULL) {
      /* URI contains parameters. NULL-terminate the base URI */
      *params = '\0';
      params++;
    }
    /* Does the base URI we have isolated correspond to a CGI handler? */
    params_cgi = (char *)strstr(uri, ".cgi");
    if (params_cgi != NULL) {
      for (i = 0; i < NUM_CONFIG_CGI_URIS; i++) {
        if (strcmp(uri, g_psConfigCGIURIs[i].pcCGIName) == 0) {

        		hs = hs_alloc();
        		alloc_mem = true;
        		if (hs == NULL)
        		{
        		    UARTprintf("http_accept: Out of memory, RST\n");
        		    return;
        		}
        		else
        		{
          //  		UARTprintf("get mem\n");
            		count = wifly_extract_uri_parameters(hs, params);
            		uri = g_psConfigCGIURIs[i].pfnCGIHandler(i, count, hs->params,
                                              hs->param_vals);
            		break;
        		}

        }
      }
    }
/*
    params_htm = (char *)strstr(uri, "index.htm");
    params_shtm = (char *)strstr(uri, ".shtm");

    if((params_shtm != NULL) && logoff_shtm)//intercept .shtm at starting up
    {
    	uri = "/login.shtm";
    }

    if((params_htm != NULL) && logoff_htm)//intercept index.htm when OS running
    {
        uri = "/login.shtm";
    }
    logoff_htm = true;
    UARTprintf("file is %s\n",uri);
*/


    /*file = fs_open(uri);
    if (file == NULL) {
      //file = http_get_404_file(&uri);
    }*/

    params_shtm = (char *)strstr(uri, "/searchWifi.shtm");
    if(params_shtm != NULL)
    	Ret_Search_Page = true;

    for(loop = 0; loop < NUM_WIFLY_FILENAMES; loop++)
          {
            if(strcmp(uri, wifly_psFileSys[loop].name) == 0)
            {
              /*if(strcmp(uri, "/wlanap.shtm") == 0)
            	httpState = HTTP_CONN_OPEN;
              else*/
            	wifi_http_init_file(loop, uri, hs);
              if(alloc_mem == true)
              hs_free(hs);
              alloc_mem = false;
              return;
            }
          }
    //If run here,file requested is not found.
    //httpState = HTTP_CONN_OPEN;
  }
}

void
wifi_http_parse_request(char *wifiRxBuf)
{
	/*
	 *calculate the length of GET "/..." HTTP/1.1
	*/
	char *sp1, *sp2;
	char *data;
	int is_09 = 0;
	u16_t uri_len;
	/*struct wifi_http_state *hs;
	hs = hs_alloc();
	if (hs == NULL) {
	    UARTprintf("http_accept: Out of memory, RST\n");
	    return ;
	  }*/
	data = wifiRxBuf;
	      /* parse method */
	      sp1 = strstr(data, "ET ");
			if(sp1 != NULL)
	        sp1 = sp1 + 2;
	     /* else {
	         null-terminate the METHOD (pbuf is freed anyway wen returning)
	        data[4] = 0;
	        httpState = HTTP_CONN_OPEN;
	        return ;
	      }*/
	      /* if we come here, method is OK, parse URI */
	      sp2 = strstr(sp1 + 1, " ");

	      if (sp2 == NULL) {
	        /* HTTP 0.9: respond with correct protocol version */
	        sp2 = strstr(sp1 + 1, "\r\n");
	        is_09 = 1;
	      }

	      uri_len = sp2 - (sp1 + 1);
	      if ((sp2 != 0) && (sp2 > sp1)) {
	        char *uri = sp1 + 1;
//	        UARTprintf("%s\n",uri);
	        /* null-terminate the METHOD (pbuf is freed anyway wen returning)
			* to here,uri has changed to GET'\0'"/..."'\0'HTTP/1.1
			*/
	        *sp1 = 0;
	        uri[uri_len] = 0;
	        wifi_http_find_file(uri, is_09);
 }
 }

void WiflyPage_TCP_CLOSE(void)
{
		tWiFiCMD pxMessage;
		tWiFiCMD sWiFiCMD;
		UARTprintf("\ncl_cmd_s\n");
		//setup data path;
		save_WiFi_TCP_connected= WiFi_TCP_connected;
		save_Webpage_Visit_Req= Webpage_Visit_Req;
		WiFi_TCP_connected = false;
		Webpage_Visit_Req = false;

		pxMessage.ui8Name = WiFlyPage_TCP_Close;
		xQueueSendToBack( g_QueWiFiCMD, ( void * ) &pxMessage, ( portTickType ) 0 );
		xQueueReceive( g_QueWiFiDataBack, &sWiFiCMD, portMAX_DELAY );

		//resume breakpoint;
		WiFi_TCP_connected= save_WiFi_TCP_connected;
		Webpage_Visit_Req= Webpage_Visit_Req;
		UARTprintf("\ncl_cmd_d\n");
}
//*****************************************************************************
//for wifi web state machine
//*****************************************************************************
void Wifi_Web_Sta_Machine(void)
{
	uint8_t hint;
//	uint32_t wifibuffer_used;
	char previous_char_c;
/*
#if HW_Triger_TCP
                	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5);//set wifi gpio5 high
                	delayms(10);
                	while(GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)&GPIO_PIN_6 != GPIO_PIN_6) {
                		UARTprintf("\n wait gpio6: 1\n");
                	}
 #endif
*/
	while(RingBufUsed(&wifi_pageRxBuf)) //when wifi rx ringbuffer has data, receive it;
	   {
			Wifly_Acc = true;
		 	 hint = (uint8_t)Wifly_CharReceive();

//		 	UARTprintf(">>%d\n",httpState);
			switch(httpState)
			{
            case HTTP_CONN_WAIT:  //0

                if (hint == '~')
                {
                    httpState = HTTP_CONN_WAIT1; // received ~
                    previouschar = '\0';
                }
                else httpState = HTTP_CONN_WAIT;
                break;

            case HTTP_CONN_WAIT1:  //1
            	//********************************************
            	//'c' item for handling received string '~c~ ~o~GET /favicon.ico HTTP/1.1';
            	//********************************************
            	if ((hint == 'o')||(hint == 'c'))
                {
                   	httpState = HTTP_CONN_WAIT2; // received 	~o
                   	previouschar = hint;
                }
                else
                   httpState = HTTP_CONN_WAIT;
                break;
            case HTTP_CONN_WAIT2:  //2
                if (hint == '~')
                {
                	if(previouschar == 'c')
                	{
                		// if still have data in webpage buffer, need to HTTP_CONN_OPEN status again;
                		if(RingBufUsed(&wifi_pageRxBuf))
                		{
                			httpState = HTTP_CONN_OPEN;
                		}
                		else
                		{
                                	WiFi_TCP_connected = false;
                                	Webpage_Visit_Req = false;
                                	httpState = HTTP_CONN_WAIT; // received ~o~ ~c~
                                	previouschar = '\0';
                    			}

                		}
                	else
                	{
                        httpState = HTTP_CONN_OPEN;
                        UARTprintf("\n**tcp OPEN**\n");
                        //sbidx = 0;
                        //httpLine = 0;
                        previouschar = '\0';
                	}


                }
                else
                    httpState = HTTP_CONN_WAIT;
                break;
            case HTTP_CONN_OPEN:  //3
            	//********************************************
            	//Used to handle received string '~c~ ~o~' when last TCP not closed;
            	//********************************************
                if (hint == '~')
                {
       //         	Judge whether received string '~c~';
                	httpState = HTTP_CONN_CLOSE;
                }
                else if (hint == '\r')  // the end of each line received after open,indeed come here
                {

#if HW_Triger_TCP
                	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5);//set wifi gpio5 high
                	delayms(10);
                	while(GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)&GPIO_PIN_6 != GPIO_PIN_6) {
                		UARTprintf("\n wait gpio6: 1\n");
                	}
 #endif

                	wifiRxBuf[sbidx] = '\0';//to end current line
//                    UARTprintf("\nwifibuf is \n%s\n",wifiRxBuf);
                    if(httpCal)
                    	httpLine++;
                    	UARTprintf("httpline is %d\n",httpLine);
                    httpState = HTTP_CONN_HANDLE;
                }
                else
                {
                     // in the middle of each line received after open
                    if (sbidx < (sizeof(wifiRxBuf)-1))//max 512bytes
                    {
                        wifiRxBuf[sbidx++] = hint;
	//					UARTprintf("\nget string is %s\n",wifiRxBuf);
                    }

                    else
                    {
                    	wifiRxBuf[sbidx++] = hint;
                    	wifiRxBuf[sbidx] = '\0';//to end current line
                    	if(httpCal)
                    	   httpLine++;
                    	httpState = HTTP_CONN_HANDLE;
                    }
                }

                break;
            case HTTP_CONN_HANDLE:   //4
            {

            	//int32_t i32Val;
                //get = (char *)strstr(wifiRxBuf, "ET /");
                //ico = (char *)strstr(wifiRxBuf, ".ico");
                if (((char *)strstr(wifiRxBuf, "ET /")) != NULL)
                {
            	UARTprintf("\n**HANDLE**\n");
            	//UARTprintf("\n%c\n",hint );
            	//else

            	wifi_http_parse_request(wifiRxBuf);
            	httpLine = 1;
            	httpCal = true;

            	//memset(wifiRxBuf,0,sizeof(wifiRxBuf));
                }
                else
                {
                	//UARTprintf("\n**NO GET**\n");
                }
                sbidx = 0;
                if(httpLine==3)//)||(httpLine==8)
                {
                	httpLine  = 0;
                	httpCal = false;
                	//delayms(1);
#if HW_Triger_TCP
                	delayms(10);
                	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);
                	delayms(10);
                	while(GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)&GPIO_PIN_6 != 0) {
                		UARTprintf("\n wait gpio6: 0\n");
                	}

					//delayms(1);
					UARTprintf("\n**close TCP**\n");

 #endif
				/*    while(!RingBufEmpty(&wifi_pageTxBuf)){

				    	UARTprintf("\n>:%d\n",RingBufUsed(&wifi_pageTxBuf));
				    	vTaskDelay(1/portTICK_RATE_MS);
				    }*/

					if(Ret_Search_Page)
					{
						//UARTprintf("\n**&**\n");
						vTaskDelay(1000/portTICK_RATE_MS);//1000
						Ret_Search_Page = false;
					}
					vTaskDelay(100/portTICK_RATE_MS);
				    	if(!Wifly_AP_STA)
				    	{
				    		TCP_CLOSE();
				    	}
				    	else
				    		Wifly_AP_STA = false;
	                	UARTprintf("\n**close**\n");
	                	WiFi_TCP_connected = false;
	                	Webpage_Visit_Req = false;
	                	httpState = HTTP_CONN_WAIT; // received ~o~ ~c~
	                	previouschar = '\0';
	                	if(!Find_TCP_open_in_Command)
	                	RingBufFlush(&wifi_pageRxBuf);
	                	Find_TCP_open_in_Command = false;


/*
					  	WiflyPage_TCP_CLOSE();
                     	UARTprintf("\n*cmd_close*\n");
                     	WiFi_TCP_connected = false;
                     	Webpage_Visit_Req = false;
                     	httpState = HTTP_CONN_WAIT; // received ~o~ ~c~
*/

				}
                else
                httpState = HTTP_CONN_OPEN;
                //RingBufFlush(&wifi_pageRxBuf);
            	break;
            }
            case HTTP_CONN_CLOSE:        //5
                if (hint == 'c')
                {
                    httpState = HTTP_CONN_CLOSE1;
                    previous_char_c = hint;
                }

                else
                	// received '~?' still need to save and handel as webpage content;
                {

                    // in the middle of each line received after open
                   if (sbidx < (sizeof(wifiRxBuf)-3))//max 512bytes
                   {
                	   wifiRxBuf[sbidx++] = '~';
                	   wifiRxBuf[sbidx++] = hint;
                       httpState = HTTP_CONN_OPEN;
                   }
                   else
                   {
                	wifiRxBuf[sbidx++] = '~';
                   	wifiRxBuf[sbidx++] = hint;
                   	wifiRxBuf[sbidx] = '\0';//to end current line
                   	httpState = HTTP_CONN_HANDLE;
                   }

                }
                break;
            case HTTP_CONN_CLOSE1:    //6
            {
                if (hint == '~')
                {
                	//if(previous_char_c =='c')
                	//{
                		UARTprintf("\nwebbuf:%d\n",RingBufUsed(&wifi_pageRxBuf));
                		// if still have data in webpage buffer, need to HTTP_CONN_OPEN status again;
                		if(RingBufUsed(&wifi_pageRxBuf))
                		{
                			httpState = HTTP_CONN_OPEN;
                		}
                		else
                		{

//                			if(wifi_sendpage)
  //              				httpState = HTTP_CONN_OPEN;
 //               			else{
                            	UARTprintf("\n**close**\n");
                            	WiFi_TCP_connected = false;
                            	Webpage_Visit_Req = false;
                            	httpState = HTTP_CONN_WAIT; // received ~o~ ~c~
                            	previouschar = '\0';
                			}

//                	}
/*
                	else{
                			UARTprintf("\nrev ~o~\n");
                			httpState = HTTP_CONN_OPEN;
                		}
*/



                }
                else  //received '~c?',still need save and handle;
                {
                    // in the middle of each line received after open
                   if (sbidx < (sizeof(wifiRxBuf)-4))//max 512bytes
                   {
                	   wifiRxBuf[sbidx++] = '~';
                	   wifiRxBuf[sbidx++] = 'c';
                	   wifiRxBuf[sbidx++] = hint;
                       httpState = HTTP_CONN_OPEN;
                   }
                   else
                   {
                	   wifiRxBuf[sbidx++] = '~';
                	   wifiRxBuf[sbidx++] = 'c';
                	   wifiRxBuf[sbidx++] = hint;
                       wifiRxBuf[sbidx] = '\0';//to end current line
                	   httpState = HTTP_CONN_HANDLE;
                   }
                }
                break;
            }
            default: break;
			}
	   }
	Wifly_Acc = false;

}

static void
WiFi_WebAccess_Task(void *pvParameters)
{

	portTickType xLastWakeTime;
	uint8_t	data_rev_qty;
	Wifly_modname_modify = false;
	WiFly_Webaccess_Ready = false; //default false;only setting is TCP server, it will be set to true;
/*
	  if((g_sParameters.sWiFiModule.sWiFiModSel.WiFiTransPcolFlag==WiFi_UDP)
			  &&(g_sParameters.sWiFiModule.sWiFiModSel.ui16DataLocalPort==1901)
			  &&(g_sParameters.sWiFiModule.sWiFiModSel.ui16DataRemotePort==1901)
		//	  &&(wifiCMD_Executing==false)&&(WiFi_Assistant_Set_flag == false)
	  )
	WiFly_Webaccess_Ready = false;
	  else
*/
	if((g_sParameters.sWiFiModule.sWiFiModSel.WiFiTransPcolFlag & WiFi_SERVER) == WiFi_SERVER)
	{
			WiFly_Webaccess_Ready = true;
	}
	  Find_TCP_open_in_Command = false;
//	httpState   = HTTP_CONN_WAIT;
	sbidx = 0;
	//UARTprintf("\nCome into\n");

	xLastWakeTime = xTaskGetTickCount();
   while(1)
   {

	   if(Wifly_modname_modify)
	   {
	   //UARTprintf("\naaaaaaa\n");
#if HW_Triger_TCP
       	ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);
       	while(GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)&GPIO_PIN_6 != 0)
       	{
       		UARTprintf("\n wait gpio6: 0\n");
       	}
#endif
       	SetModnameCMDtoWiFi(g_sParameters.ui8ModName);
       	Wifly_modname_modify = false;
	   }
	   if((WiFly_Webaccess_Ready)&&(Wifi_Set_OK)&&(wifi_restoring == false)&&(wifiCMD_Executing==false)&&(TCP_close_processing==false))
	   {
		   Wifi_Web_Sta_Machine();
	   }
#if TI_Master_Board
		  //wired lan control to com port by UDP;
		  if((slave_data_rdy_syn == 0)&&((Switch_WiFi_to_COM ==0)&&(Switch_LAN_to_COM == 0)))
		  {
		//	  UARTprintf("rec qty string is:%s\n",g_sRxBuf[COM_Port].pui8Buf[g_sRxBuf[COM_Port].ui32ReadIndex]);
			//  UARTprintf("rec qty string is:%s\n",g_sRxBuf[COM_Port].pui8Buf);
			  data_rev_qty = SerialReceiveAvailable(COM_Port);

//			  UARTprintf("rec qty:%d\n",data_rev_qty);
			  if(data_rev_qty>=5) // response to slave read cmd, length is 6;
			  {
				  // sendback flag to locator to unblock;
				  slave_data_rdy_syn =1;
			xQueueSendToBack( g_Que_TI_Master_to_Slave, ( void * ) &slave_data_rdy_syn, ( portTickType ) 0 ); // sync;

			  }
	   }
#endif

	vTaskDelayUntil( &xLastWakeTime, ( 100 / portTICK_RATE_MS ) );
   }
}

//*****************************************************************************
//
// Initialize the task for webpage access through WiFi.
//
//*****************************************************************************
uint32_t
WiFi_WebAccess_TaskInit(void)
{
		//httpMethod  = HTTP_METHOD_FORCE_STARTOVER;
    //
    // Create the Serial task.
    //
    if(xTaskCreate(WiFi_WebAccess_Task, (signed portCHAR *)"WiFi_WebAccess_Task", STACKSIZE_WiflypageTASK, NULL, tskIDLE_PRIORITY + PRIORITY_WiFi_WebAccess, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}
