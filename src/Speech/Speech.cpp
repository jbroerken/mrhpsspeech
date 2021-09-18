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
#include "./Method/Voice.h"

// Pre-defined
#ifndef MRH_SPEECH_CLI_ENABLED
    #define MRH_SPEECH_CLI_ENABLED 0
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Speech::Speech() : e_Method(MRH_EvSpeechMethod::VOICE),
                   b_Update(true)
{
    // Add methods
    for (size_t i = 0; i < METHOD_COUNT; ++i)
    {
        try
        {
            switch (i)
            {
                case CLI:
#if MRH_SPEECH_CLI_ENABLED > 0
                    m_Method.insert(std::make_pair(CLI, new class CLI()));
#endif
                    break;
                case MRH_SRV:
                    break;
                case VOICE:
                    //m_Method.insert(std::make_pair(VOICE, new Voice()));
                    break;
            }
        }
        catch (std::exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Failed to add method: " +
                                                                   std::string(e.what()),
                                           "Speech.cpp", __LINE__);
        }
    }
    
    // Can we do anything?
    if (m_Method.size() == 0)
    {
        throw Exception("No usable speech methods!");
    }
    
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
    SpeechMethod* p_Method = NULL;
    
    while (p_Instance->b_Update == true)
    {
        // Grab the method to use
        // NOTE: We check the method each time for all even if the current one is
        //       valid to catch cli connections, etc.
        for (auto& Method : p_Instance->m_Method)
        {
            // Is this method usable and different?
            if (Method.second == p_Method || Method.second->IsUsable() == false)
            {
                continue;
            }
            
            // Stop old and start new
            if (p_Method != NULL)
            {
                p_Method->Pause();
            }
            
            Method.second->Resume();
            
            // Set the new method
            c_Logger.Log(MRH_PSBLogger::INFO, "Set speech method in use to " +
                                              std::to_string(Method.first),
                         "Speech.cpp", __LINE__);
            p_Method = Method.second;
            
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
            
            // Method set, end
            break;
        }
        
        // No method?
        if (p_Method == NULL)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        
        // Exchange data
        try
        {
            p_Method->Listen();
            p_Method->Say(c_OutputStorage);
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
