#ifndef __LWIPLIB_H__
#define __LWIPLIB_H__

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
// lwIP Options
//
//*****************************************************************************
#include "lwip/opt.h"

//*****************************************************************************
//
// Ensure that AUTOIP COOP option is configured correctly.
//
//*****************************************************************************
#undef LWIP_DHCP_AUTOIP_COOP
#define LWIP_DHCP_AUTOIP_COOP   ((LWIP_DHCP) && (LWIP_AUTOIP))

//*****************************************************************************
//
// lwIP API Header Files
//
//*****************************************************************************
#include <stdint.h>
#include "lwip/api.h"
#include "lwip/netifapi.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
#include "lwip/def.h"
#include "lwip/tcp_impl.h"
#include "lwip/timers.h"

//*****************************************************************************
//
// IP Address Acquisition Modes
//
//*****************************************************************************
#define IPADDR_USE_STATIC       0
#define IPADDR_USE_DHCP         1
#define IPADDR_USE_AUTOIP       2

//*****************************************************************************
//
// Hardware timer interrupt callback function type (available only when running
// on TM4C parts).  This function is called in interrupt context whenever the
// Ethernet MAC reports an interrupt from the IEEE-1588 timestamping
// timer.  The first parameter is the base address of the MAC and the second
// is the interrupt status as reported via EthMACTimestampIntStatus.
//
//*****************************************************************************
typedef void (* tHardwareTimerHandler)(uint32_t ui32Base,
                                       uint32_t ui32IntStatus);

//*****************************************************************************
//
// lwIP Abstraction Layer API
//
//*****************************************************************************
extern void lwIPInit(uint32_t ui32SysClkHz, const uint8_t *pui8Mac,
                     uint32_t ui32IPAddr, uint32_t ui32NetMask,
                     uint32_t ui32GWAddr, uint32_t ui32IPMode);
extern void lwIPTimerCallbackRegister(tHardwareTimerHandler pfnTimerFunc);
extern void lwIPTimer(uint32_t ui32TimeMS);
extern void lwIPEthernetIntHandler(void);
extern uint32_t lwIPLocalIPAddrGet(void);
extern uint32_t lwIPLocalNetMaskGet(void);
extern uint32_t lwIPLocalGWAddrGet(void);
extern void lwIPLocalMACGet(uint8_t *pui8Mac);
extern void lwIPNetworkConfigChange(uint32_t ui32IPAddr, uint32_t ui32NetMask,
                                    uint32_t ui32GWAddr, uint32_t ui32IPMode);
extern uint32_t lwIPAcceptUDPPort(uint16_t ui16Port);
extern bool g_bLinkActive;

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __LWIPLIB_H__
