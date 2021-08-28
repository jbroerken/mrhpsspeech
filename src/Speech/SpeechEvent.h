/**
 *  SpeechEvent.h
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

#ifndef SpeechEvent_h
#define SpeechEvent_h

// C / C++
#include <mutex>

// External

// Project
#include "../Exception.h"


class SpeechEvent
{
public:
    
    //*************************************************************************************
    // Destructor
    //*************************************************************************************
    
    /**
     *  Default destructor.
     */
    
    virtual ~SpeechEvent() noexcept;
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    struct ListenID
    {
        //*************************************************************************************
        // Constructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         */
        
        ListenID() noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        MRH_Uint32 u32_StringID;
        std::mutex c_Mutex;
    };
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Store same for each class to prevent ID reuse on method switch!
    static ListenID c_ListenID;
    
protected:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    SpeechEvent() noexcept;
    
    //*************************************************************************************
    // Listen
    //*************************************************************************************
    
    /**
     *  Create speech input events for a string.
     *
     *  \param s_String The speech input string.
     */
    
    void SendInput(std::string const& s_String);
    
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
