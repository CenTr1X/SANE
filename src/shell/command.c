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

#include "FreeRTOS.h"
#include "task.h"
#include "lwip/stats.h"
#include "lwip/inet.h"
#include "print.h"
#include "printf.h"
#include "command.h"
#include "debug.h"

#include "ifconfig.h"
#include "ping.h"
#include "vconf.h"

#include "net.h"
#include "cmdlwiperf.h"
#include "iperfapp.h"
#include "lan91c.h"

#include "tcpecho.h"
#include "rawtcpecho.h"
#include "rawudpecho.h"
#include "rawmultiecho.h"


#ifdef USE_SOMEIP
#include "SomeIpSd.h"
#include "SomeIp.h"
#include "SomeIp_Cfg.h"
#elif defined(USE_DOIP)
#include "DoIP.h"
#elif defined(USE_DDS)
  #if defined(USE_DETECTION)
    #include "app/detection.h"
  #elif defined(USE_MEASUREMENT)
    #include "measurement.h"
  #else
    #include "app/publisher.h"
    #include "app/subscriber.h"
  #endif
#endif

#define MAX_COMMAND_BUF_SIZE (5*1024)

static char buf[MAX_COMMAND_BUF_SIZE];

static struct netif *netif = &(gLan91c.netif[2]);

void cmdCmdTest(int argc, char* argv[]);
void ramDiskTestTask(void *pvParameters);
void cmdRamDiskTest(int argc, char* argv[]);
void cmdArp(int argc, char* argv[]);
void cmdNet(int argc, char* argv[]);
void cmdIperfServer(int argc, char *argv[]);
void cmdIperfApp(int argc, char *argv[]);

/****************************************************************************
 *
 ****************************************************************************/
const ShellCmd SHELL_CMDS[] =
{
#if 0
   {"pwd", "current directory", fsUtils_pwd},
   {"cd", "change directory", fsUtils_cd},
   {"ls", "list files in current directory", fsUtils_ls},
   {"cat", "display content of a file", fsUtils_cat},
   {"lsof", "list open file handlers", vfsInfo},
   {"ifconfig", "ifconfig <interface> <IP> <netmask> <gateway>", netIfconfig},
   {"ping", "ping <IP>", cmdPing},
#endif
   {"ps", "Display threads information", cmdPs},
   {"top", "Display threads runtime information", cmdTop},
   {"ifconfig", "show network interface information, and config interface", cmdIfconfig},
   {"vconf", "show / config vlan config for interfaces: vconf, vconf <intf> <VLAN ID>", cmdVconf},   
   {"ping", "ping <IP>", cmdPing},
   {"lwiperf", "iperf server mode (both TCP and UDP)", cmdIperfServer},
   {"iperfapp", "iperf server / client app (TCP only)", cmdIperfApp},
   {"arp", "show/update arp table: arp, arp -d [IP], arp -i IP MAC Intf", cmdArp},
   {"stat", "show usage information of memory heap, memory pool, and system sem/mutex/mbox", cmdStat},
   {"net", "lwIP utilities: open/lstn/acpt/send/recv/udpc/udpl/udpn/udpb/usnd/stat/idxtoname/nametoidx/gethostnm", cmdNet},
   {"cmdtest", "run batch of common commands", cmdCmdTest},
   {"ramdisktest", "run batch of RAM disk test", cmdRamDiskTest},
   {"help", "display help message", cmdHelp},
   {"echo", "echo input message", cmdEcho},
   {"setip", "simply set ip and gateway with /24 mask", cmdSetIp},
   {"setmac", "set mac addr", cmdSetMac},
#ifdef USE_SOMEIP 
   {"sdopens", "open service in SOME/IP-SD", cmdSdOpenService},
   {"sdopenc", "open client in SOME/IP-SD", cmdSdOpenClient},
   {"subscribe", "subscribe service with given eventgroup", cmdSubscribe},
   {"request", "send request msg to publisher", cmdRequest},
   {"fire", "send fireorforgot msg to publisher", cmdFire},
   {"run", "run measurement program", cmdRun},
   {"n", "send notification msg to client", cmdNotify},
   {"go", "open client and subscribe", cmdGo},
#elif defined(USE_DOIP)
   {"doip", "start doip as a client(gatway)", cmdDoip},
   {"tester", "start doip as a tester", cmdDoipTester},
#elif defined(USE_DDS)
#if defined(USE_DETECTION)
   {"INFO", "handle automatically sent info messages", cmdHandleINFO},
   {"warn", "send warning to handler via DDS", cmdWarn},
   {"start", "start handler to deal with warnings", cmdStartHandle},
   {"recv", "start receiver to handle hud info", cmdStartReceiver},
#elif defined(USE_MEASUREMENT)
   {"init", "init gateway in measurement application", cmdInitGateway},
   {"iq", "init gateway in measurement application quick", cmdInitGatewayQ},
#ifndef USE_MULTI
   {"s", "send dds data from expect", cmdSend},
#endif
#else
   {"publish", "use dds to publish something", cmdPublish},
   {"subscribe", "use dds to subscribe something", cmdSubscribe},
   {"sub", "subscribe in DDS", cmdDDSSubscribe},
   {"pub", "quick set ip with 10.0.2.5/24 for l2 and publish in DDS", cmdDDSPublish},
#endif
#else
   {"tcpecho", "build tcpecho server or client to test connection", cmdTcpEchoTest},
#endif

#ifndef USE_DDS

   {"rawtcpecho", "using raw LWIP POSIX api to do tcpecho instead of SoAd", cmdRawTcpEchoTest},
   {"rawudpecho", "using raw LWIP POSIX api to do udpecho instead of SoAd", cmdRawUdpEchoTest},
   {"rawmultiecho", "using raw LWIP POSIX api to do udpecho using multicast", cmdRawMultiEchoTest},
#endif
    {"setipq", "quick set ip with 10.0.0.5/24 for l0", cmdSetIpQ},
    {"setipqq", "quick set ip with 10.0.2.5/24 for l2", cmdSetIpQQ},
    {"set", "set ip for specified interface", cmdSet},
   {NULL, NULL}
};

