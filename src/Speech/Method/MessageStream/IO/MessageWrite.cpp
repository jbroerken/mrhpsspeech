/**
 *  MessageWrite.cpp
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

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./MessageWrite.h"

// Pre-defined
using namespace MessagePacket;

#define PAYLOAD_SIZE_MAX MESSAGE_PACKET_SIZE - MESSAGE_PACKET_PAYLOAD_POS


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

MessageWrite::MessageWrite() noexcept
{
    for (MRH_Uint8 i = 0; i < PACKET_STREAM_COUNT; ++i)
    {
        p_Stream[i].first = false;
    }
}

MessageWrite::~MessageWrite() noexcept
{}

//*************************************************************************************
// Clear
//*************************************************************************************

void MessageWrite::ClearWrite() noexcept
{
    for (MRH_Uint8 i = 0; i < PACKET_STREAM_COUNT; ++i)
    {
        p_Stream[i].first = false;
        p_Stream[i].second.clear();
    }
}

//*************************************************************************************
// Add
//*************************************************************************************

bool MessageWrite::AddWriteMessage(MessagePacket::PacketStream e_Stream, std::vector<MRH_Uint8>& v_Message) noexcept
{
    if (e_Stream > PACKET_STREAM_MAX)
    {
        // Stream not existing
        return false;
    }
    else if (p_Stream[e_Stream].second.size() > 0)
    {
        // Stream occupied
        return false;
    }
    
    p_Stream[e_Stream].first = false; // Not started to send, new
    p_Stream[e_Stream].second.swap(v_Message);
    
    return true;
}

//*************************************************************************************
// Write
//*************************************************************************************

MessageWrite::WriteResult MessageWrite::WriteMessages(int i_SocketFD) noexcept
{
    static MRH_Uint8 p_Buffer[MESSAGE_PACKET_SIZE];
    int i_WriteResult;
    MRH_Uint16 u16_PayloadSize;
    bool b_MessageRemains = false;
    
    for (MRH_Uint8 i = 0; i < PACKET_STREAM_COUNT; ++i)
    {
        bool& b_Started = p_Stream[i].first;
        std::vector<MRH_Uint8>& v_Message = p_Stream[i].second;
        
        // Do we need to write something?
        if (v_Message.size() == 0)
        {
            continue;
        }
        
        // Set stream id, same for all packets
        p_Buffer[MESSAGE_PACKET_STREAM_ID_POS] = i;
        
        // Write as many packets as possible for stream
        while (v_Message.size() > 0)
        {
            // Setup packet for writing
            if (v_Message.size() > PAYLOAD_SIZE_MAX)
            {
                u16_PayloadSize = PAYLOAD_SIZE_MAX;
                
                // Packet type setting
                p_Buffer[MESSAGE_PACKET_TYPE_POS] = b_Started ? TYPE_CONT : TYPE_START;
            }
            else
            {
                u16_PayloadSize = v_Message.size();
                
                // Pad payload
                std::memset(&(p_Buffer[MESSAGE_PACKET_PAYLOAD_POS + u16_PayloadSize]),
                            '\0',
                            PAYLOAD_SIZE_MAX - u16_PayloadSize);
                
                // Packet type setting
                p_Buffer[MESSAGE_PACKET_TYPE_POS] = b_Started ? TYPE_END : TYPE_SINGLE;
            }
            
            std::memcpy(&(p_Buffer[MESSAGE_PACKET_PAYLOAD_SIZE_POS]),
                        &u16_PayloadSize,
                        MESSAGE_PACKET_PAYLOAD_SIZE_SIZE);
            
            std::memcpy(&p_Buffer[MESSAGE_PACKET_PAYLOAD_POS],
                        &v_Message[0],
                        u16_PayloadSize);
            
            // Now write packet
            i_WriteResult = WriteSocket(i_SocketFD, p_Buffer);
            
            if (i_WriteResult < 0)
            {
                return WRITE_FAIL;
            }
            if (i_WriteResult == 0)
            {
                // Notifier set
                if (b_MessageRemains == false)
                {
                    b_MessageRemains = true;
                }
                
                // Could not completely write, end here for stream
                // @NOTE: We keep going with the next stream in case
                //        of the other end making space to send
                break;
            }
            
            // Set started if not done so
            if (b_Started == false)
            {
                b_Started = true;
            }
            
            // Remove written content
            v_Message.erase(v_Message.begin(),
                            v_Message.begin() + u16_PayloadSize);
        }
    }
    
    // Did we manage to write everything or not?
    return b_MessageRemains ? WRITE_MESSAGE_AVAILABLE : WRITE_SUCCESS;
}

int MessageWrite::WriteSocket(int i_SocketFD, const MRH_Uint8* p_Buffer) noexcept
{
    ssize_t ss_Write = write(i_SocketFD, p_Buffer, MESSAGE_PACKET_SIZE);
    
    if (ss_Write < 0)
    {
        // Would be blocking?
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            return 0;
        }
        
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Socket write failed for socket " +
                                                             std::to_string(i_SocketFD) +
                                                             ": " +
                                                             std::string(std::strerror(errno)) +
                                                             " (" +
                                                             std::to_string(errno) +
                                                             ")!",
                                       "MessageWrite.cpp", __LINE__);
        return -1;
    }
    
    return 1;
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool MessageWrite::GetMessageWriteable() const noexcept
{
    for (MRH_Uint8 i = 0; i < PACKET_STREAM_COUNT; ++i)
    {
        if (p_Stream[i].second.size() > 0)
        {
            return true;
        }
    }
    
    return false;
}
