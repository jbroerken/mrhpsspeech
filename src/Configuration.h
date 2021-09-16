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
#include <mutex>

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
     *  Get the class instance. This function is thread safe.
     *
     *  \return The class instance.
     */

    static Configuration& Singleton() noexcept;
    
    //*************************************************************************************
    // Load
    //*************************************************************************************

    /**
     *  Load the configuration. This function is thread safe.
     */
    
    void Load();

    //*************************************************************************************
    // Getters
    //*************************************************************************************

    /**
     *  Get the trigger string to start listening. This function is thread safe.
     *
     *  \return The trigger string.
     */

    std::string GetTriggerString() noexcept;
    
    /**
     *  Get the required trigger similarity in percent. This function is thread safe.
     *
     *  \return The required trigger similarity.
     */
    
    MRH_Sfloat32 GetTriggerLSSimilarity() noexcept;
    
    /**
     *  Get the trigger recognized timout in seconds. This function is thread safe.
     *
     *  \return The trigger recognized reset timeout.
     */
    
    MRH_Uint32 GetTriggerTimeoutS() noexcept;
    
    /**
     *  Get the id of the device used for the microhpone. This function is thread safe.
     *
     *  \return The id of the device used for the microhpone.
     */
    
    MRH_Uint32 GetPAMicDeviceID() noexcept;
    
    /**
     *  Get the microhpone KHz. This function is thread safe.
     *
     *  \return The microhpone KHz.
     */
    
    MRH_Uint32 GetPAMicKHz() noexcept;
    
    /**
     *  Get the microhpone channels. This function is thread safe.
     *
     *  \return The microhpone channels.
     */
    
    MRH_Uint8 GetPAMicChannels() noexcept;
    
    /**
     *  Get the microhpone frame sample count. This function is thread safe.
     *
     *  \return The microhpone frame sample count.
     */
    
    MRH_Uint32 GetPAMicFrameSamples() noexcept;
    
    /**
     *  Get the microhpone sample count storage size. This function is thread safe.
     *
     *  \return The microhpone sample count storage size.
     */
    
    MRH_Uint32 GetPAMicSampleStorageSize() noexcept;
    
    /**
     *  Get the id of the device used for the speaker. This function is thread safe.
     *
     *  \return The id of the device used for the speaker.
     */
    
    MRH_Uint32 GetPASpeakerDeviceID() noexcept;
    
    /**
     *  Get the speaker KHz. This function is thread safe.
     *
     *  \return The speaker KHz.
     */
    
    MRH_Uint32 GetPASpeakerKHz() noexcept;
    
    /**
     *  Get the speaker channels. This function is thread safe.
     *
     *  \return The speaker channels.
     */
    
    MRH_Uint8 GetPASpeakerChannels() noexcept;
    
    /**
     *  Get the speaker frame sample count. This function is thread safe.
     *
     *  \return The speaker frame sample count.
     */
    
    MRH_Uint32 GetPASpeakerFrameSamples() noexcept;
    
    /**
     *  Get the directory path to the pocket sphinx model. This function is thread
     *  safe.
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

    std::mutex c_Mutex;
    
    // Trigger
    std::string s_TriggerString;
    MRH_Sfloat32 f32_TriggerLSSimilarity;
    MRH_Uint32 u32_TriggerTimeoutS;
    
    // PA Microphone
    MRH_Uint32 u32_PAMicDeviceID;
    MRH_Uint32 u32_PAMicKHz;
    MRH_Uint8 u8_PAMicChannels;
    MRH_Uint32 u32_PAMicFrameSamples;
    MRH_Uint32 u32_PAMicSampleStorageSize;
    
    // PA Speaker
    MRH_Uint32 u32_PASpeakerDeviceID;
    MRH_Uint32 u32_PASpeakerKHz;
    MRH_Uint8 u8_PASpeakerChannels;
    MRH_Uint32 u32_PASpeakerFrameSamples;
    
    // Pocket Sphinx
    std::string s_SphinxModelDirPath;
    
protected:

};

#endif /* Configuration_h */
