/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef _LWIP_SHELL_H_
#define _LWIP_SHELL_H_

#include "lwip/opt.h"
#include "lwip/if_api.h"

#include "lwip/mem.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/api.h"
#include "lwip/stats.h"

#if LWIP_SOCKET
#include "lwip/errno.h"
#include "lwip/if_api.h"
#endif

struct command {
  struct netconn *conn;
  s8_t (* exec)(struct command *);
  u8_t nargs;
  char *args[10];
};

void sendstr(const char *str, struct netconn *conn);
s8_t com_open(struct command *com);
s8_t com_lstn(struct command *com);
s8_t com_clos(struct command *com);
s8_t com_acpt(struct command *com);

#if LWIP_STATS
s8_t com_stat(struct command *com);
#endif

s8_t com_send(struct command *com);
s8_t com_recv(struct command *com);
s8_t com_udpc(struct command *com);
s8_t com_udpl(struct command *com);
s8_t com_udpn(struct command *com);
s8_t com_udpb(struct command *com);
s8_t com_usnd(struct command *com);

#if LWIP_SOCKET
s8_t com_idxtoname(struct command *com);
s8_t com_nametoidx(struct command *com);
#endif /* LWIP_SOCKET */

#if LWIP_DNS
s8_t com_gethostbyname(struct command *com);
#endif /* LWIP_DNS */

s8_t com_help(struct command *com);

void lwip_shell_init(void);

#endif /* _LWIP_SHELL_H_ */
