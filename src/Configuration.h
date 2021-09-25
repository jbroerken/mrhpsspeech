/**
 *  Configuration.h
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

#ifndef Configuration_h
#define Configuration_h

// C / C++

// External
#include <MRH_Typedefs.h>

// Project
#include "./Exception.h"


class Configuration
{
public:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************

    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_Configuration Configuration class source.
     */

    Configuration(Configuration const& c_Configuration) = delete;
    
    //*************************************************************************************
    // Singleton
    //*************************************************************************************

    /**
     *  Get the class instance.
     *
     *  \return The class instance.
     */

    static Configuration& Singleton() noexcept;
    
    //*************************************************************************************
    // Load
    //*************************************************************************************

    /**
     *  Load the configuration.
     */
    
    void Load();

    //*************************************************************************************
    // Getters
    //*************************************************************************************

    /**
     *  Get the trigger key phrase to start listening.
     *
     *  \return The trigger key phrase.
     */

    std::string GetTriggerKeyphrase() noexcept;
    
    /**
     *  Get the trigger recognized timout in seconds.
     *
     *  \return The trigger recognized reset timeout.
     */
    
    MRH_Uint32 GetTriggerTimeoutS() noexcept;
    
    /**
     *  Get the path to the trigger recognized sound.
     *
     *  \return The path to the trigger recognized sound.
     */
    
    std::string GetTriggerSoundPath() noexcept;
    
    /**
     *  Get the id of the device used for the microhpone.
     *
     *  \return The id of the device used for the microhpone.
     */
    
    MRH_Uint32 GetPAMicDeviceID() noexcept;
    
    /**
     *  Get the microhpone KHz.
     *
     *  \return The microhpone KHz.
     */
    
    MRH_Uint32 GetPAMicKHz() noexcept;
    
    /**
     *  Get the microhpone frame samples.
     *
     *  \return The microhpone frame samples.
     */
    
    MRH_Uint32 GetPAMicFrameSamples() noexcept;
    
    /**
     *  Get the microhpone recording storage size.
     *
     *  \return The microhpone recording storage size.
     */
    
    MRH_Uint32 GetPAMicRecordingStorageS() noexcept;
    
    /**
     *  Get the id of the device used for the speaker.
     *
     *  \return The id of the device used for the speaker.
     */
    
    MRH_Uint32 GetPASpeakerDeviceID() noexcept;
    
    /**
     *  Get the speaker KHz.
     *
     *  \return The speaker KHz.
     */
    
    MRH_Uint32 GetPASpeakerKHz() noexcept;
    
    /**
     *  Get the speaker frame samples.
     *
     *  \return The speaker frame samples.
     */
    
    MRH_Uint32 GetPASpeakerFrameSamples() noexcept;
    
    /**
     *  Get the directory path to the pocket sphinx model.
     *
     *  \return The sphinx model directory path.
     */
    
    std::string GetSphinxModelDirPath() noexcept;
    
private:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************

    /**
     *  Default constructor.
     */

    Configuration() noexcept;

    /**
     *  Default destructor.
     */

    ~Configuration() noexcept;

    //*************************************************************************************
    // Data
    //**************************************************************************************

    // Trigger
    std::string s_TriggerKeyphrase;
    MRH_Uint32 u32_TriggerTimeoutS;
    std::string s_TriggerSoundPath;
    
    // PA Microphone
    MRH_Uint32 u32_PAMicDeviceID;
    MRH_Uint32 u32_PAMicKHz;
    MRH_Uint32 u32_PAMicFrameSamples;
    MRH_Uint32 u32_PAMicRecordingStorageS;
    
    // PA Speaker
    MRH_Uint32 u32_PASpeakerDeviceID;
    MRH_Uint32 u32_PASpeakerKHz;
    MRH_Uint32 u32_PASpeakerFrameSamples;
    
    // Pocket Sphinx
    std::string s_SphinxModelDirPath;
    
protected:

};

#endif /* Configuration_h */
