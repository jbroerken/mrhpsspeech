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
#include <chrono>

// External
#include <libmrhpsb/MRH_PSBLogger.h>
#include <libmrhvt/String/Compare/MRH_Levenshtein.h>

// Project
#include "./Voice.h"
#include "../../Configuration.h"

// Pre-defined
#define LISTEN_CHECK_WAIT_MS 250 // Wait before checking again
#define LISTEN_PAUSE_TIMEOUT_S 3 // Time until audio is processed

using std::chrono::system_clock;
using std::chrono::seconds;
using std::chrono::duration_cast;


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Voice::Voice()
{
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
    // Just start recording
    c_AudioStream.Record();
}

void Voice::Reset()
{
    // Reset current audio but keep listening
    c_GoogleSTT.ResetAudio();
}

void Voice::Pause()
{
    // Reset Components
    c_AudioStream.StopAll(); // Stop both playback and recording
    c_GoogleSTT.ResetAudio(); // Reset audio
}

//*************************************************************************************
// Listen
//*************************************************************************************

void Voice::Listen()
{
    /**
     *  Wait
     */
    
    // NOTE: We ALWAYS wait - either for input recording to not rapid fire or
    //       for playback to continue
    std::this_thread::sleep_for(std::chrono::milliseconds(LISTEN_CHECK_WAIT_MS));
    
    // Are we event recording right now?
    if (c_AudioStream.GetRecording() == false)
    {
        return;
    }
    
    /**
     *  Process
     */
    
    auto CurrentTime = system_clock::now().time_since_epoch();
    static auto ProccessTime = CurrentTime;
    
    try
    {
        // Do we need to set a primary device?
        if (c_AudioStream.GetPrimaryRecordingDeviceSet() == false)
        {
            c_AudioStream.SelectPrimaryRecordingDevice();
        }
        
        // Grab sample
        AudioTrack const& c_Audio = c_AudioStream.GetRecordedAudio();
        
        if (c_Audio.GetAudioExists() == 0)
        {
            // Wait before we start to process audio
            if (CurrentTime >= ProccessTime)
            {
                // Transcribe and send as input
                try
                {
                    SendInput(c_GoogleSTT.Transcribe());
                }
                catch (Exception& e)
                {
                    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                                   "Voice.cpp", __LINE__);
                }
            }
        }
        else
        {
            // Add existing audio to speech to text
            c_GoogleSTT.AddAudio(c_Audio);
            
            // Set last time audio was gotten
            ProccessTime = duration_cast<seconds>(CurrentTime + seconds(LISTEN_PAUSE_TIMEOUT_S));
        }
    }
    catch (...)
    {}
}

//*************************************************************************************
// Say
//*************************************************************************************

void Voice::Say(OutputStorage& c_OutputStorage)
{
    /**
     *  Playback Check
     */
    
    // Do nothing during output playback
    if (c_AudioStream.GetPlayback() == true)
    {
        return;
    }
    
    /**
     *  Playback Synthesised String
     */
    
    if (b_StringSet == false && c_OutputStorage.GetFinishedAvailable() == true)
    {
        OutputStorage::String c_String = c_OutputStorage.GetFinishedString();
        
        try
        {
            c_AudioStream.Playback(c_GoogleTTS.Synthesise(c_String.s_String));
            
            u32_SayStringID = c_String.u32_StringID;
            u32_SayGroupID = c_String.u32_GroupID;
        
            b_StringSet = true;
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                           "Voice.cpp", __LINE__);
        }
    }
    
    /**
     *  Restart Recording
     */
    
    if (c_AudioStream.GetRecording() == false)
    {
        // Nothing to play, start listening again
        c_AudioStream.Record();
        
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
