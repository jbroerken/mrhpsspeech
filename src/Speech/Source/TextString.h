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

#ifndef TextString_h
#define TextString_h

// C / C++
#include <atomic>

// External
#include <libmrhpsb/MRH_Callback.h>

// Project
#include "../../Configuration.h"
#include "../LocalStream.h"
#include "../OutputStorage.h"


class TextString : private LocalStream
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param c_Configuration The configuration to construct with.
     */
    
    TextString(Configuration const& c_Configuration);
    
    /**
     *  Default destructor.
     */
    
    ~TextString() noexcept;
    
    //*************************************************************************************
    // Receive
    //*************************************************************************************
    
    /**
     *  Receive input data from the text string client.
     *
     *  \param u32_StringID The string id to use for the first input.
     *
     *  \return The new string id after retrieving.
     */
    
    MRH_Uint32 Receive(MRH_Uint32 u32_StringID) noexcept;
    
    //*************************************************************************************
    // Send
    //*************************************************************************************
    
    /**
     *  Send output to the text string client.
     *
     *  \param c_OutputStorage The output storage to send from.
     */
    
    void Send(OutputStorage& c_OutputStorage) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if a text string communication is active.
     *
     *  \return true if active, false if not.
     */
    
    bool GetCommunicationActive() const noexcept;
    
private:
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if a message communication is currently in progress.
     *
     *  \param u64_TimestampS The time stamp for the communication.
     *  \param u32_TimeoutS The timeout after which a communication is considered complete.
     *
     *  \return true if communicating, false if not.
     */
    
    static bool GetCommunicationActive(MRH_Uint64 u64_TimestampS, MRH_Uint32 u32_TimeoutS) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Communication Timeout
    std::atomic<MRH_Uint64> u64_RecieveTimestampS;
    MRH_Uint32 u32_RecieveTimeoutS;
    
protected:

};

#endif /* TextString_h */