void vTaskCmdTest(void *pvParameters){

    char buf[1024];

    int i;
    int maxtests = 20;

    for (i=0; i< maxtests; i++){
        int k = xTaskGetTickCount() % 4;

        printf_("---------------------------------\n");
        printf_("i = %d, k = %d\n", i, k);
        printf_("---------------------------------\n");

        switch(k){
            case 0:
                // ps
                vTaskList(buf);
                printf_("Thread Name\tState\tPrio\tRStack\tThreadID\n");
                printf_("------------------------------------------------\n");
                printf_(buf);
                break;
            case 1:
                // top
                vTaskGetRunTimeStats(buf);
                printf_("Thread Name\tRuntime\t\t%%CPU\n");
                printf_("------------------------------------\n");
                printf_("%s", buf);
                break;
            case 2:
                // ifconfig
                stats_display();
                break;
            case 3:
                // ifconfig
                cmdIfconfig(0, NULL);
                break;
        }
        vTaskDelay( 10 / portTICK_RATE_MS );
    }

    printf_("\nvTaskTest ends.\n");

    vTaskDelete(NULL);

    /* suppress a warning since 'params' is ignored */
    (void) pvParameters;
}

void cmdCmdTest(int argc, char* argv[]){
    if ( pdPASS != xTaskCreate(vTaskCmdTest, "CmdTest", 1024, NULL, 2, NULL) )
    {
        SANE_PLATFORM_ERROR(("Could not create CmdTest task\r\n"));
    }
}

void cmdRamDiskTest(int argc, char* argv[]){
    if ( pdPASS != xTaskCreate(ramDiskTestTask, "DiskTest", 1024, NULL, 2, NULL) )
    {
        SANE_PLATFORM_ERROR(("Could not create DiskTest task\r\n"));
    }
}

/*
 * the following three FreeRTOS macros need to be defined in order to call vTaskList()
 *
 * configGENERATE_RUN_TIME_STATS
 * configUSE_TRACE_FACILITY
 * configUSE_STATS_FORMATTING_FUNCTIONS
 */

void cmdPs(int argc, char* argv[])
{
    vTaskList(buf); 
    printf_("Thread Name\tState\tPrio\tRStack\tThreadID\n");
    printf_("------------------------------------------------\n");
    printf_(buf);
}

/*
 * the following three FreeRTOS macros need to be defined in order to call vTaskGetRunTimeStats()
 *
 * configGENERATE_RUN_TIME_STATS
 * configSUPPORT_DYNAMIC_ALLOCATION
 * configUSE_STATS_FORMATTING_FUNCTIONS
 */

