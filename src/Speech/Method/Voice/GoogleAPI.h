/**
 *  GoogleAPI.h
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

#ifndef GoogleAPI_h
#define GoogleAPI_h

// C / C++

// External

// Project
#include "./AudioSample.h"


class GoogleAPI
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    GoogleAPI() noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~GoogleAPI() noexcept;
    
    //*************************************************************************************
    // Speech to Text
    //*************************************************************************************
    
    /**
     *  Add mono audio data for speech to text.
     *
     *  \param p_Buffer The audio buffer to add.
     *  \param us_Length The length of the data buffer.
     *  \param u32_KHz The audio buffer KHz.
     */
    
    void AddAudio(const MRH_Sint16* p_Buffer, size_t us_Length, MRH_Uint32 u32_KHz) noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    AudioSample c_Sample;
    
protected:
    
};

#endif /* GoogleAPI_h */
