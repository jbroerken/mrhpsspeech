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

// C / C++
#include <cstring>

// External
#include <libmrhbf.h>

// Project
#include "./Configuration.h"

// Pre-defined
#ifndef MRH_SPEECH_CONFIGURATION_PATH
    #define MRH_SPEECH_CONFIGURATION_PATH "/usr/local/etc/mrh/mrhpservice/Speech.conf"
#endif

namespace
{
    enum Identifier
    {
        // Block Name
        BLOCK_SERVICE = 0,
        BLOCK_VOICE = 1,
        BLOCK_GOOGLE_API = 2,
        BLOCK_TEXT_STRING = 3,
        
        // Service Key
        SERVICE_METHOD_WAIT_MS = 4,
        
        // Voice Key
        VOICE_SOCKET_PATH = 5,
        VOICE_RECORDING_KHZ = 6,
        VOICE_PLAYBACK_KHZ = 7,
        VOICE_RECORDING_TIMEOUT_S,
        VOICE_API_PROVIDER,
        
        // Google API Key
        GOOGLE_API_LANGUAGE_CODE,
        GOOGLE_API_VOICE_GENDER,
        
        // Text String Key
        TEXT_STRING_SOCKET_PATH,
        TEXT_STRING_RECIEVE_TIMEOUT_S,
        
        // Bounds
        IDENTIFIER_MAX = TEXT_STRING_RECIEVE_TIMEOUT_S,

        IDENTIFIER_COUNT = IDENTIFIER_MAX + 1
    };

    const char* p_Identifier[IDENTIFIER_COUNT] =
    {
        // Block Name
        "Service",
        "Voice",
        "Google Cloud API",
        "TextString",
        
        // Service Key
        "MethodWaitMS",
        
        // Voice Key
        "SocketPath",
        "RecordingKHz",
        "PlaybackKHz",
        "RecordingTimeoutS",
        "APIProvider",
        
        // Google API Key
        "LanguageCode",
        "VoiceGender",
        
        // Server Key
        "SocketPath",
        "RecieveTimeoutS"
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Configuration::Configuration() : u32_ServiceMethodWaitMS(100),
                                 s_VoiceSocketPath("/tmp/mrh/mrhpsspeech_voice.sock"),
                                 u32_VoiceRecordingKHz(16000),
                                 u32_VoicePlaybackKHz(16000),
                                 u32_VoiceRecordingTimeoutS(3),
                                 u8_VoiceAPIProvider(0),
                                 s_GoogleLangCode("en"),
                                 u32_GoogleVoiceGender(0),
                                 s_TextStringSocketPath("/tmp/mrh/mrhpsspeech_text.sock"),
                                 u32_TextStringRecieveTimeoutS(30)
{
    try
    {
        MRH_BlockFile c_File(MRH_SPEECH_CONFIGURATION_PATH);
        
        for (auto& Block : c_File.l_Block)
        {
            if (Block.GetName().compare(p_Identifier[BLOCK_SERVICE]) == 0)
            {
                u32_ServiceMethodWaitMS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[SERVICE_METHOD_WAIT_MS])));
            }
            else if (Block.GetName().compare(p_Identifier[BLOCK_VOICE]) == 0)
            {
                s_VoiceSocketPath = Block.GetValue(p_Identifier[VOICE_SOCKET_PATH]);
                u32_VoiceRecordingKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_RECORDING_KHZ])));
                u32_VoicePlaybackKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_PLAYBACK_KHZ])));
                u32_VoiceRecordingTimeoutS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_RECORDING_TIMEOUT_S])));
                u8_VoiceAPIProvider = static_cast<MRH_Uint8>(std::stoull(Block.GetValue(p_Identifier[VOICE_API_PROVIDER])));
            }
            else if (Block.GetName().compare(p_Identifier[BLOCK_GOOGLE_API]) == 0)
            {
                s_GoogleLangCode = Block.GetValue(p_Identifier[GOOGLE_API_LANGUAGE_CODE]);
                u32_GoogleVoiceGender = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[GOOGLE_API_VOICE_GENDER])));
            }
            else if (Block.GetName().compare(p_Identifier[BLOCK_TEXT_STRING]) == 0)
            {
                s_TextStringSocketPath = Block.GetValue(p_Identifier[TEXT_STRING_SOCKET_PATH]);
                u32_TextStringRecieveTimeoutS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[TEXT_STRING_RECIEVE_TIMEOUT_S])));
            }
        }
    }
    catch (std::exception& e)
    {
        throw Exception("Could not read configuration: " + std::string(e.what()));
    }
}

Configuration::~Configuration() noexcept
{}

//*************************************************************************************
// Getters
//*************************************************************************************

MRH_Uint32 Configuration::GetServiceMethodWaitMS() const noexcept
{
    return u32_ServiceMethodWaitMS;
}

std::string Configuration::GetVoiceSocketPath() const noexcept
{
    return s_VoiceSocketPath;
}

MRH_Uint32 Configuration::GetVoiceRecordingKHz() const noexcept
{
    return u32_VoiceRecordingKHz;
}

MRH_Uint32 Configuration::GetVoicePlaybackKHz() const noexcept
{
    return u32_VoicePlaybackKHz;
}

MRH_Uint32 Configuration::GetVoiceRecordingTimeoutS() const noexcept
{
    return u32_VoiceRecordingTimeoutS;
}

MRH_Uint8 Configuration::GetVoiceAPIProvider() const noexcept
{
    return u8_VoiceAPIProvider;
}

std::string Configuration::GetGoogleLanguageCode() const noexcept
{
    return s_GoogleLangCode;
}

MRH_Uint32 Configuration::GetGoogleVoiceGender() const noexcept
{
    return u32_GoogleVoiceGender;
}

std::string Configuration::GetTextStringSocketPath() const noexcept
{
    return s_TextStringSocketPath;
}

MRH_Uint32 Configuration::GetTextStringRecieveTimeoutS() const noexcept
{
    return u32_TextStringRecieveTimeoutS;
}
