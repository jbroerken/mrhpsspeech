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

#ifndef AudioBuffer_h
#define AudioBuffer_h

// C / C++
#include <vector>

// External
#include <MRH_Typedefs.h>

// Project
#include "../../../Exception.h"


class AudioBuffer
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param u32_KHz The audio buffer KHz.
     */
    
    AudioBuffer(MRH_Uint32 u32_KHz) noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~AudioBuffer() noexcept;
    
    //*************************************************************************************
    // Clear
    //*************************************************************************************
    
    /**
     *  Clear all audio chunks.
     *
     *  \param u32_KHz The KHz stored in the audio buffer.
     */
    
    void Clear(MRH_Uint32 u32_KHz) noexcept;
    
    //*************************************************************************************
    // Add
    //*************************************************************************************
    
    /**
     *  Add new audio to the track.
     *
     *  \param p_Buffer The audio buffer.
     *  \param us_Elements The elements in the audio buffer.
     */
    
    void AddAudio(const MRH_Sint16* p_Buffer, size_t us_Elements);
    
    /**
     *  Add new audio to the track.
     *
     *  \param v_Buffer The audio buffer.
     */
    
    void AddAudio(std::vector<MRH_Sint16> const& v_Buffer);
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the audio buffer.
     *
     *  \return The audio buffer.
     */
    
    const MRH_Sint16* GetBuffer() const noexcept;
    
    /**
     *  Get the amount of samples currently stored.
     *
     *  \return The currently stored sample count.
     */
    
    size_t GetSampleCount() const noexcept;
    
    /**
     *  Get the audio buffer Khz.
     *
     *  \return The audio buffer KHz.
     */
    
    MRH_Uint32 GetKHz() const noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::vector<MRH_Sint16> v_Samples;
    size_t us_SampleCount; // Buffer doesn't shrink, this defines samples
    
    MRH_Uint32 u32_KHz;
    
protected:
    
};

#endif /* AudioBuffer_h */
