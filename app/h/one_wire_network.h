#ifndef ONE_WIRE_NETWORK
#define ONE_WIRE_NETWORK

#define ONE_WIRE_RX_BUFFER 35 // es tambien el tamaño maximo de algun comando...generalmente no pasa de 20, que es un match ROM + read Page
#define MAX_ROM_CODES       2  // cantidad de nodos permitidos en la red..
#define MAX_FAIL_CODES     20 // cantidad de veces que falla intentando leer el numero de codigo en la busqueda de codigos... usualmente falla cuando el cableado es malo y lee cualquier cosa. entonces con esto se puede reportar el problema...

enum One_Wire_Network_Events {
    End_Of_One_Wire_Msg_Event     = 0xD000,
    One_Wire_Detected_Event       = 0xD001,
    One_Wire_Not_Detected_Event   = 0xD002,
    One_Wire_Cmd_Event            = 0xD003,
    One_Wire_End_Of_Read_Event    = 0xD004,
    One_Wire_Read_Next_Byte_Event = 0xD005,
    Search_Codes_Event            = 0xD006,
    Actual_Code_End_Event         = 0xD007,
    Search_Codes_End_Event        = 0xD008,
    Smaller_Discrepance_Event     = 0xD009,
    Equal_Discrepance_Event       = 0xD00A,
    Bigger_Discrepance_Event      = 0xD00B,
    Nobody_On_Bus_Event           = 0xD00E,
    Anybody_On_Bus_Event          = 0xD00F
   };
//------------------------------------------------------
struct Rom_Codes_Struct
{
 uint8_t Code[8]     ; // guarda el codigo del nodo...
 int16_t T           ; // guarda la temperatura actual del nodo,
 uint8_t Crc         ; // Crc
};
//---------------------------------------------------
void           Init_One_Wire_Network       ( void                          );
const State**  One_Wire_Network            ( void                          );
// -----------
uint8_t        One_Wire_Rx_As_Char         ( uint8_t Pos                   );
uint8_t*       One_Wire_Rx_As_PChar        ( uint8_t Pos                   );
// ---------------Comandos ----------------------------------
void           Execute_Cmd                 ( uint8_t Length,uint8_t* Cmd   );
void           Read_Rom                    ( void                          );
void           Match_Rom                   ( uint8_t* Rom_Code             );
void           Skip_Rom                    ( void                          );
void           Broadcast_T                 ( void                          );
// -------------- ALL -------------------------------
uint8_t        One_Wire_On_Line_Nodes  ( void         );
void           Print_Rom_Codes         ( void         );
void           Search_Codes            ( void         );
unsigned int   Convert_Nodes_Bin2Ascci ( uint8_t* Buf );
uint8_t        One_Wire_Family_Code    ( uint8_t Node );
uint8_t        One_Wire_Crc            ( uint8_t Node );
int16_t        One_Wire_T              ( uint8_t Node );
uint8_t*       One_Wire_Code           ( uint8_t Node );
void           Reload_One_Wire_Codes   ( void         );
void           Mark_All_Crc_Fail       ( void         );
// ---------------DS18B20------------------------------------
void           Read_DS18B20_Scratchpad     ( uint8_t Node                  );
void           Calculate_DS18B20_12Bit_T   ( uint8_t Node                  );
uint8_t        DS18B20_Convert_Bin2Ascci_T ( uint8_t* Destiny,uint8_t Code );
void Print_Temp_Nodes(struct tcp_pcb* tpcb);
// ------------------------------------------------------

#endif
