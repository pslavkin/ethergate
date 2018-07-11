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

const State* ActualState;
Events Event;

const State 
   Doing_Nothing[];
//-----------------------------------------------------------------------
void           Set_State  ( const State* New_State,const State** Machine ) { *Machine=New_State                       ;}
const State**  Empty_Sm   ( void                                         ) { return (const State**)Empty_State_Machine;}
const State*   Empty_App  ( void                                         ) { return Doing_Nothing                     ;}
void           Rien       ( void                                         ) { }
// void      Delay_100useg  (void)                  {unsigned int i;for(i=0;i<400;i++);}
void           Delay_Useg ( unsigned int Useg                            )
{
 unsigned int i;
 Useg=Useg*3 + (Useg*5)/10;
 for(i=0;i<Useg;i++);
}
//-----------------------------------------------------------------------
unsigned int   Actual_Event ( void ) { return Event.Event  ;}
const State**  Actual_Sm    ( void ) { return Event.Machine;}
void           Soft_Reset   ( void ) { }
//-----------------------------------------------------------------------
void State_Machine(void)               //esta funcion ejecuta la maquina de estados donde el evento viene en la variable Event... que se decidio que no sea por parametro para permitir la recursividad infinita...  
{
 Event  = Atomic_Read_Event();
 if(Event.Machine!=Empty_State_Machine)
 {
  ActualState = *(Event.Machine);
  for(;ActualState->Event!=ANY_Event && ActualState->Event!=Event.Event;ActualState++);
  *Event.Machine=ActualState->Next_State;
  ActualState->Func();
 }
 return;
}

//------------------------------------------------------------------------------------------
void  Print_Doing_Nothing  (void)   {} //Send_NVDebug_State_Machine_Data2Serial(13,"Doing Nothing\n");}
//------------------------------------------------------------------------------------------
const State Doing_Nothing[] =
{
{ ANY_Event ,Print_Doing_Nothing ,Doing_Nothing} ,
};
//------------------------------------------------------------------------------------------

