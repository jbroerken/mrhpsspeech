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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <clocale>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>

// External
#include <MRH_Typedefs.h>

// Project
#include "./Revision.h"

// Pre-defined
#ifndef MRH_SPEECH_CLI_SOCKET_PATH
    #define MRH_SPEECH_CLI_SOCKET_PATH "/tmp/mrhpsspeech_cli.socket"
#endif
#define MRH_SPEECH_CLI_READ_SIZE sizeof(MRH_Uint32)
#define MRH_SPEECH_CLI_WRITE_SIZE sizeof(MRH_Uint32)


//*************************************************************************************
// Data
//*************************************************************************************

namespace
{
    std::atomic<bool> b_Update(true);
    std::atomic<int> i_FD(-1);
}

//*************************************************************************************
// Read
//*************************************************************************************

static void Read() noexcept
{
    std::vector<MRH_Uint8> v_Read;
    ssize_t ss_Read;
    MRH_Uint32 u32_Read = 0;
    struct pollfd s_PollFD;
    bool b_PollResult;
    
    // Prepare polling
    s_PollFD.fd = i_FD;
    s_PollFD.events = POLLIN;
    
    while (b_Update == true)
    {
        // Poll for data
        switch (poll(&s_PollFD, (nfds_t)1, 100))
        {
            case -1:
                std::cout << "[ ERROR ] Failed to poll socket!" << std::endl;
                b_Update = false;
                return;
            case 0:
                b_PollResult = false;
                break;
                
            default:
                b_PollResult = true;
                break;
        }
        
        if (b_PollResult == false)
        {
            continue;
        }
        
        // Prepare read buffer
        MRH_Uint32 u32_Required;
        
        if (u32_Read < MRH_SPEECH_CLI_READ_SIZE)
        {
            u32_Required = MRH_SPEECH_CLI_READ_SIZE;
        }
        else
        {
            u32_Required = (*((MRH_Uint32*)(v_Read.data()))) + MRH_SPEECH_CLI_READ_SIZE;
        }
        
        v_Read.resize(u32_Required, '\0');
        
        // Read to vector
        do
        {
            if ((ss_Read = read(i_FD, &(v_Read[u32_Read]), u32_Required - u32_Read)) < 0)
            {
                if (errno != EAGAIN)
                {
                    std::cout << "[ ERROR ] Failed to read message!" << std::endl;
                    b_Update = false;
                    return;
                }
            }
            else if (ss_Read == 0)
            {
                std::cout << "[ ERROR ] 0 bytes read but polling says data available!" << std::endl;
                b_Update = false;
                break;
            }
            else if (ss_Read > 0)
            {
                u32_Read += ss_Read;
            }
        }
        while (b_Update == true && ss_Read > 0 && u32_Read < u32_Required);
        
        // Print
        if (u32_Read > MRH_SPEECH_CLI_READ_SIZE && u32_Read == u32_Required)
        {
            MRH_Uint8* p_Start = &v_Read[MRH_SPEECH_CLI_READ_SIZE];
            MRH_Uint8* p_End = &v_Read[u32_Required];
            u32_Read = 0;
            
            std::cout << std::string(p_Start, p_End) << std::endl;
        }
    }
}

//*************************************************************************************
// Write
//*************************************************************************************

static void Write() noexcept
{
    std::vector<MRH_Uint8> v_Write;
    std::string s_Input("");
    ssize_t ss_Write;
    MRH_Uint32 u32_Written = 0;
    
    while (b_Update == true)
    {
        // Grab input
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
        
        // Prepare
        v_Write.resize(s_Input.size() + MRH_SPEECH_CLI_WRITE_SIZE, '\0');
        
        *((MRH_Uint32*)(&v_Write[0])) = static_cast<MRH_Uint32>(s_Input.size());
        memcpy((MRH_Uint8*)&v_Write[MRH_SPEECH_CLI_WRITE_SIZE], s_Input.data(), s_Input.size());
        u32_Written = 0;
        
        // Send to service
        do
        {
            if ((ss_Write = write(i_FD, &(v_Write[u32_Written]), v_Write.size() - u32_Written)) < 0)
            {
                if (errno != EAGAIN)
                {
                    std::cout << "[ ERROR ] Failed to write message!" << std::endl;
                    b_Update = false;
                    return;
                }
            }
            else if (ss_Write > 0)
            {
                u32_Written += ss_Write;
            }
        }
        while (b_Update == true && ss_Write >= 0 && u32_Written < v_Write.size());
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
    
    // Connect to socket
    if ((i_FD = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "[ ERROR ] Failed to create socket!" << std::endl;
        return EXIT_FAILURE;
    }
    
    struct sockaddr_un c_Address;
    socklen_t us_AddressLength;
    
    us_AddressLength = sizeof(c_Address);
    memset(&c_Address, 0, us_AddressLength);
    c_Address.sun_family = AF_UNIX;
    strcpy(c_Address.sun_path, MRH_SPEECH_CLI_SOCKET_PATH);
    
    if (connect(i_FD, (struct sockaddr*)&c_Address, us_AddressLength) < 0)
    {
        std::cout << "[ ERROR ] Connection failed for " << MRH_SPEECH_CLI_SOCKET_PATH << "!" << std::endl;
        close(i_FD);
        return EXIT_FAILURE;
    }
    
    // Inform user
    std::cout << "Connected to service." << std::endl;
    std::cout << "Type input and press the enter key to send." << std::endl;
    std::cout << "Type \"quit\" to disconnect from service." << std::endl;
    std::cout << std::endl;
    
    // Connected, start read thread
    std::thread c_Thread = std::thread(Read);
    
    // Write until terminate
    Write();
    
    // Clean up and exit
    c_Thread.join();
    close(i_FD);
    return EXIT_SUCCESS;
}
