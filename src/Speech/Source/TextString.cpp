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
#include <cstring>

// External
#include <libmrhpsb/MRH_PSBLogger.h>
#include <libmrhls/Error/MRH_LocalStreamError.h>

// Project
#include "./TextString.h"
#include "../SpeechEvent.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

TextString::TextString(Configuration const& c_Configuration) : LocalStream(c_Configuration.GetTextStringSocketPath()),
                                                               u64_RecieveTimestampS(0),
                                                               u32_RecieveTimeoutS(c_Configuration.GetTextStringRecieveTimeoutS())
{
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Using text string communication.",
                                   "TextString.cpp", __LINE__);
}

TextString::~TextString() noexcept
{}

//*************************************************************************************
// Receive
//*************************************************************************************

MRH_Uint32 TextString::Receive(MRH_Uint32 u32_StringID) noexcept
{
    MRH_LS_M_String_Data c_Message;
    std::vector<MRH_Uint8> v_Message(MRH_STREAM_MESSAGE_BUFFER_SIZE, 0);
    MRH_Uint32 u32_NextStringID = u32_StringID;
    
    // Read all messages recieved
    while (LocalStream::Receive(v_Message) == true)
    {
        if (MRH_LS_GetBufferMessage(v_Message.data()) != MRH_LS_M_STRING)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Unknown local stream message recieved!",
                                           "TextString.cpp", __LINE__);
            continue;
        }
        else if (MRH_LS_BufferToMessage(&c_Message, v_Message.data(), v_Message.size()) < 0)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                                           "TextString.cpp", __LINE__);
            continue;
        }
        
        try
        {
            SpeechEvent::InputRecieved(u32_NextStringID, c_Message.p_String); // @NOTE: Always terminated!
            ++u32_NextStringID;
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                           "TextString.cpp", __LINE__);
        }
    }
    
    // If the string is > 0 then we recieved new info
    if (u32_StringID != u32_NextStringID)
    {
        u64_RecieveTimestampS = time(NULL);
    }
    
    // Return new string id
    return u32_NextStringID;
}

//*************************************************************************************
// Send
//*************************************************************************************

void TextString::Send(OutputStorage& c_OutputStorage) noexcept
{
    if (c_OutputStorage.GetAvailable() == false)
    {
        return;
    }
    else if (LocalStream::IsConnected() == false)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Text string stream is not connected!",
                                       "TextString.cpp", __LINE__);
        return;
    }
    
    while (c_OutputStorage.GetAvailable() == true)
    {
        try
        {
            // Build string first
            auto String = c_OutputStorage.GetString();
            
            MRH_LS_M_String_Data c_Message;
            std::vector<MRH_Uint8> v_Message(MRH_STREAM_MESSAGE_BUFFER_SIZE, 0);
            MRH_Uint32 u32_Size;
            
            strncpy(c_Message.p_String, String.s_String.c_str(), MRH_STREAM_MESSAGE_BUFFER_SIZE);
            
            if (MRH_LS_MessageToBuffer(&(v_Message[0]), &u32_Size, MRH_LS_M_STRING, &c_Message) < 0)
            {
                throw Exception(MRH_ERR_GetLocalStreamErrorString());
            }
            
            v_Message.resize(u32_Size);
            
            // Send and set performed
            LocalStream::Send(v_Message);
            SpeechEvent::OutputPerformed(String.u32_StringID,
                                         String.u32_GroupID);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                           "TextString.cpp", __LINE__);
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool TextString::GetCommunicationActive() const noexcept
{
    return GetCommunicationActive(u64_RecieveTimestampS, u32_RecieveTimeoutS);
}

bool TextString::GetCommunicationActive(MRH_Uint64 u64_TimestampS, MRH_Uint32 u32_TimeoutS) noexcept
{
    if (u64_TimestampS < (time(NULL) - u32_TimeoutS))
    {
        return false;
    }
    
    return true;
}
