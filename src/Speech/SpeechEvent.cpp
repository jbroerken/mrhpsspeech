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
