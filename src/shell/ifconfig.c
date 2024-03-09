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

// NETIF_FOREACH
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "lwip/mem.h"
#include "lwip/stats.h"

#include "lan91c.h"

#if LWIP_STATS

static char padding_10spaces[] = "          ";

#define NEWLINE "\n"

#define PROTOCOL_STATS (LINK_STATS && ETHARP_STATS && IPFRAG_STATS && IP_STATS \
                        && ICMP_STATS && UDP_STATS && TCP_STATS && IGMP_STATS)

static const char* shell_stat_proto_names[] = {
#if LINK_STATS
  "LINK      ",
#endif
#if ETHARP_STATS
  "ETHARP    ",
#endif
#if IPFRAG_STATS
  "IP_FRAG   ",
#endif
#if IP_STATS
  "IP        ",
#endif
#if ICMP_STATS
  "ICMP      ",
#endif
#if UDP_STATS
  "UDP       ",
#endif
#if TCP_STATS
  "TCP       ",
#endif
#if IGMP_STATS
  "IGMP       ",
#endif
  "last"
};

static struct stats_proto* shell_stat_proto_stats[] = {
#if LINK_STATS
  &lwip_stats.link,
#endif
#if ETHARP_STATS
  &lwip_stats.etharp,
#endif
#if IPFRAG_STATS
  &lwip_stats.ip_frag,
#endif
#if IP_STATS
  &lwip_stats.ip,
#endif
#if ICMP_STATS
  &lwip_stats.icmp,
#endif
#if UDP_STATS
  &lwip_stats.udp,
#endif
#if TCP_STATS
  &lwip_stats.tcp,
#endif
#if IGMP_STATS
  &lwip_stats.igmp,
#endif
};

const size_t num_protostats = sizeof(shell_stat_proto_stats) / sizeof(struct stats_proto*);

struct stat_msgs_proto_type {
    char name[32];
    char message[64];
};

static struct stat_msgs_proto_type stat_msgs_proto[] = {
  {"xmit", " * transmitted "},
  {"recv", " * received "},
  {"fw", " * forwarded "},
  {"drop", " * dropped "},
  {"chkerr", " * checksum errors "},
  {"lenerr", " * length errors "},
  {"memerr", " * memory errors "},
  {"rterr", " * routing errors "},
  {"proterr", " * protocol errors "},
  {"opterr", " * option errors "},
  {"err", " * misc errors "},
  {"cachehit", " * cache hits "}
};

static void print_all_proto_stats(char *key)
{
    int k;
    size_t s = sizeof(shell_stat_proto_names)/sizeof(char *);
    LWIP_ASSERT("stats not in sync", s == num_protostats + 1);

    for(k = 0; k < num_protostats  && k < s; k++) {

        if (strcmp(key, "xmit") == 0)
            if (k == num_protostats - 1)
                printf_("%10"STAT_COUNTER_F, lwip_stats.igmp.xmit);
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->xmit);
        else if (strcmp(key, "recv") == 0)
            if (k == num_protostats - 1)
                printf_("%10"STAT_COUNTER_F, lwip_stats.igmp.recv);
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->recv);
        else if (strcmp(key, "fw") == 0)
            if (k == num_protostats - 1)
                printf_(" ");
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->fw);
        else if (strcmp(key, "drop") == 0)
            if (k == num_protostats - 1)
                printf_("%10"STAT_COUNTER_F, lwip_stats.igmp.drop);
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->drop);
        else if (strcmp(key, "chkerr") == 0)
            if (k == num_protostats - 1)
                printf_("%10"STAT_COUNTER_F, lwip_stats.igmp.chkerr);
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->chkerr);
        else if (strcmp(key, "lenerr") == 0)
            if (k == num_protostats - 1)
                printf_("%10"STAT_COUNTER_F, lwip_stats.igmp.lenerr);
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->lenerr);
        else if (strcmp(key, "memerr") == 0)
            if (k == num_protostats - 1)
                printf_("%10"STAT_COUNTER_F, lwip_stats.igmp.memerr);
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->memerr);
        else if (strcmp(key, "rterr") == 0)
           if (k == num_protostats - 1)
                printf_(" ");
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->rterr);
        else if (strcmp(key, "proterr") == 0)
            if (k == num_protostats - 1)
                printf_("%10"STAT_COUNTER_F, lwip_stats.igmp.proterr);
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->proterr);
        else if (strcmp(key, "opterr") == 0)
           if (k == num_protostats - 1)
                printf_(" ");
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->opterr);
        else if (strcmp(key, "err") == 0)
           if (k == num_protostats - 1)
                printf_(" ");
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->err);
        else if (strcmp(key, "cachehit") == 0)
           if (k == num_protostats - 1)
                printf_(" ");
            else
                printf_("%10"STAT_COUNTER_F, shell_stat_proto_stats[k]->cachehit);
    }
}

