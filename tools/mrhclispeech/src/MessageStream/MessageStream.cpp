/**
 *  MessageStream.cpp
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <iostream>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./MessageStream.h"

// Pre-defined
#ifndef MRH_SPEECH_MESSAGE_STREAM_SOCKET_PATH
    #define MRH_SPEECH_MESSAGE_STREAM_SOCKET_PATH "/tmp/mrh/mrhpsspeech.sock"
#endif

#define MRH_SPEECH_CLIENT_TIMEOUT_S 300


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

MessageStream::MessageStream(bool b_KeepAlive) : b_Update(true),
                                                 b_KeepAlive(b_KeepAlive),
                                                 b_Connected(false)
{
    try
    {
        c_Thread = std::thread(Update, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start update thread: " +
                        std::string(e.what()));
    }
}

MessageStream::~MessageStream() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Clear
//*************************************************************************************

void MessageStream::ClearRecieved(MRH_Uint8 u8_OpCode) noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_ReadMutex);
    
    for (auto It = l_Read.begin(); It != l_Read.end();)
    {
        if ((*It)[OPCODE_OPCODE_POS] == u8_OpCode)
        {
            It = l_Read.erase(It);
        }
        else
        {
            ++It;
        }
    }
}

void MessageStream::ClearAllRecieved() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_ReadMutex);
    l_Read.clear();
}

//*************************************************************************************
// Update
//*************************************************************************************

void MessageStream::Update(MessageStream* p_Instance) noexcept
{
    int i_ClientFD = -1;
    bool b_KeepAlive = p_Instance->b_KeepAlive;
    std::atomic<bool>& b_Connected = p_Instance->b_Connected;
    
    std::mutex& c_ReadMutex = p_Instance->c_ReadMutex;
    std::mutex& c_WriteMutex = p_Instance->c_WriteMutex;
    
    std::list<std::vector<MRH_Uint8>>& l_Read = p_Instance->l_Read;
    std::list<std::vector<MRH_Uint8>>& l_Write = p_Instance->l_Write;
    
    MessagePacket::PacketStream e_WriteStream;
    
    MRH_Uint64 u64_NextHelloS = time(NULL) + MRH_SPEECH_CLIENT_TIMEOUT_S * 0.9f;
    int i_ReadTimeoutMS;
    
    while (p_Instance->b_Update == true)
    {
        /**
         *  Connection
         */
        
        // Connection required to be handled?
        if (i_ClientFD < 0)
        {
            if (b_Connected == true)
            {
                b_Connected = false;
            }
            
            if ((i_ClientFD = p_Instance->Connect()) < 0) // Blocking call
            {
                continue;
            }
            
            b_Connected = true;
            
            if (b_KeepAlive == true)
            {
                u64_NextHelloS = time(NULL) + MRH_SPEECH_CLIENT_TIMEOUT_S * 0.9f;
            }
            
            // Clear data of old connection
            p_Instance->ClearRead();
            p_Instance->ClearWrite();
        }
        
        /**
         *  Write
         */
        
        // Grab message if available
        c_WriteMutex.lock();
        
        if (l_Write.size() > 0)
        {
            for (auto It = l_Write.begin(); It != l_Write.end();)
            {
                // Add to the right stream
                switch (MessageOpCode::GetOpCode(l_Write.front()))
                {
                    case MessageOpCode::STRING_CS_STRING:
                    case MessageOpCode::AUDIO_CS_AUDIO:
                        e_WriteStream = MessagePacket::STREAM_SPEECH;
                        break;
                        
                    default:
                        e_WriteStream = MessagePacket::STREAM_COMMAND;
                        break;
                }
                
                if (p_Instance->AddWriteMessage(e_WriteStream, l_Write.front()) == true)
                {
                    It = l_Write.erase(It);
                }
                else
                {
                    ++It;
                }
            }
            
            // Wrote messages, move hello timeout
            if (b_KeepAlive == true)
            {
                u64_NextHelloS = time(NULL) + MRH_SPEECH_CLIENT_TIMEOUT_S;
            }
        }
        else if (b_KeepAlive == true && time(NULL) >= u64_NextHelloS)
        {
            MessageOpCode::OpCodeData c_HelloOpCode(MessageOpCode::HELLO_C_HELLO);
            p_Instance->AddWriteMessage(MessagePacket::STREAM_COMMAND, c_HelloOpCode.v_Data);
            
            u64_NextHelloS = time(NULL) + MRH_SPEECH_CLIENT_TIMEOUT_S;
        }
        
        c_WriteMutex.unlock();
        
        // Now write bytes
        if (p_Instance->GetMessageWriteable() == true)
        {
            if (p_Instance->WriteMessages(i_ClientFD) == MessageWrite::WRITE_FAIL)
            {
                // Write failed, disconnect
                i_ClientFD = p_Instance->CloseConnection(i_ClientFD);
                continue;
            }
        }
        
        /**
         *  Read
         */
        
        i_ReadTimeoutMS = p_Instance->GetMessageWriteable() == true ? 0 : 100;
        
        switch (p_Instance->ReadMessages(i_ClientFD, i_ReadTimeoutMS))
        {
            case MessageRead::READ_SUCCESS:
            {
                // Read but no data
                break;
            }
            case MessageRead::READ_MESSAGE_AVAILABLE:
            {
                // Add everything which could be fully read
                while (p_Instance->GetMessageAvailable() == true)
                {
                    std::lock_guard<std::mutex> c_Guard(c_ReadMutex);
                    l_Read.push_back(p_Instance->GetReadMessage());
                }
                break;
            }
                
            default:
            {
                // Read failed, disconnect
                i_ClientFD = p_Instance->CloseConnection(i_ClientFD);
                break;
            }
        }
    }
    
    // Update ended
    i_ClientFD = p_Instance->CloseConnection(i_ClientFD);
    b_Connected = false;
}

