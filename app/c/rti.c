#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/rom_map.h"
#include "utils/lwiplib.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "clk.h"
#include "everythings.h"
#include "commands.h"

#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)
//---------------------------------------------------
void Init_Rti(void)
{
 SysTickPeriodSet ( Actual_Clk_Get( )/SYSTICKHZ);
 SysTickEnable    (                 )     ;
 SysTickIntEnable (                 )     ;
}

void SysTickIntHandler(void)
{
    //
    // Call the lwIP timer handler.
    //
   lwIPTimer(SYSTICKMS);
   Everythings_Rti();
}
//---------------------------------------------------

