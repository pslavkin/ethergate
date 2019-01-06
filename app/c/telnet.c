#include <stdint.h>
#include <string.h>
#include "utils/lwiplib.h"
#include "utils/cmdline.h"
#include "utils/cmdline.h"
#include "opt.h"
#include "state_machine.h"
#include "events.h"
#include "usr_flash.h"
#include "commands.h"
#include "utils/uartstdio.h"
#include "utils/ringbuf.h"
#include "parser.h"
#include "telnet.h"
#include "leds.h"
#include "utils/uartstdio.h"

struct tcp_pcb *Soc_Cmd;
struct tcp_pcb *Soc_Rs232;
struct tcp_pcb *Soc_Sniffer;
struct Conn_List_Struct
{
   struct tcp_pcb* tpcb;
   bool Sniff;
} Conn[TCP_REGISTERED_LIST];

//-------------------------------------------------------------------------------------
void Create_Cmd_Socket   ( void* nil );
void Create_Rs232_Socket ( void* nil );
//-------------------------------------------------------------------------------------
void Init_Telnet(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
   Init_Conn();
   tcpip_callback ( Create_Cmd_Socket     ,0 );
   tcpip_callback ( Create_Rs232_Socket   ,0 );
   tcpip_callback ( Create_Sniffer_Socket ,0 );
}

//-----------ETH Console---------------------------------------------------------------
err_t Rcv_Cmd_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   uint8_t Data;
   uint16_t i;
   struct Parser_Queue_Struct* B=arg;
   if(p!=NULL)  {
      for(i=0;i<p->len;i++) {
         Data=((uint8_t*)p->payload)[i];
         if(Data=='\n' || Data=='\r') {
            if( (i+1)<p->len                                      &&
               ((Data=='\n' && ((uint8_t*)p->payload)[i+1]=='\r') ||
                (Data=='\r' && ((uint8_t*)p->payload)[i+1]=='\n'))
              )
               i++;
            B->Buff[B->Index] = '\0';
            xQueueSend(Parser_Queue,B,portMAX_DELAY);
            B->Id++;
            B->Index = 0;
         }
         else
            if(B->Index<sizeof(B->Buff))
               B->Buff[B->Index++]=Data;
      }
      tcp_recved(tpcb,p->len);
      pbuf_free(p);                    //libero bufer
      return ERR_OK;
   }
   else {
      vPortFree    ( arg     ); // libero el buffer de recepcion
      tcp_accepted ( Soc_Cmd ); // libreo 1 lugar para el blog
      tcp_close    ( tpcb    ); // cierro
      return ERR_OK;
   }
}

void Telnet_Close ( struct tcp_pcb *tpcb)
{
   if(tpcb!=DEBUG_MSG && tpcb!=UART_MSG ) {
      tcp_close(tpcb);
   }
}

err_t Accept_Cmd_Fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   struct Parser_Queue_Struct* R= ( struct Parser_Queue_Struct* )pvPortMalloc(sizeof(struct Parser_Queue_Struct));
   R->Id    = 0;
   R->Index = 0;
   R->tpcb  = newpcb;
   tcp_recv ( newpcb ,Rcv_Cmd_Fn );
   tcp_arg  ( newpcb ,R          );
   return 0;
}

