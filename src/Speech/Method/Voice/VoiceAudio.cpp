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
#include <samplerate.h>
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./VoiceAudio.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

VoiceAudio::VoiceAudio(const MRH_Sint16* p_Buffer,
                       size_t us_Elements,
                       MRH_Uint32 u32_KHz) noexcept : u32_KHz(u32_KHz),
                                                      f32_Peak(0.f)
{
    // Insert data first for calculation
    if (us_Elements == 0)
    {
        return;
    }
    
    v_Buffer.insert(v_Buffer.end(),
                    p_Buffer,
                    p_Buffer + (us_Elements * sizeof(MRH_Sint16)));
    
    // Calculate peak amplitude
    us_Elements = v_Buffer.size();
    float f32_Value;

    for (unsigned long i = 0; i < us_Elements; ++i)
    {
        // Get float value first
        f32_Value = v_Buffer[i] / 32768.f;
        f32_Value = fabs(f32_Value);
        
        // Check the current peak
        if (f32_Value > f32_Peak)
        {
            f32_Peak = f32_Value;
        }
    }
}

VoiceAudio::~VoiceAudio() noexcept
{}
