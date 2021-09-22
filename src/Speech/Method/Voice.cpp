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
#include "./Voice/RateConverter.h"
#include "./Voice/PADevice.h"
#include "./Voice/PocketSphinx.h"
#include "./Voice/GoogleAPI.h"
#include "../../Configuration.h"

// Pre-defined
#define LISTEN_CHECK_WAIT_MS 100 // Wait before checking again


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Voice::Voice() : p_Device(NULL),
                 p_PocketSphinx(NULL),
                 p_GoogleAPI(NULL),
                 b_TriggerRecognized(false),
                 u64_TriggerValidS(0),
                 b_ListenAudioAvailable(false),
                 us_ListenWaitSamples(0),
                 b_StringSet(false),
                 p_Converter(NULL)
{
    // Init components
    try
    {
        p_Converter = new RateConverter(POCKET_SPHINX_REQUIRED_KHZ);
        p_Device = new PADevice();
        p_PocketSphinx = new PocketSphinx(Configuration::Singleton().GetSphinxModelDirPath());
        p_GoogleAPI = new GoogleAPI();
    }
    catch (Exception& e)
    {
        if (p_Converter != NULL)
        {
            delete p_Converter;
            p_Converter = NULL;
        }
        
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
    if (p_Converter != NULL)
    {
        delete p_Converter;
    }
    
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

void Voice::Resume()
{
    // Just start recording
    if (p_Device != NULL)
    {
        p_Device->Record();
    }
}

void Voice::Pause()
{
    // Stop device
    if (p_Device != NULL)
    {
        p_Device->StopAll();
    }
    
    if (p_GoogleAPI != NULL)
    {
        // Remove old data, we dont know how long we're stopped
        p_GoogleAPI->ClearSTTAudio();
        p_GoogleAPI->RecieveStringsSTT(); // Grab list to empty
    }
    
    if (p_PocketSphinx != NULL)
    {
        // Reset decoder, new recognition next time
        p_PocketSphinx->ResetDecoder();
    }
    
    if (p_Converter != NULL)
    {
        p_Converter->Reset();
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
    
    // NOTE: We ALWAYS wait - either for input recording to not rapid fire or
    //       for playback to continue
    std::this_thread::sleep_for(std::chrono::milliseconds(LISTEN_CHECK_WAIT_MS));
    
    // Can we even record?
    if (p_Device->GetRecording() == false)
    {
        return;
    }
    
    /**
     *  Process
     */
    
    // Grab sample
    VoiceAudio c_Audio = p_Device->GetRecordedAudio();
    
    if (c_Audio.v_Buffer.size() == 0)
    {
        return;
    }
    
    // Is the sample data valid for sphinx?
    if (c_Audio.u32_KHz == POCKET_SPHINX_REQUIRED_KHZ)
    {
        // Same, simply add
        p_PocketSphinx->AddAudio(c_Audio.v_Buffer);
    }
    else
    {
        try
        {
            // Not the same, convert then add
            p_PocketSphinx->AddAudio(p_Converter->Convert(c_Audio.v_Buffer,
                                                          c_Audio.u32_KHz));
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, e.what(),
                                           "Voice.cpp", __LINE__);
            return;
        }
    }
    
    /**
     *  Check Speech
     */
    
    // Added, was this speech?
    if (p_PocketSphinx->AudioContainsSpeech() == true)
    {
        // Audio valid, add to speech to text api
        p_GoogleAPI->AddAudioSTT(c_Audio.v_Buffer,
                                 c_Audio.u32_KHz);
        
        // Grab next
        b_ListenAudioAvailable = true;
        return; // Always return, more audio might come
    }
    else
    {
        if (b_ListenAudioAvailable == false)
        {
            // No usable audio stored, return directly
            return;
        }
        else if (us_ListenWaitSamples < c_Audio.u32_KHz) // 1 Sec
        {
            // Add the pause
            p_GoogleAPI->AddAudioSTT(c_Audio.v_Buffer,
                                     c_Audio.u32_KHz);
            
            // Stil in "pause" phase, wait
            us_ListenWaitSamples += c_Audio.v_Buffer.size();
            return;
        }
    }
    
    // Reset, available will now be processed
    b_ListenAudioAvailable = false;
    us_ListenWaitSamples = 0;
    p_Converter->Reset(); // Reset for next audio (state based)
    
    // TEST
    //
    //    ----> TODO: Test 44100 KHZ input / output! Add Mono Audio to all channels for output callback!
    //    -> Add flag for remove trigger from first string to send if recorded with trigger recognized
    //
    //
    printf("-- %s\n", p_PocketSphinx->Recognize().c_str());
    p_GoogleAPI->ProcessAudioSTT();
    return;
    
    
    /**
     *  Trigger
     */
    
    // Do we need to check for the trigger
    Configuration& c_Config = Configuration::Singleton();
    MRH_Uint64 u64_CurrentTimeS = time(NULL);
    
    if (u64_TriggerValidS < u64_CurrentTimeS)
    {
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
            p_GoogleAPI->ClearSTTAudio();
            return;
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
        
        // NOTE: This should never throw - Playback happens after full conversion, which
        //       causes the currently given TTS string to be reset. New strings can't be
        //       grabbed during playback or conversion because of b_StringSet.
        try
        {
            p_GoogleAPI->AddStringTTS(c_String.s_String);
            u32_SayStringID = c_String.u32_StringID;
            u32_SayGroupID = c_String.u32_GroupID;
        
            b_StringSet = true;
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Dropped say string: " + std::string(e.what()),
                                           "Voice.cpp", __LINE__);
        }
    }
    
    /**
     *  Playback
     */
    
    // Do nothing during output playback
    if (p_Device->GetPlayback() == true)
    {
        return;
    }
    
    // Should we start speaking or resume listening?
    if (p_GoogleAPI->TTSAudioAvailable() == true)
    {
        try
        {
            // Add speech as output data and play
            p_Device->SetPlaybackAudio(p_GoogleAPI->GrabTTSAudio());
            
            printf("Start Playback\n");
            p_Device->Playback();
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to add audio: " + std::string(e.what()),
                                           "Voice.cpp", __LINE__);
        }
    }
    else if (p_Device->GetRecording() == false)
    {
        // Nothing to play, start listening again
        printf("Start Listening\n");
        p_Device->Record();
        
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
