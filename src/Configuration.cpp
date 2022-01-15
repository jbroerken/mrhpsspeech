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
        BLOCK_SERVER = 2,
        
        // Service Key
        SERVICE_METHOD_WAIT_MS = 3,
        
        // Voice Key
        VOICE_TRIGGER_KEYPHRASE = 4,
        VOICE_TRIGGER_TIMEOUT_S = 5,
        VOICE_RECORDING_KHZ = 6,
        VOICE_RECORDING_STORAGE_S = 7,
        VOICE_PLAYBACK_KHZ,
        VOICE_GOOGLE_API_LANGUAGE_CODE,
        VOICE_GOOGLE_API_VOICE_GENDER,
        
        // Server Key
        SERVER_ACCOUNT_MAIL,
        SERVER_ACCOUNT_PASSWORD,
        SERVER_DEVICE_KEY,
        SERVER_DEVICE_PASSWORD,
        SERVER_CONNECTION_ADDRESS,
        SERVER_CONNECTION_PORT,
        SERVER_COMMUNICATION_CHANNEL,
        SERVER_TIMEOUT_S,
        
        // Bounds
        IDENTIFIER_MAX = SERVER_TIMEOUT_S,

        IDENTIFIER_COUNT = IDENTIFIER_MAX + 1
    };

    const char* p_Identifier[IDENTIFIER_COUNT] =
    {
        // Block Name
        "Service",
        "Voice",
        "Server",
        
        // Service Key
        "MethodWaitMS",
        
        // Voice Key
        "Keyphrase",
        "TimeoutS",
        "RecordingKHz",
        "RecordingStorageS",
        "PlaybackKHz",
        "GoogleLanguageCode",
        "GoogleVoiceGender",
        
        // Server Key
        "AccountMail",
        "AccountPassword",
        "DeviceKey",
        "DevicePassword",
        "ConnectionAddress",
        "ConnectionPort",
        "CommunicationChannel",
        "TimeoutS"
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Configuration::Configuration() noexcept : u32_ServiceMethodWaitMS(100),
                                          s_VoiceTriggerKeyphrase("Hey Mamao"),
                                          u32_VoiceTriggerTimeoutS(30),
                                          u32_VoiceRecordingKHz(16000),
                                          u32_VoiceRecordingStorageS(5),
                                          u32_VoicePlaybackKHz(16000),
                                          s_VoiceGoogleLangCode("en"),
                                          u32_VoiceGoogleVoiceGender(0),
                                          s_ServerAccountMail(""),
                                          s_ServerAccountPassword(""),
                                          s_ServerDeviceKey(""),
                                          s_ServerDevicePassword(""),
                                          s_ServerConnectionAddress("127.0.0.1"),
                                          i_ServerConnectionPort(16096),
                                          s_ServerCommunicationChannel("de.mrh.speech"),
                                          u32_ServerTimeoutS(60)
{}

Configuration::~Configuration() noexcept
{}

//*************************************************************************************
// Singleton
//*************************************************************************************

Configuration& Configuration::Singleton() noexcept
{
    static Configuration c_Configuration;
    return c_Configuration;
}

//*************************************************************************************
// Load
//*************************************************************************************

void Configuration::Load()
{
    static bool b_IsLoaded = false;
    
    if (b_IsLoaded == true)
    {
        throw Exception("Configuration already loaded! Reloading blocked to prevent issues.");
    }
    
    b_IsLoaded = true;
    
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
                s_VoiceTriggerKeyphrase = Block.GetValue(p_Identifier[VOICE_TRIGGER_KEYPHRASE]);
                u32_VoiceTriggerTimeoutS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_TRIGGER_TIMEOUT_S])));
                u32_VoiceRecordingKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_RECORDING_KHZ])));
                u32_VoiceRecordingStorageS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_RECORDING_STORAGE_S])));
                u32_VoicePlaybackKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_PLAYBACK_KHZ])));
                s_VoiceGoogleLangCode = Block.GetValue(p_Identifier[VOICE_GOOGLE_API_LANGUAGE_CODE]);
                u32_VoiceGoogleVoiceGender = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_GOOGLE_API_VOICE_GENDER])));
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
            }
        }
    }
    catch (std::exception& e)
    {
        throw Exception("Could not read configuration: " + std::string(e.what()));
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

MRH_Uint32 Configuration::GetServiceMethodWaitMS() noexcept
{
    return u32_ServiceMethodWaitMS;
}

std::string Configuration::GetVoiceTriggerKeyphrase() noexcept
{
    return s_VoiceTriggerKeyphrase;
}

MRH_Uint32 Configuration::GetVoiceTriggerTimeoutS() noexcept
{
    return u32_VoiceTriggerTimeoutS;
}

MRH_Uint32 Configuration::GetVoiceRecordingKHz() noexcept
{
    return u32_VoiceRecordingKHz;
}

MRH_Uint32 Configuration::GetVoiceRecordingStorageS() noexcept
{
    return u32_VoiceRecordingStorageS;
}

MRH_Uint32 Configuration::GetVoicePlaybackKHz() noexcept
{
    return u32_VoicePlaybackKHz;
}

std::string Configuration::GetVoiceGoogleLanguageCode() noexcept
{
    return s_VoiceGoogleLangCode;
}

MRH_Uint32 Configuration::GetVoiceGoogleVoiceGender() noexcept
{
    return u32_VoiceGoogleVoiceGender;
}

std::string Configuration::GetServerAccountMail() noexcept
{
    return s_ServerAccountMail;
}

std::string Configuration::GetServerAccountPassword() noexcept
{
    return s_ServerAccountPassword;
}

std::string Configuration::GetServerDeviceKey() noexcept
{
    return s_ServerDeviceKey;
}

std::string Configuration::GetServerDevicePassword() noexcept
{
    return s_ServerDevicePassword;
}

std::string Configuration::GetServerConnectionAddress() noexcept
{
    return s_ServerConnectionAddress;
}

int Configuration::GetServerConnectionPort() noexcept
{
    return i_ServerConnectionPort;
}

std::string Configuration::GetServerCommunicationChannel() noexcept
{
    return s_ServerCommunicationChannel;
}

MRH_Uint32 Configuration::GetServerTimeoutS() noexcept
{
    return u32_ServerTimeoutS;
}
