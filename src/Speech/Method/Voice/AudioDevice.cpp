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
                         int i_Port) : b_Update(true),
                                       s_Name(s_Name),
                                       u32_ID(u32_ID),
                                       i_SocketFD(-1),
                                       e_State(STOPPED),
                                       b_CanPlay(false),
                                       b_CanRecord(false),
                                       f32_AvgAmplitude(-1.f)
{
    // Create buffer list
    Configuration& c_Configuration = Configuration::Singleton();
    
    try
    {
        for (size_t i = 0; i < 2; ++i)
        {
            l_Audio.emplace_back(c_Configuration.GetRecordingKHz(),
                                 c_Configuration.GetRecordingFrameSamples(),
                                 c_Configuration.GetRecordingStorageS(),
                                 false);
        }
    }
    catch (...)
    {
        throw;
    }
    
    ActiveAudio = l_Audio.begin();
    
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
        Disconnect();
        throw Exception("Failed to connect to audio device: " +
                        std::string(std::strerror(errno)) +
                        " (" +
                        std::to_string(errno) +
                        ")!");
    }
    
    // We are connected, send auth request to device
    AudioDeviceOpCode::OpCode u32_RequestOpCode = AudioDeviceOpCode::OpCodeList::SERVICE_CONNECT_REQUEST;
    AudioDeviceOpCode::SERVICE_CONNECT_REQUEST_DATA c_Request;
    c_Request.u32_OpCodeVersion = AUDIO_DEVICE_OPCODE_VERSION;
    c_Request.u32_RecordingKHz = c_Configuration.GetRecordingKHz();
    c_Request.u32_RecordingFrameElements = c_Configuration.GetRecordingFrameSamples();
    c_Request.u32_PlaybackKHz = c_Configuration.GetPlaybackKHz();
    c_Request.u32_PlaybackFrameElements = c_Configuration.GetPlaybackFrameSamples();
    
#if MRH_HOST_IS_BIG_ENDIAN > 0
    // @TODO: Get Little Endian Bytes to Write if needed
#else
    if (WriteAll((const MRH_Uint8*)&u32_RequestOpCode, sizeof(u32_RequestOpCode)) == false ||
        WriteAll((const MRH_Uint8*)&(c_Request.u32_OpCodeVersion), sizeof(c_Request.u32_OpCodeVersion)) == false ||
        WriteAll((const MRH_Uint8*)&(c_Request.u32_RecordingKHz), sizeof(c_Request.u32_RecordingKHz)) == false ||
        WriteAll((const MRH_Uint8*)&(c_Request.u32_RecordingFrameElements), sizeof(c_Request.u32_RecordingFrameElements)) == false ||
        WriteAll((const MRH_Uint8*)&(c_Request.u32_PlaybackKHz), sizeof(c_Request.u32_PlaybackKHz)) == false ||
        WriteAll((const MRH_Uint8*)&(c_Request.u32_PlaybackFrameElements), sizeof(c_Request.u32_PlaybackFrameElements)) == false)
    {
        Disconnect();
        throw Exception("Audio device authentication failed (Write)!");
    }
#endif
    
    // Sent, now read response
    AudioDeviceOpCode::OpCode u32_ResponseOpCode = AudioDeviceOpCode::OpCodeList::SERVICE_CONNECT_REQUEST;
    AudioDeviceOpCode::DEVICE_CONNECT_RESPONSE_DATA c_Response;
    
    if (ReadAll((MRH_Uint8*)&u32_ResponseOpCode, sizeof(u32_ResponseOpCode), 5000) == false ||
        ReadAll((MRH_Uint8*)&(c_Response.u32_Error), sizeof(c_Response.u32_Error), 5000) == false ||
        ReadAll((MRH_Uint8*)&(c_Response.u8_CanRecord), sizeof(c_Response.u8_CanRecord), 5000) == false ||
        ReadAll((MRH_Uint8*)&(c_Response.u8_CanPlay), sizeof(c_Response.u8_CanPlay), 5000) == false)
    {
        Disconnect();
        throw Exception("Audio device authentication failed (Read)!");
    }
    
    // All read, now do setup
#if MRH_HOST_IS_BIG_ENDIAN > 0
    // @TODO: Convert values to big endian if needed (Opcode values)
#endif
    
    if (u32_ResponseOpCode != AudioDeviceOpCode::OpCodeList::DEVICE_CONNECT_RESPONSE ||
        c_Response.u32_Error != AudioDeviceOpCode::OpCodeErrorList::NONE)
    {
        Disconnect();
        throw Exception("Audio device authentication failed (Response)! Error: " + std::to_string(c_Response.u32_Error));
    }
    
    b_CanRecord = (c_Response.u8_CanRecord == AUDIO_DEVICE_BOOL_TRUE ? true : false);
    b_CanPlay = (c_Response.u8_CanPlay == AUDIO_DEVICE_BOOL_TRUE ? true : false);
    
    // Response OK, start updating
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
    
    Disconnect();
}

//*************************************************************************************
// Stop
//*************************************************************************************

