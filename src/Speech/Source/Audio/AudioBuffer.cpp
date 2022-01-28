/**
 *  AudioBuffer.cpp
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
#include <cstring>

// External

// Project
#include "./AudioBuffer.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioBuffer::AudioBuffer(MRH_Uint32 u32_KHz) noexcept : us_SampleCount(0),
                                                        u32_KHz(u32_KHz)
{}

AudioBuffer::~AudioBuffer() noexcept
{}

//*************************************************************************************
// Clear
//*************************************************************************************

void AudioBuffer::Clear(MRH_Uint32 u32_KHz) noexcept
{
    // Set the count, no vector modify
    us_SampleCount = 0;
    
    // Update KHz
    this->u32_KHz = u32_KHz;
}

//*************************************************************************************
// Add
//*************************************************************************************

void AudioBuffer::AddAudio(const MRH_Sint16* p_Buffer, size_t us_Elements)
{
    if (v_Samples.size() < (us_SampleCount + us_Elements))
    {
        try
        {
            v_Samples.insert(v_Samples.end(),
                             (us_SampleCount + us_Elements) - v_Samples.size(),
                             0);
        }
        catch (...)
        {
            throw;
        }
    }
    
    std::memcpy(&(v_Samples[us_SampleCount]),
                p_Buffer,
                us_Elements * sizeof(MRH_Sint16));
    
    us_SampleCount += us_Elements;
}

void AudioBuffer::AddAudio(std::vector<MRH_Sint16> const& v_Buffer)
{
    try
    {
        AddAudio(&(v_Buffer[0]), v_Buffer.size());
    }
    catch (...)
    {
        throw;
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

const MRH_Sint16* AudioBuffer::GetBuffer() const noexcept
{
    return &(v_Samples[0]);
}

size_t AudioBuffer::GetSampleCount() const noexcept
{
    return us_SampleCount;
}

MRH_Uint32 AudioBuffer::GetKHz() const noexcept
{
    return u32_KHz;
}
