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
#include "opt.h"
#include "udp.h"
#include "snmp_agent.h"
#include "state_machine.h"
#include "events.h"
#include "commands.h"
#include "one_wire_network.h"
#include "usr_flash.h"/*}}}*/

//------------------------------------------------------------------------------
struct Snmp_Header_Struct*    Snmp_Header ( char* p ) { return ( struct Snmp_Header_Struct*)p                                                                                        ;}
struct Snmp_Msg_Struct*       Snmp_Msg    ( char* p ) { return ( struct Snmp_Msg_Struct*)(Snmp_Header(p)->Community+Snmp_Header(p)->Community_Length)                           ;}
struct Snmp_Object_Struct*    Snmp_Object ( char* p ) { return ( struct Snmp_Object_Struct*)                ( Snmp_Msg    (p)->Request_Id+Snmp_Msg     (p)->Request_Id_Length) ;}
struct Snmp_Value_Struct*     Snmp_Value  ( char* p ) { return ( struct Snmp_Value_Struct*)                 ( Snmp_Object (p)->Object_Name+Snmp_Object (p)->Object_Name_Length);}
//------------------------------------------------------------------------------
bool  Parse_Version   ( char* p ) { return Snmp_Header(p)->Version==0 || Snmp_Header(p)->Version==1       ;}
bool  Parse_Community ( char* p ) { return strncmp(Snmp_Header(p)->Community,Usr_Flash_Params.Snmp_Community,strlen(Usr_Flash_Params.Snmp_Community))==0;}
bool  Object_Name_Match ( char* p, uint8_t* Index ) {
   uint8_t i;
   for(i=0;i<MAX_ROM_CODES;i++) {
     if(Snmp_Object(p)->Object_Name_Length==Usr_Flash_Params.Snmp_Iso_Len[i] &&
           memcmp(Usr_Flash_Params.Snmp_Iso[i],Snmp_Object(p)->Object_Name, Usr_Flash_Params.Snmp_Iso_Len[i])==0) {
        *Index=i;
        break;
     }
   }
  return i<MAX_ROM_CODES;
}
bool  Next_Or_Bulk ( char* p ) { return Snmp_Msg(p)->Msg_Code==Get_Next_Request_Event || Snmp_Msg(p)->Msg_Code==Get_Bulk_Request_Event;}
//------------------------------------------------------------------------------
void Snmp_Packet_Arrived (struct udp_pcb *upcb, struct pbuf *p, ip_addr_t* addr,  u16_t port)
{
   uint8_t Sensor_Code_Index;
   char* snmpIn = p->payload;

   if ( Parse_Version(snmpIn) && Parse_Community(snmpIn)) {
      if(Object_Name_Match(snmpIn,&Sensor_Code_Index)) {
         UART_ETHprintf(DEBUG_MSG,"snmp match\r\n");
         if(Next_Or_Bulk(snmpIn)) {
            Sensor_Code_Index++;
            UART_ETHprintf(DEBUG_MSG,"bulk or walk next=%d\r\n",Sensor_Code_Index);
         }
         if(Sensor_Code_Index<MAX_ROM_CODES) {
            Response_Int(upcb,snmpIn,addr,port,Sensor_Code_Index,Find_One_Wire_T4Sensor_Code(Usr_Flash_Params.Sensor_Codes[Sensor_Code_Index]));
         }
         else {
            UART_ETHprintf(DEBUG_MSG,"snmp walk last\r\n");
            Response_Err(upcb,snmpIn,addr,port);
         }
      }
      else {
         if(Next_Or_Bulk(snmpIn)) {
            Sensor_Code_Index=0;
            UART_ETHprintf(DEBUG_MSG,"snmp not match but bulk\r\n");;
            Response_Int(upcb,snmpIn,addr,port,Sensor_Code_Index,Find_One_Wire_T4Sensor_Code(Usr_Flash_Params.Sensor_Codes[Sensor_Code_Index]));
         }
         else {
            UART_ETHprintf(DEBUG_MSG,"snmp not match neither bulk\r\n");;
            Response_Err(upcb,snmpIn,addr,port);
         }
      }
   }
   else {
      UART_ETHprintf ( DEBUG_MSG,"Bad Version or Community\r\n" );
   }
   pbuf_free(p);
}
//-----------------------------------------------------------------------------
void Send_Snmp_Ans(struct udp_pcb *upcb, struct pbuf *p,ip_addr_t* addr, u16_t port) {
   UART_ETHprintf(DEBUG_MSG,"Envio paquete UDP:\r\n%H\r\n",p->payload,p->tot_len);
   if ( udp_connect(upcb,addr,port ) == ERR_OK)
      if ( udp_send( upcb,p ) == ERR_OK)
         udp_disconnect ( upcb );
   pbuf_free ( p );
}

