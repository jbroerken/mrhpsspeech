/**
 *  RateConverter.h
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

#ifndef RateConverter_h
#define RateConverter_h

// C / C++
#include <vector>

// External
#include <MRH_Typedefs.h>
#include <samplerate.h>

// Project
#include "../../../Exception.h"


class RateConverter
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param u32_SourceKHz The KHz to convert from.
     */
    
    RateConverter(MRH_Uint32 u32_SourceKHz);
    
    /**
     *  Default destructor.
     */
    
    ~RateConverter() noexcept;
    
    //*************************************************************************************
    // Convert
    //*************************************************************************************
    
    /**
     *  Reset the converter.
     */
    
    void Reset();
    
    /**
     *  Convert a audio sample.
     *
     *  \param v_Buffer The audio buffer to convert.
     *  \param u32_KHz The KHz to convert to.
     *
     *  \return The converted audio data.
     */
    
    std::vector<MRH_Sint16> Convert(std::vector<MRH_Sint16> const& v_Buffer, MRH_Uint32 u32_KHz);
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    SRC_STATE* p_State;
    
    MRH_Uint32 u32_SourceKHz;
    
protected:
    
};

#endif /* RateConverter_h */
