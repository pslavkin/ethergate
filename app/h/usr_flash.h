#ifndef  USR_FLASH
#define  USR_FLASH


struct Usr_Flash_Struct {
   uint8_t     N                               ;
   uint8_t     Crc                             ;
   uint8_t     Mac_Addr      [ 8 ]             ;
   uint32_t    Ip_Addr                         ;
   uint32_t    Mask_Addr                       ;
   uint32_t    Gateway_Addr                    ;
   uint8_t     Dhcp_Enable                     ;
   uint16_t    Virtual_Port                    ;
   uint16_t    Config_Port                     ;
   uint16_t    Rs232_Port                      ;
   uint16_t    Sniffer_Port                    ;
   uint16_t    Temp_Port                       ;
   uint16_t    Snmp_Port                       ;
   char        Snmp_Community[ 20 ]            ;
   uint8_t     Snmp_Iso_Len [MAX_ROM_CODES]    ;
   uint8_t     Snmp_Iso[ MAX_ROM_CODES][ 20 ]  ;
   uint8_t     Sensor_Codes[MAX_ROM_CODES][ 8 ];
   float       Tmax                            ;
   float       Tmin                            ;
   uint16_t    Reload_T_TOut                   ;
   char        Id[ 20 ]                        ;
   char        Pwd[ 10 ]                       ;
   uint32_t    Wdog                            ;
   uint16_t    Rs232_Len                       ;
   uint16_t    Rs232_Tout                      ;
   uint16_t    Rs232_Term                      ;
   uint32_t    Rs232_Baud                      ;
   bool        Rs232_Menu_Enable               ;
};
extern struct Usr_Flash_Struct Usr_Flash_Params;

void Init_Usr_Flash                 ( void );
extern void Usr_Flash2Defult_Values ( void );
extern void Save_Usr_Flash          ( void );
extern void Get_Usr_Flash           ( void );

#endif

