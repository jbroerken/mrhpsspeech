/**
 *  OutputStorage.cpp
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
#include <cstring>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./OutputStorage.h"

// Pre-defined
#ifndef MRH_SPEECH_SERVICE_PRINT_OUTPUT
    #define MRH_SPEECH_SERVICE_PRINT_OUTPUT 0
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

OutputStorage::OutputStorage() noexcept
{}

OutputStorage::~OutputStorage() noexcept
{}

OutputStorage::String::String(std::string const& s_String,
                              MRH_Uint32 u32_StringID,
                              MRH_Uint32 u32_GroupID) noexcept : s_String(s_String),
                                                                 u32_StringID(u32_StringID),
                                                                 u32_GroupID(u32_GroupID)
{}

//*************************************************************************************
// Clear
//*************************************************************************************

void OutputStorage::Clear() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    dq_Output.clear();
}

//*************************************************************************************
// Add
//*************************************************************************************

void OutputStorage::AddString(MRH_EvD_S_String_U const& c_String, MRH_Uint32 u32_GroupID) noexcept
{
    size_t us_Length = strnlen(c_String.p_String, MRH_EVD_S_STRING_BUFFER_MAX);
    
    if (us_Length == 0 || us_Length > MRH_EVD_S_STRING_BUFFER_MAX)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Tried to add string with size " +
                                                             std::to_string(us_Length) +
                                                             "!",
                                       "OutputStorage.cpp", __LINE__);
        return;
    }
    
    try
    {
        std::lock_guard<std::mutex> c_Guard(c_Mutex);
        
        dq_Output.emplace_back(c_String.p_String,
                               c_String.u32_ID,
                               u32_GroupID);
        
#if MRH_SPEECH_SERVICE_PRINT_OUTPUT > 0
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Recieved say output: [ " +
                                                            std::string(c_String.p_String) +
                                                            " (ID: " +
                                                            std::to_string(c_String.u32_ID) +
                                                            ")]",
                                       "OutputStorage.cpp", __LINE__);
#endif
    }
    catch (std::exception& e) // Catch all
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                       "OutputStorage.cpp", __LINE__);
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool OutputStorage::GetAvailable() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return dq_Output.size() > 0 ? true : false;
}

OutputStorage::String OutputStorage::GetString()
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    
    if (dq_Output.size() == 0)
    {
        throw Exception("No output string available!");
    }
    
    OutputStorage::String c_Result(dq_Output.front());
    dq_Output.pop_front();
    
    return c_Result;
}
