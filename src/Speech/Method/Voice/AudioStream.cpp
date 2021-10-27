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
    
    // No selected recording device
    PrimaryRecordingDevice = l_Device.end();
    PrimaryPlaybackDevice = l_Device.end();
}

AudioStream::~AudioStream() noexcept
{}

//*************************************************************************************
// Stream
//*************************************************************************************

void AudioStream::StopAll() noexcept
{
    // Reset primary devices
    PrimaryRecordingDevice = l_Device.end();
    PrimaryPlaybackDevice = l_Device.end();
    
    // Stop all for everything
    for (auto& Device : l_Device)
    {
        Device.Stop();
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
            Device.Record();
        }
        else if (Device.GetState() != AudioDevice::STOPPED)
        {
            Device.Stop();
        }
    }
}

void AudioStream::SelectPrimaryRecordingDevice()
{
    // Reset old
    PrimaryRecordingDevice = l_Device.end();
    
    for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
    {
        // Skip devices not used for recording
        size_t us_AvailableSamples = It->GetAvailableSamples();
        
        if (It->GetCanRecord() == false ||
            It->GetState() == AudioDevice::STOPPED ||
            us_AvailableSamples == 0) /* Gives us no audio, so not important */
        {
            continue;
        }
        
        // Set the recording device to use
        if (PrimaryRecordingDevice == l_Device.end())
        {
            PrimaryRecordingDevice = It;
        }
        else if (PrimaryRecordingDevice->GetAvailableSamples() < us_AvailableSamples)
        {
            PrimaryRecordingDevice->Stop();
            PrimaryRecordingDevice = It;
        }
    }
    
    if (PrimaryRecordingDevice == l_Device.end())
    {
        throw Exception("Failed to select a primary recording device!");
    }
}

//*************************************************************************************
// Output
//*************************************************************************************

void AudioStream::Playback(AudioTrack const& c_Audio)
{
    // Audio?
    if (c_Audio.GetAudioExists() == false)
    {
        throw Exception("No audio to play!");
    }
    
    // Reset old first
    PrimaryPlaybackDevice = l_Device.end();
    
    // Can we use the device for the recording as playback?
    if (PrimaryRecordingDevice != l_Device.end() &&
        PrimaryRecordingDevice->GetCanPlay() == true)
    {
        PrimaryPlaybackDevice = PrimaryRecordingDevice;
        PrimaryPlaybackDevice->Play(c_Audio);
    }
    
    // Stop all other devices (cancel recording) and set
    // a playback device if the primary recording device
    // does not provide playback functionality
    for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
    {
        if (It->GetCanPlay() == false ||
            It->GetState() == AudioDevice::STOPPED)
        {
            continue;
        }
        
        // No device set before? pick as playback
        if (PrimaryPlaybackDevice == l_Device.end())
        {
            It->Play(c_Audio);
            PrimaryPlaybackDevice = It;
        }
        else
        {
            It->Stop();
        }
    }
    
    // No device usable?
    if (PrimaryPlaybackDevice == l_Device.end())
    {
        throw Exception("Failed to select a primary playback device!");
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

AudioTrack const& AudioStream::GetRecordedAudio()
{
    // No device selected?
    if (PrimaryRecordingDevice == l_Device.end())
    {
        throw Exception("No recording device selected!");
    }
    
    return PrimaryRecordingDevice->GetRecordedAudio();
}

bool AudioStream::GetPlayback() noexcept
{
    for (auto& Device : l_Device)
    {
        if (Device.GetState() == AudioDevice::PLAYING)
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

bool AudioStream::GetPrimaryRecordingDeviceSet() noexcept
{
    return PrimaryRecordingDevice != l_Device.end() ? true : false;
}

bool AudioStream::GetPrimaryPlaybackDeviceSet() noexcept
{
    return PrimaryPlaybackDevice != l_Device.end() ? true : false;
}
