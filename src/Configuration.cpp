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
        BLOCK_TRIGGER = 0,
        BLOCK_RECORDING = 1,
        BLOCK_PLAYBACK = 2,
        BLOCK_POCKET_SPHINX = 3,
        BLOCK_GOOGLE_API = 4,
        
        // Trigger Key
        TRIGGER_KEYPHRASE = 5,
        TRIGGER_TIMEOUT_S = 6,
        
        // Recording Key
        RECORDING_KHZ = 7,
        RECORDING_FRAME_SAMPLES,
        RECORDING_STORAGE_S,
        
        // Playback Key
        PLAYBACK_KHZ,
        PLAYBACK_FRAME_SAMPLES,
        
        // Google API Key
        GOOGLE_API_LANGUAGE_CODE,
        GOOGLE_API_VOICE_GENDER,
        
        // Bounds
        IDENTIFIER_MAX = GOOGLE_API_VOICE_GENDER,

        IDENTIFIER_COUNT = IDENTIFIER_MAX + 1
    };

    const char* p_Identifier[IDENTIFIER_COUNT] =
    {
        // Block Name
        "Trigger",
        "Recording",
        "Playback",
        "PocketSphinx",
        "GoogleAPI",
        
        // Trigger Key
        "Keyphrase",
        "TimeoutS",
        
        // Recording Key
        "KHz",
        "FrameSamples",
        "RecordingStorageS",
        
        // Playback Key
        "KHz",
        "FrameSamples",
        
        // Google API Key
        "LanguageCode",
        "VoiceGender"
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Configuration::Configuration() noexcept : s_TriggerKeyphrase("Hey Mamao"),
                                          u32_TriggerTimeoutS(30),
                                          u32_RecordingKHz(16000),
                                          u32_RecordingFrameSamples(2048),
                                          u32_RecordingStorageS(5),
                                          u32_PlaybackKHz(16000),
                                          u32_PlaybackFrameSamples(2048),
                                          s_GoogleLangCode("en"),
                                          u32_GoogleVoiceGender(0)
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
            if (Block.GetName().compare(p_Identifier[BLOCK_TRIGGER]) == 0)
            {
                s_TriggerKeyphrase = Block.GetValue(p_Identifier[TRIGGER_KEYPHRASE]);
                u32_TriggerTimeoutS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[TRIGGER_TIMEOUT_S])));
            }
            else if (Block.GetName().compare(p_Identifier[BLOCK_RECORDING]) == 0)
            {
                u32_RecordingKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[RECORDING_KHZ])));
                u32_RecordingFrameSamples = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[RECORDING_FRAME_SAMPLES])));
                u32_RecordingStorageS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[RECORDING_STORAGE_S])));
            }
            else if (Block.GetName().compare(p_Identifier[BLOCK_PLAYBACK]) == 0)
            {
                u32_PlaybackKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PLAYBACK_KHZ])));
                u32_PlaybackFrameSamples = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PLAYBACK_FRAME_SAMPLES])));
            }
            else if (Block.GetName().compare(p_Identifier[BLOCK_GOOGLE_API]) == 0)
            {
                s_GoogleLangCode = Block.GetValue(p_Identifier[GOOGLE_API_LANGUAGE_CODE]);
                u32_GoogleVoiceGender = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[GOOGLE_API_VOICE_GENDER])));
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

std::string Configuration::GetTriggerKeyphrase() noexcept
{
    return s_TriggerKeyphrase;
}

MRH_Uint32 Configuration::GetTriggerTimeoutS() noexcept
{
    return u32_TriggerTimeoutS;
}

std::string Configuration::GetTriggerSoundPath() noexcept
{
    return s_TriggerSoundPath;
}

MRH_Uint32 Configuration::GetRecordingKHz() noexcept
{
    return u32_RecordingKHz;
}

MRH_Uint32 Configuration::GetRecordingFrameSamples() noexcept
{
    return u32_RecordingFrameSamples;
}

MRH_Uint32 Configuration::GetRecordingStorageS() noexcept
{
    return u32_RecordingStorageS;
}

MRH_Uint32 Configuration::GetPlaybackKHz() noexcept
{
    return u32_PlaybackKHz;
}

MRH_Uint32 Configuration::GetPlaybackFrameSamples() noexcept
{
    return u32_PlaybackFrameSamples;
}

std::string Configuration::GetGoogleLanguageCode() noexcept
{
    return s_GoogleLangCode;
}

MRH_Uint32 Configuration::GetGoogleVoiceGender() noexcept
{
    return u32_GoogleVoiceGender;
}
