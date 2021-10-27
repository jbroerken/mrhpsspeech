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
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <cstring>
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
#define AUDIO_SAMPLE_SIZE_B sizeof(MRH_Sint16)

#define AUDIO_READ_BUFFER_SIZE_U8 1024
#define AUDIO_READ_BUFFER_SIZE_S16 AUDIO_READ_BUFFER_SIZE_U8 / AUDIO_SAMPLE_SIZE_B

#define DEVICE_CONNECTION_SLEEP_WAIT_S 15
#define DEVICE_CONNECTION_HEARTBEAT_S 30

#define RECORDING_AUDIO_TRACK_COUNT 2


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioDevice::AudioDevice(std::string const& s_Name,
                         int i_SocketFD,
                         bool b_CanPlay,
                         bool b_CanRecord) : i_SocketFD(i_SocketFD),
                                             s_Name(s_Name),
                                             b_CanPlay(b_CanPlay),
                                             b_CanRecord(b_CanRecord),
                                             us_RecievedSamples(0),
                                             us_PlaybackFrameSamples(0)
{
    // Create buffer list
    Configuration& c_Configuration = Configuration::Singleton();
    
    // Make sure samples are a multiple of the storage buffer
    us_PlaybackFrameSamples = c_Configuration.GetPlaybackFrameSamples();
    size_t us_RecordingFrameSamples = c_Configuration.GetRecordingFrameSamples();
    
    if (us_RecordingFrameSamples % AUDIO_READ_BUFFER_SIZE_S16 != 0)
    {
        throw Exception("Recording frame samples must be a multiple of " +
                        std::to_string(AUDIO_READ_BUFFER_SIZE_S16) +
                        "!");
    }
    
    try
    {
        for (size_t i = 0; i < RECORDING_AUDIO_TRACK_COUNT; ++i)
        {
            l_RecordingAudio.emplace_back(c_Configuration.GetRecordingKHz(),
                                          us_RecordingFrameSamples,
                                          c_Configuration.GetRecordingStorageS(),
                                          false);
        }
    }
    catch (...)
    {
        throw;
    }
    
    // Set the initial active buffer
    ActiveAudio = l_RecordingAudio.begin();
    
    // All set, start updating
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
    if (i_SocketFD != AUDIO_DEVICE_SOCKET_DISCONNECTED)
    {
        close(i_SocketFD);
        i_SocketFD = AUDIO_DEVICE_SOCKET_DISCONNECTED;
    }
    
    c_Thread.join();
}

//*************************************************************************************
// Update
//*************************************************************************************

void AudioDevice::Update(AudioDevice* p_Instance) noexcept
{
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    
    while (p_Instance->i_SocketFD != AUDIO_DEVICE_SOCKET_DISCONNECTED)
    {
        try
        {
            p_Instance->Recieve();
            p_Instance->Send();
        }
        catch (Exception& e)
        {
            c_Logger.Log(MRH_PSBLogger::ERROR, e.what(),
                         "AudioDevice.cpp", __LINE__);
            break;
        }
    }
    
    close(p_Instance->i_SocketFD);
    p_Instance->i_SocketFD = -1;
}

void AudioDevice::SwitchRecordingBuffer() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_RecordingMutex);
    
    if ((++ActiveAudio) == l_RecordingAudio.end())
    {
        ActiveAudio = l_RecordingAudio.begin();
        
        ActiveAudio->Clear();
        us_RecievedSamples = 0;
    }
}

//*************************************************************************************
// Record
//*************************************************************************************

void AudioDevice::StartRecording() noexcept
{
    // Signal recording start
    c_WriteMutex.lock();
    l_WriteData.emplace_front(1, AudioDeviceOpCode::SERVICE_START_RECORDING);
    c_WriteMutex.unlock();
    
    // Reset the current recording
    std::lock_guard<std::mutex> c_Guard(c_RecordingMutex);
    ActiveAudio->Clear();
    us_RecievedSamples = 0;
}

