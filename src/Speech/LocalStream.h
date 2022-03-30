/**
 *  LocalStream.h
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

#ifndef LocalStream_h
#define LocalStream_h

// C / C++
#include <thread>
#include <mutex>
#include <atomic>
#include <deque>
#include <vector>

// External
#include <libmrhls/MRH_StreamMessage.h>

// Project
#include "../Exception.h"


class LocalStream
{
public:
    
    //*************************************************************************************
    // Destructor
    //*************************************************************************************
    
    /**
     *  Default destructor.
     */
    
    virtual ~LocalStream() noexcept;
    
private:
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Local stream thread update.
     *  
     *  \param p_Instance The local stream instance to update with.
     *  \param s_FilePath The full path to the local stream socket.
     */
    
    static void Update(LocalStream* p_Instance, std::string s_FilePath) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    std::atomic<bool> b_Update;
    
    std::atomic<bool> b_Connected;
    
    std::mutex c_ReceiveMutex;
    std::deque<std::vector<MRH_Uint8>> dq_Received;
    
    std::mutex c_SendMutex;
    std::deque<std::vector<MRH_Uint8>> dq_Send;
    
protected:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *  
     *  \param s_FilePath The full path to the local stream socket.  
     */
    
    LocalStream(std::string const& s_FilePath);
    
    //*************************************************************************************
    // Clear
    //*************************************************************************************
    
    /**
     *  Clear all received messages.
     */
    
    void ClearReceived() noexcept;
    
    /**
     *  Clear all send messages.
     */
    
    void ClearSend() noexcept;
    
    //*************************************************************************************
    // Send
    //*************************************************************************************
    
    /**
     *  Add a message to send.
     *  
     *  \param v_Data The message data. The data is consumed.
     */
    
    void Send(std::vector<MRH_Uint8>& v_Data);
    
    //*************************************************************************************
    // Receive
    //*************************************************************************************
    
    /**
     *  Receive a read message.
     *  
     *  \param v_Data The received message data.
     *  
     *  \return true if a message was received, false if not.
     */
    
    bool Receive(std::vector<MRH_Uint8>& v_Data) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if the local stream is connected.
     *  
     *  \return true if connected, false if not.
     */
    
    bool IsConnected() noexcept;
};

#endif /* LocalStream_h */
