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
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <list>

// External
#include <MRH_Typedefs.h>

// Project
#include "./AudioTrack.h"

// Pre-defined
#define AUDIO_DEVICE_SOCKET_DISCONNECTED -1


class AudioDevice
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param s_Name The device name.
     *  \param i_SocketFD The device connection socket.
     *  \param b_CanPlay If the device can play audio.
     *  \param b_CanRecord If the device can record audio.
     */
    
    AudioDevice(std::string const& s_Name,
                int i_SocketFD,
                bool b_CanPlay,
                bool b_CanRecord);
    
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
     *  Get the device connection status.
     *
     *  \return true if connected, false if not.
     */
    
    bool GetConnected() noexcept;
    
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
     *  Check if data is currently being sent.
     *
     *  \return true if data is being sent, false if not.
     */
    
    bool GetSendingActive() noexcept;
    
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
    
    /**
     *  Switch the recording buffer.
     */
    
    void SwitchRecordingBuffer() noexcept;
    
    //*************************************************************************************
    // Record
    //*************************************************************************************
    
    /**
     *  Recieve data from the device.
     */
    
    void Recieve();
    
    //*************************************************************************************
    // Playback
    //*************************************************************************************
    
    /**
     *  Send service data to the device.
     */
    
    void Send();
    
    //*************************************************************************************
    // I/O
    //*************************************************************************************
    
    /**
     *  Read bytes from the device connection.
     *
     *  \param p_Dst The destination to read to.
     *  \param us_Length The length to read in bytes.
     *  \param i_TimeoutMS The poll timeout in milliseconds.
     *
     *  \return The amount of bytes read.
     */
    
    ssize_t Read(MRH_Uint8* p_Dst, size_t us_Length, int i_TimeoutMS);
    
    /**
     *  Write bytes to the device connection.
     *
     *  \param p_Src The source to write from.
     *  \param us_Length The length to write in bytes.
     *
     *  \return The amount of bytes written.
     */
    
    ssize_t Write(const MRH_Uint8* p_Src, size_t us_Length);
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    
    std::atomic<int> i_SocketFD;
    std::string s_Name;
    
    bool b_CanRecord;
    bool b_CanPlay;
    size_t us_PlaybackFrameSamples;
    
    std::mutex c_RecordingMutex;
    std::list<AudioTrack> l_RecordingAudio;
    std::list<AudioTrack>::iterator ActiveAudio;
    std::atomic<size_t> us_RecievedSamples;
    
    std::mutex c_WriteMutex;
    std::list<std::vector<MRH_Uint8>> l_WriteData;
    
protected:
    
};

#endif /* AudioDevice_h */
