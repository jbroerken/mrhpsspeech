/**
 *  MonoAudio.h
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

#ifndef MonoAudio_h
#define MonoAudio_h

// C / C++
#include <vector>

// External
#include <MRH_Typedefs.h>

// Project
#include "../../../Exception.h"


class MonoAudio
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_Buffer The sample data stored in this audio.
     *  \param us_Elements The length of the sample data to use in Sint16 elements.
     *  \param u32_KHz The sample KHz.
     */
    
    MonoAudio(const MRH_Sint16* p_Buffer,
              size_t us_Elements,
              MRH_Uint32 u32_KHz) noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~MonoAudio() noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::vector<MRH_Sint16> v_Buffer;
    
    MRH_Uint32 u32_KHz;
    MRH_Sfloat32 f32_Peak;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
protected:
    
};

#endif /* MonoAudio_h */