void Create_Cmd_Socket(void* nil)
{
   Soc_Cmd=tcp_new                 (                                                    );
   tcp_bind                        ( Soc_Cmd ,IP_ADDR_ANY ,Usr_Flash_Params.Config_Port );
   Soc_Cmd=tcp_listen_with_backlog ( Soc_Cmd ,3                                         );
   tcp_accept                      ( Soc_Cmd ,Accept_Cmd_Fn                             );
}
//-------CONNECTION LIST----------------------------------------------------------------
bool Free_Conn(struct tcp_pcb* New)
{
   uint8_t i,Empty=0;
   for(i=0;i<TCP_REGISTERED_LIST;i++) {
      if(Conn[i].tpcb==New) {
         Conn[i].tpcb=NULL;
      }
      if(Conn[i].tpcb==NULL)
          Empty++;
   }
   if(Empty==TCP_REGISTERED_LIST)
      Send_Event(Conn_Free_Event,Commands());
   return false;
}
bool Add_Conn(struct tcp_pcb* New, bool Sniff)
{
   uint8_t i;
   for(i=0;i<TCP_REGISTERED_LIST;i++) {
      if(Conn[i].tpcb==NULL) {
         Conn[i].tpcb=New;
         Conn[i].Sniff=Sniff;
         Send_Event(Conn_Regi_Event,Commands());
         return true;
      }
   }
   return false;
}
bool Is_Any_Conn(void)
{
   uint8_t i;
   for(i=0;i<TCP_REGISTERED_LIST && Conn[i].tpcb==NULL;i++)
      ;
   return i<TCP_REGISTERED_LIST;
}
bool Send_To_Conn_Tcp(uint8_t* Data, uint16_t Len,bool Sniff)
{
   uint8_t i;
   bool Ans=false;
   for(i=0;i<TCP_REGISTERED_LIST;i++)
      if(Conn[i].tpcb!=NULL &&
        (Sniff==false || (Sniff==true && Conn[i].Sniff==true))) {
            tcp_write(Conn[i].tpcb,Data,Len,TCP_WRITE_FLAG_COPY);
            tcp_output(Conn[i].tpcb);
            Ans=true;
      }
   return Ans;
}
bool Send_To_All_Tcp(uint8_t* Data, uint16_t Len)
{
   return Send_To_Conn_Tcp(Data,Len,false);
}
bool Send_To_Sniffer_Tcp(uint8_t* Data, uint16_t Len)
{
   return Send_To_Conn_Tcp(Data,Len,true);
}
void Init_Conn(void)
{
   uint8_t i;
   for(i=0;i<TCP_REGISTERED_LIST;i++)
      Conn[i].tpcb=NULL;
}
//-------RS232<>ETH RAW----------------------------------------------------------------
err_t Rcv_Rs232_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   if(p!=NULL)  {
      if(UARTTxBytesFree()>=p->len) {
         UARTwrite           ( p->payload ,p->len );
         Send_To_Sniffer_Tcp ( p->payload ,p->len );
         tcp_recved          ( tpcb       ,p->len );
         pbuf_free           ( p                  ); // libero bufer
         return ERR_OK;
      }
      else
         return ERR_INPROGRESS; //parece que anda, pero no lo revise.. o sea no puedo mandar.. volve mas tarde
   }
   else {
      tcp_accepted    ( Soc_Cmd ); // libreo 1 lugar para el blog
      Free_Conn ( tpcb    );
      tcp_close       ( tpcb    ); // cierro
      return ERR_OK;
   }
}
err_t Accept_Rs232_Fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   tcp_recv ( newpcb ,Rcv_Rs232_Fn );
   tcp_arg  ( newpcb ,NULL         );
   Add_Conn ( newpcb,false         );
   return 0;
}
void Create_Rs232_Socket(void* nil)
{
   Soc_Rs232=tcp_new                 (                                                     );
   tcp_bind                          ( Soc_Rs232 ,IP_ADDR_ANY ,Usr_Flash_Params.Rs232_Port );
   Soc_Rs232=tcp_listen_with_backlog ( Soc_Rs232 ,3                                        );
   tcp_accept                        ( Soc_Rs232 ,Accept_Rs232_Fn                           );
}
//---ETH&RS232 to ETH--------------------------------------------------------------
err_t Rcv_Sniffer_Fn(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   if(p!=NULL)  {
         tcp_recved ( tpcb ,p->len );
         pbuf_free  ( p            ); // libero bufer
         return ERR_OK;
   }
   else {
      tcp_accepted ( Soc_Cmd ); // libreo 1 lugar para el blog
      Free_Conn    ( tpcb    );
      tcp_close    ( tpcb    ); // cierro
      return ERR_OK;
   }
}
err_t Accept_Sniffer_Fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   tcp_recv ( newpcb ,Rcv_Sniffer_Fn );
   tcp_arg  ( newpcb ,NULL           );
   Add_Conn ( newpcb ,true           );
   return 0;
}
void Create_Sniffer_Socket(void* nil)
{
   Soc_Sniffer=tcp_new                 (                                                     );
   tcp_bind                            ( Soc_Sniffer ,IP_ADDR_ANY ,Usr_Flash_Params.Sniffer_Port );
   Soc_Sniffer=tcp_listen_with_backlog ( Soc_Sniffer ,3                                          );
   tcp_accept                          ( Soc_Sniffer ,Accept_Sniffer_Fn                          );
}




