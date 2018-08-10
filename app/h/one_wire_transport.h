#ifndef ONE_WIRE_TRANSPORT
#define ONE_WIRE_TRANSPORT

enum One_Wire_Transport_Events {
    End_Of_Nodes_Event = 0x0000,
    Reload_Codes_Event = 0x0001,
    Crc_Ok_Event       = 0x0002,
    Crc_Fail_Event     = 0x0003
   };
enum One_Wire_Maxim_Ic_Codes {
    DS18S20 = 0x10,
    DS18B20 = 0x28,
    DS2438  = 0x26
   };
//------------------------------------------------------
void        Init_One_Wire_Transport         ( void );
const State**     One_Wire_Transport              ( void );
//------------------------------------------------------
void        Reload_One_Wire_Codes           ( void          );
uint32_t    One_Wire_Next_Reload_Time       ( void          );
void        Refresh_One_Wire_Reload_TOut    ( uint32_t TOut );
void        Send_Temp2Serial                ( void          );
void        Send_One_Wire_Info2Tcp          ( void          );
void        Begin_Send_One_Wire_Info2Serial ( void          );
void        Send_Next_One_Wire_Info2Serial  ( void          );
//------------------------------------------------------

#endif
