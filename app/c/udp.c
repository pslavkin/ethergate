#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/rom_map.h"
#include "utils/lwiplib.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/cmdline.h"
#include "drivers/pinout.h"
#include "io.h"
#include "commands.h"
#include "opt.h"
#include "commands.h"
#include "udp.h"
#include "snmp_agent.h"

struct udp_pcb* usoc;
void Create_Udp_Socket(void* nil);
//-------------------------------------------------------------------------------------
void Init_Udp(void)
{
   Init_Snmp_Agent ();
   tcpip_callback(Create_Udp_Socket,0);
}

void URcv_Fn (void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t* addr,  u16_t port)
{
   if(p!=NULL) {
      Snmp_Packet_Arrived(upcb,p,addr,port);
   }
}
void Create_Udp_Socket(void* nil)
{
   usoc=udp_new (                          );
   udp_bind     ( usoc ,IP_ADDR_ANY ,161 );
   udp_recv     ( usoc ,URcv_Fn     ,NULL  );
}
