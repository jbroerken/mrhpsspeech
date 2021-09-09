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

bool PAMicrophone::b_SetupPA = true;

PAMicrophone::PAMicrophone()
{
    Configuration& c_Configuration = Configuration::Singleton();
    PaError i_Error;
    
    // Setup PortAudio
    if (b_SetupPA == true)
    {
        if ((i_Error = Pa_Initialize()) != paNoError)
        {
            throw Exception("Failed to initialuze PortAudio! " + std::string(Pa_GetErrorText(i_Error)));
        }
        
        b_SetupPA = true;
    }
    
    // Update audio info
    c_Audio.u16_Format = paInt16;
    c_Audio.u32_KHz = c_Configuration.GetPAMicKHz();
    c_Audio.u8_Channels = c_Configuration.GetPAMicChannels();
    c_Audio.u32_StreamLengthS = c_Configuration.GetPAMicStreamLengthS();
    
    // Open audio stream
    i_Error = Pa_OpenDefaultStream(&p_Stream,
                                   c_Audio.u8_Channels,
                                   0,
                                   paInt16,
                                   c_Audio.u32_KHz,
                                   c_Configuration.GetPAMicSamples(),
                                   PACallback,
                                   &c_Audio);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to open PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
    
    // Stream opened, start recording
    if ((i_Error = Pa_StartStream(p_Stream)) != paNoError)
    {
        throw Exception("Failed to start PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
    
    /*
    // Define audio input format
    SDL_AudioSpec c_Want;
    SDL_AudioSpec c_Have;

    SDL_zero(c_Want);
    
    c_Want.freq = c_Stream.u32_KHz;
    c_Want.format = c_Stream.u16_Format;
    c_Want.channels = c_Stream.u8_Channels;
    c_Want.samples = c_Configuration.GetPAMicSamples();
    c_Want.callback = &SDLCallback;
    c_Want.userdata = &c_Stream;
    
    // Open device
    if (SDL_GetNumAudioDevices(1) == 0)
    {
        throw Exception("No audio recording devices!");
    }
    
    std::string s_DevName = SDL_GetAudioDeviceName(c_Configuration.GetSDLMicDeviceID(), 1);
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Using audio recording device: " + s_DevName,
                                   "SDLMicrophone.cpp", __LINE__);
    
    if ((u32_DevID = SDL_OpenAudioDevice(s_DevName.c_str(), 1, &c_Want, &c_Have, 0)) == 0)
    {
        throw Exception("Failed to open audio recording device " + std::to_string(c_Configuration.GetSDLMicDeviceID()) + "!");
    }
    
    if (c_Want.format != c_Have.format ||
        c_Want.freq != c_Have.freq ||
        c_Want.channels != c_Have.channels)
    {
        SDL_CloseAudioDevice(u32_DevID);
        u32_DevID = 0;
        
        throw Exception("Unusbale format for audio recording device " + std::to_string(c_Configuration.GetSDLMicDeviceID()) + "!");
    }
    
    SDL_PauseAudioDevice(u32_DevID, 0);
    */
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
    else if ((i_Error = Pa_CloseStream(p_Stream)) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to close PortAudio Stream! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PAMicrophone.cpp", __LINE__);
    }
    else if ((i_Error = Pa_Terminate()) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to deinitialize PortAudio! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PAMicrophone.cpp", __LINE__);
    }
}

PAMicrophone::Audio::Audio() noexcept : u16_Format(paInt16),
                                        u32_KHz(16000),
                                        u8_Channels(2),
                                        u32_StreamLengthS(5)
{}

PAMicrophone::Audio::~Audio() noexcept
{
    for (auto& Sample : v_Sample)
    {
        delete Sample;
    }
}

PAMicrophone::Sample::Sample() noexcept : u16_Format(paInt16),
                                          u32_KHz(16000),
                                          u8_Channels(2),
                                          f32_Amplitude(0.f),
                                          f32_Peak(0.f),
                                          u64_TimepointS(0)
{}

PAMicrophone::Sample::~Sample() noexcept
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

/*
void PAMicrophone::PACallback(void* p_UserData, Uint8* p_Buffer, int i_Length) noexcept
{
    PAMicrophone::Stream* p_Stream = static_cast<PAMicrophone::Stream*>(p_UserData);
    
    // Grab the sample to use
    Sample* p_Sample = NULL;
    Uint64 u64_TimepointS = time(NULL);
    
    p_Stream->c_Mutex.lock();
    
    if (p_Stream->v_Sample.size() > 0 &&
        p_Stream->v_Sample[0]->u64_TimepointS < (u64_TimepointS - p_Stream->u32_StreamLengthS))
    {
        // Oldest sample too old, reuse
        p_Sample = p_Stream->v_Sample[0];
        p_Stream->v_Sample.erase(p_Stream->v_Sample.begin());
    }
    
    p_Stream->c_Mutex.unlock();
    
    if (p_Sample == NULL)
    {
        // All samples valid, append new
        p_Sample = new Sample();
    }
    
    p_Sample->u64_TimepointS = u64_TimepointS; // New timepoint
    
    // Sample inherits stream format
    p_Sample->u16_Format = p_Stream->u16_Format;
    p_Sample->u32_KHz = p_Stream->u32_KHz;
    p_Sample->u8_Channels = p_Stream->u8_Channels;
    
    // Add space for all bytes
    if (p_Sample->v_Buffer.size() != i_Length)
    {
        p_Sample->v_Buffer.resize(i_Length, 0);
    }
    
    // Channel RMS
    std::vector<float> v_RMS(p_Sample->u8_Channels, 0.f);
    std::vector<float>::iterator RMS = v_RMS.begin();
    
    // Run variables
    float f32_Value;
    float f32_ABS;
    size_t us_Step = sizeof(float);

    for (size_t i = 0; i < i_Length; i += us_Step)
    {
        // Get float value first
        f32_Value = *((float*)&(p_Buffer[i]));
        
        // Check the current peak (Global for all channels)
        f32_ABS = fabs(f32_Value);
        
        if (f32_ABS > p_Sample->f32_Peak)
        {
            p_Sample->f32_Peak = f32_ABS;
        }
        
        // Grab RMS for this channel
        *RMS += f32_Value * f32_Value;
        
        if ((++RMS) == v_RMS.end())
        {
            RMS = v_RMS.begin();
        }
        
        // Copy bytes
        *((float*)&(p_Sample->v_Buffer[i])) = f32_Value;
    }
    
    // Grab max amplitude
    for (auto& Amplitude : v_RMS)
    {
        if (p_Sample->f32_Amplitude < Amplitude)
        {
            p_Sample->f32_Amplitude = Amplitude;
        }
    }
    
    p_Sample->f32_Amplitude = (sqrt(p_Sample->f32_Amplitude / ((i_Length / v_RMS.size()) / us_Step)));
    
    // Sample was built, add!
    p_Stream->c_Mutex.lock();
    p_Stream->v_Sample.emplace_back(p_Sample);
    printf("Amplitude: %f | Peak: %f | Total Samples: %zu\n", p_Sample->f32_Amplitude, p_Sample->f32_Peak, p_Stream->v_Sample.size());
    p_Stream->c_Mutex.unlock();
}
*/

int PAMicrophone::PACallback(const void* p_Input,
                             void* p_Output,
                             unsigned long u32_FrameCount,
                             const PaStreamCallbackTimeInfo* p_TimeInfo,
                             PaStreamCallbackFlags e_StatusFlags,
                             void* p_UserData) noexcept
{
    return -1;
}

//*************************************************************************************
// Sample
//*************************************************************************************

void PAMicrophone::ConvertTo(Sample* p_Sample, PaSampleFormat u16_Format, MRH_Uint32 u32_KHz, MRH_Uint8 u8_Channels) noexcept
{
    // TODO: Convert
}

size_t PAMicrophone::GetSampleCount() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Audio.c_Mutex);
    return c_Audio.v_Sample.size();
}

PAMicrophone::Sample* PAMicrophone::GrabSample(size_t us_Sample)
{
    std::lock_guard<std::mutex> c_Guard(c_Audio.c_Mutex);
    
    std::vector<Sample*>& v_Sample = c_Audio.v_Sample;
    
    if (v_Sample.size() <= us_Sample)
    {
        throw Exception("Invalid sample requested!");
    }
    
    Sample* p_Sample = v_Sample[us_Sample];
    v_Sample.erase(v_Sample.begin() + us_Sample);
    
    return p_Sample;
}

void PAMicrophone::ReturnSample(Sample* p_Sample)
{
    std::lock_guard<std::mutex> c_Guard(c_Audio.c_Mutex);
    
    auto It = c_Audio.v_Sample.begin();
    auto End = c_Audio.v_Sample.end();
    
    for (; It != End; ++It)
    {
        if ((*It)->u64_TimepointS >= p_Sample->u64_TimepointS)
        {
            c_Audio.v_Sample.insert(It, p_Sample);
            return;
        }
    }
    
    // Newest, add at end
    c_Audio.v_Sample.emplace_back(p_Sample);
}
