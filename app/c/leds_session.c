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

const State 
   Flashing[];

const State* Leds_Session_Sm;
volatile unsigned char Button_Filter;
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

void Buzzer_Init(void)
{
//  PORTA_PCR8 = (uint32_t)((PORTA_PCR8 & (uint32_t)~0x01000400UL) | (uint32_t)0x0100UL); //asigno la pata del uC a la pata GPIO PTA8 que esta conectada al buzzer
//  GPIOA_PDDR|=0x00000100;                        //seteo la PTA8 como salida
}
void Buzzer_On(void)
{
// GPIOA_PSOR|=0x00000100;    //prendo bit
}
void Buzzer_Off(void)
{
//  GPIOA_PCOR|=0x00000100;   //apago bit
}
void Beep   (void)
{
unsigned int i,j;
for(j=980;j<1000;j++)
 {
   for(i=0;i<j;i++) Buzzer_Off(); 
   for(i=0;i<j;i++) Buzzer_On();
 }
}
void Boop   (void)   
{
unsigned int i,j;
Led_Power_Reset();
for(j=200;j<300;j++)
 {
   for(i=0;i<j;i++) Buzzer_Off(); 
   for(i=0;i<j;i++) Buzzer_On();
  }
}
void Led_All_Off ( void ) { Led_Power_Reset() ;Led_Link_Reset();Led_Run_Reset();Led_One_Wire_Reset();}
void Led1        ( void ) { Led_Power_Set()   ;}
void Led2        ( void ) { Led_Link_Set()    ;}
void Led3        ( void ) { Led_Run_Set()     ;}
void Led4        ( void ) { Led_One_Wire_Set();}
//----------
struct Led_Effect_Struct Led_Effects[]=
{
 {0x8000 ,0x8888 ,Beep             ,Rien}               ,
 {0xAAAA ,0x8FFF ,Led_Power_Set    ,Led_Power_Reset}    ,
 {0xF0F0 ,0x01FF ,Led_Link_Set     ,Led_Link_Reset}     ,
 {0xAAAA ,0x003F ,Led_Run_Set      ,Led_Run_Reset}      ,
 {0xAAAA ,0x0007 ,Led_One_Wire_Set ,Led_One_Wire_Reset} ,
};
//struct Led_Effect_Struct Led_Effects[sizeof(Led_Effects_Template)/sizeof(struct Led_Effect_Struct)];
//----------
void Init_Leds_Effects    ( void                                  ) {
//   memcpy((char*)Led_Effects,(char*)Led_Effects_Template,sizeof(Led_Effects_Template));
}
void Set_Led_Effect       ( unsigned char Led,unsigned int Effect ) { Led_Effects[Led].Effect=Led_Effects[Led].Temp_Effect=Effect                                               ;}
void Set_Temp_Led_Effect  ( unsigned char Led,unsigned int Effect ) { Led_Effects[Led].Temp_Effect=Effect                                                                       ;}
void Set_Fixed_Led_Effect ( unsigned char Led,unsigned int Effect ) { Led_Effects[Led].Effect=Effect                                                                            ;}
void Led_Effects_Func     ( void                                  )
{
 unsigned char Actual_Led;
 for(Actual_Led=0;Actual_Led<sizeof(Led_Effects)/sizeof(struct Led_Effect_Struct);Actual_Led++)
  {
   (Led_Effects[Actual_Led].Temp_Effect&0x0001)?Led_Effects[Actual_Led].On_Function():Led_Effects[Actual_Led].Off_Function(); //se usa el metodo del shifteop para trasladar la indo de la variable al estado del led...
   if(!(Led_Effects[Actual_Led].Temp_Effect>>=1)) Led_Effects[Actual_Led].Temp_Effect=Led_Effects[Actual_Led].Effect;
  }
}
void Stairs_Effects(void)
{
 Set_Temp_Led_Effect(Led_Power    ,0x0007);
 Set_Temp_Led_Effect(Led_Link     ,0x000F);
 Set_Temp_Led_Effect(Led_Run      ,0x001F);
 Set_Temp_Led_Effect(Led_One_Wire ,0x003F);
}
//------------------------------------------------------------------
void Init_Leds_Session(void)
{
 Leds_Session_Sm=Flashing;
 Init_Leds_Effects ( )   ;
 Init_Leds         ( )   ;
 Init_Buttons      ( )   ;
 Button_Filter=0         ;
 Buzzer_Init       ( )   ;
}
const State**  Leds_Session      (void)         {return &Leds_Session_Sm;}
void     Leds_Session_Rti  (void)
{
   Led_Effects_Func();
// unsigned int Event=ANY_Event;
//   if(Button1_Read())
//    {
//            if(Button_Filter)
//           {
//             Event=(Button_Filter<3)?Short_Released_Event:Long_Released_Event;
//             Button_Filter=0;
//             Update_Or_New_None_Periodic_Schedule(ABORT_TOUT,Abort_Event,Leds_Session());
//            }
//         }
// else    if(Button_Filter<10)    Button_Filter++;
// Atomic_Send_Event(Event,Leds_Session());
}
//-------------------------------------------------------------------
//--------------------- MAQUINA DE ESTADOS PARA EVERYTHINGS --------------------
const State Flashing[] =
{
   { Long_Released_Event ,Stairs_Effects   ,Flashing },
   { Abort_Event         ,Rien             ,Flashing },
   { ANY_Event           ,Led_Effects_Func ,Flashing },
};
//-------------------------------------------------------------------------------
