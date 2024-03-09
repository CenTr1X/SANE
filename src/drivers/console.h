
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

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "print.h"
#include "printf.h"

/* Uart(s) to print to and/or to receive from */
#define PRINT_UART_NR                    ( 0 )
#define RECV_UART_NR                     ( 0 )

//
// used by main.c to create startupTask
//
#define PRIOR_START_TASK                 ( 1 )

/* Settings for print.c */

/* Size of the queue with pointers to strings that will be printed */
#define PRINT_QUEUE_SIZE                 ( 10 )

/* Number of string buffers to print individual characters */
#define PRINT_CHR_BUF_SIZE               ( 5 )


/* Settings for receive.c */

/* Size of the queue holding received characters, that have not been processed yet. */
#define RECV_QUEUE_SIZE                  ( 50 )

/* Number of string buffers necessary to print received strings */
#define RECV_BUFFER_SIZE                 ( 3 )

/*
 * Number of characters in a buffer.
 * Note: this limit does not include '\0' and additional extra characters, necessary
 * to print the string properly.
 */
#define RECV_BUFFER_LEN                  ( 50 )

/*
 * Priorities of certain tasks.
 * Note: priorities should not be greater than configMAX_PRIORITIES - 1,
 * defined in FreeRTOSConfig.h (its default value equals 5).
 * If any priority is greater than this value, xTasCreate will
 * silently reduce it.
 */
#define PRIOR_PRINT_GATEKEEPR            ( 1 )
#define PRIOR_RECEIVER                   ( 1 )

void consoleInit();

#endif