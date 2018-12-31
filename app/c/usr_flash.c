#include <stdbool.h>
#include <stdint.h>
#include "utils/lwiplib.h"
#include "utils/cmdline.h"
#include "utils/flash_pb.h"
#include "utils/uartstdio.h"
#include "opt.h"
#include "state_machine.h"
#include "one_wire_network.h"
#include "usr_flash.h"


struct Usr_Flash_Struct Default_Usr_Flash_Params = {
      .Mac_Addr       = {0x00,0x15,0xA5,0x5D,0x03,0xE8},
      .Ip_Addr        = 0xC0A8020A,
      .Mask_Addr      = 0xFFFFFF00,
      .Gateway_Addr   = 0xC0A80201,
      .Dhcp_Enable    = false,
      .Rs232_Port     = 49154,
      .Config_Port    = 49152,
      .Temp_Port      = 49153,
      .Snmp_Port      = 161,
      .Snmp_Community = "public",
      .Snmp_Iso_Len   = 10,
      .Snmp_Iso       = {{0x2b,0x06,0x01,0x02,0x01,0x21,0x01,0x02,0x07,0},
                         {0x2b,0x06,0x01,0x02,0x01,0x21,0x01,0x02,0x07,1}},
      .Sensor_Codes   = {{0x43,0x00,0x00,0x07,0xF5,0x98,0xB3,0x28},
                         {0xD0,0x00,0x00,0x07,0xF5,0x24,0x1B,0x28}},
      .Tmax           = 30,
      .Tmin           = 25,
      .Reload_T_TOut  = 2,
      .Id             = "ehergate_12345678",
      .Pwd            = "1234",
      .Wdog           = 0,
      .Hang_Times     = 0,      // solo para debug del cuelgue por MAC sorda
      .Rs232_Len      = APP_INPUT_BUF_SIZE,   // longitud en donde corta la trama 232 (tiene que ser menor que el largo del buffer limitar al recibir
      .Rs232_Tout     =  5,     // en slots de 100msegs (10=1seg)
      .Rs232_Term     = 0xFFFF, // '\r', // caracter que corta la trama 232 (es un entero 16 para poder invalidarlo poniendo un numero mayor a 255
      .Rs232_Baud     = 115200, //baud rate para puerto 232
      .Rs232_Menu_Enable = true, //permite entrar al menu de configuracion por rs232 siempre qu eno este ninguna conexion abierta
};
struct Usr_Flash_Struct Usr_Flash_Params;


void Init_Usr_Flash(void)
{
   FlashPBInit(USR_FLASH_START,USR_FLASH_END,USR_FLASH_SIZE);
   UART_ETHprintf(DEBUG_MSG,"flash inicializada\n");
   Usr_Flash2Defult_Values();
   Get_Usr_Flash();
}
void Usr_Flash2Defult_Values(void)
{
   Usr_Flash_Params=Default_Usr_Flash_Params;
}
void Save_Usr_Flash(void)
{
   FlashPBSave(&Usr_Flash_Params.N);
   UART_ETHprintf(DEBUG_MSG,"flash grabada\n");
}
void Get_Usr_Flash(void)
{
   uint8_t* New_Params=FlashPBGet();
   if(New_Params!=NULL) {
         Usr_Flash_Params=*(struct Usr_Flash_Struct*)New_Params;
         UART_ETHprintf(DEBUG_MSG,"flash leida=%d\n",Usr_Flash_Params.N);
   }
   else
      UART_ETHprintf(DEBUG_MSG,"flash no leida=%d\n",Usr_Flash_Params.N);
}

//-------------------------------------------------------------------------------------
//
