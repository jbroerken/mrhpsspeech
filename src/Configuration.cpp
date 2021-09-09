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
    #define MRH_SPEECH_CONFIGURATION_PATH "/etc/mrhpservice/Speech.conf"
#endif

namespace
{
    enum Identifier
    {
        // Block Name
        BLOCK_TRIGGER = 0,
        BLOCK_VOICE = 1,
        
        // Trigger Key
        TRIGGER_STRING = 2,
        TRIGGER_LS_SIMILARITY = 3,
        TRIGGER_TIMEOUT_S = 4,
        
        // Voice Key
        VOICE_LISTEN_DEVICE_ID = 5,
        VOICE_LISTEN_KHZ = 6,
        VOICE_LISTEN_CHANNELS = 7,
        VOICE_LISTEN_SAMPLES,
        VOICE_LISTEN_STREAM_LENGTH_S,
        
        // Bounds
        IDENTIFIER_MAX = VOICE_LISTEN_STREAM_LENGTH_S,

        IDENTIFIER_COUNT = IDENTIFIER_MAX + 1
    };

    const char* p_Identifier[IDENTIFIER_COUNT] =
    {
        // Block Name
        "Trigger",
        "Voice",
        
        // Trigger Key
        "String",
        "LSSimilarity",
        "TimeoutS",
        
        // Voice Key
        "ListenDeviceID",
        "ListenKHz",
        "ListenChannels",
        "ListenSamples",
        "ListenStreamLengthS",
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Configuration::Configuration() noexcept : s_TriggerString("Susu"),
                                          f32_TriggerLSSimilarity(0.75f),
                                          u32_TriggerTimeoutS(30),
                                          u32_ListenDeviceID(0),
                                          u32_ListenKHz(44100),
                                          u8_ListenChannels(2),
                                          u32_ListenSamples(4096),
                                          u32_ListenStreamLengthS(10)
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
                else if (Block.GetName().compare(p_Identifier[BLOCK_VOICE]) == 0)
                {
                    u32_ListenDeviceID = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_LISTEN_DEVICE_ID])));
                    u32_ListenKHz = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_LISTEN_KHZ])));
                    u8_ListenChannels = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_LISTEN_CHANNELS])));
                    u32_ListenSamples = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_LISTEN_SAMPLES])));
                    u32_ListenStreamLengthS = static_cast<MRH_Uint32>(std::stoull(Block.GetValue(p_Identifier[VOICE_LISTEN_STREAM_LENGTH_S])));
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

MRH_Uint32 Configuration::GetListenDeviceID() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_ListenDeviceID;
}

MRH_Uint32 Configuration::GetListenKHz() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_ListenKHz;
}

MRH_Uint8 Configuration::GetListenChannels() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u8_ListenChannels;
}

MRH_Uint32 Configuration::GetListenSamples() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_ListenSamples;
}

MRH_Uint32 Configuration::GetListenStreamLengthS() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    return u32_ListenStreamLengthS;
}
