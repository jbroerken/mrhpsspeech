/**
 *  Server.h
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

#ifndef Server_h
#define Server_h

// C / C++
#include <thread>
#include <mutex>
#include <atomic>

// External
#include <libmrhsrv.h>

// Project
#include "../SpeechMethod.h"


class Server : public SpeechMethod
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    Server();
    
    /**
     *  Default destructor.
     */
    
    ~Server() noexcept;
    
    //*************************************************************************************
    // Switch
    //*************************************************************************************
    
    /**
     *  Start speech method.
     */
    
    void Start() override;
    
    /**
     *  Stop speech method.
     */
    
    void Stop() override;
    
    //*************************************************************************************
    // Listen
    //*************************************************************************************
    
    /**
     *  Listen to speech input.
     */
    
    void Listen() override;
    
    //*************************************************************************************
    // Say
    //*************************************************************************************
    
    /**
     *  Perform speech output.
     *
     *  \param c_OutputStorage The output storage to use.
     */
    
    void Say(OutputStorage& c_OutputStorage) override;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if this speech method is usable.
     *
     *  \return true if usable, false if not.
     */
    
    bool IsUsable() noexcept override;
    
private:
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Update the server communication.
     *
     *  \param p_Instance The server instance to update with.
     */
    
    static void Update(Server* p_Instance) noexcept;
    
    //*************************************************************************************
    // Listen
    //*************************************************************************************
    
    /**
     *  Recieve a message from the server.
     *
     *  \param p_Server The server to recieve from.
     *  \param i_State The current client state.
     *
     *  \return The next state the client moved to, -1 on no message.
     */
    
    int RecieveMessage(MRH_Srv_Server* p_Server, int i_State) noexcept;
    
    //*************************************************************************************
    // Say
    //*************************************************************************************
    
    /**
     *  Send a message from the server.
     *
     *  \param p_Server The server to recieve from.
     *  \param i_State The current client state.
     *
     *  \return true if a message was sent, false if not.
     */
    
    bool SendMessage(MRH_Srv_Server* p_Server, int i_State) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Thread
    std::thread c_Thread;
    std::atomic<bool> b_Run;
    
    // Connection
    std::atomic<bool> b_AppPaired;
    
    char p_AccountMail[MRH_SRV_SIZE_ACCOUNT_MAIL];
    char p_AccountPassword[MRH_SRV_SIZE_ACCOUNT_PASSWORD];
    
    char p_DeviceKey[MRH_SRV_SIZE_DEVICE_KEY];
    char p_DevicePassword[MRH_SRV_SIZE_DEVICE_PASSWORD];
    
    char p_ConServerAddress[MRH_SRV_SIZE_SERVER_ADDRESS];
    int i_ConServerPort;
    
    char p_ComServerChannel[MRH_SRV_SIZE_SERVER_CHANNEL];
    char p_ComServerAddress[MRH_SRV_SIZE_SERVER_ADDRESS];
    int i_ComServerPort;
    
    // Messages
    std::mutex c_RecieveMutex;
    std::list<std::string> l_Recieve;
    
    std::mutex c_SendMutex;
    std::list<std::string> l_Send;
    
protected:

};

#endif /* Server_h */
