/****************************************************************************
 * Copyright (c) 2021, 2022, Haiyong Xie
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not 
 * use this file except in compliance with the License. You may obtain a copy 
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   - Neither the name of the author nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "debug.h"
#include "print.h"
#include "printf.h"

#include "history.h"
#include "readline.h"

#include "shell.h"
#include "command.h"

#include "receive.h"

ReadlineData readlineData = READLINE_DATA(256);
HistoryData historyData = HISTORY_DATA(10);

/****************************************************************************
 *
 ****************************************************************************/
static int parseArgs(char* line, char* argv[SHELL_MAX_ARGS])
{
   int argc = 0;
   int i = 0;

   while (argc < (SHELL_MAX_ARGS - 1))
   {
      char quote = '\0';

      while (line[i] == ' ')
         i++;
      if (line[i] == '\0')
         break;

      argv[argc++] = &line[i];

      while (line[i] != '\0')
      {
         if (line[i] == quote)
         {
            quote = '\0';
            memmove(&line[i], &line[i + 1], strlen(&line[i]));
         }
         else if (quote != '\0')
         {
            i++;
         }
         else if ((line[i] == '\'') || (line[i] == '\"'))
         {
            quote = line[i];
            memmove(&line[i], &line[i + 1], strlen(&line[i]));
         }
         else if (line[i] != ' ')
         {
            i++;
         }
         else
         {
            line[i++] = '\0';
            break;
         }
      }
   }

   return argc;
}

inline HistoryData *getHistoryData() 
{ 
   return &historyData;
   //return NULL; 
}

inline ReadlineData *getReadlineData() 
{ 
   return &readlineData; 
   //return NULL;
}

/* 
** Shell
*/
static void vShellTask( void *pvParameters )
{
    ShellCmd* cmd = (ShellCmd *) pvParameters;

    for( ; ; )
    {
        /* Print out the name of this task. */

        char* line = readline(SHELL_PROMPT);

        char* argv[SHELL_MAX_ARGS];
        
        int argc, i;
        if (line == NULL) 
           break;
        
        if (line != NULL && line[0] != '\0')
        {         

           argc = parseArgs(line, argv);
           
           for (i = 0; cmd[i].name != NULL; i++)
           {
              if (strcmp(cmd[i].name, argv[0]) == 0) 
              {
                 add_history(line);
                 cmd[i].fx(argc, argv);
                 break;
              }
           }
           
           if (cmd[i].name == NULL)
           {
              printf_("command not found: %s\n", argv[0]);
           }
         }

        vTaskDelay( 100 / portTICK_RATE_MS );
    }

    /*
     * we should never reach here
     */
    SANE_PLATFORM_ERROR(("Shell exited. System still runs but there is no shell.\n"));

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}

#define USE_SIMPLE_SHELL 0

#if USE_SIMPLE_SHELL

/*******************************
 * a simple shell
 *******************************/

/* This code is received when BackSpace is pressed: */
#define CODE_BS             ( 0x7F )
/* Enter (CR): */
#define CODE_CR             ( 0x0D )

#define CMDBUF_SIZE 256
static char cmdbuf[CMDBUF_SIZE];

static char *simple_readline(char *prompt)
{
   portCHAR ch;
   int pos = 0;

   if (prompt != NULL)
      vDirectPrintMsg(prompt);

   for ( ; ; )
   {
      /* The task is blocked until something appears in the queue */
      //xQueueReceive(recvQueue, (void*) &ch, portMAX_DELAY);
      ch = getChar();
      
      vPrintChar(ch);

      if (ch == CODE_BS)
      {
         pos -= 1;
         if (pos < 0) pos = 0;
      }
      else if (ch == CODE_CR)
      {
         cmdbuf[pos++] = 0;
         return cmdbuf;
      }
      else
      {
         cmdbuf[pos++] = ch;
         // always reserve the last char position for '\0'
         if (pos > CMDBUF_SIZE - 1) 
            pos = CMDBUF_SIZE - 1;
      }
   }
}

static void vSimpleShellTask( void *pvParameters )
{
    ShellCmd* cmd = (ShellCmd *) pvParameters;

    for( ; ; )
    {

        char* line = simple_readline(SHELL_PROMPT);

        char* argv[SHELL_MAX_ARGS];
        
        int argc, i;
        // if (line == NULL) break;
        
        if (line != NULL && line[0] != '\0')
        {         

           argc = parseArgs(line, argv);
           
           for (i = 0; cmd[i].name != NULL; i++)
           {
              if (strcmp(cmd[i].name, argv[0]) == 0) 
              {
                 //add_history(line);
                 cmd[i].fx(argc, argv);
                 break;
              }
           }
           
           if (cmd[i].name == NULL)
           {
              printf_("command not found: %s\n", argv[0]);
           }
         }

        vTaskDelay( 100 / portTICK_RATE_MS );
    }

    /*
     * we should never reach here
     */
    SANE_PLATFORM_ERROR(("Shell exited. System still runs but there is no shell.\n"));

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}
#endif

void start_shell_task()
{

#if USE_SIMPLE_SHELL
    if ( pdPASS != xTaskCreate(vSimpleShellTask, "console shell", 1024, (void*) &SHELL_CMDS, 1, NULL) )
#else
    /*
     * the stack size should be big enough, as some tasks (e.g., ifconfig, sys_stats) need 
     * large buffers. If the stack size if too small, memory will be corrupted due to stack
     * overflow
     */
    if ( pdPASS != xTaskCreate(vShellTask, "console shell", 4096, (void*) &SHELL_CMDS, 1, NULL) )
#endif
    {
        SANE_PLATFORM_ERROR(("Could not create task1\r\n"));
    }

}
