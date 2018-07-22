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
#include "everythings.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


//*****************************************************************************
//
// Interrupt priority definitions.  The top 3 bits of these values are
// significant with lower values indicating higher priority interrupts.
//
//*****************************************************************************
#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0


//*****************************************************************************
//
// Timeout for DHCP address request (in seconds).
//
//*****************************************************************************
#ifndef DHCP_EXPIRE_TIMER_SECS
#define DHCP_EXPIRE_TIMER_SECS  45
#endif

//*****************************************************************************
//
// The current IP address.
//
//*****************************************************************************
//uint32_t g_ui32IPAddress;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************

//*****************************************************************************
//
// Required by lwIP library to support any host-related timer functions.
//
//*****************************************************************************
void lwIPHostTimerHandler(void)
{
}

#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)

int main(void)
{
    uint32_t ui32User0, ui32User1;
    uint8_t pui8MACArray[8];

   Init_Clk();

    //
    // Configure the device pins.
    //
    PinoutSet(false, false);

    UARTStdioConfig(0, 115200, Actual_Clk_Get());
    UARTprintf ("\033[2J\033[H");
    UARTprintf ("Ethernet IO Example\n\n");

//    Init_Rti();
    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.  The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.
    //
    MAP_FlashUserGet(&ui32User0, &ui32User1);
    ui32User0=0x010203;
    ui32User1=0x040506;
    //
    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split
    // MAC address needed to program the hardware registers, then program
    // the MAC address into the Ethernet Controller registers.
    //
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);

    //
    // Set the interrupt priorities.  We set the SysTick interrupt to a higher
    // priority than the Ethernet interrupt to ensure that the file system
    // tick is processed if SysTick occurs while the Ethernet handler is being
    // processed.  This is very likely since all the TCP/IP and HTTP work is
    // done in the context of the Ethernet interrupt.
    //
    MAP_IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    MAP_IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);
    //
    // Initialze the lwIP library, using DHCP.
    //
   lwIPInit(Actual_Clk_Get(), pui8MACArray,0xC0A8020A, 0xFFFFFF00,0xC0A80201, IPADDR_USE_STATIC);

   xTaskCreate(State_Machine ,"sm"  ,configMINIMAL_STACK_SIZE*2 ,NULL ,2 ,NULL);

    Init_Events();
    Init_Everythings();
    vTaskStartScheduler();
    while(1)
       ;
}
