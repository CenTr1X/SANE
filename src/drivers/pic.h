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


/**
 * 
 * the board's primary interrupt controller (PIC).
 *
 */

#ifndef _PIC_H_
#define _PIC_H_

#include <stdbool.h>
#include <stdint.h>

#define PIC_MAX_PRIORITY     ( 127 )

/****************************************************************************
 *
 ****************************************************************************/
#define PICIRQSTATUS(p)    (*((volatile unsigned long*) (p->base + 0x000)))
#define PICFIQSTATUS(p)    (*((volatile unsigned long*) (p->base + 0x004)))
#define PICRAWINTR(p)      (*((volatile unsigned long*) (p->base + 0x008)))
#define PICINTSELECT(p)    (*((volatile unsigned long*) (p->base + 0x00C)))
#define PICINTENABLE(p)    (*((volatile unsigned long*) (p->base + 0x010)))
#define PICINTENCLEAR(p)   (*((volatile unsigned long*) (p->base + 0x014)))
#define PICSOFTINT(p)      (*((volatile unsigned long*) (p->base + 0x018)))
#define PICSOFTINTCLEAR(p) (*((volatile unsigned long*) (p->base + 0x01C)))
#define PICPROTECTION(p)   (*((volatile unsigned long*) (p->base + 0x020)))
#define PICVECTADDR(p)     (*((volatile unsigned long*) (p->base + 0x030)))
#define PICDEFVECTADDR(p)  (*((volatile unsigned long*) (p->base + 0x034)))
#define PICVECTADDR0(p)    (*((volatile unsigned long*) (p->base + 0x100)))
#define PICVECTADDR1(p)    (*((volatile unsigned long*) (p->base + 0x104)))
#define PICVECTADDR2(p)    (*((volatile unsigned long*) (p->base + 0x108)))
#define PICVECTADDR3(p)    (*((volatile unsigned long*) (p->base + 0x10C)))
#define PICVECTADDR4(p)    (*((volatile unsigned long*) (p->base + 0x110)))
#define PICVECTADDR5(p)    (*((volatile unsigned long*) (p->base + 0x114)))
#define PICVECTADDR6(p)    (*((volatile unsigned long*) (p->base + 0x118)))
#define PICVECTADDR7(p)    (*((volatile unsigned long*) (p->base + 0x11C)))
#define PICVECTADDR8(p)    (*((volatile unsigned long*) (p->base + 0x120)))
#define PICVECTADDR9(p)    (*((volatile unsigned long*) (p->base + 0x124)))
#define PICVECTADDR10(p)   (*((volatile unsigned long*) (p->base + 0x128)))
#define PICVECTADDR11(p)   (*((volatile unsigned long*) (p->base + 0x12C)))
#define PICVECTADDR12(p)   (*((volatile unsigned long*) (p->base + 0x130)))
#define PICVECTADDR13(p)   (*((volatile unsigned long*) (p->base + 0x134)))
#define PICVECTADDR14(p)   (*((volatile unsigned long*) (p->base + 0x138)))
#define PICVECTADDR15(p)   (*((volatile unsigned long*) (p->base + 0x13C)))
#define PICVECTCNTL0(p)    (*((volatile unsigned long*) (p->base + 0x200)))
#define PICVECTCNTL1(p)    (*((volatile unsigned long*) (p->base + 0x204)))
#define PICVECTCNTL2(p)    (*((volatile unsigned long*) (p->base + 0x208)))
#define PICVECTCNTL3(p)    (*((volatile unsigned long*) (p->base + 0x20C)))
#define PICVECTCNTL4(p)    (*((volatile unsigned long*) (p->base + 0x210)))
#define PICVECTCNTL5(p)    (*((volatile unsigned long*) (p->base + 0x214)))
#define PICVECTCNTL6(p)    (*((volatile unsigned long*) (p->base + 0x218)))
#define PICVECTCNTL7(p)    (*((volatile unsigned long*) (p->base + 0x21C)))
#define PICVECTCNTL8(p)    (*((volatile unsigned long*) (p->base + 0x220)))
#define PICVECTCNTL9(p)    (*((volatile unsigned long*) (p->base + 0x224)))
#define PICVECTCNTL10(p)   (*((volatile unsigned long*) (p->base + 0x228)))
#define PICVECTCNTL11(p)   (*((volatile unsigned long*) (p->base + 0x22C)))
#define PICVECTCNTL12(p)   (*((volatile unsigned long*) (p->base + 0x230)))
#define PICVECTCNTL13(p)   (*((volatile unsigned long*) (p->base + 0x234)))
#define PICVECTCNTL14(p)   (*((volatile unsigned long*) (p->base + 0x238)))
#define PICVECTCNTL15(p)   (*((volatile unsigned long*) (p->base + 0x23C)))

/****************************************************************************
 *
 ****************************************************************************/
typedef struct IrqCtrl
{
   void (*addHandler)(struct IrqCtrl* ctrl, unsigned int n,
                      void (*fx)(unsigned int, void*), void* arg, bool edge,
                      unsigned int cpuMask);

} IrqCtrl;

/****************************************************************************
 *
 ****************************************************************************/
#define PIC_CREATE(base) {{}, base, {}, {}}

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   IrqCtrl ctrl;
   unsigned long base;
   void (*vector[32])(unsigned int, void*);
   void* arg[32];

} PIC;

extern PIC pic;

/**
 * Required prototype for vectored ISR servicing routines
 */
typedef void (*pVectoredIsrPrototype)(void);

void irq_enableIrqMode(void);

void irq_disableIrqMode(void);

void pic_init(void);

void pic_enableInterrupt(uint8_t irq);

void pic_disableInterrupt(uint8_t irq);

void pic_disableAllInterrupts(void);

int8_t pic_isInterruptEnabled(uint8_t irq);

int8_t pic_getInterruptType(uint8_t irq);

void pic_setInterruptType(uint8_t irq, int8_t toIrq);

void pic_setDefaultVectorAddr(pVectoredIsrPrototype addr);

int8_t pic_registerIrq(uint8_t irq, pVectoredIsrPrototype addr, uint8_t priority );

void pic_unregisterIrq(uint8_t irq);

void pic_unregisterAllIrqs(void);

int8_t pic_setSwInterruptNr(uint8_t irq);

int8_t pic_clearSwInterruptNr(uint8_t irq);

int8_t pic_setSoftwareInterrupt(void);

int8_t pic_clearSoftwareInterrupt(void);

void picInit(PIC* pic);

#endif  /* _PIC_H_ */