void AudioDevice::Stop() noexcept
{
    if (e_State == STOPPED)
    {
        return;
    }
    
    /*
    c_SendMutex.lock();
    v_Send.clear();
    c_SendMutex.unlock();
    */
    
    f32_AvgAmplitude = -1.f;
    
    e_State = STOPPED;
}


void AudioDevice::Disconnect() noexcept
{
    if (i_SocketFD < 0)
    {
        return;
    }
    
    close(i_SocketFD);
    i_SocketFD = -1;
}

//*************************************************************************************
// Record
//*************************************************************************************

void AudioDevice::Record()
{
    if (b_CanRecord == false)
    {
        throw Exception("This audio device cannot record audio!");
    }
    else if (e_State == RECORDING)
    {
        return;
    }
    
    // Recording resets the inner buffer
    c_RecordingMutex.lock();
    ActiveAudio->Clear();
    c_RecordingMutex.unlock();
    
    // Reset amplitude, new recording
    f32_AvgAmplitude = -1.f;
    
    e_State = RECORDING;
}

void AudioDevice::Recieve()
{
    /**
     *  Read OpCode
     */
    
    static AudioDeviceOpCode::OpCode u32_OpCode;
    static bool b_OpCodeRead = false;
    int i_TimeoutMS = (e_State == PLAYING ? 0 : 100); // Do not wait on active playback
    ssize_t ss_Result;
    
    if (b_OpCodeRead == false)
    {
        ss_Result = Read((MRH_Uint8*)&u32_OpCode, sizeof(u32_OpCode), i_TimeoutMS);
        
        if (ss_Result != sizeof(u32_OpCode))
        {
            // Read failed, connection is now invalid
            if (ss_Result < 0)
            {
                Disconnect();
            }
            
            // Not enough data
            return;
        }
        
#if MRH_HOST_IS_BIG_ENDIAN > 0
        // @TODO: Flip opcode if needed
#endif
    }
    
    /**
     *  Handle OpCode Data
     */
    
    static size_t us_Read = 0;
    MRH_Uint8* p_Dst = NULL;
    size_t us_Length = 0;
    
    // Select the current position to read
    if (u32_OpCode == AudioDeviceOpCode::DEVICE_AUDIO_RECORDING_AUDIO)
    {
        Configuration& c_Config = Configuration::Singleton();
        static std::vector<MRH_Sint16> v_Audio(c_Config.GetRecordingFrameSamples(), 0);
        
        size_t us_AudioBytes = c_Config.GetRecordingFrameSamples() * sizeof(MRH_Sint16);
        
        if (us_Read == us_AudioBytes)
        {
            std::lock_guard<std::mutex> c_Guard(c_RecordingMutex);
            
#if MRH_HOST_IS_BIG_ENDIAN > 0
            // @TODO: Loop over array flip all samples to big endian if needed
#endif
            
            // Grab avg amplitude
            size_t us_SampleAverage = 0;
            
            for (size_t i = 0; i < v_Audio.size(); ++i)
            {
                us_SampleAverage += abs(v_Audio[i]);
            }
            
            us_SampleAverage /= v_Audio.size();
            f32_AvgAmplitude = static_cast<float>(static_cast<double>(us_SampleAverage) / 32768.f);
            
            // Add written audio
            try
            {
                ActiveAudio->AddChunk(v_Audio.data(), v_Audio.size());
            }
            catch (...)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Device recording buffer full!",
                                               "AudioDevice.cpp", __LINE__);
            }
        }
        else
        {
            p_Dst = (MRH_Uint8*)(v_Audio.data() + us_Read);
            us_Length = us_AudioBytes - us_Read;
        }
    }
    else if (u32_OpCode == AudioDeviceOpCode::DEVICE_STATE_CHANGED)
    {
        static AudioDeviceOpCode::DEVICE_STATE_CHANGED_DATA c_OpCode;
        static size_t us_ErrorSize = sizeof(c_OpCode.u32_Error);
        
        // @NOTE: This works since we're using little endian for our byte order!
        //        Also, it's not pretty... at all
        if (us_Read < us_ErrorSize)
        {
            us_Length = us_ErrorSize - us_Read;
            p_Dst = &(((MRH_Uint8*)&(c_OpCode.u32_Error))[us_ErrorSize - us_Length]);
        }
        else
        {
#if MRH_HOST_IS_BIG_ENDIAN > 0
            // @TODO: Flip to big endian if needed, and set state
#endif
            // All data read, set current device state if it differs
            if (c_OpCode.u32_Error != AudioDeviceOpCode::OpCodeErrorList::NONE)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Device state OpCode error: " +
                                                                       std::to_string(c_OpCode.u32_Error),
                                               "AudioDevice.cpp", __LINE__);
            }
        }
    }
    else
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Unknown OpCode read: " +
                                                               std::to_string(u32_OpCode),
                                       "AudioDevice.cpp", __LINE__);
    }
    
    /**
     *  Gather Required OpCode Data
     */
    
    // OpCode info set, start reading
    if (p_Dst != NULL && us_Length > 0)
    {
        ss_Result = Read(p_Dst, us_Length, i_TimeoutMS);
        
        if (ss_Result != sizeof(u32_OpCode))
        {
            // Read failed, connection is now invalid
            if (ss_Result < 0)
            {
                Disconnect();
            }
            
            // Not enough data
            us_Read += ss_Result;
            return;
        }
    }
    else
    {
        // Reset, this opcode was fully read and processed
        // We now read the next opcode id
        b_OpCodeRead = false;
    }
}

