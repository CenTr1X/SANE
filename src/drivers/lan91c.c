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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include <FreeRTOS.h>
#include <task.h>
#include "queue.h"

#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/arch.h"
#include "lwip/etharp.h"

#include "printf.h"
#include "ethernetif.h"
#include "sic.h"

#include "lan91c.h"

#include "debug.h"
#ifdef LAN91C_IRQ_USE_SYS_MBOX
sys_mbox_t lan91cmbox;
#else
QueueHandle_t lan91cqueue;
#endif

LAN91C gLan91c;

void setup_lan91c_base_address()
{
   LAN91C* lan91c = &gLan91c;
   lan91c->base = LAN91C111_BASE_ADDR;

/**
 * VLAN support
 */
#if ETHARP_SUPPORT_VLAN
   /**
    * set default VLAN ID for each interface
    */
   for (int i = 0; i < LAN91C_MAX_NETIF; i++)
      (lan91c->netif[i]).vlanid = 0;
#endif

   /*
    * get PHY hwaddr
    */
   SET_BSR(lan91c, 1);
   lan91c->hwaddr[0] = B1_IARn(lan91c)[0];
   lan91c->hwaddr[1] = B1_IARn(lan91c)[1];
   lan91c->hwaddr[2] = B1_IARn(lan91c)[2];
   lan91c->hwaddr[3] = B1_IARn(lan91c)[3];
   lan91c->hwaddr[4] = B1_IARn(lan91c)[4];
   lan91c->hwaddr[5] = B1_IARn(lan91c)[5];
}

#if 0
/****************************************************************************
 *
 ****************************************************************************/
static void miiTx(uint32_t value, unsigned int count)
{
   unsigned int i;

   SET_BSR(3);

   B3_MGMT |= 0x0008;

   for (i = 0; i < count; i++)
   {
      B3_MGMT &= ~0x0001;
      B3_MGMT |= (uint16_t) ((value >> (31 - i)) & 1);

      B3_MGMT |= 0x0004;
      B3_MGMT &= ~0x0004;
   }

   B3_MGMT &= ~0x0008;
}

/****************************************************************************
 *
 ****************************************************************************/
static uint32_t miiRx(unsigned int count)
{
   uint32_t value = 0;
   unsigned int i;

   SET_BSR(3);

   B3_MGMT &= ~0x0008;

   for (i = 0; i < count; i++)
   {
      B3_MGMT |= 0x0004;
      value = (value << 1) | (B3_MGMT >> 1);
      B3_MGMT &= ~0x0004;
   }

   return value;
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
static err_t lan91cTx(struct netif* netif, struct pbuf* pbuf)
{
   LAN91C* lan91c = netif->state;
   struct pbuf* q = NULL;

   B2_MSK(lan91c) &= ~0x01;
   B2_MMUCR(lan91c) = 1 << 5; /* allocate TX buffer */

   while ((B2_IST(lan91c) & 0x08) == 0)
   {
      printf("loop...\n");
      if (B2_ARR(lan91c) & 0x80)
      {
         B2_MSK(lan91c) |= 0x01;

         taskYIELD();

         B2_MSK(lan91c) &= ~0x01;

         B2_MMUCR(lan91c) = 1 << 5;
      }
   }

   B2_MSK(lan91c) |= 0x01;

   B2_PNR(lan91c) = B2_ARR(lan91c);
   B2_PTR(lan91c) = 0x4000;

   B2_DATA16n(lan91c)[0] = 0;
   B2_DATA16n(lan91c)[0] = (pbuf->tot_len & ~1) + 6;

   for (q = pbuf; q != NULL; q = q->next)
   {
      uintptr_t payload = (uintptr_t) q->payload;
      uint16_t i;

      for (i = 0; i < (q->len / 4); i++)
      {
         B2_DATA32(lan91c) = *(uint32_t*) payload;
         payload += 4;
      }

      if (q->len & 2)
      {
         B2_DATA16n(lan91c)[0] = *(uint16_t*) payload;
         payload += 2;
      }

      if (q->len & 1)
      {
         B2_DATA8n(lan91c)[0] = *(uint8_t*) payload;
         payload++;
      }
   }
   if (B2_PTR(lan91c) & 1)
   {
      B2_PTR(lan91c) &= ~0x4001;
      B2_DATA16n(lan91c)[0] = 0x2000 | B2_DATA8n(lan91c)[0];
   }
   else
   {
      B2_DATA16n(lan91c)[0] = 0;
   }
   B2_MSK(lan91c) &= ~0x01;
   B2_MMUCR(lan91c) = 6 << 5;
   B2_MSK(lan91c) |= 0x01;
   return ERR_OK;
}

/****************************************************************************
 *
 ****************************************************************************/
static void lan91cRx(void* state)
{
   LAN91C* lan91c = state;
   struct pbuf* pbuf = NULL;
   uint8_t pkt;
   uint16_t length;
   uintptr_t payload;
   uint16_t i;
   int time = xTaskGetTickCount();
#ifdef LAN91C_IRQ_USE_SYS_MBOX
   if ( sys_arch_mbox_tryfetch(lan91c->rxPacket, &pkt) != 0 ){
      SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cRx: sys_arch_mbox_tryfetch error\n"));
      return;
   }else {
      SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cRx: sys_arch_mbox_tryfetch succeed. pkt = %d\n", pkt));
   }
#else
   if ( pdPASS != xQueueReceive(lan91c->rxPacket, &pkt, portMAX_DELAY) ){
      SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cRx: xQueueReceive error\n"));
      return;
   }else {
      SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cRx: xQueueReceive succeed. pkt = %d\n", pkt));
   }
#endif
   SANE_DEBUGF(SANE_DBG_IRQ, ("lan91cRX: entry, pkt = %d, mtu=%d\n", pkt, lan91c->pcurnetif->mtu));
   //printf("lan91cRX: entry, pkt = %d, mtu=%d\n", pkt, lan91c->pcurnetif->mtu);
   B2_PNR(lan91c) = pkt;
   B2_PTR(lan91c) = 2;
   length = B2_DATA16n(lan91c)[0];

   B2_PTR(lan91c) = length - 2;

   if (B2_DATA16n(lan91c)[0] & 0x2000)
      length++;

   B2_PTR(lan91c) = 0x4004;
   length -= 6;

   pbuf = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);
   payload = (uintptr_t) pbuf->payload;

   for (i = 0; i < length / 4; i++)
   {
      *(uint32_t*) payload = B2_DATA32(lan91c);
      payload += 4;
   }

   if (length & 2)
   {
      *(uint16_t*) payload = B2_DATA16n(lan91c)[0];
      payload += 2;
   }

   if (length & 1)
      *(uint8_t*) payload = B2_DATA8n(lan91c)[0];

   B2_MSK(lan91c) &= ~0x01;
   B2_MMUCR(lan91c) = 5 << 5;
   while (B2_MMUCR(lan91c) & 0x0001);
   B2_MSK(lan91c) |= 0x01;

   SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cRx: invoking netif->input(), length = %d payload = %d\n", length, payload));
   if (lan91c->pcurnetif->input(pbuf, lan91c->pcurnetif) != ERR_OK){
      SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cRx: netif->input() failed.\n"));
      pbuf_free(pbuf);
   }
   
}

