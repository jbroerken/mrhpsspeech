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

#ifndef OutputStorage_h
#define OutputStorage_h

// C / C++
#include <mutex>
#include <deque>

// External
#include <libmrhevdata/Version/1/MRH_EvSay_V1.h>

// Project
#include "../Exception.h"


class OutputStorage
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************

    class String
    {
    public:
        
        //*************************************************************************************
        // Constructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         *
         *  \param s_String The output string.
         *  \param u32_StringID The id of the output string.
         *  \param u32_GroupID The id of the output string event group.
         */
        
        String(std::string const& s_String,
               MRH_Uint32 u32_StringID,
               MRH_Uint32 u32_GroupID) noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::string s_String;
        MRH_Uint32 u32_StringID;
        MRH_Uint32 u32_GroupID;
    };
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    OutputStorage() noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~OutputStorage() noexcept;
    
    //*************************************************************************************
    // Clear
    //*************************************************************************************
    
    /**
     *  Clear all current output. This function is thread safe.
     */
    
    void Clear() noexcept;
    
    //*************************************************************************************
    // Add
    //*************************************************************************************
    
    /**
     *  Add a output string to the storage. This function is thread safe.
     *
     *  \param c_String The string data to add.
     *  \param u32_GroupID The group id of the event to add.
     */
    
    void AddString(MRH_EvD_S_String_U const& c_String, MRH_Uint32 u32_GroupID) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if output is available. This function is thread safe.
     *
     *  \return true if available, false if not.
     */
    
    bool GetAvailable() noexcept;
    
    /**
     *  Get the next UTF-8 output string. This function is thread safe.
     *
     *  \return The next UTF-8 output string with its string id.
     */
    
    String GetString();
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::mutex c_Mutex;
    std::deque<String> dq_Output; // UTF-8
    
protected:

};

#endif /* OutputStorage_h */
