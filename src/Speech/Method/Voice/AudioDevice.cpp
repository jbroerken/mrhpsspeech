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
#include <ctime>
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #include <byteswap.h>
#endif

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./AudioDevice.h"
#include "./AudioDeviceOpCode.h"
#include "../../../Configuration.h"

// Pre-defined
#define HANDLE_OPCODE_ID false
#define HANDLE_OPCODE_DATA true

#define AUDIO_SAMPLE_SIZE_B sizeof(MRH_Sint16)

#define AUDIO_READ_BUFFER_SIZE_U8 1024
#define AUDIO_READ_BUFFER_SIZE_S16 AUDIO_READ_BUFFER_SIZE_U8 / AUDIO_SAMPLE_SIZE_B

#define DEVICE_CONNECTION_SLEEP_WAIT_S 15
#define DEVICE_CONNECTION_HEARTBEAT_S 60


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioDevice::AudioDevice(MRH_Uint32 u32_ID,
                         std::string const& s_Name,
                         std::string const& s_Address,
                         int i_Port) : b_Update(true),
                                       s_Name(s_Name),
                                       u32_ID(u32_ID),
                                       s_Address(s_Address),
                                       i_Port(i_Port),
                                       i_SocketFD(-1),
                                       e_State(STOPPED),
                                       c_ReadInfo(HANDLE_OPCODE_ID, 0),
                                       c_WriteInfo(HANDLE_OPCODE_ID, 0),
                                       b_CanPlay(false),
                                       b_CanRecord(false),
                                       us_AvailableSamples(0),
                                       u64_HeartbeatTimeoutS(0)
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
    
    // Set the initial active buffer
    ActiveAudio = l_Audio.begin();
    
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
    
    // Recording resets the inner buffer
    c_RecordingMutex.lock();
    ActiveAudio->Clear();
    c_RecordingMutex.unlock();
    
    // Reset amplitude, new recording
    us_AvailableSamples = 0;
    
    e_State = STOPPED;
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
    us_AvailableSamples = 0;
    
    e_State = RECORDING;
}

void AudioDevice::Recieve()
{
    static AudioDeviceOpCode::OpCode u32_OpCode;
    
    // Read opcode if needed
    if (c_ReadInfo.first == HANDLE_OPCODE_ID)
    {
        ssize_t ss_Result = Read((MRH_Uint8*)&u32_OpCode,
                                 sizeof(u32_OpCode),
                                 (e_State == PLAYING ? 0 : 100));
        
        if (ss_Result != sizeof(u32_OpCode))
        {
            // Read failed, connection is now invalid
            if (ss_Result < 0)
            {
                throw Exception("OpCode ID read returned -1!");
            }
            else
            {
                c_ReadInfo.second += ss_Result;
            }
            
            // Not enough data
            return;
        }
        
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        u32_OpCode = bswap_32(u32_OpCode);
#endif
        
        // Set as read
        c_ReadInfo.first = HANDLE_OPCODE_DATA;
    }
    
    // We read our opcode (or know it already), now perform reading of data
    // @NOTE: If read successfully, switch back read data to opcode and size to 0!
    try
    {
        switch (u32_OpCode)
        {
            case AudioDeviceOpCode::ALL_HEARTBEAT:
                u64_HeartbeatTimeoutS = time(NULL) + DEVICE_CONNECTION_HEARTBEAT_S;
                c_ReadInfo.first = HANDLE_OPCODE_ID;
                c_ReadInfo.second = 0;
                break;
                
            case AudioDeviceOpCode::DEVICE_RECORDED_AUDIO:
                if ((c_ReadInfo.first = RecieveAudio()) == HANDLE_OPCODE_ID)
                {
                    c_ReadInfo.second = 0;
                }
                break;
                
            case AudioDeviceOpCode::DEVICE_STATE_CHANGED:
                if ((c_ReadInfo.first = RecieveStateChanged()) == HANDLE_OPCODE_ID)
                {
                    c_ReadInfo.second = 0;
                }
                break;
                
            default:
                break;
        }
    }
    catch (...)
    {
        throw;
    }
    
    // Heartbeat issue?
    if (time(NULL) > u64_HeartbeatTimeoutS)
    {
        throw Exception("Heartbeat timeout!");
    }
}

