/**
 *  Speech.cpp
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
#include "./Speech.h"
#include "./Method/CLI.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Speech::Speech() noexcept : e_Method(MRH_EvSpeechMethod::TEXT),
                            b_Update(true)
{
    // Add methods
    m_Method.insert(std::make_pair(CLI, new class CLI()));
    
    // Run
    try
    {
        c_Thread = std::thread(Update, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start update thread: " + std::string(e.what()));
    }
}

Speech::~Speech() noexcept
{
    b_Update = false;
    c_Thread.join();
    
    for (auto& Method : m_Method)
    {
        Method.second->Stop();
        delete Method.second;
    }
}

//*************************************************************************************
// Update
//*************************************************************************************

void Speech::Update(Speech* p_Instance) noexcept
{
    // Set starting method
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    OutputStorage& c_OutputStorage = p_Instance->c_OutputStorage;
    auto Active = p_Instance->m_Method.begin()->second;
    
    while (p_Instance->b_Update == true)
    {
        // First, check usable
        if (Active->IsUsable() == false)
        {
            Active->Stop();
            
            // Grab next usable
            for (auto& Method : p_Instance->m_Method)
            {
                // Not connected, no output, etc...
                if (Method.second->IsUsable() == false)
                {
                    continue;
                }
                
                try
                {
                    Method.second->Start(); // Start before setting, catch exception
                    Active = Method.second;
                }
                catch (Exception& e)
                {
                    c_Logger.Log(MRH_PSBLogger::WARNING, e.what(),
                                 "Speech.cpp", __LINE__);
                    continue;
                }
                
                // Method callback - set correct method
                switch (Method.first)
                {
                    case CLI:
                    case MRH_SRV:
                        p_Instance->e_Method = MRH_EvSpeechMethod::TEXT;
                        break;
                        
                    default:
                        p_Instance->e_Method = MRH_EvSpeechMethod::VOICE;
                        break;
                }
                
                // Setup done, end loop
                break;
            }
        }
        
        // Exchange data
        try
        {
            Active->Listen();
            Active->PerformOutput(c_OutputStorage);
        }
        catch (Exception& e)
        {
            c_Logger.Log(MRH_PSBLogger::WARNING, e.what(),
                         "Speech.cpp", __LINE__);
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

OutputStorage& Speech::GetOutputStorage() noexcept
{
    return c_OutputStorage;
}

MRH_EvSpeechMethod::Method Speech::GetMethod() noexcept
{
    return e_Method;
}
