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

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./MessageStream.h"

// Pre-defined
#ifndef MRH_SPEECH_MESSAGE_STREAM_SOCKET_DIR
    #define MRH_SPEECH_MESSAGE_STREAM_SOCKET_DIR "/tmp/mrh/"
#endif
#define MRH_SPEECH_MESSAGE_STREAM_SOCKET_PREFIX "mrhpsspeech_"
#define MRH_SPEECH_MESSAGE_STREAM_SOCKET_EXT ".sock"

#define MRH_SPEECH_CLIENT_TIMEOUT_S 300


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

MessageStream::MessageStream(std::string const& s_Channel) : b_Update(true),
                                                             s_Channel(s_Channel),
                                                             i_ConnectionFD(-1),
                                                             b_ClientConnected(false)
{
    // Create socket path
    std::string s_SocketPath = MRH_SPEECH_MESSAGE_STREAM_SOCKET_DIR
                               MRH_SPEECH_MESSAGE_STREAM_SOCKET_PREFIX +
                               s_Channel +
                               MRH_SPEECH_MESSAGE_STREAM_SOCKET_EXT;
    
    // @NOTE: No error check, might be closed correctly last time
    unlink(s_SocketPath.c_str());
    
    // Create a file descriptor first
#ifdef __APPLE__
    // @NOTE MacOS does not support SOCK_SEQPACKET
    //       If SOCK_STREAM works, great. If not, though luck
    //       This is just here for dev testing anyway
    if ((i_ConnectionFD = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
#else
    if ((i_ConnectionFD = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0)
#endif
    {
        throw Exception("Failed to create " +
                        s_Channel +
                        " connection socket: " +
                        std::string(std::strerror(errno)) +
                        " (" +
                        std::to_string(errno) +
                        ")!");
    }
    
    // Setup socket for connections
    struct sockaddr_un c_Address;
    memset(&c_Address, '\0', sizeof(c_Address));
    c_Address.sun_family = AF_UNIX;
    strcpy(c_Address.sun_path, s_SocketPath.c_str());
    
    if (bind(i_ConnectionFD, (struct sockaddr*)&c_Address, sizeof(c_Address)) < 0 ||
        listen(i_ConnectionFD, 1) < 0) // 1 = Connection backlog, only 1 is allowed
    {
        close(i_ConnectionFD);
        throw Exception("Failed to setup " +
                        s_Channel +
                        " connection socket: " +
                        std::string(std::strerror(errno)) +
                        " (" +
                        std::to_string(errno) +
                        ")!");
    }
    
    // Got our socket, now connect
    try
    {
        c_Thread = std::thread(Update, this);
    }
    catch (std::exception& e)
    {
        close(i_ConnectionFD);
        throw Exception("Failed to start " +
                        s_Channel +
                        " update thread: " +
                        std::string(e.what()));
    }
    
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, s_Channel +
                                                        " socket now available at " +
                                                        std::string(s_SocketPath) +
                                                        ".",
                                   "MessageStream.cpp", __LINE__);
}

MessageStream::~MessageStream() noexcept
{
    b_Update = false;
    c_Thread.join();
    
    CloseConnection(i_ConnectionFD);
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
    std::atomic<bool>& b_ClientConnected = p_Instance->b_ClientConnected;
    
    std::mutex& c_ReadMutex = p_Instance->c_ReadMutex;
    std::mutex& c_WriteMutex = p_Instance->c_WriteMutex;
    
    std::list<std::vector<MRH_Uint8>>& l_Read = p_Instance->l_Read;
    std::list<std::vector<MRH_Uint8>>& l_Write = p_Instance->l_Write;
    
    MessagePacket::PacketStream e_WriteStream;
    
    MRH_Uint64 u64_ClientTimeoutS = time(NULL) + MRH_SPEECH_CLIENT_TIMEOUT_S;
    int i_ReadTimeoutMS;
    
    while (p_Instance->b_Update == true)
    {
        /**
         *  Connection
         */
        
        // Connection required to be handled?
        if (i_ClientFD < 0)
        {
            if (b_ClientConnected == true)
            {
                b_ClientConnected = false;
            }
            
            if ((i_ClientFD = p_Instance->AcceptConnection()) < 0) // Blocking call
            {
                continue;
            }
            
            b_ClientConnected = true;
            u64_ClientTimeoutS = time(NULL) + MRH_SPEECH_CLIENT_TIMEOUT_S;
            
            // Clear data of old connection
            p_Instance->ClearRead();
            p_Instance->ClearWrite();
        }
        else if (time(NULL) > u64_ClientTimeoutS)
        {
            // Client timed out
            i_ClientFD = p_Instance->CloseConnection(i_ClientFD);
            continue;
        }
        
        /**
         *  Write
         */
        
        // Grab message if available
        c_WriteMutex.lock();
        
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
                // Added to stream
                It = l_Write.erase(It);
            }
            else
            {
                // Stream occupied, keep in order
                ++It;
            }
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
                // Reset client timeout, recieved data
                u64_ClientTimeoutS = time(NULL) + MRH_SPEECH_CLIENT_TIMEOUT_S;
                
                // Add everything which could be fully read
                while (p_Instance->GetMessageAvailable() == true)
                {
                    std::vector<MRH_Uint8> v_Message = p_Instance->GetReadMessage();
                    
                    if (MessageOpCode::GetOpCode(v_Message) != MessageOpCode::HELLO_C_HELLO)
                    {
                        std::lock_guard<std::mutex> c_Guard(c_ReadMutex);
                        l_Read.push_back(std::move(v_Message));
                    }
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
    b_ClientConnected = false;
}

int MessageStream::AcceptConnection() noexcept
{
    int i_SocketFD;
    struct sockaddr_un c_Adress;
    socklen_t us_ClientLen;
    
    if ((i_SocketFD = accept(i_ConnectionFD, (struct sockaddr*)&c_Adress, &us_ClientLen)) < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Client connection failed on channel " +
                                                                 s_Channel +
                                                                 ": " +
                                                                 std::string(std::strerror(errno)) +
                                                                 " (" +
                                                                 std::to_string(errno) +
                                                                 ")!",
                                           "MessageStream.cpp", __LINE__);
        }
        
        return -1;
    }
    else if (fcntl(i_SocketFD, F_SETFL, fcntl(i_SocketFD, F_GETFL, 0) | O_NONBLOCK) < 0)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Client socket setup failed on channel " +
                                                             s_Channel +
                                                             ": " +
                                                             std::string(std::strerror(errno)) +
                                                             " (" +
                                                             std::to_string(errno) +
                                                             ")!",
                                       "MessageStream.cpp", __LINE__);
        
        return CloseConnection(i_SocketFD);
    }
    
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Client connection accepted on channel " +
                                                        s_Channel,
                                   "MessageStream.cpp", __LINE__);
    return i_SocketFD;
}

int MessageStream::CloseConnection(int i_SocketFD) noexcept
{
    if (i_SocketFD != -1)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Client connection closed on channel " +
                                                            s_Channel +
                                                            " for socket " +
                                                            std::to_string(i_SocketFD),
                                       "MessageStream.cpp", __LINE__);
        
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
    return b_ClientConnected;
}
