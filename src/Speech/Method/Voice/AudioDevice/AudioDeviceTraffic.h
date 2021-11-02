/**
 *  AudioDeviceTraffic.h
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

#ifndef AudioDeviceTraffic_h
#define AudioDeviceTraffic_h

// C / C++
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <list>

// External
#include <MRH_Typedefs.h>

// Project
#include "../../../../Exception.h"


class AudioDeviceTraffic
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    AudioDeviceTraffic();
    
    /**
     *  Default destructor.
     */
    
    virtual ~AudioDeviceTraffic() noexcept;
    
    //*************************************************************************************
    // Clear
    //*************************************************************************************
    
    /**
     *  Clear all recieved messages for a device.
     *
     *  \param s_Address The device address.
     *  \param i_Port The device port.
     */
    
    void ClearRecieved(std::string const& s_Address, int i_Port) noexcept;
    
    //*************************************************************************************
    // Recieve
    //*************************************************************************************
    
    /**
     *  Recieve data from the device.
     *
     *  \param s_Address The address of the device to recieve from.
     *  \param i_Port The port of the device to recieve from.
     *  \param v_Data The recieved data.
     *
     *  \return true if data was set, false if not.
     */
    
    bool Recieve(std::string const& s_Address, int i_Port, std::vector<MRH_Uint8>& v_Data);
    
    /**
     *  Recieve data from the device.
     *
     *  \param u8_OpCode The expected opcode recieved.
     *  \param v_Data The recieved data.
     *
     *  \return true if data was set, false if not.
     */
    
    bool Recieve(MRH_Uint8 u8_OpCode, std::vector<MRH_Uint8>& v_Data);
    
    //*************************************************************************************
    // Send
    //*************************************************************************************
    
    /**
     *  Send data to the device.
     *
     *  \param s_Address The address of the device to send to.
     *  \param i_Port The port of the device to send to.
     *  \param v_Data The data to send. The reference is consumed.
     */
    
    void Send(std::string const& s_Address, int i_Port, std::vector<MRH_Uint8>& v_Data);
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    struct Message
    {
    public:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::string s_Address;
        int i_Port;
        std::vector<MRH_Uint8> v_Payload;
    };
    
    //*************************************************************************************
    // Update
    //*************************************************************************************

    /**
     *  Update the device traffic.
     *
     *  \param p_Instance The class instance to use.
     */
    
    static void Update(AudioDeviceTraffic* p_Instance) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if a message is fully read.
     *
     *  \param v_Data The full message data.
     *
     *  \return true if fully read, false if not.
     */
    
    bool GetMessageFinished(std::vector<MRH_Uint8> const& v_Data) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    std::atomic<bool> b_Update;
    
    //lsquic_engine_t* p_Engine;
    
    std::string s_ServiceKey;
    
    std::mutex c_ReadMutex;
    std::mutex c_WriteMutex;
    
    // <Device ID, Message list<Bytes>>
    // @NOTE: Bytes include opcode and data length, min length is 5!
    std::list<Message> l_Read;
    std::list<Message> l_Write;
    
protected:

};

#endif /* AudioDeviceTraffic_h */
