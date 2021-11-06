/**
 *  Main.cpp
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
#include <iostream>
#include <clocale>

// External
#include <MRH_Typedefs.h>

// Project
#include "./Revision.h"
#include "./MessageStream/MessageStream.h"

// Pre-defined
#define MRH_CLI_SPEECH_CHANNEL "speech_cli"


//*************************************************************************************
// Read
//*************************************************************************************

static void Read(MessageStream* p_MessageStream, std::atomic<bool>* p_Update, const char* p_Locale) noexcept
{
    std::vector<MRH_Uint8> v_Data;
    
    while (*p_Update == true)
    {
        if (p_MessageStream->GetConnected() == false)
        {
            *p_Update = false;
            break;
        }
        else if (p_MessageStream->Recieve(v_Data) == false)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        if (MessageOpCode::GetOpCode(v_Data) == MessageOpCode::STRING_CS_STRING)
        {
            MessageOpCode::STRING_CS_STRING_DATA c_OpCode(v_Data);
            std::cout << MessageOpCode::STRING_CS_STRING_DATA(v_Data).GetString() << std::endl;
        }
    }
}

//*************************************************************************************
// Write
//*************************************************************************************

static void Write(MessageStream& c_MessageStream, std::atomic<bool>& b_Update, const char* p_Locale) noexcept
{
    std::string s_Input;
    
    while (b_Update == true)
    {
        std::getline(std::cin, s_Input);
    
        if (c_MessageStream.GetConnected() == false || s_Input.compare("quit") == 0)
        {
            b_Update = false;
            break;
        }
        else if (s_Input.size() == 0)
        {
            continue;
        }
        
        MessageOpCode::STRING_CS_STRING_DATA c_OpCode(s_Input);
        c_MessageStream.Send(c_OpCode.v_Data);
        s_Input = "";
    }
}

//*************************************************************************************
// Main
//*************************************************************************************

int main(int argc, char* argv[])
{
    // Greet
    std::cout << "Started CLI Speech, Revision " << VERSION_NUMBER << std::endl;
    
    // Check params
    if (argc < 2)
    {
        std::cout << "[ ERROR ] Missing locale parameter!" << std::endl;
        return EXIT_FAILURE;
    }
    
    // Set locale
    std::setlocale(LC_ALL, argv[1]);
    
    // Create connection
    MessageStream c_MessageStream(MRH_CLI_SPEECH_CHANNEL,
                                  true);
    MRH_Uint64 u64_ConnectionTimeoutS = time(NULL) + 60;
    
    do
    {
        if (c_MessageStream.GetConnected() == true)
        {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    while (time(NULL) < u64_ConnectionTimeoutS);
    
    if (time(NULL) >= u64_ConnectionTimeoutS)
    {
        std::cout << "[ ERROR ] Connection timeout!" << std::endl;
        return EXIT_FAILURE;
    }
    
    // Inform user
    std::cout << "Connected to service." << std::endl;
    std::cout << "Type input and press the enter key to send." << std::endl;
    std::cout << "Type \"quit\" to disconnect from service." << std::endl;
    std::cout << std::endl;
    
    // Connected, R/W start
    std::atomic<bool> b_Update(true);
    std::thread c_Thread = std::thread(Read, &c_MessageStream, &b_Update, argv[1]);
    Write(c_MessageStream, b_Update, argv[1]);
    
    // Clean up and exit
    c_Thread.join();
    return EXIT_SUCCESS;
}
