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

// External
#include <MRH_Typedefs.h>

// Project

// Pre-defined
#define AUDIO_STREAM_OPCODE_VERSION 1

#define AUDIO_STREAM_CONNECT_NAME_SIZE 1024
#define AUDIO_STREAM_CONNECT_ADDRESS_SIZE 512
#define AUDIO_STREAM_PAIR_KEY_SIZE 1024
#define AUDIO_STREAM_PAIR_SSID_SIZE 1024
#define AUDIO_STREAM_PAIR_PASSWORD_SIZE 1024


namespace AudioDeviceOpCode
{
    //*************************************************************************************
    // OpCodes
    //*************************************************************************************
    
    enum OpCodeList
    {
        // Shared
        ALL_HEARTBEAT = 0,                      // Check if partner is still available
        ALL_DISCONNECT = 1,                     // Disconnect from partner
        
        // Speech Service
        SERVICE_CONNECT_REQUEST = 2,            // Connect to device
        
        SERVICE_PAIR_REQUEST = 3,               // Request pairing with a audio device
        SERVICE_PAIR_CONNECTION_DETAILS = 4,    // Send connection details (wifi, etc) to device, encrypted
        
        SERVICE_AUDIO_PLAYBACK_START = 5,       // Start playback
        SERVICE_AUDIO_PLAYBACK_DATA = 6,        // Playback data buffer
        SERVICE_AUDIO_PLAYBACK_END = 7,         // Stop playback
        
        SERVICE_DISABLE_PLAYBACK,               // Disable device playback
        SERVICE_DISABLE_RECORDING,              // Disable device recording
        SERVICE_ENABLE_PLAYBACK,                // Enable device playback
        SERVICE_ENABLE_RECORDING,               // Enable device recording
        
        // Audio Device
        DEVICE_CONNECT_RESPONSE,                // Connection response of service request
        
        DEVICE_PAIR_RESPONSE,                   // Response to pair request
        DEVICE_PAIR_GIVE_KEY,                   // Hand a public key to the service which wants to pair
        DEVICE_PAIR_GIVE_CONNECION,             // Hand the device connection info (name, ip, etc.) to the service wanting to pair
        
        DEVICE_AUDIO_RECORDING_START,           // Start recording
        DEVICE_AUDIO_RECORDING_DATA,            // Recording data buffer
        DEVICE_AUDIO_RECORDING_END,             // Stop recording
        
        DEVICE_PLAYBACK_STATE,                  // Return current device playback state
        DEVICE_RECORDING_STATE,                 // Return current device recording state
        
        // Bounds
        OPCODE_MAX = DEVICE_RECORDING_STATE,
        
        OPCODE_COUNT = OPCODE_MAX + 1
    };
    
    typedef MRH_Uint32 OpCode; // OpCodeList, set size for sending
    
    //*************************************************************************************
    // OpCode Error
    //*************************************************************************************
    
    enum OpCodeErrorList
    {
        // Default
        NONE = 0,
        
        // Connection
        ALREADY_CONNECTED = 1,
        UNAVAILABLE = 2,
        
        // Pairing
        ALREADY_PAIRED = 3,
        
        // Device Playback State
        NO_SPEAKER = 4,
        SPEAKER_ALREADY_DISABLED = 5,
        
        // Device Recording State
        NO_MICROPHONE = 6,
        MICROPHONE_ALREADY_DISABLED = 7,
        
        // Bounds
        OPCODE_ERROR_MAX = NO_MICROPHONE,
        
        OPCODE_ERROR_COUNT = OPCODE_ERROR_MAX + 1
    };
    
    typedef MRH_Uint32 OpCodeError; // OpCodeErrorList, set size for sending
    
    //*************************************************************************************
    // OpCode Data
    //*************************************************************************************
    
