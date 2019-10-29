#include <stdio.h>/*{{{*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib/sysctl.h"
#include "driverlib/emac.h"
#include "driverlib/flash.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/lwiplib.h"
#include "usr_flash.h"
#include "parser.h"
#include "commands.h"
#include "events.h"
#include "state_machine.h"
#include "leds.h"
#include "buttons.h"
#include "rs232.h"
#include "telnet.h"
#include "opt.h"
#include "wdog.h"
#include "one_wire_network.h"
#include "one_wire_transport.h"
#include "usr_flash.h"
#include "schedule.h"
#include "third_party/lwip-1.4.1/src/include/ipv4/lwip/ip_addr.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"/*}}}*/

tCmdLineEntry Login_Cmd_Table[] =/*{{{*/
{
    { "login"  ,Cmd_Login   ,": login" }                                ,
    { "id"     ,Cmd_Show_Id ,": show id name" }                         ,
#if ONE_WIRE_ENABLE
    { "t"      ,Cmd_T       ,": show temperatures" }                    ,
    { "tstart" ,Cmd_T_Start ,": start show temperatures periodically" } ,
    { "tstop"  ,Cmd_T_Stop  ,": stop show temperatures periodically" }  ,
#endif
    { "exit"   ,Cmd_Exit    ,": exit and close connection" }            ,
    { "?"      ,Cmd_Help    ,": help" }                                 ,
    { 0        ,0           ,0 }
};/*}}}*/
tCmdLineEntry Main_Cmd_Table[] =/*{{{*/
{
    { "net"    ,Cmd_Main2Ip     ,": network options"     },
#if ONE_WIRE_ENABLE
    { "temp"   ,Cmd_Main2T      ,": temperature options" },
    { "snmp"   ,Cmd_Main2Snmp   ,": snmp options"        },
#endif
#if RS232_ETH_ENABLE
    { "rs232"  ,Cmd_Main2Rs232  ,": Rs232 options"       },
#endif
    { "system" ,Cmd_Main2System ,": system options"      },
    { "stats"  ,Cmd_Main2Stats  ,": stats options"       },
    { "?"      ,Cmd_Help        ,": help"                },
    { "<"      ,Cmd_Back2Login  ,": back"                },
    { 0        ,0               ,0                       }
};/*}}}*/
tCmdLineEntry Ip_Cmd_Table[] =/*{{{*/
{
    { "mac"  ,Cmd_Mac                ,": show MAC address"                                 } ,
    { "add"  ,Cmd_Actual_Address     ,": show/save actual address"                         } ,
    { "ip"   ,Cmd_Static_Ip          ,": show/save static ip"                              } ,
    { "mask" ,Cmd_Static_Mask        ,": show/save static mask"                            } ,
    { "gw"   ,Cmd_Static_Gateway     ,": show/save static gateway"                         } ,
    { "dhcp" ,Cmd_Dhcp               ,": show/save dhcp (0=disable 1=enable)"              } ,
    { "cp"   ,Cmd_Config_Port        ,": show/save config tcp port [default 49152]"        } ,
    { "tp"   ,Cmd_Temp_Port          ,": show/save temperature port [default 49153]"       } ,
    { "bp"   ,Cmd_Rs232_Port         ,": show/save rs232<>tcp bridge port [default 49154]" } ,
    { "sp"   ,Cmd_Sniffer_Port       ,": show/save sniffer port [default 49155]"           } ,
    { "vp"   ,Cmd_Virtual_Port       ,": show/save virtual rs232 port [default 49156]"     } ,
    { "link" ,Cmd_Link_State         ,": show link state"                                  } ,
    { "con"  ,Cmd_Connect            ,": Connect "                                         } ,
    { "list" ,Cmd_Print_Clients_List ,": Print Client List "                               } ,
    { "init" ,Cmd_Init_Client_List   ,": Init Client List "                                } ,
    { "?"    ,Cmd_Help               ,": help"                                             } ,
    { "<"    ,Cmd_Back2Main          ,": back"                                             } ,
    { 0      ,0                      ,0                                                    }
};/*}}}*/
tCmdLineEntry T_Cmd_Table[] =/*{{{*/
{
   { "t"         ,Cmd_T             ,": show temperatures" }                    ,
   { "tstart"    ,Cmd_T_Start       ,": start show temperatures periodically" } ,
   { "tstop"     ,Cmd_T_Stop        ,": stop show temperatures periodically" }  ,
   { "tprom"     ,Cmd_T_Prom        ,": show mean temperature" }                ,
   { "tmax"      ,Cmd_Tmax          ,": show/save tmax" }                       ,
   { "tmin"      ,Cmd_Tmin          ,": show/save tmin" }                       ,
   { "scan"      ,Cmd_Reload_T      ,": scan sensor list" }                     ,
   { "scan_tout" ,Cmd_Reload_T_TOut ,": show/save scan sensor list tout" }      ,
   { "?"         ,Cmd_Help          ,": help" }                                 ,
   { "<"         ,Cmd_Back2Main     ,": back" }                                 ,
   { 0           ,0                 ,0 }
};/*}}}*/
tCmdLineEntry Snmp_Cmd_Table[] =/*{{{*/
{
   { "com" ,Cmd_Snmp_Community ,": show/save snmp community name" }                                         ,
   { "iso" ,Cmd_Snmp_Iso       ,": show/save snmp Iso. i.e: iso 0 43000007F598B328 43 6 1 2 1 33 1 2 7 1" } ,
   { "?"   ,Cmd_Help           ,": help" }                                                                  ,
   { "<"   ,Cmd_Back2Main      ,": back" }                                                                  ,
   { 0     ,0                  ,0 }
};/*}}}*/
tCmdLineEntry Rs232_Cmd_Table[] =/*{{{*/
{
{ "baud"  ,Cmd_Rs232_Baud        ,": show/save RS232 baud rate"                        },
{ "len"   ,Cmd_Rs232_Len         ,": show/save RS232 packet len"                       },
{ "aterm" ,Cmd_Rs232_Ascii_Term  ,": show/save RS232 ascii packet terminator"          },
{ "cterm" ,Cmd_Rs232_Code_Term   ,": show/save RS232 code packet terminator"           },
{ "tout"  ,Cmd_Rs232_Tout        ,": show/save RS232 packet time out"                  },
{ "menu"  ,Cmd_Rs232_Menu_Enable ,": show/save RS232 command menu. 1=enable 0=disable" },
{ "?"     ,Cmd_Help              ,": help"                                             },
{ "<"     ,Cmd_Back2Main         ,": back"                                             },
{ 0       ,0                     ,0                                                    }
};/*}}}*/
tCmdLineEntry System_Cmd_Table[] =/*{{{*/
{
{ "id"      ,Cmd_Id        ,": show and/or save id"                } ,
{ "pwd"     ,Cmd_Pwd       ,": show and/or save password"          } ,
{ "reboot"  ,Cmd_Reboot    ,": reboot"                             } ,
{ "task"    ,Cmd_TaskList  ,": rsv"                                } ,
{ "ut"      ,Cmd_Uptime    ,": uptime [secs]"                      } ,
{ "wdog"    ,Cmd_Wdog_Tout ,": wdog tout [secs] (0 disable)"       } ,
{ "restore" ,Cmd_Restore   ,": restore defaults values and reboot" } ,
{ "?"       ,Cmd_Help      ,": help"                               } ,
{ "<"       ,Cmd_Back2Main ,": back"                               } ,
{ 0         ,0             ,0                                      }
};/*}}}*/
tCmdLineEntry Stats_Cmd_Table[] =/*{{{*/
{
{ "sys"  ,Cmd_SysStats  ,": sys stats"  },
{ "mem"  ,Cmd_MemStats  ,": mem stats"  },
{ "memp" ,Cmd_MempStats ,": memp stats" },
{ "eth"  ,Cmd_EthStats  ,": eth stats"  },
{ "icmp" ,Cmd_IcmpStats ,": icmp stats" },
{ "all"  ,Cmd_AllStats  ,": all stats"  },
{ "?"    ,Cmd_Help      ,": help"       },
{ "<"    ,Cmd_Back2Main ,": back"       },
{ 0      ,0             ,0              }
};/*}}}*/
//--------------------------------------------------------------------------------
//WELCOME-HELP{{{
int Cmd_Welcome(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   UART_ETHprintf(P->tpcb,"\r\n+++ Ethergate V8.0 +++\r\nwww.disenioconingenio.com.ar\r\n");
   return 0;
}
int Cmd_Help(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
    tCmdLineEntry *pEntry;
    UART_ETHprintf(P->tpcb,"\r\navailable commands\r\n------------------\r\n");
    pEntry = P->CmdTable;
    for(;pEntry->pcCmd;pEntry++)
        UART_ETHprintf(P->tpcb,"%15s%s\r\n", pEntry->pcCmd, pEntry->pcHelp);
    return 0;
}/*}}}*/
//LOGIN{{{
int Cmd_Login(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc>1) {
      if(ustrcmp(argv[1],Usr_Flash_Params.Pwd)==0) {
         UART_ETHprintf(P->tpcb,"success\r\n");
         P->CmdTable      = Main_Cmd_Table;
         P->Ref->CmdTable = Main_Cmd_Table;
         Cmd_Help(P,argc,argv);
      }
      else
         UART_ETHprintf(P->tpcb,"invalid\r\n");
   }
   else
      UART_ETHprintf(P->tpcb,"no pwd\r\n");
   return 0;
}
int Cmd_Exit(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   Telnet_Close(P->tpcb);
   return 0;
}
int Cmd_Main2Ip(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   P->Ref->CmdTable=Ip_Cmd_Table;
//   Cmd_Help(P->tpcb, argc, argv);
   return 0;
}
int Cmd_Main2T(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   P->Ref->CmdTable=T_Cmd_Table;
//   Cmd_Help(P->tpcb, argc, argv);
   return 0;
}
int Cmd_Main2Snmp(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   P->Ref->CmdTable=Snmp_Cmd_Table;
//   Cmd_Help(P->tpcb, argc, argv);
   return 0;
}

