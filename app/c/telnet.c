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
struct tcp_pcb* Rs232_List[TCP_REGISTERED_LIST];
//-------------------------------------------------------------------------------------
void Create_Cmd_Socket   ( void* nil );
void Create_Rs232_Socket ( void* nil );
//-------------------------------------------------------------------------------------
void Init_Telnet(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
   tcpip_callback ( Create_Cmd_Socket   ,0 );
   Init_Rs232_Conn();
   tcpip_callback ( Create_Rs232_Socket ,0 );
}


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
            if(B->Index<APP_INPUT_BUF_SIZE)
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

//---------------------------------------------------------------------------------------
bool Free_Rs232_Conn(struct tcp_pcb* New)
{
   uint8_t i,Empty=0;
   for(i=0;i<TCP_REGISTERED_LIST;i++) {
      if(Rs232_List[i]==New) {
         Rs232_List[i]=NULL;
      }
      if(Rs232_List[i]==NULL)
          Empty++;
   }
   if(Empty==TCP_REGISTERED_LIST)
      Send_Event(Conn_Free_Event,Commands());
   return false;
}
bool Register_Rs232_Conn(struct tcp_pcb* New)
{
   uint8_t i;
   for(i=0;i<TCP_REGISTERED_LIST;i++) {
      if(Rs232_List[i]==NULL) {
         Rs232_List[i]=New;
         Send_Event(Conn_Regi_Event,Commands());
         return true;
      }
   }
   return false;
}
void Init_Rs232_Conn(void)
{
   uint8_t i;
   for(i=0;i<TCP_REGISTERED_LIST;i++)
      Rs232_List[i]=NULL;
}
err_t Rcv_Rs232_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   if(p!=NULL)  {
      if(UARTTxBytesFree()>=p->len) {
         UARTwrite  ( p->payload ,p->len );
         tcp_recved ( tpcb       ,p->len );
         pbuf_free  ( p                  ); // libero bufer
         Led_Rgb_Only_Blue(); //debug
         return ERR_OK;
      }
      else
         return ERR_INPROGRESS; //parece que anda, pero no lo revise.. o sea no puedo mandar.. volve mas tarde
   }
   else {
      tcp_accepted    ( Soc_Cmd ); // libreo 1 lugar para el blog
      Free_Rs232_Conn ( tpcb    );
      tcp_close       ( tpcb    ); // cierro
      return ERR_OK;
   }
}
bool Is_Any_Connection_Registered(void)
{
   uint8_t i;
   for(i=0;i<TCP_REGISTERED_LIST && Rs232_List[i]==NULL;i++)
      ;
   return i<TCP_REGISTERED_LIST;
}
bool Send_To_All_Tcp(uint8_t* Data, uint16_t Len)
{
   uint8_t i;
   bool Ans=false;
   for(i=0;i<TCP_REGISTERED_LIST;i++) {
      if(Rs232_List[i]!=NULL) {
        tcp_write(Rs232_List[i],Data,Len,TCP_WRITE_FLAG_COPY);//|TCP_WRITE_FLAG_MORE);
        Ans=true;
      }
   }
   return Ans;
}
err_t Accept_Rs232_Fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   tcp_recv            ( newpcb ,Rcv_Rs232_Fn );
   tcp_arg             ( newpcb ,NULL         );
   Register_Rs232_Conn ( newpcb               );
   return 0;
}
void Create_Rs232_Socket(void* nil)
{
   Soc_Rs232=tcp_new                 (                                                     );
   tcp_bind                          ( Soc_Rs232 ,IP_ADDR_ANY ,Usr_Flash_Params.Rs232_Port );
   Soc_Rs232=tcp_listen_with_backlog ( Soc_Rs232 ,3                                        );
   tcp_accept                        ( Soc_Rs232 ,Accept_Rs232_Fn                           );
}




