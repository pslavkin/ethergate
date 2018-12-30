#ifndef PARSER
#define PARSER

struct Parser_Queue_Struct
{
   uint8_t Buff[APP_INPUT_BUF_SIZE];
   struct tcp_pcb* tpcb;
   uint32_t Id;
};
void Parser_Task(void* nil);
extern QueueHandle_t                Parser_Queue;
extern struct Parser_Queue_Struct   Actual_Cmd;

#endif

