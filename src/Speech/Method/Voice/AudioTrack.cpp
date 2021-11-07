/**
 *  AudioTrack.cpp
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
#include <cmath>
#include <cstring>

// External

// Project
#include "./AudioTrack.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioTrack::AudioTrack(MRH_Uint32 u32_KHz,
                       MRH_Uint8 u8_StorageSizeS,
                       bool b_CanGrow) : u32_KHz(u32_KHz),
                                         b_CanGrow(b_CanGrow)
{
    // Create all chunks needed
    size_t us_TotalSize = u8_StorageSizeS * u32_KHz;
    
    v_Samples.insert(v_Samples.end(),
                     us_TotalSize,
                     0);
}

AudioTrack::~AudioTrack() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void AudioTrack::Clear() noexcept
{
    // Set the count, no vector modify
    us_SampleCount = 0;
}

void AudioTrack::AddAudio(const MRH_Sint16* p_Buffer, size_t us_Elements)
{
    if (v_Samples.size() < us_Elements)
    {
        if (b_CanGrow == true)
        {
            v_Samples.insert(v_Samples.end(),
                             us_Elements - v_Samples.size(),
                             0);
        }
        else
        {
            throw Exception("Audio buffer missing space for new audio!");
        }
    }
    
    std::memcpy(&(v_Samples[us_SampleCount]),
                p_Buffer,
                us_Elements * sizeof(MRH_Sint16));
    
    us_SampleCount += us_Elements;
}

//*************************************************************************************
// Getters
//*************************************************************************************

MRH_Sint16* AudioTrack::GetBuffer() noexcept
{
    return &(v_Samples[0]);
}

const MRH_Sint16* AudioTrack::GetBufferConst() const noexcept
{
    return &(v_Samples[0]);
}

size_t AudioTrack::GetSampleCount() const noexcept
{
    return us_SampleCount;
}
