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
        BLOCK_PA_MICROPHONE = 1,
        BLOCK_PA_SPEAKER = 2,
        BLOCK_POCKET_SPHINX = 3,
        BLOCK_GOOGLE_API = 4,
        
        // Trigger Key
        TRIGGER_KEYPHRASE = 5,
        TRIGGER_TIMEOUT_S = 6,
        TRIGGER_SOUND_PATH = 7,
        
        // PA Microphone Key
        PA_MICROPHONE_DEVICE_ID,
        PA_MICROPHONE_KHZ,
        PA_MICROPHONE_FRAME_SAMPLES,
        PA_MICROPHONE_RECORDING_STORAGE_S,
        
        // PA Speaker Key
        PA_SPEAKER_DEVICE_ID,
        PA_SPEAKER_KHZ,
        PA_SPEAKER_FRAME_SAMPLES,
        
        // Pocket Sphinx Key
        POCKET_SPHINX_MODEL_DIR_PATH,
        
        // Google API Key
        GOOGLE_API_LANGUAGE_CODE,
        
        // Bounds
        IDENTIFIER_MAX = GOOGLE_API_LANGUAGE_CODE,

        IDENTIFIER_COUNT = IDENTIFIER_MAX + 1
    };

    const char* p_Identifier[IDENTIFIER_COUNT] =
    {
        // Block Name
        "Trigger",
        "PAMicrophone",
        "PASpeaker",
        "PocketSphinx",
        "GoogleAPI",
        
        // Trigger Key
        "Keyphrase",
        "TimeoutS",
        "SoundFilePath",
        
        // PA Microphone Key
        "DeviceID",
        "KHz",
        "FrameSamples",
        "RecordingStorageS",
        
        // PA Speaker Key
        "DeviceID",
        "KHz",
        "FrameSamples",
        
        // Sphinx Key
        "ModelDirPath",
        
        // Google API Key
        "LanguageCode"
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Configuration::Configuration() noexcept : s_TriggerKeyphrase("Hey Maro"),
                                          u32_TriggerTimeoutS(30),
                                          s_TriggerSoundPath("/var/mrh/mrhpsspeech/triggered.raw"),
                                          u32_PAMicDeviceID(0),
                                          u32_PAMicKHz(16000),
                                          u32_PAMicFrameSamples(2048),
                                          u32_PAMicRecordingStorageS(5),
                                          u32_PASpeakerDeviceID(0),
                                          u32_PASpeakerKHz(16000),
                                          u32_PASpeakerFrameSamples(2048),
                                          s_SphinxModelDirPath("/var/mrh/mrhpsspeech/sphinx/"),
                                          s_GoogleLangCode("en")
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
    try
    {
        MRH_BlockFile c_File(MRH_SPEECH_CONFIGURATION_PATH);
        
        for (auto& Block : c_File.l_Block)
        {
            if (Block.GetName().compare(p_Identifier[BLOCK_TRIGGER]) == 0)
            {
                s_TriggerKeyphrase = Block.GetValue(p_Identifier[TRIGGER_KEYPHRASE]);
                u32_TriggerTimeoutS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[TRIGGER_TIMEOUT_S])));
                s_TriggerSoundPath = Block.GetValue(p_Identifier[TRIGGER_SOUND_PATH]);
            }
            else if (Block.GetName().compare(p_Identifier[BLOCK_PA_MICROPHONE]) == 0)
            {
                u32_PAMicDeviceID = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_MICROPHONE_DEVICE_ID])));
                u32_PAMicKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_MICROPHONE_KHZ])));
                u32_PAMicFrameSamples = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_MICROPHONE_FRAME_SAMPLES])));
                u32_PAMicRecordingStorageS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_MICROPHONE_RECORDING_STORAGE_S])));
            }
            else if (Block.GetName().compare(p_Identifier[BLOCK_PA_SPEAKER]) == 0)
            {
                u32_PASpeakerDeviceID = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_SPEAKER_DEVICE_ID])));
                u32_PASpeakerKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_SPEAKER_KHZ])));
                u32_PASpeakerFrameSamples = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_SPEAKER_FRAME_SAMPLES])));
            }
            else if (Block.GetName().compare(p_Identifier[BLOCK_POCKET_SPHINX]) == 0)
            {
                s_SphinxModelDirPath = Block.GetValue(p_Identifier[POCKET_SPHINX_MODEL_DIR_PATH]);
            }
            else if (Block.GetName().compare(p_Identifier[BLOCK_GOOGLE_API]) == 0)
            {
                s_GoogleLangCode = Block.GetValue(p_Identifier[GOOGLE_API_LANGUAGE_CODE]);
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

MRH_Uint32 Configuration::GetPAMicDeviceID() noexcept
{
    return u32_PAMicDeviceID;
}

MRH_Uint32 Configuration::GetPAMicKHz() noexcept
{
    return u32_PAMicKHz;
}

MRH_Uint32 Configuration::GetPAMicFrameSamples() noexcept
{
    return u32_PAMicFrameSamples;
}

MRH_Uint32 Configuration::GetPAMicRecordingStorageS() noexcept
{
    return u32_PAMicRecordingStorageS;
}

MRH_Uint32 Configuration::GetPASpeakerDeviceID() noexcept
{
    return u32_PASpeakerDeviceID;
}

MRH_Uint32 Configuration::GetPASpeakerKHz() noexcept
{
    return u32_PASpeakerKHz;
}

MRH_Uint32 Configuration::GetPASpeakerFrameSamples() noexcept
{
    return u32_PASpeakerFrameSamples;
}

std::string Configuration::GetSphinxModelDirPath() noexcept
{
    return s_SphinxModelDirPath;
}

std::string Configuration::GetGoogleLanguageCode() noexcept
{
    return s_GoogleLangCode;
}
