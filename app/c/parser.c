#include <stdint.h>
#include "utils/cmdline.h"
#include "opt.h"
#include "commands.h"
#include "parser.h"
#include "utils/uartstdio.h"

QueueHandle_t Parser_Queue;
struct Parser_Queue_Struct Cmd;

void Parser_Task(void* nil)
{
   Parser_Queue= xQueueCreate(PARSER_QUEUE_SIZE,sizeof(struct Parser_Queue_Struct));
   while(1) {
      xQueueReceive(Parser_Queue,&Cmd,portMAX_DELAY);
      CmdLineProcess ((char* )Cmd.Buff,Cmd.tpcb);
   }
}

