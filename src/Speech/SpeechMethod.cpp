/**
 *  SpeechMethod.cpp
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
#include <libmrhcevs/Event/V1/Service/MRH_CEListenS_V1.h>
#include <libmrhcevs/Event/V1/Service/MRH_CESayS_V1.h>
#include <libmrhvt/String/MRH_SpeechString.h>
#include <libmrhpsb/MRH_EventStorage.h>
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./SpeechMethod.h"

// Pre-defined
#ifndef MRH_SPEECH_SERVICE_PRINT_INPUT
    #define MRH_SPEECH_SERVICE_PRINT_INPUT 0
#endif
#ifndef MRH_SPEECH_SERVICE_PRINT_OUTPUT
    #define MRH_SPEECH_SERVICE_PRINT_OUTPUT 0
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

SpeechMethod::ListenID SpeechMethod::c_ListenID;

SpeechMethod::SpeechMethod() noexcept
{}

SpeechMethod::~SpeechMethod() noexcept
{}

SpeechMethod::ListenID::ListenID() noexcept : u32_StringID(0)
{}

//*************************************************************************************
// Useage
//*************************************************************************************

void SpeechMethod::Resume()
{
    throw Exception("Default speech resume function called!");
}

void SpeechMethod::Reset()
{
    throw Exception("Default speech reset function called!");
}

void SpeechMethod::Pause()
{
    throw Exception("Default speech pause function called!");
}

//*************************************************************************************
// Listen
//*************************************************************************************

void SpeechMethod::Listen()
{
    throw Exception("Default speech listen function called!");
}

void SpeechMethod::SendInput(std::string const& s_String)
{
    MRH_EventStorage& c_Storage = MRH_EventStorage::Singleton();
    
    // Grab id to use first
    c_ListenID.c_Mutex.lock();
    
    MRH_Uint32 u32_CurrentID = c_ListenID.u32_StringID;
    c_ListenID.u32_StringID += 1; // Next ID for use
    
    c_ListenID.c_Mutex.unlock();
    
    // Split string
    try
    {
        std::map<MRH_Uint32, std::string> m_Part(MRH_SpeechString::SplitString(s_String));
        
        for (auto It = m_Part.begin(); It != m_Part.end(); ++It)
        {
            c_Storage.Add(MRH_L_STRING_S((It == --(m_Part.end())) ? MRH_S_STRING_U::END : MRH_S_STRING_U::UNFINISHED,
                                         u32_CurrentID,
                                         It->first,
                                         It->second),
                          0); // LISTEN_STRING_S has no group id!
        }
        
#if MRH_SPEECH_SERVICE_PRINT_INPUT > 0
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Recieved listen input: [ " +
                                                            s_String +
                                                            " ]",
                                       "SpeechMethod.cpp", __LINE__);
#endif
    }
    catch (...)
    {
        throw Exception("Failed to create input events!");
    }
}

//*************************************************************************************
// Say
//*************************************************************************************

void SpeechMethod::Say(OutputStorage& c_OutputStorage)
{
    throw Exception("Default speech say function called!");
}

void SpeechMethod::OutputPerformed(MRH_Uint32 u32_StringID, MRH_Uint32 u32_GroupID)
{
    try
    {
        MRH_EventStorage::Singleton().Add(MRH_S_STRING_S(u32_StringID), u32_GroupID);
        
#if MRH_SPEECH_SERVICE_PRINT_OUTPUT > 0
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Performed say output: [ " +
                                                            std::to_string(u32_StringID) +
                                                            " ]",
                                       "SpeechMethod.cpp", __LINE__);
#endif
    }
    catch (...)
    {
        throw Exception("Failed to create output performed event!");
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool SpeechMethod::IsUsable() noexcept
{
    return false;
}
