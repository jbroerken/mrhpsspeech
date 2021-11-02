/**
 *  AudioDevice.cpp
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
#include <ctime>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./AudioDevice.h"
#include "./AudioDeviceOpCode.h"
#include "../../../../Configuration.h"

// Pre-defined
#define AUDIO_SAMPLE_SIZE_B sizeof(MRH_Sint16)
#define AUDIO_DEVICE_HEARTBEAT_TIMEOUT_S 60

using namespace AudioDeviceOpCode;


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioDevice::AudioDevice(AudioDeviceTraffic& c_DeviceTraffic,
                         std::string const& s_Address,
                         int i_Port,
                         bool b_CanRecord,
                         bool b_CanPlay) : c_DeviceTraffic(c_DeviceTraffic),
                                           s_Address(s_Address),
                                           i_Port(i_Port),
                                           b_CanPlay(b_CanPlay),
                                           b_CanRecord(b_CanRecord),
                                           us_RecievedSamples(0),
                                           b_PlaybackActive(false),
                                           u64_LastDeviceHeartbeatS(time(NULL)), /* Newly created, assume ok */
                                           c_Recording(Configuration::Singleton().GetRecordingKHz(),
                                                       Configuration::Singleton().GetRecordingFrameSamples(),
                                                       Configuration::Singleton().GetRecordingStorageS(),
                                                       false)
{}

AudioDevice::~AudioDevice() noexcept
{}

//*************************************************************************************
// Record
//*************************************************************************************

void AudioDevice::StartRecording() noexcept
{
    // Update old data first
    GetRecievedData();
    
    // Signal recording start
    OpCodeData c_Data(SERVICE_START_RECORDING);
    c_DeviceTraffic.Send(s_Address, i_Port, c_Data.v_Data);
    
    // Reset the current recording
    c_DeviceTraffic.ClearRecieved(s_Address, i_Port);
    us_RecievedSamples = 0;
}

void AudioDevice::StopRecording() noexcept
{
    // Update old data first
    GetRecievedData();
    
    // Signal recording start
    OpCodeData c_Data(SERVICE_STOP_RECORDING);
    c_DeviceTraffic.Send(s_Address, i_Port, c_Data.v_Data);
}

//*************************************************************************************
// Playback
//*************************************************************************************

void AudioDevice::Play(AudioTrack const& c_Audio)
{
    // Update data for heartbeat etc
    GetRecievedData();
    
    // Check conditions
    size_t us_FrameSamples = Configuration::Singleton().GetPlaybackFrameSamples();
    
    if (b_CanPlay == false)
    {
        throw Exception("This audio device cannot play audio!");
    }
    else if (b_PlaybackActive == true)
    {
        throw Exception("Already playing audio!");
    }
    else if (c_Audio.us_ChunkElements != us_FrameSamples)
    {
        throw Exception("Invalid chunk size for given audio track!");
    }
    
    // Set the audio to write
    std::list<AudioTrack::Chunk> const& l_Chunk = c_Audio.GetChunksConst();
    
    for (auto& Chunk : l_Chunk)
    {
        if (Chunk.GetElementsCurrent() == 0)
        {
            break;
        }
        
        ALL_AUDIO_DATA c_Data(Chunk.GetBufferConst(),
                              Chunk.GetElementsCurrent());
        
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        c_Data.ConvertSamples();
#endif
        
        try
        {
            c_DeviceTraffic.Send(s_Address, i_Port, c_Data.v_Data);
        }
        catch (Exception& e)
        {
            throw;
        }
        
        // Now in playback
        if (b_PlaybackActive == false)
        {
            b_PlaybackActive = true;
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

void AudioDevice::GetRecievedData() noexcept
{
    std::vector<MRH_Uint8> v_Message;
    
    while (c_DeviceTraffic.Recieve(s_Address, i_Port, v_Message) == true)
    {
        // Handle data types
        if (v_Message[0] == AudioDeviceOpCode::ALL_HEARTBEAT)
        {
            u64_LastDeviceHeartbeatS = time(NULL);
            
            // Answer with heartbeat
            try
            {
                // @NOTE: No data, reuse buffer!
                c_DeviceTraffic.Send(s_Address, i_Port, v_Message);
            }
            catch (Exception& e)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                               "AudioDevice.cpp", __LINE__);
            }
        }
        else if (v_Message[0] == AudioDeviceOpCode::ALL_AUDIO)
        {
            ALL_AUDIO_DATA c_Data(v_Message);
            
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            c_Data.ConvertSamples();
#endif
        
            // Add audio
            try
            {
                c_Recording.AddAudio(c_Data.GetAudioBuffer(),
                                     c_Data.GetSampleCount());
            }
            catch (Exception& e)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, e.what(),
                                               "AudioDevice.cpp", __LINE__);
                break;
            }
        }
        else if (v_Message[0] == AudioDeviceOpCode::DEVICE_PLAYBACK_FINISHED)
        {
            b_PlaybackActive = false;
        }
        
        // @NOTE: Throw away unknown or not for this update meant messages
        //        Stuff like connection opcodes etc. are technically valid,
        //        but totaly useless here
    }
}

bool AudioDevice::GetAvailable() noexcept
{
    GetRecievedData();
    return u64_LastDeviceHeartbeatS >= (time(NULL) + AUDIO_DEVICE_HEARTBEAT_TIMEOUT_S) ? true : false;
}

std::string const& AudioDevice::GetAddress() noexcept
{
    return s_Address;
}

int AudioDevice::GetPort() noexcept
{
    return i_Port;
}

AudioTrack const& AudioDevice::GetRecordedAudio() noexcept
{
    GetRecievedData();
    return c_Recording;
}

bool AudioDevice::GetCanRecord() noexcept
{
    return b_CanRecord;
}

bool AudioDevice::GetCanPlay() noexcept
{
    return b_CanPlay;
}

size_t AudioDevice::GetRecievedSamples() noexcept
{
    GetRecievedData();
    return us_RecievedSamples;
}

bool AudioDevice::GetPlaybackActive() noexcept
{
    GetRecievedData();
    return b_PlaybackActive;
}
