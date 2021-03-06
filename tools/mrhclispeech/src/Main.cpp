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

// C / C++
#include <cstring>
#include <iostream>
#include <clocale>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <list>

// External
#include <libmrhls.h>

// Project
#include "./Revision.h"


//*************************************************************************************
// Data
//*************************************************************************************

namespace
{
    std::atomic<bool> b_Update(true);
    std::mutex c_Mutex;
    MRH_LS_M_String_Data c_Input;
}

//*************************************************************************************
// Stream
//*************************************************************************************

static void UpdateStream(const char* p_FilePath)
{
    // Create connection
    MRH_LocalStream* p_Stream = MRH_LS_Open(p_FilePath, -1);
    
    if (p_Stream == NULL)
    {
        std::cout << "[ ERROR ] Failed to create local stream: " << MRH_ERR_GetLocalStreamErrorString() << std::endl;
        b_Update = false;
        return;
    }
    else if (MRH_LS_Connect(p_Stream) < 0)
    {
        std::cout << "[ ERROR ] Failed to connect local stream: " << MRH_ERR_GetLocalStreamErrorString() << std::endl;
        b_Update = false;
        return;
    }
    
    // Update send / recieve
    MRH_Uint8 p_Send[MRH_STREAM_MESSAGE_TOTAL_SIZE];
    MRH_Uint8 p_Recieve[MRH_STREAM_MESSAGE_TOTAL_SIZE];
    MRH_Uint32 u32_SendSize = 0;
    MRH_Uint32 u32_RecieveSize = 0;
    
    while (b_Update == true)
    {
        /**
         *  Connnection
         */
        
        if (MRH_LS_GetConnected(p_Stream) < 0)
        {
            std::cout << "[ ERROR ] Local stream connection lost!" << std::endl;
            b_Update = false;
            break;
        }
        
        /**
         *  Send
         */
        
        if (MRH_LS_GetWriteMessageSet(p_Stream) < 0)
        {
            std::lock_guard<std::mutex> c_Guard(c_Mutex);
            
            if (strnlen(c_Input.p_String, MRH_STREAM_MESSAGE_BUFFER_SIZE) > 0)
            {
                if (MRH_LS_MessageToBuffer(p_Send, &u32_SendSize, MRH_LS_M_STRING, &c_Input) < 0)
                {
                    std::cout << "[ ERROR ] Failed to set local stream write message!" << std::endl;
                }
                else
                {
                    memset(c_Input.p_String, '\0', MRH_STREAM_MESSAGE_BUFFER_SIZE);
                    MRH_LS_Write(p_Stream, p_Send, u32_SendSize); // Ignore error, but set message
                }
            }
        }
        else if (MRH_LS_Write(p_Stream, p_Send, u32_SendSize) < 0)
        {
            std::cout << "[ ERROR ] Failed to write local stream!" << std::endl;
            b_Update = false;
        }
        
        /**
         *  Receive
         */
        
        switch (MRH_LS_Read(p_Stream, 100, p_Recieve, &u32_RecieveSize))
        {
            case -1:
            {
                std::cout << "[ ERROR ] Failed to read local stream!" << std::endl;
                b_Update = false;
            }
                
            case 0:
            {
                MRH_LS_M_String_Data c_Output;
                
                if (MRH_LS_BufferToMessage(&c_Output, p_Recieve, u32_RecieveSize) < 0)
                {
                    std::cout << "[ ERROR ] Failed to receive local stream message!" << std::endl;
                }
                else
                {
                    std::cout << c_Output.p_String << std::endl;
                }
                break;
            }
                
            default:
            {
                break;
            }
        }
    }
    
    // Clean Up
    MRH_LS_Close(p_Stream);
}

//*************************************************************************************
// CLI
//*************************************************************************************

static void ReadCLI() noexcept
{
    std::string s_Input;
    
    while (b_Update == true)
    {
        std::getline(std::cin, s_Input);
    
        if (s_Input.compare("quit") == 0)
        {
            b_Update = false;
            break;
        }
        else if (s_Input.size() == 0)
        {
            continue;
        }
        
        std::lock_guard<std::mutex> c_Guard(c_Mutex);
        
        strncpy(c_Input.p_String, s_Input.c_str(), MRH_STREAM_MESSAGE_BUFFER_SIZE);
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
        std::cout << "[ ERROR ] Missing socket parameter!" << std::endl;
        return EXIT_FAILURE;
    }
    else if (argc < 3)
    {
        std::cout << "[ ERROR ] Missing locale parameter!" << std::endl;
        return EXIT_FAILURE;
    }
    
    // Set locale
    std::setlocale(LC_ALL, argv[2]);
    
    // Inform user
    std::cout << "Connected to service." << std::endl;
    std::cout << "Type input and press the enter key to send." << std::endl;
    std::cout << "Type \"quit\" to disconnect from service." << std::endl;
    std::cout << std::endl;
    
    // Connected, R/W start
    std::thread c_Thread = std::thread(ReadCLI);
    UpdateStream(argv[1]);
    
    // Clean up and exit
    b_Update = false;
    c_Thread.join();
    
    return EXIT_SUCCESS;
}
