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
#include "../../../Configuration.h"

// Pre-defined
#define MRH_SPEECH_SPHINX_LM_EXT ".lm.bin"
#define MRH_SPEECH_SPHINX_DICT_EXT ".dict"
#define MRH_SPEECH_SPHINX_DEFAULT_LOCALE "en_US"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

PocketSphinx::PocketSphinx(std::string const& s_ModelDir) : p_Decoder(NULL),
                                                            p_Config(NULL),
                                                            b_DecoderRunning(false)
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
                                "-logfn", "/dev/null",
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
    
    // Now set the keyphrase
    std::string s_Keyphrase(Configuration::Singleton().GetTriggerKeyphrase());
    
    if (s_Keyphrase.size() == 0 ||
        ps_set_keyphrase(p_Decoder, "keyphrase_search", s_Keyphrase.c_str()) != 0 ||
        ps_set_search(p_Decoder, "keyphrase_search") != 0)
    {
        ps_free(p_Decoder);
        p_Decoder = NULL;
        
        cmd_ln_free_r(p_Config);
        p_Config = NULL;
        
        throw Exception("Failed to set sphinx keyphrase search!");
    }
    
    // Start initial
    if (ps_start_utt(p_Decoder) < 0)
    {
        ps_free(p_Decoder);
        p_Decoder = NULL;
        
        cmd_ln_free_r(p_Config);
        p_Config = NULL;
        
        throw Exception("Failed to start sphinx utterance!");
    }
    
    b_DecoderRunning = true;
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
// Reset
//*************************************************************************************

void PocketSphinx::ResetDecoder() noexcept
{
    // End decoder first
    if (b_DecoderRunning == true)
    {
        if (ps_end_utt(p_Decoder) < 0)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to end utterance!",
                                           "PocketSphinx.cpp", __LINE__);
        }
        
        b_DecoderRunning = false;
    }
    
    // Start again
    if (ps_start_utt(p_Decoder) < 0)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to start utterance!",
                                       "PocketSphinx.cpp", __LINE__);
    }
    
    b_DecoderRunning = true;
}

//*************************************************************************************
// Recognize
//*************************************************************************************

void PocketSphinx::AddAudio(std::vector<MRH_Sint16> const& v_Buffer) noexcept
{
    if (b_DecoderRunning == false)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Decoder not running!",
                                       "PocketSphinx.cpp", __LINE__);
    }
    else if (ps_process_raw(p_Decoder, v_Buffer.data(), v_Buffer.size(), FALSE, FALSE) < 0)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to process audio sample!",
                                       "PocketSphinx.cpp", __LINE__);
    }
}

bool PocketSphinx::AudioContainsSpeech() noexcept
{
    if (b_DecoderRunning == false)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Decoder not running!",
                                       "PocketSphinx.cpp", __LINE__);
        return false;
    }
    else if (ps_get_in_speech(p_Decoder) < 1)
    {
        return false;
    }
    
    return true;
}

bool PocketSphinx::Recognize() noexcept
{
    if (b_DecoderRunning == false)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Decoder not running!",
                                       "PocketSphinx.cpp", __LINE__);
        return false;
    }
    else if (ps_end_utt(p_Decoder) < 0)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to end utterance!",
                                       "PocketSphinx.cpp", __LINE__);
        return false;
    }
    
    b_DecoderRunning = false;
    
    // Check hypo
    bool b_Result = false;
    MRH_Sint32 s32_Score;
    
#ifdef DEBUG
    const char* p_Hypothesis;
    
    if ((p_Hypothesis = ps_get_hyp(p_Decoder, &s32_Score)) != NULL)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Hypothesis: " + std::string(p_Hypothesis),
                                       "PocketSphinx.cpp", __LINE__);
        b_Result = true;
    }
    else
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "No keyphrase hypothesis for current input!",
                                       "PocketSphinx.cpp", __LINE__);
    }
#else
    if (ps_get_hyp(p_Decoder, &s32_Score) != NULL)
    {
        b_Result = true;
    }
#endif
    
    // Got hypo, restart utt
    if (ps_start_utt(p_Decoder) < 0)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to restart sphinx utterance!",
                                       "PocketSphinx.cpp", __LINE__);
    }
    
    b_DecoderRunning = true;
    
    // Reset, give result
    return b_Result;
}
