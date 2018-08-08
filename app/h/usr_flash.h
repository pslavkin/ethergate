#ifndef  USR_FLASH
#define  USR_FLASH

struct Usr_Flash_Struct {
   uint8_t n;
   uint8_t crc;
   uint8_t Mac_Addr   [ 8 ];
   uint32_t Ip_Addr     ;
   uint32_t Mask_Addr   ;
   uint32_t Gateway_Addr;
   uint16_t Config_Port;
   uint16_t Temp_Port;
   uint16_t Snmp_Port;
   uint8_t Dhcp_Enable;
};
extern struct Usr_Flash_Struct Usr_Flash_Params;

void Init_Usr_Flash(void);
extern void Save_Usr_Flash(void);
extern void Get_Usr_Flash(void);

#endif

