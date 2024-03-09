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

#include "debug.h"
#include "print.h"
#include "receive.h"

/*
 * Priorities of certain tasks.
 * Note: priorities should not be greater than configMAX_PRIORITIES - 1,
 * defined in FreeRTOSConfig.h (its default value equals 5).
 * If any priority is greater than this value, xTasCreate will
 * silently reduce it.
 */
#define PRIOR_PERIODIC                   ( 2 )
#define PRIOR_FIX_FREQ_PERIODIC          ( 3 )

/* 
** Struct with settings for each task 
*/
typedef struct _paramStruct
{
    portCHAR    *text;           /* text to be printed by the task */
    UBaseType_t  delay;          /* delay in milliseconds */
} paramStruct;

/* Parameters for two tasks */
static const paramStruct tParam[2] =
{
    (paramStruct) { 
                    .text="Reg Task\r\n", 
                    .delay=1000 
                  },
    (paramStruct) { 
                    .text="Periodic Task\r\n", 
                    .delay=3000 
                  }
};

/* 
** Default parameters if no parameter struct is available 
*/
static const portCHAR defaultText[] = "<NO TEXT>\r\n";
static const UBaseType_t defaultDelay = 1000;


/* 
** Task function - may be instantiated in multiple tasks 
*/
void vTaskFunction( void *pvParameters )
{
    const portCHAR* taskName;
    UBaseType_t  delay;
    paramStruct* params = (paramStruct*) pvParameters;

    taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
    delay = ( NULL==params ? defaultDelay : params->delay);

    for( ; ; )
    {
        /* Print out the name of this task. */

        vDirectPrintMsg(taskName);

        vTaskDelay( delay / portTICK_RATE_MS );
    }

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}

/* 
** Fixed frequency periodic task function - may be instantiated in multiple tasks 
*/
void vPeriodicTaskFunction(void* pvParameters)
{
    const portCHAR* taskName;
    UBaseType_t delay;
    paramStruct* params = (paramStruct*) pvParameters;
    TickType_t lastWakeTime;

    taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
    delay = ( NULL==params ? defaultDelay : params->delay);

    /*
     * This variable must be initialized once.
     * Then it will be updated automatically by vTaskDelayUntil().
     */
    lastWakeTime = xTaskGetTickCount();

    for( ; ; )
    {
        /* 
        ** Print out the name of this task. 
        */
        vDirectPrintMsg(taskName);

        /*
        ** The task will unblock exactly after 'delay' milliseconds (actually
        ** after the appropriate number of ticks), relative from the moment
        ** it was last unblocked.
        */
        vTaskDelayUntil( &lastWakeTime, delay / portTICK_RATE_MS );
    }

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}

/*
** two demo tasks: two periodic print-a-message tasks
*/
void demoTasks()
{

    /* And finally create two tasks: */
    if ( pdPASS != xTaskCreate(vTaskFunction, "task1", 128, (void*) &tParam[0],
                               PRIOR_PERIODIC, NULL) )
    {
        FreeRTOS_Error("Could not create task1\r\n");
    }

    if ( pdPASS != xTaskCreate(vPeriodicTaskFunction, "task2", 128, (void*) &tParam[1],
                               PRIOR_FIX_FREQ_PERIODIC, NULL) )
    {
        FreeRTOS_Error("Could not create task2\r\n");
    }
}