void AudioDevice::StopRecording() noexcept
{
    // Signal recording stop
    std::lock_guard<std::mutex> c_Guard(c_WriteMutex);
    l_WriteData.emplace_front(1, AudioDeviceOpCode::SERVICE_STOP_RECORDING);
}

void AudioDevice::Recieve()
{
    /**
     *  OpCode
     */
    
    static AudioDeviceOpCode::OpCode u8_OpCode;
    static bool b_OpCodeRead = false;
    static size_t us_ReadBytes = 0;
    ssize_t ss_Result;
    size_t us_Length;
    
    // Read opcode if needed
    if (b_OpCodeRead == false)
    {
        ss_Result = Read(&u8_OpCode,
                         1,
                         (l_WriteData.size() > 0 ? 0 : 100));
        
        if (ss_Result != 1)
        {
            // Read failed, connection is now invalid
            if (ss_Result < 0)
            {
                throw Exception("OpCode ID read returned -1!");
            }
            else
            {
                us_ReadBytes += ss_Result;
            }
            
            // Not enough data
            return;
        }
        
        // Set as read
        b_OpCodeRead = true;
    }
    
    /**
     *  Heartbeat
     */
    
    static MRH_Uint64 u64_HeartbeatTimerS = time(NULL) + DEVICE_CONNECTION_HEARTBEAT_S;
    
    // Did we read a heartbeat opcode?
    if (u8_OpCode == AudioDeviceOpCode::ALL_HEARTBEAT)
    {
        u64_HeartbeatTimerS = time(NULL) + DEVICE_CONNECTION_HEARTBEAT_S;
        
        b_OpCodeRead = false;
        us_ReadBytes = 0;
        
        return;
    }
    else if (time(NULL) > u64_HeartbeatTimerS)
    {
        throw Exception("Heartbeat timeout!");
    }
    
    /**
     *  Audio
     */
    
    // OpCode Data is audio
    static MRH_Uint8 p_AudioBuffer[AUDIO_READ_BUFFER_SIZE_U8]; // 512 elements / pass
    
    size_t us_FrameSize = Configuration::Singleton().GetRecordingFrameSamples() * AUDIO_SAMPLE_SIZE_B;
    
    do
    {
        // Get read size first
        us_Length = AUDIO_READ_BUFFER_SIZE_U8 - (us_ReadBytes % AUDIO_READ_BUFFER_SIZE_U8);
        ss_Result = Read(&(p_AudioBuffer[AUDIO_READ_BUFFER_SIZE_U8 - us_Length]),
                         us_Length,
                         (l_WriteData.size() > 0 ? 0 : 100));
        
        if (ss_Result != us_Length)
        {
            if (ss_Result < 0)
            {
                throw Exception("Audio buffer read returned -1!");
            }
            else
            {
                us_ReadBytes += ss_Result;
            }
            
            return;
        }
        
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        MRH_Sint16* p_Result = (MRH_Sint16*)p_AudioBuffer;
        
        for (size_t i = 0; i < AUDIO_READ_BUFFER_SIZE_S16; ++i)
        {
            p_Result[i] = (MRH_Sint16)((p_Result[i] << 8) + (p_Result[i] >> 8));
        }
#endif
        
        // Remember added read size
        us_ReadBytes += ss_Result;
        
        // Add this fully read buffer as audio!
        try
        {
            std::lock_guard<std::mutex> c_Guard(c_RecordingMutex);
            ActiveAudio->AddAudio((const MRH_Sint16*)p_AudioBuffer,
                                  AUDIO_READ_BUFFER_SIZE_S16);
            us_RecievedSamples += AUDIO_READ_BUFFER_SIZE_S16; // Outside info
        }
        catch (...)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Device recording buffer full!",
                                           "AudioDevice.cpp", __LINE__);
        }
        
    }
    while (us_ReadBytes < us_FrameSize); // Framesize = Full OpCode Data
    
    // Fully read, next is opcode
    b_OpCodeRead = false;
}

//*************************************************************************************
// Playback
//*************************************************************************************

