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
#define LISTEN_PAUSE_TIMEOUT_S 3 // Time until audio is processed


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
    c_DevicePool.StartDevices();
}

void Voice::Reset()
{
    // Reset current audio but keep listening
    c_GoogleSTT.ResetAudio();
}

void Voice::Pause()
{
    // Reset Components
    c_DevicePool.StopDevices();
    c_GoogleSTT.ResetAudio(); // Reset audio already added
}

//*************************************************************************************
// Listen
//*************************************************************************************

void Voice::Listen()
{
    // @NOTE: We ALWAYS wait - give the devices some time to fill the recording
    //        buffer to retrieve
    std::this_thread::sleep_for(std::chrono::milliseconds(LISTEN_CHECK_WAIT_MS));
    
    static MRH_Uint64 u64_ProccessTimeS = time(NULL) + LISTEN_PAUSE_TIMEOUT_S;
    
    try
    {
        // Do we need to set a recording device?
        if (c_DevicePool.GetRecordingDeviceSelected() == false)
        {
            c_DevicePool.SelectRecordingDevice();
        }
        
        // Grab sample
        AudioTrack const& c_Audio = c_DevicePool.GetRecordedAudio();
        
        if (c_Audio.GetAudioExists() == false)
        {
            // Wait before we start to process audio
            if (time(NULL) > u64_ProccessTimeS)
            {
                // Transcribe and send as input
                try
                {
                    SendInput(c_GoogleSTT.Transcribe());
                }
                catch (Exception& e)
                {
                    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to send input: " +
                                                                         std::string(e.what()),
                                                   "Voice.cpp", __LINE__);
                }
                
                // Reset the used recording device to choose a new one
                c_DevicePool.ResetRecordingDevice();
            }
        }
        else
        {
            // Add existing audio to speech to text
            c_GoogleSTT.AddAudio(c_Audio);
            
            // Set next timeout for processing
            u64_ProccessTimeS = time(NULL) + LISTEN_PAUSE_TIMEOUT_S;
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
    // Is playback currently active?
    if (b_StringSet == true && c_DevicePool.GetPlaybackActive() == false)
    {
        // Send info about performed output
        b_StringSet = false;
        
        try
        {
            OutputPerformed(u32_SayStringID, u32_SayGroupID);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to set performed: " +
                                                                 std::string(e.what()),
                                           "Voice.cpp", __LINE__);
        }
    }
    
    // Do we need to set a string?
    if (b_StringSet == false && c_OutputStorage.GetFinishedAvailable() == true)
    {
        OutputStorage::String c_String = c_OutputStorage.GetFinishedString();
        
        try
        {
            c_DevicePool.Playback(c_GoogleTTS.Synthesise(c_String.s_String));
            
            u32_SayStringID = c_String.u32_StringID;
            u32_SayGroupID = c_String.u32_GroupID;
        
            b_StringSet = true;
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to start playback: " +
                                                                 std::string(e.what()),
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
