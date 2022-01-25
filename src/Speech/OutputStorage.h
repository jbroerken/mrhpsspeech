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
#include <libmrhvt/String/MRH_SpeechString.h>

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
    // Reset
    //*************************************************************************************
    
    /**
     *  Reset all unfinished output. This function is thread safe.
     */
    
    void ResetUnfinished() noexcept;
    
    /**
     *  Reset all finished output. This function is thread safe.
     */
    
    void ResetFinished() noexcept;
    
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
     *  Check if a finished string is available. This function is thread safe.
     *
     *  \return true if available, false if not.
     */
    
    bool GetFinishedAvailable() noexcept;
    
    /**
     *  Get the next finished UTF-8 string. This function is thread safe.
     *
     *  \return The next finished UTF-8 string with its string id.
     */
    
    String GetFinishedString();
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // one Mutex for adding event data, one for finished strings
    std::mutex c_UnfinishedMutex;
    std::mutex c_FinishedMutex;
    
    // Unfinished strings (<String ID, <String, Group ID>>
    std::unordered_map<MRH_Uint32, std::pair<MRH_SpeechString, MRH_Uint32>> m_Unfinished;
    
    // Finished output
    std::list<String> l_Finished; // UTF-8
    
protected:

};

#endif /* CBAvail_h */
