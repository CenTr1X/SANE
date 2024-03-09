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
#include "printf.h"

#include "pic.h"
#include "sic.h"

#include "debug.h"

// VIC is PIC, use original implementation pic_registerIrq() in interrupt.c
// initialized in _init() before entering main()
SIC sic;

/****************************************************************************
 *
 ****************************************************************************/
inline void enable_SIC_IRQ(){
    (*((volatile unsigned long*) (BSP_PIC_BASE_ADDRESS + 0x14))) |= 1 << 31;
}

/****************************************************************************
 *
 ****************************************************************************/
inline void enable_SIC_ETH(){
    (*((volatile unsigned long*) (BSP_SIC_BASE_ADDRESS + 0x08))) |= 1 << 25;
}

/****************************************************************************
 *
 ****************************************************************************/
inline void vectorsHigh()
{
   unsigned long cr;
   __asm__ __volatile__("mrc p15, 0, %0, c1, c0, 0" : "=r" (cr));
   cr |= 0x00002000;
   __asm__ __volatile__("mcr p15, 0, %0, c1, c0, 0" : : "r" (cr));
}

/****************************************************************************
 *
 ****************************************************************************/
inline bool interruptsEnabled()
{
   unsigned long cpsr;
   __asm__ __volatile__("mrs %0, CPSR" : "=r" (cpsr));
   return (cpsr & CPU_I_BIT) ? false : true;
}

/****************************************************************************
 *
 ****************************************************************************/
inline bool disableInterrupts()
{
   unsigned long cpsr;
   __asm__ __volatile__("mrs %0, CPSR" : "=r" (cpsr));
   __asm__ __volatile__("msr CPSR, %0" : : "r" (cpsr | CPU_I_BIT) :
                        "memory");
   return (cpsr & CPU_I_BIT) ? false : true;
}

/****************************************************************************
 *
 ****************************************************************************/
inline void enableInterrupts()
{
   unsigned long cpsr;
   __asm__ __volatile__("mrs %0, CPSR" : "=r" (cpsr));
   __asm__ __volatile__("msr CPSR, %0" : : "r" (cpsr & ~CPU_I_BIT) :
                        "memory");
}

/****************************************************************************
 *
 ****************************************************************************/
inline unsigned short bSwap16(unsigned short value)
{
   value = (value >> 8) | (value << 8);
   return value;
}

/****************************************************************************
 *
 ****************************************************************************/
inline unsigned long bSwap32(unsigned long value)
{
   unsigned long tmp = 0;

   __asm__
   (
      "eor %0, %3, %3, ror #16 \n"
      "bic %0, #0x00FF0000     \n"
      "mov %1, %3, ror #8      \n"
      "eor %1, %0, lsr #8      \n"
      : "=r" (tmp), "=r" (value) : "0" (tmp), "1" (value)
   );

   return value;
}

/****************************************************************************
 *
 ****************************************************************************/
inline int _cpuID()
{
#ifdef SMP
   int id;
   __asm__ volatile("mrc p15, 0, %0, c0, c0, 5" : "=r" (id));
   return id & 3;
#else
   return 0;
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
void enable_sic()
{
    // enable SIC
    /* Attempt to register SIC IRQ on VIC */
    if ( pic_registerIrq(31, &sicIRQ, 200) < 0 )
    {
      SANE_DEBUGF(SANE_DBG_IRQ, ("enable_sic: pic_registerIrq irq=31 failed\n"));
    }

    /* Enable the SIC's IRQ on VIC */

    // two ways, should be equivalent
    pic_enableInterrupt(31);
}

/****************************************************************************
 *
 ****************************************************************************/
static void addHandler(struct IrqCtrl* ctrl, unsigned int n,
                       void (*fx)(unsigned int, void*), void* arg, bool edge,
                       unsigned int cpuMask)
{
   SIC* sic = (SIC*) ctrl;

   if (fx != NULL)
      // enable this irq
      SIC_INT_ENABLESET(sic) |= 1 << n;
   else
      // disable this irq
      SIC_INT_ENABLECLR(sic) |= 1 << n;

   sic->vector[n] = fx;
   sic->arg[n] = arg;
}

/****************************************************************************
 *
 ****************************************************************************/

volatile int sicirqcnt = 0;

void sicIRQ(void)
{
   SIC *s = &sic;
   unsigned long status = SIC_INT_STATUS(s);
   unsigned int i = 0;

   SANE_DEBUGF(SANE_DBG_IRQ, ("\nsicIRQ: sicirqcnt = %d\n", sicirqcnt));

   while (status)
   {
      if (status & 1)
      {
         if (s->vector[i] != NULL)
            s->vector[i](i, s->arg[i]);
         else
            SANE_PLATFORM_ERROR(("unhandled irq: %d\n", i));
      }

      status >>= 1;
      i++;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void sicInit(SIC* sic)
{
   sic->base = BSP_SIC_BASE_ADDRESS;
   sic->ctrl.addHandler = addHandler;
}
