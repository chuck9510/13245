//*****************************************************************************
//
// modbus.c - Modbus session support routines.
//
// Copyright (c) 2014 Texas Instruments Incorporated.  All rights reserved.
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
// Note:  This is a deriviate of the RDK-S2E firmware (telnet.c) modified for
// support of Modbus functions.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "utils/lwiplib.h"
#include "lwip/sys.h"
#include "utils/ustdlib.h"
#include "serial.h"

#include "modbus.h"
#include "user_define.h"


//extern void CheckModbusTCPStatemachine(void);


//*****************************************************************************
//
// The possible states of the TCP session.
//
//*****************************************************************************
typedef enum
{
    //
    // The TCP session is idle.  No connection has been attempted, nor has it
    // been configured to listen on any port.
    //
    STATE_TCP_IDLE,

    //
    // The TCP session is listening (server mode).
    //
    STATE_TCP_LISTEN,

    //
    // The TCP session is connected.
    //
    STATE_TCP_CONNECTED,
}
tTCPState;

//*****************************************************************************
//
// This structure is used holding the state of a given modbus session.
//
//*****************************************************************************
typedef struct
{
    //
    // This value holds the pointer to the TCP PCB associated with this
    // connected modbus session.
    //
    struct tcp_pcb *pConnectPCB;

    //
    // This value holds the pointer to the TCP PCB associated with this
    // listening modbus session.
    //
    struct tcp_pcb *pListenPCB;

    //
    // The current state of the TCP session.
    //
    tTCPState eTCPState;

    //
    // The listen port for the modbus server or the local port for the modbus
    // client.
    //
    unsigned short usModbusLocalPort;

    //
    // The remote port that the modbus client connects to.
    //
    unsigned short usModbusRemotePort;

    //
    // The remote address that the modbus client connects to.
    //
    unsigned long ulModbusRemoteIP;

    //
    // A counter for the TCP connection timeout.
    //
    unsigned long ulConnectionTimeout;

    //
    // The max time for TCP connection timeout counter.
    //
    unsigned long ulMaxTimeout;

    //
    // This value holds an array of pbufs.
    //
    struct pbuf *pBufQ[PBUF_POOL_SIZE];

    //
    // This value holds the read index for the pbuf queue.
    //
    int iBufQRead;

    //
    // This value holds the write index for the pbuf queue.
    //
    int iBufQWrite;

    //
    // This value holds the head of the pbuf that is currently being
    // processed (that has been popped from the queue).
    //
    struct pbuf *pBufHead;

    //
    // This value holds the actual pbuf that is being processed within the
    // pbuf chain pointed to by the pbuf head.
    //
    struct pbuf *pBufCurrent;

    //
    // This value holds the offset into the payload section of the current
    // pbuf.
    //
    unsigned long ulBufIndex;

    //
    // The amount of time passed since rx byte count has changed.
    //
    unsigned long ulLastTCPSendTime;

    //
    // The indication that link layer has been lost.
    //
    bool bLinkLost;

    //
    // Debug and diagnostic counters.
    //
    unsigned char ucErrorCount;
    unsigned char ucReconnectCount;
    unsigned char ucConnectCount;

    //
    // The last error reported by lwIP while attempting to make a connection.
    //
    err_t eLastErr;
}
tModbusSessionData;

//*****************************************************************************
//
// The modbus session data array, for use in the modbus handler function.
//
//*****************************************************************************
static tModbusSessionData g_sModbusSession;

//*****************************************************************************
//
// External Reference to millisecond timer.
//
//*****************************************************************************
extern uint32_t g_ui32SystemTimeMS;

