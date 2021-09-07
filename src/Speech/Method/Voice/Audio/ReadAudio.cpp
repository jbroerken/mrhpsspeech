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
    
    // Define audio input format for sphinx
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
    
    std::string s_DevName = SDL_GetAudioDeviceName(c_Configuration.GetDeviceInID(), 1);
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Using audio recording device: " + s_DevName,
                                   "ReadAudio.cpp", __LINE__);
    
    if ((u32_DevID = SDL_OpenAudioDevice(s_DevName.c_str(), 1, &c_Want, &c_Have, 0)) == 0)
    {
        throw Exception("Failed to open audio recording device " + std::to_string(c_Configuration.GetDeviceInID()) + "!");
    }
    
    if (c_Want.format != c_Have.format ||
        c_Want.freq != c_Have.freq ||
        c_Want.channels != c_Have.channels)
    {
        SDL_CloseAudioDevice(u32_DevID);
        u32_DevID = 0;
        
        throw Exception("Unusbale format for audio recording device " + std::to_string(c_Configuration.GetDeviceInID()) + "!");
    }
    
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

void ReadAudio::SDLCallback(void* p_UserData, Uint8* p_Stream, int i_Length) noexcept
{
    AudioStream* p_Audio = static_cast<AudioStream*>(p_UserData);
    
    p_Audio->c_Mutex.lock();
    p_Audio->v_Buffer.insert(p_Audio->v_Buffer.end(),
                             p_Stream,
                             p_Stream + i_Length);
    p_Audio->c_Mutex.unlock();
}

//*************************************************************************************
// Sphinx
//*************************************************************************************

std::vector<Uint8> ReadAudio::ConvertToSphinx(Uint8* p_Buffer, int i_Length, SDL_AudioFormat u16_Format, Uint32 u32_KHz, Uint8 u8_Channels) noexcept
{
    std::vector<Uint8> v_Result;
    SDL_AudioCVT c_CVT;
    SDL_BuildAudioCVT(&c_CVT,
                      u16_Format, u8_Channels, u32_KHz,
                      AUDIO_S16SYS, 1, 16000);
    
    // Define buffer
    v_Result.resize(i_Length * c_CVT.len_mult);
    memcpy(&(v_Result[0]), p_Buffer, i_Length);
    
    c_CVT.len = i_Length;
    c_CVT.buf = &(v_Result[0]);
    
    // Setup, run conversion
    SDL_ConvertAudio(&c_CVT);
    
    // All done!
    return v_Result;
}
