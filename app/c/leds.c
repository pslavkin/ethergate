#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/emac.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "leds.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-------------------------------------------------------------------
void Init_Led_RGB(void)
{
 MAP_SysCtlPeripheralEnable (LED_RED_PERIPH);
 MAP_GPIOPinTypeGPIOOutput  (LED_RED_PORT,LED_RED_PIN);
 MAP_SysCtlPeripheralEnable (LED_GREEN_PERIPH);
 MAP_GPIOPinTypeGPIOOutput  (LED_GREEN_PORT,LED_GREEN_PIN);
 MAP_SysCtlPeripheralEnable (LED_BLUE_PERIPH);
 MAP_GPIOPinTypeGPIOOutput  (LED_BLUE_PORT,LED_BLUE_PIN);
}

void Led_Green_Set   ( void ) { GPIOPinReset ( LED_GREEN_PORT,LED_GREEN_PIN);}
void Led_Green_Reset ( void ) { GPIOPinSet   ( LED_GREEN_PORT,LED_GREEN_PIN);}
void Led_Red_Set     ( void ) { GPIOPinReset ( LED_RED_PORT,LED_RED_PIN)    ;}
void Led_Red_Reset   ( void ) { GPIOPinSet   ( LED_RED_PORT,LED_RED_PIN)    ;}
void Led_Blue_Set    ( void ) { GPIOPinReset ( LED_BLUE_PORT,LED_BLUE_PIN)  ;}
void Led_Blue_Reset  ( void ) { GPIOPinSet   ( LED_BLUE_PORT,LED_BLUE_PIN)  ;}


void vApplicationIdleHook(void)
{
//   static uint16_t State=0;
//   MAP_GPIOPinWrite(LED_GREEN_PORT,LED_GREEN_PIN,++State&0x4000?LED_GREEN_PIN:0);
}


SemaphoreHandle_t Led_Link_Semphr;
//te prende cuando hay link y ademas destella uando hay datos
void Led_Link_Task ( void* nil )
{
   MAP_SysCtlPeripheralEnable (LED_LINK_PERIPH);
   MAP_GPIOPinTypeGPIOOutput  (LED_LINK_PORT,LED_LINK_PIN);
   bool Link_State=false;
   Led_Link_Semphr = xSemaphoreCreateCounting(2,0);
   while(1) {
      Link_State=EMACPHYLinkUp();
      if(Link_State)
         while(1) {
            GPIOPinSet ( LED_LINK_PORT,LED_LINK_PIN ) ;
            vTaskDelay ( pdMS_TO_TICKS(10                   ));
            if( xSemaphoreTake ( Led_Link_Semphr, pdMS_TO_TICKS(1000))==pdFAIL) {
               break;
            }
            GPIOPinReset ( LED_LINK_PORT,LED_LINK_PIN ) ;
            vTaskDelay   ( pdMS_TO_TICKS(50                  ));
         }
      else {
         GPIOPinReset ( LED_LINK_PORT,LED_LINK_PIN ) ;
         vTaskDelay   ( pdMS_TO_TICKS(1000                 ));
      }

   }
}
//--------------------------------------------------------------------------------
void Led_Serial_Task(void* nil)
{
   while(1)
      vTaskDelay ( pdMS_TO_TICKS(100000 ));
}
//--------------------------------------------------------------------------------
void Led_Rgb_Task(void* nil)
{
   Init_Led_RGB();
   while(1) {
      Led_Red_Set();
      vTaskDelay ( pdMS_TO_TICKS(1000 ));
      Led_Red_Reset();
      Led_Green_Set();
      vTaskDelay ( pdMS_TO_TICKS(1000 ));
      Led_Green_Reset();
      Led_Blue_Set();
      vTaskDelay ( pdMS_TO_TICKS(1000 ));
      Led_Blue_Reset();
   }
}
//------------------------------------------------------------------
