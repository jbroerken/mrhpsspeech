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
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <limits.h>
#include <cstring>
#include <cmath>
#include <ctime>
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #include <byteswap.h>
#endif

// External
#include <libmrhpsb/MRH_PSBLogger.h>
#include <libmrhbf.h>

// Project
#include "./AudioDevicePool.h"
#include "../../../Configuration.h"

// Pre-defined
#define AUDIO_DEVICE_INVALID -1
#define AUDIO_DEVICE_CONNECTION_BACKLOG 255 // Uint8, Device Count Max


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
    i_RecordingDevice = AUDIO_DEVICE_INVALID;
    i_PlaybackDevice = AUDIO_DEVICE_INVALID;
}

AudioDevicePool::~AudioDevicePool() noexcept
{}

//*************************************************************************************
// Devices
//*************************************************************************************

void AudioDevicePool::StartDevices() noexcept
{
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    
    // Reset recording device
    i_RecordingDevice = AUDIO_DEVICE_INVALID;
    i_PlaybackDevice = AUDIO_DEVICE_INVALID;
    
    // Create a file descriptor first
    if ((i_SocketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to create audio device pool connection socket: " +
                                           std::string(std::strerror(errno)) +
                                           " (" +
                                           std::to_string(errno) +
                                           ")!",
                     "AudioDevicePool.cpp", __LINE__);
    }
    
    // Setup socket for connections
    struct sockaddr_in c_Address;
    memset(&c_Address, '\0', sizeof(c_Address));
    
    c_Address.sin_family = AF_INET;
    c_Address.sin_addr.s_addr = INADDR_ANY;
    c_Address.sin_port = htons(Configuration::Singleton().GetDeviceConnectionPort());
    
    if (bind(i_SocketFD, (struct sockaddr*)&c_Address, sizeof(c_Address)) < 0 ||
        fcntl(i_SocketFD, F_SETFL, fcntl(i_SocketFD, F_GETFL, 0) | O_NONBLOCK) < 0 || /* Non blocking for update */
        listen(i_SocketFD, AUDIO_DEVICE_CONNECTION_BACKLOG) < 0)
    {
        close(i_SocketFD);
        i_SocketFD = AUDIO_DEVICE_SOCKET_DISCONNECTED;
        
        c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to setup audio device pool connection socket: " +
                                           std::string(std::strerror(errno)) +
                                           " (" +
                                           std::to_string(errno) +
                                           ")!",
                     "AudioDevicePool.cpp", __LINE__);
    }
    
    // Update devices for connections
    UpdateDevices();
}

void AudioDevicePool::StopDevices() noexcept
{
    // Reset recording device
    i_RecordingDevice = AUDIO_DEVICE_INVALID;
    i_PlaybackDevice = AUDIO_DEVICE_INVALID;
    
    // Disconnect all devices
    m_Device.clear();
    
    // Disable connection socket
    if (i_SocketFD != AUDIO_DEVICE_SOCKET_DISCONNECTED)
    {
        close(i_SocketFD);
        i_SocketFD = AUDIO_DEVICE_SOCKET_DISCONNECTED;
    }
}

void AudioDevicePool::UpdateDevices() noexcept
{
    // Check which devices are disconnected first
    // Some disconnected devices might attempt to
    // reconnect
    for (auto It = m_Device.begin(); It != m_Device.end();)
    {
        if (It->second.GetConnected() == false)
        {
            It = m_Device.erase(It);
        }
        else
        {
            ++It;
        }
    }
    
    // Now connect waiting devices
    while (i_SocketFD != AUDIO_DEVICE_SOCKET_DISCONNECTED)
    {
        struct sockaddr_in c_Adress;
        socklen_t us_ClientLen;
        int i_ClientFD;
        
        if ((i_ClientFD = accept(i_SocketFD, (struct sockaddr*)&c_Adress, &us_ClientLen)) < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Audio device connection failed: " +
                                                                       std::string(std::strerror(errno)) +
                                                                       " (" +
                                                                       std::to_string(errno) +
                                                                       ")!",
                                               "AudioDevicePool.cpp", __LINE__);
            }
            
            // Nothing left to do
            return;
        }
        else
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Audio device connection accepted.",
                                           "AudioDevicePool.cpp", __LINE__);
        }
        
        // Now perform authentication
        // @TODO: Auth, Read Request Opcode and Send Response Opcode
        //        Then add device to map with recieved device id
        //        -> Disconnect on duplicate ID or no capabilities
    }
}

//*************************************************************************************
// Record
//*************************************************************************************

void AudioDevicePool::SelectRecordingDevice()
{
    // Update devices before selecting a recording source
    UpdateDevices();
    
    // Reset old
    i_RecordingDevice = AUDIO_DEVICE_INVALID;
    
    for (auto It = m_Device.begin(); It != m_Device.end(); ++It)
    {
        // Skip devices not used for recording
        size_t us_RecievedSamples = It->second.GetRecievedSamples();
        
        if (It->second.GetCanRecord() == false || us_RecievedSamples == 0)
        {
            continue;
        }
        
        // Compare devices
        auto Device = m_Device.find(i_RecordingDevice);
        
        if (Device == m_Device.end() ||
            Device->second.GetRecievedSamples() < us_RecievedSamples)
        {
            i_RecordingDevice = It->first;
        }
    }
    
    if (i_RecordingDevice == AUDIO_DEVICE_INVALID)
    {
        throw Exception("Failed to select a recording device!");
    }
    
    // Selected, stop all others
    for (auto It = m_Device.begin(); It != m_Device.end(); ++It)
    {
        if (It->second.GetCanRecord() == true && It->first != i_RecordingDevice)
        {
            It->second.StopRecording();
        }
    }
    
    // Set next playback device, most likely the recording device
    i_PlaybackDevice = i_RecordingDevice;
}

void AudioDevicePool::ResetRecordingDevice() noexcept
{
    // Update to get current devices
    UpdateDevices();
    
    // Reset old
    i_RecordingDevice = AUDIO_DEVICE_INVALID;
    
    // Allow recording for all
    for (auto& Device : m_Device)
    {
        Device.second.StartRecording();
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
    auto Device = m_Device.find(i_PlaybackDevice);
    
    if (Device != m_Device.end() && Device->second.GetCanPlay() == true)
    {
        Device->second.Play(c_Audio);
    }
    else
    {
        // Select a device capable of playback
        for (auto It = m_Device.begin(); It != m_Device.end(); ++It)
        {
            if (It->second.GetCanPlay() == false)
            {
                continue;
            }
            
            i_PlaybackDevice = It->first;
            It->second.Play(c_Audio);
            
            return;
        }
    }
    
    // No device usable?
    i_PlaybackDevice = AUDIO_DEVICE_INVALID;
    
    throw Exception("Failed to select a playback device!");
}

//*************************************************************************************
// Getters
//*************************************************************************************

AudioTrack const& AudioDevicePool::GetRecordedAudio()
{
    // No device selected?
    auto Device = m_Device.find(i_RecordingDevice);
    
    if (Device == m_Device.end())
    {
        throw Exception("No recording device selected!");
    }
    
    return Device->second.GetRecordedAudio();
}

bool AudioDevicePool::GetRecordingDeviceSelected() noexcept
{
    return m_Device.find(i_RecordingDevice) != m_Device.end() ? true : false;
}

bool AudioDevicePool::GetPlaybackActive() noexcept
{
    auto Device = m_Device.find(i_PlaybackDevice);
    
    if (Device == m_Device.end())
    {
        return false;
    }
    
    // The selected playback device is the only one sending audio
    return Device->second.GetSendingActive();
}
