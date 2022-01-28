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


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Speech::Speech(Configuration const& c_Configuration) : b_Update(true),
#if MRH_SPEECH_USE_LOCAL_STREAM > 0
                                                       c_LocalStream(c_Configuration),
#endif
                                                       c_NetServer(c_Configuration),
                                                       e_Method(LOCAL),
                                                       b_MethodSelected(false)
{
#if MRH_SPEECH_USE_NET_SERVER <= 0 && MRH_SPEECH_USE_LOCAL_STREAM <= 0
    throw Exception("No usable speech methods!");
#endif
    
    try
    {
        c_Thread = std::thread(Update, this, c_Configuration.GetServiceMethodWaitMS());
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
}

//*************************************************************************************
// Update
//*************************************************************************************

void Speech::Update(Speech* p_Instance, MRH_Uint32 u32_MethodWaitMS) noexcept
{
    // Set used objects
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    OutputStorage& c_OutputStorage = p_Instance->c_OutputStorage;
    
    // Select sources
#if MRH_SPEECH_USE_LOCAL_STREAM > 0
    LocalStream& c_Stream = p_Instance->c_LocalStream;
#endif
#if MRH_SPEECH_USE_NET_SERVER > 0
    NetServer& c_NetServer = p_Instance->c_NetServer;
#endif
    
    // Shared default string id
    MRH_Uint32 u32_StringID = 0;
    
    while (p_Instance->b_Update == true)
    {
        // Wait a bit for data
        // @NOTE: We ALWAYS wait - we want servers and audio devices to have sent
        //        some data when calling Listen()
        std::this_thread::sleep_for(std::chrono::milliseconds(u32_MethodWaitMS));
        
        /**
         *  Net Server
         */
        
#if MRH_SPEECH_USE_NET_SERVER > 0
        // Connected?
        if (c_NetServer.GetAppClientConnected() == true)
        {
            if (p_Instance->e_Method != REMOTE)
            {
                p_Instance->e_Method = REMOTE;
            }
        }
        
        try
        {
            // Retrieve messages
            u32_StringID = c_NetServer.Retrieve(u32_StringID);
        }
        catch (Exception& e)
        {
            c_Logger.Log(MRH_PSBLogger::ERROR, e.what(),
                         "Speech.cpp", __LINE__);
        }
        
        // Stop voice exchange if connected
        if (p_Instance->e_Method == REMOTE)
        {
            continue;
        }
#endif
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

OutputStorage& Speech::GetOutputStorage() noexcept
{
    return c_OutputStorage;
}

Speech::Method Speech::GetMethod() noexcept
{
    return e_Method;
}

bool Speech::GetMethodSelected() noexcept
{
    return b_MethodSelected;
}
