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

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./CLI.h"

// Pre-defined
#ifndef MRH_SPEECH_CLI_SOCKET_PATH
    #define MRH_SPEECH_CLI_SOCKET_PATH "/tmp/mrhpsspeech_cli.socket"
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

CLI::CLI() : b_Update(true),
             i_ConnectionFD(-1),
             i_ClientFD(-1)
{
    // @NOTE: No error check, might be closed correctly last time
    unlink(MRH_SPEECH_CLI_SOCKET_PATH);
    
    // Create a file descriptor first
    if ((i_ConnectionFD = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        throw Exception("Failed to create CLI connection socket: " +
                        std::string(std::strerror(errno)) +
                        " (" +
                        std::to_string(errno) +
                        ")!");
    }
    
    // Setup socket for connections
    struct sockaddr_un c_Address;
    memset(&c_Address, '\0', sizeof(c_Address));
    c_Address.sun_family = AF_UNIX;
    strcpy(c_Address.sun_path, MRH_SPEECH_CLI_SOCKET_PATH);
    
    if (bind(i_ConnectionFD, (struct sockaddr*)&c_Address, sizeof(c_Address)) < 0 ||
        fcntl(i_ConnectionFD, F_SETFL, fcntl(i_ConnectionFD, F_GETFL, 0) | O_NONBLOCK) < 0 ||
        listen(i_ConnectionFD, 1) < 0) // 1 = Connection backlog, only 1 is allowed
    {
        close(i_ConnectionFD);
        throw Exception("Failed to setup CLI connection socket: " +
                        std::string(std::strerror(errno)) +
                        " (" +
                        std::to_string(errno) +
                        ")!");
    }
    
    // Run
    try
    {
        c_Thread = std::thread(Update, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start CLI update thread: " + std::string(e.what()));
    }
}

CLI::~CLI() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Update
//*************************************************************************************

bool CLI::PollSocket(int i_FD, int i_TimeoutMS) noexcept
{
    struct pollfd s_PollFD;
    
    s_PollFD.fd = i_FD;
    s_PollFD.events = POLLIN;
    
    switch (poll(&s_PollFD, (nfds_t)1, i_TimeoutMS))
    {
        case -1:
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "CLI polling failed: " +
                                                                   std::string(std::strerror(errno)) +
                                                                   " (" +
                                                                   std::to_string(errno) +
                                                                   ")!",
                                           "CLI.cpp", __LINE__);
        case 0:
            return false;
            
        default:
            return true;
    }
}

void CLI::Update(CLI* p_Instance) noexcept
{
    while (p_Instance->b_Update == true)
    {
        // Connection required to be handled?
        if (p_Instance->i_ClientFD > -1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        else if (p_Instance->PollSocket(p_Instance->i_ConnectionFD, 1000) == false)
        {
            continue;
        }
        
        // Found, accept
        struct sockaddr_un c_Adress;
        socklen_t us_ClientLen;
        
        if ((p_Instance->i_ClientFD = accept(p_Instance->i_ConnectionFD, (struct sockaddr*)&c_Adress, &us_ClientLen)) < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "CLI client connection failed: " +
                                                                       std::string(std::strerror(errno)) +
                                                                       " (" +
                                                                       std::to_string(errno) +
                                                                       ")!",
                                               "CLI.cpp", __LINE__);
            }
        }
    }
    
    // Ended, close all
    close(p_Instance->i_ClientFD);
    p_Instance->i_ClientFD = -1;
    
    close(p_Instance->i_ConnectionFD);
    p_Instance->i_ConnectionFD = -1;
}

//*************************************************************************************
// Listen
//*************************************************************************************

void CLI::Listen()
{
    // No client, do nothing
    if (i_ClientFD < 0)
    {
        return;
    }
}

//*************************************************************************************
// Say
//*************************************************************************************

void CLI::Say(OutputStorage& c_OutputStorage)
{
    // No client, do nothing
    if (i_ClientFD < 0)
    {
        return;
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool CLI::IsUsable() noexcept
{
    return i_ClientFD > -1 ? true : false;
}
