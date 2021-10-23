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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <limits.h>
#include <cstring>
#include <cmath>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./AudioDevice.h"
#include "./AudioDeviceOpCode.h"
#include "../../../Configuration.h"

// Pre-defined
#define AVG_SAMPLES_MAX_AMOUNT (SIZE_MAX / INT16_MAX)


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioDevice::AudioDevice(MRH_Uint32 u32_ID,
                         std::string const& s_Name,
                         std::string const& s_Address,
                         int i_Port) : e_State(NONE),
                                       b_StateChanged(false),
                                       i_SocketFD(-1),
                                       u8_Endianess(0),
                                       f32_LastAmplitude(0.f),
                                       u32_ID(u32_ID),
                                       s_Name(s_Name),
                                       b_CanPlay(false),
                                       b_CanRecord(false)
{
    // Create a file descriptor first
    if ((i_SocketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        throw Exception("Failed to create audio device socket: " +
                        std::string(std::strerror(errno)) +
                        " (" +
                        std::to_string(errno) +
                        ")!");
    }
    
    // Setup socket for connection
    struct sockaddr_in c_Address;
    memset(&c_Address, '\0', sizeof(c_Address));
    c_Address.sin_family = AF_INET;
    c_Address.sin_port = htons(i_Port);
    c_Address.sin_addr.s_addr = inet_addr(s_Address.c_str());
    
    // Now connect
    if (connect(i_SocketFD, (struct sockaddr*)&c_Address, sizeof(c_Address)))
    {
        close(i_SocketFD);
        throw Exception("Failed to connect to audio device: " +
                        std::string(std::strerror(errno)) +
                        " (" +
                        std::to_string(errno) +
                        ")!");
    }
    
    // We are connected, send auth request to device
    Configuration& c_Configuration = Configuration::Singleton();
    
    AudioDeviceOpCode::SERVICE_CONNECT_REQUEST_DATA c_Request;
    c_Request.u32_OpCodeVersion = AUDIO_DEVICE_OPCODE_VERSION;
    c_Request.u32_RecordingKHz = c_Configuration.GetRecordingKHz();
    c_Request.u32_RecordingFrameElements = c_Configuration.GetRecordingFrameSamples();
    c_Request.u32_PlaybackKHz = c_Configuration.GetPlaybackKHz();
    c_Request.u32_PlaybackFrameElements = c_Configuration.GetPlaybackFrameSamples();
    
    // SERVICE_CONNECT_REQUEST
    
    // Sent, now read response
    AudioDeviceOpCode::DEVICE_CONNECT_RESPONSE_DATA c_Response;
    
    
    
    
    
    /*
    
    SERVICE_CONNECT_REQUEST
    
    typedef struct SERVICE_CONNECT_REQUEST_DATA_t
    {
        MRH_Uint32 u32_OpCodeVersion;
        
        // Recording data format to request
        MRH_Uint32 u32_RecordingKHz;
        MRH_Uint32 u32_RecordingFrameElements;
        
        // Playback data format to request
        MRH_Uint32 u32_PlaybackKHz;
        MRH_Uint32 u32_PlaybackFrameElements;
        
    }SERVICE_CONNECT_REQUEST_DATA;
    
    
    
    DEVICE_CONNECT_RESPONSE
    
    typedef struct DEVICE_CONNECT_RESPONSE_DATA_t
    {
        // Connection error (0 if none)
        OpCodeError u32_Error;
     
     
     
     // Endianess
     MRH_Uint8 u8_Endianess; // 0 = little, 1 = big
        
        // Device capabilities
        MRH_Uint8 u8_CanRecord;
        MRH_Uint8 u8_CanPlay;
        
    }DEVICE_CONNECT_RESPONSE_DATA;
    
    */
    
    
    try
    {
        c_Thread = std::thread(AudioDevice::Update, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start audio device thread!" + std::string(e.what()));
    }
}

AudioDevice::~AudioDevice() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Update
//*************************************************************************************

void AudioDevice::Update(AudioDevice* p_Instance) noexcept
{
    while (p_Instance->b_Update == true)
    {
        // @TODO: Run depending on state
    }
}

//*************************************************************************************
// Record
//*************************************************************************************

void AudioDevice::Record()
{
    MRH_Sint16 p_Data[2048] = { 0 };
    size_t testsize = 2048;
    
    // @TODO: Switch if required and wait for response,
    //        Otherwise recieve
    //
    //        CALCULATE SAMPLE -> AVERAGE <- AMPLITUDE HERE!
    //        Also, set last average
    
    /**
     *  Average Amplitude
     */
    
    // Do we need to use arrays for the average?
    size_t us_TotalSamples = 2048; // TODO: Buffer Size
    size_t us_SampleAverage = 0;
    
    if (us_TotalSamples > AVG_SAMPLES_MAX_AMOUNT)
    {
        // First, store all sample averages in groups for size constraints
        size_t us_Iterations = us_TotalSamples / AVG_SAMPLES_MAX_AMOUNT;
        us_Iterations += (us_TotalSamples % AVG_SAMPLES_MAX_AMOUNT > 0 ? 1 : 0);
        
        size_t us_Pos = 0; // Pos in buffer
        
        for (size_t i = 0; i < us_Iterations; ++i)
        {
            size_t us_GroupTotal = 0;
            size_t us_End;
            size_t us_ProcessedSamples;
            
            if (us_TotalSamples > (us_Pos + AVG_SAMPLES_MAX_AMOUNT))
            {
                us_ProcessedSamples = AVG_SAMPLES_MAX_AMOUNT;
                us_End = us_Pos + AVG_SAMPLES_MAX_AMOUNT;
            }
            else
            {
                us_ProcessedSamples = us_TotalSamples - us_Pos;
                us_End = us_TotalSamples;
            }
            
            for (; us_Pos < us_End; ++us_Pos)
            {
                us_GroupTotal += abs(p_Data[us_Pos]); // TODO: Buffer read
            }
            
            us_SampleAverage += (us_GroupTotal / us_ProcessedSamples);
        }
        
        // Now, create average for all
        us_SampleAverage /= us_Iterations;
    }
    else
    {
        for (size_t i = 0; i < us_TotalSamples; ++i)
        {
            us_SampleAverage += abs(p_Data[i]); // TODO: Buffer Read
        }
        
        us_SampleAverage /= us_TotalSamples;
    }
    
    f32_LastAmplitude = static_cast<double>(us_SampleAverage) / 32768.f;
}

//*************************************************************************************
// Playback
//*************************************************************************************

void AudioDevice::Play()
{
    // @TODO: Switch if required and wait for response,
    //        Otherwise send
    //        -> Data is correct here, no checking needed!
}

//*************************************************************************************
// Getters
//*************************************************************************************

AudioDevice::DeviceState AudioDevice::GetState() noexcept
{
    return e_State;
}

float AudioDevice::GetRecordingAmplitude() noexcept
{
    return f32_LastAmplitude;
}

bool AudioDevice::GetCanRecord() noexcept
{
    return b_CanRecord;
}

bool AudioDevice::GetCanPlay() noexcept
{
    return b_CanPlay;
}

//*************************************************************************************
// Setters
//*************************************************************************************

void AudioDevice::SetState(DeviceState e_State)
{
    if (e_State == RECORDING && b_CanRecord == false)
    {
        throw Exception("Device is unable to record audio!");
    }
    else if (e_State == PLAYBACK && b_CanPlay == false)
    {
        throw Exception("Device is unable to play audio!");
    }
    
    this->e_State = e_State;
    b_StateChanged = true;
}
