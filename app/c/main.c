#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/flash.h"
#include "utils/uartstdio.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "commands.h"
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
#include "schedule.h"
#include "telnet.h"
#include "udp.h"
#include "one_wire_transport.h"
#include "usr_flash.h"


// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


// Required by lwIP library to support any host-related timer functions.
void lwIPHostTimerHandler(void)
{
}

int main(void)
{
   Init_Clk();
//
//    //
//    // Configure the device pins.
//    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig ( 0, 115200, configCPU_CLOCK_HZ);
    UART_ETHprintf ( UART_MSG,"\033[2J\033[H" );
    UART_ETHprintf ( UART_MSG,"Ethergate\n\n" );
//    // Convert the 24/24 split MAC address from NV ram into a 32/16 split
//    // MAC address needed to program the hardware registers, then program
//    // the MAC address into the Ethernet Controller registers.
//    //
   Init_Usr_Flash          ( );

   lwIPInit(configCPU_CLOCK_HZ,
            Usr_Flash_Params.Mac_Addr,
            Usr_Flash_Params.Ip_Addr,
            Usr_Flash_Params.Mask_Addr,
            Usr_Flash_Params.Gateway_Addr,
            IPADDR_USE_STATIC);
//
//
   Init_Wdog    ( );
   Init_Events  ( );
   Init_Schedule();
   xTaskCreate ( State_Machine      ,"sm"            ,configMINIMAL_STACK_SIZE ,NULL ,2 ,NULL );
   xTaskCreate ( Schedule           ,"schedule"      ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
   xTaskCreate ( Led_Link_Task      ,"led link"      ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
   xTaskCreate ( Led_Rgb_Task       ,"leds RGB"      ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
   xTaskCreate ( Led_Serial_Task    ,"led serial"    ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
   xTaskCreate ( User_Commands_Task ,"user commands" ,configMINIMAL_STACK_SIZE*2 ,NULL ,1 ,NULL );
   xTaskCreate ( Button1_Task       ,"Button1"       ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
   Init_One_Wire_Transport ( );
   Init_Telnet             ( );
   Init_Udp                ( );
//
//
   vTaskStartScheduler();
   while(1)
       ;
}
