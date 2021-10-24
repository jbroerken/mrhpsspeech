/**
 *  GoogleTTS.h
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

#ifndef GoogleTTS_h
#define GoogleTTS_h

// C / C++
#include <string>

// External

// Project
#include "./AudioTrack.h"


class GoogleTTS
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    GoogleTTS();
    
    /**
     *  Default destructor.
     */
    
    ~GoogleTTS() noexcept;
    
    //*************************************************************************************
    // Synthesise
    //*************************************************************************************
    
    /**
     *  Synthesise a string to audio.
     *
     *  \param s_String The UTF-8 string to synthesise.
     */
    
    AudioTrack const& Synthesise(std::string const& s_String);
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    AudioTrack c_Audio; // Result
    
protected:
    
};

#endif /* GoogleTTS_h */
