#include <stdint.h>
#include <stdbool.h>
#include "utils/lwiplib.h"
#include "leds.h"
#include "opt.h"
#include "usr_flash.h"
#include "state_machine.h"

//*****************************************************************************
// Ensure that ICMP checksum offloading is enabled; otherwise the TM4C129
// driver will not operate correctly.
//*****************************************************************************
#define LWIP_OFFLOAD_ICMP_CHKSUM 1
//*****************************************************************************
// Include lwIP high-level API code.
//*****************************************************************************
#include "third_party/lwip-1.4.1/src/api/api_lib.c"
#include "third_party/lwip-1.4.1/src/api/api_msg.c"
#include "third_party/lwip-1.4.1/src/api/err.c"
#include "third_party/lwip-1.4.1/src/api/netbuf.c"
#include "third_party/lwip-1.4.1/src/api/netdb.c"
#include "third_party/lwip-1.4.1/src/api/netifapi.c"
#include "third_party/lwip-1.4.1/src/api/sockets.c"
#include "third_party/lwip-1.4.1/src/api/tcpip.c"

//*****************************************************************************
// Include the core lwIP TCP/IP stack code.
//*****************************************************************************
#include "third_party/lwip-1.4.1/src/core/def.c"
#include "third_party/lwip-1.4.1/src/core/dhcp.c"
#include "third_party/lwip-1.4.1/src/core/dns.c"
#include "third_party/lwip-1.4.1/src/core/init.c"
#include "third_party/lwip-1.4.1/src/core/mem.c"
#include "third_party/lwip-1.4.1/src/core/memp.c"
#include "third_party/lwip-1.4.1/src/core/netif.c"
#include "third_party/lwip-1.4.1/src/core/pbuf.c"
#include "third_party/lwip-1.4.1/src/core/raw.c"
#include "third_party/lwip-1.4.1/src/core/stats.c"
#include "third_party/lwip-1.4.1/src/core/sys.c"
#include "third_party/lwip-1.4.1/src/core/tcp.c"
#include "third_party/lwip-1.4.1/src/core/tcp_in.c"
#include "third_party/lwip-1.4.1/src/core/tcp_out.c"
#include "third_party/lwip-1.4.1/src/core/timers.c"
#include "third_party/lwip-1.4.1/src/core/udp.c"

//*****************************************************************************
// Include the IPV4 code.
//*****************************************************************************
#include "third_party/lwip-1.4.1/src/core/ipv4/autoip.c"
#include "third_party/lwip-1.4.1/src/core/ipv4/icmp.c"
#include "third_party/lwip-1.4.1/src/core/ipv4/igmp.c"
#include "third_party/lwip-1.4.1/src/core/ipv4/inet.c"
#include "third_party/lwip-1.4.1/src/core/ipv4/inet_chksum.c"
#include "third_party/lwip-1.4.1/src/core/ipv4/ip.c"
#include "third_party/lwip-1.4.1/src/core/ipv4/ip_addr.c"
#include "third_party/lwip-1.4.1/src/core/ipv4/ip_frag.c"

//*****************************************************************************
// Include the IPV6 code.
// Note:  Code is experimental and not ready for use.
// References are included for completeness.
//*****************************************************************************
#if 0
#include "third_party/lwip-1.4.1/src/core/ipv6/icmp6.c"
#include "third_party/lwip-1.4.1/src/core/ipv6/inet6.c"
#include "third_party/lwip-1.4.1/src/core/ipv6/ip6.c"
#include "third_party/lwip-1.4.1/src/core/ipv6/ip6_addr.c"
#endif

//*****************************************************************************
// Include the lwIP SNMP code.
//*****************************************************************************
#if 0
#include "third_party/lwip-1.4.1/src/core/snmp/asn1_dec.c"
#include "third_party/lwip-1.4.1/src/core/snmp/asn1_enc.c"
#include "third_party/lwip-1.4.1/src/core/snmp/mib2.c"
#include "third_party/lwip-1.4.1/src/core/snmp/mib_structs.c"
#include "third_party/lwip-1.4.1/src/core/snmp/msg_in.c"
#include "third_party/lwip-1.4.1/src/core/snmp/msg_out.c"
#endif
//*****************************************************************************
// Include the network interface code.
//*****************************************************************************
#include "third_party/lwip-1.4.1/src/netif/etharp.c"
//*****************************************************************************
// Include the network interface PPP code.
//*****************************************************************************
#if 0
#include "third_party/lwip-1.4.1/src/netif/ppp/auth.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/chap.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/chpms.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/fsm.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/ipcp.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/lcp.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/magic.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/md5.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/pap.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/ppp.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/ppp_oe.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/randm.c"
#include "third_party/lwip-1.4.1/src/netif/ppp/vj.c"
#endif

