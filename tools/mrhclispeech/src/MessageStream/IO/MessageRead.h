/**
 *  MessageRead.h
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

#ifndef MessageRead_h
#define MessageRead_h

// C / C++
#include <list>
#include <vector>

// External
#include <MRH_Typedefs.h>

// Project
#include "./MessagePacket.h"
#include "../../Exception.h"


class MessageRead
{
public:
    
    //*************************************************************************************
    // Destructor
    //*************************************************************************************
    
    /**
     *  Default destructor.
     */
    
    virtual ~MessageRead() noexcept;
    
private:
    
    //*************************************************************************************
    // Read
    //*************************************************************************************
    
    /**
     *  Read from a connected client.
     *
     *  \param i_SocketFD The socket to read from.
     *  \param i_TimeoutMS The poll timeout in milliseconds.
     *  \param p_Buffer The buffer to read into.
     *
     *  \return -1 on failure, 0 on no data, 1 on data read.
     */
    
    int ReadSocket(int i_SocketFD, int i_TimeoutMS, MRH_Uint8* p_Buffer) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::vector<MRH_Uint8> p_Unfinished[MessagePacket::PACKET_STREAM_COUNT];
    
    std::list<std::vector<MRH_Uint8>> l_Finished;
    
protected:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum ReadResult
    {
        READ_FAIL = -1,                  // Failure, close
        READ_SUCCESS = 0,                // No messages after read
        READ_MESSAGE_AVAILABLE = 1,      // New message after read
    };
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    MessageRead() noexcept;
    
    //*************************************************************************************
    // Clear
    //*************************************************************************************
    
    /**
     *  Clear all unfinished and finished messages.
     */
    
    void ClearRead() noexcept;
    
    //*************************************************************************************
    // Read
    //*************************************************************************************
    
    /**
     *  Read from a connected client.
     *
     *  \param i_SocketFD The socket to read from.
     *  \param i_TimeoutMS The poll timeout in milliseconds.
     *
     *  \return The read result for message availability.
     */
    
    ReadResult ReadMessages(int i_SocketFD, int i_TimeoutMS) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if finished messages are available.
     *
     *  \return true if a messages are available, false if not.
     */
    
    bool GetMessageAvailable() const noexcept;
    
    /**
     *  Get a fully read message.
     *
     *  \return The fully read message.
     */
    
    std::vector<MRH_Uint8> GetReadMessage();
};

#endif /* MessageRead_h */
