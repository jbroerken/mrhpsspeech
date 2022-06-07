/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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
#if MRH_SPEECH_USE_VOICE > 0
                                                       c_Voice(c_Configuration),
#endif
#if MRH_SPEECH_USE_TEXT_STRING > 0
                                                       c_TextString(c_Configuration),
#endif
                                                       e_Method(AUDIO)
{
#if MRH_SPEECH_USE_TEXT_STRING <= 0 && MRH_SPEECH_USE_VOICE <= 0
    throw Exception("No usable speech methods!");
#endif
    
    try
    {
        c_Thread = std::thread(Update, 
                               this, 
                               c_Configuration.GetServiceMethodWaitMS());
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
#if MRH_SPEECH_USE_VOICE > 0
    Voice& c_Voice = p_Instance->c_Voice;
#endif
#if MRH_SPEECH_USE_TEXT_STRING > 0
    TextString& c_TextString = p_Instance->c_TextString;
#endif
    
    // Shared default string id
    MRH_Uint32 u32_StringID = 0;
    
    while (p_Instance->b_Update == true)
    {
        // Wait a bit for data
        // @NOTE: We ALWAYS wait - we want servers, string clients and audio devices to have sent
        //        some data before recieving
        std::this_thread::sleep_for(std::chrono::milliseconds(u32_MethodWaitMS));
        
        /**
         *  Text String
         */
        
#if MRH_SPEECH_USE_TEXT_STRING > 0
        // Recieve messages first
        u32_StringID = c_TextString.Receive(u32_StringID);
        
        // Connected?
        if (c_TextString.GetCommunicationActive() == true)
        {
            // Switch to text string method
            if (p_Instance->e_Method != TEXT_STRING)
            {
#if MRH_SPEECH_USE_VOICE > 0
                c_Voice.StopRecording();
#endif
                p_Instance->e_Method = TEXT_STRING;
            }
            
            // Send messages to text string client
            c_TextString.Send(c_OutputStorage);
        }
        else
        {
            // No connection, switch to audio again
            if (p_Instance->e_Method == TEXT_STRING)
            {
#if MRH_SPEECH_USE_VOICE > 0
                c_Voice.StartRecording();
#endif
                p_Instance->e_Method = AUDIO;
            }
        }
#endif
        
        /**
         *  Voice
         */
        
#if MRH_SPEECH_USE_VOICE > 0
        try
        {
            // Text string in use, retrieve and discard input
            if (p_Instance->e_Method == TEXT_STRING)
            {
                // Discard input but recieve finished output before returning
                c_Voice.Retrieve(0, true);
                continue;
            }
            
            // Voice in use, get input and performed output
            // before sending output
            u32_StringID = c_Voice.Retrieve(u32_StringID, false);
            c_Voice.Send(c_OutputStorage);
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
    
#if MRH_SPEECH_USE_VOICE > 0
    b_Usable = c_Voice.GetSourceConnected();
#endif
#if MRH_SPEECH_USE_TEXT_STRING > 0
    if (b_Usable == false)
    {
        b_Usable = c_TextString.GetCommunicationActive();
    }
#endif
    
    return b_Usable;
}
