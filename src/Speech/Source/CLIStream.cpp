/**
 *  CLIStream.cpp
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

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./CLIStream.h"
#include "../SpeechEvent.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

CLIStream::CLIStream()
{}

CLIStream::~CLIStream() noexcept
{}

//*************************************************************************************
// Listen
//*************************************************************************************

MRH_Uint32 CLIStream::Retrieve(MRH_Uint32 u32_StringID, bool b_DiscardInput)
{
    // No client or data, do nothing
    if (c_Stream.GetConnected() == false)
    {
        return u32_StringID;
    }
    
    // Recieve data
    MessageOpCode::STRING_CS_STRING_DATA c_OpCode("");
    
    // Recieve as many as possible!
    while (c_Stream.Recieve(c_OpCode.v_Data) == true)
    {
        // Is this a usable opcode for cli?
        if (c_OpCode.GetOpCode() != MessageOpCode::STRING_CS_STRING)
        {
            break;
        }
        
        try
        {
            SpeechEvent::InputRecieved(u32_StringID, c_OpCode.GetString());
            ++u32_StringID;
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                           "CLIStream.cpp", __LINE__);
        }
    }
    
    return u32_StringID;
}

//*************************************************************************************
// Send
//*************************************************************************************

void CLIStream::Send(OutputStorage& c_OutputStorage)
{
    if (c_OutputStorage.GetFinishedAvailable() == false)
    {
        return;
    }
    
    // Nothing sent, send next output
    try
    {
        auto String = c_OutputStorage.GetFinishedString();
        
        // Send over CLI
        c_Stream.Send(MessageOpCode::STRING_CS_STRING_DATA(String.s_String).v_Data);
        
        // Immediatly inform of performed
        SpeechEvent::OutputPerformed(String.u32_StringID,
                                     String.u32_GroupID);
    }
    catch (Exception& e)
    {
        throw;
    }
}
