#if MODBUS_SUPPORT

#define MODBUS_READ_COIL 0x1
#define MODBUS_READ_DISCRETE_INPUT 0x2
#define MODBUS_READ_HOLDING_REGISTERS 0x3
#define MODBUS_READ_INPUT_REGISTERS 0x4
#define MODBUS_WRITE_SINGLE_COIL 0x5
#define MODBUS_WRITE_SINGLE_REGISTERS 0x6
#define MODBUS_WRITE_MULTIPLE_COILS  0x7
#define MODBUS_WRITE_MULTIPLE_REGISTERS 0x8

#define MODBUS_WRITE_MULTIPLE_COILS2  		0xf
#define MODBUS_WRITE_MULTIPLE_REGISTERS2 	0x10
#define MODBUS_READ_FILE					0x14
#define MODBUS_WRITE_FILE 					0x15
#define MODBUS_RST_LINK 					0x19
uint8_t pui8TempRTU[256],i32IndexRTU,Qty_send,cnt_data_for_get_len;
/*
 typedef enum
{
MODBUS_RTU_STATE_IDLE,
MODBUS_RTU_STATE_INIT,
MODBUS_RTU_STATE_FRAME_START,
MODBUS_RTU_STATE_SLAVE_ID,
MODBUS_RTU_STATE_FUNC_ID,
MODBUS_RTU_STATE_FRAME_LENGTH,
MODBUS_RTU_STATE_FRAME_CRC1,
MODBUS_RTU_STATE_FRAME_CRC2,
MODBUS_RTU_STATE_COIL_ADDRESS_HIGH,
MODBUS_RTU_STATE_COIL_ADDRESS_LOW,
MODBUS_RTU_STATE_WRITE_DATA_HIGH,
MODBUS_RTU_STATE_WRITE_DATA_LOW,
MODBUS_RTU_STATE_FRAME_SENT,        
MODBUS_RTU_STATE_INVALID        
}
tMODBUSRTUState;

tMODBUSRTUState modbusRTUState;
*/
#define RTU_MAX_LENGTH  256

unsigned int  g_serialRecvCount;
unsigned char g_mbapRTU[1024];
unsigned char g_slaveAddressRTU;
unsigned char g_funcIDRTU;
unsigned char g_funcIDRTU2;
unsigned char g_funcIDRTU3;

unsigned char g_frameLengthRTU;
unsigned char g_frameLengthRTU2;
unsigned char g_frameLengthRTU3;

unsigned char g_frameDataCount;
unsigned char g_frameDataCount2;
unsigned char g_frameDataCount3;

unsigned char g_crcRTU[2];
unsigned char g_enableTimer;
unsigned char g_timeOut;

unsigned char g_time3Out;
unsigned char g_WiFi_RTU_enableTimer;

void initModbusRTUStatemachine(void);
void ResetModbusRTUStatemachine(void);
void CheckModbusRTUStatemachine(void);
void SetInitModbusRTUStatemachine(void);

void ModbusRTURecvTCPSend(unsigned char ucChar);

void ModbusTimerInit(void);
void ModbusTimerArm(void);
void ModbusTimerDisArm(void);
extern void ModbusPDU_Req_WiFi_Port(unsigned char ucChar);
extern void ModbusRTURecv_COM_Port(unsigned char ucChar);
#endif 
