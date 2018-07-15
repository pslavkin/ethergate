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
#include "drivers/pinout.h"
#include "io.h"
#include "commands.h"
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
//-------------------------------------------------------------------------------------
void Init_Telnet(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
   Telnet_Sm=Creating;
   New_Periodic_Schedule(10,ANY_Event,Telnet());
}

const State**  Telnet      ( void ) { return &Telnet_Sm                 ;} 
void           Telnet_Rti  ( void ) { Atomic_Send_Event(ANY_Event,Telnet())    ;} 


struct tcp_pcb* soc;
err_t Rcv_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   if(p!=NULL) {
      UARTwrite (p->payload,p->len);
      tcp_recved(tpcb, p->len);        //aviso que el windows crecio
      pbuf_free(p);                    //libreo bufer
      return 0;
   }
   else {
      UARTprintf("session cerrada\n",soc,err);
      tcp_accepted(soc);               //libreo 1 lugar para el blog
      tcp_close(tpcb);                 //cierro
      return ERR_ABRT;                 //aviso que cerre (no se si esta ok)
   }
}

struct tcp_pcb* s;
err_t accept_fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   UARTprintf("llego conexion=%d error=%d\n",newpcb,err);
   UARTprintf("backlog=%d\n",((struct tcp_pcb_listen*)soc)->accepts_pending);
   s=newpcb;
   tcp_recv(newpcb,Rcv_Fn);
   return 0;
}
void Create_Socket(void)
{
   UARTprintf("new=%d\n" ,soc=tcp_new());
   UARTprintf("bind=%d\n" ,tcp_bind(soc ,IP_ADDR_ANY ,1234));
   UARTprintf("listen=%d \n" ,soc=tcp_listen_with_backlog(soc,3));
   UARTprintf("backlog=%d\n",((struct tcp_pcb_listen*)soc)->backlog);
   tcp_accept(soc,accept_fn);
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
