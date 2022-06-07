/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// C / C++

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./CBSpeechMethod.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

CBSpeechMethod::CBSpeechMethod(std::shared_ptr<Speech>& p_Speech) noexcept : p_Speech(p_Speech)
{}

CBSpeechMethod::~CBSpeechMethod() noexcept
{}

//*************************************************************************************
// Callback
//*************************************************************************************

void CBSpeechMethod::Callback(const MRH_Event* p_Event, MRH_Uint32 u32_GroupID) noexcept
{
    MRH_Event* p_Result = NULL;
    
    if (p_Event->u32_Type == MRH_EVENT_LISTEN_GET_METHOD_U)
    {
        MRH_EvD_L_GetMethod_S c_Data;
        c_Data.u8_Result = 1;
        
        switch (p_Speech->GetMethod())
        {
            case Speech::TEXT_STRING:
                c_Data.u8_Method = MRH_EVD_L_SPEECH_TEXT_STRING;
                break;
                
            default:
                c_Data.u8_Method = MRH_EVD_L_SPEECH_AUDIO;
                break;
        }
        
        p_Result = MRH_EVD_CreateSetEvent(MRH_EVENT_LISTEN_GET_METHOD_S, &c_Data);
    }
    else if (p_Event->u32_Type == MRH_EVENT_SAY_GET_METHOD_U)
    {
        MRH_EvD_S_GetMethod_S c_Data;
        c_Data.u8_Result = 1;
        
        switch (p_Speech->GetMethod())
        {
            case Speech::TEXT_STRING:
                c_Data.u8_Method = MRH_EVD_S_SPEECH_TEXT_STRING;
                break;
                
            default:
                c_Data.u8_Method = MRH_EVD_S_SPEECH_AUDIO;
                break;
        }
        
        p_Result = MRH_EVD_CreateSetEvent(MRH_EVENT_SAY_GET_METHOD_S, &c_Data);
    }
    else
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to create response event!",
                                       "CBSpeechMethod.cpp", __LINE__);
        return;
    }
    
    if (p_Result == NULL)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to create response event!",
                                       "CBSpeechMethod.cpp", __LINE__);
        return;
    }
    
    p_Result->u32_GroupID = u32_GroupID;
    
    try
    {
        MRH_EventStorage::Singleton().Add(p_Result);
    }
    catch (MRH_PSBException& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                       "CBSpeechMethod.cpp", __LINE__);
        MRH_EVD_DestroyEvent(p_Result);
    }
}
