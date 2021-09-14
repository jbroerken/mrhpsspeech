/**
 *  PAMicrophone.cpp
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
#include "./PAMicrophone.h"
#include "../../../Configuration.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

std::atomic<int> PAMicrophone::i_PAUsers(0);

PAMicrophone::PAMicrophone()
{
    Configuration& c_Configuration = Configuration::Singleton();
    PaError i_Error;
    
    // Setup PortAudio
    if (i_PAUsers == 0)
    {
        if ((i_Error = Pa_Initialize()) != paNoError)
        {
            throw Exception("Failed to initialuze PortAudio! " + std::string(Pa_GetErrorText(i_Error)));
        }
    }
    
    i_PAUsers += 1;
    
    // Update audio info
    c_Audio.u32_KHz = c_Configuration.GetPAMicKHz();
    c_Audio.u8_Channels = c_Configuration.GetPAMicChannels();
    c_Audio.u32_FrameSamples = c_Configuration.GetPAMicFrameSamples();
    c_Audio.us_BufferSize = c_Audio.u8_Channels * c_Audio.u32_FrameSamples * c_Configuration.GetPAMicSampleStorageSize();
    c_Audio.p_BufferA = new MRH_Sint16[c_Audio.us_BufferSize];
    c_Audio.p_BufferB = new MRH_Sint16[c_Audio.us_BufferSize];
    c_Audio.p_Buffer = c_Audio.p_BufferA; // Start at A
    c_Audio.us_BufferPos = 0;
    
    // Set input stream info
    PaStreamParameters c_InputParameters;
    MRH_Uint32 u32_DevID = c_Configuration.GetPAMicDeviceID();
    
    bzero(&c_InputParameters, sizeof(c_InputParameters));
    c_InputParameters.channelCount = c_Audio.u8_Channels;
    c_InputParameters.device = u32_DevID;
    c_InputParameters.hostApiSpecificStreamInfo = NULL;
    c_InputParameters.sampleFormat = paInt16;
    c_InputParameters.suggestedLatency = Pa_GetDeviceInfo(u32_DevID)->defaultLowInputLatency;
    c_InputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
    
    // Print info
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    c_Logger.Log(MRH_PSBLogger::INFO, "Device: " + std::string(Pa_GetDeviceInfo(u32_DevID)->name), "PAMicrohpone.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "KHz: " + std::to_string(c_Audio.u32_KHz), "PAMicrohpone.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Channels: " + std::to_string(c_Audio.u8_Channels), "PAMicrohpone.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Frame Samples: " + std::to_string(c_Audio.u32_FrameSamples), "PAMicrohpone.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Storage Size: " + std::to_string(c_Configuration.GetPAMicSampleStorageSize()), "PAMicrohpone.cpp", __LINE__);
    c_Logger.Log(MRH_PSBLogger::INFO, "Total Storage Byte Size: " + std::to_string(c_Audio.us_BufferSize * 2), "PAMicrohpone.cpp", __LINE__);
    
    // Open audio stream
    i_Error = Pa_OpenStream(&p_Stream,
                            &c_InputParameters,
                            NULL, // No output info
                            c_Audio.u32_KHz,
                            c_Audio.u32_FrameSamples,
                            paClipOff,// paNoFlag,
                            PACallback,
                            &c_Audio);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to open PortAudio input stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
    
    // Stream opened, start recording
    if ((i_Error = Pa_StartStream(p_Stream)) != paNoError)
    {
        throw Exception("Failed to start PortAudio input stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

PAMicrophone::~PAMicrophone() noexcept
{
    PaError i_Error;
    
    if ((i_Error = Pa_StopStream(p_Stream)) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to stop PortAudio Stream! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PAMicrophone.cpp", __LINE__);
    }
    
    if ((i_Error = Pa_CloseStream(p_Stream)) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to close PortAudio Stream! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PAMicrophone.cpp", __LINE__);
    }
    
    i_PAUsers -= 1;
    
    if (i_PAUsers == 0 && (i_Error = Pa_Terminate()) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to deinitialize PortAudio! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PAMicrophone.cpp", __LINE__);
    }
}

PAMicrophone::Audio::Audio() noexcept : p_Buffer(NULL),
                                        p_BufferA(NULL),
                                        p_BufferB(NULL),
                                        us_BufferSize(0),
                                        us_BufferPos(0),
                                        u32_KHz(0),
                                        u8_Channels(0)
{}

PAMicrophone::Audio::~Audio() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void PAMicrophone::StartListening()
{
    PaError i_Error = Pa_StartStream(p_Stream);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to start PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

void PAMicrophone::StopListening()
{
    PaError i_Error = Pa_StopStream(p_Stream);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to stop PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

int PAMicrophone::PACallback(const void* p_Input,
                             void* p_Output,
                             unsigned long u32_FrameCount,
                             const PaStreamCallbackTimeInfo* p_TimeInfo,
                             PaStreamCallbackFlags e_StatusFlags,
                             void* p_UserData) noexcept
{
    PAMicrophone::Audio* p_Audio = static_cast<PAMicrophone::Audio*>(p_UserData);
    
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
// Getters
//*************************************************************************************

VoiceAudio PAMicrophone::GetVoiceAudio() noexcept
{
    MRH_Sint16* p_Buffer;
    size_t us_Length;
    
    c_Audio.c_Mutex.lock();
    
    // Grab current used buffer
    p_Buffer = c_Audio.p_Buffer;
    us_Length = c_Audio.us_BufferPos;
    
    // Set next buffer
    c_Audio.p_Buffer = (p_Buffer == c_Audio.p_BufferA ? c_Audio.p_BufferB : c_Audio.p_BufferA);
    c_Audio.us_BufferPos = 0;
    
    c_Audio.c_Mutex.unlock();
    
    // Create audio sample
    return VoiceAudio(p_Buffer,
                      us_Length,
                      c_Audio.u32_KHz,
                      c_Audio.u8_Channels,
                      c_Audio.u32_FrameSamples);
}
