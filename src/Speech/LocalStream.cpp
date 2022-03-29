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
    std::deque<std::pair<MRH_StreamMessage, void*>>& dq_Send = p_Instance->dq_Send;
    std::deque<std::pair<MRH_StreamMessage, void*>>& dq_Received = p_Instance->dq_Received;
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
    MRH_LSM_Version_Data c_Version;
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
                p_Instance->b_Connected = false;
            }
            
            // Attempt to connect
            if (MRH_LS_Connect(p_Stream) < 0)
            {
                // Wait before retry if connection error
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            else if (p_Instance->b_Connected == false)
            {
                p_Instance->b_Connected = true;
            }
            
            // Connected, add version info
            if (MRH_LS_SetMessage(p_Stream, MRH_LSM_VERSION, &c_Version) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                             "LocalStream.cpp", __LINE__);
            }
        }
        
        /**
         *  Write
         */
        
        // Do we need to set a message to write?
        if (MRH_LS_GetMessageSet(p_Stream) < 0)
        {
            std::lock_guard<std::mutex> c_Guard(c_SendMutex);
            
            if (dq_Send.size() > 0)
            {
                auto& Front = dq_Send.front();
                
                if (MRH_LS_SetMessage(p_Stream, Front.first, Front.second) < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                                 "LocalStream.cpp", __LINE__);
                }
                
                // Clear always
                p_Instance->DestroyMessage(Front.first, Front.second);
                dq_Send.pop_front();
            }
        }
        
        // Check again, message to write?
        if (MRH_LS_GetMessageSet(p_Stream) == 0)
        {
            // Write message data
            // @NOTE: No loop, skip to reading to empty socket if unfinished
            if (MRH_LS_Write(p_Stream) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                             "LocalStream.cpp", __LINE__);
                
                // Failed, disconnect
                MRH_LS_Disconnect(p_Stream);
                continue;
            }
        }
        
        /**
         *  Read
         */
        
        // Read message data
        // @NOTE: No loop, skip to writing to empty socket
        i_Result = MRH_LS_Read(p_Stream, 100);
        
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
            MRH_StreamMessage e_Message = MRH_LS_GetLastMessage(p_Stream);
            void* p_Data = NULL;
            
            switch (e_Message)
            {
                case MRH_LSM_STRING:
                    if ((p_Data = (MRH_LSM_String_Data*)malloc(sizeof(MRH_LSM_String_Data))) == NULL)
                    {
                        c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to allocate string message data!",
                                     "LocalStream.cpp", __LINE__);
                    }
                    break;
                case MRH_LSM_AUDIO:
                    if ((p_Data = (MRH_LSM_Audio_Data*)malloc(sizeof(MRH_LSM_Audio_Data))) == NULL)
                    {
                        c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to allocate audio message data!",
                                     "LocalStream.cpp", __LINE__);
                    }
                    break;
                    
                default:
                    break;
            }
            
            // Got data, add
            if (p_Data != NULL)
            {
                if (MRH_LS_GetLastMessageData(p_Stream, p_Data) < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                                 "LocalStream.cpp", __LINE__);
                }
                else
                {
                    std::lock_guard<std::mutex> c_Guard(c_ReceiveMutex);
                    dq_Received.emplace_back(e_Message, p_Data);
                }
            }
        }
        // @NOTE: No 1, retry happens after write
    }
    
    // Termination, close stream
    MRH_LS_Close(p_Stream);
}

//*************************************************************************************
// Clear
//*************************************************************************************

void* LocalStream::DestroyMessage(MRH_StreamMessage e_Message, void* p_Data) noexcept
{
    if (p_Data == NULL)
    {
        return NULL;
    }
    
    switch (e_Message)
    {
        case MRH_LSM_STRING:
            free(((MRH_LSM_String_Data*)p_Data)->p_String);
            free((MRH_LSM_String_Data*)p_Data);
            return NULL;
        case MRH_LSM_AUDIO:
            free(((MRH_LSM_Audio_Data*)p_Data)->p_Samples);
            free((MRH_LSM_Audio_Data*)p_Data);
            return NULL;
            
        default:
            return p_Data;
    }
}

void LocalStream::ClearReceived() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_SendMutex);
    
    for (auto& Message : dq_Received)
    {
        DestroyMessage(Message.first, Message.second);
    }
}

void LocalStream::ClearSend() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_SendMutex);
    
    for (auto& Message : dq_Send)
    {
        DestroyMessage(Message.first, Message.second);
    }
}

//*************************************************************************************
// Send
//*************************************************************************************

void LocalStream::Send(MRH_StreamMessage& e_Message, void*& p_Data)
{
    try
    {
        std::lock_guard<std::mutex> c_Guard(c_SendMutex);
        dq_Send.emplace_back(e_Message, p_Data);
        
        // Reset
        e_Message = MRH_LSM_UNK;
        p_Data = NULL;
    }
    catch (std::exception& e)
    {
        throw Exception(e.what());
    }
}

//*************************************************************************************
// Recieve
//*************************************************************************************

bool LocalStream::Receive(MRH_StreamMessage& e_Message, void*& p_Data) noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_ReceiveMutex);
    
    if (dq_Received.size() == 0)
    {
        return false;
    }
    
    auto& Front = dq_Received.front();
    
    e_Message = Front.first;
    p_Data = Front.second;
    
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