//*****************************************************************************
//
// Free up any queued pbufs associated with at modbus session.
//
// \param pState is the pointer ot the modbus session state data.
//
// This will free up any pbufs on the queue, and any currently active pbufs.
//
// \return None.
//
//*****************************************************************************
static void
ModbusFreePbufs(tModbusSessionData *pState)
{
    SYS_ARCH_DECL_PROTECT(lev);

    //
    // This should be done in a protected/critical section.
    //
    SYS_ARCH_PROTECT(lev);

    //
    // Pop a pbuf off of the rx queue, if one is available, and we are
    // not already processing a pbuf.
    //
    if(pState->pBufHead != NULL)
    {
        pbuf_free(pState->pBufHead);
        pState->pBufHead = NULL;
        pState->pBufCurrent = NULL;
        pState->ulBufIndex = 0;
    }

    while(pState->iBufQRead != pState->iBufQWrite)
    {
        pbuf_free(pState->pBufQ[pState->iBufQRead]);
        pState->iBufQRead = ((pState->iBufQRead + 1) % PBUF_POOL_SIZE);
    }

    //
    // Restore previous level of protection.
    //
    SYS_ARCH_UNPROTECT(lev);
}

//*****************************************************************************
//
// Processes a character received from the modbus port.
//
// \param ucChar is the character in question.
// \param pState is the modbus state data for this connection.
//
// This function processes a character received from the modbus port, handling
// the interpretation of modbus commands (as indicated by the modbus interpret
// as command (IAC) byte).
//
// \return None.
//
//*****************************************************************************
static void
ModbusProcessCharacter(unsigned char ucChar, tModbusSessionData *pState)
{
    //
    // New packet comes before the response is sent.
    // 
    if(CheckModbusTCPStatemachine()) // modbus is not in waiting for client response status;
    {
        ModbusTCPReceiveSerialSend(0, ucChar);
    //	ModbusTCPReceiveSerialSend(3, ucChar);
    }

    //
    // And return.
    //
    return;
}

//*****************************************************************************
//
// Receives a TCP packet from lwIP for the modbus server.
//
// \param arg is the modbus state data for this connection.
// \param pcb is the pointer to the TCP control structure.
// \param p is the pointer to the pbuf structure containing the packet data.
// \param err is used to indicate if any errors are associated with the
// incoming packet.
//
// This function is called when the lwIP TCP/IP stack has an incoming packet
// to be processed.
//
// \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t
ModbusReceive(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    tModbusSessionData *pState = arg;
    int iNextWrite;
    SYS_ARCH_DECL_PROTECT(lev);

    //
    // Place the incoming packet onto the queue if there is space.
    //
    if((err == ERR_OK) && (p != NULL))
    {
        //
        // This should be done in a protected/critical section.
        //
        SYS_ARCH_PROTECT(lev);

        //
        // Do we have space in the queue?
        //
        iNextWrite = ((pState->iBufQWrite + 1) % PBUF_POOL_SIZE);
        if(iNextWrite == pState->iBufQRead)
        {
            //
            // The queue is full - discard the pbuf and return since we can't
            // handle it just now.
            //

            //
            // Restore previous level of protection.
            //
            SYS_ARCH_UNPROTECT(lev);

            //
            // Free up the pbuf.  Note that we don't acknowledge receipt of
            // the data since we want it to be retransmitted later.
            //
            pbuf_free(p);
        }
        else
        {
            //
            // Place the pbuf in the circular queue.
            //
            pState->pBufQ[pState->iBufQWrite] = p;

            //
            // Increment the queue write index.
            //
            pState->iBufQWrite = iNextWrite;

            //
            // Restore previous level of protection.
            //
            SYS_ARCH_UNPROTECT(lev);
        }
    }

    //
    // If a null packet is passed in, close the connection.
    //
    else if((err == ERR_OK) && (p == NULL))
    {
        //
        // Clear out all of the TCP callbacks.
        //
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_poll(pcb, NULL, 1);

        //
        // Close the TCP connection.
        //
        tcp_close(pcb);

        //
        // Clear out any pbufs associated with this session.
        //
        ModbusFreePbufs(pState);

        //
        // Clear out the modbus session PCB.
        //
        pState->pConnectPCB = NULL;

        //
        // Revert to listening state.
        //
        pState->eTCPState = STATE_TCP_LISTEN;
    }

    //
    // Return okay.
    //
    return(ERR_OK);
}

