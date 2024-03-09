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

#include <stddef.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

/* FreeRTOS+FAT includes. */
#include "ff_headers.h"
#include "ff_stdio.h"
#include "ff_ramdisk.h"

#include "untar.h"

#include "console.h"
#include "debug.h"

/* function to create the RAM disk */
extern void CreateRamDisk( void );

/* function to test the RAM disk */
void vCreateAndVerifyExampleFiles( const char *pcMountPath );

void ListFATDir( const char *pcDirectoryToScan );

/*
** Binary tar file address and size
*/
extern unsigned int _binary_tarfile_size;
extern unsigned int _binary_tarfile_start;


/*
** Startup task 
*/
void ramDiskTestTask(void* pvParameters)
{
    int   status;

    printf_("--- ramDiskTestTask starting! ---\n");

    /*
    ** Create the RAM disk
    */
    printf_("Creating RAM Disk\n");
    CreateRamDisk();

    #if 1
       /*
       ** Test the RAM disk
       */
       printf_("Testing the RAM Disk\n");
       vCreateAndVerifyExampleFiles("/ram");
    #endif

    /*
    ** Ensure in the root of the mount being used.
    */
    int32_t lResult = ff_chdir( "/ram" );
    printf_("Status of ff_chdir = %ld\n",lResult);

    printf_("Get directory for /ram - before untar\n");
    ListFATDir("/ram");

    /*
    ** Unpack tar file
    */
    printf_("Address of tarfile data = 0x%X\n",(unsigned int)&_binary_tarfile_start);
    printf_("Size of tarfile data = %lu\n",(unsigned long) &_binary_tarfile_size);
    printf_("Size of tarfile data (hex)= 0x%lX\n",(unsigned long) &_binary_tarfile_size);

    printf_("Calling Untar_FromMemory\n"); 
    status = Untar_FromMemory(
                (unsigned char *)(&_binary_tarfile_start),
                (unsigned long)&_binary_tarfile_size);

    printf_("Utar_FromMemory returned - status = %d\n",status); 

    printf_("Get directory for /ram after untar\n");
    ListFATDir("/ram");

    printf_("Get directory for /ram/rootfs/cf after untar\n");
    ListFATDir("/ram/rootfs/cf");

    printf_(" --- ramDiskTestTask jobs complete - exiting task\n");
    vTaskDelete(NULL);

    /* suppress a warning since 'params' is ignored */
    (void) pvParameters;
}
