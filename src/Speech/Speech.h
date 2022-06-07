/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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
