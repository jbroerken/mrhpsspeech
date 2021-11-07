/**
 *  AudioTrack.h
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

#ifndef AudioTrack_h
#define AudioTrack_h

// C / C++
#include <vector>

// External
#include <MRH_Typedefs.h>

// Project
#include "../../../Exception.h"


class AudioTrack
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param u32_KHz The audio track KHz for all chunks.
     *  \param u8_StorageSizeS The initial amount of audio storable in seconds.
     *  \param b_CanGrow If the track can be expanded beyond the initial size.
     */
    
    AudioTrack(MRH_Uint32 u32_KHz,
               MRH_Uint8 u8_StorageSizeS,
               bool b_CanGrow);
    
    /**
     *  Default destructor.
     */
    
    ~AudioTrack() noexcept;
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Clear all audio chunks.
     */
    
    void Clear() noexcept;
    
    /**
     *  Add new audio to the track.
     *
     *  \param p_Buffer The audio buffer.
     *  \param us_Elements The elements in the audio buffer.
     */
    
    void AddAudio(const MRH_Sint16* p_Buffer, size_t us_Elements);
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the audio buffer.
     *
     *  \return The audio buffer.
     */
    
    MRH_Sint16* GetBuffer() noexcept;
    
    /**
     *  Get the const audio buffer.
     *
     *  \return The const audio buffer.
     */
    
    const MRH_Sint16* GetBufferConst() const noexcept;
    
    /**
     *  Get the amount of samples currently stored.
     *
     *  \return The currently stored sample count.
     */
    
    size_t GetSampleCount() const noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    const MRH_Uint32 u32_KHz;
    const bool b_CanGrow;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::vector<MRH_Sint16> v_Samples;
    size_t us_SampleCount;
    
protected:
    
};

#endif /* AudioTrack_h */
