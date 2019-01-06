#ifndef  TELNET
#define  TELNET

void     Init_Telnet                  ( void                                                             );
void     Telnet_Close                 ( struct tcp_pcb *tpcb                                             );
err_t    Rcv_Rs232_Fn                 ( void *arg     ,struct tcp_pcb *tpcb   ,struct pbuf *p ,err_t err );
err_t    Accept_Rs232_Fn              ( void *arg     ,struct tcp_pcb *newpcb ,err_t err                 );
void     Create_Rs232_Socket          ( void* nil                                                        );
bool     Send_To_All_Tcp              ( uint8_t* Data ,uint16_t Len                                      );
bool Send_To_Conn_Tcp(uint8_t* Data, uint16_t Len,bool Sniff);
bool     Is_Any_Conn ( void );
bool Free_Conn(struct tcp_pcb* New);
void Init_Conn(void);
bool Add_Conn(struct tcp_pcb* New, bool Sniff);
void Create_Sniffer_Socket(void* nil);
err_t Accept_Sniffer_Fn (void *arg, struct tcp_pcb *newpcb, err_t err);
err_t Rcv_Sniffer_Fn(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

#endif

