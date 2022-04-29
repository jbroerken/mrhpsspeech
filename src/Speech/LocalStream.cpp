/**
 *  LocalStream.cpp
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
#include <libmrhls.h>

// Project
#include "./LocalStream.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

LocalStream::LocalStream(std::string const& s_FilePath) : b_Update(true),
                                                          b_Connected(false)
{
    try
    {
        c_Thread = std::thread(Update, 
                               this, 
                               s_FilePath);
    }
    catch (std::exception& e)
    {
        throw Exception(e.what());
    }
}

LocalStream::~LocalStream() noexcept
{
    b_Update = false;
    c_Thread.join();
    
    ClearReceived();
    ClearSend();
}

//*************************************************************************************
// Update
//*************************************************************************************

void LocalStream::Update(LocalStream* p_Instance, std::string s_FilePath) noexcept
{
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    std::deque<std::vector<MRH_Uint8>>& dq_Send = p_Instance->dq_Send;
    std::deque<std::vector<MRH_Uint8>>& dq_Received = p_Instance->dq_Received;
    std::mutex& c_SendMutex = p_Instance->c_SendMutex;
    std::mutex& c_ReceiveMutex = p_Instance->c_ReceiveMutex;
    int i_Result;
    
    // Build stream first
    c_Logger.Log(MRH_PSBLogger::INFO, "Opening local stream: " + s_FilePath,
                 "LocalStream.cpp", __LINE__);
    
    MRH_LocalStream* p_Stream = MRH_LS_Open(s_FilePath.c_str(), 0);
    
    if (p_Stream == NULL)
    {
        c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                     "LocalStream.cpp", __LINE__);
        return;
    }
    
    // Now begin exchange
    MRH_Uint8 p_Send[MRH_STREAM_MESSAGE_BUFFER_SIZE];
    MRH_Uint8 p_Recieve[MRH_STREAM_MESSAGE_BUFFER_SIZE];
    MRH_Uint32 u32_SendSize;
    MRH_Uint32 u32_RecieveSize;
    
    MRH_LS_M_Version_Data c_Version;
    c_Version.u32_Version = MRH_STREAM_MESSAGE_VERSION;
    
    while (p_Instance->b_Update == true)
    {
        /**
         *  Connect
         */
        
        if (MRH_LS_GetConnected(p_Stream) < 0)
        {
            // Switch flag
            if (p_Instance->b_Connected == true)
            {
                c_Logger.Log(MRH_PSBLogger::INFO, "Local stream client disconnected.",
                             "LocalStream.cpp", __LINE__);
                
                p_Instance->b_Connected = false;
            }
            
            // Attempt to connect
            if (MRH_LS_Connect(p_Stream) < 0)
            {
                // Error?
                if (MRH_ERR_GetLocalStreamError() != MRH_LOCAL_STREAM_ERROR_UNK)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Connect failed: " + 
                                                       std::string(MRH_ERR_GetLocalStreamErrorString()), 
                                 "LocalStream.cpp", __LINE__);
                    
                    // Reset is important here, so that simply no partner existing won't throw the
                    // same error again
                    MRH_ERR_LocalStreamReset();
                }
                
                // Wait before retry if connection error
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            else if (p_Instance->b_Connected == false)
            {
                c_Logger.Log(MRH_PSBLogger::INFO, "Local stream client connected.",
                             "LocalStream.cpp", __LINE__);
                
                p_Instance->b_Connected = true;
            }
            
            // Connected, add version info
            if (MRH_LS_MessageToBuffer(p_Send, &u32_SendSize, MRH_LS_M_VERSION, &c_Version) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                             "LocalStream.cpp", __LINE__);
                
                // Reset for connect
                MRH_ERR_LocalStreamReset();
            }
            else
            {
                std::lock_guard<std::mutex> c_Guard(c_SendMutex);
                dq_Send.emplace_back(p_Send, p_Send + u32_SendSize);
            }
        }
        
        /**
         *  Write
         */
        
        // Lock
        c_SendMutex.lock();
        
        // Now write if possible
        if (dq_Send.size() > 0)
        {
            auto& Current = dq_Send.front();
            i_Result = MRH_LS_Write(p_Stream, Current.data(), Current.size());
            
            if (i_Result < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                             "LocalStream.cpp", __LINE__);
                
                // Failed, disconnect
                MRH_LS_Disconnect(p_Stream);
                continue;
            }
            else if (i_Result == 0)
            {
                // Done writing
                dq_Send.pop_front();
            }
            // @NOTE: 1 is handled next loop
        }
        
        // Done, unlock
        c_SendMutex.unlock();
        
        /**
         *  Read
         */
        
        // Read message data
        // @NOTE: No loop, skip to writing to empty socket
        i_Result = MRH_LS_Read(p_Stream, 100, p_Recieve, &u32_RecieveSize);
        
        if (i_Result < 0)
        {
            c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                         "LocalStream.cpp", __LINE__);
            
            // Failed, disconnect
            MRH_LS_Disconnect(p_Stream);
            continue;
        }
        else if (i_Result == 0)
        {
            std::lock_guard<std::mutex> c_Guard(c_ReceiveMutex);
            dq_Received.emplace_back(p_Recieve, p_Recieve + u32_RecieveSize);
        }
        // @NOTE: No 1, retry happens after write
    }
    
    // Termination, close stream
    MRH_LS_Close(p_Stream);
}

//*************************************************************************************
// Clear
//*************************************************************************************

void LocalStream::ClearReceived() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_SendMutex);
    dq_Received.clear();
}

void LocalStream::ClearSend() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_SendMutex);
    dq_Send.clear(); // @NOTE: Local stream write copies, safe to clear all
}

//*************************************************************************************
// Send
//*************************************************************************************

void LocalStream::Send(std::vector<MRH_Uint8>& v_Data)
{
    try
    {
        std::lock_guard<std::mutex> c_Guard(c_SendMutex);
        
        dq_Send.emplace_back();
        dq_Send.back().swap(v_Data);
    }
    catch (std::exception& e)
    {
        throw Exception(e.what());
    }
}

//*************************************************************************************
// Recieve
//*************************************************************************************

bool LocalStream::Receive(std::vector<MRH_Uint8>& v_Data) noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_ReceiveMutex);
    
    if (dq_Received.size() == 0)
    {
        return false;
    }
    
    v_Data.swap(dq_Received.front());
    dq_Received.pop_front();
    
    return true;
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool LocalStream::IsConnected() noexcept
{
    return b_Connected;
}
