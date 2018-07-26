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
#include "udp.h"


//-------------------------------------------------------------------------------------
void Init_Everythings(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
//   Init_Wdog();
   Init_Telnet();
   Init_Udp();

   xTaskCreate(CheckForUserCommands ,"user commands" ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL);
   xTaskCreate(Led_Effects_Func     ,"Leds effect"   ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL);
   xTaskCreate(Schedule             ,"schedule"      ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL);
}
//------------------------------------------------------------------
