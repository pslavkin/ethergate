#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/emac.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "buttons.h"
#include "leds.h"
#include "utils/uartstdio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-------------------------------------------------------------------
QueueHandle_t Button1_Queue;
void Init_Button1(void)
{
   MAP_SysCtlPeripheralEnable ( BUTTON1_PERIPH                                                   );
   MAP_GPIOPinTypeGPIOInput   ( BUTTON1_PORT,BUTTON1_PIN                                         );
   MAP_GPIOPadConfigSet       ( BUTTON1_PORT,BUTTON1_PIN,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU );

   MAP_GPIOIntTypeSet ( BUTTON1_PORT,BUTTON1_PIN, GPIO_FALLING_EDGE );
   MAP_GPIOIntClear   ( BUTTON1_PORT, BUTTON1_PIN                   );
   MAP_GPIOIntEnable  ( BUTTON1_PORT,BUTTON1_PIN                    );
   MAP_IntEnable      ( BUTTON1_INT                                 );
}

void Button1IntHandler(void)
{
   TickType_t Tick=xTaskGetTickCount();
   BaseType_t Context_Change=pdFALSE;
   MAP_GPIOIntClear  ( BUTTON1_PORT, BUTTON1_PIN                    );
   xQueueSendFromISR ( Button1_Queue,&Tick,&Context_Change );
   portYIELD_FROM_ISR(Context_Change);
}

bool Button1_Read (void)
{
   return !MAP_GPIOPinRead(BUTTON1_PORT,BUTTON1_PIN);
}

void Button1_Task(void* nil)
{
   TickType_t Tick;
   Button1_Queue = xQueueCreate(1,sizeof (TickType_t));
   SysCtlPeripheralEnable ( LED_SERIAL_PERIPH               );
   GPIOPinTypeGPIOOutput  ( LED_SERIAL_PORT,LED_SERIAL_PIN  );
   GPIOPinReset           ( LED_SERIAL_PORT, LED_SERIAL_PIN );
   Init_Button1();
   while(1) {
      if(xQueueReceive(Button1_Queue,&Tick,pdMS_TO_TICKS(200))==pdFALSE) {
   //      GPIOPinToogle ( LED_SERIAL_PORT, LED_SERIAL_PIN);
         }
      else {
         GPIOPinSet   ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
         vTaskDelay   ( pdMS_TO_TICKS(20                ));
         GPIOPinReset ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
      }
   }
}
//------------------------------------------------------------------
