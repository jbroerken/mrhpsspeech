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

PADevice::PADevice()
{
    // Init methods
    try
    {
        SetupInput();
        //SetupOutput();
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
    // Close streams first
    CloseInput();
    //CloseOutput();
}

PADevice::Input::Input() noexcept : p_Buffer(NULL),
                                    p_BufferA(NULL),
                                    p_BufferB(NULL),
                                    us_BufferSize(0),
                                    us_BufferPos(0),
                                    u32_KHz(0),
                                    u8_Channels(0)
{}

PADevice::Input::~Input() noexcept
{}

//*************************************************************************************
// Input
//*************************************************************************************

void PADevice::SetupInput()
{
    Configuration& c_Config = Configuration::Singleton();
    PaError i_Error;
    
    // Update audio info
    c_InputAudio.u32_KHz = c_Config.GetPAMicKHz();
    c_InputAudio.u8_Channels = c_Config.GetPAMicChannels();
    c_InputAudio.u32_FrameSamples = c_Config.GetPAMicFrameSamples();
    c_InputAudio.us_BufferSize = c_InputAudio.u8_Channels * c_InputAudio.u32_FrameSamples * c_Config.GetPAMicSampleStorageSize();
    c_InputAudio.p_BufferA = new MRH_Sint16[c_InputAudio.us_BufferSize];
    c_InputAudio.p_BufferB = new MRH_Sint16[c_InputAudio.us_BufferSize];
    c_InputAudio.p_Buffer = c_InputAudio.p_BufferA; // Start at A
    c_InputAudio.us_BufferPos = 0;
    
    // Set input stream info
    PaStreamParameters c_InputParameters;
    MRH_Uint32 u32_DevID = c_Config.GetPAMicDeviceID();
    
    bzero(&c_InputParameters, sizeof(c_InputParameters));
    c_InputParameters.channelCount = c_InputAudio.u8_Channels;
    c_InputParameters.device = u32_DevID;
    c_InputParameters.hostApiSpecificStreamInfo = NULL;
    c_InputParameters.sampleFormat = paInt16;
    c_InputParameters.suggestedLatency = Pa_GetDeviceInfo(u32_DevID)->defaultLowInputLatency;
    c_InputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
    
    // Print info
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    c_Logger.Log(MRH_PSBLogger::INFO, "Device: " + std::string(Pa_GetDeviceInfo(u32_DevID)->name), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "KHz: " + std::to_string(c_InputAudio.u32_KHz), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Channels: " + std::to_string(c_InputAudio.u8_Channels), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Frame Samples: " + std::to_string(c_InputAudio.u32_FrameSamples), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Storage Size: " + std::to_string(c_Config.GetPAMicSampleStorageSize()), "PADevice.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Total Storage Byte Size: " + std::to_string(c_InputAudio.us_BufferSize * 2), "PADevice.cpp", __LINE__);
    
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
    
    // Stream opened, start recording
    if ((i_Error = Pa_StartStream(p_InputStream)) != paNoError)
    {
        throw Exception("Failed to start PortAudio input stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

void PADevice::CloseInput() noexcept
{
    PaError i_Error;
    
    if ((i_Error = Pa_StopStream(p_InputStream)) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to stop PortAudio input tream! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PADevice.cpp", __LINE__);
    }
    
    if ((i_Error = Pa_CloseStream(p_InputStream)) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to close PortAudio input tream! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PADevice.cpp", __LINE__);
    }
}

void PADevice::StartListening()
{
    PaError i_Error = Pa_IsStreamActive(p_InputStream);
    
    if (i_Error == 1) // Already active
    {
        return;
    }
    else if (i_Error < 0)
    {
        throw Exception("Failed to get PortAudio input stream state! " + std::string(Pa_GetErrorText(i_Error)));
    }
    else if ((i_Error = Pa_StartStream(p_InputStream)) != paNoError)
    {
        throw Exception("Failed to start PortAudio input stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

void PADevice::StopListening()
{
    PaError i_Error = Pa_IsStreamActive(p_InputStream);
    
    if (i_Error == 0) // Already stopped
    {
        return;
    }
    else if (i_Error < 0)
    {
        throw Exception("Failed to get PortAudio input stream state! " + std::string(Pa_GetErrorText(i_Error)));
    }
    else if ((i_Error = Pa_StopStream(p_InputStream)) != paNoError)
    {
        throw Exception("Failed to stop PortAudio input stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

void PADevice::ResetInputAudio()
{
    // Simply swap buffers, causing a override
    c_InputAudio.c_Mutex.lock();
    
    c_InputAudio.p_Buffer = (c_InputAudio.p_Buffer == c_InputAudio.p_BufferA ? c_InputAudio.p_BufferB : c_InputAudio.p_BufferA);
    c_InputAudio.us_BufferPos = 0;
    
    c_InputAudio.c_Mutex.unlock();
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
    std::memcpy(p_Buffer + us_BufferPos, // Current position, continuous
                p_Input,
                ss_Copy * sizeof(MRH_Sint16)); // sample for frame * bytes
    p_Audio->us_BufferPos += ss_Copy;
    
    // Done
    return paContinue;
}

//*************************************************************************************
// Output
//*************************************************************************************

// @TODO: Output

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
                      c_InputAudio.u32_KHz,
                      c_InputAudio.u8_Channels,
                      c_InputAudio.u32_FrameSamples);
}
