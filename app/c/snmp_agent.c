#include <stdbool.h>/*{{{*/
#include <stdint.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/rom_map.h"
#include "utils/lwiplib.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/cmdline.h"
#include "drivers/pinout.h"
#include "commands.h"
#include "opt.h"
#include "commands.h"
#include "udp.h"
#include "snmp_agent.h"
#include "state_machine.h"
#include "events.h"
#include "one_wire_network.h"
#include "usr_flash.h"/*}}}*/

const State*      Snmp_Agent_Sm;
struct pbuf*      Snmp_Data=NULL;
struct udp_pcb    Snmp_Pcb;
ip_addr_t         Snmp_Addr;
u16_t             Snmp_Port;
//------------------------------------------------------------------------------
char * Udp_Rx_Payload                        ( void ) { return Snmp_Data->payload                                                                                  ;}
struct Snmp_Header_Struct*    Rx_Snmp_Header ( void ) { return (struct Snmp_Header_Struct*) Udp_Rx_Payload()                                                       ;}
struct Snmp_Msg_Struct*       Rx_Snmp_Msg    ( void ) { return (struct Snmp_Msg_Struct*)    (Rx_Snmp_Header()->Community+Rx_Snmp_Header()->Community_Length)       ;}
struct Snmp_Object_Struct*    Rx_Snmp_Object ( void ) { return (struct Snmp_Object_Struct*)   (Rx_Snmp_Msg()->Request_Id+Rx_Snmp_Msg()->Request_Id_Length)         ;}
struct Snmp_Value_Struct*     Rx_Snmp_Value  ( void ) { return (struct Snmp_Value_Struct*)     (Rx_Snmp_Object()->Object_Name+Rx_Snmp_Object()->Object_Name_Length);}
//------------------------------------------------------------------------------
bool  Parse_Version   ( void ) { return Rx_Snmp_Header()->Version==0 || Rx_Snmp_Header()->Version==1       ;}
bool  Parse_Community ( void ) { return strncmp(Rx_Snmp_Header()->Community,Usr_Flash_Params.Snmp_Community,Rx_Snmp_Header()->Community_Length)==0;}
bool  Object_Name_Match ( uint8_t* Index ) {
   uint8_t i;
   if(Rx_Snmp_Object( )->Object_Name_Length!=Usr_Flash_Params.Snmp_Iso_Len)
      return false;
   for(i=0;i<MAX_ROM_CODES &&
           memcmp(Usr_Flash_Params.Snmp_Iso[i],
                  Rx_Snmp_Object( )->Object_Name,
                  Usr_Flash_Params.Snmp_Iso_Len)!=0 ;i++)
      ;
   *Index=i;
  return i<MAX_ROM_CODES;
}
bool  Next_Or_Bulk ( void ) { return Rx_Snmp_Msg( )->Msg_Code==Get_Next_Request_Event || Rx_Snmp_Msg( )->Msg_Code==Get_Bulk_Request_Event;}
//------------------------------------------------------------------------------
void Snmp_Packet_Arrived (struct udp_pcb *upcb, struct pbuf *p, ip_addr_t* addr,  u16_t port)
{
   if(Snmp_Data!=NULL) { //si eta operando en un mensaje anterior.. tira el nuevo.. solo piuedo procesar 1 peticion de snmp a la vez
      pbuf_free(p);
      return;
   }
//   struct pbuf* q = pbuf_alloc(PBUF_RAW,2,PBUF_RAM);
//   pbuf_chain(p,q);
   Snmp_Data=p; // me guardo en una local el puntero al mensaje
                // no funciona realloc para estirar pbuf aun.. tengo que hacerme una copia lamentablemente0
                // pbuf_realloc(Snmp_Data,Snmp_Data->tot_len+2); // estiro 2 bytes... porque necesito 2 mas para mandar un entero

   Snmp_Pcb  = *upcb;
   Snmp_Addr = *addr;
   Snmp_Port = port;

   uint8_t Sensor_Code_Index;
   if ( Parse_Version() && Parse_Community()) {
      if(Object_Name_Match(&Sensor_Code_Index)) {
         UART_ETHprintf(DEBUG_MSG,"snmp match\r\n");;
         if(Next_Or_Bulk()) {
            Sensor_Code_Index++;
            UART_ETHprintf(DEBUG_MSG,"bulk or walk next=%d\r\n",Sensor_Code_Index);;
         }
         if(Sensor_Code_Index<MAX_ROM_CODES) {
            uint16_t T;
            if(Find_One_Wire_T4Sensor_Code(Usr_Flash_Params.Sensor_Codes[Sensor_Code_Index],&T))
               Response_Int(Sensor_Code_Index,T);
            else
               Response_Int(Sensor_Code_Index,0x7FFE);
         }
         else {
            UART_ETHprintf(DEBUG_MSG,"snmp walk last\r\n");
            Response_Err();
         }
      }
      else {
         if(Next_Or_Bulk()) {
            uint16_t T;
            Sensor_Code_Index=0;
            UART_ETHprintf(DEBUG_MSG,"snmp not match but bulk\r\n");;
            if(Find_One_Wire_T4Sensor_Code(Usr_Flash_Params.Sensor_Codes[Sensor_Code_Index],&T))
               Response_Int(Sensor_Code_Index,T);
            else
               Response_Int(Sensor_Code_Index,0x7FFE);
         }
         else {
            UART_ETHprintf(DEBUG_MSG,"snmp not match neither bulk\r\n");;
            Response_Err();
         }
      }
   }
   else {
      pbuf_free(Snmp_Data);                    //libreo bufer
      Snmp_Data=NULL;
      UART_ETHprintf ( DEBUG_MSG,"Bad Version or Community\r\n" );
   }
}
//-----------------------------------------------------------------------------
void Send_Snmp_Ans(void* nil) {
   udp_connect    ( &Snmp_Pcb,&Snmp_Addr,Snmp_Port );
   udp_send       ( &Snmp_Pcb,Snmp_Data            );
   udp_disconnect ( &Snmp_Pcb                      );
   pbuf_free      ( Snmp_Data                      ); // libreo bufer
   Snmp_Data=NULL;
}
void Response_Int(uint8_t Node,uint16_t Value)/*{{{*/
{
 UART_ETHprintf(DEBUG_MSG,"Snmp Response\r\n");

 Rx_Snmp_Object ( )->Object_Name_Length= Usr_Flash_Params.Snmp_Iso_Len;
 memcpy ( Rx_Snmp_Object( )->Object_Name,
          Usr_Flash_Params.Snmp_Iso[Node],
          Usr_Flash_Params.Snmp_Iso_Len);

 Rx_Snmp_Value ( )->Value_Code   = 0x02              ; // codigo que corresponde a Integer...
 Rx_Snmp_Value ( )->Value_Length = 2                 ; // solo mando integer de 2 bytes...
 Rx_Snmp_Value ( )->Value[0]     = ((char*)&Value)[1];
 Rx_Snmp_Value ( )->Value[1]     = ((char*)&Value)[0];

 Rx_Snmp_Object ( )->Item_Code      = 0x30                                                                  ;
 Rx_Snmp_Object ( )->Item_Length    = Rx_Snmp_Object()->Object_Name_Length+2+Rx_Snmp_Value()->Value_Length+2;
 Rx_Snmp_Object ( )->Binding_Code   = 0x30                                                                  ;
 Rx_Snmp_Object ( )->Binding_Length = Rx_Snmp_Object()->Item_Length+2                                       ;
 Rx_Snmp_Object ( )->Error_Status   = 0x00                                                                  ;

 Rx_Snmp_Msg ( )->Msg_Code       = Get_Response_Event                                                       ;
 Rx_Snmp_Msg ( )->Msg_Length     = Rx_Snmp_Object()->Binding_Length+2+6 + Rx_Snmp_Msg()->Request_Id_Length+2;

 Rx_Snmp_Header                                   ( )->Message_Length = Rx_Snmp_Msg()->Msg_Length+2+Rx_Snmp_Header()->Community_Length+2+3;
 Snmp_Data->len=Snmp_Data->tot_len=Rx_Snmp_Header ( )->Message_Length+2                                                                   ;
 Send_Snmp_Ans(0);
// tcpip_callback ( Send_Snmp_Ans,0 );
}/*}}}*/
void Response_Err(void)/*{{{*/
{
 UART_ETHprintf(DEBUG_MSG,"Snmp Response Err\r\n");;

 Rx_Snmp_Value ( )->Value_Code   = 0x05; // codigo que corresponde a NO-SUCH-NAME
 Rx_Snmp_Value ( )->Value_Length = 1   ;
 Rx_Snmp_Value ( )->Value[0]     = 0   ;

 Rx_Snmp_Object ( )->Item_Length    = Rx_Snmp_Object()->Object_Name_Length+2+Rx_Snmp_Value()->Value_Length+2;
 Rx_Snmp_Object ( )->Binding_Length = Rx_Snmp_Object()->Item_Length+2                                       ;
 Rx_Snmp_Object ( )->Error_Status   = 0x02                                                                  ;

 Rx_Snmp_Msg    ( )->Msg_Code       = Get_Response_Event;
 Rx_Snmp_Msg    ( )->Msg_Length     = Rx_Snmp_Object()->Binding_Length+2+6 + Rx_Snmp_Msg()->Request_Id_Length+2;

 Rx_Snmp_Header ( )->Message_Length = Rx_Snmp_Msg()->Msg_Length+2+Rx_Snmp_Header()->Community_Length+2+3   ;

 Snmp_Data->len=Snmp_Data->tot_len=Rx_Snmp_Header ( )->Message_Length+2                                                                   ;
 Send_Snmp_Ans(0);
 //tcpip_callback ( Send_Snmp_Ans,0 );
}/*}}}*/
//-----------------------------------------------------------------------------
bool Find_One_Wire_T4Sensor_Code(uint8_t* Sensor_Code,uint16_t* T)
{
   uint8_t i;
   for ( i=0;i<One_Wire_On_Line_Nodes();i++ )
      if(memcmp(One_Wire_Code(i),Sensor_Code,8)==0) {
            UART_ETHprintf(DEBUG_MSG,"find sensor %d\r\n",i);
           *T=One_Wire_T(i);
           return true;
         }
   UART_ETHprintf(DEBUG_MSG,"not find sensor %d\r\n",i);
   return false;
}
