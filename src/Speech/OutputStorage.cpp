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

void OutputStorage::AddEvent(const MRH_S_STRING_U* p_Event, MRH_Uint32 u32_GroupID) noexcept
{
    c_UnfinishedMutex.lock();
    
    // Already found?
    auto Unfinished = m_Unfinished.find(p_Event->GetID());
    
    if (Unfinished == m_Unfinished.end())
    {
        // Not found, add new
        try
        {
            MRH_SpeechString c_String(p_Event->GetString(),
                                      p_Event->GetPart(),
                                      p_Event->GetID(),
                                      p_Event->GetType() == MRH_S_STRING_U::END ? true : false);
            
            if (m_Unfinished.insert(std::make_pair(p_Event->GetID(), std::make_pair(c_String, u32_GroupID))).second == false)
            {
                throw Exception("Failed to add output to map!");
            }
            
            c_UnfinishedMutex.unlock();
            return;
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
                    Unfinished->second.first.Add(p_Event->GetString(),
                                                 p_Event->GetPart(),
                                                 p_Event->GetType() == MRH_S_STRING_U::END ? true : false);
                    break;
                case MRH_SpeechString::COMPLETE:
                    Unfinished->second.first.Reset(p_Event->GetString(),
                                                   p_Event->GetID(),
                                                   p_Event->GetPart(),
                                                   p_Event->GetType() == MRH_S_STRING_U::END ? true : false);
                    break;
                    
                default:
                    throw Exception("Unknown speech string state!");
            }
        }
        else
        {
            Unfinished->second.second = u32_GroupID;
            Unfinished->second.first.Reset(p_Event->GetString(),
                                           p_Event->GetID(),
                                           p_Event->GetPart(),
                                           p_Event->GetType() == MRH_S_STRING_U::END ? true : false);
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
