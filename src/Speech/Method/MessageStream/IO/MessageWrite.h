/**
 *  MessageWrite.h
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

#ifndef MessageWrite_h
#define MessageWrite_h

// C / C++
#include <list>
#include <vector>
#include <map>
#include <utility>

// External
#include <MRH_Typedefs.h>

// Project
#include "../../../../Exception.h"


class MessageWrite
{
public:
    
    //*************************************************************************************
    // Destructor
    //*************************************************************************************
    
    /**
     *  Default destructor.
     */
    
    virtual ~MessageWrite() noexcept;
    
private:
    
    //*************************************************************************************
    // Write
    //*************************************************************************************
    
    /**
     *  Write to a connected client.
     *
     *  \param i_SocketFD The socket to write to.
     *  \param p_Buffer The buffer to write from.
     *
     *  \return -1 on failure, 0 on not writeable, 1 on data written.
     */
    
    int WriteSocket(int i_SocketFD, const MRH_Uint8* p_Buffer) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // <Stream ID, <Started, Message Bytes>>
    std::map<MRH_Uint8, std::pair<bool, std::vector<MRH_Uint8>>> m_Write;
    
protected:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum WriteResult
    {
        WRITE_FAIL = -1,                  // Failure, close
        WRITE_MESSAGE_AVAILABLE = 0,      // Success, but messages remain
        WRITE_SUCCESS = 1,                // Data was written
    };
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    MessageWrite();
    
    //*************************************************************************************
    // Clear
    //*************************************************************************************
    
    /**
     *  Clear all unfinished messages.
     */
    
    void ClearWrite() noexcept;
    
    //*************************************************************************************
    // Add
    //*************************************************************************************
    
    /**
     *  Add a new message to write.
     *
     *  \param v_Message The message to add.
     */
    
    void AddWriteMessage(std::vector<MRH_Uint8>& v_Message);
    
    //*************************************************************************************
    // Write
    //*************************************************************************************
    
    /**
     *  Write unfinished messages to a connected client.
     *
     *  \param i_SocketFD The socket to write to.
     *
     *  \return The write result.
     */
    
    WriteResult WriteMessages(int i_SocketFD) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if messages are writeable.
     *
     *  \return true if a messages are writeable, false if not.
     */
    
    bool GetMessageWriteable() const noexcept;
};

#endif /* MessageWrite_h */
