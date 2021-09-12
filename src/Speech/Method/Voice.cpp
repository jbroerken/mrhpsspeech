/**
 *  Voice.cpp
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
#include <ctime>

// External
#include <libmrhpsb/MRH_PSBLogger.h>
#include <libmrhvt/String/Compare/MRH_Levenshtein.h>

// Project
#include "./Voice.h"
#include "./Voice/PAMicrophone.h"
#include "./Voice/PocketSphinx.h"
#include "./Voice/GoogleAPI.h"
#include "../../Configuration.h"

// Pre-defined
#define LISTEN_SAMPLE_MINIMUM_TIME_S 2 // Time diff for sample size
#define LISTEN_SAMPLE_THREAD_WAIT_MS 100 // Wait before checking again


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Voice::Voice() : p_Microphone(NULL),
                 p_PocketSphinx(NULL),
                 p_GoogleAPI(NULL),
                 u64_LastSampleS(time(NULL) + LISTEN_SAMPLE_MINIMUM_TIME_S) // 2 seconds ahead
{
    // Init components
    try
    {
        p_Microphone = new PAMicrophone();
        p_PocketSphinx = new PocketSphinx(Configuration::Singleton().GetSphinxModelDirPath());
        p_GoogleAPI = new GoogleAPI();
    }
    catch (Exception& e)
    {
        if (p_Microphone != NULL)
        {
            delete p_Microphone;
            p_Microphone = NULL;
        }
        
        if (p_PocketSphinx != NULL)
        {
            delete p_PocketSphinx;
            p_PocketSphinx = NULL;
        }
        
        throw;
    }
    
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Voice speech now available.",
                                   "PocketSphinx.cpp", __LINE__);
}

Voice::~Voice() noexcept
{
    if (p_Microphone != NULL)
    {
        delete p_Microphone;
    }
    
    if (p_PocketSphinx != NULL)
    {
        delete p_PocketSphinx;
    }
    
    if (p_GoogleAPI != NULL)
    {
        delete p_GoogleAPI;
    }
}

//*************************************************************************************
// Listen
//*************************************************************************************

void Voice::Listen()
{
    // Check time passed
    MRH_Uint64 u64_CurrentTimeS = time(NULL);
    
    if (u64_LastSampleS > u64_CurrentTimeS + LISTEN_SAMPLE_MINIMUM_TIME_S)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(LISTEN_SAMPLE_THREAD_WAIT_MS));
        u64_CurrentTimeS = time(NULL);
        
        // Still not enough?
        if (u64_LastSampleS > u64_CurrentTimeS + LISTEN_SAMPLE_MINIMUM_TIME_S)
        {
            // Return for say
            return;
        }
    }
    
    // Grab samples
    AudioSample c_Sample = p_Microphone->GetAudioSample();
    size_t us_Pos = 0;
    size_t us_Size = c_Sample.u32_Samples;
    static bool b_AudioAvailable = false;
    
    while (us_Pos < c_Sample.v_Buffer.size())
    {
        std::vector<MRH_Sint16> v_Buffer;
        
        // Is the sample data valid for sphinx?
        if (c_Sample.u32_KHz == 16000 && c_Sample.u8_Channels == 1)
        {
            // Same, simply add
            printf("Add Audio!\n");
            p_PocketSphinx->AddAudio(&(c_Sample.v_Buffer[us_Pos]), us_Size);
        }
        else
        {
            // Not the same, convert
            std::vector<MRH_Sint16> v_Buffer = c_Sample.Convert(us_Pos, us_Size, 16000, 1);
            p_PocketSphinx->AddAudio(v_Buffer.data(), us_Size);
        }
        
        // Added, was this speech?
        if (p_PocketSphinx->AudioContainsSpeech() == true)
        {
            printf("Still in speech!\n");
            
            // Was speech, keep and grab next
            us_Pos += us_Size;
            b_AudioAvailable = true;
            
            continue;
        }
        else if (us_Pos == 0) // Special case: Start of buffer is 0
        {
            c_Sample.v_Buffer.erase(c_Sample.v_Buffer.begin(),
                                    c_Sample.v_Buffer.begin() + us_Size);
            
            // Removed, but do we have audio stil from the last loop?
            if (b_AudioAvailable == false)
            {
                continue;
            }
        }

        std::string s_SphinxResult = p_PocketSphinx->Recognize();
        
        // @TODO: Remove, TEsting only
        printf("TESTING: Recognized: %s\n", s_SphinxResult.c_str());
        continue;
        
        // Do we need to check for the trigger?
        if (u64_TriggerValidS < u64_CurrentTimeS)
        {
            Configuration& c_Config = Configuration::Singleton();
            
            // Set trigger recognized
            if (MRH_StringCompareLS::ContainedIn(c_Config.GetTriggerString(),
                                                 s_SphinxResult,
                                                 c_Config.GetTriggerLSSimilarity()) == true)
            {
                b_TriggerRecognized = true;
                u64_TriggerValidS = u64_CurrentTimeS;
            }
            else
            {
                // Not recognized, reset available do nothing
                b_AudioAvailable = false;
                continue;
            }
        }
        else
        {
            // Reset timer, valid audio with input
            u64_TriggerValidS = u64_CurrentTimeS;
        }
        
        // Audio valid, add to speech to text api
        p_GoogleAPI->AddAudio(c_Sample.v_Buffer.data(), us_Pos, c_Sample.u32_KHz);
        
        // Audio processed
        b_AudioAvailable = false;
    }
}

//*************************************************************************************
// Say
//*************************************************************************************

void Voice::Say(OutputStorage& c_OutputStorage)
{
    // @TODO: Say
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool Voice::IsUsable() noexcept
{
    if (p_Microphone != NULL && p_PocketSphinx != NULL && p_GoogleAPI != NULL)
    {
        return true;
    }
    
    return false;
}
