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
#include "utils/ringbuf.h"
#include "serial.h"
#include "modbus.h"




tMODBUSState modbusState;

#define ADU_MAX_LENGTH  263

//6 bytes of header from MBAH
unsigned char g_mbapTransactionIdentifier[2];
unsigned char g_mbapProtocolIdentifier[2];
unsigned int  g_mbapADULength[2];
unsigned char g_mbapSlaveAddress;
unsigned char g_mbapFunctionId;
unsigned int  g_dataLength;
unsigned int  g_datarecv;

// To store the payload of the packets
unsigned char g_mbapADU[ADU_MAX_LENGTH];

unsigned char g_highCrcbyte;
unsigned char g_lowCrcbyte;


#if MODBUS_SUPPORT
extern void SetInitModbusRTUStatemachine(void);
extern void ModbusTimerArm(void);

extern void RS485_TxEnable(uint32_t ui32Port);
extern void RS485_TxDisable(uint32_t ui32Port);

extern int UARTwrite(const char *pcBuf, uint32_t ui32Len);

int CheckModbusTCPStatemachine()
{
if(modbusState !=  MODBUS_STATE_WAIT_FOR_CLIENT_RESPONSE)
	return 1;
else
	return 0;
	
}


 void ResetModbusTCPStatemachine()
{
 modbusState = MODBUS_STATE_INIT;
}

void initModbusTCPStatemachine()
{
 int i;

 g_mbapTransactionIdentifier[0] = 0;
 g_mbapTransactionIdentifier[1] = 0;
 g_mbapProtocolIdentifier[0] = 0;
 g_mbapProtocolIdentifier[1] = 0;
 g_mbapADULength[0] = 0;
 g_mbapADULength[1] = 0;
 g_dataLength = 0;
 g_lowCrcbyte = 0;
 g_highCrcbyte = 0;
 g_datarecv = 0;
 g_mbapSlaveAddress = 0;
 g_mbapFunctionId = 0;

 for(i=0;i<ADU_MAX_LENGTH;i++)
 	  g_mbapADU[i] = 0;

}


