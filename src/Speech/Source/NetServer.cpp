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
    MRH_StreamMessage e_Message;
    void* p_Data;
    std::string s_String;
    
    // Read all messages recieved
    while (LocalStream::Receive(e_Message, p_Data) == true)
    {
        if (e_Message == MRH_LSM_STRING)
        {
            try
            {
                s_String = std::string(((MRH_LSM_String_Data*)p_Data)->p_String,
                                       ((MRH_LSM_String_Data*)p_Data)->u32_Size);
            
                SpeechEvent::InputRecieved(u32_StringID, std::string());
                ++u32_StringID;
            }
            catch (Exception& e)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                               "NetServer.cpp", __LINE__);
            }
        }
        
        if (LocalStream::DestroyMessage(e_Message, p_Data) != NULL)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Message deallocation failed!",
                                           "NetServer.cpp", __LINE__);
        }
    }
    
    // If the string is > 0 then we recieved new info
    if (s_String.size() > 0)
    {
        u64_RecieveTimestampS = time(NULL);
    }
    
    // Return new string id
    return u32_StringID;
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
            
            MRH_StreamMessage e_Message = MRH_LSM_STRING;
            void* p_Data = (MRH_LSM_String_Data*)malloc(sizeof(MRH_LSM_String_Data));
            
            if (p_Data)
            {
                throw Exception("Failed to allocate output data!");
            }
            
            MRH_LSM_String_Data* p_Cast = (MRH_LSM_String_Data*)p_Data;
            p_Cast->u32_Size = String.s_String.size();
            
            if ((p_Cast->p_String = (char*)malloc(p_Cast->u32_Size)) == NULL)
            {
                free(p_Cast);
                throw Exception("Failed to allocate output string!");
            }
            
            strncpy(p_Cast->p_String, String.s_String.c_str(), p_Cast->u32_Size);
            
            // Send and set performed
            LocalStream::Send(e_Message, p_Data);
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
