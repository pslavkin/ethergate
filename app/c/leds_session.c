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
#include "everythings.h"
#include "leds_session.h"
#include "state_machine.h"
#include "events.h"
#include "schedule.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-------------------------------------------------------------------
void Init_Buttons(void)
{
 SysCtlPeripheralEnable ( BUTTON1_PERIPH                                                   );
 GPIOPinTypeGPIOInput   ( BUTTON1_PORT,BUTTON1_PIN                                         );
 GPIOPadConfigSet       ( BUTTON1_PORT,BUTTON1_PIN,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU );

 SysCtlPeripheralEnable ( BUTTON2_PERIPH                                                   );
 GPIOPinTypeGPIOInput   ( BUTTON2_PORT,BUTTON2_PIN                                         );
 GPIOPadConfigSet       ( BUTTON2_PORT,BUTTON2_PIN,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU );
}
uint8_t Button1_Read (void)
{
 return GPIOPinRead(BUTTON1_PORT,BUTTON1_PIN);
}

void Init_Leds(void)
{
 SysCtlPeripheralEnable (LED_POWER_PERIPH);
 GPIOPinTypeGPIOOutput  (LED_POWER_PORT,LED_POWER_PIN);

 SysCtlPeripheralEnable (LED_LINK_PERIPH);
 GPIOPinTypeGPIOOutput  (LED_LINK_PORT,LED_LINK_PIN);

 SysCtlPeripheralEnable (LED_RUN_PERIPH);
 GPIOPinTypeGPIOOutput  (LED_RUN_PORT,LED_RUN_PIN);

 SysCtlPeripheralEnable (LED_ONE_WIRE_PERIPH);
 GPIOPinTypeGPIOOutput  (LED_ONE_WIRE_PORT,LED_ONE_WIRE_PIN);
}
void Led_Power_Set      ( void ) { GPIOPinSet (LED_POWER_PORT,LED_POWER_PIN)         ;}
void Led_Power_Reset    ( void ) { GPIOPinReset  (LED_POWER_PORT,LED_POWER_PIN)      ;}
void Led_Link_Set       ( void ) { GPIOPinSet (LED_LINK_PORT,LED_LINK_PIN)           ;}
void Led_Link_Reset     ( void ) { GPIOPinReset  (LED_LINK_PORT,LED_LINK_PIN)        ;}
void Led_Run_Set        ( void ) { GPIOPinSet (LED_RUN_PORT,LED_RUN_PIN)             ;}
void Led_Run_Reset      ( void ) { GPIOPinReset  (LED_RUN_PORT,LED_RUN_PIN)          ;}
void Led_One_Wire_Set   ( void ) { GPIOPinSet (LED_ONE_WIRE_PORT,LED_ONE_WIRE_PIN)   ;}
void Led_One_Wire_Reset ( void ) { GPIOPinReset  (LED_ONE_WIRE_PORT,LED_ONE_WIRE_PIN);}

struct Led_Effect_Struct Led_Effects[]=
{
 {0xAAAA ,0x8FFF ,Led_Power_Set    ,Led_Power_Reset}    ,
 {0xF000 ,0x01FF ,Led_Link_Set     ,Led_Link_Reset}     ,
 {0xFF00 ,0x003F ,Led_Run_Set      ,Led_Run_Reset}      ,
 {0x0000 ,0x0001 ,Led_One_Wire_Set ,Led_One_Wire_Reset} ,
};
//----------
void Set_Led_Effect       ( unsigned char Led,unsigned int Effect ) { Led_Effects[Led].Effect=Led_Effects[Led].Temp_Effect=Effect                                               ;}
void Set_Temp_Led_Effect  ( unsigned char Led,unsigned int Effect ) { Led_Effects[Led].Temp_Effect=Effect                                                                       ;}
void Set_Fixed_Led_Effect ( unsigned char Led,unsigned int Effect ) { Led_Effects[Led].Effect=Effect                                                                            ;}
void Led_Effects_Func ( void* nil )
{
   unsigned char Actual_Led;
   Init_Leds();
   while(1) {
      for(Actual_Led=0;Actual_Led<sizeof(Led_Effects)/sizeof(struct Led_Effect_Struct);Actual_Led++) {
         (Led_Effects[Actual_Led].Temp_Effect&0x0001)?Led_Effects[Actual_Led].On_Function():Led_Effects[Actual_Led].Off_Function(); //se usa el metodo del shifteop para trasladar la indo de la variable al estado del led...
         if(!(Led_Effects[Actual_Led].Temp_Effect>>=1)) Led_Effects[Actual_Led].Temp_Effect=Led_Effects[Actual_Led].Effect;
      }
   vTaskDelay( 100 / portTICK_RATE_MS );
   }
}
//------------------------------------------------------------------
