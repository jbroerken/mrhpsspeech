/**
 *  PocketSphinx.cpp
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
#include "./PocketSphinx.h"

// Pre-defined
#define MRH_SPEECH_SPHINX_LM_EXT ".lm.bin"
#define MRH_SPEECH_SPHINX_DICT_EXT ".dict"
#define MRH_SPEECH_SPHINX_DEFAULT_LOCALE "en_US"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

PocketSphinx::PocketSphinx(std::string const& s_ModelDir)
{
    // Grab locale first
    std::string s_Locale(MRH_SPEECH_SPHINX_DEFAULT_LOCALE);
    
    if (std::setlocale(LC_CTYPE, NULL) == NULL)
    {
        s_Locale = std::setlocale(LC_CTYPE, NULL);
        s_Locale = s_Locale.substr(0, s_Locale.find_first_of('.'));
    }
    
    // Setup sphinx
    std::string s_HMM = s_ModelDir + s_Locale + "/" + s_Locale;
    std::string s_LM = s_ModelDir + s_Locale + "/" + s_Locale + MRH_SPEECH_SPHINX_LM_EXT;
    std::string s_Dict = s_ModelDir + s_Locale + "/" + s_Locale + MRH_SPEECH_SPHINX_DICT_EXT;
        
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Sphinx HMM: " + s_HMM,
                                   "PocketSphinx.cpp", __LINE__);
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Sphinx LM: " + s_LM,
                                   "PocketSphinx.cpp", __LINE__);
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Sphinx Dict: " + s_Dict,
                                   "PocketSphinx.cpp", __LINE__);

    if ((p_Config = cmd_ln_init(NULL, ps_args(), TRUE,
                                "-hmm", s_HMM.c_str(),
                                "-lm", s_LM.c_str(),
                                "-dict", s_Dict.c_str(),
                                NULL)) == NULL)
    {
        throw Exception("Failed to create sphinx config object!");
    }
    else if ((p_Decoder = ps_init(p_Config)) == NULL)
    {
        cmd_ln_free_r(p_Config);
        p_Config = NULL;
            
        throw Exception("Failed to create sphinx recognizer!");
    }
}

PocketSphinx::~PocketSphinx() noexcept
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
// Recognize
//*************************************************************************************

void PocketSphinx::StartRecognition() noexcept
{
    if (ps_start_utt(p_Decoder) < 0)
    {
        printf("START ERROR\n");
    }
}

void PocketSphinx::AddSample(const MRH_Uint8* p_Buffer, MRH_Uint32 u32_Length)
{
    if (ps_process_raw(p_Decoder, (const int16*)p_Buffer, u32_Length, FALSE, FALSE) < 0)
    {
        printf("PROCESS ERROR!\n");
    }
}

std::string PocketSphinx::Recognize() noexcept
{
    if (ps_end_utt(p_Decoder) < 0)
    {
        printf("End UTT Error!\n");
    }
    
    MRH_Sint32 s32_Score;
    char const* p_Hypothesis;
    
    if ((p_Hypothesis = ps_get_hyp(p_Decoder, &s32_Score)) == NULL)
    {
        printf("HYP Error!\n");
    }
    
    printf("%s was result with score %d\n", p_Hypothesis, s32_Score);
    
    return p_Hypothesis;
}
