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
#include "./Voice/PADevice.h"
#include "./Voice/PocketSphinx.h"
#include "./Voice/GoogleAPI.h"
#include "../../Configuration.h"

// Pre-defined
#define NEXT_LISTEN_WAIT_TIME_S 2 // Time diff for listen processing
#define LISTEN_CHECK_WAIT_MS 100 // Wait before checking again


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Voice::Voice() : p_Device(NULL),
                 p_PocketSphinx(NULL),
                 p_GoogleAPI(NULL),
                 u64_NextListenS(time(NULL) + NEXT_LISTEN_WAIT_TIME_S), // 2 seconds ahead
                 b_TriggerRecognized(false),
                 u64_TriggerValidS(0),
                 b_ListenAudioAvailable(false),
                 us_ListenWaitSamples(0),
                 b_StringSet(false)
{
    // Init components
    try
    {
        p_Device = new PADevice();
        p_PocketSphinx = new PocketSphinx(Configuration::Singleton().GetSphinxModelDirPath());
        p_GoogleAPI = new GoogleAPI();
    }
    catch (Exception& e)
    {
        if (p_Device != NULL)
        {
            delete p_Device;
            p_Device = NULL;
        }
        
        if (p_PocketSphinx != NULL)
        {
            delete p_PocketSphinx;
            p_PocketSphinx = NULL;
        }
        
        // Last throw is google api, never not null on throw
        throw;
    }
    
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Voice speech now available.",
                                   "PocketSphinx.cpp", __LINE__);
}

Voice::~Voice() noexcept
{
    if (p_Device != NULL)
    {
        delete p_Device;
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
// Useage
//*************************************************************************************

void Voice::Start()
{
    // Just start listening
    if (p_Device != NULL)
    {
        p_Device->ResetInputAudio();
        p_Device->StartListening();
    }
    
    // Set next listen processing
    u64_NextListenS = time(NULL) + NEXT_LISTEN_WAIT_TIME_S;
}

void Voice::Stop()
{
    // Stop device
    if (p_Device != NULL)
    {
        p_Device->StopPlayback();
        p_Device->StopListening();
    }
    
    if (p_GoogleAPI != NULL)
    {
        // Remove old data, we dont know how long we're stopped
        p_GoogleAPI->ClearSTT();
    }
    
    if (p_PocketSphinx != NULL)
    {
        // Reset decoder, new recognition
        p_PocketSphinx->ResetDecoder();
    }
    
    // Reset listen flags
    b_TriggerRecognized = false;
    u64_TriggerValidS = 0;
    b_ListenAudioAvailable = false;
    us_ListenWaitSamples = 0;
    
    // Reset say flags
    b_StringSet = false;
}

//*************************************************************************************
// Listen
//*************************************************************************************

void Voice::Listen()
{
    /**
     *  Events
     */
    
    // Grab google api speech results
    std::list<std::string> l_Processed = p_GoogleAPI->RecieveStringsSTT();
    
    for (auto& String : l_Processed)
    {
        try
        {
            SendInput(String);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                           "Voice.cpp", __LINE__);
        }
    }
    
    /**
     *  Wait
     */
    
    // Check time passed
    MRH_Uint64 u64_CurrentTimeS = time(NULL);
    
    if (u64_NextListenS > u64_CurrentTimeS)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(LISTEN_CHECK_WAIT_MS));
        u64_CurrentTimeS = time(NULL);
        
        // Still not enough?
        if (u64_NextListenS > u64_CurrentTimeS)
        {
            // Return for say
            return;
        }
    }
    
    // Next listen time
    u64_NextListenS = u64_CurrentTimeS + NEXT_LISTEN_WAIT_TIME_S;
    
    // Grab samples
    VoiceAudio c_Audio = p_Device->GetInputAudio();
    size_t us_Pos = 0;
    size_t us_Size = c_Audio.u32_FrameSamples;
    
    while (us_Pos < c_Audio.v_Buffer.size())
    {
        /**
         *  Convert Audio
         */
        
        // Is the sample data valid for sphinx?
        if (c_Audio.u32_KHz == 16000 && c_Audio.u8_Channels == 1)
        {
            // Same, simply add
            p_PocketSphinx->AddAudio(&(c_Audio.v_Buffer[us_Pos]), us_Size);
        }
        else
        {
            // Not the same, convert then add
            std::vector<MRH_Sint16> v_Buffer = c_Audio.Convert(us_Pos, us_Size, 16000, 1);
            p_PocketSphinx->AddAudio(v_Buffer.data(), us_Size);
        }
        
        // Buffer pos has to be incremented!
        us_Pos += us_Size;
        
        /**
         *  Check Speech
         */
        
        // Added, was this speech?
        if (p_PocketSphinx->AudioContainsSpeech() == true)
        {
            // Audio valid, add to speech to text api
            // @NOTE: Add ->LAST<- buffer data (- size, 1 step back)
            p_GoogleAPI->AddAudioSTT(&(c_Audio.v_Buffer[us_Pos - us_Size]), us_Size, c_Audio.u32_KHz);
            
            // Grab next
            b_ListenAudioAvailable = true;
            continue;
        }
        else
        {
            if (b_ListenAudioAvailable == false)
            {
                // No usable audio stored, return directly
                continue;
            }
            else if (us_ListenWaitSamples < c_Audio.u32_KHz) // 1 Sec
            {
                // Stil in "pause" phase, wait
                us_ListenWaitSamples += us_Size;
                continue;
            }
        }
        
        // Reset, available will now be processed
        b_ListenAudioAvailable = false;
        us_ListenWaitSamples = 0;
        
        /**
         *  Trigger
         */
        
        // Do we need to check for the trigger?
        if (u64_TriggerValidS < u64_CurrentTimeS)
        {
            Configuration& c_Config = Configuration::Singleton();
            std::string s_SphinxResult = p_PocketSphinx->Recognize();
            
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
                // Not recognized, reset api buffer
                p_GoogleAPI->ClearSTT();
                continue;
            }
        }
        else
        {
            // Reset decoder for new check later
            p_PocketSphinx->ResetDecoder();
            
            // Reset timer, valid audio with input
            u64_TriggerValidS = u64_CurrentTimeS;
        }
        
        /**
         *  Convert
         */
        
        // Run speech recognition with google api
        p_GoogleAPI->ProcessAudioSTT();
    }
}

