#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "opt.h"
#include "utils/lwiplib.h"

#define CMDLINE_BAD_CMD       ( -1 ) // ! Defines the value that is returned if the command is not found.
#define CMDLINE_TOO_MANY_ARGS ( -2 ) // ! Defines the value that is returned if there are too many arguments.
#define CMDLINE_TOO_FEW_ARGS  ( -3 ) // ! Defines the value that is returned if there are too few arguments.
#define CMDLINE_INVALID_ARG   ( -4 ) // ! Defines the value that is returned if an argument is invalid.

// Command line function callback type.
struct Parser_Queue_Struct;
typedef int (*pfnCmdLine)(struct Parser_Queue_Struct* P, int argc, char *argv[]);

typedef struct Cmd_Table_Struct{        // ! Structure for an entry in the command list table.
    const char *pcCmd;  // ! A pointer to a string containing the name of the command.
    pfnCmdLine pfnCmd;  // ! A function pointer to the implementation of the command.
    const char *pcHelp; // ! A pointer to a string of brief help text for the command.
} tCmdLineEntry;



// Prototypes for the APIs.
void CmdLineProcess(struct Parser_Queue_Struct* P);
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __CMDLINE_H__
