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
#include <libmrhmstream/MRH_MessageOpCode.h>

// Project
#include "./NetServer.h"
#include "../SpeechEvent.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

NetServer::NetServer(Configuration const& c_Configuration) : c_Stream(c_Configuration.GetServerSocketPath(),
                                                                      true),
                                                             u64_RecieveTimestampS(0),
                                                             u32_RecieveTimeoutS(c_Configuration.GetServerRecieveTimeoutS())
{
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Using remote net server.",
                                   "NetServer.cpp", __LINE__);
}

NetServer::~NetServer() noexcept
{}

//*************************************************************************************
// Exchange
//*************************************************************************************

void NetServer::Receive() noexcept
{
    // No stream client or data, do nothing
    if (c_Stream.GetConnected() == false)
    {
        return;
    }
    
    // Recieve data
    MRH_MessageOpCode::STRING_CS_STRING_DATA c_OpCode("");
    
    // Recieve as many as possible!
    size_t us_OldReceived = l_Recieved.size();
    
    while (c_Stream.Recieve(c_OpCode.v_Data) == true)
    {
        // Is this a usable opcode?
        if (c_OpCode.GetOpCode() != MRH_MessageOpCode::STRING_CS_STRING)
        {
            continue;
        }
        
        l_Recieved.emplace_back(c_OpCode.GetString());
    }
    
    if (us_OldReceived != l_Recieved.size())
    {
        u64_RecieveTimestampS = time(NULL);
    }
}

MRH_Uint32 NetServer::Exchange(MRH_Uint32 u32_StringID, OutputStorage& c_OutputStorage) noexcept
{
    // Input
    try
    {
        for (auto It = l_Recieved.begin(); It != l_Recieved.end();)
        {
            SpeechEvent::InputRecieved(u32_StringID, *It);
            ++u32_StringID;
            
            It = l_Recieved.erase(It);
        }
    }
    catch (Exception& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                       "NetServer.cpp", __LINE__);
    }
    
    // Output
    if (c_OutputStorage.GetFinishedAvailable() == false)
    {
        return u32_StringID;
    }
    else if (c_Stream.GetConnected() == false)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Net server stream is not connected!",
                                       "NetServer.cpp", __LINE__);
        return u32_StringID;
    }
    
    while (c_OutputStorage.GetFinishedAvailable() == true)
    {
        try
        {
            auto String = c_OutputStorage.GetFinishedString();
            
            // Send to client
            c_Stream.Send(MRH_MessageOpCode::STRING_CS_STRING_DATA(String.s_String).v_Data);
            
            // Immediatly inform of performed
            SpeechEvent::OutputPerformed(String.u32_StringID,
                                         String.u32_GroupID);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                           "NetServer.cpp", __LINE__);
        }
    }
    
    // Result
    return u32_StringID;
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
