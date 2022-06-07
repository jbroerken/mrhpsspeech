/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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
