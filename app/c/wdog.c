#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "wdog.h"
#include "state_machine.h"
#include "one_wire_network.h"
#include "usr_flash.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

uint32_t Uptime    = 0;
void Wdog_Handler ( void )
{
//   MAP_WatchdogIntClear(WATCHDOG0_BASE);
   MAP_UARTCharPut(UART0_BASE, 'w');
   MAP_UARTCharPut(UART0_BASE, 'd');
   MAP_UARTCharPut(UART0_BASE, 'o');
   MAP_UARTCharPut(UART0_BASE, 'g');
   MAP_UARTCharPut(UART0_BASE, ' ');
}

uint32_t Read_Uptime(void)
{
   return Uptime;
}
uint8_t Read_Uptime_Secs(void)
{
   return Uptime%60;
}
uint8_t Read_Uptime_Mins(void)
{
   return (Uptime/(60))%60;
}
uint8_t Read_Uptime_Hours(void)
{
   return (Uptime/(3660))%24;
}
uint32_t Read_Uptime_Days(void)
{
   return Uptime/(3660*24);
}
void Wdog_Task(void* nil)
{
   while(1) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      MAP_WatchdogReloadSet(WATCHDOG0_BASE, 2*configCPU_CLOCK_HZ);
      Uptime++;
      if(Uptime==Usr_Flash_Params.Wdog)
         Soft_Reset();
   }
}
void Init_Wdog(void)
{
   MAP_SysCtlPeripheralEnable ( SYSCTL_PERIPH_WDOG0                  );
   MAP_IntEnable              ( INT_WATCHDOG                         );
   MAP_WatchdogIntEnable      ( WATCHDOG0_BASE                       ); // Enable the watchdog interrupt.
   MAP_WatchdogReloadSet      ( WATCHDOG0_BASE, 2*configCPU_CLOCK_HZ ); // Set the period of the watchdog timer to 1 second.
   MAP_WatchdogResetEnable    ( WATCHDOG0_BASE                       ); // Enable reset generation from the watchdog timer.
   MAP_WatchdogEnable         ( WATCHDOG0_BASE                       ); // Enable the watchdog timer.
   MAP_IntPrioritySet         ( INT_WATCHDOG,1<<5                    );
   xTaskCreate ( Wdog_Task ,"wdog" ,configMINIMAL_STACK_SIZE ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
}
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
   while(1) {
      MAP_UARTCharPut(UART0_BASE, 's');
      MAP_UARTCharPut(UART0_BASE, 't');
      MAP_UARTCharPut(UART0_BASE, 'a');
      MAP_UARTCharPut(UART0_BASE, 'c');
      MAP_UARTCharPut(UART0_BASE, ' ');
   }
}
// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line) {
   while(1) {
      MAP_UARTCharPut(UART0_BASE, 'd');
      MAP_UARTCharPut(UART0_BASE, 'r');
      MAP_UARTCharPut(UART0_BASE, 'i');
      MAP_UARTCharPut(UART0_BASE, 'v');
      MAP_UARTCharPut(UART0_BASE, 'e');
      MAP_UARTCharPut(UART0_BASE, 'r');
      MAP_UARTCharPut(UART0_BASE, ' ');
   }
}
#endif


