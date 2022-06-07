/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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

    Configuration();

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
     *  Get the full text string socket file path.
     *
     *  \return The full text string socket file path.
     */
    
    std::string GetTextStringSocketPath() const noexcept;
    
    /**
     *  Get the text string recieve timeout in seconds.
     *
     *  \return The text string recieve timeout in seconds.
     */
    
    MRH_Uint32 GetTextStringRecieveTimeoutS() const noexcept;
    
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
    std::string s_TextStringSocketPath;
    MRH_Uint32 u32_TextStringRecieveTimeoutS;
    
protected:

};

#endif /* Configuration_h */
