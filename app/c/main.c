#include <stdbool.h>/*{{{*/
#include <stdint.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/flash.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "clk.h"
#include "state_machine.h"
#include "events.h"
#include "wdog.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "leds.h"
#include "buttons.h"
#include "commands.h"
#include "rs232.h"
#include "schedule.h"
#include "telnet.h"
#include "udp.h"
#include "parser.h"
#include "one_wire_transport.h"
#include "one_wire_network.h"
#include "usr_flash.h"/*}}}*/

int main(void)
{
   Init_Clk    ( );
   MAP_IntMasterDisable ( );
   Init_Events ( );
   Init_Uart   ( );
   MAP_UARTCharPut(UART0_BASE, '0'); //debug TODO

   Init_Usr_Flash ( );
   Init_Wdog      ( );

#if RS232_ETH_ENABLE
   xTaskCreate ( Rs232_Task      ,"rs232"      ,configMINIMAL_STACK_SIZE   ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
#endif
   xTaskCreate ( State_Machine   ,"sm"         ,configMINIMAL_STACK_SIZE*3 ,NULL ,tskIDLE_PRIORITY+2 ,NULL );
   xTaskCreate ( Schedule        ,"schedule"   ,configMINIMAL_STACK_SIZE   ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
   xTaskCreate ( Led_Link_Task   ,"led link"   ,configMINIMAL_STACK_SIZE   ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
   xTaskCreate ( Led_Rgb_Task    ,"leds RGB"   ,configMINIMAL_STACK_SIZE   ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
   xTaskCreate ( Button1_Task    ,"Button1"    ,configMINIMAL_STACK_SIZE   ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
#if ONE_WIRE_ENABLE
   Init_One_Wire_Transport ( );
#endif
   lwIPInit    ( configCPU_CLOCK_HZ, Usr_Flash_Params.Mac_Addr );
   Init_Telnet (                                               );
   Init_Udp    (                                               );

   MAP_IntMasterEnable ( );

   MAP_UARTCharPut(UART0_BASE, '7'); //debug TODO
   vTaskStartScheduler ( );
   while(1)
       ;
}


