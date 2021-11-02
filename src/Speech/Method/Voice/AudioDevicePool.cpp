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

// Project
#include "./AudioDevicePool.h"
#include "./AudioDevice/AudioDeviceOpCode.h"
#include "../../../Configuration.h"

// Pre-defined
using namespace AudioDeviceOpCode;


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
    
    // No selected devices in the beginning
    RecordingDevice = l_Device.end();
    PlaybackDevice = l_Device.end();
}

AudioDevicePool::~AudioDevicePool() noexcept
{}

//*************************************************************************************
// Devices
//*************************************************************************************

void AudioDevicePool::StartDevices() noexcept
{
    // Update devices for connections
    UpdateDevices();
    
    // Restart recording
    for (auto& Device : l_Device)
    {
        Device.StartRecording();
    }
}

void AudioDevicePool::StopDevices() noexcept
{
    // Reset used device
    RecordingDevice = l_Device.end();
    PlaybackDevice = l_Device.end();
    
    // Stop all devices
    for (auto& Device : l_Device)
    {
        Device.StopRecording();
    }
}

void AudioDevicePool::UpdateDevices() noexcept
{
    // Check which devices are disconnected first
    // Some disconnected devices might attempt to
    // reconnect
    for (auto It = l_Device.begin(); It != l_Device.end();)
    {
        if (It->GetAvailable() == false)
        {
            It = l_Device.erase(It);
        }
        else
        {
            ++It;
        }
    }
    
    // Can we accept one or more connections?
    std::vector<MRH_Uint8> v_Data;
    
    if (c_DeviceTraffic.Recieve(DEVICE_CONNECT_REQUEST, v_Data) == false)
    {
        return;
    }
    
    // Add first and other new devices
    do
    {
        // Grab info for response (and maybe device)
        DEVICE_CONNECT_REQUEST_DATA c_Request(v_Data);
        
        // Valid data?
        if (c_Request.GetAddress().size() == 0)
        {
            // No answer sendable, ignore
            continue;
        }
        
        // Usable?
        if (c_Request.GetOpCodeVersion() == AUDIO_DEVICE_OPCODE_VERSION)
        {
            // Inform of format
            SERVICE_CONNECT_RESPONSE_OK_DATA c_OK(c_RecordingFormat.first,
                                                  c_RecordingFormat.second,
                                                  c_PlaybackFormat.first,
                                                  c_PlaybackFormat.second);
            
            c_DeviceTraffic.Send(c_Request.GetAddress(),
                                 c_Request.GetPort(),
                                 c_OK.v_Data);
            
            // Add device
            l_Device.emplace_back(c_DeviceTraffic,
                                  c_Request.GetAddress(),
                                  c_Request.GetPort(),
                                  c_Request.GetCanRecord(),
                                  c_Request.GetCanPlay());
        }
        else
        {
            // Not connectable, inform
            OpCodeData c_Fail(SERVICE_CONNECT_RESPONSE_FAIL);
            
            c_DeviceTraffic.Send(c_Request.GetAddress(),
                                 c_Request.GetPort(),
                                 c_Fail.v_Data);
        }
    }
    while (c_DeviceTraffic.Recieve(DEVICE_CONNECT_REQUEST, v_Data) == true);
}

//*************************************************************************************
// Record
//*************************************************************************************

void AudioDevicePool::SelectRecordingDevice()
{
    // Update devices before selecting a recording source
    UpdateDevices();
    
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
        
        // Compare devices
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
    
    // Set next playback device, most likely the recording device
    PlaybackDevice = RecordingDevice;
}

void AudioDevicePool::ResetRecordingDevice() noexcept
{
    // Update to get current devices
    UpdateDevices();
    
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
    
    // Update devices for playback sources
    UpdateDevices();
    
    // Attempt to find a playback device
    if (PlaybackDevice != l_Device.end() && PlaybackDevice->GetCanPlay() == true)
    {
        PlaybackDevice->Play(c_Audio);
    }
    else
    {
        // Select a device capable of playback
        PlaybackDevice = l_Device.end();
        
        for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
        {
            if (It->GetCanPlay() == false)
            {
                continue;
            }
            
            PlaybackDevice = It;
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
    
    // The selected playback device is the only one sending audio
    return PlaybackDevice->GetPlaybackActive();
}