void cmdIperfServer(int argc, char *argv[])
{

    lwip_iperf_dual_server_init();

}

void cmdIperfApp(int argc, char *argv[])
{
  cmd_iperfapp(argc, argv);

}

void cmdTop(int argc, char* argv[])
{
    vTaskGetRunTimeStats(buf); 
    printf_("Thread Name\tRuntime\t\t%%CPU\n");
    printf_("------------------------------------\n");
    printf_("%s", buf);
}

void cmdPing(int argc, char *argv[])
{
   if (argc != 2) {
      printf("Usage: ping [destination]\n");
      return;
   }

   ip_addr_t target;
   target.addr = inet_addr(argv[1]);
   ping_thread((void *)&target);
}

static struct command netcommands;

void cmdNet(int argc, char *argv[])
{
    struct command *com = &netcommands;
    int i;

    if (strncmp((const char *)argv[1], "open", 4) == 0) {
    com->exec = com_open;
    com->nargs = 2;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

  } else if (strncmp((const char *)argv[1], "lstn", 4) == 0) {
    com->exec = com_lstn;
    com->nargs = 1;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

  } else if (strncmp((const char *)argv[1], "acpt", 4) == 0) {
    com->exec = com_acpt;
    com->nargs = 1;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

  } else if (strncmp((const char *)argv[1], "clos", 4) == 0) {
    com->exec = com_clos;
    com->nargs = 1;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

#if LWIP_STATS
  } else if (strncmp((const char *)argv[1], "stat", 4) == 0) {
    com->exec = com_stat;
    com->nargs = 0;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];
#endif
  } else if (strncmp((const char *)argv[1], "send", 4) == 0) {
    com->exec = com_send;
    com->nargs = 2;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

  } else if (strncmp((const char *)argv[1], "recv", 4) == 0) {
    com->exec = com_recv;
    com->nargs = 1;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

  } else if (strncmp((const char *)argv[1], "udpc", 4) == 0) {
    com->exec = com_udpc;
    com->nargs = 3;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

  } else if (strncmp((const char *)argv[1], "udpb", 4) == 0) {
    com->exec = com_udpb;
    com->nargs = 2;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

  } else if (strncmp((const char *)argv[1], "udpl", 4) == 0) {
    com->exec = com_udpl;
    com->nargs = 3;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

  } else if (strncmp((const char *)argv[1], "udpn", 4) == 0) {
    com->exec = com_udpn;
    com->nargs = 3;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

  } else if (strncmp((const char *)argv[1], "usnd", 4) == 0) {
    com->exec = com_usnd;
    com->nargs = 2;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

#if LWIP_SOCKET
  } else if (strncmp((const char *)argv[1], "idxtoname", 9) == 0) {
    com->exec = com_idxtoname;
    com->nargs = 1;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];

  } else if (strncmp((const char *)argv[1], "nametoidx", 9) == 0) {
    com->exec = com_nametoidx;
    com->nargs = 1;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];
#endif /* LWIP_SOCKET */
#if LWIP_DNS
  } else if (strncmp((const char *)argv[1], "gethostnm", 9) == 0) {
    com->exec = com_gethostbyname;
    com->nargs = 1;
    for (i = 0; i < com->nargs && i + 2 < argc; i++)
        com->args[i] = argv[i+2];
#endif /* LWIP_DNS */
  } else if (strncmp((const char *)argv[1], "help", 4) == 0) {
    com->exec = com_help;
    com->nargs = 0;
  } else {
    return;
  }

  com->conn = NULL;
  //com->conn = netconn_new(NETCONN_TCP);
  com->exec(com);

}

void cmdHelp(int argc, char *argv[])
{
   printf_("-----------------------------------------------------------------------------------\n");
   printf_("%16s\tUsage\n", "Command");
   printf_("-----------------------------------------------------------------------------------\n");
   for (int i = 0; SHELL_CMDS[i].name != NULL; i++)
   {
      printf_("%16s\t%s\n", SHELL_CMDS[i].name, SHELL_CMDS[i].helpmsg);
   }
}

void cmdEcho(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++)
  {
    printf_("%s ", argv[i]);
  }
  printf_("\n");
}

void cmdSetIp(int argc, char *argv[])
{
  if (argc != 4){
    printf_("Usage: setip <intf> <IP> <gateway>");
    return ;
  }
  
  int new_argc = 7;
  char * new_argv[7];

  new_argv[0] = "ifconfig";
  new_argv[1] = argv[1]; //intf
  new_argv[2] = argv[2]; //ip
  new_argv[3] = "netmask";
  new_argv[4] = "255.255.255.0";
  new_argv[5] = "gw";
  new_argv[6] = argv[3];

  cmdIfconfig(new_argc, new_argv);
}


