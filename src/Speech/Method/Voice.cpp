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
#include "../../Configuration.h"

// Pre-defined
#define LISTEN_CHECK_WAIT_MS 250 // Wait before checking again
#define LISTEN_PAUSE_ACCEPT_TIME_S 3 // 3 Second pauses count as speech


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Voice::Voice() : c_PocketSphinx(Configuration::Singleton().GetSphinxModelDirPath()),
                 c_Converter(POCKET_SPHINX_REQUIRED_KHZ),
                 c_TriggerSound({}, 0, Configuration::Singleton().GetPASpeakerKHz()),
                 u64_TriggerValidS(0),
                 b_PlayTriggerSound(false),
                 b_ListenAudioAvailable(false),
                 us_ListenWaitSamples(0),
                 b_StringSet(false)
{
    // Load trigger sound
    std::FILE* p_File = std::fopen(Configuration::Singleton().GetTriggerSoundPath().c_str(), "rb");
    
    if (p_File != NULL)
    {
        // @NOTE: Raw Data, has to match otherwise unpleasant sounds happen
        char p_Buffer[256];
        
        while (std::feof(p_File) == 0)
        {
            std::fread(p_Buffer, 256, 1, p_File);
            c_TriggerSound.v_Buffer.insert(c_TriggerSound.v_Buffer.end(),
                                           (MRH_Sint16*)p_Buffer,
                                           (MRH_Sint16*)&(p_Buffer[255]));
        }
    }
    else
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Failed to read trigger sound file!",
                                       "PocketSphinx.cpp", __LINE__);
    }
    
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Voice speech now available.",
                                   "PocketSphinx.cpp", __LINE__);
}

Voice::~Voice() noexcept
{}

//*************************************************************************************
// Useage
//*************************************************************************************

void Voice::Resume()
{
    // Reset Google conversions
    c_GoogleSTT.ResetStrings();
    c_GoogleTTS.ResetAudio();
    
    // Just start recording
    c_Device.Record();
}

void Voice::Pause()
{
    // Reset Components
    c_Device.StopDevice(); // Stop both playback and recording
    c_PocketSphinx.ResetDecoder(); // Reset decoder, new recognition next time
    c_GoogleSTT.ResetAudio(); // Reset audio
    c_GoogleTTS.ResetStrings(); // Reset passed strings
    c_Converter.ResetConverter(); // State based, reset for new convert
    
    // Reset listen flags
    u64_TriggerValidS = 0;
    b_ListenAudioAvailable = false;
    us_ListenWaitSamples = 0;
    
    // Reset say flags
    b_StringSet = false;
    b_PlayTriggerSound = false;
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
    std::list<std::string> l_Processed = c_GoogleSTT.GetStrings();
    
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
    if (c_Device.GetRecording() == false)
    {
        return;
    }
    
    /**
     *  Process
     */
    
    // Grab sample
    MonoAudio c_Audio = c_Device.GetRecordedAudio();
    
    if (c_Audio.v_Buffer.size() == 0)
    {
        return;
    }
    
    // Is the sample data valid for sphinx?
    if (c_Audio.u32_KHz == POCKET_SPHINX_REQUIRED_KHZ)
    {
        // Same, simply add
        c_PocketSphinx.AddAudio(c_Audio.v_Buffer);
    }
    else
    {
        try
        {
            // Not the same, convert then add
            c_PocketSphinx.AddAudio(c_Converter.Convert(c_Audio.v_Buffer,
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
     *  Grab Speech
     */
    
    // Set current time for trigger validity
    MRH_Uint64 u64_CurrentTimeS = time(NULL);
    Configuration& c_Config = Configuration::Singleton();
    
    // Was this speech?
    if (c_PocketSphinx.AudioContainsSpeech() == true)
    {
        // Audio valid, add to speech to text api
        if (u64_TriggerValidS >= u64_CurrentTimeS)
        {
            // Trigger was valid, now keep valid during speech
            u64_TriggerValidS = u64_CurrentTimeS + c_Config.GetTriggerTimeoutS();
            c_GoogleSTT.AddAudio(c_Audio);
        }
        
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
        else if (us_ListenWaitSamples < (c_Audio.u32_KHz * LISTEN_PAUSE_ACCEPT_TIME_S))
        {
            // Add the pause
            if (u64_TriggerValidS >= u64_CurrentTimeS)
            {
                // Trigger was valid, now keep valid during speech
                u64_TriggerValidS = u64_CurrentTimeS + c_Config.GetTriggerTimeoutS();
                c_GoogleSTT.AddAudio(c_Audio);
            }
            
            // Stil in "pause" phase, wait
            us_ListenWaitSamples += c_Audio.v_Buffer.size();
            return;
        }
    }
    
    // Reset, available will now be processed
    b_ListenAudioAvailable = false;
    us_ListenWaitSamples = 0;
    c_Converter.ResetConverter(); // Reset for next audio (state based)
    
    /**
     *  Process
     */
    
    // Do we need to check for the trigger
    if (u64_TriggerValidS < u64_CurrentTimeS)
    {
        // Check, was the trigger recognized?
        if (c_PocketSphinx.Recognize() == true)
        {
            // Trigger recognized, valid
            u64_TriggerValidS = u64_CurrentTimeS + c_Config.GetTriggerTimeoutS();
            
            // Reset decoder for audio recognition
            c_PocketSphinx.ResetDecoder();
            
            // Play signal sound next
            if (c_TriggerSound.v_Buffer.size() > 0)
            {
                b_PlayTriggerSound = true;
            }
        }
    }
    else
    {
        // Run speech recognition with google api
        c_GoogleSTT.ProcessAudio();
        
        // Reset decoder for audio recognition
        c_PocketSphinx.ResetDecoder();
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
        
        // NOTE: This should never throw - Playback happens after full conversion, which
        //       causes the currently given TTS string to be reset. New strings can't be
        //       grabbed during playback or conversion because of b_StringSet.
        try
        {
            c_GoogleTTS.AddString(c_String.s_String);
            
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
    if (c_Device.GetPlayback() == true)
    {
        return;
    }
    
    // Remember playback time for trigger timeout
    static MRH_Uint64 u64_PlaybackStartS;
    
    // Should we start speaking or resume listening?
    if (b_PlayTriggerSound == true || c_GoogleTTS.GetAudioAvailable() == true)
    {
        try
        {
            // Add sound as output data and play
            if (b_PlayTriggerSound == true)
            {
                c_Device.SetPlaybackAudio(c_TriggerSound);
                b_PlayTriggerSound = false; // Reset request
            }
            else
            {
                c_Device.SetPlaybackAudio(c_GoogleTTS.GetAudio());
            }
            
            c_Device.Playback();
            
            // Set start
            u64_PlaybackStartS = time(NULL);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to add audio: " + std::string(e.what()),
                                           "Voice.cpp", __LINE__);
        }
    }
    else if (c_Device.GetRecording() == false)
    {
        // Nothing to play, start listening again
        c_Device.Record();
        
        // Add time passed in seconds to trigger timeout
        u64_TriggerValidS += (time(NULL) - u64_PlaybackStartS);
        
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
    return true;
}
