#ifndef RS232
#define RS232

enum Rs232_Events {
    Conn_Regi_Event       = 0,
    Conn_Free_Event       = 1,
    Data_Arrived_Event    = 2,
    Console_Enable_Event  = 3,
    Console_Disable_Event = 4,
    Term_Found_Event      = 5,
    Max_Length_Event      = 6,
    TOut_Event            = 7,
    Enter_Found_Event     = 8,
};

struct Line_Process_Struct
{
   uint8_t  Buff[TCP_MSS];
   uint32_t Id;
   uint32_t Index;
   uint32_t Tout;
};
void              Init_Rs232 ( void );
const State**     Rs232      ( void );
void              Init_Uart  ( void );

void Rs232_Task             ( void* nil                                                    );
void Line_Process           ( void                                                         );
void Parser_Process         ( void                                                         );
void Send_Data2Tcp          ( void* n                                                      );
void Callback_Send_Data2Tcp ( void                                                         );
void Bridge_Data_Process    ( void                                                         );
void Send_Data2Parser       ( void                                                         );
void Console_Data_Process   ( void                                                         );
void Is_Console_Enabled     ( void                                                         );
void Bridge2Console         ( void                                                         );
void Console2Bridge         ( void                                                         );
bool manageEnter            ( uint8_t Char, bool isNext, uint8_t nextChar, uint16_t *index );
void manageLastInput        ( struct Parser_Queue_Struct* B                                );
#endif
