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
        VOICE_RECORDING_KHZ = 5,
        VOICE_PLAYBACK_KHZ = 6,
        VOICE_RECORDING_TIMEOUT_S = 7,
        VOICE_API_PROVIDER,
        
        // Google API Key
        GOOGLE_API_LANGUAGE_CODE,
        GOOGLE_API_VOICE_GENDER,
        
        // Server Key
        SERVER_ACCOUNT_MAIL,
        SERVER_ACCOUNT_PASSWORD,
        SERVER_DEVICE_KEY,
        SERVER_DEVICE_PASSWORD,
        SERVER_CONNECTION_ADDRESS,
        SERVER_CONNECTION_PORT,
        SERVER_COMMUNICATION_CHANNEL,
        SERVER_TIMEOUT_S,
        SERVER_RETRY_WAIT_S,
        
        // Bounds
        IDENTIFIER_MAX = SERVER_RETRY_WAIT_S,

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
        "RecordingKHz",
        "PlaybackKHz",
        "RecordingTimeoutS",
        "APIProvider",
        
        // Google API Key
        "LanguageCode",
        "VoiceGender",
        
        // Server Key
        "AccountMail",
        "AccountPassword",
        "DeviceKey",
        "DevicePassword",
        "ConnectionAddress",
        "ConnectionPort",
        "CommunicationChannel",
        "TimeoutS",
        "RetryWaitS"
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Configuration::Configuration() noexcept : u32_ServiceMethodWaitMS(100),
                                          u32_VoiceRecordingKHz(16000),
                                          u32_VoicePlaybackKHz(16000),
                                          u32_VoiceRecordingTimeoutS(3),
                                          u8_VoiceAPIProvider(0),
                                          s_GoogleLangCode("en"),
                                          u32_GoogleVoiceGender(0),
                                          s_ServerAccountMail(""),
                                          s_ServerAccountPassword(""),
                                          s_ServerDeviceKey(""),
                                          s_ServerDevicePassword(""),
                                          s_ServerConnectionAddress("127.0.0.1"),
                                          i_ServerConnectionPort(16096),
                                          s_ServerCommunicationChannel("de.mrh.speech"),
                                          u32_ServerTimeoutS(60),
                                          u32_ServerRetryWaitS(300)
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
                s_ServerAccountMail = Block.GetValue(p_Identifier[SERVER_ACCOUNT_MAIL]);
                s_ServerAccountPassword = Block.GetValue(p_Identifier[SERVER_ACCOUNT_PASSWORD]);
                s_ServerDeviceKey = Block.GetValue(p_Identifier[SERVER_DEVICE_KEY]);
                s_ServerDevicePassword = Block.GetValue(p_Identifier[SERVER_DEVICE_PASSWORD]);
                s_ServerConnectionAddress = Block.GetValue(p_Identifier[SERVER_CONNECTION_ADDRESS]);
                i_ServerConnectionPort = std::stoi(Block.GetValue(p_Identifier[SERVER_CONNECTION_PORT]));
                s_ServerCommunicationChannel = Block.GetValue(p_Identifier[SERVER_COMMUNICATION_CHANNEL]);
                u32_ServerTimeoutS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[SERVER_TIMEOUT_S])));
                u32_ServerRetryWaitS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[SERVER_RETRY_WAIT_S])));
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

std::string Configuration::GetServerAccountMail() const noexcept
{
    return s_ServerAccountMail;
}

std::string Configuration::GetServerAccountPassword() const noexcept
{
    return s_ServerAccountPassword;
}

std::string Configuration::GetServerDeviceKey() const noexcept
{
    return s_ServerDeviceKey;
}

std::string Configuration::GetServerDevicePassword() const noexcept
{
    return s_ServerDevicePassword;
}

std::string Configuration::GetServerConnectionAddress() const noexcept
{
    return s_ServerConnectionAddress;
}

int Configuration::GetServerConnectionPort() const noexcept
{
    return i_ServerConnectionPort;
}

std::string Configuration::GetServerCommunicationChannel() const noexcept
{
    return s_ServerCommunicationChannel;
}

MRH_Uint32 Configuration::GetServerTimeoutS() const noexcept
{
    return u32_ServerTimeoutS;
}

MRH_Uint32 Configuration::GetServerRetryWaitS() const noexcept
{
    return u32_ServerRetryWaitS;
}
