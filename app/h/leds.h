#ifndef  LEDS_SESSION
#define  LEDS_SESSION

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
extern SemaphoreHandle_t   Led_Link_Semphr                   ;
extern void                Led_Link_Task        ( void* nil );
extern void                Led_Serial_Task      ( void* nil );
extern void                vApplicationIdleHook ( void      );
extern void                Led_Rgb_Task         ( void* nil );
extern void                Led_Green_Set        ( void      );
extern void                Led_Green_Reset      ( void      );
extern void                Led_Red_Set          ( void      );
extern void                Led_Red_Reset        ( void      );
extern void                Led_Blue_Set         ( void      );
extern void                Led_Blue_Reset       ( void      );
extern float               Get_T_Prom           ( void      );
//----------------------------------------------------
//red
#define LED_RED_PIN        GPIO_PIN_1
#define LED_RED_PORT       GPIO_PORTM_BASE
#define LED_RED_PERIPH     SYSCTL_PERIPH_GPIOM
//green
#define LED_GREEN_PIN      GPIO_PIN_0
#define LED_GREEN_PORT     GPIO_PORTM_BASE
#define LED_GREEN_PERIPH   SYSCTL_PERIPH_GPIOM
//blue
#define LED_BLUE_PIN       GPIO_PIN_2
#define LED_BLUE_PORT      GPIO_PORTM_BASE
#define LED_BLUE_PERIPH    SYSCTL_PERIPH_GPIOM

//led de link y data
#define LED_LINK_PIN          GPIO_PIN_6
#define LED_LINK_PORT         GPIO_PORTC_BASE
#define LED_LINK_PERIPH       SYSCTL_PERIPH_GPIOC

//led del lado de las RJ del serial
#define LED_SERIAL_PIN         GPIO_PIN_6
#define LED_SERIAL_PORT        GPIO_PORTK_BASE
#define LED_SERIAL_PERIPH      SYSCTL_PERIPH_GPIOK

//button
#define BUTTON1_PIN        GPIO_PIN_0
#define BUTTON1_PORT       GPIO_PORTH_BASE
#define BUTTON1_PERIPH     SYSCTL_PERIPH_GPIOH
#define BUTTON1_INT        INT_GPIOH
//---------------------------------------------------------

#endif

