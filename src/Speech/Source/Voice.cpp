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

// External
#include <libmrhpsb/MRH_PSBLogger.h>
#include <libmrhls.h>

// Project
#include "./Voice.h"
#if MRH_API_PROVIDER_GOOGLE_CLOUD_API > 0
#include "./APIProvider/GoogleCloudAPI.h"
#endif
#include "../SpeechEvent.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Voice::Voice(Configuration const& c_Configuration) : LocalStream(c_Configuration.GetVoiceSocketPath()),
                                                     c_Input(c_Configuration.GetVoiceRecordingKHz()),
                                                     u32_RecordingTimeoutS(c_Configuration.GetVoiceRecordingTimeoutS()),
                                                     u64_LastAudioTimePointS(time(NULL)),
                                                     c_Output(c_Configuration.GetVoicePlaybackKHz()),
                                                     b_OutputSet(false),
                                                     e_APIProvider(static_cast<APIProvider>(c_Configuration.GetVoiceAPIProvider())),
#if MRH_API_PROVIDER_GOOGLE_CLOUD_API > 0
                                                     s_GoogleLangCode(c_Configuration.GetGoogleLanguageCode()),
                                                     u8_GoogleVoiceGender(c_Configuration.GetGoogleVoiceGender())
#endif
{
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Using local audio stream. API providers are: "
#if MRH_API_PROVIDER_GOOGLE_CLOUD_API > 0
                                                        "[ Google Cloud API ]"
#endif
                                                        ".",
                                   "Voice.cpp", __LINE__);
}

Voice::~Voice() noexcept
{}

//*************************************************************************************
// Recording
//*************************************************************************************

void Voice::StartRecording() noexcept
{
    //c_Stream.Send({ MRH_MessageOpCode::AUDIO_S_STOP_RECORDING });
}

void Voice::StopRecording() noexcept
{
    //c_Stream.Send({ MRH_MessageOpCode::AUDIO_S_START_RECORDING });
}

//*************************************************************************************
// Listen
//*************************************************************************************

MRH_Uint32 Voice::Retrieve(MRH_Uint32 u32_StringID, bool b_DiscardInput)
{
    // No client or data, do nothing
    if (LocalStream::IsConnected() == false)
    {
        // Reset sent waiting
        if (b_OutputSet == true)
        {
            b_OutputSet = false;
        }
        
        return u32_StringID;
    }
    
    // Recieve data
    MRH_StreamMessage e_Message;
    void* p_Data;
    
    try
    {
        while (LocalStream::Receive(e_Message, p_Data) == true)
        {
            // Is this a usable opcode?
            switch (e_Message) // OpCode id size = Uint8
            {
                /**
                 *  Input
                 */
                
                case MRH_LSM_AUDIO:
                {
                    // @NOTE: Messages are sent / recieved in sequence
                    //        Adding them in a loop adds them correctly
                    c_Input.AddAudio(((MRH_LSM_Audio_Data*)p_Data)->p_Samples,
                                     ((MRH_LSM_Audio_Data*)p_Data)->u32_Samples);
                    
                    // Increase timer for timeout to transcribe
                    u64_LastAudioTimePointS = time(NULL);
                    break;
                }
                    
                /**
                 *  Output
                 */
                    
                case MRH_LSM_AUDIO_PLAYBACK_FINISHED:
                {
                    if (b_OutputSet == true)
                    {
                        // Reset even if performed event fails
                        b_OutputSet = false;
                        
                        SpeechEvent::OutputPerformed(u32_OutputID,
                                                     u32_OutputGroup);
                    }
                    break;
                }
                    
                /**
                 *  Default
                 */
                    
                default: { break; }
            }
            
            // Now destroy
            if (LocalStream::DestroyMessage(e_Message, p_Data) != NULL)
            {
                MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Failed to deallocate recieved message!",
                                               "Voice.cpp", __LINE__);
                
            }
        }
    }
    catch (Exception& e)
    {
        throw;
    }
    
    // Can we work with the data we have
    if (c_Input.GetSampleCount() == 0 || u64_LastAudioTimePointS < (time(NULL) + u32_RecordingTimeoutS))
    {
        return u32_StringID;
    }
    
    // Got data, should we transcribe?
    if (b_DiscardInput == false)
    {
        std::string s_Input;
        
        try
        {
            switch (e_APIProvider)
            {
#if MRH_API_PROVIDER_GOOGLE_CLOUD_API > 0
                case GOOGLE_CLOUD_API:
                    s_Input = GoogleCloudAPI::Transcribe(c_Input,
                                                         s_GoogleLangCode);
                    break;
#endif
                default:
                    throw Exception("Unknown API provider!");
            }
            
            // Transcribed, add input
            SpeechEvent::InputRecieved(u32_StringID, s_Input);
            ++u32_StringID;
        }
        catch (Exception& e)
        {
            throw;
        }
    }
          
    // Clean up and return
    c_Input.Clear(c_Input.GetKHz());
    return u32_StringID;
}

//*************************************************************************************
// Send
//*************************************************************************************

void Voice::Send(OutputStorage& c_OutputStorage)
{
    if (c_OutputStorage.GetAvailable() == false)
    {
        return;
    }
    else if (LocalStream::IsConnected() == false)
    {
        throw Exception("Audio stream is not connected!");
    }
    
    // Check if output is currently being sent
    if (b_OutputSet == true)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Can't send output, waiting for result for output" +
                                                               std::to_string(u32_OutputID),
                                       "Voice.cpp", __LINE__);
        return;
    }
    
    // Nothing sent, send next output
    try
    {
        auto String = c_OutputStorage.GetString();
        
        switch (e_APIProvider)
        {
#if MRH_API_PROVIDER_GOOGLE_CLOUD_API > 0
            case GOOGLE_CLOUD_API:
                GoogleCloudAPI::Synthesise(c_Output,
                                           String.s_String,
                                           s_GoogleLangCode,
                                           u8_GoogleVoiceGender);
                break;
#endif
            default:
                throw Exception("Unknown API provider!");
        }
        
        // Create output message
        MRH_StreamMessage e_Message = MRH_LSM_AUDIO;
        void* p_Data = (MRH_LSM_Audio_Data*)malloc(sizeof(MRH_LSM_Audio_Data));
        
        if (p_Data)
        {
            throw Exception("Failed to allocate output data!");
        }
        
        MRH_LSM_Audio_Data* p_Cast = (MRH_LSM_Audio_Data*)p_Data;
        p_Cast->u32_KHz = c_Output.GetKHz();
        p_Cast->u8_Channels = 1; // @NOTE: Change if more supported
        p_Cast->u32_Samples = c_Output.GetSampleCount();
        
        size_t us_SampleSize = p_Cast->u32_Samples * sizeof(MRH_Sint16);
        
        if ((p_Cast->p_Samples = (MRH_Sint16*)malloc(us_SampleSize)) == NULL)
        {
            free(p_Cast);
            throw Exception("Failed to allocate output sample buffer!");
        }
        
        memcpy(p_Cast->p_Samples, c_Output.GetBuffer(), us_SampleSize);
        
        // Add output to send
        c_Output.Clear(c_Output.GetKHz());
        LocalStream::Send(e_Message, p_Data);
        
        // Remember output data
        u32_OutputID = String.u32_StringID;
        u32_OutputGroup = String.u32_GroupID;
        
        b_OutputSet = true;
    }
    catch (Exception& e)
    {
        throw;
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool Voice::GetSourceConnected() noexcept
{
    return LocalStream::IsConnected();
}