int Cmd_Main2Rs232(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   P->Ref->CmdTable=Rs232_Cmd_Table;
//   Cmd_Help(P->tpcb, argc, argv);
   return 0;
}
int Cmd_Main2System(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   P->Ref->CmdTable=System_Cmd_Table;
//   Cmd_Help(P->tpcb, argc, argv);
   return 0;
}
int Cmd_Main2Stats(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   P->Ref->CmdTable=Stats_Cmd_Table;
//   Cmd_Help(P->tpcb, argc, argv);
   return 0;
}
int Cmd_Back2Main(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   P->Ref->CmdTable=Main_Cmd_Table;
//   Cmd_Help(P->tpcb, argc, argv);
   return 0;
}
int Cmd_Back2Login(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   P->Ref->CmdTable=Login_Cmd_Table;
//   Cmd_Help(P->tpcb, argc, argv);
   return 0;
}/*}}}*/
//TEMP{{{
int Cmd_T(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   Print_Temp_Nodes(P->tpcb);
   return 0;
}
struct tcp_pcb* tpcb4per;
void Print_Temp_Nodes_Per(void)
{
   if(tpcb4per->state!=ESTABLISHED) {
      Free_Func_Schedule(Print_Temp_Nodes_Per);
      tpcb4per=NULL;
   }
   else
      tcpip_callback((tcpip_callback_fn)Print_Temp_Nodes,(void*)tpcb4per);
}
int Cmd_T_Stop(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
      Free_Func_Schedule(Print_Temp_Nodes_Per);
      tpcb4per=NULL;
      return 0;
}
int Cmd_T_Start(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   tpcb4per=P->tpcb;
   Free_Func_Schedule(Print_Temp_Nodes_Per);
   Print_Temp_Nodes(P->tpcb);
   New_Periodic_Func_Schedule(20,Print_Temp_Nodes_Per);
   return 0;
}
int Cmd_T_Prom(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   UART_ETHprintf(P->tpcb,"%f\r\n",Get_T_Prom());
   return 0;
}
int Cmd_Tmax       ( struct Parser_Queue_Struct* P, int argc, char *argv[] )
{
   if(argc==1)
      UART_ETHprintf(P->tpcb,"%f\r\n",Usr_Flash_Params.Tmax);
   else {
      float fVal= ustrtof(argv[1],NULL);
      UART_ETHprintf(P->tpcb,"new tmax=%f\r\n",fVal);
      Usr_Flash_Params.Tmax=fVal;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Tmin       ( struct Parser_Queue_Struct* P, int argc, char *argv[] )
{
   if(argc==1)
      UART_ETHprintf(P->tpcb,"%f\r\n",Usr_Flash_Params.Tmin);
   else {
      float fVal= ustrtof(argv[1],NULL);
      UART_ETHprintf(P->tpcb,"new tmin=%f\r\n",fVal);
      Usr_Flash_Params.Tmin=fVal;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Reload_T_TOut    ( struct Parser_Queue_Struct* P, int argc, char *argv[] )
{
   if(argc==1)
      UART_ETHprintf(P->tpcb,"%d mins\r\nnext scan in %f mins\r\n",
            Usr_Flash_Params.Reload_T_TOut,
            (float)One_Wire_Next_Reload_Time()/600
            );
   else {
      uint8_t TOut= atoi(argv[1]);
      UART_ETHprintf(P->tpcb,"new reload T Tout=%d mins\r\n",TOut);
      Usr_Flash_Params.Reload_T_TOut=TOut;
      Refresh_One_Wire_Reload_TOut(TOut);
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Reload_T       ( struct Parser_Queue_Struct* P, int argc, char *argv[] )
{
   UART_ETHprintf(P->tpcb,"scanning...\r\n");
   Reload_One_Wire_Codes( );
   return 0;
}/*}}}*/
//IP{{{
int Cmd_Mac(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   uint8_t Actual_Mac[6];
   uint8_t* Mac;
   if(argc==7) {
      uint8_t i;
      Mac=Usr_Flash_Params.Mac_Addr;
      for(i=0;i<6;i++)
         Mac[i]=hextoc(argv[i+1]);
      Save_Usr_Flash();
      UART_ETHprintf(P->tpcb,"new ");
   }
   else {
      Mac=Actual_Mac;
      lwIPLocalMACGet ( Mac );
   }
   UART_ETHprintf(P->tpcb,"mac: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                        Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5]);
   return 0;
}

int Cmd_Actual_Address(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
      UART_ETHprintf(P->tpcb,"Actual address:\r\nip     :");
      DisplayIPAddress ( P ,lwIPLocalIPAddrGet  ( ));
      UART_ETHprintf(P->tpcb,"mask   :");
      DisplayIPAddress ( P ,lwIPLocalNetMaskGet ( ));
      UART_ETHprintf(P->tpcb,"gateway:");
      DisplayIPAddress ( P ,lwIPLocalGWAddrGet  ( ));
      return 0;
}
int Cmd_Static_Ip(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc==1)
      DisplayIPAddress(P,ntohl(Usr_Flash_Params.Ip_Addr));
   else {
      ip_addr_t New_Ip;
      UART_ETHprintf(P->tpcb,"new ip:");
      if(ipaddr_aton(argv[1],&New_Ip) == 1) {
         DisplayIPAddress(P,New_Ip.addr);
         Usr_Flash_Params.Ip_Addr=htonl(New_Ip.addr);
         Save_Usr_Flash();
      }
      else
         UART_ETHprintf(P->tpcb,"invalid\r\n");
   }
   return 0;
}
int Cmd_Static_Mask(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc==1)
      DisplayIPAddress(P,ntohl(Usr_Flash_Params.Mask_Addr));
   else {
      ip_addr_t New_Ip;
      UART_ETHprintf(P->tpcb,"new mask:");
      if(ipaddr_aton(argv[1],&New_Ip) == 1) {
         DisplayIPAddress(P,New_Ip.addr);
         Usr_Flash_Params.Mask_Addr=htonl(New_Ip.addr);
         Save_Usr_Flash();
      }
      else
         UART_ETHprintf(P->tpcb,"invalid\r\n");
   }
   return 0;
}
int Cmd_Static_Gateway(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc==1)
      DisplayIPAddress(P,ntohl(Usr_Flash_Params.Gateway_Addr));
   else {
      ip_addr_t New_Ip;
      UART_ETHprintf(P->tpcb,"new gateway:");
      if(ipaddr_aton(argv[1],&New_Ip) == 1) {
         DisplayIPAddress(P,New_Ip.addr);
         Usr_Flash_Params.Gateway_Addr=htonl(New_Ip.addr);
         Save_Usr_Flash();
      }
      else
         UART_ETHprintf(P->tpcb,"invalid\r\n");
   }
   return 0;
}
int Cmd_Dhcp(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc>1) {
      Usr_Flash_Params.Dhcp_Enable=atoi(argv[1]);
      Save_Usr_Flash();
   }
   Print_Enable_Disable(P,"Dhcp",Usr_Flash_Params.Dhcp_Enable);
   return 0;
}
int Cmd_Port(struct Parser_Queue_Struct* P, int argc, char *argv[],uint16_t* Struct_Port)
{
   if(argc>1) {
      *Struct_Port=atoi(argv[1]);
      Save_Usr_Flash();
   }
   UART_ETHprintf(P->tpcb,"port: %d\r\n",*Struct_Port);
   return 0;
}
int Cmd_Config_Port(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   Cmd_Port(P,argc,argv,&Usr_Flash_Params.Config_Port);
   return 0;
}
int Cmd_Virtual_Port(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   Cmd_Port(P,argc,argv,&Usr_Flash_Params.Virtual_Port);
   return 0;
}
int Cmd_Rs232_Port(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   Cmd_Port(P,argc,argv,&Usr_Flash_Params.Rs232_Port);
   return 0;
}
int Cmd_Sniffer_Port(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   Cmd_Port(P,argc,argv,&Usr_Flash_Params.Sniffer_Port);
   return 0;
}
int Cmd_Temp_Port(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   Cmd_Port(P,argc,argv,&Usr_Flash_Params.Temp_Port);
   return 0;
}
void Print_Enable_Disable(struct Parser_Queue_Struct* P,const char* Legend,bool State)
{
   UART_ETHprintf(P->tpcb,"%s %s\r\n",Legend,State?"enable":"disable");
}
void DisplayIPAddress(struct Parser_Queue_Struct* P,uint32_t ui32Addr)
{
    UART_ETHprintf(P->tpcb, "%d.%d.%d.%d\r\n", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);
}
int Cmd_Link_State(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   UART_ETHprintf(P->tpcb,"link %s\r\n",EMACPHYLinkUp()?"up":"down");
   return 0;
}/*}}}*/
//SNMP /*{{{*/
int Cmd_Snmp_Community(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(P->tpcb,"%s\r\n", Usr_Flash_Params.Snmp_Community);
   else {
      UART_ETHprintf(P->tpcb,"new community:%s \r\n",argv[1]);
      ustrncpy(Usr_Flash_Params.Snmp_Community,argv[1],sizeof(Usr_Flash_Params.Snmp_Community)-1);
      Save_Usr_Flash();
   }
   return 0;
}
void Print_Snmp_Iso(struct Parser_Queue_Struct* P,uint8_t* Iso,uint8_t Len)
{
   uint8_t i;
   for(i=0;i<Len;i++)
      UART_ETHprintf(P->tpcb," %02d",Iso[i]);
   UART_ETHprintf(P->tpcb,"\r\n");
}
int Cmd_Snmp_Iso(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   uint8_t i;
   if(argc==1)
      for(i=0;i<MAX_ROM_CODES;i++) {
         UART_ETHprintf(P->tpcb,"channel: %d sensor: %H snmp:",i,Usr_Flash_Params.Sensor_Codes[i],8);
         Print_Snmp_Iso(P,Usr_Flash_Params.Snmp_Iso[i],Usr_Flash_Params.Snmp_Iso_Len[i]);
      }
   else {
      if(argc>3) {
         uint8_t Channel=atoi(argv[1]);
         if(Channel<MAX_ROM_CODES) {
            for(i=0;i<8;i++)
               Usr_Flash_Params.Sensor_Codes[Channel][i]=hextoc(argv[2]+2*i);

            for(i=3;i<argc && i<sizeof(Usr_Flash_Params.Snmp_Iso[0]);i++)
               Usr_Flash_Params.Snmp_Iso[Channel][i-3]=atoi(argv[i]);
            Usr_Flash_Params.Snmp_Iso_Len[Channel]=argc-3;
            UART_ETHprintf(P->tpcb,"new snmp sensor link, channel: %d sensor: %H snmp:",Channel,Usr_Flash_Params.Sensor_Codes[Channel],8);
            Print_Snmp_Iso(P,Usr_Flash_Params.Snmp_Iso[Channel],Usr_Flash_Params.Snmp_Iso_Len[Channel]);
            Save_Usr_Flash();
         }
      }
   }
   return 0;
}/*}}}*/
//RS232{{{
int Cmd_Rs232_Baud(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(P->tpcb,"Baud: %d bps\r\n", Usr_Flash_Params.Rs232_Baud);
   else {
      Usr_Flash_Params.Rs232_Baud=atoi(argv[1]);
      UART_ETHprintf(P->tpcb,"New baud: %d bps\r\n", Usr_Flash_Params.Rs232_Baud);
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Rs232_Len(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(P->tpcb,"Len: %d\r\n", Usr_Flash_Params.Rs232_Len);
   else {
      uint16_t Len=atoi(argv[1]);
      Usr_Flash_Params.Rs232_Len=Len<=sizeof(((struct Line_Process_Struct*)0)->Buff)?
                                Len:
                                sizeof(((struct Line_Process_Struct*)0)->Buff);
      UART_ETHprintf(P->tpcb,"New Len: %d\r\n", Usr_Flash_Params.Rs232_Len);
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Rs232_Ascii_Term(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc>1) {
      Usr_Flash_Params.Rs232_Term= argv[1][0];
      Save_Usr_Flash();
   }
   UART_ETHprintf(P->tpcb,"Packet terminator: %c\r\n", Usr_Flash_Params.Rs232_Term);
   return 0;
}
int Cmd_Rs232_Code_Term(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc>1) {
      Usr_Flash_Params.Rs232_Term= atoi(argv[1]);
      Save_Usr_Flash();
   }
   UART_ETHprintf(P->tpcb,"Packet terminator: %d\r\n", Usr_Flash_Params.Rs232_Term);
   return 0;
}
int Cmd_Rs232_Tout(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(P->tpcb,"Tout: %d secs/10\r\n", Usr_Flash_Params.Rs232_Tout);
   else {
      Usr_Flash_Params.Rs232_Tout=atoi(argv[1]);
      UART_ETHprintf(P->tpcb,"New Tout: %d secs/10\r\n", Usr_Flash_Params.Rs232_Tout);
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Rs232_Menu_Enable(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc>1) {
      Usr_Flash_Params.Rs232_Menu_Enable=atoi(argv[1])>=1?true:false;
      Save_Usr_Flash();
   }
   Print_Enable_Disable(P,"Menu",Usr_Flash_Params.Rs232_Menu_Enable);
   return 0;
}/*}}}*/
//SYSTEM{{{
int Cmd_Uptime(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   UART_ETHprintf(P->tpcb,"%d days %02d:%02d:%02d \r\n",
                  Read_Uptime_Days  ( ),
                  Read_Uptime_Hours ( ),
                  Read_Uptime_Mins  ( ),
                  Read_Uptime_Secs  ( )
                  );
   return 0;
}
int Cmd_Wdog_Tout(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(P->tpcb,"%d secs\r\n",Usr_Flash_Params.Wdog);
   else {
      uint16_t New_Time=atoi(argv[1]);
      if(New_Time>=120 || New_Time==0) {
         UART_ETHprintf(P->tpcb,"new wdog=%d secs\r\n",New_Time);
         Usr_Flash_Params.Wdog=New_Time;
         Save_Usr_Flash();
      }
      else
         UART_ETHprintf(P->tpcb,"wdog too short (<120 secs) %d secs\r\n",New_Time);
   }
   return 0;
}
int Cmd_TaskList(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
#if (configUSE_TRACE_FACILITY == 1)
   if(argc==2 && ustrcmp("tareas",argv[1])==0) {
      char* Buff=(char*)pvPortMalloc(UART_TX_BUFFER_SIZE);
      UART_ETHprintf(P->tpcb,"%14s state   pri     stack  num\r\n","name");
      vTaskList( Buff );
      UART_ETHprintf(P->tpcb,Buff);
      UART_ETHprintf(P->tpcb,"Total Heap=%d\r\n", xPortGetFreeHeapSize());
      //UART_ETHprintf(P->tpcb,"Total Heap=%d Min=%d\r\n", xPortGetFreeHeapSize(),xPortGetMinimumEverFreeHeapSize());
      vPortFree(Buff);
   }
#else
   UART_ETHprintf(P->tpcb,"Trace not defined\r\n");
#endif
   return 0;
}
int Cmd_Reboot(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   Telnet_Close(P->tpcb);
   New_None_Periodic_Func_Schedule(20,Soft_Reset); //tengo que rebootear dede fuerea de Rcv_fn
   return 0;
}
int Cmd_Show_Id(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   UART_ETHprintf(P->tpcb,"%s\r\n", Usr_Flash_Params.Id);
   return 0;
}
int Cmd_Id(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(P->tpcb,"%s\r\n", Usr_Flash_Params.Id);
   else {
      UART_ETHprintf(P->tpcb,"new id:%s \r\n",argv[1]);
      ustrncpy(Usr_Flash_Params.Id,argv[1],sizeof(Usr_Flash_Params.Id)-1);
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Restore(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   Usr_Flash2Defult_Values (                    ) ;
   Save_Usr_Flash          (                    ) ;
   vTaskDelay              ( pdMS_TO_TICKS(2000 ));
   Soft_Reset              (                    ) ;
   return 0;
}
int Cmd_Pwd(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(P->tpcb,"%s\r\n", Usr_Flash_Params.Pwd);
   else {
      UART_ETHprintf(P->tpcb,"new pwd:%s \r\n",argv[1]);
      ustrncpy(Usr_Flash_Params.Pwd,argv[1],sizeof(Usr_Flash_Params.Pwd)-1);
      Save_Usr_Flash();
   }
   return 0;
}/*}}}*/
//STATS{{{
int Cmd_SysStats(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   SYS_STATS_DISPLAY();
   return 0;
}
int Cmd_MemStats(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   MEM_STATS_DISPLAY();
   return 0;
}
int Cmd_MempStats(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   uint8_t i;
   for (i = 0; i < MEMP_MAX; i++) {
      MEMP_STATS_DISPLAY(i);
   }
   return 0;
}
int Cmd_EthStats(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   ETHARP_STATS_DISPLAY();
   return 0;
}
int Cmd_IcmpStats(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   ICMP_STATS_DISPLAY();
   return 0;
}
int Cmd_AllStats(struct Parser_Queue_Struct* P, int argc, char *argv[])
{
   stats_display();
   return 0;
}
/*}}}*/
