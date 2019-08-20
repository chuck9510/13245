
#define MODBUS_SUPPORT 1

#if MODBUS_SUPPORT
 typedef enum
{
MODBUS_STATE_INIT,
MODBUS_STATE_FRAME_START,
MODBUS_STATE_TRANSACTION_IDENTIFIER_1,
MODBUS_STATE_TRANSACTION_IDENTIFIER_2,
MODBUS_STATE_PROTOCOL_IDENTIFIER_1,
MODBUS_STATE_PROTOCOL_IDENTIFIER_2,
MODBUS_STATE_FRAME_LENGTH_1,
MODBUS_STATE_FRAME_LENGTH_2,
MODBUS_STATE_SLAVE_ID,
MODBUS_STATE_FUNCTION_ID,
MODBUS_STATE_ADU_DATA,
MODBUS_STATE_CRC_CALC,
MODBUS_STATE_SEND_SERIAL,
MODBUS_STATE_WAIT_FOR_CLIENT_RESPONSE,
MODBUS_STATE_INVALID
}
tMODBUSState;

extern tMODBUSState modbusState;
void initModbusTCPStatemachine(void);
void ResetModbusTCPStatemachine(void);
int CheckModbusTCPStatemachine(void);
void ModbusCRC(unsigned char *str, unsigned char len);
void ModbusTCPReceiveSerialSend(unsigned long ulPort, unsigned char ucChar);

#endif 