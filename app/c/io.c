#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_pwm.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "utils/ustdlib.h"
#include "io.h"

//*****************************************************************************
//
// Hardware connection for the user LED.
//
//*****************************************************************************
#define LED_PORT_BASE GPIO_PORTN_BASE
#define LED_PIN GPIO_PIN_0

//*****************************************************************************
//
// Hardware connection for the animation LED.
//
//*****************************************************************************
#define LED_ANIM_PORT_BASE GPIO_PORTN_BASE
#define LED_ANIM_PIN GPIO_PIN_1

void io_init(void)
{
    //
    // Configure Port N0 for as an output for the status LED.
    //
    ROM_GPIOPinTypeGPIOOutput(LED_PORT_BASE, LED_PIN);

    //
    // Configure Port N0 for as an output for the animation LED.
    //
    ROM_GPIOPinTypeGPIOOutput(LED_ANIM_PORT_BASE, LED_ANIM_PIN);

    //
    // Initialize LED to OFF (0)
    //
    ROM_GPIOPinWrite(LED_PORT_BASE, LED_PIN, 0);

    //
    // Initialize animation LED to OFF (0)
    //
    ROM_GPIOPinWrite(LED_ANIM_PORT_BASE, LED_ANIM_PIN, 0);

}

void io_set_led(bool bOn)
{
    //
    // Turn the LED on or off as requested.
    //
    ROM_GPIOPinWrite(LED_PORT_BASE, LED_PIN, bOn ? LED_PIN : 0);
}

//*****************************************************************************
//
// Return LED state as an integer, 1 on, 0 off.
//
//*****************************************************************************
int io_is_led_on(void)
{
    return ROM_GPIOPinRead(LED_PORT_BASE, LED_PIN);
}
