#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "utils/lwiplib.h"
#include "opt.h"
#include "udp.h"
#include "snmp_agent.h"
#include "state_machine.h"
#include "one_wire_network.h"
#include "usr_flash.h"

struct udp_pcb* usoc;
void Create_Udp_Socket(void* nil);
//-------------------------------------------------------------------------------------
void Init_Udp(void)
{
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
   udp_bind     ( usoc ,IP_ADDR_ANY ,Usr_Flash_Params.Snmp_Port );
   udp_recv     ( usoc ,URcv_Fn     ,NULL  );
}