//*****************************************************************************
//
// Handles lwIP TCP/IP errors.
//
// \param arg is the modbus state data for this connection.
// \param err is the error that was detected.
//
// This function is called when the lwIP TCP/IP stack has detected an error.
// The connection is no longer valid.
//
// \return None.
//
//*****************************************************************************
static void
ModbusError(void *arg, err_t err)
{
    tModbusSessionData *pState = arg;

    //
    // Increment our error counter.
    //
    pState->ucErrorCount++;
    pState->eLastErr = err;

    //
    // Free the pbufs associated with this session.
    //
    ModbusFreePbufs(pState);

    //
    // Reinitialize the server state to wait for incoming connections.
    //
    pState->pConnectPCB = NULL;
    pState->eTCPState = STATE_TCP_LISTEN;
    pState->ulConnectionTimeout = 0;
    pState->iBufQRead = 0;
    pState->iBufQWrite = 0;
    pState->pBufHead = NULL;
    pState->pBufCurrent = NULL;
    pState->ulBufIndex = 0;
    pState->ulLastTCPSendTime = 0;
    pState->bLinkLost = false;
}

//*****************************************************************************
//
// Handles lwIP TCP/IP polling and timeout requests.
//
// \param arg is the modbus state data for this connection.
// \param pcb is the pointer to the TCP control structure.
//
// This function is called periodically and is used to re-establish dropped
// client connections and to reset idle server connections.
//
// \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t
ModbusPoll(void *arg, struct tcp_pcb *pcb)
{
    tModbusSessionData *pState = arg;

    //
    // We are operating as a server. Increment the timeout value and close
    // the modbus connection if the configured timeout has been exceeded.
    //
    pState->ulConnectionTimeout++;
    if((pState->ulMaxTimeout != 0) &&
       (pState->ulConnectionTimeout > pState->ulMaxTimeout))
    {
        //
        // Close the modbus connection.
        //
        tcp_abort(pcb);
    }

    //
    // Return OK.
    //
    return(ERR_OK);
}

//*****************************************************************************
//
// Handles acknowledgment of data transmitted via Ethernet.
//
// \param arg is the modbus state data for this connection.
// \param pcb is the pointer to the TCP control structure.
// \param len is the length of the data transmitted.
//
// This function is called when the lwIP TCP/IP stack has received an
// acknowledgment for data that has been transmitted.
//
// \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t
ModbusSent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    tModbusSessionData *pState = arg;

    //
    // Reset the connection timeout.
    //
    pState->ulConnectionTimeout = 0;

    //
    // Return OK.
    //
    return(ERR_OK);
}

//*****************************************************************************
//
// Accepts a TCP connection for the modbus port.
//
// \param arg is the modbus state data for this connection.
// \param pcb is the pointer to the TCP control structure.
// \param err is not used in this implementation.
//
// This function is called when the lwIP TCP/IP stack has an incoming
// connection request on the modbus port.
//
// \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t
ModbusAccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    tModbusSessionData *pState = arg;

    //
    // If we are not in the listening state, refuse this connection.
    //
    if(pState->eTCPState != STATE_TCP_LISTEN)
    {
        //
        // If we haven't lost the link, then refuse this connection.
        //
        if(!pState->bLinkLost)
        {
            //
            // If we already have a connection, kill it and start over.
            //
            return(ERR_CONN);
        }

        //
        // Reset the flag for next time.
        //
        pState->bLinkLost = false;

        //
        // Abort the existing TCP connection.
        //
        tcp_abort(pState->pConnectPCB);

        //
        // Clear out any pbufs associated with this session.
        //
        ModbusFreePbufs(pState);

        //
        // Clear out the modbus session PCB.
        //
        pState->pConnectPCB = NULL;

    }

    //
    // Save the PCB for future reference.
    //
    pState->pConnectPCB = pcb;

    //
    // Change TCP state to connected.
    //
    pState->eTCPState = STATE_TCP_CONNECTED;

    //
    // Acknowledge that we have accepted this connection.
    //
    tcp_accepted(pcb);

    //
    // Reset the serial port associated with this session to its default
    // parameters.
    //
    SerialSetDefault(COM_Port);

    //
    // Set the connection timeout to 0.
    //
    pState->ulConnectionTimeout = 0;

    //
    // Setup the TCP connection priority.
    //
    tcp_setprio(pcb, TCP_PRIO_MIN);

    //
    // Setup the TCP receive function.
    //
    tcp_recv(pcb, ModbusReceive);

    //
    // Setup the TCP error function.
    //
    tcp_err(pcb, ModbusError);

    //
    // Setup the TCP polling function/interval.
    //
    tcp_poll(pcb, ModbusPoll, (1000 / TCP_SLOW_INTERVAL));

    //
    // Setup the TCP sent callback function.
    //
    tcp_sent(pcb, ModbusSent);

    //
    // Return a success code.
    //
    return(ERR_OK);
}

