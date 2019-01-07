#ifndef  TELNET
#define  TELNET

typedef enum TPCB_TYPE {
   SNIFF   = 0x01,
   VIRTUAL = 0x02,
   NORMAL  = 0x04,
} Tpcb_Type_T;

void     Init_Telnet           ( void                                                             );
void     Telnet_Close          ( struct tcp_pcb *tpcb                                             );
err_t    Rcv_Rs232_Fn          ( void *arg     ,struct tcp_pcb *tpcb   ,struct pbuf *p ,err_t err );
err_t    Accept_Rs232_Fn       ( void *arg     ,struct tcp_pcb *newpcb ,err_t err                 );
void     Create_Rs232_Socket   ( void* nil                                                        );
bool     Send_To_Normal_Tcp    ( uint8_t* Data, uint16_t Len                                      );
bool     Send_To_Virtual_Tcp   ( uint8_t* Data, uint16_t Len                                      );
bool     Send_To_Conn_Tcp      ( uint8_t* Data, uint16_t Len,Tpcb_Type_T Type                     );
bool     Is_Any_Conn           ( void                                                             );
bool     Free_Conn             ( struct tcp_pcb* New                                              );
void     Init_Conn             ( void                                                             );
bool     Add_Conn              ( struct tcp_pcb* New, Tpcb_Type_T Type                            );
void     Create_Sniffer_Socket ( void* nil                                                        );
err_t    Accept_Sniffer_Fn     ( void *arg, struct tcp_pcb *newpcb, err_t err                     );
err_t    Rcv_Sniffer_Fn        ( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err       );
void     Create_Virtual_Socket ( void* nil                                                        );
err_t    Accept_Virtual_Fn     ( void *arg, struct tcp_pcb *newpcb, err_t err                     );
err_t    Rcv_Virtual_Fn        ( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err       );

#endif

