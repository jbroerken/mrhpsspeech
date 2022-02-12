/**
 *  NetServer.h
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

#ifndef NetServer_h
#define NetServer_h

// C / C++
#include <atomic>

// External
#include <libmrhpsb/MRH_Callback.h>
#include <libmrhmstream/MRH_MessageStream.h>

// Project
#include "../../Configuration.h"
#include "../OutputStorage.h"


class NetServer
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
    
    NetServer(Configuration const& c_Configuration);
    
    /**
     *  Default destructor.
     */
    
    ~NetServer() noexcept;
    
    //*************************************************************************************
    // Exchange
    //*************************************************************************************
    
    /**
     *  Retceive data from the net server client.
     */
    
    void Receive() noexcept;
    
    /**
     *  Proccess recieved data from the net server client.
     *
     *  \param u32_StringID The string id to use for the first input.
     *  \param c_OutputStorage The output storage to send from.
     *
     *  \return The new string id after retrieving.
     */
    
    MRH_Uint32 Exchange(MRH_Uint32 u32_StringID, OutputStorage& c_OutputStorage) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if a server communication is active.
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
    
    // Stream
    MRH_MessageStream c_Stream;
    
    // Messages
    std::list<std::string> l_Recieved;
    
    // Communication Timeout
    std::atomic<MRH_Uint64> u64_RecieveTimestampS;
    MRH_Uint32 u32_RecieveTimeoutS;
    
protected:

};

#endif /* NetServer_h */
