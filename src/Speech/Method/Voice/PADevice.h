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
    
    void StartPlayback();
    
    /**
     *  Stop playback.
     */
    
    void StopPlayback();
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Grab the currently stored recorded voice audio.
     *
     *  \return The current voice audio.
     */
    
    VoiceAudio GetInputAudio() noexcept;
    
    /**
     *  Check if audio is currently being played.
     *
     *  \return true if audio is playing, false if not.
     */
    
    bool GetOutputPlayback() noexcept;
    
    /**
     *  Check if audio input is currently being recorded.
     *
     *  \return true if audio is being recorded, false if not.
     */
    
    bool GetInputRecording() noexcept;
    
    //*************************************************************************************
    // Setters
    //*************************************************************************************
    
    /**
     *  Set the audio for playback.
     *
     *  \param c_Audio The audio for playback.
     */
    
    void SetOutputAudio(VoiceAudio const& c_Audio);
    
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
    
    class Output
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         */
        
        Output() noexcept;
        
        /**
         *  Default destructor.
         */
        
        ~Output() noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::atomic<bool> b_Playback;
        
        std::vector<MRH_Sint16> v_Buffer;
        size_t us_BufferPos;
        
        MRH_Uint32 u32_KHz;
        MRH_Uint8 u8_Channels;
        MRH_Uint32 u32_FrameSamples;
    };
    
    //*************************************************************************************
    // Stream
    //*************************************************************************************
    
    /**
     *  Start a PortAudio stream.
     *
     *  \param p_Stream The stream to start.
     */
    
    void StartStream(PaStream* p_Stream);
    
    /**
     *  Stop a PortAudio stream.
     *
     *  \param p_Stream The stream to stop.
     */
    
    void StopStream(PaStream* p_Stream);
    
    /**
     *  Close a PortAudio stream.
     *
     *  \param p_Stream The stream to close.
     */
    
    void CloseStream(PaStream* p_Stream) noexcept;
    
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
    
    /**
     *  Setup output device and stream.
     */
    
    void SetupOutput();
    
    /**
     *  Close output device and stream.
     */
    
    void CloseOutput() noexcept;
    
    /**
     *  Update callback for output audio.
     */
    
    static int PAOutputCallback(const void* p_Input,
                                void* p_Output,
                                unsigned long u32_FrameCount,
                                const PaStreamCallbackTimeInfo* p_TimeInfo,
                                PaStreamCallbackFlags e_StatusFlags,
                                void* p_UserData) noexcept;
    
    //*************************************************************************************
    // Setters
    //*************************************************************************************
    
    /**
     *  Set the output audio buffer.
     *
     *  \param v_Buffer The buffer to use.
     */
    
    void SetOutputAudioBuffer(std::vector<MRH_Sint16> const& v_Buffer) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    PaStream* p_InputStream;
    PaStream* p_OutputStream;
    
    Input c_InputAudio;
    Output c_OutputAudio;
    
protected:
    
};

#endif /* PADevice_h */
