/**
 *  CLI.cpp
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <cstring>
#include <clocale>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./CLI.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

CLI::CLI() : c_CLIStream("speech_cli")
{}

CLI::~CLI() noexcept
{}

//*************************************************************************************
// Reset
//*************************************************************************************

void CLI::Reset()
{
    // Clear old recieved, might not be wanted anymore
    c_CLIStream.ClearAllRecieved();
}

//*************************************************************************************
// Listen
//*************************************************************************************

void CLI::Listen()
{
    // Wait a bit for data
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // No client or data, do nothing
    if (c_CLIStream.GetConnected() == false)
    {
        return;
    }
    
    // Recieve data
    MessageOpCode::STRING_CS_STRING_DATA c_OpCode("");
    
    // Recieve as many as possible!
    while (c_CLIStream.Recieve(c_OpCode.v_Data) == true)
    {
        // Is this a usable opcode for cli?
        if (c_OpCode.GetOpCode() != MessageOpCode::STRING_CS_STRING)
        {
            return;
        }
        
        // Got everything, send new input
        try
        {
            SendInput(c_OpCode.GetString());
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, e.what(),
                                           "CLI.cpp", __LINE__);
        }
    }
}

//*************************************************************************************
// Say
//*************************************************************************************

void CLI::Say(OutputStorage& c_OutputStorage)
{
    // No client, do nothing
    if (c_CLIStream.GetConnected() == false)
    {
        return;
    }
    
    // Got a string to send?
    if (c_OutputStorage.GetFinishedAvailable() == false)
    {
        return;
    }
    
    // Build opcode
    OutputStorage::String c_String = c_OutputStorage.GetFinishedString();
    MessageOpCode::STRING_CS_STRING_DATA c_OpCode(c_String.s_String);
    
    // Send full and inform
    try
    {
        c_CLIStream.Send(c_OpCode.v_Data);
        
        OutputPerformed(c_String.u32_StringID,
                        c_String.u32_GroupID);
    }
    catch (Exception& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, e.what(),
                                       "CLI.cpp", __LINE__);
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool CLI::IsUsable() noexcept
{
    return c_CLIStream.GetConnected();
}
