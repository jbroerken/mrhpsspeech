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
#include <cstring>

// External
#include <libmrhevdata.h>
#include <libmrhpsb/MRH_EventStorage.h>
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./SpeechEvent.h"

// Pre-defined
#ifndef MRH_SPEECH_SERVICE_PRINT_INPUT
    #define MRH_SPEECH_SERVICE_PRINT_INPUT 0
#endif
#ifndef MRH_SPEECH_SERVICE_PRINT_OUTPUT
    #define MRH_SPEECH_SERVICE_PRINT_OUTPUT 0
#endif


//*************************************************************************************
// Listen
//*************************************************************************************

void SpeechEvent::InputRecieved(MRH_Uint32 u32_StringID, std::string const& s_String)
{
    // Create string data first
    MRH_EvD_L_String_S c_Data;
    
    memset((c_Data.p_String), '\0', MRH_EVD_S_STRING_BUFFER_MAX_TERMINATED);
    strncpy(c_Data.p_String, s_String.c_str(), MRH_EVD_S_STRING_BUFFER_MAX);
    c_Data.u32_ID = u32_StringID;
    
    // Now build event
    MRH_Event* p_Event = MRH_EVD_CreateSetEvent(MRH_EVENT_LISTEN_STRING_S, &c_Data);
    
    if (p_Event == NULL)
    {
        throw Exception("Failed to create listen string event!");
    }
    
    // Created, now add to event storage
    try
    {
        MRH_EventStorage::Singleton().Add(p_Event);
        
#if MRH_SPEECH_SERVICE_PRINT_INPUT > 0
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Recieved listen input: [ " +
                                                            s_String +
                                                            " (ID: " +
                                                            std::to_string(u32_StringID) +
                                                            ")]",
                                       "SpeechEvent.cpp", __LINE__);
#endif
    }
    catch (MRH_PSBException& e)
    {
        MRH_EVD_DestroyEvent(p_Event);
        throw Exception("Failed to send input: " + e.what2());
    }
}

//*************************************************************************************
// Say
//*************************************************************************************

void SpeechEvent::OutputPerformed(MRH_Uint32 u32_StringID, MRH_Uint32 u32_GroupID)
{
    MRH_EvD_S_String_S c_Data;
    c_Data.u32_ID = u32_StringID;
    
    MRH_Event* p_Event = MRH_EVD_CreateSetEvent(MRH_EVENT_SAY_STRING_S, &c_Data);
    
    if (p_Event == NULL)
    {
        throw Exception("Failed to create output performed event!");
    }
    
    p_Event->u32_GroupID = u32_GroupID;
    
    try
    {
        MRH_EventStorage::Singleton().Add(p_Event);
        
#if MRH_SPEECH_SERVICE_PRINT_OUTPUT > 0
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Performed say output: [ " +
                                                            std::to_string(u32_StringID) +
                                                            " ]",
                                       "SpeechEvent.cpp", __LINE__);
#endif
    }
    catch (...)
    {
        throw Exception("Failed to add output performed event!");
    }
}
