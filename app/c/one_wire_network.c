#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/emac.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "state_machine.h"
#include "events.h"
#include "one_wire_link.h"
#include "one_wire_network.h"
#include "one_wire_transport.h"
#include "leds.h"
#include "usr_flash.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


const State
   Idle                   [ ],
   Detecting              [ ],
   Writing_Reading        [ ],
   Detecting4Searching_Rom[ ],
   Searching_Rom          [ ],
   Colisioning            [ ],
   Parsing_Last_Code_Bit  [ ];

const State*               One_Wire_Network_Sm;
struct Rom_Codes_Struct    Rom_Codes[MAX_ROM_CODES];
static uint8_t             Actual_Bit;
static uint8_t             Actual_Code;
static uint8_t             Actual_Marker;
static uint8_t             Last_Marker;
static uint8_t             Fail_Codes;
static uint8_t             Rx_Buffer[ONE_WIRE_RX_BUFFER];
static uint8_t             Bytes2Read;
static uint8_t             *Point2Write;
static uint8_t             *Point2Read;
//------------------------------------------------------------------
void           Clear_Bit_On_String ( unsigned char* Data, unsigned char Bit ) { Data[Bit/8]&=~(0x80>>Bit%8)          ;}
void           Set_Bit_On_String   ( unsigned char* Data, unsigned char Bit ) { Data[Bit/8]|=(0x80>>Bit%8)           ;}
unsigned char  Read_Bit4String     ( unsigned char* Data,unsigned char Bit  ) { return (Data[Bit/8]&(0x80>>Bit%8))!=0;}
unsigned char  Update_crc          ( unsigned char New, unsigned char Last  )
{
//8-bit CRC value  using polynomial  X^8 + X^5 + X^4 + 1 
 #define POLYVAL 0x8C
 unsigned char i;
 for(i=0;i<8;i++) {
  Last=((Last^New)&0x01)?(Last>>1)^POLYVAL:Last>>1;
  New>>=1;
 }
 return Last;
}

