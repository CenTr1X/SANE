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

#include "readline.h"
#include "history.h"
#include "shell.h"

void add_history(char* line)
{
   HistoryData* history = getHistoryData();
   unsigned int i = 0;

   if ((history == NULL) || (history->size == 0))
      return;

   while ((i < history->size) && (history->buffer[i] != NULL))
      i++;

   if (i == history->size)
   {
      i--;
      rl_free(history->buffer[i]);
      history->buffer[i] = NULL;
   }

   char* tmp = rl_realloc(NULL, strlen(line) + 1);

   if (tmp == NULL)
      return;

   while (i > 0)
   {
      history->buffer[i] = history->buffer[i - 1];
      i--;
   }

   history->buffer[0] = tmp;
   strcpy(history->buffer[0], line);
}

void clear_history()
{
   HistoryData* history = getHistoryData();

   if (history != NULL)
   {
      unsigned int i = 0;

      while ((i < history->size) && (history->buffer[i] != NULL))
      {
         rl_free(history->buffer[i]);
         history->buffer[i++] = NULL;
      }
   }
}
