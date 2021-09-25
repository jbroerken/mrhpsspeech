/**
 *  PADevice.cpp
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
#include <cmath>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./PADevice.h"
#include "../../../Configuration.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

PADevice::PADevice() : p_InputStream(NULL),
                       p_OutputStream(NULL),
                       c_Converter(Configuration::Singleton().GetPASpeakerKHz())
{
    // Init methods
    try
    {
        SetupInput();
        SetupOutput();
    }
    catch (Exception& e)
    {
        CloseInput();
        
        // Output is last, never open on failure
        throw;
    }
}

PADevice::~PADevice() noexcept
{
    CloseStream(p_InputStream);
    CloseStream(p_OutputStream);
}

PADevice::Input::Input() noexcept : p_Buffer(NULL),
                                    p_BufferA(NULL),
                                    p_BufferB(NULL),
                                    us_BufferSize(0),
                                    us_BufferPos(0),
                                    u32_KHz(16000),
                                    u8_Channels(1)
{}

PADevice::Input::~Input() noexcept
{
    if (p_BufferA != NULL)
    {
        delete[] p_BufferA;
    }
    
    if (p_BufferB != NULL)
    {
        delete[] p_BufferB;
    }
}

PADevice::Output::Output() noexcept : us_BufferPos(0),
                                      u32_KHz(16000),
                                      u8_Channels(1)
{}

PADevice::Output::~Output() noexcept
{}

//*************************************************************************************
// Stream
//*************************************************************************************

void PADevice::StartStream(PaStream* p_Stream)
{
    PaError i_Error = Pa_IsStreamActive(p_Stream);
    
    if (i_Error == 1) // Already active
    {
        return;
    }
    else if (i_Error < 0)
    {
        throw Exception("Failed to get PortAudio stream state! " + std::string(Pa_GetErrorText(i_Error)));
    }
    else if ((i_Error = Pa_StartStream(p_Stream)) != paNoError)
    {
        throw Exception("Failed to start PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

void PADevice::StopStream(PaStream* p_Stream)
{
    PaError i_Error = Pa_IsStreamStopped(p_Stream);
    
    if (i_Error == 1) // Already stopped
    {
        return;
    }
    else if (i_Error < 0)
    {
        throw Exception("Failed to get PortAudio stream state! " + std::string(Pa_GetErrorText(i_Error)));
    }
    else if ((i_Error = Pa_StopStream(p_Stream)) != paNoError)
    {
        throw Exception("Failed to stop PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

void PADevice::CloseStream(PaStream* p_Stream) noexcept
{
    try
    {
        StopStream(p_Stream);
    }
    catch (Exception& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                       "PADevice.cpp", __LINE__);
    }
    
    PaError i_Error;
    if ((i_Error = Pa_CloseStream(p_Stream)) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to close PortAudio tream! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PADevice.cpp", __LINE__);
    }
}

void PADevice::StopAll() noexcept
{
    try
    {
        StopStream(p_InputStream);
    }
    catch (Exception& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Input Stream: " + std::string(e.what()),
                                       "PADevice.cpp", __LINE__);
    }
    
    try
    {
        StopStream(p_OutputStream);
    }
    catch (Exception& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Output Stream: " + std::string(e.what()),
                                       "PADevice.cpp", __LINE__);
    }
}

//*************************************************************************************
// Input
//*************************************************************************************

void PADevice::SetupInput()
{
    Configuration& c_Config = Configuration::Singleton();
    PaError i_Error;
    
    // Grab device info
    MRH_Uint32 u32_DevID = c_Config.GetPAMicDeviceID();
    const PaDeviceInfo* p_DevInfo;
    
    if ((p_DevInfo = Pa_GetDeviceInfo(u32_DevID)) == NULL)
    {
        throw Exception("Failed to get device info for input device " + std::to_string(u32_DevID));
    }
    
    // Set input stream info
    PaStreamParameters c_InputParameters;
    
    bzero(&c_InputParameters, sizeof(c_InputParameters));
    c_InputParameters.channelCount = p_DevInfo->maxInputChannels;
    c_InputParameters.device = u32_DevID;
    c_InputParameters.hostApiSpecificStreamInfo = NULL;
    c_InputParameters.sampleFormat = paInt16;
    c_InputParameters.suggestedLatency = p_DevInfo->defaultLowInputLatency;
    
    MRH_Uint32 u32_KHz = c_Config.GetPAMicKHz();
    
    // Open audio stream
    i_Error = Pa_OpenStream(&p_InputStream,
                            &c_InputParameters,
                            NULL, // No output info
                            u32_KHz,
                            c_Config.GetPAMicFrameSamples(),
                            paClipOff,// paNoFlag,
                            PAInputCallback,
                            &c_InputAudio);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to open PortAudio input stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
    
    const PaStreamInfo* p_StreamInfo = Pa_GetStreamInfo(p_InputStream);
    
    if (p_StreamInfo == NULL)
    {
        throw Exception("Failed to get PortAudio input stream info!");
    }
    
    // Update audio info
    c_InputAudio.u32_KHz = u32_KHz;
    c_InputAudio.u8_Channels = p_DevInfo->maxInputChannels;
    c_InputAudio.us_BufferSize = u32_KHz * c_Config.GetPAMicRecordingStorageS();
    c_InputAudio.p_BufferA = new MRH_Sint16[c_InputAudio.us_BufferSize];
    c_InputAudio.p_BufferB = new MRH_Sint16[c_InputAudio.us_BufferSize];
    c_InputAudio.p_Buffer = c_InputAudio.p_BufferA; // Start at A
    c_InputAudio.us_BufferPos = 0;
    
    // Print info
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    c_Logger.Log(MRH_PSBLogger::INFO, "Input Device: " + std::string(p_DevInfo->name), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Input KHz: " + std::to_string(c_InputAudio.u32_KHz), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Input Channels: " + std::to_string(c_InputAudio.u8_Channels), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Input Sample Frames: " + std::to_string(c_Config.GetPAMicFrameSamples()), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Input Recording Storage Seconds: " + std::to_string(c_Config.GetPAMicRecordingStorageS()), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Input Recording Storage Byte Size: " + std::to_string(c_InputAudio.us_BufferSize * sizeof(MRH_Sint16) * 2), "PADevice.cpp", __LINE__);
    
    // @NOTE: Do not start recording yet, wait for Record() to be called
}

void PADevice::CloseInput() noexcept
{
    CloseStream(p_InputStream);
}

void PADevice::FlipInputBuffer() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_InputAudio.c_Mutex);
    
    if (c_InputAudio.p_Buffer == c_InputAudio.p_BufferA)
    {
        c_InputAudio.p_Buffer = c_InputAudio.p_BufferB;
    }
    else
    {
        c_InputAudio.p_Buffer = c_InputAudio.p_BufferA;
    }
    
    c_InputAudio.us_BufferPos = 0;
}

void PADevice::Record()
{
    try
    {
        // Stop any playback
        StopStream(p_OutputStream);
        
        // Ready, start recording
        FlipInputBuffer();
        StartStream(p_InputStream);
    }
    catch (...)
    {
        throw;
    }
}

int PADevice::PAInputCallback(const void* p_Input,
                              void* p_Output,
                              unsigned long u32_FrameCount,
                              const PaStreamCallbackTimeInfo* p_TimeInfo,
                              PaStreamCallbackFlags e_StatusFlags,
                              void* p_UserData) noexcept
{
    PADevice::Input* p_Audio = static_cast<PADevice::Input*>(p_UserData);
    
    // Lock buffer for full operation
    std::lock_guard<std::mutex> c_Guard(p_Audio->c_Mutex);
    
    // Define vars first
    MRH_Sint16* p_Src = (MRH_Sint16*)p_Input;
    MRH_Sint16* p_Dst = &(p_Audio->p_Buffer[p_Audio->us_BufferPos]);
    MRH_Uint8 u8_Channels = p_Audio->u8_Channels;
    
    // Set bounds
    size_t us_Copy = p_Audio->us_BufferSize - p_Audio->us_BufferPos;
    
    if (us_Copy > u32_FrameCount)
    {
        us_Copy = u32_FrameCount;
    }
    
    // Now copy
    for (size_t i = 0; i < us_Copy; ++i)
    {
        // Remember last ABS
        MRH_Sint16 p_ABS[2] = { 0 };
        MRH_Sint16 s16_Peak = 0;
        
        for (MRH_Uint8 j = 0; j < u8_Channels; ++j)
        {
            // Grab strongest
            if (p_ABS[0] < (p_ABS[1] = abs(*p_Src)))
            {
                p_ABS[0] = p_ABS[1];
                s16_Peak = *p_Src;
            }
            
            ++p_Src;
        }
        
        *p_Dst = s16_Peak;
        ++p_Dst;
    }
    
    // Warn if we had to drop frames
    if (us_Copy < u32_FrameCount)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Input samples dropped, storage full!",
                                       "PADevice.cpp", __LINE__);
    }
    
    // Set next pos
    p_Audio->us_BufferPos += us_Copy;
    
    // We always continue
    return paContinue;
}

//*************************************************************************************
// Output
//*************************************************************************************

void PADevice::SetupOutput()
{
    Configuration& c_Config = Configuration::Singleton();
    PaError i_Error;
    
    // Grab device info
    MRH_Uint32 u32_DevID = c_Config.GetPASpeakerDeviceID();
    const PaDeviceInfo* p_DevInfo;
    
    if ((p_DevInfo = Pa_GetDeviceInfo(u32_DevID)) == NULL)
    {
        throw Exception("Failed to get device info for output device " + std::to_string(u32_DevID));
    }
    
    // Set input stream info
    PaStreamParameters c_OutputParameters;
    
    bzero(&c_OutputParameters, sizeof(c_OutputParameters));
    c_OutputParameters.channelCount = p_DevInfo->maxOutputChannels;
    c_OutputParameters.device = u32_DevID;
    c_OutputParameters.hostApiSpecificStreamInfo = NULL;
    c_OutputParameters.sampleFormat = paInt16;
    c_OutputParameters.suggestedLatency = p_DevInfo->defaultLowInputLatency;
    
    MRH_Uint32 u32_KHz = c_Config.GetPASpeakerKHz();
    
    // Open audio stream
    i_Error = Pa_OpenStream(&p_OutputStream,
                            NULL, // No input, output only
                            &c_OutputParameters,
                            u32_KHz,
                            c_Config.GetPASpeakerFrameSamples(),
                            paClipOff,// paNoFlag,
                            PAOutputCallback,
                            &c_OutputAudio);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to open PortAudio output stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
    
    // Update audio info
    c_OutputAudio.u32_KHz = u32_KHz;
    c_OutputAudio.u8_Channels = p_DevInfo->maxOutputChannels;
    c_OutputAudio.us_BufferPos = 0;
    
    // Print info
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    c_Logger.Log(MRH_PSBLogger::INFO, "Output Device: " + std::string(p_DevInfo->name), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Output KHz: " + std::to_string(c_OutputAudio.u32_KHz), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Output Channels: " + std::to_string(c_OutputAudio.u8_Channels), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Output Sample Frames: " + std::to_string(c_Config.GetPASpeakerFrameSamples()), "PADevice.cpp", __LINE__);
    
    // @NOTE: Only start on available audio!
}

void PADevice::CloseOutput() noexcept
{
    CloseStream(p_OutputStream);
}

void PADevice::Playback()
{
    if (c_OutputAudio.v_Buffer.size() == c_OutputAudio.us_BufferPos)
    {
        throw Exception("No audio for playback!");
    }
    
    try
    {
        // Stop listening first
        StopStream(p_InputStream);
        
        // Now start playback stream
        StartStream(p_OutputStream);
    }
    catch (...)
    {
        throw;
    }
}

int PADevice::PAOutputCallback(const void* p_Input,
                               void* p_Output,
                               unsigned long u32_FrameCount,
                               const PaStreamCallbackTimeInfo* p_TimeInfo,
                               PaStreamCallbackFlags e_StatusFlags,
                               void* p_UserData) noexcept
{
    PADevice::Output* p_Audio = static_cast<PADevice::Output*>(p_UserData);
    
    // Lock
    std::lock_guard<std::mutex> c_Guard(p_Audio->c_Mutex);
    
    // Define vars first
    MRH_Sint16* p_Src = (MRH_Sint16*)&(p_Audio->v_Buffer[p_Audio->us_BufferPos]);
    MRH_Sint16* p_Dst = (MRH_Sint16*)p_Output;
    MRH_Uint8 u8_Channels = p_Audio->u8_Channels;
    
    // Set bounds
    size_t us_Copy = p_Audio->v_Buffer.size() - p_Audio->us_BufferPos;
    
    if (us_Copy > u32_FrameCount)
    {
        us_Copy = u32_FrameCount;
    }
     
    // Copy audio data
    for (size_t i = 0; i < us_Copy; ++i)
    {
        for (MRH_Uint8 j = 0; j < u8_Channels; ++j)
        {
            *p_Dst = *p_Src;
            ++p_Dst;
        }
        
        ++p_Src;
    }
    
    // Zero missing
    if (us_Copy < u32_FrameCount)
    {
        std::memset(p_Dst,
                    0,
                    (u32_FrameCount - us_Copy) * sizeof(MRH_Sint16) * u8_Channels); // Bytes * Format * Channel
    }
    
    // Check copy result
    if ((p_Audio->us_BufferPos += us_Copy) >= p_Audio->v_Buffer.size())
    {
        return paComplete;
    }
    
    return paContinue;
}

//*************************************************************************************
// Getters
//*************************************************************************************

VoiceAudio PADevice::GetRecordedAudio() noexcept
{
    // Grab current used buffer
    MRH_Sint16* p_Buffer = c_InputAudio.p_Buffer;
    size_t us_Length = c_InputAudio.us_BufferPos;
    
    // Set next buffer
    FlipInputBuffer();
    
    // Create audio sample
    return VoiceAudio(p_Buffer,
                      us_Length,
                      c_InputAudio.u32_KHz);
}

bool PADevice::GetPlayback() noexcept
{
    return Pa_IsStreamActive(p_OutputStream) == 1 ? true : false;
}

bool PADevice::GetRecording() noexcept
{
    return Pa_IsStreamActive(p_InputStream) == 1 ? true : false;
}

//*************************************************************************************
// Setters
//*************************************************************************************

void PADevice::SetPlaybackAudio(VoiceAudio const& c_Audio)
{
    if (GetPlayback() == true)
    {
        throw Exception("Playback in progress!");
    }
    else if (c_Audio.v_Buffer.size() == 0)
    {
        throw Exception("Recieved empty playback buffer!");
    }
    
    std::lock_guard<std::mutex> c_Guard(c_OutputAudio.c_Mutex);
    
    // Copy to buffer
    if (c_OutputAudio.u32_KHz != c_Audio.u32_KHz)
    {
        // Difference, we need to convert
        try
        {
            c_Converter.Reset(); // New audio
            c_OutputAudio.v_Buffer = c_Converter.Convert(c_Audio.v_Buffer,
                                                         c_Audio.u32_KHz);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                           "PADevice.cpp", __LINE__);
            return;
        }
    }
    else
    {
        // Same KHz, simply copy
        c_OutputAudio.v_Buffer = c_Audio.v_Buffer;
    }
    
    // Reset output pos for next playback
    c_OutputAudio.us_BufferPos = 0;
}