/****************************************************************************
 *
 ****************************************************************************/

volatile int ethirqcnt = 0;

void lan91cIRQ(unsigned int n, void* _lan91c)
{

   LAN91C* lan91c = _lan91c;

   ethirqcnt += 1;

   SANE_DEBUGF(SANE_DBG_IRQ, ("lan91cIRQ: ethirqcnt = %d\n", ethirqcnt));
    
   if (B2_IST(lan91c) & 0x02)
      SANE_DEBUGF(SANE_DBG_IRQ, ("fatal SMSC LAN91C111 TX error\n"));

   if (B2_IST(lan91c) & 0x01)
   {
      uint8_t pkt = B2_RXFIFO(lan91c) & ~0x80;

#ifdef LAN91C_IRQ_USE_SYS_MBOX
      SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cIRQ: calling sys_mbox_trypost_from_isr, ethirqcnt = %d, pkt = %d %p\n", ethirqcnt, pkt, &pkt));

      //if ( ERR_OK != sys_mbox_trypost(lan91c->rxPacket, (void *)&pkt)) {
      if ( ERR_OK != sys_mbox_trypost_fromisr(lan91c->rxPacket, &pkt)){
         SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cIRQ: sys_mbox_post Error\n"));
      }else{
         SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cIRQ: sys_mbox_post succeeds\n"));
      }
#else 
      // use FreeRTOS Queue
      SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cIRQ: calling xQueueSendToBackFromISR, ethirqcnt = %d, pkt = %d %p\n", ethirqcnt, pkt, &pkt));
      BaseType_t re = xQueueSendToBackFromISR(lan91c->rxPacket,  &pkt, pdFALSE);
      if (pdPASS != re) {
         SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cIRQ: xQueueSendToBackFromISR Error\n"));
      }else{
         SANE_DEBUGF(SANE_DBG_IRQ, ("\nlan91cIRQ: xQueueSendToBackFromISR succeeds\n"));
      }
#endif

      tcpip_callbackmsg_trycallback_fromisr(lan91c->ethRxMsg);

      B2_MMUCR(lan91c) = 3 << 5;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void linkChange(void* state)
{
   LAN91C* lan91c = state;

   netif_set_link_up(lan91c->netif);

#ifdef LWIP_DHCP
   if ((lan91c->pcurnetif->ip_addr.addr == 0) &&
       ((lan91c->pcurnetif->flags & NETIF_FLAG_UP) != 0))
   {
      dhcp_start(lan91c->pcurnetif);
   }
#endif

}

/****************************************************************************
 *
 ****************************************************************************/

static int lan91c_initialized = 0;

static err_t _lan91cInit_low_level_init(struct netif* netif)
{

   /*
    * set up packet recv queue
    */

   LAN91C* lan91c = &gLan91c;

   lan91c->linkChangeMsg = tcpip_callbackmsg_new(linkChange, lan91c);
   lan91c->ethRxMsg = tcpip_callbackmsg_new(lan91cRx, lan91c);

#ifdef LAN91C_IRQ_USE_SYS_MBOX
   if ( ERR_OK != sys_mbox_new(&lan91cmbox, 10)){
      SANE_DEBUGF(SANE_DBG_IRQ, ("Error: sys_mbox_new failed\n"));
      return;
   }
   lan91c->rxPacket = &lan91cmbox;
#else
   lan91cqueue = xQueueCreate(50, sizeof(uint8_t *));
   lan91c->rxPacket = lan91cqueue;
#endif

   /*
    * initialize hardware PHY
    */
   SET_BSR(lan91c, 2);
   B2_MMUCR(lan91c) = 2 << 5; /* reset MMU */
   while (B2_MMUCR(lan91c) & 0x0001);


   SET_BSR(lan91c, 1);
   B1_CTR(lan91c) |= 0x0800; /* auto-release */

   SET_BSR(lan91c, 0);
   B0_RPCR(lan91c) |= 0x0800; /* enable auto-negotiation */
   B0_TCR(lan91c) |= 0x0081; /* pad enable, enable TX */
   B0_RCR(lan91c) |= 0x0300; /* strip CRC, enable RX */

   SET_BSR(lan91c, 2);
   B2_MSK(lan91c) |= 0x03; /* TX,RX INT */

   // 
   // Important: 
   //
   // enable ETH on SIC
   // this will generate a lot of ETH interrupts
   sic.ctrl.addHandler(&sic.ctrl, 25, lan91cIRQ, lan91c, false, 1);

#if 0
   /*************
    * To Do: add recv thread
    */
  /* create the task that handles the ETH_MAC */
  xTaskCreate(lan91cRx, netif->state, netifINTERFACE_TASK_STACK_SIZE, NULL, netifINTERFACE_TASK_PRIORITY,NULL);
#endif

   return ERR_OK;
}

static int initialized = 0;

int lwip_network_init_lan91cInit(struct netif *netif)
{

   /* Since we are using the tcp/ip thread for input, we can just use the real
    * input function.
    */
#if LWIP_NETIF_HOSTNAME
   /* Initialize interface hostname */
   netif->hostname = MY_HOSTNAME;
#endif /* LWIP_NETIF_HOSTNAME */

   netif->mtu = 1500;

   netif->hwaddr_len = ETHARP_HWADDR_LEN;

   /*
    * Initialize the snmp variables and counters inside the struct netif.
    * The last argument should be replaced with your link speed, in units
    * of bits per second.
    */
   // NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_NETIF_IN_BPS); */

   netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_UP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP;

   /* We directly use etharp_output() here to save a function call.
    * You can instead declare your own function an call etharp_output()
    * from it if you have to do some checks before sending (e.g. if link
    * is available...) */
   netif->output = etharp_output;
   netif->linkoutput = lan91cTx;

#define USE_REAL_HW_ADDR

#ifdef USE_REAL_HW_ADDR
   LAN91C *lan91c = &gLan91c;
   if(!initialized)
   {
      srand(1);
      initialized = 1;
   }
   u8_t random = (rand() % 254)+1;
   netif->hwaddr[0] = lan91c->hwaddr[0];
   netif->hwaddr[1] = lan91c->hwaddr[1];
   netif->hwaddr[2] = lan91c->hwaddr[2];
   netif->hwaddr[3] = lan91c->hwaddr[3];
   netif->hwaddr[4] = lan91c->hwaddr[4];
   //netif->hwaddr[5] = lan91c->hwaddr[5];
   netif->hwaddr[5] = random;
   //netif->hwaddr[5] = lan91c->hwaddr[5] + offset;
   //offset++;
#else
   /* set netif MAC hardware address */
   netif->hwaddr[0] =  MAC_ADDR0;
   netif->hwaddr[1] =  MAC_ADDR1;
   netif->hwaddr[2] =  MAC_ADDR2;
   netif->hwaddr[3] =  MAC_ADDR3;
   netif->hwaddr[4] =  MAC_ADDR4;
   netif->hwaddr[5] =  MAC_ADDR5;
#endif

   /*
    * make sure lan91c only initialize once 
    */
   if (0 == lan91c_initialized)
   {
      lan91c_initialized = 1;
      /* initialize the hardware */
      _lan91cInit_low_level_init(netif);
   }

   return ERR_OK;
}