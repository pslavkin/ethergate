#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "state_machine.h"
#include "events.h"
#include "FreeRTOS.h"
#include "timers.h"

const State* ActualState;
Events Event;

const State 
   Doing_Nothing[];
//-----------------------------------------------------------------------
void           Set_State  ( const State* New_State,const State** Machine ) { *Machine=New_State                       ;}
const State**  Empty_Sm   ( void                                         ) { return (const State**)Empty_State_Machine;}
const State*   Empty_App  ( void                                         ) { return Doing_Nothing                     ;}
void           Rien       ( void                                         ) { }
void Delay_Useg(uint32_t d)
{
   MAP_SysCtlDelay((configCPU_CLOCK_HZ/3000000)*d);
}
//-----------------------------------------------------------------------
uint16_t   Actual_Event ( void ) { return Event.Event  ;}
const State**  Actual_Sm    ( void ) { return Event.Machine;}
void           Soft_Reset   ( void ) {
 HWREG(NVIC_APINT) = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
}
//-----------------------------------------------------------------------
void State_Machine(void* nil)               //esta funcion ejecuta la maquina de estados donde el evento viene en la variable Event... que se decidio que no sea por parametro para permitir la recursividad infinita...  
{
   while(1) {
   Event  = Atomic_Read_Event();
   if(Event.Machine!=Empty_State_Machine) {
      ActualState = *(Event.Machine);
      for(;ActualState->Event!=ANY_Event && ActualState->Event!=Event.Event;ActualState++);
      *Event.Machine=ActualState->Next_State;
      ActualState->Func();
   }
   vTaskDelay( pdMS_TO_TICKS(100) ); // Envia la tarea al estado bloqueado durante 500ms
   }
}

//------------------------------------------------------------------------------------------
void  Print_Doing_Nothing  (void)   {} //Send_NVDebug_State_Machine_Data2Serial(13,"Doing Nothing\n");}
//------------------------------------------------------------------------------------------
const State Doing_Nothing[] =
{
{ ANY_Event ,Print_Doing_Nothing ,Doing_Nothing} ,
};
//------------------------------------------------------------------------------------------

