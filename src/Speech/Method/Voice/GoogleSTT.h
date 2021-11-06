/**
 *  GoogleSTT.h
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

#ifndef GoogleSTT_h
#define GoogleSTT_h

// C / C++
#include <string>
#include <utility>
#include <vector>

// External

// Project
#include "./AudioTrack.h"


class GoogleSTT
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    GoogleSTT() noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~GoogleSTT() noexcept;
    
    //*************************************************************************************
    // Reset
    //*************************************************************************************
    
    /**
     *  Clear speech to text audio buffer.
     */
    
    void ResetAudio() noexcept;
    
    //*************************************************************************************
    // Audio
    //*************************************************************************************
    
    /**
     *  Add mono audio data for speech to text.
     *
     *  \param c_Audio The audio to add.
     */
    
    void AddAudio(AudioTrack const& c_Audio) noexcept;
    
    //*************************************************************************************
    // Transcribe
    //*************************************************************************************
    
    /**
     *  Transcribe the currently added audio.
     *
     *  \return The transcription result string.
     */
    
    std::string Transcribe();
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if audio exists to transcribe.
     *
     *  \return true if audio exists, false if not.
     */
    
    bool GetAudioAvailable() noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::pair<MRH_Uint32, std::vector<MRH_Sint16>> c_Audio;
    
protected:
    
};

#endif /* GoogleSTT_h */
