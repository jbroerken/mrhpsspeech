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
#include <libmrhcevs/Event/V1/User/MRH_CESayU_V1.h>
#include <libmrhvt/String/MRH_SpeechString.h>

// Project
#include "../Exception.h"


class OutputStorage
{
public:
    
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
    // Singleton
    //*************************************************************************************

    /**
     *  Get the class instance. This function is thread safe.
     *
     *  \return The class instance.
     */

    static OutputStorage& Singleton() noexcept;
    
    //*************************************************************************************
    // Reset
    //*************************************************************************************
    
    /**
     *  Reset all unfinished output.
     */
    
    void ResetUnfinished() noexcept;
    
    /**
     *  Reset all finished output.
     */
    
    void ResetFinished() noexcept;
    
    //*************************************************************************************
    // Add
    //*************************************************************************************
    
    /**
     *  Add a output event to the storage.
     *
     *  \param p_Event The event to add.
     */
    
    void AddEvent(const MRH_S_STRING_U* p_Event) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if a finished string is available.
     *
     *  \return true if available, false if not.
     */
    
    bool GetFinishedAvailable() noexcept;
    
    /**
     *  Get the next finished UTF-8 string.
     *
     *  \return The next finished UTF-8 string.
     */
    
    std::string GetFinishedString();
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // one Mutex for adding event data, one for finished strings
    std::mutex c_UnfinishedMutex;
    std::mutex c_FinishedMutex;
    
    // Unfinished strings (<String ID, String>
    std::unordered_map<MRH_Uint32, MRH_SpeechString> m_Unfinished;
    
    // Finished output
    std::list<std::string> l_Finished; // UTF-8
    
protected:

};

#endif /* CBAvail_h */