void AudioDevice::Play(AudioTrack const& c_Audio)
{
    if (b_CanPlay == false)
    {
        throw Exception("This audio device cannot play audio!");
    }
    else if (GetSendingActive() == true)
    {
        throw Exception("Already playing audio!");
    }
    else if (c_Audio.us_ChunkElements != us_PlaybackFrameSamples)
    {
        throw Exception("Invalid chunk size for given audio track!");
    }
    
    // Set the audio to write
    std::lock_guard<std::mutex> c_Guard(c_WriteMutex);
    std::list<AudioTrack::Chunk> const& l_Chunk = c_Audio.GetChunksConst();
    
    for (auto& Chunk : l_Chunk)
    {
        l_WriteData.emplace_back(1, AudioDeviceOpCode::SERVICE_PLAYBACK_AUDIO);
        
        // @NOTE: No checking for padding, audio track chunks are always padded!
        const MRH_Uint8* p_BufferStart = (const MRH_Uint8*)(Chunk.GetBufferConst());
        const MRH_Uint8* p_BufferEnd = p_BufferStart + (Chunk.GetElementsCurrent() * AUDIO_SAMPLE_SIZE_B);
        
        l_WriteData.emplace_back(p_BufferStart,
                                 p_BufferEnd);
        
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        MRH_Sint16* p_Samples = (MRH_Sint16*)(l_WriteData.back().data());
        size_t us_Elements = l_WriteData.back().size() / AUDIO_SAMPLE_SIZE_B;
        
        for (size_t i = 0; i < us_Elements; ++i)
        {
            p_Samples[i] = (MRH_Sint16)((p_Samples[i] << 8) + (p_Samples[i] >> 8));
        }
#endif
    }
}

void AudioDevice::Send()
{
    /**
     *  Heartbeat
     */
    
    static MRH_Uint64 u64_HeartbeatTimerS = time(NULL) + (DEVICE_CONNECTION_HEARTBEAT_S / 2);
    
    // Is a heartbeat required?
    if (u64_HeartbeatTimerS <= time(NULL))
    {
        // Simply add into write queue
        std::lock_guard<std::mutex> c_Guard(c_WriteMutex);
        l_WriteData.emplace_front(1, AudioDeviceOpCode::SERVICE_PLAYBACK_AUDIO);
        
        u64_HeartbeatTimerS = time(NULL) + DEVICE_CONNECTION_HEARTBEAT_S;
    }
    
    /**
     *  Write Buffer
     */
    
    static std::vector<MRH_Uint8> v_OpCode;
    static size_t us_WrittenBytes = 0;
    ssize_t ss_Result;
    size_t us_Length;
    
    // Do we need to grab new send data?
    if (us_WrittenBytes == 0)
    {
        std::lock_guard<std::mutex> c_Guard(c_WriteMutex);
        
        // No data needed to be written?
        if (l_WriteData.size() > 0)
        {
            return;
        }
        
        // Set
        // @NOTE: Byte swap happened on adding into the queue!
        v_OpCode = std::move(l_WriteData.front());
        l_WriteData.pop_front();
    }
    
    // Got data, write
    us_Length = v_OpCode.size() - us_WrittenBytes;
    ss_Result = Write(&(v_OpCode[us_WrittenBytes]),
                      us_Length);
    
    if (ss_Result != us_Length)
    {
        if (ss_Result < 0)
        {
            throw Exception("Write buffer write returned -1!");
        }
        else
        {
            us_WrittenBytes += ss_Result;
        }
    }
    else
    {
        // Reset written on finish
        us_WrittenBytes = 0;
    }
}

//*************************************************************************************
// I/O
//*************************************************************************************

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

bool AudioDevice::GetConnected() noexcept
{
    return i_SocketFD != AUDIO_DEVICE_SOCKET_DISCONNECTED ? true : false;
}

AudioTrack const& AudioDevice::GetRecordedAudio() noexcept
{
    // Flip buffer for recording
    auto Result = ActiveAudio;
    SwitchRecordingBuffer();
    
    return *Result;
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
    return us_RecievedSamples;
}

bool AudioDevice::GetSendingActive() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_WriteMutex);
    return l_WriteData.size() > 0 ? true : false;
}
