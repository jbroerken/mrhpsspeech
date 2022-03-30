/**
 *  NetServer.cpp
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
#include <libmrhls/Error/MRH_LocalStreamError.h>

// Project
#include "./NetServer.h"
#include "../SpeechEvent.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

NetServer::NetServer(Configuration const& c_Configuration) : LocalStream(c_Configuration.GetServerSocketPath()),
                                                             u64_RecieveTimestampS(0),
                                                             u32_RecieveTimeoutS(c_Configuration.GetServerRecieveTimeoutS())
{
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Using remote net server.",
                                   "NetServer.cpp", __LINE__);
}

NetServer::~NetServer() noexcept
{}

//*************************************************************************************
// Receive
//*************************************************************************************

MRH_Uint32 NetServer::Receive(MRH_Uint32 u32_StringID) noexcept
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
                                           "NetServer.cpp", __LINE__);
            continue;
        }
        else if (MRH_LS_BufferToMessage(&c_Message, v_Message.data(), v_Message.size()) < 0)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                                           "NetServer.cpp", __LINE__);
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
                                           "NetServer.cpp", __LINE__);
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

void NetServer::Send(OutputStorage& c_OutputStorage) noexcept
{
    if (c_OutputStorage.GetAvailable() == false)
    {
        return;
    }
    else if (LocalStream::IsConnected() == false)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Net server stream is not connected!",
                                       "NetServer.cpp", __LINE__);
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
                                           "NetServer.cpp", __LINE__);
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool NetServer::GetCommunicationActive() const noexcept
{
    return GetCommunicationActive(u64_RecieveTimestampS, u32_RecieveTimeoutS);
}

bool NetServer::GetCommunicationActive(MRH_Uint64 u64_TimestampS, MRH_Uint32 u32_TimeoutS) noexcept
{
    if (u64_TimestampS < (time(NULL) - u32_TimeoutS))
    {
        return false;
    }
    
    return true;
}