//*****************************************************************************
//
// Closes an existing Ethernet connection.
//
// This function is called when the the Modbus/TCP session associated with
// the specified serial port is to be closed.
//
// \return None.
//
//*****************************************************************************
void
ModbusClose(void)
{
    tModbusSessionData *pState;

    //
    // Check the arguments.
    //
    pState = &g_sModbusSession;

    //
    // If we have a connect PCB, close it down.
    //
    if(pState->pConnectPCB != NULL)
    {
        //
        // Clear out all of the TCP callbacks.
        //
        tcp_arg(pState->pConnectPCB, NULL);
        tcp_sent(pState->pConnectPCB, NULL);
        tcp_recv(pState->pConnectPCB, NULL);
        tcp_err(pState->pConnectPCB, NULL);
        tcp_poll(pState->pConnectPCB, NULL, 1);

        //
        // Close the TCP connection.
        //
        tcp_close(pState->pConnectPCB);

        //
        // Clear out any pbufs associated with this session.
        //
        ModbusFreePbufs(pState);
    }

    //
    // If we have a listen PCB, close it down as well.
    //
    if(pState->pListenPCB != NULL)
    {
        //
        // Close the TCP connection.
        //
        tcp_close(pState->pListenPCB);

        //
        // Clear out any pbufs associated with this session.
        //
        ModbusFreePbufs(pState);
    }

    //
    // Reset the session data for this port.
    //
    pState->pConnectPCB = NULL;
    pState->pListenPCB = NULL;
    pState->eTCPState = STATE_TCP_IDLE;
    pState->ulConnectionTimeout = 0;
    pState->ulMaxTimeout = 0;
    pState->iBufQRead = 0;
    pState->iBufQWrite = 0;
    pState->pBufHead = NULL;
    pState->pBufCurrent = NULL;
    pState->ulBufIndex = 0;
    pState->ulLastTCPSendTime = 0;
    pState->bLinkLost = false;
}

//*****************************************************************************
//
// Opens a modbus server session (listen).
//
// \param usModbusPort is the modbus port number to listen on.
//
// This function establishes a TCP session in listen mode as a modbus server.
//
// \return None.
//
//*****************************************************************************
void
ModbusListen(unsigned short usModbusPort)
{
    void *pcb;
    tModbusSessionData *pState;

    //
    // Check the arguments.
    //
    ASSERT(usModbusPort != 0);
    pState = &g_sModbusSession;

    //
    // Fill in the modbus state data structure for this session in listen
    // (in other words, server) mode.
    //
    pState->pConnectPCB = NULL;
    pState->eTCPState = STATE_TCP_LISTEN;
    pState->ulConnectionTimeout = 0;
    pState->ulMaxTimeout = 0;
    pState->usModbusLocalPort = usModbusPort;
    pState->usModbusRemotePort = 0;
    pState->ulModbusRemoteIP = 0;
    pState->iBufQRead = 0;
    pState->iBufQWrite = 0;
    pState->pBufHead = NULL;
    pState->pBufCurrent = NULL;
    pState->ulBufIndex = 0;
    pState->ulLastTCPSendTime = 0;
    pState->bLinkLost = false;

    //
    // Initialize the application to listen on the requested modbus port.
    //
    pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, usModbusPort);
    pcb = tcp_listen(pcb);
    pState->pListenPCB = pcb;

    //
    // Save the requested information and set the TCP callback functions
    // and arguments.
    //
    tcp_arg(pcb, pState);
    tcp_accept(pcb, ModbusAccept);
}