//*************************************************************************************
// Playback
//*************************************************************************************

void AudioDevice::Play()
{
    if (b_CanPlay == false)
    {
        throw Exception("This audio device cannot play audio!");
    }
    else if (e_State == PLAYING)
    {
        return;
    }
    
    // @NOTE: Audio set before
    //        Also, keep last amplitude for recording!
    
    e_State = PLAYING;
}

void AudioDevice::Send()
{
    @TODO: Switch if required and wait for response,
           Otherwise send
            -> Data is correct here, no checking needed!
            -> Copy Audio to static var to protect against resets (for
               last opcode with audio data to be sent correctly)
            -> Remove Playback State if all audio was sent
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
// I/O
//*************************************************************************************

bool AudioDevice::ReadAll(MRH_Uint8* p_Dst, size_t us_Length, int i_TimeoutMS)
{
    if (Read(p_Dst, us_Length, i_TimeoutMS) != us_Length)
    {
        return false;
    }
    
    return true;
}

ssize_t AudioDevice::Read(MRH_Uint8* p_Dst, size_t us_Length, int i_TimeoutMS)
{
    struct pollfd s_PollFD;
    
    s_PollFD.fd = i_SocketFD;
    s_PollFD.events = POLLIN;
    
    switch (poll(&s_PollFD, (nfds_t)1, i_TimeoutMS))
    {
        case -1:
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Audio device polling failed: " +
                                                                   std::string(std::strerror(errno)) +
                                                                   " (" +
                                                                   std::to_string(errno) +
                                                                   ")!",
                                           "AudioDevice.cpp", __LINE__);
            return -1;
        case 0:
            return 0;
            
        default:
            break;
    }
    
    // Read to destination
    size_t us_Pos = 0;
    ssize_t ss_Read;
    
    do
    {
        if ((ss_Read = read(i_SocketFD, &(p_Dst[us_Pos]), us_Length - us_Pos)) < 0)
        {
            if (errno != EAGAIN)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Audio device read failed: " +
                                                                       std::string(std::strerror(errno)) +
                                                                       " (" +
                                                                       std::to_string(errno) +
                                                                       ")!",
                                               "AudioDevice.cpp", __LINE__);
                return -1;
            }
        }
        else if (ss_Read == 0)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "0 bytes read but polling succeeded!",
                                           "AudioDevice.cpp", __LINE__);
            break;
        }
        else if (ss_Read > 0)
        {
            us_Pos += ss_Read;
        }
    }
    while (ss_Read > 0 && us_Pos < us_Length);
    
    // Return read result
    return us_Pos;
}

bool AudioDevice::WriteAll(const MRH_Uint8* p_Src, size_t us_Length)
{
    if (Write(p_Src, us_Length) != us_Length)
    {
        return false;
    }
    
    return true;
}

ssize_t AudioDevice::Write(const MRH_Uint8* p_Src, size_t us_Length)
{
    // Write as much as possible
    ssize_t ss_Write;
    size_t us_Pos = 0;
    
    do
    {
        if ((ss_Write = write(i_SocketFD, &(p_Src[us_Pos]), us_Length - us_Pos)) < 0)
        {
            if (errno != EAGAIN)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Audio device write failed: " +
                                                                       std::string(std::strerror(errno)) +
                                                                       " (" +
                                                                       std::to_string(errno) +
                                                                       ")!",
                                               "AudioDevice.cpp", __LINE__);
                return -1;
            }
        }
        else if (ss_Write > 0)
        {
            us_Pos += ss_Write;
        }
    }
    while (ss_Write > 0 && us_Pos < us_Length);
    
    // Return write result
    return us_Pos;
}

//*************************************************************************************
// Getters
//*************************************************************************************

AudioTrack const& AudioDevice::GetRecordedAudio() noexcept
{
    // Flip buffer for recording
    std::lock_guard<std::mutex> c_Guard(c_RecordingMutex);
    
    if ((++ActiveAudio) == l_Audio.end())
    {
        ActiveAudio = l_Audio.begin();
    }
    
    return *ActiveAudio;
}

AudioDevice::DeviceState AudioDevice::GetState() noexcept
{
    return e_State;
}

bool AudioDevice::GetCanRecord() noexcept
{
    return b_CanRecord;
}

bool AudioDevice::GetCanPlay() noexcept
{
    return b_CanPlay;
}

float AudioDevice::GetAvgRecordingAmplitude() noexcept
{
    return f32_AvgAmplitude;
}
