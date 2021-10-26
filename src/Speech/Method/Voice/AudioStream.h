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
#include "./AudioDevice.h"


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
     *  Start recording audio. All devices unable to record will be stopped.
     */
    
    void Record();
    
    /**
     *  Select the input device to set as the primary recording device. This device will
     *  then be used to retrieve audio. All other recording devices will be stopped.
     */
    
    void SelectPrimaryRecordingDevice();
    
    //*************************************************************************************
    // Output
    //*************************************************************************************
    
    /**
     *  Play the currently set audio.
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
    
    /**
     *  Check if the primary recording device was set.
     *
     *  \return true if a device was set, false if not.
     */
    
    bool GetPrimaryRecordingDeviceSet() noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::list<AudioDevice> l_Device;
    
    // Selected recording device.
    std::list<AudioDevice>::iterator PrimaryRecordingDevice;
    
    // Audio Info
    std::pair<MRH_Uint32, MRH_Uint32> c_RecordingFormat; // <KHz, Frame Elements>
    std::pair<MRH_Uint32, MRH_Uint32> c_PlaybackFormat;
    
    // Audio to add to devices
    std::vector<MRH_Sint16> v_Send;
    
protected:
    
};

#endif /* AudioStream_h */
