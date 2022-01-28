/**
 *  AudioBuffer.h
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
