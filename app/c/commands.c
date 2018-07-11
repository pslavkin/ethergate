//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "commands.h"
#include "opt.h"

//*****************************************************************************
//
// This is the table that holds the command names, implementing functions, and
// brief description.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    { "help",        Cmd_help,        ": Display list of commands" },
    { "h",           Cmd_help,        ": alias for help" },
    { "?",           Cmd_help,        ": alias for help" },
    { "Mac",         Cmd_Mac,        ": show MAC address" },
    { 0, 0, 0 }
};

//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int Cmd_help(int argc, char *argv[])
{
    tCmdLineEntry *pEntry;
    UARTprintf("\nAvailable commands\n------------------\n");
    pEntry = g_psCmdTable;
    for(;pEntry->pcCmd;pEntry++) 
        UARTprintf("%15s%s\n", pEntry->pcCmd, pEntry->pcHelp);
    return 0;
}
//----------------------------------------------------------------------------------------------------
int Cmd_Mac(int argc, char *argv[])
{
   UARTprintf("parametros %d\n",argc);
   UpdateMACAddr();
   return(0);
}


void UpdateMACAddr(void)
{
    uint8_t pui8MACAddr[6]={1,2,3,4,5,6};
    UARTprintf("MAC: %02x:%02x:%02x:%02x:%02x:%02x",
            pui8MACAddr[0], pui8MACAddr[1], pui8MACAddr[2], pui8MACAddr[3],
            pui8MACAddr[4], pui8MACAddr[5]);
}
void DisplayIPAddress(uint32_t ui32Addr)
{
    UARTprintf( "%d.%d.%d.%d", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);
}


//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
char g_cInput[APP_INPUT_BUF_SIZE];
//*****************************************************************************
//
// Prompts the user for a command, and blocks while waiting for the user's
// input. This function will return after the execution of a single command.
//
//*****************************************************************************
void
CheckForUserCommands(void)
{
    int iStatus;

    //
    // Peek to see if a full command is ready for processing
    //
    if(UARTPeek('\r') == -1)
    {
        //
        // If not, return so other functions get a chance to run.
        //
        return;
    }

    //
    // If we do have commands, process them immediately in the order they were
    // received.
    //
    while(UARTPeek('\r') != -1)
    {
        //
        // Get a user command back
        //
        UARTgets(g_cInput, APP_INPUT_BUF_SIZE);

        //
        // Process the received command
        //
        iStatus = CmdLineProcess(g_cInput);

        //
        // Handle the case of bad command.
        //
        if(iStatus == CMDLINE_BAD_CMD)
        {
            UARTprintf("Bad command!\n");
        }

        //
        // Handle the case of too many arguments.
        //
        else if(iStatus == CMDLINE_TOO_MANY_ARGS)
        {
            UARTprintf("Too many arguments for command processor!\n");
        }
    }

    //
    // Print a prompt
    //
    UARTprintf("\n> ");

}
