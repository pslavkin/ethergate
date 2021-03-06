#ifndef PARSER
#define PARSER

struct Cmd_Table_Struct;
struct Parser_Queue_Struct
{
   uint8_t                       Buff[APP_INPUT_BUF_SIZE];
   uint16_t                      Index;
   uint16_t                      lastIndex;
   struct tcp_pcb*               tpcb;
   struct Cmd_Table_Struct*      CmdTable;
   uint32_t                      Id;
};

void                                Parser_Task(void* nil);
extern QueueHandle_t                Parser_Queue;
extern struct Parser_Queue_Struct   Actual_Cmd;

#endif

