#include <stdint.h>/*{{{*/
#include <string.h>
#include "utils/lwiplib.h"
#include "utils/cmdline.h"
#include "utils/cmdline.h"
#include "opt.h"
#include "state_machine.h"
#include "events.h"
#include "usr_flash.h"
#include "rs232.h"
#include "commands.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/ringbuf.h"
#include "parser.h"
#include "telnet.h"
#include "leds.h"
#include "lwip/ip_addr.h"/*}}}*/

struct tcp_pcb *Soc_Cmd;
struct tcp_pcb *Soc_Rs232;
struct tcp_pcb *Soc_Sniffer;
struct tcp_pcb *Soc_Virtual;


struct Soc_Clients_Struct Clients_List[MAX_TCP_CLIENTS];
struct Conn_List_Struct
{
   struct tcp_pcb*   Tpcb;
   Tpcb_Type_T       Type;
} Conn[TCP_REGISTERED_LIST];

const State
   Disabled  [ ],
   Waiting   [ ],
   Connecting[ ],
   Connected [ ],
   Closing   [ ];

//-------------------------------------------------------------------------------------
void Create_Cmd_Socket   ( void* nil );
void Create_Rs232_Socket ( void* nil );
//-------------------------------------------------------------------------------------
void Init_Telnet(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
   Init_Conn();
   tcpip_callback ( Create_Cmd_Socket     ,0 );
   tcpip_callback ( Create_Sniffer_Socket ,0 );
#if RS232_ETH_ENABLE
   tcpip_callback ( Create_Rs232_Socket   ,0 );
   tcpip_callback ( Create_Virtual_Socket ,0 );
#endif
}
void Telnet_Close ( struct tcp_pcb *tpcb) //TODO: en realidad tengo que cerrar todas las conecciones abiertas si quiero rebootear, no solo la que lo pide...
{
   if(tpcb!=DEBUG_MSG && tpcb!=UART_MSG ) {
      if(tpcb->callback_arg!=(void*)0xFFFFFFFF) {
         tcpip_callback((tcpip_callback_fn)tcp_close,tpcb);
//         tcp_close(tpcb);
         vPortFree    ( tpcb->callback_arg );  // libero el buffer de recepcion
         tcp_accepted ( Soc_Cmd            );  // libreo 1 lugar para el backlog
         tpcb->callback_arg=(void*)0xFFFFFFFF; // con esto me acuerdo si ya estoy en el proceso de cierre o no porque es diferente si arranco el cierre desde el equipo o desde el cliente
         UART_ETHprintf(DEBUG_MSG,"close Soc_Cmd backlogs=%i\r\n",((struct tcp_pcb_listen *)Soc_Cmd)->accepts_pending);
      }
   }
}
//-------CONNECTION LIST----------------------------------------------------------------{{{
bool Free_Conn(struct tcp_pcb* New)
{
   uint8_t i,Empty=0;
   for(i=0;i<TCP_REGISTERED_LIST;i++) {
      if(Conn[i].Tpcb==New) {
         Conn[i].Tpcb=NULL;
      }
      if(Conn[i].Tpcb==NULL || Conn[i].Type!=NORMAL )
          Empty++;
   }
   if(Empty==TCP_REGISTERED_LIST)
      Send_Event(Conn_Free_Event,Rs232());
   return false;
}
bool Add_Conn(struct tcp_pcb* New, Tpcb_Type_T Type)
{
   uint8_t i;
   for(i=0;i<TCP_REGISTERED_LIST;i++) {
      if(Conn[i].Tpcb==NULL) {
         Conn[i].Tpcb=New;
         Conn[i].Type=Type;
         if(Type==NORMAL) Send_Event(Conn_Regi_Event,Rs232());
         return true;
      }
   }
   return false;
}
bool Is_Any_Conn(void)
{
   uint8_t i;
   for(i=0;i<TCP_REGISTERED_LIST && Conn[i].Tpcb==NULL;i++)
      ;
   return i<TCP_REGISTERED_LIST;
}
bool Send_To_Conn_Tcp(uint8_t* Data, uint16_t Len,Tpcb_Type_T Type)
{
   uint8_t i;
   bool Ans=false;
   for(i=0;i<TCP_REGISTERED_LIST;i++)
      if(Conn[i].Tpcb!=NULL && Conn[i].Type&Type) {
            tcp_write  ( Conn[i].Tpcb,Data,Len,TCP_WRITE_FLAG_COPY );
            tcp_output ( Conn[i].Tpcb                              );
            Ans=true;
      }
   return Ans;
}
bool Send_To_Normal_Tcp(uint8_t* Data, uint16_t Len)
{
   return Send_To_Conn_Tcp(Data,Len,NORMAL);
}
bool Send_To_Sniffer_Tcp(uint8_t* Data, uint16_t Len)
{
   return Send_To_Conn_Tcp(Data,Len,SNIFF);
}
bool Send_To_Virtual_Tcp(uint8_t* Data, uint16_t Len)
{
   return Send_To_Conn_Tcp(Data,Len,VIRTUAL);
}
void Init_Conn(void)
{
   uint8_t i;
   for(i=0;i<TCP_REGISTERED_LIST;i++)
      Conn[i].Tpcb=NULL;
}/*}}}*/
//-------CMD CONSOLE---------------------------------------------------------------{{{
void Err_Cmd_Fn (void *arg, err_t err)
{  //debug msg TODO
   UART_ETHprintf(UART_MSG,"error fatal de conexion %i\r\n",err);
}
static uint32_t Id=0; //debug TODO
err_t Rcv_Cmd_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   uint8_t Data;
   uint16_t i;
   char Buff[20]="que tal\r\n";
   uint8_t len;
   struct Parser_Queue_Struct* B=arg;
   if(p!=NULL)  {
      for(i=0;i<p->len;i++) {
         Data=((uint8_t*)p->payload)[i];
         if(manageEnter(Data,(i+1)<p->len,((uint8_t*)p->payload)[i+1],&i)) {
            manageLastInput(B);
            //xQueueSend(Parser_Queue,B,0);//portMAX_DELAY);
            CmdLineProcess(B);
            //len=usprintf(Buff,"chau %d\r\n",Id++);
            //tcp_write(tpcb,Buff,len,TCP_WRITE_FLAG_COPY);
            B->Id++;
            B->Index = 0;
            //Telnet_Close(tpcb);
         }
         else
            if(B->Index<sizeof(B->Buff)) {
               B->Buff[B->Index++] = Data;
               B->lastIndex        = B->Index;
            }
      }
      tcp_recved(tpcb,p->len);
      pbuf_free(p);                    //libero bufer
      return ERR_OK;
   }
   else {
      Telnet_Close(tpcb);
      return ERR_ABRT;
   }
   return ERR_OK;
}
err_t Accept_Cmd_Fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   struct Parser_Queue_Struct* R= ( struct Parser_Queue_Struct* )pvPortMalloc(sizeof(struct Parser_Queue_Struct));
   if(R!=NULL) {
      R->Id       = 0;
      R->Index    = 0;
      R->lastIndex= 0;
      R->tpcb     = newpcb;
      R->CmdTable = Login_Cmd_Table;
      R->Ref      = R;
      tcp_recv ( newpcb ,Rcv_Cmd_Fn );
      tcp_err  ( newpcb ,Err_Cmd_Fn );
      tcp_arg  ( newpcb ,R          );
   }
   else {
      UART_ETHprintf(UART_MSG,"Accept_Cmd_Fn no tiene espacio\r\n");
      tcp_arg ( newpcb ,NULL );
      Telnet_Close(newpcb);
   }
   return 0;
}