//*****************************************************************************
// Include Tiva-specific lwIP interface/porting layer code.
//*****************************************************************************
#include "third_party/lwip-1.4.1/ports/tiva-tm4c129/perf.c"
#include "third_party/lwip-1.4.1/ports/tiva-tm4c129/sys_arch.c"
#include "third_party/lwip-1.4.1/ports/tiva-tm4c129/netif/tiva-tm4c129.c"
//*****************************************************************************
// The link detect polling interval.
//*****************************************************************************
#define LINK_TMR_INTERVAL       10
//*****************************************************************************
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "driverlib/debug.h"
#include "driverlib/emac.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//*****************************************************************************
// The lwIP network interface structure for the Tiva Ethernet MAC.
//*****************************************************************************
static struct netif g_sNetIF;
//*****************************************************************************
// The most recently detected link state.
//*****************************************************************************
uint8_t Link_State = false;
//*****************************************************************************
// The stack size for the interrupt task.
//*****************************************************************************
#define STACKSIZE_LWIPINTTASK   1024//256 //128
//*****************************************************************************
// The handle for the "queue" (semaphore) used to signal the interrupt task
// from the interrupt handler.
//*****************************************************************************
static xQueueHandle g_pInterrupt;
//*****************************************************************************
// This task handles reading packets from the Ethernet controller and supplying
// them to the TCP/IP thread.
//*****************************************************************************
static void lwIPInterruptTask(void *pvArg)
{
   while(1) {
      while(xQueueReceive  ( g_pInterrupt, &pvArg, pdMS_TO_TICKS(500))==pdFALSE) { //portMAX_DELAY ) {

         xSemaphoreGive ( Led_Link_Semphr                     );
      }
       xSemaphoreGive ( Led_Link_Semphr                     );
      tivaif_interrupt(&g_sNetIF, (uint32_t)pvArg);                         // Processes any packets waiting to be sent or received.
      MAP_EMACIntEnable(EMAC0_BASE, (EMAC_INT_RECEIVE | EMAC_INT_TRANSMIT | // Re-enable the Ethernet interrupts.
               EMAC_INT_TX_STOPPED |
               EMAC_INT_RX_NO_BUFFER |
               EMAC_INT_RX_STOPPED | EMAC_INT_PHY));
   }
}
//*****************************************************************************
// This function performs a periodic check of the link status and responds
// appropriately if it has changed.
//*****************************************************************************
bool Read_Link_State(void)
{
   return Link_State&0x01;
}
static void lwIPLinkDetect(void)
{
   struct ip_addr ip_addr  ;
   struct ip_addr net_mask ;
   struct ip_addr gw_addr  ;
   Link_State <<= 1;
   Link_State  &= 0x03;
   Link_State  |= (MAP_EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_BMSR) & EPHY_BMSR_LINKSTAT)?
                  1:0;

   switch(Link_State) {
      case 0x00:                                // sigue apagado
         break;
      case 0x01:                                // se prendio
         if(Usr_Flash_Params.Dhcp_Enable==true) // Start DHCP, if enabled.
            dhcp_start ( &g_sNetIF );
         else {
            ip_addr.addr  = htonl ( Usr_Flash_Params.Ip_Addr      );
            net_mask.addr = htonl ( Usr_Flash_Params.Mask_Addr    );
            gw_addr.addr  = htonl ( Usr_Flash_Params.Gateway_Addr );
            netif_set_addr(&g_sNetIF, &ip_addr, &net_mask, &gw_addr);
         }
         break;
      case 0x02:                                // se apago
         ip_addr.addr  = 0;
         net_mask.addr = 0;
         gw_addr.addr  = 0;
         netif_set_addr(&g_sNetIF, &ip_addr, &net_mask, &gw_addr);
         if(Usr_Flash_Params.Dhcp_Enable==true) // Stop DHCP, if enabled.
            dhcp_stop(&g_sNetIF);
         break;
      case 0x03:                                // sigue prendida
         break;
   }
}
//*****************************************************************************
// Handles the timeout for the link detect timer when using a RTOS.
//*****************************************************************************
static void lwIPPrivateLinkTimer(void *pvArg)
{
   lwIPLinkDetect();                                           // Perform the link detection.
   sys_timeout(LINK_TMR_INTERVAL, lwIPPrivateLinkTimer, NULL); // Re-schedule the link detect timer timeout.
}
//*****************************************************************************
// Completes the initialization of lwIP.  This is directly called when not
// using a RTOS and provided as a callback to the TCP/IP thread when using a
// RTOS.
//*****************************************************************************
static void lwIPPrivateInit(void *pvArg)
{
    struct ip_addr ip_addr;
    struct ip_addr net_mask;
    struct ip_addr gw_addr;
    // If using a RTOS, create a queue (to be used as a semaphore) to signal
    // the Ethernet interrupt task from the Ethernet interrupt handler.
    g_pInterrupt = xQueueCreate(3, sizeof(void *));
    xTaskCreate(lwIPInterruptTask, (portCHAR *)"eth_int",
                STACKSIZE_LWIPINTTASK, 0, tskIDLE_PRIORITY + 2, //estaba en +1 lo paso a +3
                0);
    ip_addr.addr  = 0; //arranco en cero, el lwIPPrivateHostTimer se encargara de configurar cuando se linkee
    net_mask.addr = 0;
    gw_addr.addr  = 0;
    netif_add         ( &g_sNetIF, &ip_addr, &net_mask, &gw_addr, NULL, tivaif_init, tcpip_input );
    netif_set_default ( &g_sNetIF                                                                );
    netif_set_up      ( &g_sNetIF                                                                ); // Bring the interface up.
    //sys_timeout(HOST_TMR_INTERVAL, lwIPPrivateHostTimer, NULL);
    sys_timeout(LINK_TMR_INTERVAL, lwIPPrivateLinkTimer, NULL);                                     // Setup a timeout for the link detect callback function if using a RTOS.
}

