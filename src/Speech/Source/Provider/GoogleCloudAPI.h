/**
 *  GoogleCloudAPI.h
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

#ifndef GoogleCloudAPI_h
#define GoogleCloudAPI_h

// C / C++
#include <string>

// External

// Project
#include "../Audio/AudioBuffer.h"


namespace GoogleCloudAPI
{
    //*************************************************************************************
    // Transcribe
    //*************************************************************************************
    
    /**
     *  Transcribe audio to a string.
     *
     *  \param c_Audio The audio to transcribe.
     *  \param s_LangCode The language code for the transcription.
     *
     *  \return The transcription result string.
     */
    
    std::string Transcribe(AudioBuffer const& c_Audio, std::string s_LangCode);
    
    //*************************************************************************************
    // Synthesise
    //*************************************************************************************
    
    /**
     *  Synthesise a string to audio.
     *
     *  \param c_Audio The audio buffer to store the synthesized audio in.
     *  \param s_String The UTF-8 string to synthesise.
     *  \param s_LangCode The language code for the transcription.
     *  \param u8_VoiceGender The voice gender to use for spoken audio.
     */
    
    void Synthesise(AudioBuffer& c_Audio, std::string const& s_String, std::string s_LangCode, MRH_Uint8 u8_VoiceGender);
};

#endif /* GoogleCloudAPI_h */
