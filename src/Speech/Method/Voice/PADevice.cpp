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

// Pre-defined
#define PORTAUDIO_INPUT_CHANNELS 1 // Voice sound, simply use mono
#define PORTAUDIO_OUTPUT_CHANNELS 1


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
                                    u32_KHz(0)
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
                                      u32_KHz(0)
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
    c_InputParameters.channelCount = PORTAUDIO_INPUT_CHANNELS;
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
    c_InputAudio.us_BufferSize = u32_KHz * c_Config.GetPAMicRecordingStorageS();
    c_InputAudio.p_BufferA = new MRH_Sint16[c_InputAudio.us_BufferSize];
    c_InputAudio.p_BufferB = new MRH_Sint16[c_InputAudio.us_BufferSize];
    c_InputAudio.p_Buffer = c_InputAudio.p_BufferA; // Start at A
    c_InputAudio.us_BufferPos = 0;
    
    // Print info
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    c_Logger.Log(MRH_PSBLogger::INFO, "Input Device: " + std::string(p_DevInfo->name), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Input KHz: " + std::to_string(c_InputAudio.u32_KHz), "PADevice.cpp", __LINE__);
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
    
    MRH_Sint16* p_Buffer = p_Audio->p_Buffer;
    size_t us_BufferPos = p_Audio->us_BufferPos;
    ssize_t ss_Copy = p_Audio->us_BufferSize - us_BufferPos;
    
    // Enough space?
    if (ss_Copy > u32_FrameCount)
    {
        ss_Copy = u32_FrameCount;
    }
    else if (ss_Copy <= 0)
    {
        // Space constraints, keep until switch
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Input samples dropped, storage full!",
                                       "PADevice.cpp", __LINE__);
        return paContinue;
    }
    
    // Copy data
    std::memcpy(&(p_Buffer[us_BufferPos]), // Current position, continuous
                p_Input,
                ss_Copy * sizeof(MRH_Sint16)); // sample for frame * bytes
    p_Audio->us_BufferPos += ss_Copy;
    
    // Done
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
    c_OutputParameters.channelCount = PORTAUDIO_OUTPUT_CHANNELS;
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
    c_OutputAudio.us_BufferPos = 0;
    
    // Print info
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    c_Logger.Log(MRH_PSBLogger::INFO, "Output Device: " + std::string(p_DevInfo->name), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Output KHz: " + std::to_string(c_OutputAudio.u32_KHz), "PADevice.cpp", __LINE__);
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
    
    // Audio buffer copy size
    ssize_t ss_Copy = p_Audio->v_Buffer.size() - p_Audio->us_BufferPos;
    
    if (ss_Copy > u32_FrameCount)
    {
        ss_Copy = u32_FrameCount;
    }
    else if (ss_Copy > 0 && ss_Copy < u32_FrameCount)
    {
        // Zero missing
        std::memset(&(((MRH_Sint16*)p_Output)[ss_Copy]),
                    0,
                    (u32_FrameCount - ss_Copy) * sizeof(MRH_Sint16));
    }
    
    // Copy data
    std::memcpy(p_Output,
                &(p_Audio->v_Buffer[p_Audio->us_BufferPos]),
                ss_Copy * sizeof(MRH_Sint16)); // sample for frame * bytes
    
    // Check copy result
    if ((p_Audio->us_BufferPos += ss_Copy) >= p_Audio->v_Buffer.size())
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
