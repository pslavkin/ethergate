#ifndef ONE_WIRE_LINK
#define ONE_WIRE_LINK

#define READ_ROM_STRING    "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x33"
#define SKIP_ROM_STRING    "\xCC"
#define SEARCH_ROM_STRING  "\xF0"

//con un solo pin
//#define ONE_WIRE_PIN        GPIO_PIN_7
//#define ONE_WIRE_PORT       GPIO_PORTA_BASE
//#define ONE_WIRE_PERIPH     SYSCTL_PERIPH_GPIOA

//usando los 5 pines como one-wire para darle duro cuando tiene que medit T. sino con un solo pin apenitas si baja de los 3v.. quien sabe cada tanto me estaba danto una lectura de 85.. peor no se si es por eso...
#define ONE_WIRE_PIN     (GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4)
#define ONE_WIRE_PORT    GPIO_PORTF_BASE
#define ONE_WIRE_PERIPH  SYSCTL_PERIPH_GPIOF

enum One_Wire_Link_Commands
{
 READ_ROM   = 0x33,
 SKIP_ROM   = 0xCC,
 MATCH_ROM  = 0x55,
 SEARCH_ROM = 0xF0
};
enum One_Wire_DS18S20_Network_Command
{
 READ_SCRATCHPAD   = 0xBE,
 WRITE_SCRATCHPAD  = 0x4E,
 COPY_SCRATCHPAD   = 0x48,
 RECALL_E2         = 0xB8,
 READ_POWER_SUPPLY = 0xB4,
 CONVERT_T         = 0x44
};
enum One_Wire_DS1990_Network_Command
{
 READ_MEMORY          = 0xF0,
 EXTENDED_READ_MEMORY = 0xA5,
 READ_SUBKEY          = 0xA5,
 WRITE_SUBKEY         = 0x99,
 WRITE_PASWSWORD      = 0x5A,
 WRITE_MEMORY         = 0x0F,
 WRITE_STATUS         = 0x55,
 READ_STATUS          = 0xAA
};


#define One_Wire_PORT 1

// ------------------------------------------------------
void     Init_One_Wire_Link      ( void         );
void     One_Wire_Power_On_Reset ( void         );
bool     Presence                ( void         );
uint8_t  Write_Bit_One_And_Read  ( void         );
uint8_t  Write_Bit_Zero          ( void         );
uint8_t  Write_Read_Bit          ( uint8_t Bit  );
uint8_t  Read2Bits               ( void         );
uint8_t  Write_Read_Byte         ( uint8_t Data );
//------------------------------------------------------
#endif
