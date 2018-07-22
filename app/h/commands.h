//*****************************************************************************

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Forward declarations for command-line operations.
//
//*****************************************************************************
extern int Cmd_help              ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Mac               ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Ip                ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Send2Eth          ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Write2Eth         ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern void UpdateMACAddr        ( struct tcp_pcb* tpcb                         );
extern void CheckForUserCommands ( void* nil                                    );
extern void DisplayIPAddress     ( uint32_t ui32Addr                            );

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __COMMANDS_H__
