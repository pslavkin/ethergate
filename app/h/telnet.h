#ifndef  TELNET
#define  TELNET

extern void       Init_Telnet  ( void );
extern void       Telnet_Close ( struct tcp_pcb *tpcb);
err_t Rcv_Rs232_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t Accept_Rs232_Fn (void *arg, struct tcp_pcb *newpcb, err_t err);
void Create_Rs232_Socket(void* nil);
bool Register_Rs232_Conn(struct tcp_pcb* New);
bool Free_Rs232_Conn(struct tcp_pcb* New);
bool Send_To_All_Tcp(uint8_t* Data, uint16_t Len);
void Init_Rs232_Conn(void);
bool Is_Any_Connection_Registered(void);
#endif
