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

 #include <stdint.h>

 #include "bsp.h"

 #include "pic.h"
 #include "sic.h"

 #include "timer.h"
 #include "uart.h"
 #include "lan91c.h"

 void _init(void)
 {
     uint8_t i;
     uint8_t j;
     const uint8_t ctrs = timer_countersPerTimer();

     /* Disable IRQ triggering (may be reenabled after ISRs are properly set) */
     irq_disableIrqMode();

     /* Init the vectored interrupt controller */
     pic_init();

     picInit(&pic);
     sicInit(&sic);

     /* register SIC on PIC */
     enable_sic();

     /* Init all counters of all available timers */
     for ( i=0; i<BSP_NR_TIMERS; ++i )
     {
         for ( j=0; j<ctrs; ++j )
         {
             timer_init(i, j);
         }
     }

     /* Init all available UARTs */
     for ( i=0; i<BSP_NR_UARTS; ++i )
     {
         uart_init(i);
     }

     /* set up eth base addr. lan91c */
     setup_lan91c_base_address();
}
