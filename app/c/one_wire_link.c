#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/emac.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "utils/uartstdio.h"
#include "one_wire_link.h"
#include "leds.h"
#include "state_machine.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


void     Bus_Hi   ( void ) {
 MAP_GPIOPinTypeGPIOOutput ( ONE_WIRE_PORT,ONE_WIRE_PIN );
 MAP_GPIOPinWrite ( ONE_WIRE_PORT,ONE_WIRE_PIN , ONE_WIRE_PIN);
}
void     Bus_Lo   ( void ) {
 MAP_GPIOPinTypeGPIOOutput ( ONE_WIRE_PORT,ONE_WIRE_PIN );
 MAP_GPIOPinWrite ( ONE_WIRE_PORT,ONE_WIRE_PIN , 0);
}
void     Bus_Hz   ( void ) {
 MAP_GPIOPinTypeGPIOInput ( ONE_WIRE_PORT,ONE_WIRE_PIN );
}
uint8_t  Bus_Read ( void ) {
 return (MAP_GPIOPinRead ( ONE_WIRE_PORT,ONE_WIRE_PIN ))!=0;
}
void     Pullup   ( void ) {
   Bus_Hi ( );
   Bus_Hz ( );
}

void Init_One_Wire_Link(void) {
   MAP_SysCtlPeripheralEnable (ONE_WIRE_PERIPH);
   Bus_Hz        ( );
}
//------------------------------------------------------------------
void One_Wire_Power_On_Reset(void) {
   Bus_Lo();
}

bool Presence(void) {
   uint8_t Ans;
   Bus_Hz     (   );
   Delay_Useg ( 1 ); // aguanta que se estabilice todo.
   if( Bus_Read( )== 0) {
      Bus_Lo ( );          // hasta dentro de 1 segundo que vuelve por aca, queda el bus en cero, que equivale a apagar los sensores...
      return 1  ;          // liberen el barco!! porque asi no laburo... todo '0' es un CRC valido, asi que si esta en corto, cuenta como nodo valido!!!!
   }
   Bus_Lo       (     ); // baja
   Delay_Useg   ( 510 ); // 480<T<960 espera el tiempo de reset
   Pullup       (     ); // sube el bus con fuerza, espera y luego lo libera...
CPUcpsid     (     );
   Delay_Useg   ( 70  ); // minimo 75, maximo 300
//   GPIOPinSet   ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
   Ans=Bus_Read (     ); // lobo esta?
//   GPIOPinReset ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
CPUcpsie     (     );
   Delay_Useg   ( 430 ); // 480-70 minimo, completa el tiempo de slot...
   Pullup       (     ); // despues de un presense, solo lo deja en pullup, porque si no hay nadie ara que tenerlo en alta que podria haber un corto...
   return Ans;
}
//------------------------------------------------------------------
uint8_t Write_Bit_Zero(void)
{
CPUcpsid   (    );
   Bus_Lo     (    ); // baja
   Delay_Useg ( 70 ); // 60<T<120
   Pullup     (    ); // sube el bus con fuerza, espera y luego lo libera...
CPUcpsie   (    );
   Delay_Useg ( 5  ); // 1u<T<inf.
//   Pullup       (    ); // sube el bus con fuerza, espera y luego lo libera...
   Bus_Hi     (    ); // arriba
   return 0;
}
//------------------------------------------------------------------
uint8_t Write_Bit_One_And_Read(void)
{
   uint8_t Ans;
CPUcpsid   (    );
   Bus_Lo       (    ); // baja
   Delay_Useg   ( 10 ); // 1<T<15 espera 6useg (offset 3+3)
   Pullup       (    ); // sube el bus con fuerza, espera y luego lo libera...
   Delay_Useg   ( 5  ); // 1u<T<15 del total, es 2+3+4+3=12 + lo que tarda el Strong_Pullup   espera 8useg (offset 3+5)
//  GPIOPinSet   ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
   Ans=Bus_Read (    ); // samplea a los 14useg (Texas obliga a 15 como maximo)
//   GPIOPinReset ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
   Delay_Useg   ( 55 ); // 45 como minimo.. despues de los 15 de lectura...+ 1us de recupero
CPUcpsie     (    );
//   Pullup       (    ); // sube el bus con fuerza, espera y luego lo libera...
   Bus_Hi       (    ); // arriba
   return Ans;
}
//------------------------------------------------------------------
uint8_t Write_Read_Bit ( uint8_t Bit ) {
   return Bit?Write_Bit_One_And_Read():Write_Bit_Zero();
}
//------------------------------------------------------------------
uint8_t Read2Bits ( void ) {
   uint8_t Ans=  Write_Read_Bit(1 );
   Ans|= Write_Read_Bit(1)<<1; // hace 2 lecturas, la primera en el bit 0 y la segunda la corre una posicion.. codigos de salida 00-01-10-11
   return Ans;
}
//------------------------------------------------------------------
uint8_t Write_Read_Byte (uint8_t Data)
{
 uint8_t i,Ans=0;
 for(i=0x01;i;i<<=1)
   if(Write_Read_Bit(Data&i)) Ans|=i;
 return Ans;
}
//------------------------------------------------------------------
