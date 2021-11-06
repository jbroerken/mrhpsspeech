/**
 *  AudioStream.cpp
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

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./AudioStream.h"
#include "../../../Configuration.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioStream::AudioStream() : c_AudioStream("speech_audio"),
                             c_RecordedAudio(Configuration::Singleton().GetRecordingKHz(),
                                             Configuration::Singleton().GetRecordingFrameSamples(),
                                             Configuration::Singleton().GetRecordingStorageS(),
                                             false),
                             b_RecordingActive(false),
                             b_PlaybackActive(false)
{}

AudioStream::~AudioStream() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void AudioStream::UpdateStream() noexcept
{
    /**
     *  State
     */
    
    static bool b_LastState = c_AudioStream.GetConnected();
    
    if (c_AudioStream.GetConnected() == true && b_LastState == false)
    {
        // Send format on new connection
        MessageOpCode::AUDIO_S_AUDIO_FORMAT_DATA c_OpCode(Configuration::Singleton().GetRecordingKHz(),
                                                          Configuration::Singleton().GetRecordingFrameSamples(),
                                                          Configuration::Singleton().GetPlaybackKHz(),
                                                          Configuration::Singleton().GetPlaybackFrameSamples());
        
        c_AudioStream.Send(c_OpCode.v_Data);
        
        b_LastState = true;
    }
    else if (c_AudioStream.GetConnected() == false && b_LastState == true)
    {
        b_LastState = false;
    }
    
    /**
     *  Recieve
     */
    
    std::vector<MRH_Uint8> v_Data;
    bool b_AddAudio = true;
    
    while (c_AudioStream.Recieve(v_Data) == true)
    {
        switch (MessageOpCode::GetOpCode(v_Data))
        {
            // @NOTE: We only care about recieved audio
            //        and the playback state!
                
            case MessageOpCode::AUDIO_CS_AUDIO:
            {
                // We're not recording, discard audio
                if (b_RecordingActive == false || b_AddAudio == false)
                {
                    break;
                }
                
                MessageOpCode::AUDIO_CS_AUDIO_DATA c_OpCode(v_Data);
                
                try
                {
                    c_RecordedAudio.AddAudio(c_OpCode.GetAudioBuffer(),
                                             c_OpCode.GetSampleCount());
                }
                catch (Exception& e)
                {
                    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Failed to add recorded audio: " +
                                                                           std::string(e.what()),
                                                   "AudioStream.cpp", __LINE__);
                }
                break;
            }
                
            case MessageOpCode::AUDIO_C_PLAYBACK_FINISHED:
                if (b_PlaybackActive == true)
                {
                    b_PlaybackActive = false;
                }
                break;
                
            default:
                break;
        }
    }
}

//*************************************************************************************
// Record
//*************************************************************************************

void AudioStream::ClearRecording() noexcept
{
    c_AudioStream.ClearRecieved(MessageOpCode::AUDIO_CS_AUDIO); // Audio only removal
    c_RecordedAudio.Clear();
}

void AudioStream::StartRecording()
{
    if (c_AudioStream.GetConnected() == false)
    {
        throw Exception("No stream connection!");
    }
    else if (b_RecordingActive == true)
    {
        return;
    }
    
    // @NOTE: No stream update, no external data needed
    
    static const MessageOpCode::OpCodeData c_OpCode(MessageOpCode::AUDIO_S_START_RECORDING);
    
    c_AudioStream.Send(c_OpCode.v_Data);
    b_RecordingActive = true;
}

void AudioStream::StopRecording()
{
    if (c_AudioStream.GetConnected() == false)
    {
        throw Exception("No stream connection!");
    }
    else if (b_RecordingActive == false)
    {
        return;
    }
    
    // @NOTE: No stream update, no external data needed
    
    static const MessageOpCode::OpCodeData c_OpCode(MessageOpCode::AUDIO_S_STOP_RECORDING);
    
    c_AudioStream.Send(c_OpCode.v_Data);
    b_RecordingActive = false;
}

//*************************************************************************************
// Playback
//*************************************************************************************

void AudioStream::Playback(AudioTrack const& c_Audio)
{
    // Audio info?
    if (c_Audio.GetAudioExists() == false)
    {
        throw Exception("No audio to play!");
    }
    else if (c_AudioStream.GetConnected() == false)
    {
        throw Exception("No stream connection!");
    }
    
    // Playback is set as active, still the case?
    if (b_PlaybackActive == true)
    {
        // Update stream to recieve playback finished
        UpdateStream();
        
        // Still active?
        if (b_PlaybackActive == true)
        {
            throw Exception("Playback in progress!");
        }
    }
    
    // Create messages for each audio
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    std::list<AudioTrack::Chunk> const& l_Chunk = c_Audio.GetChunksConst();
    
    for (auto& Chunk : l_Chunk)
    {
        // Samples exist?
        if (Chunk.GetElementsCurrent() == 0)
        {
            break;
        }
        
        // Create opcode and copy data
        MessageOpCode::AUDIO_CS_AUDIO_DATA c_OpCode(Chunk.GetBufferConst(),
                                                    Chunk.GetElementsCurrent());
        
        try
        {
            c_AudioStream.Send(c_OpCode.v_Data);
        }
        catch (Exception& e)
        {
            c_Logger.Log(MRH_PSBLogger::WARNING, "Failed to send audio chunk: " +
                                                 std::string(e.what()),
                         "AudioStream.cpp", __LINE__);
        }
        
        // Set active
        // @NOTE: Set in loop in case chunks fail
        if (b_PlaybackActive == false)
        {
            b_PlaybackActive = true;
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

AudioTrack const& AudioStream::GetRecordedAudio()
{
    // Clear old audio first
    c_RecordedAudio.Clear();
    
    // Next, recieve newly recorded audio
    UpdateStream();
    
    // Return "new" buffer
    return c_RecordedAudio;
}

bool AudioStream::GetConnected() noexcept
{
    return c_AudioStream.GetConnected();
}

bool AudioStream::GetRecordingActive() noexcept
{
    // @NOTE: Internal flag, does not depend on stream state!
    return b_RecordingActive;
}

bool AudioStream::GetPlaybackActive() noexcept
{
    UpdateStream(); // Recieve playback finished message
    return b_PlaybackActive;
}