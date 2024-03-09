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

#ifndef _SIC_H_
#define _SIC_H_

#include <stdbool.h>

#include "bsp.h"
#include "pic.h" // for typedef struct IrqCtrl

/****************************************************************************
 *
 ****************************************************************************/
#define SIC_INT_STATUS(s)    (*((volatile unsigned long*) (s->base + 0x00)))
#define SIC_INT_RAWSTAT(s)   (*((volatile unsigned long*) (s->base + 0x04)))
#define SIC_INT_ENABLESET(s) (*((volatile unsigned long*) (s->base + 0x08)))
#define SIC_INT_ENABLECLR(s) (*((volatile unsigned long*) (s->base + 0x0C)))
#define SIC_INT_SOFTSET(s)   (*((volatile unsigned long*) (s->base + 0x10)))
#define SIC_INT_SOFTCLR(s)   (*((volatile unsigned long*) (s->base + 0x14)))

/****************************************************************************
 *
 ****************************************************************************/
#define be16toh bSwap16
#define be32toh bSwap32
#define htobe16 bSwap16
#define htobe32 bSwap32

/****************************************************************************
 *
 ****************************************************************************/
#define SIC_CREATE(base) {{}, base, {}, {}}

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   IrqCtrl ctrl;
   unsigned long base;
   void (*vector[32])(unsigned int, void*);
   void* arg[32];

} SIC;

extern SIC sic;

void enable_SIC_IRQ();
void enable_SIC_ETH();
void vectorsHigh();
bool interruptsEnabled();
bool disableInterrupts();
void enableInterrupts();

unsigned short bSwap16(unsigned short value);
unsigned long bSwap32(unsigned long value);
int _cpuID();
void enable_sic();

void sicIRQ(void);
void sicInit(SIC* sic);

#endif /* _SIC_H_ */