unsigned char Calculate_One_Wire_Crc(unsigned char* Data,unsigned char Length)
{
 unsigned char Crc=0;
 while(Length--) Crc=Update_crc(Data[Length],Crc);
 return Crc==0;
}
//------------------------------------------------------------------
const State**  One_Wire_Network ( void ) { return &One_Wire_Network_Sm;}
//------------------------------------------------------------------
uint8_t        One_Wire_Rx_As_Char  ( uint8_t Pos ) { return Rx_Buffer[Pos];}
uint8_t*       One_Wire_Rx_As_PChar ( uint8_t Pos ) { return Rx_Buffer+Pos ;}
unsigned int   One_Wire_Rx_As_Int   ( uint8_t Pos ) { return *(unsigned int*) (Rx_Buffer+Pos);}
int16_t        One_Wire_Rx_As_SInt  (unsigned char Pos)  {
      return Rx_Buffer[Pos+0]*256+Rx_Buffer[Pos+1];
}
//---------------------------------
void Execute_Cmd(uint8_t Length,uint8_t* Cmd)
{
 Bytes2Read  = Length;
 Point2Write = Cmd;
 Point2Read  = One_Wire_Rx_As_PChar(0);
 Send_Event(One_Wire_Cmd_Event,One_Wire_Network());
}
//----------------------------
void Write_Read_Next_Byte  (void)
{
 GPIOPinSet ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
 Bytes2Read--;
 Point2Read[Bytes2Read]=Write_Read_Byte(Point2Write[Bytes2Read]);
 Send_Event(Bytes2Read?One_Wire_Read_Next_Byte_Event:One_Wire_End_Of_Read_Event,One_Wire_Network());
 GPIOPinReset ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
}
//------------------------------------------------
void           Read_Presence ( void ) {
   GPIOPinSet ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
   Send_Event(Presence()?One_Wire_Not_Detected_Event:One_Wire_Detected_Event,One_Wire_Network());
}
void           Search_Codes         ( void              ) { Send_Event(Search_Codes_Event,One_Wire_Network());}
void           Search_Rom           ( void              ) { Write_Read_Byte(SEARCH_ROM)                      ;}
void           Read_Rom             ( void              ) { Execute_Cmd(9,(uint8_t*)READ_ROM_STRING)         ;}
void           Match_Rom            ( uint8_t* Rom_Code ) { Execute_Cmd(9,Rom_Code)                          ;}
void           Skip_Rom             ( void              ) { Execute_Cmd(1,(uint8_t*)SKIP_ROM_STRING)         ;}
void           Broadcast_T          ( void              ) { Execute_Cmd(2,(uint8_t*)"\x44\xCC")              ;}
uint8_t        One_Wire_Crc         ( uint8_t Node      ) { return Rom_Codes[Node].Crc                       ;}
int16_t        One_Wire_T           ( uint8_t Node      ) { return Rom_Codes[Node].T                         ;}
uint8_t*       One_Wire_Code        ( uint8_t Node      ) { return Rom_Codes[Node].Code                      ;}
uint8_t        One_Wire_Family_Code ( uint8_t Node      ) { return Rom_Codes[Node].Code[7]                   ;}
//------------------------- DS18B20 ----------------------------------
void Read_DS18B20_Scratchpad  (uint8_t Node)
{
 static const char Scratchpad_Template[]="\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xBE\x00\x00\x00\x00\x00\x00\x00\x00\x55";
 memcpy ( (char*)One_Wire_Rx_As_PChar(0  ),Scratchpad_Template,sizeof(Scratchpad_Template)-1);
 memcpy ( (char*)One_Wire_Rx_As_PChar(10 ),(char*)&Rom_Codes[Node].Code[0],sizeof(Rom_Codes[0].Code));
 Execute_Cmd ( sizeof(Scratchpad_Template                   )-1,One_Wire_Rx_As_PChar(0))    ;
}
void Calculate_DS18B20_12Bit_T   (uint8_t Node)
{
 if(Calculate_One_Wire_Crc(One_Wire_Rx_As_PChar(0),9)) {             // esta linea estaba mal y no andaba para numeros negativos.. porque lo estaba casteando a unsigned...
     Rom_Codes[Node].T=(((  int32_t)One_Wire_Rx_As_SInt(7))*100)/16; // ver documento DS18B20 pag.4, se multiplica por 100 para tener 2 decimales y se divide por 16 porque la parte decimal del sensor son 4 bits...
     Rom_Codes[Node].Crc=4;
  }
 else {
   if(Rom_Codes[Node].Crc) Rom_Codes[Node].Crc--;
 }
}
void Print_Temp_Nodes(struct tcp_pcb* tpcb)
{
   static uint32_t Id=0; //para mostrar que avanza la lectura..
   uint8_t i;
   UART_ETHprintf(tpcb,"\r\n");
   for(i=0;i<One_Wire_On_Line_Nodes();i++)
      UART_ETHprintf(tpcb,"#%d Sensor: %H Temp: %f Crc: %s \r\n",
            Id,
            Rom_Codes[i].Code,sizeof(Rom_Codes[0].Code),
            (float)Rom_Codes[i].T/100,
            Rom_Codes[i].Crc?"ok ":"err");
   Id++;
}
//-----------------SEARCH ROM FUNC-----------------------------------
void Reset_Actual_Bit    ( void ) { Actual_Bit    = 64;}
void Reset_Actual_Marker ( void ) { Actual_Marker = 64;}
void Reset_Last_Marker   ( void ) { Last_Marker   = 64;}
void Reset_Actual_Code   ( void ) { Actual_Code   = 0 ;}
void Reset_Fail_Codes    ( void ) { Fail_Codes    = 0 ;}
void Inc_Actual_Code     ( void ) { Actual_Code++            ;}
void Inc_Fail_Codes      ( void ) { Fail_Codes++             ;}
void Init_Markers        ( void ) { Reset_Actual_Bit()       ;Reset_Actual_Marker();Reset_Last_Marker();Reset_Actual_Code()    ;Reset_Fail_Codes();}
void Actual_Marker2Last  ( void ) { Last_Marker=Actual_Marker;}
void Actual_Bit2Marker   ( void ) { Actual_Marker=Actual_Bit ;}
void Mark_All_Crc_Fail   ( void ) {
   uint8_t i=0;
   for ( i=0;i<MAX_ROM_CODES;i++ ) {
      Rom_Codes[i].Crc=0;
      Rom_Codes[i].T=0x7FFF;
   }
}
//-------------------------------------------------
void Bit_Colision     ( void ) { Send_Event(Actual_Bit<Last_Marker?Smaller_Discrepance_Event:(Actual_Bit==Last_Marker)?Equal_Discrepance_Event:Bigger_Discrepance_Event,One_Wire_Network());UART_ETHprintf(DEBUG_MSG,"Colision\r\n")                     ;}
void Select_Bit_One   ( void ) { Write_Bit_One_And_Read()                                                                                                                                         ;Set_Bit_On_String(Rom_Codes[Actual_Code].Code,Actual_Bit)  ;}
void Select_Bit_Zero  ( void ) { Write_Bit_Zero()                                                                                                                                                 ;Clear_Bit_On_String(Rom_Codes[Actual_Code].Code,Actual_Bit);}
void Search_Next_Bit  ( void ) { Send_Event(Actual_Bit--?Read2Bits():Actual_Code_End_Event,One_Wire_Network())                                                                             ;}
void Search_Next_Code ( void )
{
 UART_ETHprintf(DEBUG_MSG,"Next Code\r\n");
 Actual_Marker2Last  ( );
 Reset_Actual_Bit    ( );
 Reset_Actual_Marker ( );
 Calculate_One_Wire_Crc ( Rom_Codes[Actual_Code].Code,sizeof(Rom_Codes[0].Code                     ))?Inc_Actual_Code():Inc_Fail_Codes()                                         ;//de paso caniazo revisa el crc. Si esta ok, incrementa el indexador de la lizta, sino incrementa la cantidad de codigos fallidos para que no quede aca para siempres y PISA el que salio mal!!! De manera que SOLo se enlistan los buenos...
 ( Last_Marker<64 && Actual_Code<MAX_ROM_CODES && Fail_Codes<MAX_FAIL_CODES )?Read_Presence():Send_Event(Search_Codes_End_Event,One_Wire_Network());//si el marcador es menor que 64 quiere decir que hay que resolver una colision, y eso implica que hay mas dispositivos que detetar. Por otro lado no podemos detectar mas nodos que el lugar disponible....
}
void           Send_Last_Code_Bit     ( void ) { Send_Event(Read_Bit4String(Rom_Codes[Actual_Code-1].Code,Actual_Bit),One_Wire_Network())          ;}
//-------------------------------------------------
uint8_t        One_Wire_On_Line_Nodes ( void ) { return Actual_Code                                                                                ;}
void           Check_On_Lines_Nodes   ( void ) { Send_Event(One_Wire_On_Line_Nodes()?Anybody_On_Bus_Event:Nobody_On_Bus_Event,One_Wire_Transport());}
void           Ans_Anybody2App        ( void ) { Send_Event(Anybody_On_Bus_Event,One_Wire_Transport())                                             ;}
void           Ans_Nobody2App         ( void ) { Send_Event(Nobody_On_Bus_Event,One_Wire_Transport())                                              ;}
void           Ans_End_Of_Msg2App     ( void ) { Send_Event(End_Of_One_Wire_Msg_Event,One_Wire_Transport())                                        ;}
//-------------------------------------------------
void           Print_Detected         ( void ) { UART_ETHprintf(DEBUG_MSG,"Detected\r\n")                                                ;}
void           Print_Not_Detected     ( void ) {
   GPIOPinReset ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
   UART_ETHprintf(DEBUG_MSG,"Not Detected\r\n")                                           ;
}
void           Print_Bit_Error        ( void ) { UART_ETHprintf(DEBUG_MSG,"Bit Error\r\n")                                              ;}
//-------------------------------------------------
void     Init_One_Wire_Network   (void)
{
   Init_One_Wire_Link ( );
   One_Wire_Network_Sm=Idle;
   Mark_All_Crc_Fail  ( );
   Reset_Actual_Code  ( );
}
//-------------------------------------------------
void Print_Detected_And_Write_Read_Next_Byte                     ( void ) { Print_Detected()                       ;Write_Read_Next_Byte();}
void Read_Presence_And_Init_Markers                              ( void ) { Read_Presence()                        ;Init_Markers()        ;}
void Print_Detected_And_Search_Rom_And_Search_Next_Bit           ( void ) { Print_Detected()                       ;Search_Rom()          ;Search_Next_Bit()  ;}
void Select_Bit_One_And_Search_Next_Bit                          ( void ) {
   GPIOPinSet      ( LED_SERIAL_PORT, LED_SERIAL_PIN );
   Select_Bit_One  (                                 );
   Search_Next_Bit (                                 );
}
void Select_Bit_Zero_And_Search_Next_Bit                         ( void ) {
   GPIOPinReset    ( LED_SERIAL_PORT, LED_SERIAL_PIN );
   Select_Bit_Zero (                                 );
   Search_Next_Bit (                                 );
}
void Select_Bit_Zero_And_Actual_Bit2Marker_And_Search_Next_Bit   ( void ) { Select_Bit_Zero()                      ;Actual_Bit2Marker()   ;Search_Next_Bit()  ;}
void Print_Not_Detected_And_Ans_Nobody2App                       ( void ) { Print_Not_Detected()                   ;Ans_Nobody2App()      ;}
void Print_Not_Detected_And_Ans_Nobody2App_And_Reset_Actual_Code ( void ) { Print_Not_Detected_And_Ans_Nobody2App();Reset_Actual_Code()   ;}
void Print_Bit_Error_And_Ans_Nobody2App_And_Mark_All_Crc_Fail    ( void ) { Print_Bit_Error()                      ;Ans_Nobody2App()      ;Mark_All_Crc_Fail();}
//-------------------------------------------------
const State Idle                   [ ] =
{
   { One_Wire_Cmd_Event            ,Read_Presence                                               ,Detecting               },
   { Search_Codes_Event            ,Read_Presence_And_Init_Markers                              ,Detecting4Searching_Rom },
   { ANY_Event                     ,Rien                                                        ,Idle                    },
};
const State Detecting              [ ] =
{
   { One_Wire_Detected_Event       ,Print_Detected_And_Write_Read_Next_Byte                     ,Writing_Reading         },
   { One_Wire_Not_Detected_Event   ,Print_Not_Detected_And_Ans_Nobody2App_And_Reset_Actual_Code ,Idle                    },
   { Search_Codes_Event            ,Read_Presence_And_Init_Markers                              ,Detecting4Searching_Rom },
   { ANY_Event                     ,Rien                                                        ,Detecting               },
};
const State Writing_Reading        [ ] =
{
   { One_Wire_Read_Next_Byte_Event ,Write_Read_Next_Byte                                        ,Writing_Reading         },
   { One_Wire_End_Of_Read_Event    ,Ans_End_Of_Msg2App                                          ,Idle                    },

   { Search_Codes_Event            ,Read_Presence_And_Init_Markers                              ,Detecting4Searching_Rom },
   { ANY_Event                     ,Rien                                                        ,Writing_Reading         },
};
//-----------------------------
const State Detecting4Searching_Rom[ ] =
{
   { One_Wire_Detected_Event       ,Print_Detected_And_Search_Rom_And_Search_Next_Bit           ,Searching_Rom           },
   { One_Wire_Not_Detected_Event   ,Print_Not_Detected_And_Ans_Nobody2App                       ,Idle                    },
   { Search_Codes_End_Event        ,Check_On_Lines_Nodes                                        ,Idle                    },
   { ANY_Event                     ,Rien                                                        ,Detecting4Searching_Rom },
};
const State Searching_Rom          [ ] =
{
   { 0x0000                        ,Bit_Colision                                                ,Colisioning             },
   { 0x0001                        ,Select_Bit_One_And_Search_Next_Bit                          ,Searching_Rom           },
   { 0x0002                        ,Select_Bit_Zero_And_Search_Next_Bit                         ,Searching_Rom           },
   { 0x0003                        ,Print_Bit_Error_And_Ans_Nobody2App_And_Mark_All_Crc_Fail    ,Idle                    },
   { Actual_Code_End_Event         ,Search_Next_Code                                            ,Detecting4Searching_Rom },
   { ANY_Event                     ,Rien                                                        ,Searching_Rom           },
};
//-----------------------------
const State Colisioning            [ ] =
{
   { Bigger_Discrepance_Event      ,Send_Last_Code_Bit                                          ,Parsing_Last_Code_Bit   },
   { Equal_Discrepance_Event       ,Select_Bit_One_And_Search_Next_Bit                          ,Searching_Rom           },
   { Smaller_Discrepance_Event     ,Select_Bit_Zero_And_Actual_Bit2Marker_And_Search_Next_Bit   ,Searching_Rom           },
   { ANY_Event                     ,Rien                                                        ,Colisioning             },
};
const State Parsing_Last_Code_Bit  [ ] =
{
   { 0x0001                        ,Select_Bit_One_And_Search_Next_Bit                          ,Searching_Rom           },
   { 0x0000                        ,Select_Bit_Zero_And_Actual_Bit2Marker_And_Search_Next_Bit   ,Searching_Rom           },
   { ANY_Event                     ,Rien                                                        ,Parsing_Last_Code_Bit   },
};
//---------------------------------------------

