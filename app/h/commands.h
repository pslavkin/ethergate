#ifndef __COMMANDS_H__
#define __COMMANDS_H__

extern int Cmd_help             ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Mac              ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Ip               ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Mask             ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Gateway          ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Temp_Port(struct tcp_pcb* tpcb, int argc, char *argv[]);
extern int Cmd_Config_Port(struct tcp_pcb* tpcb, int argc, char *argv[]);
extern int Cmd_Send2Eth         ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_TaskList         ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Links_State      ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Button_State     ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_T          ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Tmax       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Tmin       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Print_Usr_Params ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Reboot           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Save_Usr_Params  ( struct tcp_pcb* tpcb, int argc, char *argv[] );
void User_Commands_Task         ( void* nil                                    );
extern void DisplayIPAddress    ( struct tcp_pcb* tpcb,uint32_t ui32Addr       );


#endif // __COMMANDS_H__