void Create_Cmd_Socket(void* nil)
{
   Soc_Cmd = tcp_new                 (                                                    );
   tcp_bind                          ( Soc_Cmd ,IP_ADDR_ANY ,Usr_Flash_Params.Config_Port );
   Soc_Cmd = tcp_listen_with_backlog ( Soc_Cmd ,TCP_DEFAULT_LISTEN_BACKLOG                );
   tcp_accept                        ( Soc_Cmd ,Accept_Cmd_Fn                             );
}/*}}}*/
//-------RS232<>ETH RAW----------------------------------------------------------------{{{
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
      tcp_accepted ( Soc_Cmd ); // libreo 1 lugar para el blog
      Free_Conn    ( tpcb    );
      tcp_close    ( tpcb    ); // cierro
      return ERR_OK;
   }
}
err_t Accept_Rs232_Fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   tcp_recv ( newpcb ,Rcv_Rs232_Fn );
   tcp_arg  ( newpcb ,NULL         );
   Add_Conn ( newpcb ,NORMAL       );
   return 0;
}
void Create_Rs232_Socket(void* nil)
{
   Soc_Rs232=tcp_new                 (                                                     );
   tcp_bind                          ( Soc_Rs232 ,IP_ADDR_ANY ,Usr_Flash_Params.Rs232_Port );
   Soc_Rs232=tcp_listen_with_backlog ( Soc_Rs232 ,TCP_DEFAULT_LISTEN_BACKLOG               );
   tcp_accept                        ( Soc_Rs232 ,Accept_Rs232_Fn                          );
}/*}}}*/
//---SNIFFER ETH&RS232 to ETH--------------------------------------------------------------{{{
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
   Add_Conn ( newpcb ,NORMAL|SNIFF);
   return 0;
}
void Create_Sniffer_Socket(void* nil)
{
   Soc_Sniffer=tcp_new                 (                                                     );
   tcp_bind                            ( Soc_Sniffer ,IP_ADDR_ANY ,Usr_Flash_Params.Sniffer_Port );
   Soc_Sniffer=tcp_listen_with_backlog ( Soc_Sniffer ,TCP_DEFAULT_LISTEN_BACKLOG                 );
   tcp_accept                          ( Soc_Sniffer ,Accept_Sniffer_Fn                          );
}/*}}}*/
//-------VIRTUAL RS232--------------------------------------------------------------{{{
err_t Rcv_Virtual_Fn(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   if(p!=NULL)  {
      Emulate_Uart_Rx_Data(p->payload,p->len);
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
err_t Accept_Virtual_Fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   tcp_recv ( newpcb ,Rcv_Virtual_Fn );
   tcp_arg  ( newpcb ,NULL           );
   Add_Conn ( newpcb ,VIRTUAL        );
   return 0;
}
void Create_Virtual_Socket(void* nil)
{
   Soc_Virtual=tcp_new                 (                                                     );
   tcp_bind                            ( Soc_Virtual ,IP_ADDR_ANY ,Usr_Flash_Params.Virtual_Port );
   Soc_Virtual=tcp_listen_with_backlog ( Soc_Virtual ,TCP_DEFAULT_LISTEN_BACKLOG                 );
   tcp_accept                          ( Soc_Virtual ,Accept_Virtual_Fn                          );
}/*}}}*/
//---------CLIENTS--------------------------------------------------------------{{{
void Init_Clients_Socket(void)
{
   uint8_t i;
   for(i=0;i<MAX_TCP_CLIENTS;i++) {
      Clients_List[i].Accept_Fn   = Accept_Cmd_Fn;
      Clients_List[i].Port        = 12345+i;
      Clients_List[i].Tout        = 1;
      Clients_List[i].Actual_Tout = 1;
      Clients_List[i].tpcb        = NULL;
      Clients_List[i].C_State     = Disabled;
      ipaddr_aton("192.168.2.3",&Clients_List[i].Ip);
   }
   Send_Event(Telnet_Enable_Event,&Clients_List[0].C_State);
}

void Connect_Clients_Socket(void)
{
   uint8_t i;
   for(i=0;i<MAX_TCP_CLIENTS;i++) {
      if(Clients_List[i].Accept_Fn!=NULL) {
         if(Clients_List[i].Tout!=0) {
            if(Clients_List[i].Actual_Tout==0) {
               Clients_List[i].Actual_Tout=Clients_List[i].Tout;
               if(Clients_List[i].tpcb!=NULL) {
                  if(Clients_List[i].tpcb->state==CLOSED || Clients_List[i].tpcb->state==SYN_SENT){
                     tcp_connect( Clients_List[i].tpcb,
                                 &Clients_List[i].Ip,
                                 Clients_List[i].Port,
                                 Clients_List[i].Accept_Fn);
                  }
               }
               else {
                  Clients_List[i].tpcb=tcp_new();
               }
            }
            else {
               Clients_List[i].Actual_Tout--;
            }
         }
      }
   }
}
int Cmd_Print_Clients_List(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   uint8_t i;
   char Ip_Buf[16];
   for(i=0;i<MAX_TCP_CLIENTS;i++) {
      ipaddr_ntoa_r(&Clients_List[i].Ip, Ip_Buf,sizeof(Ip_Buf));
      UART_ETHprintf(P->tpcb,"Client:%d Ip:%s Port:%d Tout:%d Actual Tout:%d Conn State:%d \r\n",
                     i,
                     Ip_Buf,
                     Clients_List[i].Port,
                     Clients_List[i].Tout,
                     Clients_List[i].Actual_Tout,
                     Clients_List[i].tpcb->state);
   }
   return 0;
}
int Cmd_Connect(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   Telnet_Clients_Rti();
//   Connect_Clients_Socket();
   return 0;
}
int Cmd_Init_Client_List(struct Parser_Queue_Struct* P, int argc, char *argv[]) {
   Init_Clients_Socket();
   return 0;
}/*}}}*/


struct Soc_Clients_Struct* Clients4C_State(const State** C_State)
{
   return (struct Soc_Clients_Struct*) C_State;
}

void Waiting_Rti(void)
{
   struct Soc_Clients_Struct* CS=Clients4C_State(Actual_Sm());
   if(CS->Actual_Tout==0) {
      Send_Event(Telnet_Connect_Event,Actual_Sm());
      CS->Actual_Tout=CS->Tout;
   }
   else
      CS->Actual_Tout--;
}
void Waiting_Connect(void)
{
   struct Soc_Clients_Struct* SC=Clients4C_State(Actual_Sm());
   SC->tpcb=tcp_new();
   tcp_connect ( SC-> tpcb,
                &SC-> Ip,
                 SC-> Port,
                 SC-> Accept_Fn);
}

void Telnet_Clients_Rti(void)
{
   uint8_t i;
   for( i = 0; i < MAX_TCP_CLIENTS  ; i ++ ) {
      Send_Event(Rti_Event,&Clients_List[i].C_State);
   }
}

const State Disabled  [ ] =
{
{ Telnet_Enable_Event  ,Rien            ,Waiting    },
{ Rti_Event            ,Rien            ,Disabled   },
{ ANY_Event            ,Rien            ,Disabled   },
};
const State Waiting   [ ] =
{
{ Rti_Event            ,Waiting_Rti     ,Waiting    },
{ Telnet_Connect_Event ,Waiting_Connect ,Connecting },
{ ANY_Event            ,Rien            ,Waiting    },
};
const State Connecting[ ] =
{
{ ANY_Event            ,Rien            ,Connecting },
};
const State Connected [ ] =
{
{ ANY_Event            ,Rien            ,Connected  },
};
const State Closing   [ ] =
{
{ ANY_Event            ,Rien            ,Closing    },
};