    extern "C"
    {
        /**
         *  Shared
         */
        
        // ALL_HEARTBEAT
        // No Data
        
        // ALL_DISCONNECT
        // No Data
        
        /**
         *  Speech Service
         */
        
        // SERVICE_CONNECT_REQUEST
        typedef struct SERVICE_CONNECT_REQUEST_DATA_t
        {
            MRH_Uint32 u32_OpCodeVersion;
            
            // Recording data format to request
            MRH_Uint32 u32_RecordingKHz;
            MRH_Uint32 u32_RecordingFrameElements;
            
            // Playback data format to request
            MRH_Uint32 u32_PlaybackKHz;
            MRH_Uint32 u32_PlaybackFrameElements;
            
        }SERVICE_CONNECT_REQUEST_DATA;
        
        // SERVICE_PAIR_REQUEST
        typedef struct SERVICE_PAIR_REQUEST_DATA_t
        {
            MRH_Uint32 u32_OpCodeVersion;
            
        }SERVICE_PAIR_REQUEST_DATA;
        
        // SERVICE_PAIR_CONNECTION_DETAILS
        typedef struct SERVICE_PAIR_CONNECTION_DETAILS_DATA_t
        {
            char p_SSID[AUDIO_STREAM_PAIR_SSID_SIZE]; // Encrypted with key, slimmed with base64
            char p_Password[AUDIO_STREAM_PAIR_PASSWORD_SIZE]; // Encrypted with key, slimmed with base64
            MRH_Uint8 u8_NetworkType;
            
        }SERVICE_PAIR_CONNECTION_DETAILS_DATA;
        
        // SERVICE_AUDIO_PLAYBACK_START
        // No Data
        
        // SERVICE_AUDIO_PLAYBACK_DATA
        // @NOTE: Has data, but is simply the audio buffer
        
        // SERVICE_AUDIO_PLAYBACK_END
        // No Data
        
        // SERVICE_DISABLE_PLAYBACK
        // No Data
        
        // SERVICE_DISABLE_RECORDING
        // No Data
        
        // SERVICE_ENABLE_PLAYBACK
        // No Data
        
        // SERVICE_ENABLE_RECORDING
        // No Data
        
        /**
         *  Audio Device
         */
        
        // DEVICE_CONNECT_RESPONSE
        typedef struct DEVICE_CONNECT_RESPONSE_DATA_t
        {
            // Connection error (0 if none)
            OpCodeError u32_Error;
            
            // Device capabilities
            MRH_Uint8 u8_CanRecord;
            MRH_Uint8 u8_CanPlay;
            
        }DEVICE_CONNECT_RESPONSE_DATA;
        
        // DEVICE_PAIR_RESPONSE
        typedef struct DEVICE_PAIR_RESPONSE_DATA_t
        {
            // Pairing error (0 if none)
            OpCodeError u32_Error;
            
        }DEVICE_PAIR_RESPONSE_DATA;
        
        // DEVICE_PAIR_GIVE_KEY
        typedef struct DEVICE_PAIR_GIVE_KEY_DATA_t
        {
            // The key to connect to this device
            char p_Key[AUDIO_STREAM_PAIR_KEY_SIZE];
            
        }DEVICE_PAIR_GIVE_KEY_DATA;
        
        // DEVICE_PAIR_GIVE_CONNECION
        typedef struct DEVICE_PAIR_GIVE_CONNECION_DATA_t
        {
            // Pairing error (0 if none)
            OpCodeError u32_Error;
            
            // Info for pairing
            char p_Name[AUDIO_STREAM_CONNECT_NAME_SIZE];
            char p_Address[AUDIO_STREAM_CONNECT_ADDRESS_SIZE];
            int i_Port;
            
        }DEVICE_PAIR_GIVE_CONNECION_DATA;
        
        // DEVICE_AUDIO_RECORDING_START
        // No Data
        
        // DEVICE_AUDIO_RECORDING_DATA
        // @NOTE: Has data, but is simply the audio buffer
        
        // DEVICE_AUDIO_RECORDING_END
        // No Data
        
        // DEVICE_PLAYBACK_STATE
        typedef struct DEVICE_PLAYBACK_STATE_DATA_t
        {
            // State change error (0 if none)
            OpCodeError u32_Error;
            
            // New State (0 disabled, 1 enabled)
            MRH_Uint8 u8_State;
            
        }DEVICE_PLAYBACK_STATE_DATA;
        
        // DEVICE_RECORDING_STATE
        typedef DEVICE_PLAYBACK_STATE_DATA DEVICE_RECORDING_STATE_DATA;
    }
}

#endif /* AudioDeviceOpCode_h */
