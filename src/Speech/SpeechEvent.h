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

#ifndef SpeechEvent_h
#define SpeechEvent_h

// C / C++
#include <string>

// External
#include <MRH_Typedefs.h>

// Project
#include "../Exception.h"


namespace SpeechEvent
{
    //*************************************************************************************
    // Listen
    //*************************************************************************************
    
    /**
     *  Create speech input events for a string.
     *
     *  \param u32_StringID The string id for the recieved input.
     *  \param s_String The speech input string.
     */
    
    void InputRecieved(MRH_Uint32 u32_StringID, std::string const& s_String);
    
    //*************************************************************************************
    // Say
    //*************************************************************************************
    
    /**
     *  Create a output performed event.
     *
     *  \param u32_StringID The string id of the performed output.
     *  \param u32_GroupID The event group id to use.
     */
    
    void OutputPerformed(MRH_Uint32 u32_StringID, MRH_Uint32 u32_GroupID);
};


#endif /* SpeechEvent_h */
