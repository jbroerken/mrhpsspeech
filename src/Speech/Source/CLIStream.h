/**
 *  CLIStream.h
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

#ifndef CLIStream_h
#define CLIStream_h

// C / C++

// External
#include <libmrhpsb/MRH_Callback.h>

// Project
#include "./MessageStream/MessageStream.h"
#include "../../Configuration.h"
#include "../OutputStorage.h"


class CLIStream
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    CLIStream();
    
    /**
     *  Default destructor.
     */
    
    ~CLIStream() noexcept;
    
    //*************************************************************************************
    // Retrieve
    //*************************************************************************************
    
    /**
     *  Retrieve recieved data from the local stream.
     *
     *  \param u32_StringID The string id to use for the first input.
     *  \param b_DiscardInput If recieved input should be discarded.
     *
     *  \return The new string id after retrieving.
     */
    
    MRH_Uint32 Retrieve(MRH_Uint32 u32_StringID, bool b_DiscardInput);
    
    //*************************************************************************************
    // Send
    //*************************************************************************************
    
    /**
     *  Add output to send to the local stream.
     *
     *  \param c_OutputStorage The output storage to send from.
     */
    
    void Send(OutputStorage& c_OutputStorage);
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Stream
    MessageStream c_Stream;
    
protected:

};

#endif /* CLIStream_h */