#ifndef USE_DDS
void cmdRawTcpEchoTest(int argc, char* argv[])
{
  if (argc == 2 && strcmp(argv[1], "c") == 0)
  {
    raw_tcpecho_client();
  }
  else if (argc == 2 && strcmp(argv[1], "s") == 0)
  {
    raw_tcpecho_server();
  }
}

void cmdRawUdpEchoTest(int argc, char* argv[])
{
  if (argc == 2 && strcmp(argv[1], "c") == 0)
  {
    raw_udpecho_client();
  }
  else if (argc == 2 && strcmp(argv[1], "s") == 0)
  {
    raw_udpecho_server();
  }
}

void cmdRawMultiEchoTest(int argc, char* argv[])
{
  if (argc == 2 && strcmp(argv[1], "c") == 0)
  {
    raw_multiecho_client();
  }
  else if (argc == 2 && strcmp(argv[1], "s") == 0)
  {
    raw_multiecho_server();
  }
}
#endif

#ifdef USE_SOMEIP
void cmdSdOpenService(int argc, char* argv[])
{
  if (argc == 1)
  {
    Sd_ServerServiceSetState(0, SD_SERVER_SERVICE_AVAILABLE);
  }
  else if (argc == 2)
  {
    uint16_t service_id = atoi(argv[1]);
    Sd_ServerServiceSetState(service_id, SD_SERVER_SERVICE_AVAILABLE);
  }
  else 
  {
    printf("Usage: sdopens <service_id>\n");
  }
}

void cmdSdOpenClient(int argc, char* argv[])
{
  if (argc == 1)
  {
    Sd_ClientServiceSetState(0, SD_CLIENT_SERVICE_REQUESTED);
  }
  else if (argc == 2)
  {
    uint16_t service_id = atoi(argv[1]);
    Sd_ClientServiceSetState(service_id, SD_CLIENT_SERVICE_REQUESTED);
  }
  else 
  {
    printf("Usage: sdopenc <service_id>\n");
  }
}

void cmdSubscribe(int argc, char* argv[])
{
  if (argc == 1)
  {
    Sd_ConsumedEventGroupSetState(0, SD_CONSUMED_EVENTGROUP_REQUESTED);
  }
  else if (argc == 2)
  {
    uint16_t eventgroup_id = atoi(argv[1]);
    Sd_ConsumedEventGroupSetState(eventgroup_id, SD_CONSUMED_EVENTGROUP_REQUESTED);
  }
  else
  {
    printf("Usage: subscribe <eventGroupId>\n");
  }
}

void cmdGo(int argc, char* argv[]) {
  cmdSdOpenClient(1, NULL);
  vTaskDelay(200);
  cmdSubscribe(1, NULL);
}

static uint32_t sessionId = 0;

void cmdRequest(int argc, char* argv[])
{
  if (argc == 1)
  {
    uint32_t requestId = ((uint32_t)SOMEIP_TX_METHOD_CLIENT0_METHOD2 << 16) + (++sessionId);
    char buf[] = "testtest1234";
    SomeIp_Request(requestId, (uint8_t *)buf, sizeof(buf));
  }
  else if (argc == 2)
  {
    
    
  }
  else
  {
    printf("Usage: request <ServiceId> <MethodId> <data>\n");
  }
}

void cmdFire(int argc, char* argv[])
{
  if (argc == 1)
  {
    uint32_t requestId = ((uint32_t)SOMEIP_TX_METHOD_CLIENT0_METHOD2 << 16) + (++sessionId);
    char buf[] = "testtest1234";
    SomeIp_FireForgot(requestId, (uint8_t *)buf, sizeof(buf));
  }
  else if (argc == 2)
  {
    uint32_t requestId = ((uint32_t)SOMEIP_TX_METHOD_CLIENT0_METHOD2 << 16) + (++sessionId);
    //printf("tick:%d\n", xTaskGetTickCount());//as random seed
    srand(xTaskGetTickCount());
    int max_length = atoi(argv[1]);
    int length = rand() % max_length;
    printf("maxlength:%d length:%d\n",max_length, length); 
    char *buf = malloc(length);
    memset(buf, 'a', length);
    char output[16];
    SomeIp_FireForgot(requestId, (uint8_t *)buf, length);
  }
  else
  {
    printf("Usage: fire <ServiceId> <MethodId> <data>\n");
  }
}

