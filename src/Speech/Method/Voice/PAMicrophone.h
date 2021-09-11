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
#include "./AudioSample.h"


class PAMicrophone
{
public:
    
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
    // Getters
    //*************************************************************************************
    
    /**
     *  Grab a sample, removing it from the audio stream.
     *
     *  \param us_Sample The sample to request.
     *
     *  \return The requested sample.
     */
    
    AudioSample GetAudioSample() noexcept;
    
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
        
        MRH_Sint16* p_Buffer;
        MRH_Sint16* p_BufferA;
        MRH_Sint16* p_BufferB;
        size_t us_BufferPos;
        size_t us_BufferSize;
        
        MRH_Uint32 u32_KHz;
        MRH_Uint8 u8_Channels;
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
    
    static std::atomic<int> i_PAUsers;
    
    PaStream* p_Stream;
    
    Audio c_Audio;
    
protected:
    
};

#endif /* PAMicrophone_h */
