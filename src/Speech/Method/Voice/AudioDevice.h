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
#include "../../../Exception.h"

// Pre-defined
#ifndef MRH_HOST_IS_BIG_ENDIAN // Big endian host, needs conversion
    #define MRH_HOST_IS_BIG_ENDIAN 0
#endif


class AudioDevice
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum DeviceState
    {
        NONE = 0,
        
        RECORDING = 1,
        PLAYBACK = 2,
        
        DEVICE_STATE_MAX = PLAYBACK,
        
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
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the current audio state.
     *
     *  \return The current audio state.
     */
    
    DeviceState GetState() noexcept;
    
    /**
     *  Get the average recording amplitude.
     *
     *  \return The average recording amplitude.
     */
    
    float GetRecordingAmplitude() noexcept;
    
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
    
    //*************************************************************************************
    // Setters
    //*************************************************************************************
    
    /**
     *  Set the current audio state. Old data for the state is cleared on switching.
     *
     *  \param e_State The new audio state.
     */
    
    void SetState(DeviceState e_State);
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    const MRH_Uint32 u32_ID;
    const std::string s_Name;
    
    std::vector<std::pair<float, std::vector<MRH_Sint16>>> v_Recieved; // <Average Amp, Sound Data>
    std::mutex c_RecievedMutex;
    
    std::vector<std::vector<MRH_Sint16>> v_Send;
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
    
    /**
     *  Disconnect from the audio device.
     */
    
    void Disconnect() noexcept;
    
    /**
     *  Recieve device data from the device.
     */
    
    void Recieve();
    
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
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the average amplitude for a sample frame.
     *
     *  \param p_Buffer The sample buffer.
     *  \param us_Elements The length of the sample buffer in elements.
     *
     *  \return The average amplitude in percent.
     */
    
    float GetAverageAmplitude(const MRH_Sint16* p_Buffer, size_t us_Length) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::thread c_Thread;
    std::atomic<bool> b_Update;
    
    std::atomic<int> i_SocketFD;
    std::atomic<DeviceState> e_State; // Active state
    
    bool b_CanRecord;
    bool b_CanPlay;
    
    std::atomic<float> f32_LastAmplitude; // Last average amplitude
    
protected:
    
};

#endif /* AudioDevice_h */
