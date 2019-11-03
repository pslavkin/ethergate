#ifndef COMMANDS
#define COMMANDS

extern tCmdLineEntry Login_Cmd_Table[];

int Cmd_Welcome ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Login   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Help    ( struct Parser_Queue_Struct* P ,int argc ,char *argv[] );
int Cmd_Exit    ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_T_Start ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_T_Stop  ( struct Parser_Queue_Struct* P, int argc, char *argv[] );


int Cmd_Back2Login     ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Back2Main      ( struct Parser_Queue_Struct* P, int argc, char *argv[] );

int Cmd_Main2Ip     ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Main2T      ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Main2Snmp   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Main2System ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Main2Stats  ( struct Parser_Queue_Struct* P, int argc, char *argv[] );

int Cmd_Mac            ( struct Parser_Queue_Struct* P ,int argc ,char *argv[] );
int Cmd_Actual_Address ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Static_Ip      ( struct Parser_Queue_Struct* P ,int argc ,char *argv[] );
int Cmd_Static_Mask    ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Static_Gateway ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Dhcp           ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Config_Port    ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Sniffer_Port   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Rs232_Port     ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Virtual_Port   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Link_State     ( struct Parser_Queue_Struct* P, int argc, char *argv[] );

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
int Cmd_Pwd       ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Id        ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_Show_Id   ( struct Parser_Queue_Struct* P, int argc, char *argv[] );

//stats
int Cmd_SysStats  ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_MemStats  ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_MempStats ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_EthStats  ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_IcmpStats ( struct Parser_Queue_Struct* P, int argc, char *argv[] );
int Cmd_AllStats  ( struct Parser_Queue_Struct* P, int argc, char *argv[] );

void Print_Enable_Disable ( struct Parser_Queue_Struct* P,const char* Legend,bool State );
void DisplayIPAddress     ( struct Parser_Queue_Struct* P,uint32_t ui32Addr             );

#endif
