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
#include "netif/etharp.h"

#include "printf.h"

extern struct etharp_entry *get_arp_table();

static void cmdArp_show(void)
{
    printf_("%3s\t%15s\t%20s\t%8s\t%6s\t%5s\n", "No.", "IP Addr", "MAC Addr", "State", "Time", "Intf");
    printf_("---------------------------------------------------------------------------------\n");
    struct etharp_entry *ee;
    struct etharp_entry *arp_table = get_arp_table();
    int i, k, j;
    char ethaddr_buf[20];
    for (i = 0; i < ARP_TABLE_SIZE; i++){
        ee = &(arp_table[i]);

        if (ee->state == 0) continue;	

        printf_("%3d\t%15s\t", i, inet_ntoa(ee->ipaddr));

        for (k = 0; k <= 5; k++)
        {
            j = (ee->ethaddr.addr[k] & 0xf0) >> 4;
            ethaddr_buf[3*k] = (j>=0 && j <=9) ? (j + 0x30) : (j - 10 + 'a');

            j = ee->ethaddr.addr[k] & 0x0f;
            ethaddr_buf[3*k+1] = (j>=0 && j <=9) ? (j + 0x30) : (j - 10 + 'a');

            if (k < 5) ethaddr_buf[3*k+2] = ':';
        }
        ethaddr_buf[17] = '\0';
        printf_("%20s", ethaddr_buf);

        printf_("\t%8d\t%6d\t%c%c\n", 
            ee->state, ee->ctime, ee->netif->name[0], ee->netif->name[1] );
    }
}

#define ETHARP_FLAG_FIND_ONLY    2

uint8_t str_to_hex(char *str)
{
    uint8_t res;
    char c;
    c = str[0];
    res = (c - '0' < 10) ? (c - '0') : ( (c - 'A' < 6) ? (c - 'A' + 10) : (c - 'a' + 10));
    res = res * 10;

    c = str[1];
    res += (c - '0' < 10) ? (c - '0') : ( (c - 'A' < 6) ? (c - 'A' + 10) : (c - 'a' + 10));
    return res;
}

void extract_eth_addr(char *str, struct eth_addr *ea)
{
    int i = 0;
    char *token = strtok(str, ":");
    while (token != NULL && i < ETH_HWADDR_LEN) {
        ea->addr[i] = str_to_hex(token);
        token = strtok(NULL, ":");
        i += 1;
    }
}
void cmdArp(int argc, char *argv[]){
    if (argc == 1) {
        cmdArp_show();
        return;
    }
    if (strcmp(argv[1], "-d")== 0 && argc >= 3){
        // delete an entry
        ip_addr_t target;
        target.addr = inet_addr(argv[2]);
        int i = etharp_find_entry(&target, ETHARP_FLAG_FIND_ONLY, netif_default);
        if (i != ERR_MEM) {
            printf_("Deleting ARP entry for %s\n", inet_ntoa(target.addr));
            etharp_free_entry(i);
        }else{
            printf_("Deleting ARP entry fails. Error code = %d\n", i);
        }
    }else if (strcmp(argv[1], "-i") == 0) {
        // add an entry
        // To do: call etharp_update_arp_entry(struct netif *netif, const ip4_addr_t *ipaddr, struct eth_addr *ethaddr, u8_t flags)
        ip_addr_t target;
        target.addr = inet_addr(argv[2]);
        struct eth_addr ea;
        // parse mac addr
        extract_eth_addr(argv[3], &ea);
        printf_("ARP adding entry %s %s\n", inet_ntoa(target.addr), argv[3]);
        etharp_update_arp_entry(netif_default, &target, &ea, 2);
    }
}