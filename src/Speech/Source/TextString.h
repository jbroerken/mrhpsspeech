/**
 *  TextString.h
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