bool AudioDevice::RecieveAudio()
{
    static MRH_Uint8 p_AudioBuffer[AUDIO_READ_BUFFER_SIZE_U8]; // 512 elements / pass
    
    size_t us_FrameSize = Configuration::Singleton().GetRecordingFrameSamples() * AUDIO_SAMPLE_SIZE_B;
    size_t us_Length;
    ssize_t ss_Result;
    
    do
    {
        // Get read size first
        us_Length = AUDIO_READ_BUFFER_SIZE_U8 - (c_ReadInfo.second % AUDIO_READ_BUFFER_SIZE_U8);
        ss_Result = Read(&(p_AudioBuffer[AUDIO_READ_BUFFER_SIZE_U8 - us_Length]),
                         us_Length,
                         (e_State == PLAYING ? 0 : 100));
        
        if (ss_Result != us_Length)
        {
            if (ss_Result < 0)
            {
                throw Exception("Audio buffer read returned -1!");
            }
            else
            {
                c_ReadInfo.second += ss_Result;
            }
            
            return HANDLE_OPCODE_DATA;
        }
        
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        for (size_t i = 0; i < AUDIO_READ_BUFFER_SIZE_S16; ++i)
        {
            MRH_Sint16 s16_Val = ((MRH_Sint16*)p_AudioBuffer)[i];
            ((MRH_Sint16*)p_AudioBuffer)[i] = (MRH_Sint16)((s16_Val << 8) + (s16_Val >> 8));
        }
#endif
        
        // Remember added read size
        c_ReadInfo.second += ss_Result;
        
        // Add this fully read buffer as audio!
        try
        {
            std::lock_guard<std::mutex> c_Guard(c_RecordingMutex);
            ActiveAudio->AddAudio((const MRH_Sint16*)p_AudioBuffer,
                                  AUDIO_READ_BUFFER_SIZE_S16);
            us_AvailableSamples += AUDIO_READ_BUFFER_SIZE_S16; // Outside info
        }
        catch (...)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Device recording buffer full!",
                                           "AudioDevice.cpp", __LINE__);
        }
        
    }
    while (c_ReadInfo.second < us_FrameSize); // Framesize = Full OpCode Data
    
    return HANDLE_OPCODE_ID;
}

bool AudioDevice::RecieveStateChanged()
{
    static AudioDeviceOpCode::DEVICE_STATE_CHANGED_DATA c_OpCode;
    static size_t us_ErrorSize = sizeof(c_OpCode.u32_Error);
    
    size_t us_Length = us_ErrorSize - c_ReadInfo.second;
    ssize_t ss_Result = Read(&(((MRH_Uint8*)&(c_OpCode.u32_Error))[us_ErrorSize - us_Length]),
                             us_Length,
                             (e_State == PLAYING ? 0 : 100));
    
    if (ss_Result != us_Length)
    {
        if (ss_Result < 0)
        {
            throw Exception("Device state changed read returned -1!");
        }
        else
        {
            c_ReadInfo.second += ss_Result;
        }
        
        return HANDLE_OPCODE_DATA;
    }
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    c_OpCode.u32_Error = bswap_32(c_OpCode.u32_Error);
#endif
    
    // All data read, set current device state if it differs
    if (c_OpCode.u32_Error != AudioDeviceOpCode::OpCodeErrorList::NONE)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Device state OpCode error: " +
                                                               std::to_string(c_OpCode.u32_Error),
                                       "AudioDevice.cpp", __LINE__);
    }
    
    return HANDLE_OPCODE_ID;
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
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    
    while (p_Instance->b_Update == true)
    {
        // Are we connected? try if not
        if (p_Instance->i_SocketFD < 0)
        {
            try
            {
                p_Instance->Connect();
            }
            catch (Exception& e)
            {
                c_Logger.Log(MRH_PSBLogger::INFO, e.what(),
                             "AudioDevice.cpp", __LINE__);
                
                p_Instance->Disconnect();
                std::this_thread::sleep_for(std::chrono::seconds(DEVICE_CONNECTION_SLEEP_WAIT_S));
                continue;
            }
        }
        
        // We are connected, RW
        try
        {
            p_Instance->Recieve();
            p_Instance->Send();
        }
        catch (Exception& e)
        {
            c_Logger.Log(MRH_PSBLogger::ERROR, e.what(),
                         "AudioDevice.cpp", __LINE__);
            
            p_Instance->Disconnect();
        }
    }
}

void AudioDevice::SwitchRecordingBuffer() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_RecordingMutex);
    
    if ((++ActiveAudio) == l_Audio.end())
    {
        ActiveAudio = l_Audio.begin();
    }
}

//*************************************************************************************
// Connection
//*************************************************************************************

