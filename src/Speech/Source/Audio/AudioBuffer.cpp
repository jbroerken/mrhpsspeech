/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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
