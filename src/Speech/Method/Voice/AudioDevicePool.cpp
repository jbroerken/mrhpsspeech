/**
 *  AudioDevicePool.cpp
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

// External
#include <libmrhpsb/MRH_PSBLogger.h>
#include <libmrhbf.h>

// Project
#include "./AudioDevicePool.h"
#include "../../../Configuration.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioDevicePool::AudioDevicePool()
{
    // Set audio format
    c_RecordingFormat.first = Configuration::Singleton().GetRecordingKHz();
    c_RecordingFormat.second = Configuration::Singleton().GetRecordingFrameSamples();
    
    c_PlaybackFormat.first = Configuration::Singleton().GetPlaybackKHz();
    c_PlaybackFormat.second = Configuration::Singleton().GetPlaybackFrameSamples();
    
    // Now read devices
    /*
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
    */
    
    // No selected devices in the beginning
    RecordingDevice = l_Device.end();
    
}

AudioDevicePool::~AudioDevicePool() noexcept
{}

//*************************************************************************************
// Devices
//*************************************************************************************

void AudioDevicePool::StartDevices() noexcept
{
    // Reset recording device
    RecordingDevice = l_Device.end();
    
    // @TODO: Load list and connect
}

void AudioDevicePool::StopDevices() noexcept
{
    // Reset recording device
    RecordingDevice = l_Device.end();
    
    // Disconnect all devices
    l_Device.clear();
}

void AudioDevicePool::UpdateDevices() noexcept
{
    
}

//*************************************************************************************
// Record
//*************************************************************************************

void AudioDevicePool::SelectRecordingDevice()
{
    // Reset old
    RecordingDevice = l_Device.end();
    
    for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
    {
        // Skip devices not used for recording
        size_t us_RecievedSamples = It->GetRecievedSamples();
        
        if (It->GetCanRecord() == false || us_RecievedSamples == 0)
        {
            continue;
        }
        
        // Set the recording device to use
        if (RecordingDevice == l_Device.end() ||
            RecordingDevice->GetRecievedSamples() < us_RecievedSamples)
        {
            RecordingDevice = It;
        }
    }
    
    if (RecordingDevice == l_Device.end())
    {
        throw Exception("Failed to select a recording device!");
    }
    
    // Selected, stop all others
    for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
    {
        if (It->GetCanRecord() == true && It != RecordingDevice)
        {
            It->StopRecording();
        }
    }
}

void AudioDevicePool::ResetRecordingDevice() noexcept
{
    // Reset old
    RecordingDevice = l_Device.end();
    
    // Allow recording for all
    for (auto& Device : l_Device)
    {
        Device.StartRecording();
    }
}

//*************************************************************************************
// Playback
//*************************************************************************************

void AudioDevicePool::Playback(AudioTrack const& c_Audio)
{
    // Audio?
    if (c_Audio.GetAudioExists() == false)
    {
        throw Exception("No audio to play!");
    }
    
    // Can we use the device for the recording as playback?
    // This would make sure the device closest to the user
    // is the playback source
    if (RecordingDevice != l_Device.end() &&
        RecordingDevice->GetCanPlay() == true)
    {
        RecordingDevice->Play(c_Audio);
    }
    else
    {
        // Select a device capable of playback
        for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
        {
            if (It->GetCanPlay() == false)
            {
                continue;
            }
            
            It->Play(c_Audio);
            return;
        }
    }
    
    // No device usable?
    throw Exception("Failed to select a playback device!");
}

//*************************************************************************************
// Getters
//*************************************************************************************

AudioTrack const& AudioDevicePool::GetRecordedAudio()
{
    // No device selected?
    if (RecordingDevice == l_Device.end())
    {
        throw Exception("No recording device selected!");
    }
    
    return RecordingDevice->GetRecordedAudio();
}

bool AudioDevicePool::GetRecordingDeviceSelected() noexcept
{
    return RecordingDevice != l_Device.end() ? true : false;
}

bool AudioDevicePool::GetPlaybackActive() noexcept
{
    if (PlaybackDevice == l_Device.end())
    {
        return false;
    }
    
    return true;
}
