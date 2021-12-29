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
     *  Get the service method wait time in milliseconds.
     *
     *  \return The method wait time.
     */
    
    MRH_Uint32 GetServiceMethodWaitMS() noexcept;

    /**
     *  Get the voice trigger key phrase to start listening.
     *
     *  \return The trigger key phrase.
     */

    std::string GetVoiceTriggerKeyphrase() noexcept;
    
    /**
     *  Get the voice trigger recognized timout in seconds.
     *
     *  \return The trigger recognized reset timeout.
     */
    
    MRH_Uint32 GetVoiceTriggerTimeoutS() noexcept;
    
    /**
     *  Get the voice recording KHz.
     *
     *  \return The recording KHz.
     */
    
    MRH_Uint32 GetVoiceRecordingKHz() noexcept;
    
    /**
     *  Get the voice recording sample storage size.
     *
     *  \return The recording sample storage size.
     */
    
    MRH_Uint32 GetVoiceRecordingStorageS() noexcept;
    
    /**
     *  Get the voice playback KHz.
     *
     *  \return The playback KHz.
     */
    
    MRH_Uint32 GetVoicePlaybackKHz() noexcept;
    
    /**
     *  Get the voice google cloud api language code.
     *
     *  \return The google cloud api language code.
     */
    
    std::string GetVoiceGoogleLanguageCode() noexcept;
    
    /**
     *  Get the voice google cloud api voice gender.
     *
     *  \return The google cloud api voice gender.
     */
    
    MRH_Uint32 GetVoiceGoogleVoiceGender() noexcept;
    
    /**
     *  Get the server account mail.
     *
     *  \return The server account mail.
     */
    
    std::string GetServerAccountMail() noexcept;
    
    /**
     *  Get the server account password.
     *
     *  \return The server account password.
     */
    
    std::string GetServerAccountPassword() noexcept;
    
    /**
     *  Get the server device key.
     *
     *  \return The server device key.
     */
    
    std::string GetServerDeviceKey() noexcept;
    
    /**
     *  Get the server device password.
     *
     *  \return The server device password.
     */
    
    std::string GetServerDevicePassword() noexcept;
    
    /**
     *  Get the server connection server address.
     *
     *  \return The server connection server address.
     */
    
    std::string GetServerConnectionAddress() noexcept;
    
    /**
     *  Get the server connection server port.
     *
     *  \return The server connection server port.
     */
    
    int GetServerConnectionPort() noexcept;
    
    /**
     *  Get the server communication server channel.
     *
     *  \return The server communication server channel.
     */
    
    std::string GetServerCommunicationChannel() noexcept;
    
    /**
     *  Get the server timeout in seconds.
     *
     *  \return The server timeout in seconds.
     */
    
    MRH_Uint32 GetServerTimeoutS() noexcept;
    
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

    // Service
    MRH_Uint32 u32_ServiceMethodWaitMS;
    
    // Voice
    std::string s_VoiceTriggerKeyphrase;
    MRH_Uint32 u32_VoiceTriggerTimeoutS;
    MRH_Uint32 u32_VoiceRecordingKHz;
    MRH_Uint32 u32_VoiceRecordingStorageS;
    MRH_Uint32 u32_VoicePlaybackKHz;
    std::string s_VoiceGoogleLangCode;
    MRH_Uint32 u32_VoiceGoogleVoiceGender;
    
    // Server
    std::string s_ServerAccountMail;
    std::string s_ServerAccountPassword;
    std::string s_ServerDeviceKey;
    std::string s_ServerDevicePassword;
    std::string s_ServerConnectionAddress;
    int i_ServerConnectionPort;
    std::string s_ServerCommunicationChannel;
    MRH_Uint32 u32_ServerTimeoutS;
    
    
protected:

};

#endif /* Configuration_h */
