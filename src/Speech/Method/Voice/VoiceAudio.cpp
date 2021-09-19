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
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Creating empty sample!",
                                       "VoiceAudio.cpp", __LINE__);
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

//*************************************************************************************
// Convert
//*************************************************************************************

std::vector<MRH_Sint16> VoiceAudio::Convert(size_t us_Pos, size_t us_Elements, MRH_Uint32 u32_KHz) const
{
    // Check end pos for conversion
    size_t us_End = us_Pos + us_Elements;
    
    if (us_End > v_Buffer.size())
    {
        us_Elements = v_Buffer.size() - us_Pos;
        us_End = v_Buffer.size();
    }
    
    if (us_Elements == 0)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Returning empty sample!",
                                       "VoiceAudio.cpp", __LINE__);
        return {};
    }
    
    // Convert to float
    MRH_Sfloat32 p_In[us_Elements];
    MRH_Sfloat32 p_Out[us_Elements];
    size_t us_DstPos = 0;
    
    for (; us_Pos < us_End; ++us_Pos, ++us_DstPos)
    {
        p_In[us_DstPos] = static_cast<MRH_Sfloat32>(v_Buffer[us_Pos] / 32768.f);
    }
    
    // Convert
    SRC_DATA c_CVTInfo;
    c_CVTInfo.data_in = p_In;
    c_CVTInfo.data_out = p_Out;
    c_CVTInfo.input_frames = us_Elements;
    c_CVTInfo.output_frames = us_Elements;
    c_CVTInfo.src_ratio = static_cast<double>(u32_KHz / this->u32_KHz);
    
    if (src_simple(&c_CVTInfo, SRC_SINC_MEDIUM_QUALITY, 1) != 0)
    {
        throw Exception("Failed to convert audio sample!");
    }
    
    // Create result
    std::vector<MRH_Sint16> v_Result(0, c_CVTInfo.output_frames_gen);
    size_t us_ResultElements = c_CVTInfo.output_frames_gen;
    us_Pos = 0;
    
    for (; us_Pos < us_ResultElements; ++us_Pos)
    {
        v_Result[us_Pos] = static_cast<MRH_Sint16>(p_Out[us_Pos] * 32768);
    }
    
    return v_Result;
}
