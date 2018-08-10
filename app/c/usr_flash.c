#include <stdint.h>
#include "utils/lwiplib.h"
#include "utils/cmdline.h"
#include "utils/flash_pb.h"
#include "utils/uartstdio.h"
#include "opt.h"
#include "usr_flash.h"


struct Usr_Flash_Struct Usr_Flash_Params = {
      .Mac_Addr       = {0x12,0x34,0x56,0x78,0x9A,0xBC},
      .Ip_Addr        = 0xC0A8020A,
      .Mask_Addr      = 0xFFFFFF00,
      .Gateway_Addr   = 0xC0A80201,
      .Dhcp_Enable    = false,
      .Config_Port    = 49152,
      .Temp_Port      = 49153,
      .Snmp_Port      = 161,
      .Snmp_Community = "public",
      .Snmp_Iso       = {0x2b,0x06,0x01,0x02,0x01,0x21,0x01,0x02,0x07},
      .Snmp_Iso_Len   = 9,
      .Tmax           = 30,
      .Tmin           = 25,
      .Reload_T_TOut  = 2,
};


void Init_Usr_Flash(void)
{
   FlashPBInit(USR_FLASH_START,USR_FLASH_END,USR_FLASH_SIZE);
   UART_ETHprintf(UART_MSG,"flash inicializada\n");
   Get_Usr_Flash();
}
void Save_Usr_Flash(void)
{
   FlashPBSave(&Usr_Flash_Params.N);
   UART_ETHprintf(UART_MSG,"flash grabada\n");
}
void Get_Usr_Flash(void)
{
   uint8_t* New_Params=FlashPBGet();
   if(New_Params!=NULL) {
         Usr_Flash_Params=*(struct Usr_Flash_Struct*)New_Params;
         UART_ETHprintf(UART_MSG,"flash leida=%d\n",Usr_Flash_Params.N);
   }
   else
      UART_ETHprintf(UART_MSG,"flash no leida=%d\n",Usr_Flash_Params.N);
}

//-------------------------------------------------------------------------------------
//