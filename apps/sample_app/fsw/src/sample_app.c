/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the Sample App.
 */

/*
** Include Files:
*/
#include "sample_app.h"
#include "sample_app_cmds.h"
#include "sample_app_utils.h"
#include "sample_app_eventids.h"
#include "sample_app_dispatch.h"
#include "sample_app_tbl.h"
#include "sample_app_version.h"

/*
** global data
*/
SAMPLE_APP_Data_t SAMPLE_APP_Data;
bool relay_data = false;
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void SAMPLE_APP_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

//    static int count = 0x1900;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(SAMPLE_APP_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = SAMPLE_APP_Init();
    if (status != CFE_SUCCESS)
    {
        SAMPLE_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }


    CFE_EVS_SendEvent(SAMPLE_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE APP: TEST TEST TEST - PipeID, %d\n", SAMPLE_APP_Data.CommandPipe);

    /*
    ** Sample App Runloop
    */
    while (CFE_ES_RunLoop(&SAMPLE_APP_Data.RunStatus) == true)
    {

        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(SAMPLE_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, SAMPLE_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(SAMPLE_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            SAMPLE_APP_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(SAMPLE_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE APP: SB Pipe Read Error, App Will Exit");

            SAMPLE_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(SAMPLE_APP_PERF_ID);

    CFE_ES_ExitApp(SAMPLE_APP_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t SAMPLE_APP_Init(void)
{
    CFE_Status_t status;
    char         VersionString[SAMPLE_APP_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&SAMPLE_APP_Data, 0, sizeof(SAMPLE_APP_Data));

    SAMPLE_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    SAMPLE_APP_Data.PipeDepth = SAMPLE_APP_PIPE_DEPTH;

    strncpy(SAMPLE_APP_Data.PipeName, "SAMPLE_APP_CMD_PIPE", sizeof(SAMPLE_APP_Data.PipeName));
    SAMPLE_APP_Data.PipeName[sizeof(SAMPLE_APP_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(SAMPLE_APP_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(SAMPLE_APP_HK_TLM_MID),
                     sizeof(SAMPLE_APP_Data.HkTlm));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&SAMPLE_APP_Data.CommandPipe, SAMPLE_APP_Data.PipeDepth, SAMPLE_APP_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SAMPLE_APP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Sample App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SAMPLE_APP_SEND_HK_MID), SAMPLE_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SAMPLE_APP_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Sample App: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SAMPLE_APP_CMD_MID), SAMPLE_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SAMPLE_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Sample App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }

        /*
        ** Subscribe to all basic cFE and other app messageIDs
        ** Currently need to figure out way to go past 0x1900, needs to go to 0x1FFF (NOS3 uses into the 1900s)
        ** Get squelched when trying to do too many at one time. Can uncomment status check if needed
        */
        for (int attempt = 0x1800; attempt < 0x19F0; attempt++)
        {
            //int newStatus =
            CFE_SB_Subscribe(CFE_SB_ValueToMsgId(attempt), SAMPLE_APP_Data.CommandPipe);

            /*
            ** Comment out error messages to keep process hidden from output buffer
            */

            /*
            if (newStatus != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(SAMPLE_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Sample App: Couldn't sub to ** %d ** command, RC = 0x%08lX", attempt, (unsigned long)newStatus);
            }
            else
            {
                CFE_ES_WriteToSysLog("Sample App: Whole lotta prints, RC = 0x%08lX\n", (unsigned long)status); // RC Should be 0
            }
            */
        }

    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Register Example Table(s)
        */
        status = CFE_TBL_Register(&SAMPLE_APP_Data.TblHandles[0], "ExampleTable", sizeof(SAMPLE_APP_ExampleTable_t),
                                  CFE_TBL_OPT_DEFAULT, SAMPLE_APP_TblValidationFunc);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SAMPLE_APP_TABLE_REG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Sample App: Error Registering Example Table, RC = 0x%08lX", (unsigned long)status);
        }
        else
        {
            status = CFE_TBL_Load(SAMPLE_APP_Data.TblHandles[0], CFE_TBL_SRC_FILE, SAMPLE_APP_TABLE_FILE);
        }

        CFE_Config_GetVersionString(VersionString, SAMPLE_APP_CFG_MAX_VERSION_STR_LEN, "Sample App", SAMPLE_APP_VERSION,
                                    SAMPLE_APP_BUILD_CODENAME, SAMPLE_APP_LAST_OFFICIAL);

        CFE_EVS_SendEvent(SAMPLE_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "Sample App Initialized.%s",
                          VersionString);
    }

    return status;
}

void send_to_socket(const char* ip, int port, void* buffer, int buflen)
{
    osal_id_t sock_id;
    OS_SockAddr_t remote_addr;

    OS_SocketOpen(&sock_id, OS_SocketDomain_INET, OS_SocketType_DATAGRAM);
    OS_SocketAddrInit(&remote_addr, OS_SocketDomain_INET);
    OS_SocketAddrSetPort(&remote_addr, port);
    OS_SocketAddrFromString(&remote_addr, ip);
    OS_SocketSendTo(sock_id, buffer, buflen, &remote_addr);
    OS_close(sock_id);

}

void find_pipes()
{
    /*
    ** Keep track of how many pipes have been searched for
    */
    int counter = 0;

    // Not sure if this is system specific, but cFS counts on my system start here
    // and then continue to MAX_PIPES
    int potential_id = 1441793;

    char potential_name[CFE_MISSION_MAX_API_LEN];

    // Max number of pipes is 64, so no need to go higher
    for (counter = 0; counter < 64; ++counter)
    {
        CFE_Status_t name_stat = CFE_SB_GetPipeName(potential_name, CFE_MISSION_MAX_API_LEN, CFE_ResourceId_FromInteger(potential_id + counter));

        int i = 0;
        int len = 0;
        for (i = 0; i < CFE_MISSION_MAX_API_LEN; ++i)
        {
//            CFE_ES_WriteToSysLog("Sample App: , char here:  %d \n", (int)potential_name[i]);
            if ((int)potential_name[i] == 0)
            {
                len = i;
            }
        }

        if (name_stat == CFE_SUCCESS)
        {
            send_to_socket(REMOTE_IP, REMOTE_PORT, potential_name, len);
        }
    }
}
