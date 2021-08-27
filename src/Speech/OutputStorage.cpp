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

//*************************************************************************************
// Reset
//*************************************************************************************

void OutputStorage::ResetUnfinished() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_UnfinishedMutex);
    
    for (auto& Unfinished : m_Unfinished)
    {
        Unfinished.second.Reset(Unfinished.first);
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

void OutputStorage::AddEvent(const MRH_S_STRING_U* p_Event) noexcept
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
            
            if (m_Unfinished.insert(std::make_pair(p_Event->GetID(), c_String)).second == false)
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
        switch (Unfinished->second.GetState())
        {
            case MRH_SpeechString::UNFINISHED:
            case MRH_SpeechString::END_KNOWN:
                Unfinished->second.Add(p_Event->GetString(),
                                       p_Event->GetPart(),
                                       p_Event->GetType() == MRH_S_STRING_U::END ? true : false);
                break;
            case MRH_SpeechString::COMPLETE:
                Unfinished->second.Reset(p_Event->GetString(),
                                         p_Event->GetID(),
                                         p_Event->GetPart(),
                                         p_Event->GetType() == MRH_S_STRING_U::END ? true : false);
                break;
                
            default:
                throw Exception("Unknown speech string state!");
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
    if (Unfinished->second.GetState() != MRH_SpeechString::COMPLETE)
    {
        c_UnfinishedMutex.unlock();
        return;
    }
    
    c_FinishedMutex.lock();
    
    l_Finished.emplace_back(Unfinished->second.GetString());
    
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

std::string OutputStorage::GetFinishedString()
{
    std::lock_guard<std::mutex> c_Guard(c_FinishedMutex);
    
    if (l_Finished.size() == 0)
    {
        throw Exception("No finished string available!");
    }
    
    std::string s_Result(l_Finished.front());
    l_Finished.pop_front();
    
    return s_Result;
}
