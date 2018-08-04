#ifndef  BUTTONS
#define  BUTTONS

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-----------------------------------------------------------
extern bool    Button1_Read      ( void      );
extern void    Button1IntHandler ( void      );
extern void    Button1_Task      ( void* nil );
//----------------------------------------------------
//button
#define BUTTON1_PIN        GPIO_PIN_0
#define BUTTON1_PORT       GPIO_PORTH_BASE
#define BUTTON1_PERIPH     SYSCTL_PERIPH_GPIOH
#define BUTTON1_INT        INT_GPIOH
//---------------------------------------------------------

#endif