static void _netif_stat_proto()
{
#if PROTOCOL_STATS
  size_t i, k;
  char buf[100];
  u16_t len;

  printf_("\nProtocol statistics:\n");

  // print shell_stat_proto_names
  for(i = 0; i < num_protostats; i++) {
     if (i==0) printf("%25s", " ");
     printf_("%+10s", shell_stat_proto_names[i]);
  }
  printf_("\n");

  // print stat_msgs_proto
  int n_stat_msg = sizeof(stat_msgs_proto)/sizeof(struct stat_msgs_proto_type);
  for(i = 0; i < n_stat_msg; i++) {
     printf_("%20s", stat_msgs_proto[i].message);
     
    print_all_proto_stats(stat_msgs_proto[i].name);

    printf_("\n");
  }
#endif
}

static void _stat_mem(char *key)
{
    struct stats_mem *elem;

    if (strcmp(key, "mem") == 0){

#if MEM_STATS
        printf("\nMem statistics:\n");
        printf_("-----------------------------------------------------------------------\n");
        printf_("%15s\t\tAvail\tUsed\tHigh-Water-Mark\tError\tIllegal\n", "Name");
        printf_("-----------------------------------------------------------------------\n");
        elem = &lwip_stats.mem;
        printf_("%15s\t\t%"MEM_SIZE_F "\t%"MEM_SIZE_F "\t\t%"MEM_SIZE_F "\t%"STAT_COUNTER_F "\t%"STAT_COUNTER_F "\n",
#if defined(LWIP_DEBUG) || LWIP_STATS_DISPLAY
                elem->name,
#else
                "HEAP",
#endif
                elem->avail, elem->used, elem->max, elem->err, elem->illegal);
#endif // MEM_STATS

    }else if (strcmp(key, "mempool") == 0) 
    {

#if MEMP_STATS
        printf("\nMempool statistics:\n");
        printf_("-----------------------------------------------------------------------\n");
        printf_("%15s\t\tAvail\tUsed\tHigh-Water-Mark\tError\tIllegal\n", "Name");
        printf_("-----------------------------------------------------------------------\n");
   
        for(int i = 0; i < MEMP_MAX; i++) {
            elem = lwip_stats.memp[i];
            printf_("%15s\t\t%"MEM_SIZE_F "\t%"MEM_SIZE_F "\t\t%"MEM_SIZE_F "\t%"STAT_COUNTER_F "\t%"STAT_COUNTER_F "\n",
#if defined(LWIP_DEBUG) || LWIP_STATS_DISPLAY
                elem->name,
#else
                "HEAP",
#endif
                elem->avail, elem->used, elem->max, elem->err, elem->illegal);
        } // for
#endif // MEMP_STATS

    }else{

        return;

    }
}

static void _stat_sys()
{
#if SYS_STATS
   struct stats_syselem *elem;
   printf_("\nSystem statistics:\n");
   printf_("---------------------------------------------------------\n");
   printf_("%15s\t\tUsed\tHigh Water Mark\tErrors\n", "Name");
   printf_("---------------------------------------------------------\n");
   elem = &lwip_stats.sys.sem;
   printf_("%15s\t\t%"STAT_COUNTER_F "\t\t%"STAT_COUNTER_F "\t%"STAT_COUNTER_F "\n",
            "SEM", elem->used, elem->max, elem->err);
   elem = &lwip_stats.sys.mutex;
   printf_("%15s\t\t%"STAT_COUNTER_F "\t\t%"STAT_COUNTER_F "\t%"STAT_COUNTER_F "\n",
            "MUTEX", elem->used, elem->max, elem->err);
   elem = &lwip_stats.sys.mbox;
   printf_("%15s\t\t%"STAT_COUNTER_F "\t\t%"STAT_COUNTER_F "\t%"STAT_COUNTER_F "\n",
            "MBOX", elem->used, elem->max, elem->err);
#endif
}

