#include <stdio.h>/*{{{*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/lwiplib.h"
#include "usr_flash.h"
#include "parser.h"
#include "commands.h"
#include "events.h"
#include "state_machine.h"
#include "rs232.h"
#include "leds.h"
#include "buttons.h"
#include "telnet.h"
#include "opt.h"
#include "schedule.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"/*}}}*/
//--------------------------------------------------------------------------------
const State
   Idle2  [ ],
   Console[ ],
   Bridge [ ];

const State*   Rs232_Sm;
struct Parser_Queue_Struct *P;
struct Line_Process_Struct *L;
//------------------------------------------------------------------
void Init_Uart(void)
{
   ROM_SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOA                      );
   ROM_GPIOPinConfigure       ( GPIO_PA0_U0RX                            );
   ROM_GPIOPinConfigure       ( GPIO_PA1_U0TX                            );
   ROM_GPIOPinTypeUART        ( GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1 );

   UARTStdioConfig ( 0, Usr_Flash_Params.Rs232_Baud, configCPU_CLOCK_HZ);
}
//-------------------------------------------------------
void Rs232_Task(void* nil)
{
   Rs232_Sm   = Idle2;
   P=pvPortMalloc(sizeof(struct Parser_Queue_Struct));
   L=pvPortMalloc(sizeof(struct Line_Process_Struct));
   P->CmdTable = Login_Cmd_Table;
   P->tpcb     = UART_MSG;
   P->Ref      = P;
//   Init_Uart ( );
   Cmd_Welcome ( P ,0 ,NULL ); // debug
   while(1) {
      vTaskDelay ( pdMS_TO_TICKS( 250 ));
      Send_Event ( Rti_Event,Rs232( ));
   }
}
//-------------------------------------------------------
const State** Rs232    ( void ) { return &Rs232_Sm;}

void Is_Console_Enabled(void)
{
   Send_Event(Usr_Flash_Params.Rs232_Menu_Enable==true?
         Console_Enable_Event:Console_Disable_Event,
         Rs232());
}

void Clear_Parser_Index   ( void ) {
   P->Index=0;
}

void manageLastInput(struct Parser_Queue_Struct* B)
{
   if(B->Index==0 && B->lastIndex>0) //con esto pregunto si se trata de una linea solo de /n. en ese caso repito el caomando anterior
      B->Index=B->lastIndex;
   else
      B->Buff[B->Index] = '\0';
}
bool manageEnter(uint8_t Char, bool isNext, uint8_t nextChar, uint16_t *index )
{
   if(Char=='\n' || Char=='\r') {
      if(isNext==true) {
         if( (Char=='\n' && nextChar=='\r') ||
             (Char=='\r' && nextChar=='\n'))
            index++;
      }
      return true;
   }
   return false;
}

bool Manage_Backspace(uint8_t Char)
{
   if(Char==0x7F) {
      if(P->Index>0)
         P->Index--;
      return true;
   }
   else
      return false;
}

void Parser_Process(void)
{
   uint8_t Data;
   while(!Rx_Buffer_Empty()) {
      if(P->Index<sizeof(P->Buff)) {
         Data = Read_Next_Char();
         if(manageEnter(Data,!Rx_Buffer_Empty(),Peek_Next_Char(),&P->Index)==true) {
            Insert_Event(Enter_Found_Event,Rs232());
            return;
         }
         if(Manage_Backspace(Data)==false) {
            P->Buff[P->Index++] = Data;
            P->lastIndex       = P->Index;
         }
      }
      else {
         Insert_Event(Max_Length_Event,Rs232());
         return;
      }
   }
}
void Send_Data2Parser(void)
{
   manageLastInput(P);
   P->Id++;
   xQueueSend(Parser_Queue,P,portMAX_DELAY);
   P->Index=0;
}
//------------------------------------------------------------------------------------
void Clear_Line_Index   ( void ) {
   L->Index=0;
}
void Line_Process(void)
{
   uint8_t Char;

   while(!Rx_Buffer_Empty()) {
      L->Tout=0;
      if(L->Index<Usr_Flash_Params.Rs232_Len) {
         Char = Read_Next_Char();
         L->Buff[L->Index++]=Char;
         if(Char==Usr_Flash_Params.Rs232_Term) {
            Insert_Event(Term_Found_Event,Rs232());
            return;
         }
      }
      else {
         Insert_Event(Max_Length_Event,Rs232());
         return;
      }
   }
   if(L->Index>0 && (L->Tout++/4)>=Usr_Flash_Params.Rs232_Tout) {
         Insert_Event(TOut_Event,Rs232());
         L->Tout=0;
   }
}
void Send_Data2Tcp(void)
{
   Send_To_Normal_Tcp(L->Buff,L->Index);
   L->Index=0;
}
//-------------------------------------------------------------------------------------
const State const Idle2  [ ] =
{
   { Rti_Event            ,Is_Console_Enabled ,Idle2   } ,
   { Console_Enable_Event ,Clear_Parser_Index ,Console } ,
   { Conn_Regi_Event      ,Clear_Line_Index   ,Bridge  } ,
   { ANY_Event            ,Rien               ,Idle2   } ,
};
const State Bridge[ ] =
{
   { Rti_Event        ,Line_Process  ,Bridge } ,
   { Term_Found_Event ,Send_Data2Tcp ,Bridge } ,
   { Max_Length_Event ,Send_Data2Tcp ,Bridge } ,
   { TOut_Event       ,Send_Data2Tcp ,Bridge } ,
   { Conn_Free_Event  ,Rien          ,Idle2  } ,
   { ANY_Event        ,Rien          ,Bridge } ,
};
const State Console  [ ] =
{
   { Rti_Event         ,Parser_Process   ,Console } ,
   { Enter_Found_Event ,Send_Data2Parser ,Console } ,
   { TOut_Event        ,Send_Data2Parser ,Console } ,
   { Max_Length_Event  ,Send_Data2Parser ,Console } ,
   { Conn_Regi_Event   ,Clear_Line_Index ,Bridge  } ,
   { ANY_Event         ,Rien             ,Console } ,
};
