/**
 *  OutputStorage.h
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

#ifndef OutputStorage_h
#define OutputStorage_h

// C / C++
#include <mutex>
#include <unordered_map>
#include <list>

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
