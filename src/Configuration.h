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
     *  Get the device connection port.
     *
     *  \return The device connection port.
     */
    
    int GetDeviceConnectionPort() noexcept;
    
    /**
     *  Get the device connection service key.
     *
     *  \return The device connection service key.
     */
    
    std::string GetDeviceConnectionServiceKey() noexcept;
    
    /**
     *  Get the recording KHz.
     *
     *  \return The recording KHz.
     */
    
    MRH_Uint32 GetRecordingKHz() noexcept;
    
    /**
     *  Get the recording frame samples.
     *
     *  \return The recording frame samples.
     */
    
    MRH_Uint32 GetRecordingFrameSamples() noexcept;
    
    /**
     *  Get the recording sample storage size.
     *
     *  \return The recording sample storage size.
     */
    
    MRH_Uint32 GetRecordingStorageS() noexcept;
    
    /**
     *  Get the playback KHz.
     *
     *  \return The playback KHz.
     */
    
    MRH_Uint32 GetPlaybackKHz() noexcept;
    
    /**
     *  Get the playback frame samples.
     *
     *  \return The playback frame samples.
     */
    
    MRH_Uint32 GetPlaybackFrameSamples() noexcept;
    
    /**
     *  Get the google cloud api language code.
     *
     *  \return The google cloud api language code.
     */
    
    std::string GetGoogleLanguageCode() noexcept;
    
    /**
     *  Get the google cloud api voice gender.
     *
     *  \return The google cloud api voice gender.
     */
    
    MRH_Uint32 GetGoogleVoiceGender() noexcept;
    
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
    
    // Device connection
    int i_DeviceConnectionPort;
    std::string s_DeviceConnectionServiceKey;
    
    // Recording
    MRH_Uint32 u32_RecordingKHz;
    MRH_Uint32 u32_RecordingFrameSamples;
    MRH_Uint32 u32_RecordingStorageS;
    
    // Playback
    MRH_Uint32 u32_PlaybackKHz;
    MRH_Uint32 u32_PlaybackFrameSamples;
    
    // Google
    std::string s_GoogleLangCode;
    MRH_Uint32 u32_GoogleVoiceGender;
    
protected:

};

#endif /* Configuration_h */
