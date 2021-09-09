/**
 *  PAMicrophone.h
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

#ifndef PAMicrophone_h
#define PAMicrophone_h

// C / C++
#include <vector>
#include <atomic>

// External
#include <MRH_Typedefs.h>
#include <portaudio.h>

// Project
#include "../../../Exception.h"


class PAMicrophone
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    struct Sample
    {
        friend class PAMicrophone;
        
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
        
        std::vector<MRH_Uint8> v_Buffer;
        
        PaSampleFormat u16_Format;
        MRH_Uint32 u32_KHz;
        MRH_Uint8 u8_Channels;
        
        MRH_Sfloat32 f32_Amplitude;
        MRH_Sfloat32 f32_Peak;
        
    private:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        MRH_Uint64 u64_TimepointS;
    };
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    PAMicrophone();
    
    /**
     *  Default destructor.
     */
    
    virtual ~PAMicrophone() noexcept;
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Start listening.
     */
    
    void StartListening();
    
    /**
     *  Stop listening.
     */
    
    void StopListening();
    
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
    
    void ConvertTo(Sample* p_Sample, PaSampleFormat u16_Format, MRH_Uint32 u32_KHz, MRH_Uint8 u8_Channels) noexcept;
    
    /**
     *  Get the number of audio stream samples available.
     *
     *  \return The number of samples available.
     */
    
    size_t GetSampleCount() noexcept;
    
    /**
     *  Grab a sample, removing it from the audio stream.
     *
     *  \param us_Sample The sample to request.
     *
     *  \return The requested sample.
     */
    
    Sample* GrabSample(size_t us_Sample);
    
    /**
     *  Return a sample to the audio stream.
     *
     *  \param p_Sample The sample to return.
     */
    
    void ReturnSample(Sample* p_Sample);
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    struct Audio
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         */
        
        Audio() noexcept;
        
        /**
         *  Default destructor.
         */
        
        ~Audio() noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::mutex c_Mutex;
        
        std::vector<Sample*> v_Sample;
        
        PaSampleFormat u16_Format;
        MRH_Uint32 u32_KHz;
        MRH_Uint8 u8_Channels;
        MRH_Uint32 u32_StreamLengthS;
    };
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Update callback for read audio.
     */
    
    static int PACallback(const void* p_Input,
                          void* p_Output,
                          unsigned long u32_FrameCount,
                          const PaStreamCallbackTimeInfo* p_TimeInfo,
                          PaStreamCallbackFlags e_StatusFlags,
                          void* p_UserData) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    static bool b_SetupPA;
    
    PaStream* p_Stream;
    
    Audio c_Audio;
    
protected:
    
};

#endif /* PAMicrophone_h */
