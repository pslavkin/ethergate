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
#include "state_machine.h"
#include "events.h"
#include "one_wire_link.h"
#include "one_wire_network.h"
#include "one_wire_transport.h"
#include "leds.h"
#include "schedule.h"
#include "usr_flash.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

const State
   Searching_Rom_Codes       [ ],
   Broadcasting_T            [ ],
   Unicasting                [ ],
   Parsing_Family_Code       [ ],
   Reading_DS18B20_Scratchpad[ ],
   Checking_Crcs             [ ];

const State*   One_Wire_Transport_Sm;
unsigned char  Actual_Node,Actual_Sending_Node;
//------------------------------------------------------------------
const State** One_Wire_Transport    ( void ) { return &One_Wire_Transport_Sm;}
void  Init_One_Wire_Transport ( void )
{
 Init_One_Wire_Network ( );
 One_Wire_Transport_Sm = Searching_Rom_Codes;
 Refresh_One_Wire_Reload_TOut(Usr_Flash_Params.Reload_T_TOut);
 Reload_One_Wire_Codes();
}
void      Refresh_One_Wire_Reload_TOut(uint32_t TOut)
{
 if(TOut) Update_Or_New_Func_Schedule(600*TOut,Reload_One_Wire_Codes);
}
uint32_t One_Wire_Next_Reload_Time(void)
{
   return Read_Func_Schedule_TOut(Reload_One_Wire_Codes);
}
//------------------------------------------------------------------
void Reset_Actual_Node              ( void ) { Actual_Node=0                                                                                                                    ;}
void Inc_Actual_Node                ( void ) { Actual_Node++                                                                                                                    ;}
void Parse_Next_Family_Code         ( void ) { Send_Event(Actual_Node<One_Wire_On_Line_Nodes()?One_Wire_Family_Code(Actual_Node):End_Of_Nodes_Event,One_Wire_Transport());}
void Read_Actual_DS18B20_Scratchpad ( void ) { Read_DS18B20_Scratchpad(Actual_Node)                                                                                             ;}
void Reload_One_Wire_Codes          ( void )
{
 Mark_All_Crc_Fail();
 Send_Event(Reload_Codes_Event,One_Wire_Transport());
}
//------------------------------------------------------------------
void Print_Nobody_On_Bus ( void ) { UART_ETHprintf(DEBUG_MSG,"Nobody on Bus\n");}
void Print_All_Measured  ( void ) { UART_ETHprintf(DEBUG_MSG,"All Measured\n") ;}
//-------------------------------------------------
void Check_Crcs(void)
{
 unsigned char i=One_Wire_On_Line_Nodes();
 while(i && One_Wire_Crc(i-1)>1) i--;                 //con que uno este fallado, pagan todos...
 Send_Event(i?Crc_Fail_Event:Crc_Ok_Event,One_Wire_Transport());    //sino llego a recorrer todos... falla.
}
//-------------------------------------------------
void Reset_Actual_Node_And_Parse_Next_Family_Code                             ( void ) { Reset_Actual_Node()                   ;Parse_Next_Family_Code();}
void Print_All_Measured_And_Broadcast_T                                       ( void ) { Print_All_Measured()                  ;Broadcast_T()           ;}
void Print_Nobody_On_Bus_And_Wait1Sec                                         ( void ) { Print_Nobody_On_Bus()                 ;None_Periodic_1Sec()    ;}
void Calculate_DS18B20_12Bit_T_And_Inc_Actual_Node_And_Parse_Next_Family_Code ( void ) { Calculate_DS18B20_12Bit_T(Actual_Node);Inc_Actual_Node()       ;Parse_Next_Family_Code();}
void Free_Wait1Sec_And_Search_Codes                                           ( void ) { Free_Schedule_1Sec()                  ;Search_Codes()          ;}
void One_Wire_Power_On_Reset_And_Wait1Sec                                     ( void ) { One_Wire_Power_On_Reset()             ;None_Periodic_1Sec()    ;}
void Broadcast_T_And_Set_Led(void)
{
   Broadcast_T();
   GPIOPinSet ( LED_SERIAL_PORT, LED_SERIAL_PIN ) ;
}
//-------------------------------------------------
const State Searching_Rom_Codes       [ ]=
{
   { Nobody_On_Bus_Event  ,Print_Nobody_On_Bus_And_Wait1Sec ,Searching_Rom_Codes },
   { Sec1_Event           ,Search_Codes                     ,Searching_Rom_Codes },
   { Anybody_On_Bus_Event ,Broadcast_T                      ,Broadcasting_T      },
   { Reload_Codes_Event   ,Free_Wait1Sec_And_Search_Codes   ,Searching_Rom_Codes },
   { ANY_Event            ,Rien                             ,Searching_Rom_Codes },
};
//----------------------------------------------
const State Broadcasting_T            [ ]=
{
   { End_Of_One_Wire_Msg_Event ,None_Periodic_1Sec                                                        ,Unicasting                 },
   { Nobody_On_Bus_Event       ,Print_Nobody_On_Bus_And_Wait1Sec                                         ,Searching_Rom_Codes        },
   { Reload_Codes_Event        ,Search_Codes                                                             ,Searching_Rom_Codes        },
   { ANY_Event                 ,Rien                                                                     ,Broadcasting_T             },
};
//----------------------------------------------
const State Unicasting                [ ]=
{
   { Sec1_Event         ,Reset_Actual_Node_And_Parse_Next_Family_Code ,Parsing_Family_Code },
   { Reload_Codes_Event ,Free_Wait1Sec_And_Search_Codes               ,Searching_Rom_Codes },
   { ANY_Event          ,Rien                                         ,Unicasting          },
};
const State Parsing_Family_Code       [ ]=
{
   { DS18B20                   ,Read_Actual_DS18B20_Scratchpad                                           ,Reading_DS18B20_Scratchpad },
   { End_Of_Nodes_Event        ,Check_Crcs                                                               ,Checking_Crcs              },
   { Reload_Codes_Event        ,Search_Codes                                                             ,Searching_Rom_Codes        },
   { ANY_Event                 ,Parse_Next_Family_Code                                                   ,Parsing_Family_Code        },
};
//----------------------------------------------
const State Reading_DS18B20_Scratchpad[ ]=
{
   { End_Of_One_Wire_Msg_Event ,Calculate_DS18B20_12Bit_T_And_Inc_Actual_Node_And_Parse_Next_Family_Code ,Parsing_Family_Code        },
   { Nobody_On_Bus_Event       ,Print_Nobody_On_Bus_And_Wait1Sec                                         ,Searching_Rom_Codes        },
   { Reload_Codes_Event        ,Search_Codes                                                             ,Searching_Rom_Codes        },
   { ANY_Event                 ,Rien                                                                     ,Reading_DS18B20_Scratchpad },
};
//----------------------------------------------
const State Checking_Crcs             [ ]=
{
   { Crc_Ok_Event              ,Broadcast_T_And_Set_Led                                                              ,Broadcasting_T             },//si esta todo ok                                                           ,sigue..
   { Crc_Fail_Event            ,One_Wire_Power_On_Reset_And_Wait1Sec                                     ,Searching_Rom_Codes        },//si fallo OJO que esto entra si el CRC llego a '1' desde un valor mayor... ,APAGA el bus... durante 1 segundo y busca nodos nuevamente...
   { Reload_Codes_Event        ,Search_Codes                                                             ,Searching_Rom_Codes        },//
   { ANY_Event                 ,Rien                                                                     ,Checking_Crcs              },
};

