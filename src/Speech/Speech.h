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
#include <libmrhcevs/Event/MRH_EvSpeechMethod.h>

// Project
#include "./OutputStorage.h"
#include "./SpeechMethod.h"


class Speech
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    Speech();
    
    /**
     *  Default destructor.
     */
    
    ~Speech() noexcept;
    
    //*************************************************************************************
    // Reset
    //*************************************************************************************
    
    /**
     *  Reset speech.
     */

    void Reset() noexcept;
    
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
    
    MRH_EvSpeechMethod::Method GetMethod() noexcept;
    
private:
    
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
    // Update
    //*************************************************************************************
    
    /**
     *  Update speech methods.
     *
     *  \param p_Instance The speech instance to update.
     */
    
    static void Update(Speech* p_Instance) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    std::mutex c_Mutex;
    std::atomic<bool> b_Update;
    
    OutputStorage c_OutputStorage;
    
    std::map<Method, SpeechMethod*> m_Method;
    std::atomic<MRH_EvSpeechMethod::Method> e_Method; // Separate for thread safety
    
protected:

};

#endif /* Speech_h */
