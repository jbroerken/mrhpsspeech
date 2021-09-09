/**
 *  ReadAudio.cpp
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
#include <SDL2/SDL.h>

// Project
#include "./ReadAudio.h"
#include "../../../../Configuration.h"

// Pre-defined
#ifndef MRH_SPEECH_SPHINX_TRIGGER_PATH
    #define MRH_SPEECH_SPHINX_TRIGGER_PATH "/var/mrh/mrhpsspeech/sphinx/trigger/"
#endif
#define MRH_SPEECH_SPHINX_TRIGGER_LM_EXT ".lm.bin"
#define MRH_SPEECH_SPHINX_TRIGGER_DICT_EXT ".dict"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

ReadAudio::ReadAudio() noexcept : p_Decoder(NULL),
                                  p_Config(NULL),
                                  u32_DevID(0)
{}

ReadAudio::~ReadAudio() noexcept
{
    if (p_Decoder != NULL)
    {
        ps_free(p_Decoder);
    }
    
    if (p_Config != NULL)
    {
        cmd_ln_free_r(p_Config);
    }
}

ReadAudio::Stream::Stream() noexcept : u16_Format(AUDIO_F32),
                                       u32_KHz(44100),
                                       u8_Channels(2),
                                       u32_StreamLengthS(5)
{}

ReadAudio::Stream::~Stream() noexcept
{
    for (auto& Sample : v_Sample)
    {
        delete Sample;
    }
}

ReadAudio::Sample::Sample() noexcept : u16_Format(AUDIO_F32),
                                       u32_KHz(44100),
                                       u8_Channels(2),
                                       f32_Amplitude(0.f),
                                       f32_Peak(0.f),
                                       u64_TimepointS(0)
{}

ReadAudio::Sample::~Sample() noexcept
{}

//*************************************************************************************
// Setup
//*************************************************************************************

void ReadAudio::Setup(std::string const& s_Locale)
{
    // Already setup?
    if (u32_DevID != 0)
    {
        return;
    }
    
    Configuration& c_Configuration = Configuration::Singleton();
    
    // Update stream info
    c_Stream.u16_Format = AUDIO_F32;
    c_Stream.u32_KHz = c_Configuration.GetListenKHz();
    c_Stream.u8_Channels = c_Configuration.GetListenChannels();
    c_Stream.u32_StreamLengthS = c_Configuration.GetListenStreamLengthS();
    
    // Define audio input format
    SDL_AudioSpec c_Want;
    SDL_AudioSpec c_Have;

    SDL_zero(c_Want);
    
    c_Want.freq = c_Configuration.GetListenKHz();
    c_Want.format = AUDIO_F32;
    c_Want.channels = c_Configuration.GetListenChannels();
    c_Want.samples = c_Configuration.GetListenSamples();
    c_Want.callback = &SDLCallback;
    c_Want.userdata = &c_Stream;
    
    // Open device
    if (SDL_GetNumAudioDevices(1) == 0)
    {
        throw Exception("No audio recording devices!");
    }
    
    std::string s_DevName = SDL_GetAudioDeviceName(c_Configuration.GetListenDeviceID(), 1);
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Using audio recording device: " + s_DevName,
                                   "ReadAudio.cpp", __LINE__);
    
    if ((u32_DevID = SDL_OpenAudioDevice(s_DevName.c_str(), 1, &c_Want, &c_Have, 0)) == 0)
    {
        throw Exception("Failed to open audio recording device " + std::to_string(c_Configuration.GetListenDeviceID()) + "!");
    }
    
    if (c_Want.format != c_Have.format ||
        c_Want.freq != c_Have.freq ||
        c_Want.channels != c_Have.channels)
    {
        SDL_CloseAudioDevice(u32_DevID);
        u32_DevID = 0;
        
        throw Exception("Unusbale format for audio recording device " + std::to_string(c_Configuration.GetListenDeviceID()) + "!");
    }
    
    SDL_PauseAudioDevice(u32_DevID, 0);
    return;
    
    // Setup sphinx
    std::string s_HMM = MRH_SPEECH_SPHINX_TRIGGER_PATH + s_Locale + "/" + s_Locale;
    std::string s_LM = MRH_SPEECH_SPHINX_TRIGGER_PATH + s_Locale + "/" + s_Locale + MRH_SPEECH_SPHINX_TRIGGER_LM_EXT;
    std::string s_Dict = MRH_SPEECH_SPHINX_TRIGGER_PATH + s_Locale + "/" + s_Locale + MRH_SPEECH_SPHINX_TRIGGER_DICT_EXT;
    
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Sphinx HMM: " + s_HMM,
                                   "ReadAudio.cpp", __LINE__);
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Sphinx LM: " + s_LM,
                                   "ReadAudio.cpp", __LINE__);
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Sphinx Dict: " + s_Dict,
                                   "ReadAudio.cpp", __LINE__);

    if ((p_Config = cmd_ln_init(NULL, ps_args(), TRUE,
                                "-hmm", s_HMM.c_str(),
                                "-lm", s_LM.c_str(),
                                "-dict", s_Dict.c_str(),
                                NULL)) == NULL)
    {
        SDL_CloseAudioDevice(u32_DevID);
        u32_DevID = 0;
        
        throw Exception("Failed to create sphinx config object for ReadAudio!");
    }
    
    if ((p_Decoder = ps_init(p_Config)) == NULL)
    {
        SDL_CloseAudioDevice(u32_DevID);
        u32_DevID = 0;
        
        cmd_ln_free_r(p_Config);
        p_Config = NULL;
        
        throw Exception("Failed to create sphinx recognizer for ReadAudio!");
    }
    
    // Start
    SDL_PauseAudioDevice(u32_DevID, 0);
}

//*************************************************************************************
// Update
//*************************************************************************************

void ReadAudio::PauseListening() noexcept
{
    SDL_PauseAudioDevice(u32_DevID, 1);
}

void ReadAudio::ResumeListening() noexcept
{
    SDL_PauseAudioDevice(u32_DevID, 0);
}

void ReadAudio::SDLCallback(void* p_UserData, Uint8* p_Buffer, int i_Length) noexcept
{
    ReadAudio::Stream* p_Stream = static_cast<ReadAudio::Stream*>(p_UserData);
    
    // Grab the sample to use
    Sample* p_Sample;
    Uint64 u64_TimepointS = time(NULL);
    
    if (p_Stream->v_Sample.size() > 0 &&
        p_Stream->v_Sample[0]->u64_TimepointS < (u64_TimepointS - p_Stream->u32_StreamLengthS))
    {
        // Oldest sample too old, reuse
        p_Sample = p_Stream->v_Sample[0];
        p_Stream->v_Sample.erase(p_Stream->v_Sample.begin());
    }
    else
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
    //printf("Amplitude: %f | Peak: %f | Total Samples: %zu\n", p_Sample->f32_Amplitude, p_Sample->f32_Peak, p_Stream->v_Sample.size());
    p_Stream->c_Mutex.unlock();
}

//*************************************************************************************
// Sample
//*************************************************************************************

void ReadAudio::ConvertTo(Sample* p_Sample, SDL_AudioFormat u16_Format, Uint32 u32_KHz, Uint8 u8_Channels) noexcept
{
    SDL_AudioCVT c_CVT;
    SDL_BuildAudioCVT(&c_CVT,
                      p_Sample->u16_Format, p_Sample->u8_Channels, p_Sample->u32_KHz,
                      u16_Format, u8_Channels, u32_KHz);
    
    // Prepare buffer for conversion
    c_CVT.len = static_cast<int>(p_Sample->v_Buffer.size());
    c_CVT.buf = &(p_Sample->v_Buffer[0]);
    
    p_Sample->v_Buffer.resize(c_CVT.len * c_CVT.len_mult, 0);
    
    // Convert
    SDL_ConvertAudio(&c_CVT);
    
    p_Sample->v_Buffer.resize(c_CVT.len_cvt);
}
