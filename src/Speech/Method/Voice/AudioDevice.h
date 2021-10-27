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
#include "AudioDeviceOpCode.h"


class AudioDevice
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum DeviceState
    {
        STOPPED = 0,
        RECORDING = 1,
        PLAYING = 2,
        
        DEVICE_STATE_MAX = PLAYING,
        
        DEVICE_STATE_COUNT = DEVICE_STATE_MAX + 1
    };
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param u32_ID The unique device id.
     *  \param s_Name The device name.
     *  \param s_Address The device connection address.
     *  \param i_Port The device connection port.
     */
    
    AudioDevice(MRH_Uint32 u32_ID,
                std::string const& s_Name,
                std::string const& s_Address,
                int i_Port);
    
    /**
     *  Default destructor.
     */
    
    ~AudioDevice() noexcept;
    
    //*************************************************************************************
    // Stop
    //*************************************************************************************
    
    /**
     *  Stop the device.
     */
    
    void Stop() noexcept;
    
    //*************************************************************************************
    // Record
    //*************************************************************************************
    
    /**
     *  Record.
     */
    
    void Record();
    
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
     *  Get the currently recorded audio.
     *
     *  \return The currently recorded audio.
     */
    
    AudioTrack const& GetRecordedAudio() noexcept;
    
    /**
     *  Get the current audio state.
     *
     *  \return The current audio state.
     */
    
    DeviceState GetState() noexcept;
    
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
     *  Get the amount of available samples.
     *
     *  \return The available samples.
     */
    
    size_t GetAvailableSamples() noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    const MRH_Uint32 u32_ID;
    const std::string s_Name;
    const std::string s_Address;
    const int i_Port;
    
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
    // Connection
    //*************************************************************************************
    
    /**
     *  Connect to the audio device.
     */
    
    void Connect();
    
    /**
     *  Disconnect from the audio device.
     */
    
    void Disconnect() noexcept;
    
    //*************************************************************************************
    // Record
    //*************************************************************************************
    
    /**
     *  Recieve data from the device.
     */
    
    void Recieve();
    
    /**
     *  Recieve device recording audio buffer data.
     *
     *  \return true if all data was recieved, false if not.
     */
    
    bool RecieveAudio();
    
    /**
     *  Recieve device state changed opcode data.
     *
     *  \return true if all data was recieved, false if not.
     */
    
    bool RecieveStateChanged();
    
    //*************************************************************************************
    // Playback
    //*************************************************************************************
    
    /**
     *  Send service data to the device.
     */
    
    void Send();
    
    /**
     *  Add the device state switch opcode.
     *
     *  \param c_OpCode The OpCode data.
     */
    
    void AddSwitchStateOpCode(AudioDeviceOpCode::SERVICE_CHANGE_DEVICE_STATE_DATA& c_OpCode);
    
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
     *  \return true if reading succeeded, false if not.
     */
    
    bool ReadAll(MRH_Uint8* p_Dst, size_t us_Length, int i_TimeoutMS);
    
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
     *  \return true if writing succeeded, false if not.
     */
    
    bool WriteAll(const MRH_Uint8* p_Src, size_t us_Length);
    
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
    std::atomic<bool> b_Update;
    
    std::atomic<int> i_SocketFD;
    std::atomic<DeviceState> e_State; // Active state
    
    std::pair<bool, size_t> c_ReadInfo; // <OpCode Read, Read Size B>
    std::pair<bool, size_t> c_WriteInfo; // <OpCode Written, Write Size B>
    
    bool b_CanRecord;
    bool b_CanPlay;
    
    std::mutex c_RecordingMutex;
    std::list<AudioTrack> l_Audio;
    std::list<AudioTrack>::iterator ActiveAudio;
    std::atomic<size_t> us_AvailableSamples;
    
    std::mutex c_WriteMutex;
    std::list<std::vector<MRH_Uint8>> l_WriteBytes;
    
    MRH_Uint64 u64_HeartbeatReadS;
    MRH_Uint64 u64_HeartbeatWriteS;
    
protected:
    
};

#endif /* AudioDevice_h */
