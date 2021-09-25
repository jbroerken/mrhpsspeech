/**
 *  RateConverter.cpp
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
#include <limits.h>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./RateConverter.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

RateConverter::RateConverter(MRH_Uint32 u32_SourceKHz) : p_State(NULL),
                                                         u32_SourceKHz(u32_SourceKHz)
{
    int i_Error;
    
    p_State = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &i_Error);
    
    if (i_Error != 0)
    {
        throw Exception("Failed to reset rate converter: " + std::string(src_strerror(i_Error)));
    }
}

RateConverter::~RateConverter() noexcept
{
    if (p_State != NULL)
    {
        src_delete(p_State);
    }
}

//*************************************************************************************
// Convert
//*************************************************************************************

void RateConverter::Reset()
{
    int i_Error = src_reset(p_State);
    
    if (i_Error)
    {
        throw Exception("Failed to reset rate converter: " + std::string(src_strerror(i_Error)));
    }
}

std::vector<MRH_Sint16> RateConverter::Convert(std::vector<MRH_Sint16> const& v_Buffer, MRH_Uint32 u32_KHz)
{
    if (v_Buffer.size() == 0 || u32_SourceKHz == u32_KHz)
    {
        return v_Buffer;
    }
    
    int i_Error;
    
    // Set ratio first
    double f64_Ratio = static_cast<double>(u32_SourceKHz) / static_cast<double>(u32_KHz);
    
    if ((i_Error = src_set_ratio(p_State, f64_Ratio)) != 0)
    {
        throw Exception("Failed to convert: " + std::string(src_strerror(i_Error)));
    }
    
    // Convert data
    std::vector<MRH_Sint16> v_Result;
    size_t us_InSize = v_Buffer.size();
    size_t us_InPos = 0;
    size_t us_OutPos = 0;
    int i_CVTSize;
    
    while (us_InPos < us_InSize)
    {
        i_CVTSize = (us_InSize - us_InPos) >= INT_MAX ? INT_MAX : static_cast<int>(us_InSize - us_InPos);
        MRH_Sfloat32 p_In[i_CVTSize];
        MRH_Sfloat32 p_Out[i_CVTSize];
        SRC_DATA c_CVTInfo;
        
        src_short_to_float_array(&(v_Buffer[us_InPos]), p_In, i_CVTSize);
        
        c_CVTInfo.data_in = p_In;
        c_CVTInfo.data_out = p_Out;
        c_CVTInfo.input_frames = i_CVTSize;
        c_CVTInfo.output_frames = i_CVTSize;
        c_CVTInfo.src_ratio = f64_Ratio;
        
        if ((i_Error = src_simple(&c_CVTInfo, SRC_SINC_BEST_QUALITY, 1)) != 0)
        {
            throw Exception("Failed to convert: " + std::string(src_strerror(i_Error)));
        }
        
        us_OutPos = v_Result.size();
        v_Result.insert(v_Result.end(), c_CVTInfo.output_frames_gen, 0);
        
        src_float_to_short_array(p_Out, &(v_Result[us_OutPos]), c_CVTInfo.output_frames_gen);
        
        us_InPos += i_CVTSize; // Increment based on input
    }
    
    return v_Result;
}