//*************************************************************************************
// Say
//*************************************************************************************

void Voice::Say(OutputStorage& c_OutputStorage)
{
    /**
     *  Set String
     */
    
    if (b_StringSet == false && c_OutputStorage.GetFinishedAvailable() == true)
    {
        OutputStorage::String c_String = c_OutputStorage.GetFinishedString();
        
        p_GoogleAPI->AddStringTTS(c_String.s_String);
        u32_SayStringID = c_String.u32_StringID;
        u32_SayGroupID = c_String.u32_GroupID;
        
        b_StringSet = true;
    }
    
    /**
     *  Playback
     */
    
    // Do nothing during output playback
    if (p_Device->GetOutputPlayback() == true)
    {
        return;
    }
    
    // Should we start speaking or resume listening?
    if (p_GoogleAPI->TTSAudioAvailable() == true)
    {
        try
        {
            // Add speech as output data
            p_Device->SetOutputAudio(p_GoogleAPI->GrabTTSAudio());
            
            // Pause listening and perform output
            p_Device->StopListening();
            p_Device->StartPlayback();
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to add audio: " + std::string(e.what()),
                                           "Voice.cpp", __LINE__);
        }
    }
    else if (p_Device->GetInputRecording() == false)
    {
        // Nothing to play, start listening again
        p_Device->StartListening();
        
        // Send info about performed output
        b_StringSet = false;
        
        try
        {
            OutputPerformed(u32_SayStringID, u32_SayGroupID);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to set performed: " + std::string(e.what()),
                                           "Voice.cpp", __LINE__);
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool Voice::IsUsable() noexcept
{
    if (p_Device != NULL && p_PocketSphinx != NULL && p_GoogleAPI != NULL)
    {
        return true;
    }
    
    return false;
}
