/**
 *  ReadAudio.h
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

#ifndef ReadAudio_h
#define ReadAudio_h

// C / C++
#include <vector>
#include <atomic>

// External
#include <pocketsphinx/pocketsphinx.h>
#include <SDL2/SDL.h>

// Project
#include "../../../../Exception.h"


class ReadAudio
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    struct Sample
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         */
        
        Sample() noexcept;
        
        /**
         *  Default destructor.
         */
        
        ~Sample() noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::vector<Uint8> v_Buffer;
        
        SDL_AudioFormat u16_Format;
        Uint32 u32_KHz;
        Uint8 u8_Channels;
        
        float f32_Amplitude;
        float f32_Peak;
        
        Uint64 u64_TimepointS;
    };
    
    //*************************************************************************************
    // Destructor
    //*************************************************************************************
    
    /**
     *  Default destructor.
     */
    
    virtual ~ReadAudio() noexcept;
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Pause listening.
     */
    
    void PauseListening() noexcept;
    
    /**
     *  Resume listening.
     */
    
    void ResumeListening() noexcept;
    
    //*************************************************************************************
    // Samples
    //*************************************************************************************
    
    /**
     *  Convert a given sample to a target format.
     *
     *  \param p_Sample The sample to convert.
     *  \param u16_Format The target format.
     *  \param u32_KHz The target KHz.
     *  \param u8_Channels The target channels.
     */
    
    void ConvertTo(Sample* p_Sample, SDL_AudioFormat u16_Format, Uint32 u32_KHz, Uint8 u8_Channels) noexcept;
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    struct Stream
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         */
        
        Stream() noexcept;
        
        /**
         *  Default destructor.
         */
        
        ~Stream() noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::mutex c_Mutex;
        
        std::vector<Sample*> v_Sample;
        
        SDL_AudioFormat u16_Format;
        Uint32 u32_KHz;
        Uint8 u8_Channels;
        Uint32 u32_StreamLengthS;
    };
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Update callback for read audio.
     *
     *  \param p_UserData User supplied data.
     *  \param p_Buffer Audio stream buffer.
     *  \param i_Length The length of the audio stream bytes.
     */
    
    static void SDLCallback(void* p_UserData, Uint8* p_Buffer, int i_Length) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    SDL_AudioDeviceID u32_DevID;
    
    ps_decoder_t* p_Decoder;
    cmd_ln_t* p_Config;
    
    Stream c_Stream;
    
protected:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    ReadAudio() noexcept;
    
    //*************************************************************************************
    // Setup
    //*************************************************************************************
    
    /**
     *  Read audio setup.
     *
     *  \param s_Locale The locale to use for audio.
     */
    
    void Setup(std::string const& s_Locale);
};

#endif /* ReadAudio_h */
