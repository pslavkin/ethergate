#ifndef COMMANDS
#define COMMANDS

enum Commands_Events {
    Conn_Regi_Event       = 0,
    Conn_Free_Event       = 1,
    Data_Arrived_Event    = 2,
    Console_Enable_Event  = 3,
    Console_Disable_Event = 4,
    Term_Found_Event      = 5,
    Max_Length_Event      = 6,
    TOut_Event            = 7,
    Enter_Found_Event      = 8,
};

struct Line_Process_Struct
{
   uint8_t Buff[TCP_MSS];
//   struct tcp_pcb* tpcb;
   uint32_t Id;
   uint32_t Index;
   uint32_t Tout;
};
void              Init_Commands         ( void );
const State**     Commands              ( void );

extern tCmdLineEntry Login_Cmd_Table[];


void Init_Uart(void);
int Cmd_Welcome ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Login   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Help    ( struct Parser_Queue_Struct* P ,int argc ,char *argv[] );
int Cmd_Exit    ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_T_Start ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_T_Stop  ( struct Parser_Queue_Struct* P, int argc, char *argv[] );


int Cmd_Back2Login     ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Back2Main      ( struct Parser_Queue_Struct* P, int argc, char *argv[] );

int Cmd_Main2Ip        ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Main2T         ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Main2Snmp      ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Main2System    ( struct Parser_Queue_Struct* P, int argc, char *argv[] );

int Cmd_Mac          ( struct Parser_Queue_Struct* P ,int argc ,char *argv[] );
int Cmd_Ip           ( struct Parser_Queue_Struct* P ,int argc ,char *argv[] );
int Cmd_Mask         ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Gateway      ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Dhcp         ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Config_Port  ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Sniffer_Port ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Rs232_Port   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Virtual_Port ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Link_State   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );

int Cmd_T              ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_T_Prom         ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Tmax           ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Tmin           ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Reload_T       ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Reload_T_TOut  ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Temp_Port      ( struct Parser_Queue_Struct* P, int argc, char *argv[] );

int Cmd_Snmp_Community ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Snmp_Iso       ( struct Parser_Queue_Struct* P, int argc, char *argv[] );

int Cmd_Rs232_Baud        ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Rs232_Len         ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Rs232_Tout        ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Main2Rs232        ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Rs232_Menu_Enable ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Rs232_Ascii_Term  ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Rs232_Code_Term   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );


int Cmd_TaskList  ( struct Parser_Queue_Struct* P ,int argc ,char *argv[] );
int Cmd_Uptime    ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Wdog_Tout ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Restore   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Reboot    ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Hangs     ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Pwd       ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Id        ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Show_Id   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );


void User_Commands_Task       ( void* nil                                    );
extern void DisplayIPAddress  ( struct Parser_Queue_Struct* P,uint32_t ui32Addr       );

void Line_Process(void);
void Parser_Process(void);

void Send_Data2Tcp        ( void );
void Bridge_Data_Process  ( void );
void Send_Data2Parser     ( void );
void Console_Data_Process ( void );
void Is_Console_Enabled   ( void );
void Bridge2Console       ( void );
void Console2Bridge       ( void );



#endif // __COMMANDS_H__
