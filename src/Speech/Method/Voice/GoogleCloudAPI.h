/**
 *  GoogleCloudAPI.h
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

#ifndef GoogleCloudAPI_h
#define GoogleCloudAPI_h

// C / C++
#include <thread>
#include <atomic>

// External

// Project
#include "../../SpeechMethod.h"
#include "./Audio/HandleAudio.h"


class GoogleCloudAPI : public SpeechMethod,
                       private HandleAudio
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    GoogleCloudAPI();
    
    /**
     *  Default destructor.
     */
    
    ~GoogleCloudAPI() noexcept;
    
    //*************************************************************************************
    // Listen
    //*************************************************************************************
    
    /**
     *  Listen to speech input.
     */
    
    void Listen() override;
    
    //*************************************************************************************
    // Say
    //*************************************************************************************
    
    /**
     *  Perform speech output.
     *
     *  \param c_OutputStorage The output storage to use.
     */
    
    void Say(OutputStorage& c_OutputStorage) override;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if this speech method is usable.
     *
     *  \return true if usable, false if not.
     */
    
    bool IsUsable() noexcept override;
    
private:
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Update Voice by Google Cloud API method.
     *
     *  \param p_Instance The Voice by Google Cloud API instance to update.
     */
    
    static void Update(GoogleCloudAPI* p_Instance) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    std::atomic<bool> b_Update;
    
protected:

};

#endif /* GoogleCloudAPI_h */
