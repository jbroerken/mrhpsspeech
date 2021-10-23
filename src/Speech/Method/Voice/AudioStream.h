/**
 *  AudioStream.h
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

#ifndef AudioStream_h
#define AudioStream_h

// C / C++
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <list>

// External
#include <MRH_Typedefs.h>

// Project
#include "./MonoAudio.h"


class AudioStream
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    AudioStream();
    
    /**
     *  Default destructor.
     */
    
    ~AudioStream() noexcept;
    
    //*************************************************************************************
    // Stream
    //*************************************************************************************
    
    /**
     *  Stop all audio streams.
     */
    
    void StopAll() noexcept;
    
    //*************************************************************************************
    // Input
    //*************************************************************************************
    
    /**
     *  Start recording audio.
     */
    
    void Record();
    
    //*************************************************************************************
    // Output
    //*************************************************************************************
    
    /**
     *  Play the currently set audio.
     */
    
    void Playback();
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Grab the currently stored recorded voice audio.
     *
     *  \return The current voice audio.
     */
    
    MonoAudio GetRecordedAudio() noexcept;
    
    /**
     *  Check if audio is currently being played.
     *
     *  \return true if audio is playing, false if not.
     */
    
    bool GetPlayback() noexcept;
    
    /**
     *  Check if audio input is currently being recorded.
     *
     *  \return true if audio is being recorded, false if not.
     */
    
    bool GetRecording() noexcept;
    
    //*************************************************************************************
    // Setters
    //*************************************************************************************
    
    /**
     *  Set the audio for playback.
     *
     *  \param c_Audio The audio for playback.
     */
    
    void SetPlaybackAudio(MonoAudio const& c_Audio);
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum AudioState
    {
        NONE = 0,
        
        RECORDING = 1,
        PLAYBACK = 2,
        
        AUDIO_STATE_MAX = PLAYBACK,
        
        AUDIO_STATE_COUNT = AUDIO_STATE_MAX + 1
    };
    
    class AudioDevice
    {
    public:
    
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         *
         *  \param b_CanRecord If the device is able to record audio.
         *  \param b_CanPlay If the device is able to play audio.
         */
        
        AudioDevice(bool b_CanRecord,
                    bool b_CanPlay);
        
        /**
         *  Default destructor.
         */
        
        ~AudioDevice() noexcept;
        
        //*************************************************************************************
        // Update
        //*************************************************************************************
        
        /**
         *  Record audio.
         */
        
        void Record() noexcept;
        
        /**
         *  Play audio.
         */
        
        void Play() noexcept;
        
        //*************************************************************************************
        // Getters
        //*************************************************************************************
        
        /**
         *  Get the current audio state.
         *
         *  \return The current audio state.
         */
        
        AudioState GetState() noexcept;
        
        //*************************************************************************************
        // Setters
        //*************************************************************************************
        
        /**
         *  Switch the current audio state.
         *
         *  \param e_State The new audio state.
         */
        
        void SetState(AudioState e_State) noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        const bool b_CanRecord;
        const bool b_CanPlay;
        
        std::vector<std::pair<float, std::vector<MRH_Sint16>>> v_Recieved; // <Average Amp, Sound Data>
        std::mutex c_RecievedMutex;
        float f32_LastAmplitude; // Last average amplitude
        
        std::vector<MRH_Sint16> v_Send;
        std::mutex c_SendMutex;
        
    private:
        
        //*************************************************************************************
        // Update
        //*************************************************************************************
        
        /**
         *  Update the audio device.
         *
         *  \param p_Instance The audio device to update.
         */
        
        static void Update(AudioDevice* p_Instance) noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::thread c_Thread;
        std::atomic<bool> b_Update;
        
        std::atomic<AudioState> e_State;
        std::atomic<bool> b_StateChanged; // To send correct codes
        
    protected:
        
    };
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::list<AudioDevice> l_Device;
    
    // Audio Info
    std::pair<MRH_Uint32, MRH_Uint32> c_RecordingFormat; // <KHz, Frame Elements>
    std::pair<MRH_Uint32, MRH_Uint32> c_PlaybackFormat;
    
    // Audio to add to devices
    std::vector<MRH_Sint16> v_Send;
    
protected:
    
};

#endif /* AudioStream_h */
