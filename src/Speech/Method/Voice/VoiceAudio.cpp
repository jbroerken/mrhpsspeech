/**
 *  VoiceAudio.cpp
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

// External
#include <portaudio.h>

// Project
#include "./VoiceAudio.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

VoiceAudio::VoiceAudio(const MRH_Sint16* p_Buffer,
                       size_t us_Length,
                       MRH_Uint32 u32_KHz,
                       MRH_Uint8 u8_Channels,
                       MRH_Uint32 u32_FrameSamples) noexcept : u32_KHz(u32_KHz),
                                                               u8_Channels(u8_Channels),
                                                               u32_FrameSamples(u32_FrameSamples),
                                                               f32_Amplitude(0.f),
                                                               f32_Peak(0.f)
{
    // Insert data first for calculation
    if (us_Length == 0)
    {
        return;
    }
    
    v_Buffer.insert(v_Buffer.end(),
                    p_Buffer,
                    p_Buffer + us_Length);
    
    // Calculate peak and amplitude
    std::vector<MRH_Sfloat32> v_RMS(u8_Channels, 0.f);
    std::vector<MRH_Sfloat32>::iterator RMS = v_RMS.begin();
    float f32_Value;

    for (unsigned long i = 0; i < us_Length; ++i)
    {
        // Get float value first
        f32_Value = v_Buffer[i] / 32768.f;
        f32_Value = fabs(f32_Value);
        
        // Check the current peak (Global for all channels)
        if (f32_Value > f32_Peak)
        {
            f32_Peak = f32_Value;
        }
        
        // Grab RMS for this channel
        *RMS += f32_Value * f32_Value;
        
        if ((++RMS) == v_RMS.end())
        {
            RMS = v_RMS.begin();
        }
    }
    
    // Grab max amplitude
    for (auto& Amplitude : v_RMS)
    {
        if (f32_Amplitude < Amplitude)
        {
            f32_Amplitude = Amplitude;
        }
    }
}

VoiceAudio::~VoiceAudio() noexcept
{}

//*************************************************************************************
// Convert
//*************************************************************************************

std::vector<MRH_Sint16> VoiceAudio::Convert(size_t us_Pos, size_t us_Length, MRH_Uint32 u32_KHz, MRH_Uint8 u8_Channels) noexcept
{
    return std::vector<MRH_Sint16>(v_Buffer.data() + us_Pos,
                                   v_Buffer.data() + (us_Pos + us_Length));
}