// void cmdNotify(int argc, char *argv[])
// {
//   if (argc == 1)
//   {
//     uint32_t requestId = ((uint32_t)SOMEIP_TX_METHOD_CLIENT0_METHOD2 << 16) + (++sessionId);
//     char buf[] = "testtest1234";
//     SomeIp_Notification(requestId, (uint8_t *)buf, sizeof(buf));
//   }
//   else if (argc == 4)
//   {
//     uint32_t requestId = ((uint32_t)SOMEIP_TX_METHOD_CLIENT0_METHOD2 << 16) + (++sessionId);
//     int size1 = strlen(argv[1]) + 1; //location and \00
//     int size2 = strlen(argv[2]) + 1; //description and \00
//     uint8_t buf[1+size1+size2];
//     memset(buf, 0, 1+size1+size2);
//     buf[0] = (uint8_t)atoi(argv[3]); //level
//     strcpy(buf+1, argv[1]);
//     strcpy(buf+1+size1, argv[2]);
//     SomeIp_Notification(requestId, (uint8_t *)buf, sizeof(buf));
//   }
// }

void cmdNotify(int argc, char *argv[])
{
  if (argc == 2)
  {
    uint32_t requestId = ((uint32_t)SOMEIP_TX_METHOD_CLIENT0_METHOD2 << 16) + (++sessionId);
    int buf;
    buf = atoi(argv[1]); //level
    vPortEnterCritical();
    printf("ssend no.%d now : %d \n", buf, xTaskGetTickCount());
    vPortExitCritical();
    SomeIp_Notification(requestId, (uint8_t *)&buf, sizeof(int));
  }
}

// void cmdRunDetection(int argc, char* argv[])
// {
//   uint8_t *buf = malloc(1+sizeof(double));
//   double percentage = strtod(argv[2], NULL);
//   buf[0] = (uint8_t)atoi(argv[1]); // Number of tire, from 0 to 3
//   sprintf(buf+1, "%f", percentage);
//   uint32_t requestId = ((uint32_t)SOMEIP_TX_METHOD_CLIENT0_METHOD2 << 16) + (++sessionId);
//   SomeIp_FireForgot(requestId, (uint8_t *)buf, sizeof(buf));
// }

void cmdRun(int argc, char* argv[])
{
  uint32_t requestId = ((uint32_t)SOMEIP_TX_METHOD_CLIENT0_METHOD2 << 16) + (++sessionId);
  // for(int i = 0; i < 300; i++) {
  //   SomeIp_FireForgot(requestId, (uint8_t *)&i, sizeof(i));
  //   vPortEnterCritical();
  //   printf("ssend no.%d now:%d \n", i, xTaskGetTickCount());
  //   vPortExitCritical();
  //   vTaskDelay(150);
  // }
  for(int i = 0; i < 300; i++) {
    vPortEnterCritical();
    SomeIp_FireForgot(requestId, (uint8_t *)&i, sizeof(i));
    printf("ssend no.%d now : %d \n", i, xTaskGetTickCount());
    vPortExitCritical();
    vTaskDelay(150);
  }
}

#elif defined(USE_DOIP)
void cmdDoip(int argc, char* argv[])
{
  DoIP_ActivationLineSwitchActive();
}

void cmdDoipTester(int argc, char* argv[])
{
  doip_tester();
}
#elif defined(USE_DDS)
#if defined(USE_DETECTION)
void cmdWarn(int argc, char* argv[])
{
  if(argc == 4)
  {
    printf("warn!!!");
    publish_warning(atoi(argv[1]), argv[2], atoi(argv[3]));
  }
}

void cmdHandleINFO(int argc, char* argv[])
{
  if(argc == 3)
  {
    if(!strcmp(argv[2], "FLAT")) //level: critical
    {
      publish_warning(atoi(argv[1]), "FLAT TIRE", 3);
    }
    else if(!strcmp(argv[2], "LOW")) //level: high
    {
      publish_warning(atoi(argv[1]), "TOO LOW", 2);
    }
  }
  else
  {
    printf("bad info!\n");
  }
}

void cmdStartHandle(int argc, char* argv[])
{
  start_handler();
}

