/**
 *  AudioDeviceOpCode.h
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

#ifndef AudioDeviceOpCode_h
#define AudioDeviceOpCode_h

// C / C++
#include <vector>
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #include <byteswap.h>
#endif

// External
#include <MRH_Typedefs.h>

// Project
#include "../../../../Exception.h"

// Pre-defined
#define AUDIO_DEVICE_OPCODE_VERSION 1

#define AUDIO_DEVICE_BOOL_TRUE 1
#define AUDIO_DEVICE_BOOL_FALSE 0


namespace AudioDeviceOpCode
{
    //*************************************************************************************
    // OpCodes
    //*************************************************************************************
    
    enum OpCodeList
    {
        // Shared
        ALL_UNK = 0,                            // ???
        
        ALL_HEARTBEAT = 1,                      // Signal availability to partner
        
        ALL_AUDIO = 2,                          // Audio data
        
        // Speech Service
        SERVICE_CONNECT_RESPONSE_OK = 3,        // Connection success
        SERVICE_CONNECT_RESPONSE_FAIL = 4,      // Failed to connect
        
        SERVICE_PAIR_RESPONSE = 5,              // Responst to a device pairing request with connection data, encrypted
        
        SERVICE_START_RECORDING = 6,            // Signal recording start to device
        SERVICE_STOP_RECORDING = 7,             // Signal recording stop to device.
        
        // Audio Device
        DEVICE_CONNECT_REQUEST,                 // Connection response of service request
        
        DEVICE_PAIR_REQUEST,                    // Request pairing with the service (Hand key and info)
        
        DEVICE_PLAYBACK_FINISHED,               // Audio playback has ended
        
        // Bounds
        OPCODE_MAX = DEVICE_PLAYBACK_FINISHED,
        
        OPCODE_COUNT = OPCODE_MAX + 1
    };
    
    typedef MRH_Uint8 OpCode; // OpCodeList, set size for sending
    
    //*************************************************************************************
    // OpCode Data - Base
    //*************************************************************************************
    
    class OpCodeData
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Value constructor.
         *
         *  \param u8_OpCode The opcode represented.
         */
        
        OpCodeData(OpCode u8_OpCode) noexcept;
        
        /**
         *  Default destructor.
         */
        
        virtual ~OpCodeData() noexcept;
        
        //*************************************************************************************
        // Getters
        //*************************************************************************************
        
        /**
         *  Get the opcode identifier.
         *
         *  \return The opcode identifier.
         */
        
        OpCode GetOpCode() noexcept;
        
        /**
         *  Get the opcode data size.
         *
         *  \return The opcode data size.
         */
        
        MRH_Uint32 GetDataSize() noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::vector<MRH_Uint8> v_Data;
        
    private:
        
    protected:
        
        //*************************************************************************************
        // Constructor
        //*************************************************************************************
        
        /**
         *  Data constructor.
         *
         *  \param v_Data The data for the opcode.
         */
        
        OpCodeData(std::vector<MRH_Uint8>& v_Data) noexcept;
    };
    
    //*************************************************************************************
    // OpCode Data - ALL_AUDIO
    //*************************************************************************************
    
    class ALL_AUDIO_DATA : public OpCodeData
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Data constructor.
         *
         *  \param v_Data The data for the opcode.
         */
        
        ALL_AUDIO_DATA(std::vector<MRH_Uint8>& v_Data) noexcept;
        
        /**
         *  Value constructor.
         *
         *  \param p_Samples The samples to store.
         *  \param u32_Count The amount of samples to store.
         */
        
        ALL_AUDIO_DATA(const MRH_Sint16* p_Samples,
                       MRH_Uint32 u32_Count) noexcept;
        
        /**
         *  Default destructor.
         */
        
        ~ALL_AUDIO_DATA() noexcept;
        
        //*************************************************************************************
        // Convert
        //*************************************************************************************
        
        /**
         *  Convert the samples stored to the host format.
         */
        
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        void ConvertSamples() noexcept;
#endif
        
        //*************************************************************************************
        // Getters
        //*************************************************************************************
        
        /**
         *  Get the audio buffer.
         *
         *  \return The audio buffer.
         */
        
        const MRH_Sint16* GetAudioBuffer() noexcept;
        
        /**
         *  Get the KHz used for recording.
         *
         *  \return The KHz used for recording.
         */
        
        MRH_Uint32 GetSampleCount() noexcept;
        
    private:
        
    protected:
        
    };
    
    //*************************************************************************************
    // OpCode Data - SERVICE_CONNECT_RESPONSE_OK
    //*************************************************************************************
    
    class SERVICE_CONNECT_RESPONSE_OK_DATA : public OpCodeData
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Data constructor.
         *
         *  \param v_Data The data for the opcode.
         */
        
        SERVICE_CONNECT_RESPONSE_OK_DATA(std::vector<MRH_Uint8>& v_Data) noexcept;
        
        /**
         *  Value constructor.
         *
         *  \param u32_RecordingKHz The KHz used for recording.
         *  \param u32_RecordingFrameElements The samples per frame for recording.
         *  \param u32_PlaybackKHz The KHz used for playback.
         *  \param u32_PlaybackFrameElements The samples per frame for playback.
         */
        
        SERVICE_CONNECT_RESPONSE_OK_DATA(MRH_Uint32 u32_RecordingKHz,
                                         MRH_Uint32 u32_RecordingFrameElements,
                                         MRH_Uint32 u32_PlaybackKHz,
                                         MRH_Uint32 u32_PlaybackFrameElements) noexcept;
        
        /**
         *  Default destructor.
         */
        
        ~SERVICE_CONNECT_RESPONSE_OK_DATA() noexcept;
        
        //*************************************************************************************
        // Getters
        //*************************************************************************************
        
        /**
         *  Get the KHz used for recording.
         *
         *  \return The KHz used for recording.
         */
        
        MRH_Uint32 GetRecordingKHz() noexcept;
        
        /**
         *  Get the samples per frame for recording.
         *
         *  \return The samples per frame for recording.
         */
        
        MRH_Uint32 GetRecordingFrameSamples() noexcept;
        
        /**
         *  Get the KHz used for playback.
         *
         *  \return The KHz used for playback.
         */
        
        MRH_Uint32 GetPlaybackKHz() noexcept;
        
        /**
         *  Get the samples per frame for playback.
         *
         *  \return The samples per frame for playback.
         */
        
        MRH_Uint32 GetPlaybackFrameSamples() noexcept;
        
    private:
        
    protected:
        
    };
    
    //*************************************************************************************
    // OpCode Data - DEVICE_CONNECT_REQUEST
    //*************************************************************************************
    
    class DEVICE_CONNECT_REQUEST_DATA : public OpCodeData
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Data constructor.
         *
         *  \param v_Data The data for the opcode.
         */
        
        DEVICE_CONNECT_REQUEST_DATA(std::vector<MRH_Uint8>& v_Data) noexcept;
        
        /**
         *  Value constructor.
         *
         *  \param s_Address The device address for sending to.
         *  \param u16_Port The device port for sending to.
         *  \param u8_OpCodeVersion The opcode version in use.
         *  \param b_CanRecord If the device supports recording.
         *  \param b_CanPlay If the device supports playback.
         */
        
        DEVICE_CONNECT_REQUEST_DATA(std::string const& s_Address,
                                    MRH_Uint16 u16_Port,
                                    MRH_Uint8 u8_OpCodeVersion,
                                    bool b_CanRecord,
                                    bool b_CanPlay) noexcept;
        
        /**
         *  Default destructor.
         */
        
        ~DEVICE_CONNECT_REQUEST_DATA() noexcept;
        
        //*************************************************************************************
        // Getters
        //*************************************************************************************
        
        /**
         *  Get the device address for sending to.
         *
         *  \return The device address for sending to.
         */
        
        std::string GetAddress() noexcept;
        
        /**
         *  Get the device port for sending to.
         *
         *  \return The device port for sending to.
         */
        
        MRH_Uint16 GetPort() noexcept;
        
        /**
         *  Get the opcode version in use.
         *
         *  \return The opcode version in use.
         */
        
        MRH_Uint8 GetOpCodeVersion() noexcept;
        
        /**
         *  Get if the device supports recording.
         *
         *  \return true if recording is supported, false if not.
         */
        
        bool GetCanRecord() noexcept;
        
        /**
         *  Get if the device supports playback.
         *
         *  \return true if playback is supported, false if not.
         */
        
        bool GetCanPlay() noexcept;
        
    private:
        
    protected:
        
    };
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get a data value.
     *
     *  \param v_Data The message data.
     *  \param us_Pos The start byte position.
     *
     *  \return The casted value.
     */
    
    template <typename T> inline static T GetValue(std::vector<MRH_Uint8> const& v_Data, size_t us_Pos)
    {
        return *(T*)&(v_Data[us_Pos]);
    }
    
    //*************************************************************************************
    // Setters
    //*************************************************************************************
    
    /**
     *  Set a data value.
     *
     *  \param v_Data The message data.
     *  \param us_Pos The start byte position.
     *  \param Value The value to set.
     */
    
    template <typename T> inline void SetValue(std::vector<MRH_Uint8>& v_Data, size_t us_Pos, const T* Value)
    {
        std::memcpy(&(v_Data[us_Pos]),
                    Value,
                    sizeof(T));
    }
}

#endif /* AudioDeviceOpCode_h */