#endif

static void _netif_list(struct netif *netif) {
    int i;
    char name[8];

    memset(name, 0, sizeof(name));

    strncpy(name, netif->name, sizeof(netif->name));
    printf_("Iface %s:\n", name);
    printf_("\tIP %s ", inet_ntoa(netif->ip_addr));
    printf_("Netmask %s ", inet_ntoa(netif->netmask));
    printf_("Gateway %s ", inet_ntoa(netif->gw));
    printf_("\n");
    printf_("\tMTU %d ", netif->mtu);
    printf_("\tHWaddr: ");
    for (i = 0; i < netif->hwaddr_len; i++) {
        printf("%02x", netif->hwaddr[i]);
        if ((i+1) < netif->hwaddr_len) {
            printf_(":");
        }
    }
    printf_("\n");
    printf_("\tLink: %s\tState: %s\t",
        netif_is_link_up(netif) ? "UP" : "DOWN",
        netif_is_up(netif) ? "UP" : "DOWN");
    printf_("Link type: %s\n", "UNKNOWN");

#if ETHARP_SUPPORT_VLAN
    /*
     * find vlan ID
     */
    printf_("\tVLAN ID: %d\n", netif->vlanid);
#endif /* #if ETHARP_SUPPORT_VLAN */

    printf_("\n");
}

#if LWIP_SINGLE_NETIF
#define NETIF_FOREACH(netif) if (((netif) = netif_default) != NULL)
#else /* LWIP_SINGLE_NETIF */
/** The list of network interfaces. */
extern struct netif *netif_list;
#define NETIF_FOREACH(netif) for ((netif) = netif_list; (netif) != NULL; (netif) = (netif)->next)
#endif /* LWIP_SINGLE_NETIF */

int cmdIfconfig(int argc, char **argv)
{
    if (argc < 2) {
        /* List in interface order, which is normally reverse of list order */
        struct netif *netif = &(gLan91c.netif[0]);
        int netifs = 0;
        int listed = 0;
        u8_t i;
        NETIF_FOREACH(netif) netifs++;
        for (i = 0; listed < netifs; i++) {
            NETIF_FOREACH(netif) {
                if (i == netif->num) {
                    _netif_list(netif);
                    listed++;
                }
            }
        }
#if LWIP_STATS
//stats_display();
        _netif_stat_proto();
#endif
        return 0;

    }else if (argc == 7 && strcmp(argv[3], "netmask") == 0 && strcmp(argv[5], "gw") == 0 ){
      /*
       * example:
       *  ifconfig l0 10.0.2.10 netmask 255.255.255.0 gw 10.0.2.1 
       */

      struct netif *netif = &(gLan91c.netif[0]);
      ip_addr_t ipaddr, netmask, gw;

      ipaddr.addr = inet_addr(argv[2]);
      netmask.addr = inet_addr(argv[4]);
      gw.addr = inet_addr(argv[6]);

      NETIF_FOREACH(netif) {
        if (netif->name[0] == argv[1][0] && netif->name[1] == argv[1][1])
        {
          LOCK_TCPIP_CORE();
    
          netif_set_addr(netif, &ipaddr, &netmask, &gw);

          UNLOCK_TCPIP_CORE();
          return 0;
        }
      }
      printf_("Error: cannot find interface %s\n", argv[1]);
      return 1;

    } else {
      printf_("Usage: %s <intf> <IP> netmask <network mask> gw <gateway>\n", argv[0]);
      return 1;
    }

}

void cmdStat(int argc, char *argv[])
{
#if LWIP_STATS
    _stat_mem("mem");
    _stat_mem("mempool");
    _stat_sys();
#endif
}
