#ifndef __OPT_H__
#define __OPT_H__

//#define DEBUG_UART

#define APP_INPUT_BUF_SIZE      256
#define UART_BUFFERED
#define UART_RX_BUFFER_SIZE     1024
#define UART_TX_BUFFER_SIZE     1024 //(3*TCP_MSS+1)   //igual al buffer (mas uno porque es circular) de lwip para que un paquete de tcp pueda caber justo dentro de la cola de transmision
#define PARSER_QUEUE_SIZE       10

#define USR_FLASH_START 0x00020000                          //donde te plazca que no pise codigo. y que este alineado a 4
#define USR_FLASH_END   0x00028000                          // 2 bloques de 16 minimo para garantizar lectura
#define USR_FLASH_SIZE  (USR_FLASH_END-USR_FLASH_START)/128 // no puede haber mas de 128 bloques.. y como cada pedazo borrable de flas es de 16k. 32k/128 son 256 bytes por cada estructura de parametros.. por ahora creo que no se que poner que ocupe 256 bytes.. asi que sobra.. sino se baja este numer y chau

#define ONE_WIRE_RX_BUFFER 35 // es tambien el tamaño maximo de algun comando...generalmente no pasa de 20, que es un match ROM + read Page
#define MAX_ROM_CODES       2  // cantidad de nodos permitidos en la red..
#define MAX_FAIL_CODES     20 // cantidad de veces que falla intentando leer el numero de codigo en la busqueda de codigos... usualmente falla cuando el cableado es malo y lee cualquier cosa. entonces con esto se puede reportar el problema...

#define TCP_REGISTERED_LIST 3 //cuandtas conexiones se permiten registrar para eth<>232 para que se reenvien a todas

#endif // __COMMANDS_H__
