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
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the service method wait time in milliseconds.
     *
     *  \return The method wait time.
     */
    
    MRH_Uint32 GetServiceMethodWaitMS() const noexcept;
    
    /**
     *  Get the full voice socket file path.
     *
     *  \return The full voice socket file path.
     */
    
    std::string GetVoiceSocketPath() const noexcept;

    /**
     *  Get the voice recording KHz.
     *
     *  \return The recording KHz.
     */
    
    MRH_Uint32 GetVoiceRecordingKHz() const noexcept;
    
    /**
     *  Get the voice playback KHz.
     *
     *  \return The playback KHz.
     */
    
    MRH_Uint32 GetVoicePlaybackKHz() const noexcept;
    
    /**
     *  Get the voice recording timeout in seconds.
     *
     *  \return The voice recording timeout in seconds.
     */
    
    MRH_Uint32 GetVoiceRecordingTimeoutS() const noexcept;
    
    /**
     *  Get the voice api provider.
     *
     *  \return The voice api provider.
     */
    
    MRH_Uint8 GetVoiceAPIProvider() const noexcept;
    
    /**
     *  Get the voice google cloud api language code.
     *
     *  \return The google cloud api language code.
     */
    
    std::string GetGoogleLanguageCode() const noexcept;
    
    /**
     *  Get the voice google cloud api voice gender.
     *
     *  \return The google cloud api voice gender.
     */
    
    MRH_Uint32 GetGoogleVoiceGender() const noexcept;
    
    /**
     *  Get the full server socket file path.
     *
     *  \return The full server socket file path.
     */
    
    std::string GetServerSocketPath() const noexcept;
    
    /**
     *  Get the server recieve timeout in seconds.
     *
     *  \return The server recieve timeout in seconds.
     */
    
    MRH_Uint32 GetServerRecieveTimeoutS() const noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //**************************************************************************************

    // Service
    MRH_Uint32 u32_ServiceMethodWaitMS;
    
    // Voice
    std::string s_VoiceSocketPath;
    MRH_Uint32 u32_VoiceRecordingKHz;
    MRH_Uint32 u32_VoicePlaybackKHz;
    MRH_Uint32 u32_VoiceRecordingTimeoutS;
    MRH_Uint8 u8_VoiceAPIProvider;
    
    // Google API
    std::string s_GoogleLangCode;
    MRH_Uint32 u32_GoogleVoiceGender;
    
    // Server
    std::string s_ServerSocketPath;
    MRH_Uint32 u32_ServerRecieveTimeoutS;
    
protected:

};

#endif /* Configuration_h */
