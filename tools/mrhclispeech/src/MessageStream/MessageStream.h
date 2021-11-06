/**
 *  MessageStream.h
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

#ifndef MessageStream_h
#define MessageStream_h

// C / C++
#include <thread>
#include <atomic>
#include <string>

// External
#include <MRH_Typedefs.h>

// Project
#include "./IO/MessageRead.h"
#include "./IO/MessageWrite.h"
#include "./MessageOpCode.h"


class MessageStream : private MessageRead,
                      private MessageWrite
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param s_Channel The name identifier of the stream channel.
     *  \param b_KeepAlive If the connection should be kept alive automatically.
     */
    
    MessageStream(std::string const& s_Channel,
                  bool b_KeepAlive);
    
    /**
     *  Default destructor.
     */
    
    ~MessageStream() noexcept;
    
    //*************************************************************************************
    // Clear
    //*************************************************************************************
    
    /**
     *  Clear all recieved messages of a type.
     *
     *  \param u8_OpCode The type of opcode messages to clear.
     */
    
    void ClearRecieved(MRH_Uint8 u8_OpCode) noexcept;
    
    /**
     *  Clear all recieved messages.
     */
    
    void ClearAllRecieved() noexcept;
    
    //*************************************************************************************
    // Recieve
    //*************************************************************************************
    
    /**
     *  Recieve the oldest message.
     *
     *  \param v_Data The recieved data.
     *
     *  \return true if data was set, false if not.
     */
    
    bool Recieve(std::vector<MRH_Uint8>& v_Data);
    
    //*************************************************************************************
    // Send
    //*************************************************************************************
    
    /**
     *  Send a message.
     *
     *  \param v_Data The data to send. The reference is consumed.
     */
    
    void Send(std::vector<MRH_Uint8>& v_Data);
    
    /**
     *  Send a message.
     *
     *  \param v_Data The data to send.
     */
    
    void Send(std::vector<MRH_Uint8> const& v_Data);
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if a connection is active.
     *
     *  \return true if a connection is active, false if not.
     */
    
    bool GetConnected() const noexcept;
    
private:
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Update the message stream.
     *
     *  \param p_Instance The class instance to update.
     */
    
    static void Update(MessageStream* p_Instance) noexcept;
    
    /**
     *  Connect to a message stream.
     *
     *  \returns The socket file descriptor on success, -1 on failure.
     */
    
    int Connect() noexcept;
    
    /**
     *  Close a active connection.
     *
     *  \param i_SocketFD The socket to close.
     *
     *  \returns Always -1.
     */
    
    int CloseConnection(int i_SocketFD) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    std::atomic<bool> b_Update;
    
    const std::string s_Channel;
    const std::string s_SocketPath;
    const bool b_KeepAlive;
    std::atomic<bool> b_Connected;
    
    std::mutex c_ReadMutex;
    std::mutex c_WriteMutex;
    
    std::list<std::vector<MRH_Uint8>> l_Read;
    std::list<std::vector<MRH_Uint8>> l_Write;
    
protected:

};

#endif /* MessageStream_h */
