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
#include <atomic>

// External
#include <libmrhpsb/MRH_Callback.h>

// Project
#include "./OutputStorage.h"


class Speech : public MRH_Callback
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    Speech() noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~Speech() noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the output storage.
     *
     *  \return The output storage.
     */
    
    OutputStorage& GetOutputStorage() noexcept;
    
    /**
     *  Get the current speech method.
     *
     *  \return The current speech method.
     */
    
    MRH_EvSpeechMethod::Method GetMethod() noexcept;
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum Method
    {
        CLI = 0,
        MRH_SRV = 1,
        VOICE = 2,
        
        METHOD_MAX = VOICE,
        
        METHOD_VOUND = METHOD_MAX + 1
    };
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    OutputStorage c_OutputStorage;
    
    std::atomic<Method> e_Method;
    
protected:

};

#endif /* Speech_h */
