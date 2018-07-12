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
   Creating             [ ],
   Binding             [ ],
   Listening             [ ],
   Idle             [ ];

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

struct tcp_pcb* new_soc;
void Listen_Socket(void)
{
    new_soc=tcp_listen(soc);
   UARTprintf("Listen socket =%d\n",new_soc);
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
   { ANY_Event ,Listen_Socket                 ,Idle  } ,
};

 const State Idle [ ]=
{
   { ANY_Event ,Rien                 ,Idle  } ,
};
