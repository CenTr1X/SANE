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

#include "netif/ethernet.h"
#include "lwip/def.h"
#include "lwip/stats.h"
#include "lwip/etharp.h"
#include "lwip/ip.h"
#include "debug.h"

#include "vlan.h"

#include "lan91c.h"

#if ETHARP_SUPPORT_VLAN

#ifdef LWIP_HOOK_VLAN_CHECK
/**
 * lwip_hook_vlan_check_fn() is called from ethernet_input() if VLAN support is enabled (ETHARP_SUPPORT_VLAN defined).
 *
 * Signature:\code{.c}
 *   int lwip_hook_vlan_check_fn(struct netif *netif, struct eth_hdr *eth_hdr, struct eth_vlan_hdr *vlan_hdr);
 * \endcode
 *
 * Arguments:
 * - netif: struct netif on which the packet has been received
 * - eth_hdr: struct eth_hdr of the packet
 * - vlan_hdr: struct eth_vlan_hdr of the packet
 * Return values:
 * - 0: Packet must be dropped.
 * - != 0: Packet must be accepted.
 *
 */
int lwip_hook_vlan_check_fn(struct netif *netif, struct eth_hdr *eth_hdr, struct eth_vlan_hdr *vlan_hdr)
{
    struct netif *nif;

    /*
     * find if an interface has the matching VLAN ID
     */
    NETIF_FOREACH(nif) {
        if (nif->vlanid == lwip_ntohs((u16_t) (vlan_hdr->prio_vid)))
        {
            SANE_DEBUGF(SANE_DBG_VLAN, ("lwip_hook_vlan_check_fn: netif %c%c VLAN ID matches %d %d\n", 
                nif->name[0], nif->name[1], nif->vlanid, lwip_ntohs((u16_t) (vlan_hdr->prio_vid))));
            return 1;
        }
    }

    SANE_DEBUGF(SANE_DBG_VLAN, ("lwip_hook_vlan_check_fn: netif %c%c no matching VLAN ID %d\n", 
        netif->name[0], netif->name[1], lwip_ntohs((u16_t) (vlan_hdr->prio_vid))));

    return 0;
}

#endif /* #ifdef LWIP_HOOK_VLAN_CHECK */

#ifdef LWIP_HOOK_VLAN_CHECK_AND_FILTER_FN

/**
 * LWIP_HOOK_VLAN_CHECK_AND_FILTER_FN(netif, eth_hdr, vlan_hdr):
 *
 * Called from ethernet_input() if VLAN support is enabled AND multiple interface is enabled
 *
 * Signature:\code{.c}
 *   struct netif *my_hook(struct netif *netif, struct eth_hdr *eth_hdr, struct eth_vlan_hdr *vlan_hdr);
 * \endcode
 *
 * Arguments:
 * - netif: struct netif on which the packet has been received
 * - eth_hdr: struct eth_hdr of the packet
 * - vlan_hdr: struct eth_vlan_hdr of the packet
 *
 * Return values:
 * - NULL: no interface has a matching VLAN ID, thus the packet must be dropped.
 * - != NULL: an interface (its pointer is returned) has a matching VLAN ID, thus the packet must be accepted.
 */

struct netif *lwip_hook_vlan_check_and_filter_fn(struct netif *netif, struct eth_hdr *eth_hdr, struct eth_vlan_hdr *vlan_hdr)
{
    struct netif *nif;

    /*
     * find if an interface has the matching IP address AND a matching VLAN ID
     */
    NETIF_FOREACH(nif) {
        /**
         * TO DO:
         *
         * add condition for matching IP address
         */
        if (nif->vlanid == lwip_ntohs((u16_t) (vlan_hdr->prio_vid)) )
        {
            SANE_DEBUGF(SANE_DBG_VLAN, ("lwip_hook_vlan_check_and_filter_fn: netif %c%c VLAN ID matches %d %d\n", 
                nif->name[0], nif->name[1], nif->vlanid, lwip_ntohs((u16_t) (vlan_hdr->prio_vid))));
            return nif;
        }
    }

    SANE_DEBUGF(SANE_DBG_VLAN, ("lwip_hook_vlan_check_and_filter_fn: netif %c%c VLAN ID no-match %d\n", 
        netif->name[0], netif->name[1], lwip_ntohs((u16_t) (vlan_hdr->prio_vid))));

    return NULL;
}

#endif /* LWIP_HOOK_VLAN_CHECK_AND_FILTER_FN */

#ifdef LWIP_HOOK_VLAN_SET

/**
 * lwip_hook_vlan_set_fn() is called from ethernet_output() if VLAN support is enabled (ETHARP_SUPPORT_VLAN defined).
 *
 * Signature:\code{.c}
 *   s32_t lwip_hook_vlan_set_fn(struct netif* netif, struct pbuf* pbuf, const struct eth_addr* src, const struct eth_addr* dst, u16_t eth_type);
 * \endcode
 *
 * Arguments:
 * - netif: struct netif that the packet will be sent through
 * - p: struct pbuf packet to be sent
 * - src: source eth address
 * - dst: destination eth address
 * - eth_type: ethernet type to packet to be sent\n
 *
 * Return values:
 * - < 0: Packet shall not contain VLAN header.
 * - 0 <= return value <= 0xFFFF: Packet shall contain VLAN header. Return value is prio_vid in host byte order.
 *
 */
s32_t lwip_hook_vlan_set_fn(struct netif* netif, struct pbuf* pbuf, const struct eth_addr* src, const struct eth_addr* dst, u16_t eth_type)
{
    if (netif->vlanid <= 0){
        SANE_DEBUGF(SANE_DBG_VLAN, ("lwip_hook_vlan_set_fn: netif %c%c VLAN disabled (%d)\n", 
                netif->name[0], netif->name[1], netif->vlanid));
        return -1;
    }

    SANE_DEBUGF(SANE_DBG_VLAN, ("lwip_hook_vlan_set_fn: netif %c%c packet set VLAN ID %d\n", 
                netif->name[0], netif->name[1], netif->vlanid));
    return netif->vlanid;
}

#endif /* LWIP_HOOK_VLAN_SET */

#endif /* #if ETHARP_SUPPORT_VLAN */