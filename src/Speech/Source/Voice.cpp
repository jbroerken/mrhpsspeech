/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// C / C++
#include <cstring>

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
                                                     b_InitialRecording(false),
                                                     c_Output(c_Configuration.GetVoicePlaybackKHz()),
                                                     b_OutputSet(false),
                                                     e_APIProvider(static_cast<APIProvider>(c_Configuration.GetVoiceAPIProvider())),
#if MRH_API_PROVIDER_GOOGLE_CLOUD_API > 0
                                                     s_GoogleLangCode(c_Configuration.GetGoogleLanguageCode()),
                                                     u8_GoogleVoiceGender(c_Configuration.GetGoogleVoiceGender())
#endif
{
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Using audio stream communication. API providers are: "
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
    MRH_Uint32 u32_Message = MRH_LS_M_AUDIO_START_RECORDING;
    std::vector<MRH_Uint8> v_Message(&u32_Message, &u32_Message + sizeof(MRH_Uint32));
    
    LocalStream::Send(v_Message);
}

void Voice::StopRecording() noexcept
{
    MRH_Uint32 u32_Message = MRH_LS_M_AUDIO_STOP_RECORDING;
    std::vector<MRH_Uint8> v_Message(&u32_Message, &u32_Message + sizeof(MRH_Uint32));
    
    LocalStream::Send(v_Message);
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
        
        // Reset recording start on connection request
        if (b_InitialRecording == false)
        {
            b_InitialRecording = true;
        }
        
        return u32_StringID;
    }
    
    // Request recording start
    // @NOTE: We do this on every connection to make sure the new
    //        audio supplier knows it's OK to start recording!
    if (b_InitialRecording == false)
    {
        StartRecording();
        b_InitialRecording = true;
    }
    
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    
    // Recieve data
    std::vector<MRH_Uint8> v_Message;
    MRH_LS_M_Audio_Data c_Message;
    
    try
    {
        while (LocalStream::Receive(v_Message) == true)
        {
            // Is this a usable opcode?
            switch (MRH_LS_GetBufferMessage(v_Message.data()))
            {
                /**
                 *  Input
                 */
                
                case MRH_LS_M_AUDIO:
                {
                    if (MRH_LS_BufferToMessage(&c_Message, v_Message.data(), v_Message.size()) < 0)
                    {
                        c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetLocalStreamErrorString(),
                                     "Voice.cpp", __LINE__);
                    }
                    else
                    {
                        // @NOTE: Messages are sent / recieved in sequence
                        //        Adding them in a loop adds them correctly
                        c_Input.AddAudio(c_Message.p_Samples,
                                         c_Message.u32_Samples);
                        
                        // Increase timer for timeout to transcribe
                        u64_LastAudioTimePointS = time(NULL);
                    }
                    break;
                }
                    
                /**
                 *  Output
                 */
                    
                case MRH_LS_M_AUDIO_PLAYBACK_FINISHED:
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
                    
                default: 
                { 
                    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, "Unknown local stream message recieved!",
                                                   "Voice.cpp", __LINE__);
                    break; 
                }
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
        throw Exception("Audio local stream is not connected!");
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
        
        // Create output messages
        MRH_LS_M_Audio_Data c_Message;
        std::vector<MRH_Uint8> v_Message(MRH_STREAM_MESSAGE_TOTAL_SIZE, 0);
        MRH_Uint32 u32_Size;
        
        // Set KHz for all
        c_Message.u32_KHz = c_Output.GetKHz();
        
        // @TODO: This is a lot of copying at the moment,
        //        find a better solution!
        //        Simply assigning (if possible for message struct) doesn't
        //        work since there is no guarantee when the message will be 
        //        sent!
        size_t us_TotalSize = c_Output.GetSampleCount() * sizeof(MRH_Sint16);
        MRH_Uint32 u32_SamplesPerMessage = MRH_STREAM_MESSAGE_AUDIO_BUFFER_SIZE;
        MRH_Uint32 u32_CopySize;
        
        const MRH_Uint8* p_Current = (const MRH_Uint8*)(c_Output.GetBuffer());
        const MRH_Uint8* p_End = p_Current + us_TotalSize;
        
        while (p_Current != p_End)
        {
            // Perform copy to buffer
            if ((p_End - p_Current) > us_TotalSize)
            {
                u32_CopySize = us_TotalSize - (p_End - p_Current);
            }
            else
            {
                u32_CopySize = u32_SamplesPerMessage;
            }
            
            c_Message.u32_Samples = (u32_CopySize / 2);
            std::memcpy(c_Message.p_Samples, p_Current, u32_CopySize);
            
            p_Current += u32_CopySize;
            
            // Send message with copied audio
            if (MRH_LS_MessageToBuffer(&(v_Message[0]), &u32_Size, MRH_LS_M_AUDIO, &c_Message) < 0)
            {
                // @NOTE: No crashing, hope for next message to work
                continue;
            }
            
            LocalStream::Send(v_Message);
        }
        
        // Sent, clear
        c_Output.Clear(c_Output.GetKHz());
        
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
