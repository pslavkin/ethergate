#include "stdint.h"
#include "stdbool.h"
#include "../drv/debug.h"
#include "../h/wdog.h"
#include "rom.h"
#include "../h/leds_session.h"
#include "../inc/hw_ints.h"
#include "../drv/watchdog.h"


__attribute__ ((interrupt ("IRQ"))) void Wdog_Handler 	(void) 
{
  WatchdogIntClear(WATCHDOG0_BASE);
  Send_NVData2Sci0(6,"wdog\r\n");
}

void Wdog_Clear(void)	
{
 ROM_WatchdogReloadSet(WATCHDOG0_BASE, Actual_Clk_Get());
}


void Init_Wdog(void)	
{
 ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);

 //
 // Enable the watchdog interrupt.  
 //
 IntEnable(INT_WATCHDOG);

 //
 // Set the period of the watchdog timer to 1 second.
 //
 ROM_WatchdogReloadSet(WATCHDOG0_BASE, Actual_Clk_Get());

 //
 // Enable reset generation from the watchdog timer.
 //
 WatchdogResetEnable(WATCHDOG0_BASE);

 //
 // Enable the watchdog timer.
 //
 ROM_WatchdogEnable(WATCHDOG0_BASE);

}
