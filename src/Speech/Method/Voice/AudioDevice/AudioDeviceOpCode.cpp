/**
 *  AudioDeviceOpCode.cpp
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
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #include <byteswap.h>
#endif

// External

// Project
#include "./AudioDeviceOpCode.h"

// Pre-defined
using namespace AudioDeviceOpCode;


//*************************************************************************************
// OpCode Data - Base
//*************************************************************************************

OpCodeData::OpCodeData(std::vector<MRH_Uint8>& v_Data) noexcept
{
    this->v_Data.swap(v_Data);
}

OpCodeData::OpCodeData(OpCode u8_OpCode) noexcept
{
    v_Data.insert(v_Data.end(), 5, 0);
    v_Data[0] = u8_OpCode;
}

OpCodeData::~OpCodeData() noexcept
{}

OpCode OpCodeData::GetOpCode() noexcept
{
    return v_Data.size() > 0 ? v_Data[0] : ALL_UNK;
}

MRH_Uint32 OpCodeData::GetDataSize() noexcept
{
    if (v_Data.size() < 5)
    {
        return 0;
    }
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return bswap_32(GetValue<MRH_Sint32>(v_Data, 1));
#else
    return GetValue<MRH_Sint32>(v_Data, 1);
#endif
}

//*************************************************************************************
// OpCode Data - ALL_AUDIO
//*************************************************************************************

ALL_AUDIO_DATA::ALL_AUDIO_DATA(std::vector<MRH_Uint8>& v_Data) noexcept : OpCodeData(v_Data)
{}

ALL_AUDIO_DATA::ALL_AUDIO_DATA(const MRH_Sint16* p_Samples,
                               MRH_Uint32 u32_Count) noexcept : OpCodeData(ALL_AUDIO)
{
    v_Data.insert(v_Data.end(),
                  (const MRH_Uint8*)p_Samples,
                  (const MRH_Uint8*)p_Samples + (u32_Count * sizeof(MRH_Sint16)));
}

ALL_AUDIO_DATA::~ALL_AUDIO_DATA() noexcept
{}

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
void ALL_AUDIO_DATA::ConvertSamples() noexcept
{
    MRH_Sint16* p_Samples = (MRH_Sint16*)&(v_Data[5]);
    MRH_Uint32 u32_Elements = GetSampleCount();
    
    for (size_t i = 0; i < u32_Elements; ++i)
    {
        p_Samples[i] = (MRH_Sint16)((p_Samples[i] << 8) + (p_Samples[i] >> 8));
    }
}
#endif

const MRH_Sint16* ALL_AUDIO_DATA::GetAudioBuffer() noexcept
{
    if (GetSampleCount() == 0)
    {
        return NULL;
    }
    
    return (const MRH_Sint16*)&(v_Data[5]);
}

MRH_Uint32 ALL_AUDIO_DATA::GetSampleCount() noexcept
{
    return GetDataSize() / sizeof(MRH_Sint16);
}

//*************************************************************************************
// OpCode Data - SERVICE_CONNECT_RESPONSE_OK
//*************************************************************************************

SERVICE_CONNECT_RESPONSE_OK_DATA::SERVICE_CONNECT_RESPONSE_OK_DATA(std::vector<MRH_Uint8>& v_Data) noexcept : OpCodeData(v_Data)
{}

SERVICE_CONNECT_RESPONSE_OK_DATA::SERVICE_CONNECT_RESPONSE_OK_DATA(MRH_Uint32 u32_RecordingKHz,
                                                                   MRH_Uint32 u32_RecordingFrameElements,
                                                                   MRH_Uint32 u32_PlaybackKHz,
                                                                   MRH_Uint32 u32_PlaybackFrameElements) noexcept : OpCodeData(SERVICE_CONNECT_RESPONSE_OK)
{
    v_Data.insert(v_Data.end(), 16, 0); // OpCode already exist
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    u32_RecordingKHz = bswap_32(u32_RecordingKHz);
    u32_RecordingFrameElements = bswap_32(u32_RecordingFrameElements);
    u32_PlaybackKHz = bswap_32(u32_PlaybackKHz);
    u32_PlaybackFrameElements = bswap_32(u32_PlaybackFrameElements);
#endif
    
    SetValue<MRH_Uint32>(v_Data, 5, &u32_RecordingKHz);
    SetValue<MRH_Uint32>(v_Data, 9, &u32_RecordingFrameElements);
    SetValue<MRH_Uint32>(v_Data, 13, &u32_PlaybackKHz);
    SetValue<MRH_Uint32>(v_Data, 17, &u32_PlaybackFrameElements);
}

SERVICE_CONNECT_RESPONSE_OK_DATA::~SERVICE_CONNECT_RESPONSE_OK_DATA() noexcept
{}

MRH_Uint32 SERVICE_CONNECT_RESPONSE_OK_DATA::GetRecordingKHz() noexcept
{
    if (v_Data.size() < 9)
    {
        return 0;
    }
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return bswap_32(GetValue<MRH_Uint32>(v_Data, 5));
#else
    return GetValue<MRH_Uint32>(v_Data, 5);
#endif
}

MRH_Uint32 SERVICE_CONNECT_RESPONSE_OK_DATA::GetRecordingFrameSamples() noexcept
{
    if (v_Data.size() < 13)
    {
        return 0;
    }
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return bswap_32(GetValue<MRH_Uint32>(v_Data, 9));
#else
    return GetValue<MRH_Uint32>(v_Data, 9);
#endif
}

MRH_Uint32 SERVICE_CONNECT_RESPONSE_OK_DATA::GetPlaybackKHz() noexcept
{
    if (v_Data.size() < 17)
    {
        return 0;
    }
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return bswap_32(GetValue<MRH_Uint32>(v_Data, 13));
#else
    return GetValue<MRH_Uint32>(v_Data, 13);
#endif
}

MRH_Uint32 SERVICE_CONNECT_RESPONSE_OK_DATA::GetPlaybackFrameSamples() noexcept
{
    if (v_Data.size() < 21)
    {
        return 0;
    }
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return bswap_32(GetValue<MRH_Uint32>(v_Data, 17));
#else
    return GetValue<MRH_Uint32>(v_Data, 17);
#endif
}

//*************************************************************************************
// OpCode Data - DEVICE_PAIR_REQUEST
//*************************************************************************************

DEVICE_CONNECT_REQUEST_DATA::DEVICE_CONNECT_REQUEST_DATA(std::vector<MRH_Uint8>& v_Data) noexcept : OpCodeData(v_Data)
{}

DEVICE_CONNECT_REQUEST_DATA::DEVICE_CONNECT_REQUEST_DATA(std::string const& s_Address,
                                                         MRH_Uint16 u16_Port,
                                                         MRH_Uint8 u8_OpCodeVersion,
                                                         bool b_CanRecord,
                                                         bool b_CanPlay) noexcept : OpCodeData(DEVICE_CONNECT_REQUEST)
{
    size_t us_Length = s_Address.size();
    
    v_Data.insert(v_Data.end(),
                  us_Length + 5, // Added to existing OpCode byte
                  '\0');
    
    MRH_Uint8 u8_CanRecord = b_CanRecord ? AUDIO_DEVICE_BOOL_TRUE : AUDIO_DEVICE_BOOL_FALSE;
    MRH_Uint8 u8_CanPlay = b_CanPlay ? AUDIO_DEVICE_BOOL_TRUE : AUDIO_DEVICE_BOOL_FALSE;
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    u16_Port = bswap_16(u16_Port);
#endif
    
    SetValue<MRH_Uint16>(v_Data, 5, &u16_Port);
    SetValue<MRH_Uint8>(v_Data, 7, &u8_OpCodeVersion);
    SetValue<MRH_Uint8>(v_Data, 8, &u8_CanRecord);
    SetValue<MRH_Uint8>(v_Data, 9, &u8_CanPlay);
    
    if (s_Address.size() > 0)
    {
        std::memcpy(&(v_Data[10]),
                    s_Address.data(),
                    us_Length);
    }
}

DEVICE_CONNECT_REQUEST_DATA::~DEVICE_CONNECT_REQUEST_DATA() noexcept
{}

std::string DEVICE_CONNECT_REQUEST_DATA::GetAddress() noexcept
{
    if (v_Data.size() <= 10)
    {
        return "";
    }
    
    return std::string((const char*)&(v_Data[10]),
                       (const char*)&(v_Data[v_Data.size() - 1]));
}

MRH_Uint16 DEVICE_CONNECT_REQUEST_DATA::GetPort() noexcept
{
    if (v_Data.size() < 7)
    {
        return 0;
    }
    
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return bswap_16(GetValue<MRH_Uint32>(v_Data, 5));
#else
    return GetValue<MRH_Uint16>(v_Data, 5);
#endif
}

MRH_Uint8 DEVICE_CONNECT_REQUEST_DATA::GetOpCodeVersion() noexcept
{
    if (v_Data.size() < 8)
    {
        return 0;
    }
    
    return v_Data[7];
}

bool DEVICE_CONNECT_REQUEST_DATA::GetCanRecord() noexcept
{
    if (v_Data.size() < 9)
    {
        return false;
    }
    
    return v_Data[8] == AUDIO_DEVICE_BOOL_TRUE ? true : false;
}

bool DEVICE_CONNECT_REQUEST_DATA::GetCanPlay() noexcept
{
    if (v_Data.size() < 10)
    {
        return false;
    }
    
    return v_Data[9] == AUDIO_DEVICE_BOOL_TRUE ? true : false;
}
