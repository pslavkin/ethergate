#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib/sysctl.h"
#include "driverlib/emac.h"
#include "driverlib/flash.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/lwiplib.h"
#include "state_machine.h"
#include "commands.h"
#include "leds.h"
#include "buttons.h"
#include "opt.h"
#include "one_wire_network.h"
#include "usr_flash.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


//*****************************************************************************
//
// This is the table that holds the command names, implementing functions, and
// brief description.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    { "help"   ,Cmd_help         ,": Display list of commands" }                              ,
    { "h"      ,Cmd_help         ,": alias for help" }                                        ,
    { "?"      ,Cmd_help         ,": alias for help" }                                        ,
    { "Mac"    ,Cmd_Mac          ,": show MAC address" }                                      ,
    { "ip"     ,Cmd_Ip           ,": show and/or save ip" }                                   ,
    { "mask"   ,Cmd_Mask         ,": show and/or save mask" }                                 ,
    { "gw"     ,Cmd_Gateway      ,": show and/or save gateway" }                              ,
    { "cp"     ,Cmd_Config_Port  ,": show and/or save config tcp port [default 49152]" }      ,
    { "tp"     ,Cmd_Temp_Port    ,": show and/or save temperature tcp port [default 49153]" } ,
    { "t"      ,Cmd_T            ,": show temperature" }                                      ,
    { "tmax"   ,Cmd_Tmax         ,": show and/or save tmax" }                                 ,
    { "tmin"   ,Cmd_Tmin         ,": show and/or save tmin" }                                 ,
    { "task"   ,Cmd_TaskList     ,": lista de tareas" }                                       ,
    { "link"   ,Cmd_Links_State  ,": Estado del link ethernet" }                              ,
    { "bt"     ,Cmd_Button_State ,": Estado del boton" }                                      ,
    { "reboot" ,Cmd_Reboot       ,": Reboot" }                                                ,
    { 0        ,0                ,0 }
};

//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************


int Cmd_help(struct tcp_pcb* tpcb, int argc, char *argv[])
{
    tCmdLineEntry *pEntry;
    UART_ETHprintf(tpcb,"\nAvailable commands\n------------------\n");
    pEntry = g_psCmdTable;
    for(;pEntry->pcCmd;pEntry++)
        UART_ETHprintf(tpcb,"%15s%s\n", pEntry->pcCmd, pEntry->pcHelp);
    return 0;
}
//----------------------------------------------------------------------------------------------------
int Cmd_Mac(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint8_t Mac[6];
   lwIPLocalMACGet ( Mac );
    UART_ETHprintf(tpcb,"MAC: %02x:%02x:%02x:%02x:%02x:%02x",
            Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5]);
   return 0;
}
int Cmd_Ip(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"Actual Ip=");
   DisplayIPAddress(tpcb,lwIPLocalIPAddrGet());
   if(argc>1) {
      uint32_t New_Ip=atoi(argv[1]);
      UART_ETHprintf(tpcb,"New Ip=");
      DisplayIPAddress(tpcb,htonl(New_Ip));
      Usr_Flash_Params.Ip_Addr=New_Ip;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Mask(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"Actual Mask=");
   DisplayIPAddress(tpcb,lwIPLocalIPAddrGet());
   if(argc>1) {
      uint32_t New_Ip=atoi(argv[1]);
      UART_ETHprintf(tpcb,"New Mask=");
      DisplayIPAddress(tpcb,htonl(New_Ip));
      Usr_Flash_Params.Mask_Addr=New_Ip;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Gateway(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"Actual Gateway=");
   DisplayIPAddress(tpcb,lwIPLocalIPAddrGet());
   if(argc>1) {
      uint32_t New_Ip=atoi(argv[1]);
      UART_ETHprintf(tpcb,"New Gateway=");
      DisplayIPAddress(tpcb,htonl(New_Ip));
      Usr_Flash_Params.Gateway_Addr=New_Ip;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Config_Port(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"Actual Port=%d\n",Usr_Flash_Params.Config_Port);
   if(argc>1) {
      uint16_t New_Port=atoi(argv[1]);
      UART_ETHprintf(tpcb,"New Port=%d \n",New_Port);
      Usr_Flash_Params.Config_Port=New_Port;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Temp_Port(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"Actual Port=%d\n",Usr_Flash_Params.Temp_Port);
   if(argc>1) {
      uint16_t New_Port=atoi(argv[1]);
      UART_ETHprintf(tpcb,"New Port=%d \n",New_Port);
      Usr_Flash_Params.Temp_Port=New_Port;
      Save_Usr_Flash();
   }
   return 0;
}


void DisplayIPAddress(struct tcp_pcb* tpcb,uint32_t ui32Addr)
{
    UART_ETHprintf(tpcb, "%d.%d.%d.%d\n", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);
}

int Cmd_T(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   Print_Temp_Nodes(tpcb);
   return 0;
}
int Cmd_Tmax       ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{
   UART_ETHprintf(tpcb,"Actual Tmax=%f\n",Usr_Flash_Params.Tmax);
   if(argc>1) {
      float fVal= ustrtof(argv[1],NULL);
      UART_ETHprintf(tpcb,"New Tmax=%f\n",fVal);
      Usr_Flash_Params.Tmax=fVal;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Tmin       ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{
   UART_ETHprintf(tpcb,"Actual Tmin=%f\n",Usr_Flash_Params.Tmin);
   if(argc>1) {
      float fVal= ustrtof(argv[1],NULL);
      UART_ETHprintf(tpcb,"New Tmin=%f\n",fVal);
      Usr_Flash_Params.Tmin=fVal;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_TaskList(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   char* Buff=(char*)pvPortMalloc(UART_TX_BUFFER_SIZE);
   vTaskList( Buff );
   UART_ETHprintf(tpcb,Buff);
   vPortFree(Buff);
   return 0;
}

int Cmd_Links_State(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"PHY=%d",EMACPHYLinkUp());
   return 0;
}
int Cmd_Button_State(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"boton=%d",Button1_Read());
   return 0;
}
int Cmd_Reboot(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   Soft_Reset();
   return 0;
}


//
//
//



//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
//*****************************************************************************
//
// Prompts the user for a command, and blocks while waiting for the user's
// input. This function will return after the execution of a single command.
//
//*****************************************************************************
void User_Commands_Task(void* nil)
{
   while(1) {
      if(UARTPeek('\r') != -1) {
         char* Buff =(char*)pvPortMalloc(APP_INPUT_BUF_SIZE);
         while(UARTPeek('\r') != -1)
         {
           UARTgets(Buff, APP_INPUT_BUF_SIZE);
           CmdLineProcess(Buff,UART_MSG);
         }
        vPortFree(Buff);
      }
      vTaskDelay( 100 / portTICK_RATE_MS );
   }

}
