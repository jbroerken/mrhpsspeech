/**
 *  MessageOpCode.cpp
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

// External

// Project
#include "./MessageOpCode.h"

// Pre-defined
using namespace MessageOpCode;


//*************************************************************************************
// OpCode Data - Base
//*************************************************************************************

OpCodeData::OpCodeData(std::vector<MRH_Uint8>& v_Data) noexcept
{
    this->v_Data.swap(v_Data);
}

OpCodeData::OpCodeData(MRH_Uint8 u8_OpCode) noexcept
{
    v_Data.insert(v_Data.end(), OPCODE_DATA_POS, 0);
    v_Data[OPCODE_OPCODE_POS] = u8_OpCode;
}

OpCodeData::~OpCodeData() noexcept
{}

MRH_Uint8 OpCodeData::GetOpCode() noexcept
{
    return MessageOpCode::GetOpCode(v_Data);
}

//*************************************************************************************
// OpCode Data - STRING_CS_STRING
//*************************************************************************************

STRING_CS_STRING_DATA::STRING_CS_STRING_DATA(std::vector<MRH_Uint8>& v_Data) noexcept : OpCodeData(v_Data)
{}

STRING_CS_STRING_DATA::STRING_CS_STRING_DATA(std::string const& s_String) noexcept : OpCodeData(STRING_CS_STRING)
{
    v_Data.insert(v_Data.end(),
                  s_String.begin(),
                  s_String.end());
}

STRING_CS_STRING_DATA::~STRING_CS_STRING_DATA() noexcept
{}

std::string STRING_CS_STRING_DATA::GetString() noexcept
{
    if (v_Data.size() < OPCODE_DATA_POS)
    {
        return "";
    }
    
    return std::string((const char*)&(v_Data[OPCODE_DATA_POS]),
                       v_Data.size() - OPCODE_OPCODE_SIZE);
}

//*************************************************************************************
// OpCode Data - AUDIO_CS_AUDIO
//*************************************************************************************

AUDIO_CS_AUDIO_DATA::AUDIO_CS_AUDIO_DATA(std::vector<MRH_Uint8>& v_Data) noexcept : OpCodeData(v_Data)
{}

AUDIO_CS_AUDIO_DATA::AUDIO_CS_AUDIO_DATA(const MRH_Sint16* p_Samples,
                                         MRH_Uint32 u32_Count) noexcept : OpCodeData(AUDIO_CS_AUDIO)
{
    v_Data.insert(v_Data.end(),
                  (const MRH_Uint8*)p_Samples,
                  (const MRH_Uint8*)p_Samples + (u32_Count * sizeof(MRH_Sint16)));
}

AUDIO_CS_AUDIO_DATA::~AUDIO_CS_AUDIO_DATA() noexcept
{}

const MRH_Sint16* AUDIO_CS_AUDIO_DATA::GetAudioBuffer() noexcept
{
    if (GetSampleCount() == 0)
    {
        return NULL;
    }
    
    return (const MRH_Sint16*)&(v_Data[OPCODE_DATA_POS + sizeof(MRH_Uint32)]);
}

MRH_Uint32 AUDIO_CS_AUDIO_DATA::GetSampleCount() noexcept
{
    return (v_Data.size() - OPCODE_OPCODE_SIZE) / sizeof(MRH_Sint16);
}

//*************************************************************************************
// OpCode Data - AUDIO_S_AUDIO_FORMAT
//*************************************************************************************

AUDIO_S_AUDIO_FORMAT_DATA::AUDIO_S_AUDIO_FORMAT_DATA(std::vector<MRH_Uint8>& v_Data) noexcept : OpCodeData(v_Data)
{}

AUDIO_S_AUDIO_FORMAT_DATA::AUDIO_S_AUDIO_FORMAT_DATA(MRH_Uint32 u32_RecordingKHz,
                                                     MRH_Uint32 u32_RecordingFrameSamples,
                                                     MRH_Uint32 u32_PlaybackKHz,
                                                     MRH_Uint32 u32_PlaybackFrameSamples) noexcept : OpCodeData(AUDIO_S_AUDIO_FORMAT)
{
    v_Data.insert(v_Data.end(),
                  sizeof(MRH_Uint32) * 4,
                  0);
    
    SetValue<MRH_Uint32>(v_Data, OPCODE_DATA_POS, &u32_RecordingKHz);
    SetValue<MRH_Uint32>(v_Data, OPCODE_DATA_POS + sizeof(MRH_Uint32), &u32_RecordingFrameSamples);
    SetValue<MRH_Uint32>(v_Data, OPCODE_DATA_POS + (sizeof(MRH_Uint32) * 2), &u32_PlaybackKHz);
    SetValue<MRH_Uint32>(v_Data, OPCODE_DATA_POS + (sizeof(MRH_Uint32) * 3), &u32_PlaybackFrameSamples);
}

AUDIO_S_AUDIO_FORMAT_DATA::~AUDIO_S_AUDIO_FORMAT_DATA() noexcept
{}

MRH_Uint32 AUDIO_S_AUDIO_FORMAT_DATA::GetRecordingKHz() noexcept
{
    if (v_Data.size() < OPCODE_DATA_POS + sizeof(MRH_Uint32))
    {
        return 0;
    }
    
    return GetValue<MRH_Uint32>(v_Data, OPCODE_DATA_POS);
}

MRH_Uint32 AUDIO_S_AUDIO_FORMAT_DATA::GetRecordingFrameSamples() noexcept
{
    if (v_Data.size() < OPCODE_DATA_POS + (sizeof(MRH_Uint32) * 2))
    {
        return 0;
    }
    
    return GetValue<MRH_Uint32>(v_Data, OPCODE_DATA_POS + sizeof(MRH_Uint32));
}

MRH_Uint32 AUDIO_S_AUDIO_FORMAT_DATA::GetPlaybackKHz() noexcept
{
    if (v_Data.size() < OPCODE_DATA_POS + (sizeof(MRH_Uint32) * 3))
    {
        return 0;
    }
    
    return GetValue<MRH_Uint32>(v_Data, OPCODE_DATA_POS + (sizeof(MRH_Uint32) * 2));
}

MRH_Uint32 AUDIO_S_AUDIO_FORMAT_DATA::GetPlaybackFrameSamples() noexcept
{
    if (v_Data.size() < OPCODE_DATA_POS + (sizeof(MRH_Uint32) * 4))
    {
        return 0;
    }
    
    return GetValue<MRH_Uint32>(v_Data, OPCODE_DATA_POS + (sizeof(MRH_Uint32) * 3));
}

//*************************************************************************************
// Getters
//*************************************************************************************

MRH_Uint8 MessageOpCode::GetOpCode(std::vector<MRH_Uint8> const& v_Data) noexcept
{
    return v_Data.size() > OPCODE_OPCODE_POS ? v_Data[OPCODE_OPCODE_POS] : UNK_CS_UNK;
}
