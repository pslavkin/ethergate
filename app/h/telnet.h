#ifndef  TELNET
#define  TELNET

typedef enum TPCB_TYPE {
   SNIFF   = 0x01,
   VIRTUAL = 0x02,
   NORMAL  = 0x04,
} Tpcb_Type_T;

enum Telnet_Events {
    Telnet_Enable_Event = 0,
    Telnet_Disable_Event,
    Telnet_Connect_Event,
    Telnet_Disconnect_Event
   };

struct Soc_Clients_Struct {
   const State      *C_State; //tiene que estar primero porque lo uso ademas como puntero a la estructira con un cast
   struct tcp_pcb*  tpcb;
   struct ip_addr   Ip;
   uint16_t         Port;
   uint16_t         Tout;
   uint16_t         Actual_Tout;
   err_t            (*Accept_Fn) (void *arg, struct tcp_pcb *newpcb, err_t err);
};


void  Init_Telnet            ( void                                                                                           );
void  Telnet_Close           ( struct   tcp_pcb              *tpcb                                                            );
err_t Rcv_Rs232_Fn           ( void     *arg                     ,struct   tcp_pcb      *tpcb   ,struct pbuf    *p ,err_t err );
err_t Accept_Rs232_Fn        ( void     *arg                     ,struct   tcp_pcb      *newpcb ,err_t  err                   );
void  Create_Rs232_Socket    ( void*    nil                                                                                   );
bool  Send_To_Normal_Tcp     ( uint8_t* Data                     ,uint16_t Len                                                );
bool  Send_To_Virtual_Tcp    ( uint8_t* Data                     ,uint16_t Len                                                );
bool  Send_To_Conn_Tcp       ( uint8_t* Data                     ,uint16_t Len                  ,Tpcb_Type_T Type             );
bool  Is_Any_Conn            ( void                                                                                           );
bool  Free_Conn              ( struct   tcp_pcb*             New                                                              );
void  Init_Conn              ( void                                                                                           );
bool  Add_Conn               ( struct   tcp_pcb*             New ,Tpcb_Type_T Type                                            );
void  Create_Sniffer_Socket  ( void*    nil                                                                                   );
err_t Accept_Sniffer_Fn      ( void     *arg                     ,struct   tcp_pcb      *newpcb ,err_t  err                   );
err_t Rcv_Sniffer_Fn         ( void     *arg                     ,struct   tcp_pcb      *tpcb   ,struct pbuf    *p ,err_t err );
void  Create_Virtual_Socket  ( void*    nil                                                                                   );
err_t Accept_Virtual_Fn      ( void     *arg                     ,struct   tcp_pcb      *newpcb ,err_t  err                   );
err_t Rcv_Virtual_Fn         ( void     *arg                     ,struct   tcp_pcb      *tpcb   ,struct pbuf    *p ,err_t err );
int   Cmd_Connect            ( struct   Parser_Queue_Struct* P   ,int         argc              ,char   *argv[]               );
int   Cmd_Print_Clients_List ( struct   Parser_Queue_Struct* P   ,int         argc              ,char   *argv[]               );
int   Cmd_Init_Client_List   ( struct   Parser_Queue_Struct* P   ,int         argc              ,char   *argv[]               );
void  Telnet_Clients_Rti     ( void                                                                                           );

#endif

