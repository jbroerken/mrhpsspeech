/**
 *  CLI.h
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

#ifndef CLI_h
#define CLI_h

// C / C++
#include <thread>
#include <atomic>
#include <vector>

// External

// Project
#include "../SpeechMethod.h"


class CLI : public SpeechMethod
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    CLI();
    
    /**
     *  Default destructor.
     */
    
    ~CLI() noexcept;
    
    //*************************************************************************************
    // Useage
    //*************************************************************************************
    
    /**
     *  Resume speech method.
     */
    
    void Resume() override;
    
    /**
     *  Pause speech method.
     */
    
    void Pause() override;
    
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
     *  Poll a socket.
     *
     *  \param i_FD The socket file descriptor to poll.
     *  \param i_TimeoutMS The poll timeout in milliseconds.
     *
     *  \return true if data can be read, false if not.
     */
    
    bool PollSocket(int i_FD, int i_TimeoutMS) noexcept;
    
    /**
     *  Disconnect a client.
     */
    
    void DisconnectClient() noexcept;
    
    /**
     *  Update CLI method.
     *
     *  \param p_Instance The CLI instance to update.
     */
    
    static void Update(CLI* p_Instance) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    std::atomic<bool> b_Update;
    
    int i_ConnectionFD;
    std::atomic<int> i_ClientFD;
    std::atomic<bool> b_CanConnect;
    
    std::vector<MRH_Uint8> v_Read;
    MRH_Uint32 u32_Read;
    
    std::vector<MRH_Uint8> v_Write;
    MRH_Uint32 u32_Written;
    MRH_Uint32 u32_SayStringID;
    MRH_Uint32 u32_SayGroupID;
    
protected:

};

#endif /* CLI_h */