void AudioDevice::Connect()
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
        Disconnect();
        throw Exception("Failed to connect to audio device: " +
                        std::string(std::strerror(errno)) +
                        " (" +
                        std::to_string(errno) +
                        ")!");
    }
    
    // We are connected, send auth request to device
    Configuration& c_Config = Configuration::Singleton();
    
    AudioDeviceOpCode::OpCode u32_RequestOpCode = AudioDeviceOpCode::OpCodeList::SERVICE_CONNECT_REQUEST;
    AudioDeviceOpCode::SERVICE_CONNECT_REQUEST_DATA c_Request;
    c_Request.u32_OpCodeVersion = AUDIO_DEVICE_OPCODE_VERSION;
    c_Request.u32_RecordingKHz = c_Config.GetRecordingKHz();
    c_Request.u32_RecordingFrameElements = c_Config.GetRecordingFrameSamples();
    c_Request.u32_PlaybackKHz = c_Config.GetPlaybackKHz();
    c_Request.u32_PlaybackFrameElements = c_Config.GetPlaybackFrameSamples();
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    u32_RequestOpCode = bswap_32(u32_RequestOpCode);
    c_Request.u32_OpCodeVersion = bswap_32(c_Request.u32_OpCodeVersion);
    c_Request.u32_RecordingKHz = bswap_32(c_Request.u32_RecordingKHz);
    c_Request.u32_RecordingFrameElements = bswap_32(c_Request.u32_RecordingFrameElements);
    c_Request.u32_PlaybackKHz = bswap_32(c_Request.u32_PlaybackKHz);
    c_Request.u32_PlaybackFrameElements = bswap_32(c_Request.u32_PlaybackFrameElements);
#endif
    
    if (WriteAll((const MRH_Uint8*)&u32_RequestOpCode, sizeof(u32_RequestOpCode)) == false ||
        WriteAll((const MRH_Uint8*)&(c_Request.u32_OpCodeVersion), sizeof(c_Request.u32_OpCodeVersion)) == false ||
        WriteAll((const MRH_Uint8*)&(c_Request.u32_RecordingKHz), sizeof(c_Request.u32_RecordingKHz)) == false ||
        WriteAll((const MRH_Uint8*)&(c_Request.u32_RecordingFrameElements), sizeof(c_Request.u32_RecordingFrameElements)) == false ||
        WriteAll((const MRH_Uint8*)&(c_Request.u32_PlaybackKHz), sizeof(c_Request.u32_PlaybackKHz)) == false ||
        WriteAll((const MRH_Uint8*)&(c_Request.u32_PlaybackFrameElements), sizeof(c_Request.u32_PlaybackFrameElements)) == false)
    {
        throw Exception("Audio device authentication failed (Write)!");
    }
    
    // Sent, now read response and perform setup
    AudioDeviceOpCode::OpCode u32_ResponseOpCode = AudioDeviceOpCode::OpCodeList::SERVICE_CONNECT_REQUEST;
    AudioDeviceOpCode::DEVICE_CONNECT_RESPONSE_DATA c_Response;
    
    if (ReadAll((MRH_Uint8*)&u32_ResponseOpCode, sizeof(u32_ResponseOpCode), 5000) == false ||
        ReadAll((MRH_Uint8*)&(c_Response.u32_Error), sizeof(c_Response.u32_Error), 5000) == false ||
        ReadAll((MRH_Uint8*)&(c_Response.u8_CanRecord), sizeof(c_Response.u8_CanRecord), 5000) == false ||
        ReadAll((MRH_Uint8*)&(c_Response.u8_CanPlay), sizeof(c_Response.u8_CanPlay), 5000) == false)
    {
        throw Exception("Audio device authentication failed (Read)!");
    }
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    u32_ResponseOpCode = bswap_32(u32_ResponseOpCode);
    c_Response.u32_Error = bswap_32(c_Request.u32_Error);
#endif
    
    if (u32_ResponseOpCode != AudioDeviceOpCode::OpCodeList::DEVICE_CONNECT_RESPONSE ||
        c_Response.u32_Error != AudioDeviceOpCode::OpCodeErrorList::NONE)
    {
        throw Exception("Audio device authentication failed (Response)! Error: " + std::to_string(c_Response.u32_Error));
    }
    
    b_CanRecord = (c_Response.u8_CanRecord == AUDIO_DEVICE_BOOL_TRUE ? true : false);
    b_CanPlay = (c_Response.u8_CanPlay == AUDIO_DEVICE_BOOL_TRUE ? true : false);
    
    // Flip buffer for recording in case the current
    // buffer is in use
    SwitchRecordingBuffer();
    
    // Set next heartbeat timeout for connection
    u64_HeartbeatTimeoutS = time(NULL) + DEVICE_CONNECTION_HEARTBEAT_S;
}

void AudioDevice::Disconnect() noexcept
{
    if (i_SocketFD < 0)
    {
        return;
    }
    
    c_ReadInfo.first = HANDLE_OPCODE_ID;
    c_ReadInfo.second = 0;
    
    c_WriteInfo.first = HANDLE_OPCODE_ID;
    c_WriteInfo.second = 0;
    
    close(i_SocketFD);
    i_SocketFD = -1;
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
    SwitchRecordingBuffer();
    
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

size_t AudioDevice::GetAvailableSamples() noexcept
{
    return us_AvailableSamples;
}
