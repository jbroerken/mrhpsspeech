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
#if MRH_SPEECH_USE_VOICE > 0
#include "./Source/Voice.h"
#endif
#if MRH_SPEECH_USE_TEXT_STRING > 0
#include "./Source/TextString.h"
#endif
#if MRH_SPEECH_USE_TEXT_STRING <= 0 && MRH_SPEECH_USE_VOICE <= 0
#include "./OutputStorage.h"
#include "../Configuration.h"
#endif


class Speech
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum Method
    {
        AUDIO = 0,
        TEXT_STRING = 1,
        
        METHOD_MAX = TEXT_STRING,
        
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
     *  Check if speech is usable.
     *
     *  \return true if usable, false if not.
     */
    
    bool GetUsable() noexcept;
    
private:
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Update speech methods.
     *
     *  \param p_Instance The speech instance to update.
     *  \param u32_MethodWaitMS The wait time b etween each method update.
     */
    
    static void Update(Speech* p_Instance, MRH_Uint32 u32_MethodWaitMS) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    std::atomic<bool> b_Update;
    std::atomic<Method> e_Method;
    
    OutputStorage c_OutputStorage;
    
#if MRH_SPEECH_USE_VOICE > 0
    Voice c_Voice;
#endif
#if MRH_SPEECH_USE_TEXT_STRING > 0
    TextString c_TextString;
#endif
    
protected:

};

#endif /* Speech_h */
