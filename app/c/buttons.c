#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/emac.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "inc/hw_ints.h"
#include "buttons.h"
#include "leds.h"
#include "state_machine.h"
#include "events.h"
#include "one_wire_network.h"
#include "usr_flash.h"
#include "leds.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-------------------------------------------------------------------
QueueHandle_t Button1_Queue;
struct Button1_Data_Struct
{
   TickType_t Ticks;
   uint8_t Hi_Lo;
};

void Init_Button1(void)
{
   MAP_SysCtlPeripheralEnable ( BUTTON1_PERIPH                                                   );
   MAP_GPIOPinTypeGPIOInput   ( BUTTON1_PORT,BUTTON1_PIN                                         );
   MAP_GPIOPadConfigSet       ( BUTTON1_PORT,BUTTON1_PIN,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU );

   MAP_GPIOIntTypeSet ( BUTTON1_PORT,BUTTON1_PIN, GPIO_BOTH_EDGES );
   MAP_GPIOIntClear   ( BUTTON1_PORT, BUTTON1_PIN                 );
   MAP_GPIOIntEnable  ( BUTTON1_PORT,BUTTON1_PIN                  );
   MAP_IntPrioritySet ( INT_GPIOH, 6<<5                           );
   MAP_IntEnable      ( BUTTON1_INT                               );
}

void Button1IntHandler(void)
{
   struct Button1_Data_Struct B;
   B.Ticks=xTaskGetTickCount();
   B.Hi_Lo=MAP_GPIOPinRead(BUTTON1_PORT,BUTTON1_PIN)==BUTTON1_PIN;
   BaseType_t Context_Change=pdFALSE;
   MAP_GPIOIntClear   ( BUTTON1_PORT, BUTTON1_PIN        );
   xQueueSendFromISR  ( Button1_Queue,&B,&Context_Change );
   portYIELD_FROM_ISR ( Context_Change                   );
}

bool Button1_Read (void)
{
   return !MAP_GPIOPinRead(BUTTON1_PORT,BUTTON1_PIN);
}

void Button1_Task(void* nil)
{
   struct Button1_Data_Struct B;
   uint8_t i;
   Button1_Queue = xQueueCreate(10,sizeof (struct Button1_Data_Struct));
   Init_Button1();
   while(1) {
      do {
          while(xQueueReceive(Button1_Queue,&B,portMAX_DELAY)==pdFALSE)
            ;
          Led_Rgb_Only_White();
         } while (B.Hi_Lo==1);

         for(i=0;i<10;i++) {
             Led_Rgb_Only_White();
             if(xQueueReceive(Button1_Queue,&B,pdMS_TO_TICKS(1000))==pdTRUE)
                break;
         }
         if(i==10) {
            Usr_Flash2Defult_Values();
            Save_Usr_Flash();
            vTaskDelay(pdMS_TO_TICKS(2000));
            Soft_Reset();
            UART_ETHprintf(DEBUG_MSG,"reseting to defult values\n");
         }
   }
}
//------------------------------------------------------------------
