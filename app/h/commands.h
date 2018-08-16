#ifndef __COMMANDS_H__
#define __COMMANDS_H__


extern void Init_Uart(void);
extern int Cmd_Welcome ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Login   ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Help    ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Exit    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_T_Start ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_T_Stop  ( struct tcp_pcb* tpcb, int argc, char *argv[] );


extern int Cmd_Back2Login     ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Back2Main      ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_Main2Ip        ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Main2T         ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Main2Snmp      ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Main2System    ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_Mac            ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Ip             ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Mask           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Gateway        ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Dhcp           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Config_Port    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Link_State     ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_T              ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_T_Prom         ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Tmax           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Tmin           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Reload_T       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Reload_T_TOut  ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Temp_Port      ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_Snmp_Community ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Snmp_Iso       ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_TaskList  ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Uptime    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Wdog_Tout ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Reboot    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Hangs     ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Pwd       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Id        ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Show_Id   ( struct tcp_pcb* tpcb, int argc, char *argv[] );


void User_Commands_Task       ( void* nil                                    );
extern void DisplayIPAddress  ( struct tcp_pcb* tpcb,uint32_t ui32Addr       );


#endif // __COMMANDS_H__
