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
// Reset
//*************************************************************************************

void OutputStorage::ResetUnfinished() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_UnfinishedMutex);
    
    for (auto& Unfinished : m_Unfinished)
    {
        Unfinished.second.first.Reset(Unfinished.first);
    }
}

void OutputStorage::ResetFinished() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_FinishedMutex);
    l_Finished.clear();
}

//*************************************************************************************
// Add
//*************************************************************************************

void OutputStorage::AddString(MRH_EvD_S_String_U const& c_String, MRH_Uint32 u32_GroupID) noexcept
{
    c_UnfinishedMutex.lock();
    
    // Already found?
    auto Unfinished = m_Unfinished.find(c_String.u32_ID);
    
    if (Unfinished == m_Unfinished.end())
    {
        // Not found, add new
        try
        {
            MRH_SpeechString c_New(c_String.p_String,
                                   c_String.u32_Part,
                                   c_String.u32_ID,
                                   c_String.u8_Type == MRH_EVD_S_STRING_END ? true : false);
            
            if (m_Unfinished.insert(std::make_pair(c_String.u32_ID, std::make_pair(c_New, u32_GroupID))).second == false)
            {
                throw Exception("Failed to add output to map!");
            }
            
            Unfinished = m_Unfinished.find(c_String.u32_ID);
        }
        catch (std::exception& e) // Catch all
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, e.what(),
                                           "OutputStorage.cpp", __LINE__);
            c_UnfinishedMutex.unlock();
            return;
        }
    }
    
    // Found, replace or add
    try
    {
        // Group id needs to be the same, otherwise reset (other sender)
        if (Unfinished->second.second == u32_GroupID)
        {
            switch (Unfinished->second.first.GetState())
            {
                case MRH_SpeechString::UNFINISHED:
                case MRH_SpeechString::END_KNOWN:
                    Unfinished->second.first.Add(c_String.p_String,
                                                 c_String.u32_Part,
                                                 c_String.u8_Type == MRH_EVD_S_STRING_END ? true : false);
                    break;
                case MRH_SpeechString::COMPLETE:
                    Unfinished->second.first.Reset(c_String.p_String,
                                                   c_String.u32_ID,
                                                   c_String.u32_Part,
                                                   c_String.u8_Type == MRH_EVD_S_STRING_END ? true : false);
                    break;
                    
                default:
                    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Unknown speech string state!",
                                                   "OutputStorage.cpp", __LINE__);
                    break;
            }
        }
        else
        {
            Unfinished->second.second = u32_GroupID;
            Unfinished->second.first.Reset(c_String.p_String,
                                           c_String.u32_ID,
                                           c_String.u32_Part,
                                           c_String.u8_Type == MRH_EVD_S_STRING_END ? true : false);
        }
    }
    catch (std::exception& e) // Catch all
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, e.what(),
                                       "OutputStorage.cpp", __LINE__);
        c_UnfinishedMutex.unlock();
        return;
    }
    
    // String updated, can we add this one to the finished list?
    if (Unfinished->second.first.GetState() != MRH_SpeechString::COMPLETE)
    {
        c_UnfinishedMutex.unlock();
        return;
    }
    
    c_FinishedMutex.lock();
    
    l_Finished.emplace_back(Unfinished->second.first.GetString(),
                            Unfinished->first,
                            Unfinished->second.second);
    
#if MRH_SPEECH_SERVICE_PRINT_OUTPUT > 0
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Recieved say output: [ " +
                                                        l_Finished.back().s_String +
                                                        " (ID: " +
                                                        std::to_string(l_Finished.back().u32_StringID) +
                                                        ")]",
                                   "SpeechMethod.cpp", __LINE__);
    
#endif
    
    c_FinishedMutex.unlock();
    c_UnfinishedMutex.unlock();
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool OutputStorage::GetFinishedAvailable() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_FinishedMutex);
    return l_Finished.size() > 0 ? true : false;
}

OutputStorage::String OutputStorage::GetFinishedString()
{
    std::lock_guard<std::mutex> c_Guard(c_FinishedMutex);
    
    if (l_Finished.size() == 0)
    {
        throw Exception("No finished string available!");
    }
    
    OutputStorage::String c_Result(l_Finished.front());
    l_Finished.pop_front();
    
    return c_Result;
}
