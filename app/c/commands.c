#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib/sysctl.h"
#include "driverlib/emac.h"
#include "driverlib/flash.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/lwiplib.h"
#include "state_machine.h"
#include "commands.h"
#include "leds.h"
#include "buttons.h"
#include "telnet.h"
#include "opt.h"
#include "one_wire_network.h"
#include "one_wire_transport.h"
#include "usr_flash.h"
#include "third_party/lwip-1.4.1/src/include/ipv4/lwip/ip_addr.h"

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
tCmdLineEntry* g_psCmdTable;

tCmdLineEntry Login_Cmd_Table[] =
{
    { "login" ,Cmd_Login   ,": login" }                     ,
    { "id"    ,Cmd_Show_Id ,": show id name" }              ,
    { "exit"  ,Cmd_Exit    ,": exit and close connection" } ,
    { "?"     ,Cmd_Help    ,": help" }                      ,
    { 0       ,0           ,0 }
};
tCmdLineEntry Main_Cmd_Table[] =
{
    { "net"    ,Cmd_Main2Ip     ,": network options" }     ,
    { "temp"   ,Cmd_Main2T      ,": temperature options" } ,
    { "snmp"   ,Cmd_Main2Snmp   ,": snmp options" }        ,
    { "system" ,Cmd_Main2System ,": system options" }      ,
    { "?"      ,Cmd_Help        ,": help" }                ,
    { "<"      ,Cmd_Back2Login  ,": back" }                ,
    { 0        ,0               ,0 }
};
tCmdLineEntry Ip_Cmd_Table[] =
{
    { "Mac"  ,Cmd_Mac         ,": show MAC address" }                                 ,
    { "ip"   ,Cmd_Ip          ,": show and/or save ip" }                              ,
    { "mask" ,Cmd_Mask        ,": show and/or save mask" }                            ,
    { "gw"   ,Cmd_Gateway     ,": show and/or save gateway" }                         ,
    { "dhcp" ,Cmd_Dhcp        ,": show and/or save dhcp (0=disable 1=enable)" }       ,
    { "cp"   ,Cmd_Config_Port ,": show and/or save config tcp port [default 49152]" } ,
    { "?"    ,Cmd_Help        ,": help" }                                             ,
    { "<"    ,Cmd_Back2Main   ,": back" }                                             ,
    { 0      ,0               ,0 }
};
tCmdLineEntry T_Cmd_Table[] =
{
    { "t"         ,Cmd_T             ,": show temperatures" }                         ,
    { "tprom"     ,Cmd_T_Prom        ,": show mean temperature" }                     ,
    { "tmax"      ,Cmd_Tmax          ,": show and/or save tmax" }                     ,
    { "tmin"      ,Cmd_Tmin          ,": show and/or save tmin" }                     ,
    { "scan"      ,Cmd_Reload_T      ,": scan sensor list" }                          ,
    { "scan_tout" ,Cmd_Reload_T_TOut ,": show and/or save scan sensor list tout" }    ,
    { "tport"     ,Cmd_Temp_Port     ,": show and/or save tcp port [default 49153]" } ,
    { "?"         ,Cmd_Help          ,": help" }                                      ,
    { "<"         ,Cmd_Back2Main     ,": back" }                                      ,
    { 0           ,0                 ,0 }
};
tCmdLineEntry Snmp_Cmd_Table[] =
{
    { "community" ,Cmd_Snmp_Community ,": show and/or save snmp community name" }                        ,
    { "iso"       ,Cmd_Snmp_Iso       ,": show and/or save snmp Iso. i.e: iso 0 1122334455667788 43 6 1 2 1 33 1 2 7 1" }                                   ,
    { "?"         ,Cmd_Help           ,": help" }                                                        ,
    { "<"         ,Cmd_Back2Main      ,": back" }                                                        ,
    { 0           ,0                  ,0 }
};
tCmdLineEntry System_Cmd_Table[] =
{
    { "task"   ,Cmd_TaskList  ,": lista de tareas" }           ,
    { "id"     ,Cmd_Id        ,": show and/or save id" }       ,
    { "pwd"    ,Cmd_Pwd       ,": show and/or save password" } ,
    { "reboot" ,Cmd_Reboot    ,": reboot" }                    ,
    { "?"      ,Cmd_Help      ,": help" }                      ,
    { "<"      ,Cmd_Back2Main ,": back" }                      ,
    { 0        ,0             ,0 }
};
//--------------------------------------------------------------------------------
int Cmd_Welcome(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=Login_Cmd_Table;
   UART_ETHprintf(tpcb,"\033[2J\033[H+++ Ethergate V8.0 +++\r\nwww.disenioconingenio.com.ar\r\n");
    return 0;
}
int Cmd_Help(struct tcp_pcb* tpcb, int argc, char *argv[])
{
    tCmdLineEntry *pEntry;
    UART_ETHprintf(tpcb,"\r\navailable commands\r\n------------------\r\n");
    pEntry = g_psCmdTable;
    for(;pEntry->pcCmd;pEntry++)
        UART_ETHprintf(tpcb,"%15s%s\r\n", pEntry->pcCmd, pEntry->pcHelp);
    return 0;
}
//--------------------------------------------------------------------------------
int Cmd_Login(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc>1) {
      if(ustrcmp(argv[1],Usr_Flash_Params.Pwd)==0) {
         UART_ETHprintf(tpcb,"success\r\n");
         g_psCmdTable=Main_Cmd_Table;
         Cmd_Help(tpcb,argc,argv);
      }
      else
         UART_ETHprintf(tpcb,"invalid\r\n");
   }
   else
      UART_ETHprintf(tpcb,"no pwd\r\n");
   return 0;
}
int Cmd_Exit(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   Telnet_Close(tpcb);
   return 0;
}
int Cmd_Main2Ip(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=Ip_Cmd_Table;
//   Cmd_Help(tpcb, argc, argv);
   return 0;
}
int Cmd_Main2T(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=T_Cmd_Table;
//   Cmd_Help(tpcb, argc, argv);
   return 0;
}
int Cmd_Main2Snmp(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=Snmp_Cmd_Table;
//   Cmd_Help(tpcb, argc, argv);
   return 0;
}
int Cmd_Main2System(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=System_Cmd_Table;
//   Cmd_Help(tpcb, argc, argv);
   return 0;
}
int Cmd_Back2Main(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=Main_Cmd_Table;
//   Cmd_Help(tpcb, argc, argv);
   return 0;
}
int Cmd_Back2Login(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=Login_Cmd_Table;
//   Cmd_Help(tpcb, argc, argv);
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
   if(argc==1)
      DisplayIPAddress(tpcb,lwIPLocalIPAddrGet());
   else {
      ip_addr_t New_Ip;
      UART_ETHprintf(tpcb,"new ip:");
      if(ipaddr_aton(argv[1],&New_Ip) == 1) {
         DisplayIPAddress(tpcb,New_Ip.addr);
         Usr_Flash_Params.Ip_Addr=htonl(New_Ip.addr);
         Save_Usr_Flash();
      }
      else
         UART_ETHprintf(tpcb,"invalid\r\n");
   }
   return 0;
}
int Cmd_Mask(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      DisplayIPAddress(tpcb,lwIPLocalNetMaskGet());
   else {
      ip_addr_t New_Ip;
      UART_ETHprintf(tpcb,"new mask:");
      if(ipaddr_aton(argv[1],&New_Ip) == 1) {
         DisplayIPAddress(tpcb,New_Ip.addr);
         Usr_Flash_Params.Mask_Addr=htonl(New_Ip.addr);
         Save_Usr_Flash();
      }
      else
         UART_ETHprintf(tpcb,"invalid\r\n");
   }
   return 0;
}
int Cmd_Gateway(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      DisplayIPAddress(tpcb,lwIPLocalGWAddrGet());
   else {
      ip_addr_t New_Ip;
      UART_ETHprintf(tpcb,"new gateway:");
      if(ipaddr_aton(argv[1],&New_Ip) == 1) {
         DisplayIPAddress(tpcb,New_Ip.addr);
         Usr_Flash_Params.Gateway_Addr=htonl(New_Ip.addr);
         Save_Usr_Flash();
      }
      else
         UART_ETHprintf(tpcb,"invalid\r\n");
   }
   return 0;
}
int Cmd_Dhcp(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%d\r\n",Usr_Flash_Params.Dhcp_Enable);
   else {
      uint16_t New=atoi(argv[1]);
      UART_ETHprintf(tpcb,"new dhcp=%d \r\n",New);
      Usr_Flash_Params.Dhcp_Enable=New;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Config_Port(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%d\r\n",Usr_Flash_Params.Config_Port);
   else {
      uint16_t New_Port=atoi(argv[1]);
      UART_ETHprintf(tpcb,"new port=%d\r\n",New_Port);
      Usr_Flash_Params.Config_Port=New_Port;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Temp_Port(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%d\r\n",Usr_Flash_Params.Temp_Port);
   else {
      uint16_t New_Port=atoi(argv[1]);
      UART_ETHprintf(tpcb,"new port=%d\r\n",New_Port);
      Usr_Flash_Params.Temp_Port=New_Port;
      Save_Usr_Flash();
   }
   return 0;
}


void DisplayIPAddress(struct tcp_pcb* tpcb,uint32_t ui32Addr)
{
    UART_ETHprintf(tpcb, "%d.%d.%d.%d\r\n", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);
}

int Cmd_T(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   Print_Temp_Nodes(tpcb);
   return 0;
}
int Cmd_T_Prom(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"%f\r\n",Get_T_Prom());
   return 0;
}
int Cmd_Tmax       ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%f\r\n",Usr_Flash_Params.Tmax);
   else {
      float fVal= ustrtof(argv[1],NULL);
      UART_ETHprintf(tpcb,"new tmax=%f\r\n",fVal);
      Usr_Flash_Params.Tmax=fVal;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Tmin       ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%f\r\n",Usr_Flash_Params.Tmin);
   else {
      float fVal= ustrtof(argv[1],NULL);
      UART_ETHprintf(tpcb,"new tmin=%f\r\n",fVal);
      Usr_Flash_Params.Tmin=fVal;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Reload_T_TOut    ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%d mins\r\nnext scan in %f mins\r\n",
            Usr_Flash_Params.Reload_T_TOut,
            (float)One_Wire_Next_Reload_Time()/600
            );
   else {
      uint8_t TOut= atoi(argv[1]);
      UART_ETHprintf(tpcb,"new reload T Tout=%d mins\r\n",TOut);
      Usr_Flash_Params.Reload_T_TOut=TOut;
      Refresh_One_Wire_Reload_TOut(TOut);
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Reload_T       ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{
   UART_ETHprintf(tpcb,"scanning...\r\n");
   Reload_One_Wire_Codes( );
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
int Cmd_Snmp_Community(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%s\r\n", Usr_Flash_Params.Snmp_Community);
   else {
      UART_ETHprintf(tpcb,"new community:%s \r\n",argv[1]);
      ustrncpy(Usr_Flash_Params.Snmp_Community,argv[1],sizeof(Usr_Flash_Params.Snmp_Community)-1);
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Snmp_Iso(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint8_t i;
   if(argc==1)
      for(i=0;i<MAX_ROM_CODES;i++)
         UART_ETHprintf(tpcb,"sensor: %H snmp: %H\r\n",
               Usr_Flash_Params.Sensor_Codes[i],8,
               Usr_Flash_Params.Snmp_Iso[i],Usr_Flash_Params.Snmp_Iso_Len);
   else {
      uint8_t Channel=atoi(argv[1]);
      for(i=0;i<8;i++)
         Usr_Flash_Params.Sensor_Codes[Channel][i]=hextoc(argv[2]+2*i);

      for(i=3;i<argc && i<sizeof(Usr_Flash_Params.Snmp_Iso[0]);i++)
            Usr_Flash_Params.Snmp_Iso[Channel][i-3]=atoi(argv[i]);
         Usr_Flash_Params.Snmp_Iso_Len=argc-3;
         UART_ETHprintf(tpcb,"new snmp sensor link, sensor: %H snmp: %H\r\n",
               Usr_Flash_Params.Sensor_Codes[Channel],8,
               Usr_Flash_Params.Snmp_Iso[Channel],Usr_Flash_Params.Snmp_Iso_Len);
         Save_Usr_Flash();
   }
   return 0;
}

int Cmd_Reboot(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   Telnet_Close(tpcb);
   vTaskDelay(pdMS_TO_TICKS(2000));
   Soft_Reset();
   return 0;
}
int Cmd_Show_Id(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"%s\r\n", Usr_Flash_Params.Id);
   return 0;
}
int Cmd_Id(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%s\r\n", Usr_Flash_Params.Id);
   else {
      UART_ETHprintf(tpcb,"new id:%s \r\n",argv[1]);
      ustrncpy(Usr_Flash_Params.Id,argv[1],sizeof(Usr_Flash_Params.Id)-1);
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Pwd(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%s\r\n", Usr_Flash_Params.Pwd);
   else {
      UART_ETHprintf(tpcb,"new pwd:%s \r\n",argv[1]);
      ustrncpy(Usr_Flash_Params.Pwd,argv[1],sizeof(Usr_Flash_Params.Pwd)-1);
      Save_Usr_Flash();
   }
   return 0;
}

//--------------------------------------------------------------------------------
void User_Commands_Task(void* nil)
{
   ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
   ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
   ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
   ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

   UARTStdioConfig ( 0, 115200, configCPU_CLOCK_HZ);
   g_psCmdTable=Login_Cmd_Table;
   Cmd_Welcome(UART_MSG,0,NULL);
   Cmd_Help(UART_MSG,0,NULL);
   char* Buff =(char*)pvPortMalloc(APP_INPUT_BUF_SIZE);
   while(1) {
      while(xSemaphoreTake(Uart_Studio_Semphr,portMAX_DELAY)!=pdTRUE)
         ;
      UARTgets       ( Buff, APP_INPUT_BUF_SIZE );
      CmdLineProcess ( Buff,UART_MSG            );
   }
}
