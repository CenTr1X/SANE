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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stddef.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <FreeRTOS.h>

#include "printf.h"

// lwip/include/arch/cc.h: basic data types
#include "arch/cc.h"

/** Debug level: ALL messages*/
#define SANE_DBG_LEVEL_ALL     0x00
/** Debug level: Warnings. bad checksums, dropped packets, ... */
#define SANE_DBG_LEVEL_WARNING 0x01
/** Debug level: Serious. memory allocation failures, ... */
#define SANE_DBG_LEVEL_SERIOUS 0x02
/** Debug level: Severe */
#define SANE_DBG_LEVEL_SEVERE  0x03

#ifndef SANE_DBG_MIN_LEVEL
#define SANE_DBG_MIN_LEVEL     SANE_DBG_LEVEL_ALL
#endif

#define SANE_DBG_MASK_LEVEL    0x03
/* compatibility define only */
#define SANE_DBG_LEVEL_OFF     SANE_DBG_LEVEL_ALL


/** flag for SANE_DEBUGF to enable that debug message */
#ifndef SANE_DBG_ON
#define SANE_DBG_ON            0x80U
#endif

/** flag for SANE_DEBUGF to disable that debug message */
#define SANE_DBG_OFF           0x00U


/** flag for SANE_DEBUGF indicating a tracing message (to follow program flow) */
#define SANE_DBG_TRACE         0x40U
/** flag for SANE_DEBUGF indicating a state debug message (to follow module states) */
#define SANE_DBG_STATE         0x20U
/** flag for SANE_DEBUGF indicating newly added code, not thoroughly tested yet */
#define SANE_DBG_FRESH         0x10U
/** flag for SANE_DEBUGF to halt after printing this debug message */
#define SANE_DBG_HALT          0x08U

#define SANE_DBG_CONSOLE       0x0100U
#define SANE_DBG_IRQ           0x0200U
#define SANE_DBG_ETH_RECV      0x0400U
#define SANE_DBG_ARP_FILTER    0x0800U
#define SANE_DBG_VLAN          0x1000U
#define SANE_DBG_LAN91CTX      0x2000U

/* Plaform specific diagnostic output */
#define SANE_PLATFORM_DIAG(x)   do {                \
        printf_ x;                   \
    } while (0)

#define SANE_PLATFORM_ASSERT(x) do {                \
        printf_ x;                                  \
        printf_("Assertion failed at line %d in %s\n",  __LINE__, __FILE__);             \
        while( 1 );                        \
    } while (0)


#ifndef SANE_NOASSERT
#define SANE_ASSERT(message, assertion) do { if (!(assertion)) { \
  SANE_PLATFORM_ASSERT(message); }} while(0)
#else  /* SANE_NOASSERT */
#define SANE_ASSERT(message, assertion)
#endif /* SANE_NOASSERT */

#define SANE_PLATFORM_ERROR(message) SANE_PLATFORM_ASSERT(message)

/* if "expression" isn't true, then print "message" and execute "handler" expression */
#define SANE_ERROR(message, expression, handler) do { if (!(expression)) { \
  SANE_PLATFORM_ASSERT(message); handler;}} while(0)


#define SANE_DEBUG

#ifdef SANE_DEBUG

#define SANE_DEBUGF(debug, message) do { \
                               if ( \
                                   ((debug) & SANE_DBG_ON) && \
                                   ((s16_t)((debug) & SANE_DBG_MASK_LEVEL) >= SANE_DBG_MIN_LEVEL)) { \
                                 SANE_PLATFORM_DIAG(message); \
                                 if ((debug) & SANE_DBG_HALT) { \
                                   while(1); \
                                 } \
                               } \
                             } while(0)

#else  /* SANE_DEBUG */
#define SANE_DEBUGF(debug, message)
#endif /* SANE_DEBUG */


void FreeRTOS_Error(const portCHAR* msg);

void vAssertCalled( const char *pcFile, uint32_t ulLine );

void vApplicationMallocFailedHook( void );

#endif