//*****************************************************************************
//! Initializes the lwIP TCP/IP stack.
//! \param ui32SysClkHz is the current system clock rate in Hz.
//! \param pui8MAC is a pointer to a six byte array containing the MAC
//! address to be used for the interface.
//! \param ui32IPAddr is the IP address to be used (static).
//! \param ui32NetMask is the network mask to be used (static).
//! \param ui32GWAddr is the Gateway address to be used (static).
//! \param ui32IPMode is the IP Address Mode.  \b IPADDR_USE_STATIC will force
//! static IP addressing to be used, \b IPADDR_USE_DHCP will force DHCP with
//! fallback to Link Local (Auto IP), while \b IPADDR_USE_AUTOIP will force
//! Link Local only.
//!
//! This function performs initialization of the lwIP TCP/IP stack for the
//! Ethernet MAC, including DHCP and/or AutoIP, as configured.
//!
//! \return None.
//
//*****************************************************************************
void lwIPInit(uint32_t ui32SysClkHz, const uint8_t *pui8MAC)
{
    MAP_SysCtlPeripheralEnable ( SYSCTL_PERIPH_EMAC0 );      // Enable the ethernet peripheral.
    MAP_SysCtlPeripheralReset  ( SYSCTL_PERIPH_EMAC0 );
    MAP_SysCtlPeripheralEnable ( SYSCTL_PERIPH_EPHY0 );
    MAP_SysCtlPeripheralReset  ( SYSCTL_PERIPH_EPHY0 );
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0))   // Wait for the MAC to come out of reset.
       ;
    EMACPHYConfigSet (EMAC0_BASE, EMAC_PHY_CONFIG);          // Configure for use with whichever PHY the user requires.
    MAP_EMACInit     (EMAC0_BASE, ui32SysClkHz,              // Initialize the MAC and set the DMA mode.
                      EMAC_BCONFIG_MIXED_BURST | EMAC_BCONFIG_PRIORITY_FIXED,
                      4, 4, 0);
    MAP_EMACConfigSet(EMAC0_BASE, (EMAC_CONFIG_FULL_DUPLEX | // Set MAC configuration options.
                                   EMAC_CONFIG_CHECKSUM_OFFLOAD |
                                   EMAC_CONFIG_7BYTE_PREAMBLE |
                                   EMAC_CONFIG_IF_GAP_96BITS |
                                   EMAC_CONFIG_USE_MACADDR0 |
                                   EMAC_CONFIG_SA_FROM_DESCRIPTOR |
                                   EMAC_CONFIG_BO_LIMIT_1024),
                                   (EMAC_MODE_RX_STORE_FORWARD |
                                    EMAC_MODE_TX_STORE_FORWARD |
                                    EMAC_MODE_TX_THRESHOLD_64_BYTES |
                                    EMAC_MODE_RX_THRESHOLD_64_BYTES), 0);
    MAP_EMACAddrSet ( EMAC0_BASE, 0, (uint8_t * )pui8MAC);   // Program the hardware with its MAC address (for filtering).
    tcpip_init      ( lwIPPrivateInit, 0        )        ;   // Initialize lwIP.  The remainder of initialization is deferred to the TCP/IP thread's context if
}
//*****************************************************************************
//
//! Handles Ethernet interrupts for the lwIP TCP/IP stack.
//!
//! This function handles Ethernet interrupts for the lwIP TCP/IP stack.  At
//! the lowest level, all receive packets are placed into a packet queue for
//! processing at a higher level.  Also, the transmit packet queue is checked
//! and packets are drained and transmitted through the Ethernet MAC as needed.
//! If the system is configured without an RTOS, additional processing is
//! performed at the interrupt level.  The packet queues are processed by the
//! lwIP TCP/IP code, and lwIP periodic timers are serviced (as needed).
//!
//! \return None.
//
//*****************************************************************************
void lwIPEthernetIntHandler(void)
{
    uint32_t ui32Status;
    portBASE_TYPE xWake;

    ui32Status = EMACIntStatus(EMAC0_BASE, true); // Read and Clear the interrupt.

    // If the PMT mode exit status bit is set then enable the MAC transmit
    // and receive paths, read the PMT status to clear the interrupt and
    // clear the interrupt flag.
    if(ui32Status & EMAC_INT_POWER_MGMNT) {
        MAP_EMACTxEnable(EMAC0_BASE);
        MAP_EMACRxEnable(EMAC0_BASE);
        EMACPowerManagementStatusGet(EMAC0_BASE);
        ui32Status &= ~(EMAC_INT_POWER_MGMNT);
    }
    // If the interrupt really came from the Ethernet and not our timer, clear it.
    if(ui32Status) {
        MAP_EMACIntClear(EMAC0_BASE, ui32Status);
    }
    xQueueSendFromISR(g_pInterrupt, (void *)&ui32Status, &xWake);
    // Disable the Ethernet interrupts.  Since the interrupts have not been
    // handled, they are not asserted.  Once they are handled by the Ethernet
    // interrupt task, it will re-enable the interrupts.
    MAP_EMACIntDisable(EMAC0_BASE, (EMAC_INT_RECEIVE | EMAC_INT_TRANSMIT |
                                    EMAC_INT_TX_STOPPED |
                                    EMAC_INT_RX_NO_BUFFER |
                                    EMAC_INT_RX_STOPPED | EMAC_INT_PHY));
    portYIELD_FROM_ISR(xWake);
}

