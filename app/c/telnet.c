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
   Binding  [ ],
   Listening[ ],
   Accepting[ ],
   Connected[ ],
   Idle     [ ];

const State* Telnet_Sm;
char param[]="soy un parametro\n";
//-------------------------------------------------------------------------------------
void Init_Telnet(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
   Telnet_Sm=Creating;
   New_Periodic_Schedule(10,ANY_Event,Telnet());
}

const State**  Telnet      ( void ) { return &Telnet_Sm                 ;} 
void           Telnet_Rti  ( void ) { Atomic_Send_Event(ANY_Event,Telnet())    ;} 


struct tcp_pcb* soc;
void Create_Socket(void)
{
   soc=tcp_new();
   UARTprintf("nuevo socket id=%d\n",(int)soc);
}
void Bind_Socket(void)
{
    int err=tcp_bind(soc,IP_ADDR_ANY,1234);
   UARTprintf("Bind socket err=%d\n",err);
}
//----------------------------------------------------------------------------------------------------
err_t Rcv_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   if(p!=NULL) {
      UARTwrite (p->payload,p->len);
      return 0;
   }
   else {
      UARTprintf("session cerrada",soc,err);
      tcp_close(tpcb);
      return ERR_ABRT;
   }
}
err_t accept_fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   UARTprintf("llego conexion=%d error=%d\n",soc,err);
   UARTprintf((char*)arg);
   tcp_recv(newpcb,Rcv_Fn);
   Atomic_Send_Event(Accept_Event,Telnet());
   return 0;
}

void Listen_Socket(void)
{
   tcp_arg(soc, (void *)param);
   soc=tcp_listen(soc);
   tcp_accept(soc,accept_fn);
   UARTprintf("Listen socket =%d\n",soc);
   UARTprintf("socket state =%d\n",soc->state);
}
//----------------------------------------------------------------------------------------------------
void Set_Rcv_Fn(void)
{
   UARTprintf("listo para recibir Rcv\n");
}

 const State Creating [ ]=
{
   { ANY_Event ,Create_Socket                 ,Binding  } ,
};

 const State Binding [ ]=
{
   { ANY_Event ,Bind_Socket                 ,Listening  } ,
};

 const State Listening [ ]=
{
   { ANY_Event ,Listen_Socket                 ,Accepting  } ,
};
 const State Accepting [ ]=
{
   { Accept_Event ,Set_Rcv_Fn ,Connected  } ,
   { ANY_Event    ,Rien       ,Accepting  }      ,
};
 const State Connected [ ]=
{
   { ANY_Event               ,Rien ,Connected  }      ,
};

 const State Idle [ ]=
{
   { ANY_Event ,Rien                 ,Idle  } ,
};
