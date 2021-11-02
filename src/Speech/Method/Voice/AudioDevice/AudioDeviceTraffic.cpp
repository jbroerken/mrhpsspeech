/**
 *  AudioDeviceTraffic.cpp
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

// C / C++
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <limits.h>
#include <cstring>
#include <cmath>
#include <ctime>
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #include <byteswap.h>
#endif

// External
#include <libmrhpsb/MRH_PSBLogger.h>
//#include <lsquic.h>

// Project
#include "./AudioDeviceTraffic.h"
#include "./AudioDeviceOpCode.h"
#include "../../../../Configuration.h"

// Pre-defined
#define DEVICE_TRAFFIC_SOCKET_DISCONNECTED -1

#define DEVICE_TRAFFIC_SERVICE_KEY_SIZE 50

#define DEVICE_TRAFFIC_OPCODE_DATA_SIZE_MIN 5 // 1 OpCode byte + 4 bytes size
#define DEVICE_TRAFFIC_MESSAGE_DATA_SIZE_MIN DEVICE_TRAFFIC_SERVICE_KEY_SIZE + DEVICE_TRAFFIC_OPCODE_DATA_SIZE_MIN


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioDeviceTraffic::AudioDeviceTraffic() : b_Update(true)
{
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    
    // Grab service key
    s_ServiceKey = Configuration::Singleton().GetDeviceConnectionServiceKey();
    
    if (s_ServiceKey.size() != DEVICE_TRAFFIC_SERVICE_KEY_SIZE)
    {
        throw Exception("Invalid service key length!");
    }
    
    // Init lsquic
    /*
    if (lsquic_global_init(LSQUIC_GLOBAL_SERVER) != 0)
    {
        throw Exception("Failed to initialize lsquic!");
    }
    */
    
    // Ready, start thread update
    try
    {
        c_Thread = std::thread(AudioDeviceTraffic::Update, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start audio device traffic threads!" + std::string(e.what()));
    }
}

AudioDeviceTraffic::~AudioDeviceTraffic() noexcept
{
    // Stop thread
    b_Update = false;
    c_Thread.join();
    
    // Reset lsquic
    /*
    lsquic_global_cleanup();
    */
}

//*************************************************************************************
// Clear
//*************************************************************************************

void AudioDeviceTraffic::ClearRecieved(std::string const& s_Address, int i_Port) noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_ReadMutex);
    
    for (auto It = l_Read.begin(); It != l_Read.end();)
    {
        if (It->s_Address.compare(s_Address) == 0 &&
            It->i_Port == i_Port)
        {
            It = l_Read.erase(It);
        }
        else
        {
            ++It;
        }
    }
}

//*************************************************************************************
// Update
//*************************************************************************************

void AudioDeviceTraffic::Update(AudioDeviceTraffic* p_Instance) noexcept
{
    
}

//*************************************************************************************
// Recieve
//*************************************************************************************

bool AudioDeviceTraffic::Recieve(std::string const& s_Address, int i_Port, std::vector<MRH_Uint8>& v_Data)
{
    return false;
}

bool AudioDeviceTraffic::Recieve(MRH_Uint8 u8_OpCode, std::vector<MRH_Uint8>& v_Data)
{
    return false;
}

//*************************************************************************************
// Send
//*************************************************************************************

void AudioDeviceTraffic::Send(std::string const& s_Address, int i_Port, std::vector<MRH_Uint8>& v_Data)
{
    // Minimal data size met?
    if (v_Data.size() < DEVICE_TRAFFIC_OPCODE_DATA_SIZE_MIN)
    {
        throw Exception("Data does not meet minimum requirements!");
    }
    
    // Build message
    Message c_Message;
    
    c_Message.s_Address = s_Address;
    c_Message.i_Port = i_Port;
    
    c_Message.v_Payload.swap(v_Data);
    c_Message.v_Payload.insert(c_Message.v_Payload.end(), /* Service key identifies pairing! */
                               s_ServiceKey.begin(),
                               s_ServiceKey.end());
    
    // Now add to list for thread to grab
    std::lock_guard<std::mutex> c_Guard(c_WriteMutex);
    l_Write.push_back(std::move(c_Message));
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool AudioDeviceTraffic::GetMessageFinished(std::vector<MRH_Uint8> const& v_Data) noexcept
{
    if (v_Data.size() < DEVICE_TRAFFIC_MESSAGE_DATA_SIZE_MIN)
    {
        return false;
    }
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    MRH_Uint32 u32_TotalDataSize = bswap_32(AudioDeviceOpCode::GetValue<MRH_Uint32>(v_Data, 1));
#else
    MRH_Uint32 u32_TotalDataSize = AudioDeviceOpCode::GetValue<MRH_Uint32>(v_Data, 1);
#endif
    
    if (v_Data.size() < u32_TotalDataSize + DEVICE_TRAFFIC_MESSAGE_DATA_SIZE_MIN)
    {
        return false;
    }
    
    return false;
}
