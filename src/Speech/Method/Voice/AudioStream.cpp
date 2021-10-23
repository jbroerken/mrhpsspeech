/**
 *  AudioStream.cpp
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
#include <limits.h>
#include <cmath>

// External
#include <libmrhpsb/MRH_PSBLogger.h>
#include <libmrhbf.h>

// Project
#include "./AudioStream.h"
#include "./RateConverter.h"
#include "../../../Configuration.h"

// Pre-defined
#ifndef MRH_SPEECH_DEVICE_CONFIG_PATH
    #define MRH_SPEECH_DEVICE_CONFIG_PATH "/usr/local/etc/mrh/mrhpservice/Speech_Device.conf"
#endif

namespace
{
    enum Identifier
    {
        // Block Name
        BLOCK_DEVICE = 0,
        
        // Device Key
        DEVICE_ID = 1,
        DEVICE_NAME = 2,
        DEVICE_ADDRESS = 3,
        DEVICE_PORT = 4,
        
        // Bounds
        IDENTIFIER_MAX = DEVICE_PORT,

        IDENTIFIER_COUNT = IDENTIFIER_MAX + 1
    };

    const char* p_Identifier[IDENTIFIER_COUNT] =
    {
        // Block Name
        "Device",
        
        // Trigger Key
        "ID",
        "Name",
        "Address",
        "Port"
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioStream::AudioStream()
{
    // Set audio format
    c_RecordingFormat.first = Configuration::Singleton().GetRecordingKHz();
    c_RecordingFormat.second = Configuration::Singleton().GetRecordingFrameSamples();
    
    c_PlaybackFormat.first = Configuration::Singleton().GetPlaybackKHz();
    c_PlaybackFormat.second = Configuration::Singleton().GetPlaybackFrameSamples();
    
    // Now read devices
    try
    {
        MRH_BlockFile c_File(MRH_SPEECH_DEVICE_CONFIG_PATH);
        
        for (auto& Block : c_File.l_Block)
        {
            try
            {
                if (Block.GetName().compare(p_Identifier[BLOCK_DEVICE]) == 0)
                {
                    l_Device.emplace_back(static_cast<MRH_Uint32>(std::stoi(Block.GetValue(p_Identifier[DEVICE_ID]))),
                                          Block.GetValue(p_Identifier[DEVICE_NAME]),
                                          Block.GetValue(p_Identifier[DEVICE_ADDRESS]),
                                          std::stoi(Block.GetValue(p_Identifier[DEVICE_PORT])));
                }
            }
            catch (std::exception& e)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Failed to add device! " + std::string(e.what()),
                                               "AudioStream.cpp", __LINE__);
            }
        }
    }
    catch (std::exception& e)
    {
        throw Exception("Could not read device configuration: " + std::string(e.what()));
    }
}

AudioStream::~AudioStream() noexcept
{}

//*************************************************************************************
// Stream
//*************************************************************************************

void AudioStream::StopAll() noexcept
{
    // Stop all for everything
    for (auto& Device : l_Device)
    {
        Device.SetState(AudioDevice::NONE);
    }
}

//*************************************************************************************
// Input
//*************************************************************************************

void AudioStream::Record()
{
    // Record on all which are able to
    for (auto& Device : l_Device)
    {
        if (Device.GetCanRecord() == true)
        {
            Device.SetState(AudioDevice::RECORDING);
        }
        else
        {
            Device.SetState(AudioDevice::NONE);
        }
    }
}

//*************************************************************************************
// Output
//*************************************************************************************

void AudioStream::Playback()
{
    // First, pick the device with the highest last average amp
    std::list<AudioDevice>::iterator PlaybackDevice = l_Device.end();
    
    for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
    {
        if (It->GetCanPlay() == false) // Skip if not possible to playback
        {
            continue;
        }
        else if (PlaybackDevice == l_Device.end() || /* No device set yet */
                 PlaybackDevice->GetRecordingAmplitude() < It->GetRecordingAmplitude()) /* Device set, but quiter */
        {
            PlaybackDevice = It;
        }
    }
    
    // No devices for playback, keep recording
    if (PlaybackDevice == l_Device.end())
    {
        return;
    }
    
    static size_t us_FrameSize = Configuration::Singleton().GetPlaybackFrameSamples();
    
    // Disable recording for all and set the active device for playback
    for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
    {
        // Playback on the active playback device, stop all others
        if (It == PlaybackDevice)
        {
            // Copy audio before sending and pad if wierd order
            It->c_SendMutex.lock();
            
            TODO: Send Frame Samples richtig schreiben, endianess, write opcode func (send)
            
            //It->v_Send = v_Send; // @NOTE: No clearing, expect override
            It->c_SendMutex.unlock();
            
            // Now start playback
            It->SetState(AudioDevice::PLAYBACK);
        }
        else
        {
            It->SetState(AudioDevice::NONE);
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

MonoAudio AudioStream::GetRecordedAudio() noexcept
{
    // We have to check all samples on which to use
    std::list<AudioDevice>::iterator Choice;
    std::vector<MRH_Sint16> v_Buffer;
    size_t us_Sample = 0;
    
    while (true)
    {
        float f32_PeakAmplitude = -1.f;
        Choice = l_Device.end();
        
        // We need to check each device for the highest sample amplitude
        // We then add the loudest one
        for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
        {
            // No samples?
            if (It->GetCanRecord() == false || It->v_Recieved.size() >= us_Sample)
            {
                continue;
            }
            
            // Is this louder than the other one?
            if (f32_PeakAmplitude < It->v_Recieved[us_Sample].first)
            {
                f32_PeakAmplitude = It->v_Recieved[us_Sample].first;
                Choice = It;
            }
        }
        
        // Any choice to add?
        if (Choice == l_Device.end())
        {
            break;
        }
        
        // Found, add
        v_Buffer.insert(v_Buffer.end(),
                        Choice->v_Recieved[us_Sample].second.begin(),
                        Choice->v_Recieved[us_Sample].second.end());
        ++us_Sample;
    }
    
    return MonoAudio(v_Buffer.data(),
                     v_Buffer.size(),
                     c_RecordingFormat.first);
}

bool AudioStream::GetPlayback() noexcept
{
    for (auto& Device : l_Device)
    {
        if (Device.GetState() == AudioDevice::PLAYBACK)
        {
            return true;
        }
    }
    
    return false;
}

bool AudioStream::GetRecording() noexcept
{
    for (auto& Device : l_Device)
    {
        if (Device.GetState() == AudioDevice::RECORDING)
        {
            return true;
        }
    }
    
    return false;
}

//*************************************************************************************
// Setters
//*************************************************************************************

void AudioStream::SetPlaybackAudio(MonoAudio const& c_Audio)
{
    if (GetPlayback() == true)
    {
        throw Exception("Playback in progress!");
    }
    else if (c_Audio.v_Buffer.size() == 0)
    {
        throw Exception("Recieved empty playback buffer!");
    }
    
    // Copy to buffer
    if (c_Audio.u32_KHz != c_RecordingFormat.first)
    {
        // Difference, we need to convert
        try
        {
            v_Send = RateConverter(c_Audio.u32_KHz).Convert(c_Audio.v_Buffer,
                                                            c_Audio.u32_KHz);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                           "AudioStream.cpp", __LINE__);
            return;
        }
    }
    else
    {
        // Same KHz, simply copy
        v_Send = c_Audio.v_Buffer;
    }
    
    // Check if padding is needed
    size_t us_Padding = v_Send.size() % c_RecordingFormat.second;
    
    if (us_Padding != 0)
    {
        v_Send.insert(v_Send.end(), us_Padding, 0);
    }
}
