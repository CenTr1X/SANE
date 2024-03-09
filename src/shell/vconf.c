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

#include "lwip/netif.h"
#include "lwip/inet.h"
#include "lwip/mem.h"
#include "lwip/stats.h"

#include "lan91c.h"

int find_netif_index(struct netif *netif)
{
    LAN91C *lan91c = &gLan91c;
    int i;

    for (i = 0; i < LAN91C_MAX_NETIF; i++)
    {
        if (&(lan91c->netif[i]) == netif)
            return i;
    }
    LWIP_ASSERT("vconf: interface is not in gLan91c.netif[]\n", i < LAN91C_MAX_NETIF);

    return -1;
}

void cmdVconf(int argc, char *argv[])
{
    LAN91C *lan91c = &gLan91c;
    struct netif *netif;

    int i;

#if ETHARP_SUPPORT_VLAN

    if (argc == 1) 
    {
        /* show current VLAN config for interfaces */
        i = 0;
        printf_("VLAN Configuration for interfaces (VLAN ID = 0 means unconfigured):\n");

        NETIF_FOREACH(netif) {
            if (netif->name[0]=='l' && netif->name[1]=='o')
                /* skip loopback interface */
                continue;

            i = netif->vlanid;
            if (i > 0)
                printf_("\tIntf %c%c VLAN ID %d\n", netif->name[0], netif->name[1], i);
            else
                printf_("\tIntf %c%c VLAN ID %d (default VLAN ID = 0, unconfigured)\n", netif->name[0], netif->name[1], i);
        }
        return;
    }else if (argc == 3){
        /* vconf <intf> <VLAN ID> */
        i = 0;
        NETIF_FOREACH(netif) {
            if (netif->name[0] == argv[1][0] && netif->name[1] == argv[1][1])
            {
                int id = atoi(argv[2]);
                LWIP_ASSERT("vconf: VLAN ID is not in proper range\n", id > 0 && id <= 4095);

                printf_("vconf: set %c%c vlan id to %d\n", netif->name[0], netif->name[1], id);
                
                netif->vlanid = id;

                return;
            }
            i += 1;
        }
        printf_("Error: cannot find network interface %c%c.\n", argv[1][0], argv[1][1]);
        return;
    }else {
        printf_("Usage:\n vconf \t show current VLAN configuration\n\tvonf <intf> <VLAN ID> \t set an interface's VLAN ID\n");
        return;
    }
#else
    printf_("%s is not supported due to ETHARP_SUPPORT_VLAN is turned off.\n", argv[0]);
    printf_("Usage:\n vconf \t show current VLAN configuration\n\tvonf <intf> <VLAN ID> \t set an interface's VLAN ID\n");
#endif /* #if ETHARP_SUPPORT_VLAN */

}