void Response_Int(struct udp_pcb *upcb, char* in,ip_addr_t* addr,  u16_t port,uint8_t Node,uint16_t Value)/*{{{*/
{
   uint16_t ansLen;
   ansLen=  sizeof(struct Snmp_Header_Struct)-1 + strlen(Usr_Flash_Params.Snmp_Community);
   ansLen+= sizeof(struct Snmp_Msg_Struct)-1    + Snmp_Msg(in)->Request_Id_Length;
   ansLen+= sizeof(struct Snmp_Object_Struct)-1 + Usr_Flash_Params.Snmp_Iso_Len[Node];
   ansLen+= sizeof(struct Snmp_Value_Struct)-1  + 2;
   struct pbuf *outPbuf = pbuf_alloc(PBUF_TRANSPORT,ansLen,PBUF_RAM);
   char* out            = outPbuf->payload;

   UART_ETHprintf(DEBUG_MSG,"Snmp Response len=%i\r\n",ansLen);

   Snmp_Header (out)->Message_Code     = Snmp_Header (in)->Message_Code;
// Snmp_Header (out)->Message_Length   = Snmp_Header (in)->Message_Length  ;
   Snmp_Header (out)->Version_Code     = Snmp_Header (in)->Version_Code    ;
   Snmp_Header (out)->Version_Length   = Snmp_Header (in)->Version_Length  ;
   Snmp_Header (out)->Version          = Snmp_Header (in)->Version         ;
   Snmp_Header (out)->Community_Code   = Snmp_Header (in)->Community_Code  ;
   Snmp_Header (out)->Community_Length = strlen(Usr_Flash_Params.Snmp_Community);
   memcpy ( Snmp_Header(out)->Community,Usr_Flash_Params.Snmp_Community,Snmp_Header (out)->Community_Length);

   Snmp_Msg ( out )->Msg_Code          = Get_Response_Event;
// Snmp_Msg ( out )->Msg_Length        = Snmp_Msg(in)->Msg_Length      ;
   Snmp_Msg ( out )->Request_Id_Code   = Snmp_Msg(in)->Request_Id_Code ;
   Snmp_Msg ( out )->Request_Id_Length = Snmp_Msg(in)->Request_Id_Length;
   memcpy ( Snmp_Msg(out)->Request_Id,Snmp_Msg(in)->Request_Id,Snmp_Msg(in)->Request_Id_Length);

   Snmp_Object ( out )-> Error_Status_Code   = Snmp_Object(in)->Error_Status_Code  ;
   Snmp_Object ( out )-> Error_Status_Length = Snmp_Object(in)->Error_Status_Length;
   Snmp_Object ( out )-> Error_Status        = 0                                   ;
   Snmp_Object ( out )-> Error_Index_Code    = Snmp_Object(in)->Error_Index_Code   ;
   Snmp_Object ( out )-> Error_Index_Length  = Snmp_Object(in)->Error_Index_Length ;
   Snmp_Object ( out )-> Error_Index         = Snmp_Object(in)->Error_Index        ;
   Snmp_Object ( out )-> Binding_Code        = 0x30                                ;
// Snmp_Object   ( out )-> Binding_Length      = Snmp_Object(in)->Binding_Length     ;
   Snmp_Object ( out )-> Item_Code           = 0x30                                ;
// Snmp_Object   ( out )-> Item_Length         = Snmp_Object(in)->Item_Length        ;
   Snmp_Object ( out )-> Object_Name_Code    = Snmp_Object(in)->Object_Name_Code   ;
   Snmp_Object ( out )-> Object_Name_Length  = Usr_Flash_Params.Snmp_Iso_Len[ Node];
   memcpy      ( Snmp_Object ( out )-> Object_Name,Usr_Flash_Params.Snmp_Iso       [ Node],Snmp_Object(out)->Object_Name_Length);

   Snmp_Value ( out )->Value_Code   = 0x02              ; // codigo que corresponde a Integer...
   Snmp_Value ( out )->Value_Length = 2                 ; // solo mando integer de 2 bytes...
   Snmp_Value ( out )->Value[0]     = ((char*)&Value)[1];
   Snmp_Value ( out )->Value[1]     = ((char*)&Value)[0];

   Snmp_Object ( out )->Item_Length    = Snmp_Object(out)->Object_Name_Length+2+Snmp_Value(out)->Value_Length+2;
   Snmp_Object ( out )->Binding_Length = Snmp_Object(out)->Item_Length+2                                       ;

   Snmp_Msg    ( out )->Msg_Length     = Snmp_Object(out)->Binding_Length+2+6 + Snmp_Msg(out)->Request_Id_Length+2;
   Snmp_Header ( out )->Message_Length = Snmp_Msg(out)->Msg_Length+2+Snmp_Header(out)->Community_Length+2+3       ;
   Send_Snmp_Ans(upcb,outPbuf,addr,port);
}/*}}}*/

