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

#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "shell.h"

#include "ifconfig.h"

extern const ShellCmd SHELL_CMDS[];

void cmdPs(int argc, char* argv[]);
void cmdTop(int argc, char* argv[]);
void cmdPing(int argc, char* argv[]);
void cmdHelp(int argc, char* argv[]);
void cmdEcho(int argc, char* argv[]);
void cmdSetIp(int argc, char* argv[]);
void cmdSetIpQ(int argc, char* argv[]);
void cmdSetIpQQ(int argc, char* argv[]);
void cmdSet(int argc, char* argv[]);
void cmdSetMac(int argc, char* argv[]);

#ifndef USE_DDS
void cmdTcpEchoTest(int argc, char* argv[]);
void cmdRawUdpEchoTest(int argc, char* argv[]);
void cmdRawTcpEchoTest(int argc, char* argv[]);
void cmdRawMultiEchoTest(int argc, char* argv[]);
void cmdSdOpenService(int argc, char* argv[]);
void cmdSdOpenClient(int argc, char* argv[]);
void cmdSubscribe(int argc, char* argv[]);
void cmdRequest(int argc, char* argv[]);
void cmdFire(int argc, char* argv[]);
void cmdNotify(int argc, char* argv[]);
void cmdRun(int argc, char* argv[]);
void cmdDoip(int argc, char* argv[]);
void cmdDoipTester(int argc, char* argv[]);
void cmdGo(int argc, char* argv[]);
#else
    #ifdef USE_DETECTION
        void cmdWarn(int argc, char* argv[]);
        void cmdStartHandle(int argc, char* argv[]);
        void cmdStartReceiver(int argc, char* argv[]);
        void cmdHandleINFO(int argc, char* argv[]);
    #elif defined(USE_MEASUREMENT)
        void cmdInitGateway(int argc, char* argv[]);
        void cmdInitGatewayQ(int argc, char* argv[]);
        void cmdSend(int argc, char* argv[]);
    #else
        void cmdPublish(int argc, char* argv[]);
        void cmdSubscribe(int argc, char* argv[]);
        void cmdDDSPublish(int argc, char* argv[]);
        void cmdDDSSubscribe(int argc, char* argv[]);
    #endif
#endif
#endif