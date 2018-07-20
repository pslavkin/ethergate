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
#include "io.h"
#include "commands.h"
#include "opt.h"
#include "clk.h"
#include "rti.h"
#include "state_machine.h"
#include "events.h"
#include "schedule.h"
#include "leds_session.h"
#include "commands.h"
#include "wdog.h"
#include "telnet.h"

const State
   Creating [ ],
   Idle     [ ];

const State* Telnet_Sm;
struct tcp_pcb* soc;
struct udp_pcb* usoc;
//-------------------------------------------------------------------------------------
void Init_Telnet(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
   Telnet_Sm=Creating;
   New_Periodic_Schedule(10,ANY_Event,Telnet());
}

const State**  Telnet      ( void ) { return &Telnet_Sm                 ;} 
void           Telnet_Rti  ( void ) { Atomic_Send_Event(ANY_Event,Telnet())    ;} 

void URcv_Fn (void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t* addr,  u16_t port)
{
   if(p!=NULL) {
      udp_connect(upcb,addr,port);
      udp_send(upcb,p);
      udp_disconnect(upcb);
   }
   UART_ETHprintf(NULL,"llego algo x udp\r\n");

}
void Create_Udp_Socket(void)
{
   UART_ETHprintf(NULL,"new=%d\r\n" ,usoc=udp_new());
   UART_ETHprintf(NULL,"bind=%d\r\n" ,udp_bind(usoc ,IP_ADDR_ANY ,50000));
   udp_recv(usoc,URcv_Fn,NULL);
}
//magia, como me llega el tpcb segun quien corresponda, estare responidendo a ese
//socket y no a otro, con lo cual tengo resuelte los estaodos de cada uno asi si mas
err_t Rcv_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   if(p!=NULL)  {
      ((char*)p->payload)[p->len-1]='\0';
      CmdLineProcess(p->payload,tpcb);
      tcp_recved(tpcb, p->len);        //aviso que el windows crecio
      pbuf_free(p);                    //libreo bufer
      return 0;
   }
   else {
      UART_ETHprintf(NULL,"clsed\n",soc,err);
      tcp_accepted(soc);               //libreo 1 lugar para el blog
      tcp_close(tpcb);                 //cierro
      return ERR_ABRT;                 //aviso que cerre (no se si esta ok)
   }
}

err_t accept_fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   UART_ETHprintf(NULL,"backlog=%d\n",((struct tcp_pcb_listen*)soc)->accepts_pending);
   tcp_recv(newpcb,Rcv_Fn);
   return 0;
}
void Create_Socket(void)
{
   UART_ETHprintf(NULL,"new=%d\n" ,soc=tcp_new());
   UART_ETHprintf(NULL,"bind=%d\n" ,tcp_bind(soc ,IP_ADDR_ANY ,1234));
   UART_ETHprintf(NULL,"listen=%d\n" ,soc=tcp_listen_with_backlog(soc,3));
   UART_ETHprintf(NULL,"backlog=%d\n",((struct tcp_pcb_listen*)soc)->backlog);
   tcp_accept(soc,accept_fn);
   Create_Udp_Socket();
}
//----------------------------------------------------------------------------------------------------
const State Creating [ ]=
{
{ANY_Event ,Create_Socket ,Idle} ,
};

const State Idle [ ]=
{
{ANY_Event ,Rien          ,Idle}     ,
};
