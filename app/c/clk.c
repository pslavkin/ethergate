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
 
static uint32_t Actual_Clk;

void Init_Clk(void)
{
   // Make sure the main oscillator is enabled because this is required 
   // the PHY.  The system must have a 25MHz crystal attached to the OSC
   // pins.  The SYSCTL_MOSC_HIGHFREQ parameter is used when the crystal
   // frequency is 10MHz or higher.
   //
   SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);
   //
   // Run from the PLL at 120 MHz.
   //
   Actual_Clk=MAP_SysCtlClockFreqSet(    SYSCTL_XTAL_25MHZ    |  \
               SYSCTL_OSC_MAIN      |  \
               SYSCTL_USE_PLL       |  \
               SYSCTL_CFG_VCO_480,     \
               120000000 );
}
//devuelve el clk del sistema.. basado en la ultima configuracion..
uint32_t Actual_Clk_Get(void) {return Actual_Clk;}