void cmdStartReceiver(int argc, char* argv[])
{
  start_receiver();
}
#elif defined(USE_MEASUREMENT)
void cmdInitGateway(int argc, char* argv[])
{
  parse_config(atoi(argv[1]));
}

void cmdInitGatewayQ(int argc, char* argv[])
{
  if(atoi(argv[1])==0) {
    argv[1] = "234";
    cmdSetIpQQ(2, argv);
    parse_config(0);
  } else {
    cmdSetIpQQ(2, argv);
    parse_config(atoi(argv[1]));
  }
  
}

#ifndef USE_MULTI
//s <domainid> <no>
void cmdSend(int argc, char* argv[])
{
  if(argc == 3) {
    int domain_id = atoi(argv[1]);
    int no = atoi(argv[2]);
    int index = find_config(domain_id);
    if (index == -1) {
      printf("index not found\n");
    } else {
      send_from_cmd(index, no);
    }
  } else if (argc == 2) {
    printf("cmdSend called123!\n");
    int domain_id = 0;
    int no = atoi(argv[1]);
    int index = find_config(domain_id);
    if (index == -1) {
      printf("index not found\n");
    } else {
      send_from_cmd(index, no);
    }
  }
}
#endif


#else
void cmdPublish(int argc, char* argv[])
{
  printf("cmdPublish\n");
  dds_publish();
}

void cmdSubscribe(int argc, char* argv[])
{
  printf("cmdSubscribe\n");
  dds_subscribe();
}

void cmdDDSPublish(int argc, char* argv[])
{
  cmdSetIpQQ(0, NULL);
  cmdPublish(0, NULL);
}

void cmdDDSSubscribe(int argc, char* argv[])
{
  cmdSubscribe(0, NULL);
}
#endif
#else
void cmdTcpEchoTest(int argc, char* argv[])
{
  if ((argc != 5 && argc != 2) || argc < 2) {
    printf("Usage: tcpecho c <server_ip> <server_port> <content> or tcpecho s\n");
  }
  else if (strcmp(argv[1], "c") == 0 && argc == 5) {
    printf("start as a tcpecho client\n");
    tcpecho_client(argv[2], atoi(argv[3]), argv[4]);
  }
  else if (strcmp(argv[1], "c") == 0 && argc == 2) {
    tcpecho_client("10.0.0.5", 30490, "testtesttest1234");
  }
  else if (strcmp(argv[1], "s") == 0 && argc == 2) {
    printf("start as a tcpecho server\n");
    tcpecho_server();
  }
  else {
    printf("Usage: tcpecho c <server_ip> <server_port> <content> or tcpecho s\n");
  }
}
#endif

void cmdSetIpQ(int argc, char* argv[])
{
  int new_argc = 7;
  char * new_argv[7];

  new_argv[0] = "ifconfig";
  new_argv[1] = "l0"; //intf
  new_argv[2] = "10.0.0.5"; //ip
  new_argv[3] = "netmask";
  new_argv[4] = "255.255.255.0";
  new_argv[5] = "gw";
  new_argv[6] = "10.0.0.1";

  cmdIfconfig(new_argc, new_argv);
}

void cmdSetIpQQ(int argc, char* argv[])
{
  int new_argc = 7;
  char * new_argv[7];

  new_argv[0] = "ifconfig";
  new_argv[1] = "l2"; //intf
  
  new_argv[3] = "netmask";
  new_argv[4] = "255.255.255.0";
  new_argv[5] = "gw";
  new_argv[6] = "10.0.2.1";

  if(argc == 2) {
    new_argv[2] = strcat("10.0.2.", argv[1]);
  } else {
    new_argv[2] = "10.0.2.5"; //ip
  }

  cmdIfconfig(new_argc, new_argv);
}

void cmdSet(int argc, char* argv[])
{
  if (argc != 3) return;
  int new_argc = 7;
  char * new_argv[7];

  new_argv[0] = "ifconfig";
  new_argv[1] = argv[1]; //intf
  new_argv[2] = argv[2]; //ip
  new_argv[3] = "netmask";
  new_argv[4] = "255.255.255.0";
  new_argv[5] = "gw";
  new_argv[6] = "10.0.2.1";

  cmdIfconfig(new_argc, new_argv);
}

void cmdSetMac(int argc, char* argv[])
{
  if(argc != 2) return;
  int num = atoi(argv[1]);
  netif->hwaddr[5] = num;
}