/**
 *  SpeechEvent.cpp
 *
 *  This file is part of the MRH project.
 *  See the AUTHORS file for Copyright information.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// C / C++

// External
#include <libmrhevdata.h>
#include <libmrhvt/String/MRH_SpeechString.h>
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
    MRH_EventStorage& c_Storage = MRH_EventStorage::Singleton();
    
    MRH_Event* p_Event = NULL;
    MRH_EvD_S_String_U c_Data;
    
    // Split string
    try
    {
        std::map<MRH_Uint32, std::string> m_Part(MRH_SpeechString::SplitString(s_String));
        
        memset((c_Data.p_String), '\0', MRH_EVD_S_STRING_BUFFER_MAX_TERMINATED);
        
        for (auto It = m_Part.begin(); It != m_Part.end(); ++It)
        {
            if (It == --(m_Part.end()))
            {
                memset((c_Data.p_String), '\0', MRH_EVD_S_STRING_BUFFER_MAX_TERMINATED);
                c_Data.u8_Type = MRH_EVD_L_STRING_END;
            }
            else
            {
                c_Data.u8_Type = MRH_EVD_L_STRING_UNFINISHED;
            }
            
            strcpy((c_Data.p_String), (It->second.c_str()));
            
            c_Data.u32_ID = u32_StringID;
            c_Data.u32_Part = It->first;
            
            if (p_Event == NULL && (p_Event = MRH_EVD_CreateEvent(MRH_EVENT_LISTEN_STRING_S, NULL, 0)) == NULL)
            {
                continue;
            }
            else if (MRH_EVD_SetEvent(p_Event, MRH_EVENT_LISTEN_STRING_S, &c_Data) < 0)
            {
                continue;
            }
            
            c_Storage.Add(p_Event);
            p_Event = NULL;
        }
        
#if MRH_SPEECH_SERVICE_PRINT_INPUT > 0
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Recieved listen input: [ " +
                                                            s_String +
                                                            " ]",
                                       "SpeechMethod.cpp", __LINE__);
#endif
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to create input events!");
    }
    
    if (p_Event != NULL)
    {
        MRH_EVD_DestroyEvent(p_Event);
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
                                       "SpeechMethod.cpp", __LINE__);
#endif
    }
    catch (...)
    {
        throw Exception("Failed to add output performed event!");
    }
}
