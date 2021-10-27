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
#define AUDIO_DEVICE_OPCODE_VERSION 1

#define AUDIO_DEVICE_BOOL_TRUE 1
#define AUDIO_DEVICE_BOOL_FALSE 0

#define AUDIO_DEVICE_KEYPHRASE_SIZE 1024
#define AUDIO_DEVICE_DEVICE_NAME_SIZE 1024
#define AUDIO_DEVICE_SERVICE_ADDRESS_SIZE 512
#define AUDIO_DEVICE_SERVICE_KEY_SIZE 1024
#define AUDIO_DEVICE_PUBLIC_KEY_SIZE 1024
#define AUDIO_DEVICE_NETWORK_SSID_SIZE 1024
#define AUDIO_DEVICE_NETWORK_PASSWORD_SIZE 1024


namespace AudioDeviceOpCode
{
    //*************************************************************************************
    // OpCodes
    //*************************************************************************************
    
    enum OpCodeList
    {
        // Shared
        ALL_HEARTBEAT = 0,                      // Signal availability to partner
        
        // Speech Service
        SERVICE_CONNECT_RESPONSE = 1,           // Connect to device
        
        SERVICE_PAIR_RESPONSE = 2,              // Responst to a device pairing request with connection data, encrypted
        
        SERVICE_PLAYBACK_AUDIO = 3,             // Playback data buffer
        
        SERVICE_START_RECORDING = 4,            // Signal recording start to device
        SERVICE_STOP_RECORDING = 5,             // Signal recording stop to device.
        
        // Audio Device
        DEVICE_CONNECT_REQUEST = 6,             // Connection response of service request
        
        DEVICE_PAIR_REQUEST = 7,                // Request pairing with the service (Hand key and info)
        
        DEVICE_RECORDED_AUDIO,                  // Recording data buffer
        
        // Bounds
        OPCODE_MAX = DEVICE_RECORDED_AUDIO,
        
        OPCODE_COUNT = OPCODE_MAX + 1
    };
    
    typedef MRH_Uint8 OpCode; // OpCodeList, set size for sending
    
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
        
        // Bounds
        OPCODE_ERROR_MAX = ALREADY_PAIRED,
        
        OPCODE_ERROR_COUNT = OPCODE_ERROR_MAX + 1
    };
    
    typedef MRH_Uint32 OpCodeError; // OpCodeErrorList, set size for sending
    
    //*************************************************************************************
    // OpCode Data
    //*************************************************************************************
    
    // @NOTE: All data uses little endian!
    
    extern "C"
    {
        /**
         *  Shared
         */
        
        // ALL_HEARTBEAT
        // No Data
        
        /**
         *  Speech Service
         */
        
        // SERVICE_CONNECT_REQUEST
        typedef struct SERVICE_CONNECT_REQUEST_DATA_t
        {
            // Recording data format to use
            MRH_Uint32 u32_RecordingKHz;
            MRH_Uint32 u32_RecordingFrameElements;
            
            // Playback data format to use
            MRH_Uint32 u32_PlaybackKHz;
            MRH_Uint32 u32_PlaybackFrameElements;
            
        }SERVICE_CONNECT_REQUEST_DATA;
        
        // SERVICE_PAIR_RESPONSE
        typedef struct SERVICE_PAIR_RESPONSE_DATA_t
        {
            // Trigger Info
            char p_TriggerKeyphrase[AUDIO_DEVICE_KEYPHRASE_SIZE];
            MRH_Uint32 u32_TriggerTimeoutS;
            
            // Service Info
            char p_Address[AUDIO_DEVICE_SERVICE_ADDRESS_SIZE];
            int i_Port;
            char p_ServiceKey[AUDIO_DEVICE_SERVICE_KEY_SIZE];
            
            // Network Info
            char p_SSID[AUDIO_DEVICE_NETWORK_SSID_SIZE]; // Encrypted with key, slimmed with base64
            char p_Password[AUDIO_DEVICE_NETWORK_PASSWORD_SIZE]; // Encrypted with key, slimmed with base64
            MRH_Uint8 u8_NetworkType;
            
            // Device Info
            MRH_Uint8 u8_DeviceID; // Used for recognition
            
        }SERVICE_PAIR_RESPONSE_DATA;
        
        // SERVICE_PLAYBACK_AUDIO_DATA
        // @NOTE: Has data, but is simply the audio buffer
        
        // SERVICE_START_RECORDING
        // No Data
        
        // SERVICE_STOP_RECORDING
        // No Data
        
        /**
         *  Audio Device
         */
        
        // DEVICE_CONNECT_REQUEST
        typedef struct DEVICE_CONNECT_REQUEST_DATA_t
        {
            MRH_Uint32 u32_OpCodeVersion;
            
            // Device Info
            char p_DeviceName[AUDIO_DEVICE_DEVICE_NAME_SIZE];
            char p_ServiceKey[AUDIO_DEVICE_SERVICE_KEY_SIZE];
            MRH_Uint8 u8_DeviceID;
            
            // Device capabilities
            MRH_Uint8 u8_CanRecord;
            MRH_Uint8 u8_CanPlay;
            
        }DEVICE_CONNECT_REQUEST_DATA;
        
        // DEVICE_PAIR_REQUEST
        typedef struct DEVICE_PAIR_REQUEST_DATA_t
        {
            // OpCode version in use
            MRH_Uint32 u32_OpCodeVersion;

            // The key to encrypt data with
            char p_PublicKey[AUDIO_DEVICE_PUBLIC_KEY_SIZE];
            
            // Device Info
            char p_DeviceName[AUDIO_DEVICE_DEVICE_NAME_SIZE];
            
        }DEVICE_PAIR_REQUEST_DATA;
        
        // DEVICE_RECORDED_AUDIO_DATA
        // @NOTE: Has data, but is simply the audio buffer
    }
}

#endif /* AudioDeviceOpCode_h */