//*****************************************************************************
//
// Gets the current local port for a connection's modbus session.
//
// This function returns the local port in use by the modbus session
// associated with the given serial port.  If operating as a modbus server,
// this port is the port that is listening for an incoming connection.  If
// operating as a modbus client, this is the local port used to connect to
// the remote server.
//
// \return None.
//
//*****************************************************************************
unsigned short
ModbusGetLocalPort(void)
{
    return(g_sModbusSession.usModbusLocalPort);
}

//*****************************************************************************
//
// Gets the current remote port for a connection's modbus session.
//
// This function returns the remote port in use by the modbus session
// associated with the given serial port.  If operating as a modbus server,
// this function will return 0.  If operating as a modbus client, this is the
// server port that the connection is using.
//
// \return None.
//
//*****************************************************************************
unsigned short
ModbusGetRemotePort(void)
{
    return(g_sModbusSession.usModbusRemotePort);
}

//*****************************************************************************
//
// Initializes the modbus session(s) for the Serial to Ethernet Module.
//
// This function initializes the modbus session data parameter block.
//
// \return None.
//
//*****************************************************************************
void
ModbusInit(void)
{
    //
    // Initialize the session data for each supported port.
    //
    g_sModbusSession.pConnectPCB = NULL;
    g_sModbusSession.pListenPCB = NULL;
    g_sModbusSession.eTCPState = STATE_TCP_IDLE;
    g_sModbusSession.ulConnectionTimeout = 0;
    g_sModbusSession.ulMaxTimeout = 0;
    g_sModbusSession.usModbusRemotePort = 0;
    g_sModbusSession.usModbusLocalPort = 0;
    g_sModbusSession.ulModbusRemoteIP = 0;
    g_sModbusSession.iBufQRead = 0;
    g_sModbusSession.iBufQWrite = 0;
    g_sModbusSession.pBufHead = NULL;
    g_sModbusSession.pBufCurrent = NULL;
    g_sModbusSession.ulBufIndex = 0;
    g_sModbusSession.ulLastTCPSendTime = 0;
    g_sModbusSession.bLinkLost = false;
    g_sModbusSession.ucConnectCount = 0;
    g_sModbusSession.ucReconnectCount = 0;
    g_sModbusSession.ucErrorCount = 0;
    g_sModbusSession.eLastErr = ERR_OK;
}

