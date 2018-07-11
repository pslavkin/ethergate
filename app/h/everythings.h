#ifndef  EVERYTHINGS
#define  EVERYTHINGS

#include "state_machine.h"

//-----------------------------------------------------------
enum Everythings_Event_Code{
            Everythings_Null_Event  = 0
                 };
//-----------------------------------------------------------
extern void       Everythings_Rti   ( void );
extern void       Init_Everythings  ( void );
extern const State**    Everythings ( void );
//----------------------------------------------------

#endif