void Response_Err(struct udp_pcb *upcb, char* in,ip_addr_t* addr, u16_t port)/*{{{*/
{
   uint16_t ansLen;
   ansLen=  sizeof(struct Snmp_Header_Struct)-1 + Snmp_Header(in)->Community_Length;;
   ansLen+= sizeof(struct Snmp_Msg_Struct)-1    + Snmp_Msg(in)->Request_Id_Length;
   ansLen+= sizeof(struct Snmp_Object_Struct)-1 + Snmp_Object(in)->Object_Name_Length;
   ansLen+= sizeof(struct Snmp_Value_Struct)-1  + 1;
   struct pbuf *outPbuf = pbuf_alloc(PBUF_TRANSPORT,ansLen,PBUF_RAM);
   char* out            = outPbuf->payload;
   UART_ETHprintf(DEBUG_MSG,"Snmp Response Err len=%i\r\n",ansLen);

   Snmp_Header (out)->Message_Code     = Snmp_Header (in)->Message_Code;
// Snmp_Header (out)->Message_Length   = Snmp_Header (in)->Message_Length  ;
   Snmp_Header (out)->Version_Code     = Snmp_Header (in)->Version_Code    ;
   Snmp_Header (out)->Version_Length   = Snmp_Header (in)->Version_Length  ;
   Snmp_Header (out)->Version          = Snmp_Header (in)->Version         ;
   Snmp_Header (out)->Community_Code   = Snmp_Header (in)->Community_Code  ;
   Snmp_Header (out)->Community_Length = Snmp_Header (in)->Community_Length;
   memcpy ( Snmp_Header(out)->Community,Snmp_Header(in)->Community,Snmp_Header (out)->Community_Length);

   Snmp_Msg ( out )->Msg_Code          = Get_Response_Event;
// Snmp_Msg ( out )->Msg_Length        = Snmp_Msg(in)->Msg_Length      ;
   Snmp_Msg ( out )->Request_Id_Code   = Snmp_Msg(in)->Request_Id_Code ;
   Snmp_Msg ( out )->Request_Id_Length = Snmp_Msg(in)->Request_Id_Length;
   memcpy ( Snmp_Msg(out)->Request_Id,Snmp_Msg(in)->Request_Id,Snmp_Msg(in)->Request_Id_Length);

   Snmp_Object ( out )-> Error_Status_Code   = Snmp_Object(in)->Error_Status_Code  ;
   Snmp_Object ( out )-> Error_Status_Length = Snmp_Object(in)->Error_Status_Length;
   Snmp_Object ( out )-> Error_Status        = 0x02                                ;
   Snmp_Object ( out )-> Error_Index_Code    = Snmp_Object(in)->Error_Index_Code   ;
   Snmp_Object ( out )-> Error_Index_Length  = Snmp_Object(in)->Error_Index_Length ;
   Snmp_Object ( out )-> Error_Index         = Snmp_Object(in)->Error_Index        ;
   Snmp_Object ( out )-> Binding_Code        = Snmp_Object(in)->Binding_Code       ;
// Snmp_Object   ( out )-> Binding_Length    = Snmp_Object(in)->Binding_Length     ;
   Snmp_Object ( out )-> Item_Code           = Snmp_Object ( in )-> Item_Code       ;
// Snmp_Object   ( out )-> Item_Length       = Snmp_Object(in)->Item_Length        ;
   Snmp_Object ( out )-> Object_Name_Code    = Snmp_Object(in)->Object_Name_Code   ;
   Snmp_Object ( out )-> Object_Name_Length  = Snmp_Object(in)->Object_Name_Length;
   memcpy ( Snmp_Object ( out )-> Object_Name,Snmp_Object ( in )-> Object_Name,Snmp_Object(out)->Object_Name_Length);

    Snmp_Value  (out )->Value_Code   = 0x05; // codigo que corresponde a NO-SUCH-NAME
    Snmp_Value  (out )->Value_Length = 1   ;
    Snmp_Value  (out )->Value[0]     = 0   ;

   Snmp_Object ( out )->Item_Length    = Snmp_Object(out)->Object_Name_Length+2+Snmp_Value(out)->Value_Length+2;
   Snmp_Object ( out )->Binding_Length = Snmp_Object(out)->Item_Length+2                                       ;

   Snmp_Msg ( out )->Msg_Length     = Snmp_Object(out)->Binding_Length+2+6 + Snmp_Msg(out)->Request_Id_Length+2;
   Snmp_Header(out)->Message_Length = Snmp_Msg(out)->Msg_Length+2+Snmp_Header(out)->Community_Length+2+3;
   Send_Snmp_Ans(upcb,outPbuf,addr,port);
}/*}}}*/
//-----------------------------------------------------------------------------
uint16_t Find_One_Wire_T4Sensor_Code(uint8_t* Sensor_Code)
{
   uint8_t i;
   uint16_t T=0x7FFE;      //por defecto devuelve un valor invalido
   for ( i=0;i<One_Wire_On_Line_Nodes();i++ )
      if(memcmp(One_Wire_Code(i),Sensor_Code,8)==0) {
            UART_ETHprintf(DEBUG_MSG,"find sensor %d\r\n",i);
           T=One_Wire_T(i);
           break;
         }
   return T;
}
