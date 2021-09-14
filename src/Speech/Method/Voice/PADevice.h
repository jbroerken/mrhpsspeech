/**
 *  PADevice.h
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

#ifndef PADevice_h
#define PADevice_h

// C / C++
#include <vector>
#include <atomic>

// External
#include <MRH_Typedefs.h>
#include <portaudio.h>

// Project
#include "./VoiceAudio.h"


class PADevice
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    PADevice();
    
    /**
     *  Default destructor.
     */
    
    ~PADevice() noexcept;
    
    //*************************************************************************************
    // Input
    //*************************************************************************************
    
    /**
     *  Start listening.
     */
    
    void StartListening();
    
    /**
     *  Stop listening.
     */
    
    void StopListening();
    
    /**
     *  Reset input buffer.
     */
    
    void ResetInputAudio();
    
    //*************************************************************************************
    // Output
    //*************************************************************************************
    
    /**
     *  Start playback.
     */
    
    //void StartPlayback();
    
    /**
     *  Stop playback.
     */
    
    //void StopPlayback();
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Grab the currently stored recorded voice audio.
     *
     *  \return The vurrent voice audio.
     */
    
    VoiceAudio GetInputAudio() noexcept;
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    class Input
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         */
        
        Input() noexcept;
        
        /**
         *  Default destructor.
         */
        
        ~Input() noexcept;
        
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
        MRH_Uint32 u32_FrameSamples;
    };
    
    //*************************************************************************************
    // Input
    //*************************************************************************************
    
    /**
     *  Setup input device and stream.
     */
    
    void SetupInput();
    
    /**
     *  Close input device and stream.
     */
    
    void CloseInput() noexcept;
    
    /**
     *  Update callback for input audio.
     */
    
    static int PAInputCallback(const void* p_Input,
                               void* p_Output,
                               unsigned long u32_FrameCount,
                               const PaStreamCallbackTimeInfo* p_TimeInfo,
                               PaStreamCallbackFlags e_StatusFlags,
                               void* p_UserData) noexcept;
    
    //*************************************************************************************
    // Output
    //*************************************************************************************
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    PaStream* p_InputStream;
    
    Input c_InputAudio;
    
protected:
    
};

#endif /* PADevice_h */