void ModbusTCPReceiveSerialSend(unsigned long ulPort, unsigned char ucChar)
{
   int i ;
	// Check the arguments.
    //
    ASSERT(ulPort == 0);

	switch(modbusState)
	{
		case  MODBUS_STATE_INIT:
				initModbusTCPStatemachine();
				modbusState = MODBUS_STATE_TRANSACTION_IDENTIFIER_1;

		case  MODBUS_STATE_TRANSACTION_IDENTIFIER_1:
							g_mbapTransactionIdentifier[0] = ucChar;
							modbusState = MODBUS_STATE_TRANSACTION_IDENTIFIER_2;
							break;

		case  MODBUS_STATE_TRANSACTION_IDENTIFIER_2:
							g_mbapTransactionIdentifier[1] = ucChar;
							modbusState = MODBUS_STATE_PROTOCOL_IDENTIFIER_1;
							break;

		case  MODBUS_STATE_PROTOCOL_IDENTIFIER_1:
								g_mbapProtocolIdentifier[0] = ucChar;
								modbusState = MODBUS_STATE_PROTOCOL_IDENTIFIER_2;
								break;

		case  MODBUS_STATE_PROTOCOL_IDENTIFIER_2:
							g_mbapProtocolIdentifier[1] = ucChar;
							if(!g_mbapProtocolIdentifier[0] && !g_mbapProtocolIdentifier[1])
								modbusState = MODBUS_STATE_FRAME_LENGTH_1;
							else
								modbusState = MODBUS_STATE_INIT;
							break;

		case  MODBUS_STATE_FRAME_LENGTH_1:
							g_mbapADULength[0] = ucChar;
							modbusState = MODBUS_STATE_FRAME_LENGTH_2;
							break;

		case  MODBUS_STATE_FRAME_LENGTH_2:
							g_mbapADULength[1] = ucChar;
					    	g_dataLength = 	g_mbapADULength[1] << 256 + g_mbapADULength[0];
							modbusState = MODBUS_STATE_SLAVE_ID;
							break;

		case  MODBUS_STATE_SLAVE_ID:
							g_mbapSlaveAddress = ucChar;
							g_mbapADU[g_datarecv] = ucChar;
							g_datarecv++;
							modbusState = MODBUS_STATE_FUNCTION_ID;
							break;
		
		case  MODBUS_STATE_FUNCTION_ID:
							g_mbapFunctionId = ucChar;
							g_mbapADU[g_datarecv] = ucChar;
							g_datarecv++;
							modbusState = MODBUS_STATE_ADU_DATA;
							break;


 		case  MODBUS_STATE_ADU_DATA:
							g_mbapADU[g_datarecv] = ucChar;
							g_datarecv++;
							if(	g_datarecv >= g_dataLength)
								modbusState = MODBUS_STATE_CRC_CALC;
							else
								break;

		case  MODBUS_STATE_CRC_CALC:
							ModbusCRC(g_mbapADU, g_dataLength);
								modbusState = MODBUS_STATE_SEND_SERIAL;

		case  MODBUS_STATE_SEND_SERIAL:
						UARTIntDisable(UART1_BASE, UART_INT_TX);

				//		RS485_TxEnable(ulPort);
				//		SysCtlDelay(500);

						for(i = 0; i < g_dataLength; i++)
                       	{
							//SerialSend(g_mbapADU[i]);

					//		SysCtlDelay(5);
					//		UARTCharPut(UART1_BASE, g_mbapADU[i]);
							SerialSend(0,g_mbapADU[i]);
			//				UARTprintf("\nsend char: 0x%X\n",g_mbapADU[i]);
					//		SysCtlDelay(1);

                       	}

						//SerialSend(g_lowCrcbyte);
					//	RS485_TxEnable(ulPort);
					//	UARTCharPut(UART1_BASE, g_lowCrcbyte);
						SerialSend(0,g_lowCrcbyte);
			//			UARTprintf("\nsend char: 0x%X\n",g_lowCrcbyte);
					//	UARTprintf("send to ur lowbyte: %d\n",g_lowCrcbyte);

						//SerialSend(g_highCrcbyte);

					//	UARTCharPut(UART1_BASE, g_highCrcbyte);
						SerialSend(0,g_highCrcbyte);
			//			UARTprintf("\nsend char: 0x%X\n",g_highCrcbyte);
					//	UARTprintf("send to ur hibyte: %d\n",g_highCrcbyte);


					//	RS485_TxDisable(ulPort);
						UARTIntDisable(UART1_BASE, UART_INT_TX);

						 SetInitModbusRTUStatemachine();

		modbusState = MODBUS_STATE_WAIT_FOR_CLIENT_RESPONSE;


		case  MODBUS_STATE_WAIT_FOR_CLIENT_RESPONSE:
							 ModbusTimerArm();
								
										break;

		default:
								break;
	}

   return;

}


static const unsigned char CRCHighTable[] = {
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 };

static const unsigned char CRCLowTable[] = {
   0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
   0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
   0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
   0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
   0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
   0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
   0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
   0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
   0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
   0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
   0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
   0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
   0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
   0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
   0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
   0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40 };

/*****************************************************************************************************/
void ModbusCRC(unsigned char *str, unsigned char len)
{
    unsigned char temp;
    g_lowCrcbyte = 0xFF; g_highCrcbyte = 0xFF;

    while( len--)
        {
            temp = *str++ ^ g_lowCrcbyte;
            g_lowCrcbyte = CRCHighTable[temp] ^ g_highCrcbyte;
            g_highCrcbyte = CRCLowTable[temp];
        }
}

void Cal_CRC_and_append(unsigned char *str, unsigned char len)
{
    unsigned char temp;
    g_lowCrcbyte_TI_Master = 0xFF; g_highCrcbyte_TI_Master = 0xFF;

    while( len--)
        {
            temp = *str++ ^ g_lowCrcbyte_TI_Master;
            g_lowCrcbyte_TI_Master = CRCHighTable[temp] ^ g_highCrcbyte_TI_Master;
            g_highCrcbyte_TI_Master = CRCLowTable[temp];
        }

}
 #endif

