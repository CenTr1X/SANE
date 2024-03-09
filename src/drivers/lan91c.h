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

#ifndef LAN91C_H
#define LAN91C_H

#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>
#include "queue.h"

#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "arch/sys_arch.h"
#include "lwip/etharp.h"

/****************************************************************************
 * SMSC LAN91C111
 ****************************************************************************/

#define LAN91C111_BASE_ADDR   0x10010000

#define B0_TCR(n)     (*((volatile uint16_t*) (n->base + 0x0)))
#define B0_EPHSR(n)   (*((volatile uint16_t*) (n->base + 0x2)))
#define B0_RCR(n)     (*((volatile uint16_t*) (n->base + 0x4)))
#define B0_ECR(n)     (*((volatile uint16_t*) (n->base + 0x6)))
#define B0_MIR(n)     (*((volatile uint16_t*) (n->base + 0x8)))
#define B0_RPCR(n)    (*((volatile uint16_t*) (n->base + 0xA)))
#define B1_CR(n)      (*((volatile uint16_t*) (n->base + 0x0)))
#define B1_BAR(n)     (*((volatile uint16_t*) (n->base + 0x2)))
#define B1_IARn(n)    ((volatile uint8_t*) (n->base + 0x4))
#define B1_GPR(n)     (*((volatile uint16_t*) (n->base + 0xA)))
#define B1_CTR(n)     (*((volatile uint16_t*) (n->base + 0xC)))
#define B2_MMUCR(n)   (*((volatile uint16_t*) (n->base + 0x0)))
#define B2_PNR(n)     (*((volatile uint8_t*) (n->base + 0x2)))
#define B2_ARR(n)     (*((volatile uint8_t*) (n->base + 0x3)))
#define B2_FIFO(n)    (*((volatile uint16_t*) (n->base + 0x4)))
#define B2_TXFIFO(n)  (*((volatile uint8_t*) (n->base + 0x4)))
#define B2_RXFIFO(n)  (*((volatile uint8_t*) (n->base + 0x5)))
#define B2_PTR(n)     (*((volatile uint16_t*) (n->base + 0x6)))
#define B2_DATA8n(n)  ((volatile uint8_t*) (n->base + 0x8))
#define B2_DATA16n(n) ((volatile uint16_t*) (n->base + 0x8))
#define B2_DATA32(n)  (*((volatile uint32_t*) (n->base + 0x8)))
#define B2_IST(n)     (*((volatile uint8_t*) (n->base + 0xC)))
#define B2_MSK(n)     (*((volatile uint8_t*) (n->base + 0xD)))
#define B3_MTn(n)     ((volatile uint8_t*) (n->base + 0x0))
#define B3_MGMT(n)    (*((volatile uint16_t*) (n->base + 0x8)))
#define B3_REV(n)     (*((volatile uint16_t*) (n->base + 0xA)))
#define B3_RCV(n)     (*((volatile uint16_t*) (n->base + 0xC)))
#define BSR(n)        (*((volatile uint16_t*) (n->base + 0xE)))

/****************************************************************************
 *
 ****************************************************************************/
#define SET_BSR(n, b) do { BSR(n) &= ~7; BSR(n) |= b; } while (0)

/****************************************************************************
 *
 ****************************************************************************/
#ifdef LAN91C_IRQ_USE_SYS_MBOX
extern sys_mbox_t lan91cmbox;
#else
extern QueueHandle_t lan91cqueue;
#endif

#define LAN91C_MAX_NETIF 5

#define LAN91C_CREATE(base)            \
{                                      \
   base,                               \
   NULL,                               \
   {NULL, NULL, NULL, NULL, NULL},     \
   NULL,                               \
   NULL,                               \
   NULL                                \
}

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   unsigned long base;

/**
 * multiple (virtual) network interfaces support
 */
   struct netif* pcurnetif;
   struct netif netif[LAN91C_MAX_NETIF];
   char hwaddr[ETHARP_HWADDR_LEN];

   struct tcpip_callback_msg* linkChangeMsg;
   struct tcpip_callback_msg* ethRxMsg;
#ifdef LAN91C_IRQ_USE_SYS_MBOX
   sys_mbox_t* rxPacket;
#else
   QueueHandle_t rxPacket;
#endif
} LAN91C;

extern LAN91C gLan91c;

void setup_lan91c_base_address();

/****************************************************************************
 *
 ****************************************************************************/
void lan91cIRQ(unsigned int n, void* lan91c);

/****************************************************************************
 *
 ****************************************************************************/
int lwip_network_init_lan91cInit(struct netif *netif);

#endif
