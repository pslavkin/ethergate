#include <stdbool.h>
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
#include "usr_flash.h"


//#pragma section {Flash_UData}
//unsigned int   Usr_Flash_Params.Snmp_Iso_Len  = 9;
//unsigned char  Usr_Flash_Params.Snmp_Iso[12]      = {0x2b,0x06,0x01,0x02,0x01,0x21,0x01,0x02,0x07};
//#pragma section const {}
const State* Snmp_Agent_Sm;
struct pbuf* Snmp_Data=NULL;
struct udp_pcb Snmp_Pcb;
ip_addr_t Snmp_Addr;
u16_t Snmp_Port;

//------------------------------------------------------------------------------
char * Udp_Rx_Payload                        ( void ) { return Snmp_Data->payload                                                                                  ;}
struct Snmp_Header_Struct*    Rx_Snmp_Header ( void ) { return (struct Snmp_Header_Struct*) Udp_Rx_Payload()                                                       ;}
struct Snmp_Msg_Struct*       Rx_Snmp_Msg    ( void ) { return (struct Snmp_Msg_Struct*)    (Rx_Snmp_Header()->Community+Rx_Snmp_Header()->Community_Length)       ;}
struct Snmp_Object_Struct*    Rx_Snmp_Object ( void ) { return (struct Snmp_Object_Struct*)   (Rx_Snmp_Msg()->Request_Id+Rx_Snmp_Msg()->Request_Id_Length)         ;}
struct Snmp_Value_Struct*     Rx_Snmp_Value  ( void ) { return (struct Snmp_Value_Struct*)     (Rx_Snmp_Object()->Object_Name+Rx_Snmp_Object()->Object_Name_Length);}
//------------------------------------------------------------------------------
bool  Parse_Version   ( void ) { return Rx_Snmp_Header()->Version==0 || Rx_Snmp_Header()->Version==1       ;}
bool  Parse_Community ( void ) { return strncmp(Rx_Snmp_Header()->Community,Usr_Flash_Params.Snmp_Community,Rx_Snmp_Header()->Community_Length)==0;}
bool  Object_Name_Match ( void ) {
   return (Rx_Snmp_Object( )->Object_Name_Length==Usr_Flash_Params.Snmp_Iso_Len+1 &&             // la longitud del MIB tiene que ser justo uno mas que mi MIB, para que considere que indexa justo ese
           memcmp(Usr_Flash_Params.Snmp_Iso,Rx_Snmp_Object( )->Object_Name,Usr_Flash_Params.Snmp_Iso_Len)==0); // ah, y ademas tiene que condicid con mi MIB claro
}
bool  Next_Or_Bulk ( void ) { return Rx_Snmp_Msg( )->Msg_Code==Get_Next_Request_Event || Rx_Snmp_Msg( )->Msg_Code==Get_Bulk_Request_Event;}
//------------------------------------------------------------------------------
void Snmp_Packet_Arrived (struct udp_pcb *upcb, struct pbuf *p, ip_addr_t* addr,  u16_t port)
{
   if(Snmp_Data!=NULL) { //si eta operando en un mensaje anterior.. tira el nuevo..
      pbuf_free(p);
      return;
   }
   Snmp_Data=p;                                  // me guardo en una local el puntero al mensaje
   pbuf_realloc(Snmp_Data,Snmp_Data->tot_len+2); // estiro 2 bytes... porque necesito 2 mas para mandar un entero
   Snmp_Pcb  = *upcb;
   Snmp_Addr = *addr;
   Snmp_Port = port;

   if ( Parse_Version() && Parse_Community()) {
      if(Object_Name_Match()) {
            Response_One_Wire_T(Rx_Snmp_Object()->Object_Name[Rx_Snmp_Object()->Object_Name_Length-1]+1);
      }
      else {
         if(Next_Or_Bulk())
               Response_One_Wire_T(0);
         else
               Response_Err();
      }
   }
   else {
      pbuf_free(Snmp_Data);                    //libreo bufer
      Snmp_Data=NULL;
      UART_ETHprintf ( DEBUG_MSG,"Bad Version or Community\n" );
   }
}
//-----------------------------------------------------------------------------
void Send_Snmp_Ans(void* nil) {
   udp_connect    ( &Snmp_Pcb,&Snmp_Addr,Snmp_Port );
   udp_send       ( &Snmp_Pcb,Snmp_Data            );
   udp_disconnect ( &Snmp_Pcb                      );
   pbuf_free(Snmp_Data);                    //libreo bufer
   Snmp_Data=NULL;
}
void Response_Int(unsigned char SysDescr,uint16_t Value)/*{{{*/
{
 UART_ETHprintf(DEBUG_MSG,"Snmp Response\n");

 Rx_Snmp_Object ( )->Object_Name_Length              = Usr_Flash_Params.Snmp_Iso_Len+1; // el +1 del final es porque el OID que se graba en el equipo puede tener muchos 'hijos' de 1 byte. por que es el que se mueve en el bulk de hecho y se recibe como parametro aca...
 Rx_Snmp_Object ( )->Object_Name[Usr_Flash_Params.Snmp_Iso_Len] = SysDescr            ; // como ultimo valor pone el OID que piden...
 memcpy ( Rx_Snmp_Object( )->Object_Name,Usr_Flash_Params.Snmp_Iso,Usr_Flash_Params.Snmp_Iso_Len);    // aca se copua todo el header del OID que faltaba...

 Rx_Snmp_Value ( )->Value_Code   = 0x02;                                     // codigo que corresponde a Integer...
 Rx_Snmp_Value ( )->Value_Length = 2           ;                             // solo mando integer de 2 bytes...
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
 tcpip_callback ( Send_Snmp_Ans,0 );
}/*}}}*/
void Response_Err(void)/*{{{*/
{
 UART_ETHprintf(DEBUG_MSG,"Snmp Response Err\n");;

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
 tcpip_callback ( Send_Snmp_Ans,0 );
}/*}}}*/
//-----------------------------------------------------------------------------
void Response_One_Wire_T(uint8_t Node)
{
  if ( One_Wire_On_Line_Nodes( ) > Node)
     Response_Int(Node,One_Wire_T(Node));
  else
     Response_Err();
}
