#ifndef WDOG
#define WDOG
//-----------------------------------------------
void        Init_Wdog         ( void );
void        Wdog_Handler      ( void );
void        Wdog_Clear        ( void );
uint32_t    Read_Uptime       ( void );
uint8_t     Read_Uptime_Secs  ( void );
uint8_t     Read_Uptime_Mins  ( void );
uint8_t     Read_Uptime_Hours ( void );
uint32_t    Read_Uptime_Days  ( void );
//-----------------------------------------------
#endif
