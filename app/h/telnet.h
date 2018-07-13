#ifndef  TELNET
#define  TELNET

enum Telnet_Event_Code{
            Accept_Event  = 123
                 };

//-----------------------------------------------------------
//-----------------------------------------------------------
const State**  Telnet      ( void );
extern void       Telnet_Rti   ( void );
extern void       Init_Telnet  ( void );
//----------------------------------------------------

#endif