//*****************************************************************************
//
// Handles periodic task for modbus sessions.
//
// This function is called periodically from the lwIP timer thread context.
// This function will handle transferring data between the UART and the
// the modbus sockets.  The time period for this should be tuned to the UART
// ring buffer sizes to maintain optimal throughput.
//
// \return None.
//
//*****************************************************************************
void
ModbusHandler(void)
{
	bool testflag;
    long lCount, lIndex;
    static unsigned char pucTemp[PBUF_POOL_BUFSIZE];
    SYS_ARCH_DECL_PROTECT(lev);
    unsigned char *pucData;
    tModbusSessionData *pState;

    //
    // Initialize the state pointer.
    //
    pState = &g_sModbusSession;

    //
    // If the modbus session is not connected, skip this port.
    //
    if(pState->eTCPState != STATE_TCP_CONNECTED)
    {
        return;
    }

    //
    // While space is available in the serial output queue, process the
    // pbufs received on the modbus interface.
    //
    while(!SerialSendFull(COM_Port))
    {
        //
        // Pop a pbuf off of the rx queue, if one is available, and we are
        // not already processing a pbuf.
        //
        if(pState->pBufHead == NULL)
        {
            if(pState->iBufQRead != pState->iBufQWrite)
            {
                SYS_ARCH_PROTECT(lev);
                pState->pBufHead = pState->pBufQ[pState->iBufQRead];
                pState->iBufQRead =
                    ((pState->iBufQRead + 1) % PBUF_POOL_SIZE);
                pState->pBufCurrent = pState->pBufHead;
                pState->ulBufIndex = 0;
                SYS_ARCH_UNPROTECT(lev);
            }
        }

        //
        // If there is no packet to be processed, break out of the loop.
        //
        if(pState->pBufHead == NULL)
        {
            break;
        }

        //
        // Setup the data pointer for the current buffer.
        //
        pucData = pState->pBufCurrent->payload;

        //
        // Process the next character in the buffer.
        //
        ModbusProcessCharacter(pucData[pState->ulBufIndex], pState);

        //
        // Increment to next data byte.
        //
        pState->ulBufIndex++;

        //
        // Check to see if we are at the end of the current buffer.  If so,
        // get the next pbuf in the chain.
        //
        if(pState->ulBufIndex >= pState->pBufCurrent->len)
        {
            pState->pBufCurrent = pState->pBufCurrent->next;
            pState->ulBufIndex = 0;
        }

        //
        // Check to see if we are at the end of the chain.  If so,
        // acknowledge this data as being consumed to open up the TCP
        // window.
        //
        if((pState->pBufCurrent == NULL) && (pState->ulBufIndex == 0))
        {
            tcp_recved(pState->pConnectPCB, pState->pBufHead->tot_len);
            pbuf_free(pState->pBufHead);
            pState->pBufHead = NULL;
            pState->pBufCurrent = NULL;
            pState->ulBufIndex = 0;
        }
    }

    //
    // Flush the TCP output buffer, in the event that data was
    // queued up by processing the incoming packet.
    //
    tcp_output(pState->pConnectPCB);


    testflag = SerialReceiveAvailable(COM_Port) &&
           tcp_sndbuf(pState->pConnectPCB) &&
           (pState->pConnectPCB->snd_queuelen < TCP_SND_QUEUELEN);
		   UARTprintf("logic: %d\n",testflag);
    //
    // Process the RX ring buffer data if space is available in the
    // TCP output buffer.
    //
		   /*
    if(SerialReceiveAvailable(COM_Port) &&
       tcp_sndbuf(pState->pConnectPCB) &&
       (pState->pConnectPCB->snd_queuelen < TCP_SND_QUEUELEN))
    	*/
		   if(testflag && (modbusRTUState == MODBUS_RTU_STATE_IDLE))
    {
        //
        // Here, we have data, and we have space.  Set the total amount
        // of data we will process to the lesser of data available or
        // space available.
        //
        lCount = (long)SerialReceiveAvailable(COM_Port);
        UARTprintf("Data from RTU series port quantities: %d\n",lCount);

        if(tcp_sndbuf(pState->pConnectPCB) < lCount)
        {
            lCount = tcp_sndbuf(pState->pConnectPCB);
        }

        //
        // While we have data remaining to process, continue in this
        // loop.
        //
        while((lCount) &&
              (pState->pConnectPCB->snd_queuelen < TCP_SND_QUEUELEN))
        {
            //
            // First, reset the index into the local buffer.
            //
            lIndex = 0;

            //
            // Fill the local buffer with data while there is data
            // and/or space remaining.
            //
   //         UARTprintf("Data from RTU series port:");
            while(lCount && (lIndex < sizeof(pucTemp)))
            {
                pucTemp[lIndex] = SerialReceive(COM_Port);
   //             UARTprintf("%d ",pucTemp[lIndex]);
                lIndex++;
                lCount--;
            }
 //           pucTemp[lIndex] = '\0';
 //           UARTprintf("\n");
            //
            // Write the local buffer into the TCP buffer.
            //
            tcp_write(pState->pConnectPCB, pucTemp, lIndex, 1);
        }

        //
        // Flush the data that has been written into the TCP output
        // buffer.
        //
        tcp_output(pState->pConnectPCB);
        pState->ulLastTCPSendTime = g_ui32SystemTimeMS;
    }
}

//*****************************************************************************
//
// Handles link status notification.
//
// \param bLinkStatusUp is the boolean indicate of link layer status.
//
// This function should be called when the link layer status has changed.
//
// \return None.
//
//*****************************************************************************
void
ModbusNotifyLinkStatus(bool bLinkStatusUp)
{
    //
    // We don't care if the link is up, only if it goes down.
    //
    if(bLinkStatusUp)
    {
        return;
    }

    //
    // For every port, indicate that the link has been lost.
    //
    g_sModbusSession.bLinkLost = true;
}
