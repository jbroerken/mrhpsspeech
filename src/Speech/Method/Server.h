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
    // Server
    //*************************************************************************************
    
    /**
     *  Perform connection and authentication with a given server.
     *
     *  \param p_Context The context in use.
     *  \param p_Server The server to authenticate for.
     *  \param p_Address The server address.
     *  \param i_Port The server port.
     *
     *  \return true on success, false on failure.
     */
    
    bool ConnectToServer(MRH_Srv_Context* p_Context, MRH_Srv_Server* p_Server, const char* p_Address, int i_Port) noexcept;
    
    /**
     *  Request channel info from a server.
     *
     *  \param p_Server The server to request from.
     *  \param p_Channel The channel identifier to request.
     *  \param p_Address The reference to the address result.
     *  \param i_Port The reference to the port result.
     *
     *  \return true on success, false on failure.
     */

    bool RequestChannel(MRH_Srv_Server* p_Server, const char* p_Channel, char* p_Address, int& i_Port) noexcept;
    
    /**
     *  Update the server communication.
     *
     *  \param p_Instance The server instance to update with.
     */
    
    static void Update(Server* p_Instance) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    std::atomic<bool> b_Run;
    
    std::atomic<bool> b_AppConnected;
    
    std::mutex c_RecieveMutex;
    std::list<std::string> l_Recieve;
    
    std::mutex c_SendMutex;
    std::list<std::string> l_Send;
    
protected:

};

#endif /* Server_h */
