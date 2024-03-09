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
#include <stdbool.h>

#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"

#include "debug.h"
#include "ethernetif.h"

#include "lan91c.h"
#include "sic.h"

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void lwip_network_init(void)
{
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gw;

  /* IP address setting */
#ifdef USE_DHCP
  uint8_t iptab[4] = {0};
  uint8_t iptxt[20];
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
#else
    IP4_ADDR(&ipaddr, 10, 0, 0, 10);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, 10, 0, 0, 1);
#endif  

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
  struct ip_addr *netmask, struct ip_addr *gw,
  void *state, err_t (* init)(struct netif *netif),
  err_t (* input)(struct pbuf *p, struct netif *netif))

  Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/

  /* Since we are using the tcp/ip thread for input, we can just use the real
   * input function.
   */

  LAN91C* lan91c = &gLan91c;

  lan91c->pcurnetif = &(lan91c->netif[0]);
  lan91c->pcurnetif->name[0] = 'l';
  lan91c->pcurnetif->name[1] = '0';

  netif_add(lan91c->pcurnetif, &ipaddr, &netmask, &gw, lan91c, lwip_network_init_lan91cInit, tcpip_input);
  netif_set_up(lan91c->pcurnetif);
  netif_set_link_up(lan91c->pcurnetif);

  netif_set_default(lan91c->pcurnetif);
   
#if LWIP_NETIF_LINK_CALLBACK
  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&nxetif, lan91c->linkChangeMsg);
#endif

  /* second virtual interface */
  IP4_ADDR(&ipaddr, 10, 0, 1, 10);
  IP4_ADDR(&netmask, 255, 255, 255, 0);
  IP4_ADDR(&gw, 10, 0, 1, 1);

  lan91c->pcurnetif = &(lan91c->netif[1]);
  lan91c->pcurnetif->name[0] = 'l';
  lan91c->pcurnetif->name[1] = '1';

  netif_add(lan91c->pcurnetif, &ipaddr, &netmask, &gw, lan91c, lwip_network_init_lan91cInit, tcpip_input);
  netif_set_up(lan91c->pcurnetif);
  netif_set_link_up(lan91c->pcurnetif);

  /* second virtual interface */
  IP4_ADDR(&ipaddr, 10, 0, 2, 10);
  IP4_ADDR(&netmask, 255, 255, 255, 0);
  IP4_ADDR(&gw, 10, 0, 2, 1);

  lan91c->pcurnetif = &(lan91c->netif[2]);
  lan91c->pcurnetif->name[0] = 'l';
  lan91c->pcurnetif->name[1] = '2';

  netif_add(lan91c->pcurnetif, &ipaddr, &netmask, &gw, lan91c, lwip_network_init_lan91cInit, tcpip_input);
  netif_set_up(lan91c->pcurnetif);
  netif_set_link_up(lan91c->pcurnetif);

#ifdef USE_DHCP
  DHCP_state = DHCP_START;
#endif

}

#ifdef USE_DHCP

#define MAX_DHCP_TRIES 4

#define DHCP_START                 1
#define DHCP_WAIT_ADDRESS          2
#define DHCP_ADDRESS_ASSIGNED      3
#define DHCP_TIMEOUT               4
#define DHCP_LINK_DOWN             5

uint8_t DHCP_state;

/**
  * @brief  LwIP_DHCP_Process_Handle
  * @param  None
  * @retval None
  */
void LwIP_DHCP_task(void * pvParameters)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
  uint32_t IPaddress;
  uint8_t iptab[4] = {0};
  uint8_t iptxt[20];

  struct netif *netif = (struct netif *) pvParameters;
  
  for (;;)
  {
    switch (DHCP_state)
    {
    case DHCP_START:
      {
        dhcp_start(netif);
        /* IP address should be set to 0 
           every time we want to assign a new DHCP address */
        IPaddress = 0;
        DHCP_state = DHCP_WAIT_ADDRESS;
      }
      break;

    case DHCP_WAIT_ADDRESS:
      {
        /* Read the new IP address */
        IPaddress = netif->ip_addr.addr;

        if (IPaddress!=0) {
          DHCP_state = DHCP_ADDRESS_ASSIGNED;	

          /* Stop DHCP */
          dhcp_stop(netif);
        }
        else{
          /* DHCP timeout */
          if (netif->dhcp->tries > MAX_DHCP_TRIES){
            DHCP_state = DHCP_TIMEOUT;

            /* Stop DHCP */
            dhcp_stop(netif);

            /* Static address used */
            IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
            IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
            IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
            netif_set_addr(netif, &ipaddr , &netmask, &gw);
          }
        }
      }
      break;

    default: 
        break;
    } // switch
    
    /* wait 250 ms */
    vTaskDelay(250);

  } // for   
}

#endif  /* USE_DHCP */
