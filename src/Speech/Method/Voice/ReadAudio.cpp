/**
 *  ReadAudio.cpp
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
#include <clocale>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./ReadAudio.h"

// Pre-defined
namespace
{
    std::string s_DefaultLocale = "en_US";
}

#ifndef MRH_SPEECH_SPHINX_TRIGGER_PATH
    #define MRH_SPEECH_SPHINX_TRIGGER_PATH "/var/mrh/mrhpsspeech/sphinx/trigger/"
#endif
#define MRH_SPEECH_SPHINX_TRIGGER_LM_EXT ".lm.bin"
#define MRH_SPEECH_SPHINX_TRIGGER_DICT_EXT "-dict.dict"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

ReadAudio::ReadAudio() : p_Decoder(NULL),
                         p_Config(NULL),
                         s_Locale(s_DefaultLocale)
{
    if (std::setlocale(LC_CTYPE, NULL) == NULL)
    {
        s_Locale = std::setlocale(LC_CTYPE, NULL);
        s_Locale = s_Locale.substr(0, s_Locale.find_first_of('.'));
    }
    
    std::string s_HMM = MRH_SPEECH_SPHINX_TRIGGER_PATH + s_Locale + "/" + s_Locale;
    std::string s_LM = MRH_SPEECH_SPHINX_TRIGGER_PATH + s_Locale + "/" + s_Locale + MRH_SPEECH_SPHINX_TRIGGER_LM_EXT;
    std::string s_Dict = MRH_SPEECH_SPHINX_TRIGGER_PATH + s_Locale + "/" + s_Locale + MRH_SPEECH_SPHINX_TRIGGER_DICT_EXT;
    
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "ReadAudio HMM: " + s_HMM,
                                   "ReadAudio.cpp", __LINE__);
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "ReadAudio LM: " + s_LM,
                                   "ReadAudio.cpp", __LINE__);
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "ReadAudio Dict: " + s_Dict,
                                   "ReadAudio.cpp", __LINE__);

    if ((p_Config = cmd_ln_init(NULL, ps_args(), TRUE,
                                "-hmm", s_HMM.c_str(),
                                "-lm", s_LM.c_str(),
                                "-dict", s_Dict.c_str(),
                                NULL)) == NULL)
    {
        throw Exception("Failed to create sphinx config object!");
    }
    
    if ((p_Decoder = ps_init(p_Config)) == NULL)
    {
        throw Exception("Failed to create sphinx recognizer!");
    }
}

ReadAudio::~ReadAudio() noexcept
{
    if (p_Decoder != NULL)
    {
        ps_free(p_Decoder);
    }
    
    if (p_Config != NULL)
    {
        cmd_ln_free_r(p_Config);
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

std::string ReadAudio::GetLocale() const noexcept
{
    return s_Locale;
}