int MessageStream::Connect() noexcept
{
    /**
     *  Connection
     */
    
    int i_SocketFD;
    
#ifdef __APPLE__
    // @NOTE MacOS does not support SOCK_SEQPACKET
    //       If SOCK_STREAM works, great. If not, though luck
    //       This is just here for dev testing anyway
    if ((i_SocketFD = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
#else
    if ((i_SocketFD = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0)
#endif
    {
        std::cout << "[ ERROR ] Failed to create socket "
                  << MRH_SPEECH_MESSAGE_STREAM_SOCKET_PATH
                  << ": "
                  << std::string(std::strerror(errno))
                  << " ("
                  << std::to_string(errno)
                  << ")!"
                  << std::endl;
        
        return -1;
    }
    
    struct sockaddr_un c_Address;
    socklen_t us_AddressLength;
    
    us_AddressLength = sizeof(c_Address);
    memset(&c_Address, 0, us_AddressLength);
    c_Address.sun_family = AF_UNIX;
    strcpy(c_Address.sun_path, MRH_SPEECH_MESSAGE_STREAM_SOCKET_PATH);
    
    if (connect(i_SocketFD, (struct sockaddr*)&c_Address, us_AddressLength) < 0)
    {
        std::cout << "[ ERROR ] Socket connection failed for socket "
                  << MRH_SPEECH_MESSAGE_STREAM_SOCKET_PATH
                  << ": "
                  << std::string(std::strerror(errno))
                  << " ("
                  << std::to_string(errno)
                  << ")!"
                  << std::endl;
        
        return CloseConnection(i_SocketFD);
    }
    else if (fcntl(i_SocketFD, F_SETFL, fcntl(i_SocketFD, F_GETFL, 0) | O_NONBLOCK) < 0)
    {
        std::cout << "[ ERROR ] Failed to setup socket "
                  << MRH_SPEECH_MESSAGE_STREAM_SOCKET_PATH
                  << ": "
                  << std::string(std::strerror(errno))
                  << " ("
                  << std::to_string(errno)
                  << ")!"
                  << std::endl;
        
        return CloseConnection(i_SocketFD);
    }
    
    std::cout << "Connected on socket "
              << MRH_SPEECH_MESSAGE_STREAM_SOCKET_PATH
              << "."
              << std::endl;
    
    return i_SocketFD;
}

int MessageStream::CloseConnection(int i_SocketFD) noexcept
{
    if (i_SocketFD != -1)
    {
        std::cout << "Connection closed for socket "
                  << MRH_SPEECH_MESSAGE_STREAM_SOCKET_PATH
                  << " ("
                  << std::to_string(i_SocketFD)
                  << ")."
                  << std::endl;
        
        close(i_SocketFD);
    }
    
    return -1;
}

//*************************************************************************************
// Recieve
//*************************************************************************************

bool MessageStream::Recieve(std::vector<MRH_Uint8>& v_Data)
{
    std::lock_guard<std::mutex> c_Guard(c_ReadMutex);
    
    // No data available
    if (l_Read.size() == 0)
    {
        return false;
    }
    
    v_Data.swap(l_Read.front());
    l_Read.pop_front();
    
    return true;
}

//*************************************************************************************
// Send
//*************************************************************************************

void MessageStream::Send(std::vector<MRH_Uint8>& v_Data)
{
    // Minimal data size met?
    if (v_Data.size() < OPCODE_DATA_POS)
    {
        throw Exception("Data does not meet minimum requirements!");
    }
    
    // Build message
    std::vector<MRH_Uint8> v_Send;
    v_Send.swap(v_Data);
    
    // Now add to list for thread to grab
    std::lock_guard<std::mutex> c_Guard(c_WriteMutex);
    l_Write.push_back(std::move(v_Send));
}

void MessageStream::Send(std::vector<MRH_Uint8> const& v_Data)
{
    // Minimal data size met?
    if (v_Data.size() < OPCODE_DATA_POS)
    {
        throw Exception("Data does not meet minimum requirements!");
    }
    
    // Add message copy
    std::lock_guard<std::mutex> c_Guard(c_WriteMutex);
    l_Write.emplace_back(v_Data);
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool MessageStream::GetConnected() const noexcept
{
    return b_Connected;
}
