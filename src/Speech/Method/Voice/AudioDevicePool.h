/**
 *  AudioDevicePool.h
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

#ifndef AudioDevicePool_h
#define AudioDevicePool_h

// C / C++
#include <utility>
#include <vector>
#include <list>

// External
#include <MRH_Typedefs.h>

// Project
#include "./AudioDevice.h"


class AudioDevicePool
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    AudioDevicePool();
    
    /**
     *  Default destructor.
     */
    
    ~AudioDevicePool() noexcept;
    
    //*************************************************************************************
    // Devices
    //*************************************************************************************
    
    /**
     *  Start all audio devices.
     */
    
    void StartDevices() noexcept;
    
    /**
     *  Stop all audio devices.
     */
    
    void StopDevices() noexcept;
    
    //*************************************************************************************
    // Record
    //*************************************************************************************
    
    /**
     *  Select the audio device to use as a source for recordings. Only this device will
     *  then be used to retrieve audio. All other recording devices will be ignored.
     */
    
    void SelectRecordingDevice();
    
    /**
     *  Reset the current recording device, allowing all devices to record again for
     *  selection.
     */
    
    void ResetRecordingDevice() noexcept;
    
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
     *  Check if audio is currently being played.
     *
     *  \return true if audio is being played, false if not.
     */
    
    bool GetPlaybackActive() noexcept;
    
    /**
     *  Check if a recording device was set.
     *
     *  \return true if a device was set, false if not.
     */
    
    bool GetRecordingDeviceSelected() noexcept;
    
private:
    
    //*************************************************************************************
    // Devices
    //*************************************************************************************
    
    /**
     *  Update the decives in the device pool.
     */
    
    void UpdateDevices() noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::list<AudioDevice> l_Device;
    
    // Selected devices
    std::list<AudioDevice>::iterator RecordingDevice;
    std::list<AudioDevice>::iterator PlaybackDevice;
    
    // Audio Info
    std::pair<MRH_Uint32, MRH_Uint32> c_RecordingFormat; // <KHz, Frame Elements>
    std::pair<MRH_Uint32, MRH_Uint32> c_PlaybackFormat;
    
    // Audio to add to devices
    std::vector<MRH_Sint16> v_Send;
    
protected:
    
};

#endif /* AudioDevicePool_h */
