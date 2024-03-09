#include "tcpecho.h"
#include "SoAd.h"
#include "SoAd_Cfg.h"
#include <string.h>

void tcpecho_client(char* server_ip, int server_port, char* content) {
    PduInfoType PduInfo;
    //PduInfo.SduLength
    //PduInfo.Meta

    if(server_port < 0) {
        printf("port number wrong\n");
        return;
    }

    char *token, *rest = server_ip;
    int i = 0, ip[4];
    while((token = strtok_r(rest, ".", &rest)) != NULL && i < 4) {
        ip[i++] = atoi(token);
    }

    if (i == 4){
        TcpIp_SockAddrType remote_addr;
        remote_addr.port = server_port;
        remote_addr.addr[0] = ip[0];
        remote_addr.addr[1] = ip[1];
        remote_addr.addr[2] = ip[2];
        remote_addr.addr[3] = ip[3];
        remote_addr.domain = TCPIP_AF_INET;

        SoAd_SetRemoteAddr(SOAD_SOCKID_TCPECHO_CLIENT, &remote_addr);
        PduInfo.MetaDataPtr = (uint8_t *)&remote_addr;
        PduInfo.SduLength = strlen(content);
        //printf("content length is %d\n", strlen(content));
    }
    else {
        printf("IP address format wrong\n");
        return ;
    }

    
    PduInfo.SduDataPtr = content;
    

    //create
    SoAd_OpenSoCon(SOAD_SOCKID_TCPECHO_CLIENT);
    //Then, MainFunction will bind
    SoAd_MainFunction();
    //? is this right?
    SoAd_IfTransmit(0, &PduInfo);
}

void tcpecho_server() {
    SoAd_OpenSoCon(SOAD_SOCKID_TCPECHO_SERVER);
}