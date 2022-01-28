/**
 *  Speech.h
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

#ifndef Speech_h
#define Speech_h

// C / C++
#include <thread>
#include <atomic>

// External

// Project
#include "./Source/NetServer.h"


class Speech
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    // The order is important! Lower numbers are used before higher ones (std::map)
    enum Method
    {
        CLI = 0,
        MRH_SRV = 1,
        VOICE = 2,
        
        METHOD_MAX = VOICE,
        
        METHOD_COUNT = METHOD_MAX + 1
    };
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param c_Configuration The configuration to use.
     */
    
    Speech(Configuration const& c_Configuration);
    
    /**
     *  Default destructor.
     */
    
    ~Speech() noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the output storage. This function is thread safe.
     *
     *  \return The output storage.
     */
    
    OutputStorage& GetOutputStorage() noexcept;
    
    /**
     *  Get the current speech method. This function is thread safe.
     *
     *  \return The current speech method.
     */
    
    Method GetMethod() noexcept;
    
    /**
     *  Check if a speech method was selected.
     *
     *  \return true if selected, false if not.
     */
    
    bool GetMethodSelected() noexcept;
    
private:
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Update speech methods.
     *
     *  \param p_Instance The speech instance to update.
     *  \param u32_MethodWaitMS The wait time between each method update.
     */
    
    static void Update(Speech* p_Instance, MRH_Uint32 u32_MethodWaitMS) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    std::atomic<bool> b_Update;
    
    OutputStorage c_OutputStorage;
    
    NetServer c_NetServer;
    
    std::map<Method, SpeechMethod*> m_Method;
    std::atomic<Method> e_Method; // Separate for thread safety
    std::atomic<bool> b_MethodSelected;
    
protected:

};

#endif /* Speech_h */
