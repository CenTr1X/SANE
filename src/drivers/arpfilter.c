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

#if LWIP_IGMP
#include "lwip/igmp.h"
#endif /*LWIP_IGMP*/

#include "debug.h"

struct netif *lwip_arp_filter_netif_fn(struct pbuf *p, struct netif *netifIn, u16_t type)
{
  struct netif *netif = NULL;
  struct etharp_hdr *etharphdr = NULL;
  struct ip_hdr *iphdr = NULL;
  ip_addr_t src, dst;

  struct eth_hdr *ethhdr = (struct eth_hdr *)p->payload;

#if LWIP_ARP || ETHARP_SUPPORT_VLAN
  u16_t next_hdr_offset = SIZEOF_ETH_HDR;
#endif /* LWIP_ARP || ETHARP_SUPPORT_VLAN */

#if ETHARP_SUPPORT_VLAN

  struct eth_vlan_hdr *vlan = NULL;
  u16_t vlanid = 0;

  if (ethhdr->type == PP_HTONS(ETHTYPE_VLAN)) {

    /* VLAN tagged packet */

    /*
     * set the following data:
     *  vlan: struct eth_vlan_hdr, a pointer to VLAN header
     *  vlanid: VLAN ID
     *  type: inner packet type (NOT VLAN type) 
     */

    vlan = (struct eth_vlan_hdr *)(((char *)ethhdr) + SIZEOF_ETH_HDR);

    next_hdr_offset = SIZEOF_ETH_HDR + SIZEOF_VLAN_HDR;

    LWIP_ASSERT("lwip_arp_filter_netif_fn: find a mis-formed VLAN tagged packet with incorrect size\n", p->len > SIZEOF_ETH_HDR + SIZEOF_VLAN_HDR);
    
    vlanid = lwip_ntohs((u16_t) (vlan->prio_vid));

    type = lwip_ntohs(vlan->tpid);

  } else {

    /* untagged packet */

    /*
     * set the following data:
     *  vlan = NULL: struct eth_vlan_hdr, a pointer to VLAN header
     *  vlanid = 0: default VLAN ID
     *  type: real packet type
     */
    vlan = NULL;
    vlanid = 0;   // default VLAN ID: 0
    type = lwip_ntohs(ethhdr->type);
  }
#endif

  SANE_DEBUGF(SANE_DBG_ARP_FILTER, ("lwip_arp_filter_netif_fn: entry, netifIn = %c%c, type = %04x\n", 
    netifIn->name[0], netifIn->name[1], type));
  
  char casestr[16];
  switch (type)
  {
    /* ARP */
    case 0x0806:

#if ETHARP_SUPPORT_VLAN
        if (NULL != vlan)
        {
          /* tagged packet */
          etharphdr = (struct etharp_hdr *) ((unsigned char *)p->payload + SIZEOF_ETH_HDR + SIZEOF_VLAN_HDR);
        }
        else
        {
          /* untagged packet */
          etharphdr = (struct etharp_hdr *) ((unsigned char *)p->payload + SIZEOF_ETH_HDR);
        }
#else
        etharphdr = (struct etharp_hdr *) ((unsigned char *)p->payload + SIZEOF_ETH_HDR);
#endif

        memcpy(&dst, &etharphdr->dipaddr, sizeof(ip4_addr_t));
        memcpy(&src, &etharphdr->sipaddr, sizeof(ip_addr_t));

        for (netif = netif_list; netif != NULL; netif = netif->next)
        {

#if ETHARP_SUPPORT_VLAN
          if (netif_is_up(netif) && ip4_addr_cmp(&dst, &(netif->ip_addr)) && netif->vlanid == vlanid)
#else
          if (netif_is_up(netif) && ip4_addr_cmp(&dst, &(netif->ip_addr)))
#endif
          {
            strcpy(casestr, "ARP");
 
            break;
          }

        } // for
        break;

      /* IP */
    case 0x0800:
#if ETHARP_SUPPORT_VLAN
        if (NULL != vlan)
        {
          /* tagged packet */
          iphdr = (struct ip_hdr *) ((unsigned char *)p->payload + SIZEOF_ETH_HDR + SIZEOF_VLAN_HDR);
        }
        else
        {
          /* untagged packet */
          iphdr = (struct ip_hdr *) ((unsigned char *)p->payload + SIZEOF_ETH_HDR);
        }
#else
        iphdr = (struct ip_hdr *)((unsigned char *) p->payload + SIZEOF_ETH_HDR);
#endif
        
        ip_addr_copy_from_ip4(dst, iphdr->dest);
        ip_addr_copy_from_ip4(src, iphdr->src);

        for (netif = netif_list; netif != NULL; netif = netif->next)
        {
        
#if LWIP_IGMP
#if ETHARP_SUPPORT_VLAN
          if (netif_is_up(netif) && (ip4_addr_cmp(&dst, &(netif->ip_addr)) || igmp_lookfor_group(netif, &dst) ) && netif->vlanid == vlanid)
#else
          if (netif_is_up(netif) && ip4_addr_cmp(&dst, &(netif->ip_addr || igmp_lookfor_group(netif, &dst))))
#endif /*ETHARP_SUPPORT_VLAN*/
#else
#if ETHARP_SUPPORT_VLAN
          if (netif_is_up(netif) && ip4_addr_cmp(&dst, &(netif->ip_addr)) && netif->vlanid == vlanid)
#else
          if (netif_is_up(netif) && ip4_addr_cmp(&dst, &(netif->ip_addr)))
#endif /*ETHARP_SUPPORT_VLAN*/
#endif /*LWIP_IGMP*/
          {
            strcpy(casestr, "IP");
            break;
          }
        } // for
        break;

      /* default */
    default:
        netif = netif_list;
        strcpy(casestr, "DEFAULT");
        break;

  } /* switch */

#if ETHARP_SUPPORT_VLAN

  SANE_DEBUGF(SANE_DBG_ARP_FILTER, ("lwip_arp_filter_netif_fn: %s, case %04x, mactch netif = %c%c, dst = %s, vlan = %d\n", 
   casestr, type, netif->name[0], netif->name[1], inet_ntoa(netif->ip_addr), netif->vlanid));

#else

  SANE_DEBUGF(SANE_DBG_ARP_FILTER, ("lwip_arp_filter_netif_fn: %s, case %04x, mactch netif = %c%c, dst = %s\n", 
    casestr, type, netif->name[0], netif->name[1], inet_ntoa(netif->ip_addr)));

#endif

#if ETHARP_SUPPORT_VLAN

  SANE_DEBUGF(SANE_DBG_ARP_FILTER, ("lwip_arp_filter_netif_fn: return, found netif = %c%c, vlan = %d\n", netif->name[0], netif->name[1], netif->vlanid));

#else

  SANE_DEBUGF(SANE_DBG_ARP_FILTER, ("lwip_arp_filter_netif_fn: return, found netif = %c%c\n", netif->name[0], netif->name[1]));

#endif
  return netif;
}
