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


const State   
   Free_State1             [ ],
   Free_State2             [ ],
   Free_State3             [ ],
   Free_State4             [ ],
   Free_State5             [ ],
   Free_State6             [ ],
   Free_State7             [ ],
   Free_State8             [ ],
   Free_State9             [ ],
   Free_State10            [ ];

const State* Everythings_Sm;           //variable que lleva cuenta del estado de la maquina de estados de "detodo un poco"...
//-------------------------------------------------------------------------------------
void Init_Everythings(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
Everythings_Sm=Free_State1;
 Init_Schedule     ( );
// Init_Leds_Session ( );
// Init_Wdog();
}
//------------------------------------------------------------------
const State**  Everythings      ( void ) { return &Everythings_Sm                 ;} // devuelve la direccion de la maquina de estados Everythings para poder mandarle mensajes.
void           Everythings_Rti  ( void ) { Send_Event(ANY_Event,Everythings())    ;} // manda mensajes ANY a tiempos predefinidos...


const unsigned char buf[]=
{
 "viva  TI\r\n"
 "abajo fs\r\n"
};
void test(void)
{
//   UARTprintf("hola");
}
//--------------------- MAQUINA DE ESTADOS PARA EVERYTHINGS --------------------
const State Free_State1 [ ]=
{
   { ANY_Event ,Rien                 ,Free_State2  } ,
};
const State Free_State2 [ ]=
{
   { ANY_Event ,test                 ,Free_State3  } ,
};
const State Free_State3 [ ]=
{
   { ANY_Event ,Schedule                 ,Free_State4  } ,
};
const State Free_State4 [ ]=
{
   { ANY_Event ,Rien                 ,Free_State5  } ,
};
const State Free_State5 [ ]=
{
   { ANY_Event ,Rien                 ,Free_State6  } ,
};
const State Free_State6 [ ]=
{
   { ANY_Event ,CheckForUserCommands ,Free_State7  } ,
};
const State Free_State7 [ ]=
{
   { ANY_Event ,Rien                 ,Free_State8  } ,
};
const State Free_State8 [ ]=
{
   { ANY_Event ,Led_Effects_Func                 ,Free_State9  } ,
};
const State Free_State9 [ ]=
{
   { ANY_Event ,Rien                 ,Free_State10 } ,
};
const State Free_State10[ ]=
{
   { ANY_Event ,Rien                 ,Free_State1  } ,
};

//-------------------------------------------------------------------------------
