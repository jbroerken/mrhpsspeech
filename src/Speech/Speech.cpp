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
#if MRH_SPEECH_USE_NET_SERVER > 0
                                                       c_NetServer(c_Configuration),
#endif
                                                       e_Method(LOCAL)
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
    LocalStream& c_LocalStream = p_Instance->c_LocalStream;
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
        //        some data before recieving
        std::this_thread::sleep_for(std::chrono::milliseconds(u32_MethodWaitMS));
        
        /**
         *  Net Server
         */
        
#if MRH_SPEECH_USE_NET_SERVER > 0
        // Connected?
        if (c_NetServer.GetAppClientConnected() == true)
        {
            // Switch to remote method
            if (p_Instance->e_Method != REMOTE)
            {
#if MRH_SPEECH_USE_LOCAL_STREAM > 0 && MRH_API_PROVIDER_CLI <= 0
                c_LocalStream.StopRecording();
#endif
                p_Instance->e_Method = REMOTE;
            }
            
            // Exchange server messages
            try
            {
                u32_StringID = c_NetServer.Retrieve(u32_StringID);
                c_NetServer.Send(c_OutputStorage);
            }
            catch (Exception& e)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, e.what(),
                             "Speech.cpp", __LINE__);
            }
        }
        else
        {
            // No connection, use local
            if (p_Instance->e_Method == REMOTE)
            {
#if MRH_SPEECH_USE_LOCAL_STREAM > 0 && MRH_API_PROVIDER_CLI <= 0
                c_LocalStream.StartRecording();
#endif
                p_Instance->e_Method = LOCAL;
            }
        }
#endif
        
        /**
         *  Local Stream
         */
        
#if MRH_SPEECH_USE_LOCAL_STREAM > 0
        try
        {
            // Recieve local stream input based on
            // remote connection
            if (p_Instance->e_Method == REMOTE)
            {
                // Server in use, discard input but recieve finished output
                // before returning
                c_LocalStream.Retrieve(0, true);
                continue;
            }
            
            // Local stream in use, get input and performed output
            // before sending output
            u32_StringID = c_LocalStream.Retrieve(u32_StringID, false);
            c_LocalStream.Send(c_OutputStorage);
        }
        catch (Exception& e)
        {
            c_Logger.Log(MRH_PSBLogger::ERROR, e.what(),
                         "Speech.cpp", __LINE__);
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

bool Speech::GetUsable() noexcept
{
    bool b_Usable = false;
    
#if MRH_SPEECH_USE_LOCAL_STREAM > 0
    b_Usable = c_LocalStream.GetStreamConnected();
#endif
#if MRH_SPEECH_USE_NET_SERVER > 0
    if (b_Usable == false)
    {
        b_Usable = c_NetServer.GetAppClientConnected();
    }
#endif
    
    return b_Usable;
}
