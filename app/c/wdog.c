#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/flash.h"
#include "utils/uartstdio.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "wdog.h"
#include "clk.h"

void Wdog_Handler ( void )
{
//   MAP_WatchdogIntClear(WATCHDOG0_BASE);
   UARTprintf("wdog\n");
}

void Wdog_Clear(void)
{
  MAP_WatchdogReloadSet(WATCHDOG0_BASE, Actual_Clk_Get());
}

void Init_Wdog(void)
{
   MAP_SysCtlPeripheralEnable ( SYSCTL_PERIPH_WDOG0             ) ;
   MAP_IntEnable(INT_WATCHDOG);
   MAP_WatchdogIntEnable      ( WATCHDOG0_BASE                  ) ; // Enable the watchdog interrupt.
   MAP_WatchdogReloadSet      ( WATCHDOG0_BASE, Actual_Clk_Get( )); // Set the period of the watchdog timer to 1 second.
   MAP_WatchdogResetEnable    ( WATCHDOG0_BASE                  ) ; // Enable reset generation from the watchdog timer.
   MAP_WatchdogEnable         ( WATCHDOG0_BASE                  ) ; // Enable the watchdog timer.
}
