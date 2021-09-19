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
#include <libmrhpsb/MRH_PSBLogger.h>

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
        
        // Trigger Key
        TRIGGER_STRING = 4,
        TRIGGER_LS_SIMILARITY = 5,
        TRIGGER_TIMEOUT_S = 6,
        
        // PA Microphone Key
        PA_MICROPHONE_DEVICE_ID = 7,
        PA_MICROPHONE_KHZ,
        PA_MICROPHONE_FRAME_SAMPLES,
        PA_MICROPHONE_SAMPLE_STORAGE_SIZE,
        
        // PA Speaker Key
        PA_SPEAKER_DEVICE_ID,
        PA_SPEAKER_KHZ,
        
        // Pocket Sphinx Key
        POCKET_SPHINX_MODEL_DIR_PATH,
        
        // Bounds
        IDENTIFIER_MAX = POCKET_SPHINX_MODEL_DIR_PATH,

        IDENTIFIER_COUNT = IDENTIFIER_MAX + 1
    };

    const char* p_Identifier[IDENTIFIER_COUNT] =
    {
        // Block Name
        "Trigger",
        "PAMicrophone",
        "PASpeaker",
        "PocketSphinx",
        
        // Trigger Key
        "String",
        "LSSimilarity",
        "TimeoutS",
        
        // PA Microphone Key
        "DeviceID",
        "KHz",
        "FrameSamples",
        "SampleStorageSize",
        
        // PA Speaker Key
        "DeviceID",
        "KHz",
        
        // Sphinx Key
        "ModelDirPath"
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Configuration::Configuration() noexcept : s_TriggerString("Hey Kogoro"),
                                          f32_TriggerLSSimilarity(0.75f),
                                          u32_TriggerTimeoutS(30),
                                          u32_PAMicDeviceID(0),
                                          u32_PAMicKHz(16000),
                                          u32_PAMicFrameSamples(2048),
                                          u32_PAMicSampleStorageSize(100),
                                          u32_PASpeakerDeviceID(0),
                                          u32_PASpeakerKHz(16000),
                                          s_SphinxModelDirPath("/var/mrh/mrhpsspeech/sphinx/")
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
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    
    try
    {
        MRH_BlockFile c_File(MRH_SPEECH_CONFIGURATION_PATH);
        
        for (auto& Block : c_File.l_Block)
        {
            // Internal block, grab as much as possible
            try
            {
                if (Block.GetName().compare(p_Identifier[BLOCK_TRIGGER]) == 0)
                {
                    s_TriggerString = Block.GetValue(p_Identifier[TRIGGER_STRING]);
                    f32_TriggerLSSimilarity = std::stof(Block.GetValue(p_Identifier[TRIGGER_LS_SIMILARITY]));
                    u32_TriggerTimeoutS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[TRIGGER_TIMEOUT_S])));
                }
                else if (Block.GetName().compare(p_Identifier[BLOCK_PA_MICROPHONE]) == 0)
                {
                    u32_PAMicDeviceID = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_MICROPHONE_DEVICE_ID])));
                    u32_PAMicKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_MICROPHONE_KHZ])));
                    u32_PAMicFrameSamples = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_MICROPHONE_FRAME_SAMPLES])));
                    u32_PAMicSampleStorageSize = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_MICROPHONE_SAMPLE_STORAGE_SIZE])));
                }
                else if (Block.GetName().compare(p_Identifier[BLOCK_PA_SPEAKER]) == 0)
                {
                    u32_PASpeakerDeviceID = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_SPEAKER_DEVICE_ID])));
                    u32_PASpeakerKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[PA_SPEAKER_KHZ])));
                }
                else if (Block.GetName().compare(p_Identifier[BLOCK_POCKET_SPHINX]) == 0)
                {
                    s_SphinxModelDirPath = Block.GetValue(p_Identifier[POCKET_SPHINX_MODEL_DIR_PATH]);
                }
            }
            catch (MRH_BFException& e)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Failed to read value: " + e.what2(),
                                               "Configuration.cpp", __LINE__);
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

std::string Configuration::GetTriggerString() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return s_TriggerString;
}

MRH_Sfloat32 Configuration::GetTriggerLSSimilarity() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return f32_TriggerLSSimilarity;
}

MRH_Uint32 Configuration::GetTriggerTimeoutS() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_TriggerTimeoutS;
}

MRH_Uint32 Configuration::GetPAMicDeviceID() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_PAMicDeviceID;
}

MRH_Uint32 Configuration::GetPAMicKHz() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_PAMicKHz;
}

MRH_Uint32 Configuration::GetPAMicFrameSamples() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_PAMicFrameSamples;
}

MRH_Uint32 Configuration::GetPAMicSampleStorageSize() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_PAMicSampleStorageSize;
}

MRH_Uint32 Configuration::GetPASpeakerDeviceID() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_PASpeakerDeviceID;
}

MRH_Uint32 Configuration::GetPASpeakerKHz() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_PASpeakerKHz;
}

std::string Configuration::GetSphinxModelDirPath() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return s_SphinxModelDirPath;
}
