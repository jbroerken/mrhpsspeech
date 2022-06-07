/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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