//*****************************************************************************
//! Returns the IP address for this interface.
//!
//! This function will read and return the currently assigned IP address for
//! the Stellaris Ethernet interface.
//!
//! \return Returns the assigned IP address for this interface.
//*****************************************************************************
uint32_t lwIPLocalIPAddrGet(void)
{
    if(Link_State==true || Usr_Flash_Params.Dhcp_Enable==false) {
        return((uint32_t)g_sNetIF.ip_addr.addr);
    }
    else
        return(0xffffffff);
}
//*****************************************************************************
//! Returns the network mask for this interface.
//!
//! This function will read and return the currently assigned network mask for
//! the Stellaris Ethernet interface.
//!
//! \return the assigned network mask for this interface.
//*****************************************************************************
uint32_t lwIPLocalNetMaskGet(void)
{
    return((uint32_t)g_sNetIF.netmask.addr);
}
//*****************************************************************************
//! Returns the gateway address for this interface.
//!
//! This function will read and return the currently assigned gateway address
//! for the Stellaris Ethernet interface.
//!
//! \return the assigned gateway address for this interface.
//*****************************************************************************
uint32_t lwIPLocalGWAddrGet(void)
{
    return((uint32_t)g_sNetIF.gw.addr);
}
//*****************************************************************************
//! Returns the local MAC/HW address for this interface.
//!
//! \param pui8MAC is a pointer to an array of bytes used to store the MAC
//! address.
//!
//! This function will read the currently assigned MAC address into the array
//! passed in \e pui8MAC.
//!
//! \return None.
//*****************************************************************************
void lwIPLocalMACGet(uint8_t *pui8MAC)
{
    MAP_EMACAddrGet(EMAC0_BASE, 0, pui8MAC);
}
//*****************************************************************************
