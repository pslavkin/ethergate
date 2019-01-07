//*****************************************************************************
//
// uartstdio.h - Prototypes for the UART console functions.
//
// Copyright (c) 2007-2017 Texas Instruments Incorporated.  All rights reserved.
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
// This is part of revision 2.1.4.178 of the Tiva Utility Library.
//
//*****************************************************************************

#ifndef __UARTSTDIO_H__
#define __UARTSTDIO_H__

#include "utils/lwiplib.h"
#include <stdarg.h>
#include <stdbool.h>
#include "app/h/opt.h"

#define DEBUG_MSG ((struct tcp_pcb*)(NULL))
#define UART_MSG ((struct tcp_pcb*)(1))
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// If built for buffered operation, the following labels define the sizes of
// the transmit and receive buffers respectively.
//
//*****************************************************************************
#ifdef UART_BUFFERED
#ifndef UART_RX_BUFFER_SIZE
#define UART_RX_BUFFER_SIZE     128
#endif
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE     256
#endif
#endif

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern SemaphoreHandle_t Uart_Studio_Semphr;

extern void             UARTStdioConfig                  ( uint32_t ui32Port, uint32_t ui32Baud, uint32_t ui32SrcClock );
uint16_t                UARTget_Til_Tout_Or_Len_Or_Term  ( uint8_t *Buf, uint16_t Length, uint16_t Tout,uint16_t Term  );
uint16_t                UARTgets_Til_Tout_Or_Len_Or_Term ( uint8_t *Buf,
                                         uint16_t Length,
                                         uint16_t Tout,
                                         uint16_t Term                                                                 );
void                    Emulate_Uart_Rx_Data             ( uint8_t* Data, uint16_t Len                                 );
bool                    UARTgets                         ( uint8_t *pcBuf, uint8_t Max_Length, uint8_t* Index          );
extern unsigned char    UARTgetc                         ( void                                                        );
extern void             UARTprintf                       ( const char *pcString, ...                                   );
extern void             UART_ETHprintf                   ( struct tcp_pcb* tpcb,const char *pcString, ...              );
extern void             UARTprintf                       ( const char *pcString, ...                                   );
extern int              UARTwrite                        ( const char *pcBuf, uint32_t ui32Len                         );
#ifdef UART_BUFFERED
bool                    Rx_Buffer_Empty                  ( void                                                        );
uint8_t                 Read_Next_Char                   ( void                                                        );
uint8_t                 Peek_Next_Char                   ( void                                                        );
uint32_t                Get_Buff_Count                   ( void                                                        );
extern int              UARTPeek                         ( unsigned char ucChar                                        );
extern void             UARTFlushTx                      ( bool bDiscard                                               );
extern void             UARTFlushRx                      ( void                                                        );
extern int              UARTRxBytesAvail                 ( void                                                        );
extern int              UARTTxBytesFree                  ( void                                                        );
extern void             UARTEchoSet                      ( bool bEnable                                                );
#endif

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __UARTSTDIO_H__
