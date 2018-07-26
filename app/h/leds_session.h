#ifndef  LEDS_SESSION
#define  LEDS_SESSION

#include "state_machine.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
//-----------------------------------------------------------
void Led_Effects_Func            ( void* nil );
//----------------------------------------------------
//degino la posicino de los leds en el puerto N
#define LED_POWER_PIN      GPIO_PIN_1
#define LED_POWER_PORT     GPIO_PORTN_BASE
#define LED_POWER_PERIPH   SYSCTL_PERIPH_GPION

#define LED_LINK_PIN       GPIO_PIN_0
#define LED_LINK_PORT      GPIO_PORTN_BASE
#define LED_LINK_PERIPH    SYSCTL_PERIPH_GPION

//degino la posicino de los leds en el puerto F
#define LED_RUN_PIN         GPIO_PIN_4
#define LED_RUN_PORT        GPIO_PORTF_BASE
#define LED_RUN_PERIPH      SYSCTL_PERIPH_GPIOF
#define LED_ONE_WIRE_PIN    GPIO_PIN_0
#define LED_ONE_WIRE_PORT   GPIO_PORTF_BASE
#define LED_ONE_WIRE_PERIPH SYSCTL_PERIPH_GPIOF

//degino la posicino del boton
#define BUTTON1_PIN    GPIO_PIN_0
#define BUTTON1_PORT   GPIO_PORTJ_BASE
#define BUTTON1_PERIPH SYSCTL_PERIPH_GPIOJ
#define BUTTON2_PIN    GPIO_PIN_1
#define BUTTON2_PORT   GPIO_PORTJ_BASE
#define BUTTON2_PERIPH SYSCTL_PERIPH_GPIOJ
//---------------------------------------------------------
void Set_Led_Effect       ( unsigned char Led,unsigned int Effect );
void Set_Temp_Led_Effect  ( unsigned char Led,unsigned int Effect );
void Set_Fixed_Led_Effect ( unsigned char Led,unsigned int Effect );
//---------------------------------------------------------
enum Leds_Position
{
 Led_Power    = 0,
 Led_Link     = 1,
 Led_Run      = 2,
 Led_One_Wire = 3
};
struct Led_Effect_Struct
 {
  unsigned int Effect;
  unsigned int Temp_Effect;
  void (*On_Function)(void);
  void (*Off_Function)(void);
 };
//---------------------------------------------------------
void Led_Power_Set      ( void );
void Led_Power_Reset    ( void );
void Led_Link_Set       ( void );
void Led_Link_Reset     ( void );
void Led_Run_Set        ( void );
void Led_Run_Reset      ( void );
void Led_One_Wire_Set   ( void );
void Led_One_Wire_Reset ( void );
//---------------------------------------------------------

#endif

