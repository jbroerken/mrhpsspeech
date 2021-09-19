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
                       p_OutputStream(NULL)
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
                                    u32_KHz(0),
                                    u32_FrameSamples(0)
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

PADevice::Output::Output() noexcept : b_Playback(false),
                                      us_BufferPos(0),
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
    PaError i_Error = Pa_IsStreamActive(p_Stream);
    
    if (i_Error == 0) // Already stopped
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
    
    // Update audio info
    c_InputAudio.u32_KHz = c_Config.GetPAMicKHz();
    c_InputAudio.u32_FrameSamples = c_Config.GetPAMicFrameSamples();
    c_InputAudio.us_BufferSize = c_InputAudio.u32_FrameSamples * c_Config.GetPAMicSampleStorageSize();
    c_InputAudio.p_BufferA = new MRH_Sint16[c_InputAudio.us_BufferSize];
    c_InputAudio.p_BufferB = new MRH_Sint16[c_InputAudio.us_BufferSize];
    c_InputAudio.p_Buffer = c_InputAudio.p_BufferA; // Start at A
    c_InputAudio.us_BufferPos = 0;
    
    // Set input stream info
    PaStreamParameters c_InputParameters;
    
    bzero(&c_InputParameters, sizeof(c_InputParameters));
    c_InputParameters.channelCount = PORTAUDIO_INPUT_CHANNELS;
    c_InputParameters.device = u32_DevID;
    c_InputParameters.hostApiSpecificStreamInfo = NULL;
    c_InputParameters.sampleFormat = paInt16;
    c_InputParameters.suggestedLatency = p_DevInfo->defaultLowInputLatency;
    
    // Print info
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    c_Logger.Log(MRH_PSBLogger::INFO, "Input Device: " + std::string(p_DevInfo->name), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Input KHz: " + std::to_string(c_InputAudio.u32_KHz), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Input Frame Samples: " + std::to_string(c_InputAudio.u32_FrameSamples), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Input Storage Size: " + std::to_string(c_Config.GetPAMicSampleStorageSize()), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Input Total Storage Byte Size: " + std::to_string(c_InputAudio.us_BufferSize * sizeof(MRH_Sint16) * 2), "PADevice.cpp", __LINE__);
    
    // Open audio stream
    i_Error = Pa_OpenStream(&p_InputStream,
                            &c_InputParameters,
                            NULL, // No output info
                            c_InputAudio.u32_KHz,
                            c_InputAudio.u32_FrameSamples,
                            paClipOff,// paNoFlag,
                            PAInputCallback,
                            &c_InputAudio);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to open PortAudio input stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
    
    // @NOTE: Do not start recording yet, wait for StartListening() to be called
}

void PADevice::CloseInput() noexcept
{
    CloseStream(p_InputStream);
}

void PADevice::StartListening()
{
    try
    {
        StartStream(p_InputStream);
    }
    catch (...)
    {
        throw;
    }
}

void PADevice::StopListening()
{
    try
    {
        StopStream(p_InputStream);
    }
    catch (...)
    {
        throw;
    }
}

