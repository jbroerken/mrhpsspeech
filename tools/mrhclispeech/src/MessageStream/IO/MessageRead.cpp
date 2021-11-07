/**
 *  MessageRead.cpp
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
#include <poll.h>
#include <errno.h>
#include <iostream>

// External

// Project
#include "./MessageRead.h"

// Pre-defined
using namespace MessagePacket;


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

MessageRead::MessageRead() noexcept
{}

MessageRead::~MessageRead() noexcept
{}

//*************************************************************************************
// Clear
//*************************************************************************************

void MessageRead::ClearRead() noexcept
{
    for (MRH_Uint8 i = 0; i < PACKET_STREAM_COUNT; ++i)
    {
        p_Unfinished[i].clear();
    }
    
    l_Finished.clear();
}

//*************************************************************************************
// Read
//*************************************************************************************

MessageRead::ReadResult MessageRead::ReadMessages(int i_SocketFD, int i_TimeoutMS) noexcept
{
    static MRH_Uint8 p_Buffer[MESSAGE_PACKET_SIZE];
    int i_ReadResult;
    
    while (true)
    {
        // Read packet first
        i_ReadResult = ReadSocket(i_SocketFD, i_TimeoutMS, p_Buffer);
        
        if (i_ReadResult < 0)
        {
            return READ_FAIL;
        }
        else if (i_ReadResult == 0)
        {
            return (l_Finished.size() > 0 ? READ_MESSAGE_AVAILABLE : READ_SUCCESS);
        }
        
        // Got packet, select stream
        MRH_Uint8 u8_Stream = p_Buffer[MESSAGE_PACKET_STREAM_ID_POS];
        
        if (u8_Stream > PACKET_STREAM_MAX)
        {
            // Packet stream does not exist
            return READ_FAIL;
        }
        
        std::vector<MRH_Uint8>& v_Stream = p_Unfinished[u8_Stream];
        
        // Handle packet
        MRH_Uint16 u16_PayloadSize = *((MRH_Uint16*)&(p_Buffer[MESSAGE_PACKET_PAYLOAD_SIZE_POS]));
        
        MRH_Uint8* p_Start = &p_Buffer[MESSAGE_PACKET_PAYLOAD_POS];
        MRH_Uint8* p_End = p_Buffer + MESSAGE_PACKET_PAYLOAD_POS + u16_PayloadSize;
        
        switch (p_Buffer[MESSAGE_PACKET_TYPE_POS])
        {
            case TYPE_START:
                v_Stream = { p_Start,
                             p_End };
                break;
                
            case TYPE_CONT:
                v_Stream.insert(v_Stream.end(),
                                p_Start,
                                p_End);
                break;
                
            case TYPE_END:
                v_Stream.insert(v_Stream.end(),
                                p_Start,
                                p_End);
                l_Finished.emplace_back();
                l_Finished.back().swap(v_Stream);
                break;
                
            case TYPE_SINGLE:
                if (v_Stream.size() > 0)
                {
                    // Old unfinished data in stream, clear
                    v_Stream.clear();
                }
                l_Finished.emplace_back(p_Start,
                                        p_End);
                break;
                
            default:
                return READ_FAIL;
        }
    }
}

int MessageRead::ReadSocket(int i_SocketFD, int i_TimeoutMS, MRH_Uint8* p_Buffer) noexcept
{
    // Poll for data first
    struct pollfd s_PollFD;
    
    s_PollFD.fd = i_SocketFD;
    s_PollFD.events = POLLIN;
    
    switch (poll(&s_PollFD, (nfds_t)1, i_TimeoutMS))
    {
        case -1:
            std::cout << "[ ERROR ] Socket polling failed for socket "
                      << std::to_string(i_SocketFD)
                      << ": "
                      << std::string(std::strerror(errno))
                      << " ("
                      << std::to_string(errno)
                      << ")!"
                      << std::endl;
            
            return -1; // Polling failed
        case 0:
            return 0; // No data, but polling is ok
            
        default:
            break;
    }
    
    // Now read
    ssize_t ss_Read = read(i_SocketFD, p_Buffer, MESSAGE_PACKET_SIZE);
    
    if (ss_Read < 0)
    {
        // Would be blocking?
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            return 0;
        }
        
        std::cout << "[ ERROR ] Socket read failed for socket "
                  << std::to_string(i_SocketFD)
                  << ": "
                  << std::string(std::strerror(errno))
                  << " ("
                  << std::to_string(errno)
                  << ")!"
                  << std::endl;
        
        return -1;
    }
    else if (ss_Read == 0)
    {
        std::cout << "[ ERROR ] 0 bytes read but polling succeeded for socket "
                  << std::to_string(i_SocketFD)
                  << std::endl;
        
        return -1;
    }
    else if (ss_Read != MESSAGE_PACKET_SIZE)
    {
        std::cout << "[ ERROR ] Invalid byte count read for socket "
                  << std::to_string(i_SocketFD)
                  << std::endl;
        
        return -1;
    }
    
    return 1;
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool MessageRead::GetMessageAvailable() const noexcept
{
    return l_Finished.size() > 0 ? true : false;
}

std::vector<MRH_Uint8> MessageRead::GetReadMessage()
{
    if (l_Finished.size() == 0)
    {
        throw Exception("No messages available!");
    }
    
    std::vector<MRH_Uint8> v_Result;
    
    v_Result.swap(l_Finished.front());
    l_Finished.pop_front();
    
    return v_Result;
}
