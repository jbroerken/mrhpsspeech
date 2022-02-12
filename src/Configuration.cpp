/**
 *  Configuration.cpp
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
        BLOCK_SERVER = 3,
        
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
        
        // Server Key
        SERVER_SOCKET_PATH,
        SERVER_RECIEVE_TIMEOUT_S,
        
        // Bounds
        IDENTIFIER_MAX = SERVER_RECIEVE_TIMEOUT_S,

        IDENTIFIER_COUNT = IDENTIFIER_MAX + 1
    };

    const char* p_Identifier[IDENTIFIER_COUNT] =
    {
        // Block Name
        "Service",
        "Voice",
        "Google Cloud API",
        "Server",
        
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

Configuration::Configuration() noexcept : u32_ServiceMethodWaitMS(100),
                                          s_VoiceSocketPath("/tmp/mrh/mrhpsspeech_voice.sock"),
                                          u32_VoiceRecordingKHz(16000),
                                          u32_VoicePlaybackKHz(16000),
                                          u32_VoiceRecordingTimeoutS(3),
                                          u8_VoiceAPIProvider(0),
                                          s_GoogleLangCode("en"),
                                          u32_GoogleVoiceGender(0),
                                          s_ServerSocketPath("/tmp/mrh/mrhpsspeech_netserver.sock"),
                                          u32_ServerRecieveTimeoutS(30)
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
            else if (Block.GetName().compare(p_Identifier[BLOCK_SERVER]) == 0)
            {
                s_ServerSocketPath = Block.GetValue(p_Identifier[SERVER_SOCKET_PATH]);
                u32_ServerRecieveTimeoutS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[SERVER_RECIEVE_TIMEOUT_S])));
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

std::string Configuration::GetServerSocketPath() const noexcept
{
    return s_ServerSocketPath;
}

MRH_Uint32 Configuration::GetServerRecieveTimeoutS() const noexcept
{
    return u32_ServerRecieveTimeoutS;
}
