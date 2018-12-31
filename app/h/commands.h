#ifndef __COMMANDS_H__
#define __COMMANDS_H__


void Init_Uart(void);
int Cmd_Welcome ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Login   ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Help    ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
int Cmd_Exit    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_T_Start ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_T_Stop  ( struct tcp_pcb* tpcb, int argc, char *argv[] );


int Cmd_Back2Login     ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Back2Main      ( struct tcp_pcb* tpcb, int argc, char *argv[] );

int Cmd_Main2Ip        ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Main2T         ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Main2Snmp      ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Main2System    ( struct tcp_pcb* tpcb, int argc, char *argv[] );

int Cmd_Mac            ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
int Cmd_Ip             ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
int Cmd_Mask           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Gateway        ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Dhcp           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Config_Port    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Link_State     ( struct tcp_pcb* tpcb, int argc, char *argv[] );

int Cmd_T              ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_T_Prom         ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Tmax           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Tmin           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Reload_T       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Reload_T_TOut  ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Temp_Port      ( struct tcp_pcb* tpcb, int argc, char *argv[] );

int Cmd_Snmp_Community ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Snmp_Iso       ( struct tcp_pcb* tpcb, int argc, char *argv[] );

int Cmd_Rs232_Baud(struct tcp_pcb* tpcb, int argc, char *argv[]);
int Cmd_Rs232_Len(struct tcp_pcb* tpcb, int argc, char *argv[]);
int Cmd_Main2Rs232(struct tcp_pcb* tpcb, int argc, char *argv[]);
int Cmd_Rs232_Menu_Enable(struct tcp_pcb* tpcb, int argc, char *argv[]);


int Cmd_TaskList  ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
int Cmd_Uptime    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Wdog_Tout ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Reboot    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Hangs     ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Pwd       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Id        ( struct tcp_pcb* tpcb, int argc, char *argv[] );
int Cmd_Show_Id   ( struct tcp_pcb* tpcb, int argc, char *argv[] );


void User_Commands_Task       ( void* nil                                    );
extern void DisplayIPAddress  ( struct tcp_pcb* tpcb,uint32_t ui32Addr       );


#endif // __COMMANDS_H__
