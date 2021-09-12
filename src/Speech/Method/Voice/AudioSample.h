/**
 *  AudioSample.h
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

#ifndef AudioSample_h
#define AudioSample_h

// C / C++
#include <vector>

// External
#include <MRH_Typedefs.h>

// Project
#include "../../../Exception.h"


class AudioSample
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_Buffer The sample data stored in this sample.
     *  \param us_Length The length of the sample data to use, starting at 0.
     *  \param u32_KHz The sample KHz.
     *  \param u8_Channels The channels used in this sample.
     *  \param u32_Samples The samples per frame.
     */
    
    AudioSample(const MRH_Sint16* p_Buffer,
                size_t us_Length,
                MRH_Uint32 u32_KHz,
                MRH_Uint8 u8_Channels,
                MRH_Uint32 u32_Samples) noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~AudioSample() noexcept;
    
    //*************************************************************************************
    // Convert
    //*************************************************************************************
    
    /**
     *  Convert the sample to a target format.
     *
     *  \param us_Pos The offset of the full buffer to convert from.
     *  \param us_Length The length of data to convert.
     *  \param u32_KHz The target KHz.
     *  \param u8_Channels The target channels.
     *
     *  \return The converted sample data.
     */
    
    std::vector<MRH_Sint16> Convert(size_t us_Pos, size_t us_Length, MRH_Uint32 u32_KHz, MRH_Uint8 u8_Channels) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::vector<MRH_Sint16> v_Buffer;
    
    MRH_Uint32 u32_KHz;
    MRH_Uint8 u8_Channels;
    MRH_Uint32 u32_Samples;
    
    MRH_Sfloat32 f32_Amplitude;
    MRH_Sfloat32 f32_Peak;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
protected:
    
};

#endif /* AudioSample_h */