void PADevice::ResetInputAudio()
{
    std::lock_guard<std::mutex> c_Guard(c_InputAudio.c_Mutex);
    c_InputAudio.us_BufferPos = 0;
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
    
    // Update audio info
    c_OutputAudio.u32_KHz = c_Config.GetPASpeakerKHz();
    c_OutputAudio.us_BufferPos = 0;
    
    // Set input stream info
    PaStreamParameters c_OutputParameters;
    
    bzero(&c_OutputParameters, sizeof(c_OutputParameters));
    c_OutputParameters.channelCount = PORTAUDIO_OUTPUT_CHANNELS;
    c_OutputParameters.device = u32_DevID;
    c_OutputParameters.hostApiSpecificStreamInfo = NULL;
    c_OutputParameters.sampleFormat = paInt16;
    c_OutputParameters.suggestedLatency = p_DevInfo->defaultLowInputLatency;
    
    // Print info
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    c_Logger.Log(MRH_PSBLogger::INFO, "Output Device: " + std::string(p_DevInfo->name), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Output KHz: " + std::to_string(c_OutputAudio.u32_KHz), "PADevice.cpp", __LINE__);
    
    // Open audio stream
    i_Error = Pa_OpenStream(&p_OutputStream,
                            NULL, // No input, output only
                            &c_OutputParameters,
                            c_OutputAudio.u32_KHz,
                            paFramesPerBufferUnspecified,
                            paClipOff,// paNoFlag,
                            PAOutputCallback,
                            &c_OutputAudio);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to open PortAudio output stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
    
    // @NOTE: Only start on available audio!
}

void PADevice::CloseOutput() noexcept
{
    CloseStream(p_OutputStream);
    c_OutputAudio.b_Playback = false;
}

void PADevice::StartPlayback()
{
    if (c_OutputAudio.v_Buffer.size() == c_OutputAudio.us_BufferPos)
    {
        throw Exception("No audio for playback!");
    }
    else if (c_OutputAudio.b_Playback == true)
    {
        return;
    }
    
    try
    {
        c_OutputAudio.b_Playback = true; // Do before first cb chance
        StartStream(p_OutputStream);
    }
    catch (...)
    {
        throw;
    }
}

void PADevice::StopPlayback()
{
    try
    {
        StopStream(p_OutputStream);
        c_OutputAudio.b_Playback = false;
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
    
    // Stopped?
    if (p_Audio->b_Playback == false)
    {
        return paComplete;
    }
    
    // Zero buffer for silence
    std::memset(p_Output, 0, u32_FrameCount * sizeof(MRH_Sint16));
    
    // Audio buffer copy size
    ssize_t ss_Copy = p_Audio->v_Buffer.size() - p_Audio->us_BufferPos;
    
    if (ss_Copy > u32_FrameCount)
    {
        ss_Copy = u32_FrameCount;
    }
    else if (ss_Copy <= 0)
    {
        p_Audio->b_Playback = false; // Nothing left, stop
        return paComplete;
    }
    
    // Copy data
    std::memcpy(p_Output,
                &(p_Audio->v_Buffer[p_Audio->us_BufferPos]),
                ss_Copy * sizeof(MRH_Sint16)); // sample for frame * bytes
    
    // Check copy result
    if ((p_Audio->us_BufferPos += ss_Copy) == p_Audio->v_Buffer.size())
    {
        p_Audio->b_Playback = true;
        return paComplete;
    }
    
    return paContinue;
}

//*************************************************************************************
// Getters
//*************************************************************************************

VoiceAudio PADevice::GetInputAudio() noexcept
{
    MRH_Sint16* p_Buffer;
    size_t us_Length;
    
    c_InputAudio.c_Mutex.lock();
    
    // Grab current used buffer
    p_Buffer = c_InputAudio.p_Buffer;
    us_Length = c_InputAudio.us_BufferPos;
    
    // Set next buffer
    c_InputAudio.p_Buffer = (p_Buffer == c_InputAudio.p_BufferA ? c_InputAudio.p_BufferB : c_InputAudio.p_BufferA);
    c_InputAudio.us_BufferPos = 0;
    
    c_InputAudio.c_Mutex.unlock();
    
    // Create audio sample
    return VoiceAudio(p_Buffer,
                      us_Length,
                      c_InputAudio.u32_KHz);
}

bool PADevice::GetOutputPlayback() noexcept
{
    return c_OutputAudio.b_Playback;
}

bool PADevice::GetInputRecording() noexcept
{
    return Pa_IsStreamActive(p_InputStream) == 1 ? true : false;
}

//*************************************************************************************
// Setters
//*************************************************************************************

void PADevice::SetOutputAudio(VoiceAudio const& c_Audio)
{
    if (c_OutputAudio.b_Playback == true)
    {
        throw Exception("Currently playing audio!");
    }
    
    // Copy to buffer
    if (c_OutputAudio.u32_KHz != c_Audio.u32_KHz)
    {
        // Difference, we need to convert
        try
        {
            c_OutputAudio.v_Buffer = c_Audio.Convert(0,
                                                     c_Audio.v_Buffer.size(),
                                                     c_OutputAudio.u32_KHz);
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
