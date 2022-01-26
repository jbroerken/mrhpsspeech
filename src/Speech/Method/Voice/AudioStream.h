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
#include <utility>
#include <vector>
#include <list>

// External
#include <MRH_Typedefs.h>

// Project
#include "./AudioTrack.h"
#include "../MessageStream/MessageStream.h"
#include "../../../Configuration.h"


class AudioStream
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param c_Configuration The configuration to use.
     */
    
    AudioStream(Configuration const& c_Configuration);
    
    /**
     *  Default destructor.
     */
    
    ~AudioStream() noexcept;
    
    //*************************************************************************************
    // Record
    //*************************************************************************************
    
    /**
     *  Clear the currently recorded audio.
     */
    
    void ClearRecording() noexcept;
    
    /**
     *  Start recording audio.
     */
    
    void StartRecording();
    
    /**
     *  Stop recording audio.
     */
    
    void StopRecording();
    
    //*************************************************************************************
    // Playback
    //*************************************************************************************
    
    /**
     *  Start audio playback with a given audio track.
     *
     *  \param c_Audio The audio for playback.
     */
    
    void Playback(AudioTrack const& c_Audio);
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Grab the currently stored recorded audio.
     *
     *  \return The current recorded audio.
     */
    
    AudioTrack const& GetRecordedAudio();
    
    /**
     *  Check if the stream is connected.
     *
     *  \return true if connected, false if not.
     */
    
    bool GetConnected() noexcept;
    
    /**
     *  Check if audio is currently being recorded.
     *
     *  \return true if audio is being recorded, false if not.
     */
    
    bool GetRecordingActive() noexcept;
    
    /**
     *  Check if audio is currently being played.
     *
     *  \return true if audio is being played, false if not.
     */
    
    bool GetPlaybackActive() noexcept;
    
private:
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Update audio stream with recieved messages.
     */
    
    void UpdateStream() noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    MessageStream c_AudioStream;
    
    AudioTrack c_RecordedAudio;
    
    bool b_RecordingActive;
    bool b_PlaybackActive;
    
    std::string s_TriggerKeyPhrase;
    MRH_Uint32 u32_TriggerTimeoutS;
    MRH_Uint32 u32_RecordingKHz;
    MRH_Uint32 u32_PlaybackKHz;
    
    
protected:
    
};

#endif /* AudioStream_h */
