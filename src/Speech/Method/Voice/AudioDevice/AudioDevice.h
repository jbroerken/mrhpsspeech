/**
 *  AudioDevice.h
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

#ifndef AudioDevice_h
#define AudioDevice_h

// C / C++

// External
#include <MRH_Typedefs.h>

// Project
#include "../AudioTrack.h"
#include "./AudioDeviceTraffic.h"


class AudioDevice
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param c_DeviceTraffic The device traffic instance to use for traffic.
     *  \param s_Address The audio device address.
     *  \param i_Port The audio device port.
     *  \param b_CanRecord If the device can record audio.
     *  \param b_CanPlay If the device can play audio.
     */
    
    AudioDevice(AudioDeviceTraffic& c_DeviceTraffic,
                std::string const& s_Address,
                int i_Port,
                bool b_CanRecord,
                bool b_CanPlay);
    
    /**
     *  Default destructor.
     */
    
    ~AudioDevice() noexcept;
    
    //*************************************************************************************
    // Recording
    //*************************************************************************************
    
    /**
     *  Start recording.
     */
    
    void StartRecording() noexcept;
    
    /**
     *  Stop recording.
     */
    
    void StopRecording() noexcept;
    
    //*************************************************************************************
    // Playback
    //*************************************************************************************
    
    /**
     *  Start playback.
     *
     *  \param c_Audio The audio to play.
     */
    
    void Play(AudioTrack const& c_Audio);
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the device availability status.
     *
     *  \return true if available, false if not.
     */
    
    bool GetAvailable() noexcept;
    
    /**
     *  Get the device address.
     *
     *  \return The device address.
     */
    
    std::string const& GetAddress() noexcept;
    
    /**
     *  Get the device port.
     *
     *  \return The device port.
     */
    
    int GetPort() noexcept;
    
    /**
     *  Get the currently recorded audio.
     *
     *  \return The currently recorded audio.
     */
    
    AudioTrack const& GetRecordedAudio() noexcept;
    
    /**
     *  Check if the device can record audio.
     *
     *  \return true if the device can record audio, false if not.
     */
    
    bool GetCanRecord() noexcept;
    
    /**
     *  Check if the device can play audio.
     *
     *  \return true if the device can play audio, false if not.
     */
    
    bool GetCanPlay() noexcept;
    
    /**
     *  Get the amount of available recorded samples.
     *
     *  \return The available recorded samples.
     */
    
    size_t GetRecievedSamples() noexcept;
    
    /**
     *  Check if audio is currently being played.
     *
     *  \return true if audio is being played, false if not.
     */
    
    bool GetPlaybackActive() noexcept;
    
private:
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get all recieved data.
     */
    
    void GetRecievedData() noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    AudioDeviceTraffic& c_DeviceTraffic;
    std::string s_Address;
    int i_Port;
    
    bool b_CanRecord;
    bool b_CanPlay;
    
    AudioTrack c_Recording;
    size_t us_RecievedSamples;
    
    bool b_PlaybackActive;
    
    MRH_Uint64 u64_LastDeviceHeartbeatS;
    MRH_Uint64 u64_NextServiceHeartbeatS;
    
protected:
    
};

#endif /* AudioDevice_h */
