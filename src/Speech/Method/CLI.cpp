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

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./CLI.h"

// Pre-defined
#ifndef MRH_SPEECH_CLI_SOCKET_PATH
    #define MRH_SPEECH_CLI_SOCKET_PATH "/tmp/mrhpsspeech_cli.socket"
#endif
#define MRH_SPEECH_CLI_READ_SIZE sizeof(MRH_Uint32)
#define MRH_SPEECH_CLI_WRITE_SIZE sizeof(MRH_Uint32)


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

CLI::CLI() : b_Update(true),
             i_ConnectionFD(-1),
             i_ClientFD(-1),
             b_CanConnect(false), // Only allow connections on Resume() call
             v_Read(0),
             u32_Read(0),
             v_Write(0),
             u32_Written(0)
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
    
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "CLI socket now available at " +
                                                        std::string(MRH_SPEECH_CLI_SOCKET_PATH) +
                                                        ".",
                                   "CLI.cpp", __LINE__);
}

CLI::~CLI() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Useage
//*************************************************************************************

void CLI::Resume()
{
    b_CanConnect = true;
}

void CLI::Pause()
{
    DisconnectClient();
    b_CanConnect = false;
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
            DisconnectClient(); // Disconnect for safety
        case 0:
            return false;
            
        default:
            return true;
    }
}

void CLI::DisconnectClient() noexcept
{
    // Reset RW first before socket, so that no new connection
    // happens during reset
    u32_Read = 0;
    u32_Written = static_cast<MRH_Uint32>(v_Write.size());
    
    // Close client
    close(i_ClientFD);
    i_ClientFD = -1;
    
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Disconnected CLI client.",
                                   "CLI.cpp", __LINE__);
}

void CLI::Update(CLI* p_Instance) noexcept
{
    while (p_Instance->b_Update == true)
    {
        // Connection required to be handled?
        if (p_Instance->b_CanConnect == false || p_Instance->i_ClientFD > -1)
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
        else
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "CLI client connection accepted.",
                                           "CLI.cpp", __LINE__);
        }
    }
    
    // Ended, close all
    p_Instance->DisconnectClient();
    
    close(p_Instance->i_ConnectionFD);
    p_Instance->i_ConnectionFD = -1;
}

//*************************************************************************************
// Listen
//*************************************************************************************

void CLI::Listen()
{
    // No client or data, do nothing
    if (i_ClientFD < 0)
    {
        return;
    }
    else if (PollSocket(i_ClientFD, 100) == false)
    {
        return;
    }
    
    // Set the required size to be read
    MRH_Uint32 u32_Required;
    
    if (u32_Read < MRH_SPEECH_CLI_READ_SIZE)
    {
        u32_Required = MRH_SPEECH_CLI_READ_SIZE;
    }
    else
    {
        u32_Required = (*((MRH_Uint32*)(v_Read.data()))) + MRH_SPEECH_CLI_READ_SIZE;
    }
    
    // Resize vector to string size
    v_Read.resize(u32_Required, '\0');
    
    // Read to vector
    ssize_t ss_Read;
    
    do
    {
        if ((ss_Read = read(i_ClientFD, &(v_Read[u32_Read]), u32_Required - u32_Read)) < 0)
        {
            if (errno != EAGAIN)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "CLI read failed: " +
                                                                       std::string(std::strerror(errno)) +
                                                                       " (" +
                                                                       std::to_string(errno) +
                                                                       ")!",
                                               "CLI.cpp", __LINE__);
                DisconnectClient();
                return;
            }
        }
        else if (ss_Read == 0)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "0 bytes read but polling succeeded!",
                                           "CLI.cpp", __LINE__);
            DisconnectClient();
            return;
        }
        else if (ss_Read > 0)
        {
            u32_Read += ss_Read;
        }
    }
    while (ss_Read > 0 && u32_Read < u32_Required);
    
    // Read message? Add events if so
    if (u32_Read > MRH_SPEECH_CLI_READ_SIZE && u32_Read == u32_Required)
    {
        MRH_Uint8* p_Start = &v_Read[MRH_SPEECH_CLI_READ_SIZE];
        MRH_Uint8* p_End = &v_Read[u32_Required];
        u32_Read = 0;
        
        try
        {
            SendInput(std::string(p_Start, p_End));
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
    if (i_ClientFD < 0)
    {
        return;
    }
    
    // Grab something new to write?
    if (v_Write.size() == u32_Written)
    {
        if (c_OutputStorage.GetFinishedAvailable() == false)
        {
            return;
        }
        
        OutputStorage::String c_String = c_OutputStorage.GetFinishedString();
        
        v_Write.resize(c_String.s_String.size() + MRH_SPEECH_CLI_WRITE_SIZE, '\0');
        
        *((MRH_Uint32*)(&v_Write[0])) = static_cast<MRH_Uint32>(c_String.s_String.size());
        memcpy((MRH_Uint8*)&v_Write[MRH_SPEECH_CLI_WRITE_SIZE], c_String.s_String.data(), c_String.s_String.size());
        u32_Written = 0;
        
        u32_SayStringID = c_String.u32_StringID;
        u32_SayGroupID = c_String.u32_GroupID;
    }
    
    // Write as much as possible
    ssize_t ss_Write;
    
    do
    {
        if ((ss_Write = write(i_ClientFD, &(v_Write[u32_Written]), v_Write.size() - u32_Written)) < 0)
        {
            if (errno != EAGAIN)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "CLI write failed: " +
                                                                       std::string(std::strerror(errno)) +
                                                                       " (" +
                                                                       std::to_string(errno) +
                                                                       ")!",
                                               "CLI.cpp", __LINE__);
                DisconnectClient();
                return;
            }
        }
        else if (ss_Write > 0)
        {
            u32_Written += ss_Write;
        }
    }
    while (ss_Write >= 0 && u32_Written < v_Write.size());
    
    // Fully written?
    if (u32_Written == v_Write.size())
    {
        try
        {
            OutputPerformed(u32_SayStringID, u32_SayGroupID);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, e.what(),
                                           "CLI.cpp", __LINE__);
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool CLI::IsUsable() noexcept
{
    return i_ClientFD > -1 ? true : false;
}
