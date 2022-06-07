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

// External
#include <google/cloud/speech/v1/cloud_speech.grpc.pb.h>
#include <google/cloud/texttospeech/v1/cloud_tts.grpc.pb.h>
#include <google/longrunning/operations.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./GoogleCloudAPI.h"
#include "../../../Configuration.h"

// Pre-defined
#define AUDIO_WRITE_SIZE_ELEMENTS 32 * 1024 // Google recommends 64 * 1024 in bytes, so /2 for PCM16 elements

using google::cloud::texttospeech::v1::TextToSpeech;
using google::cloud::texttospeech::v1::SynthesizeSpeechRequest;
using google::cloud::texttospeech::v1::SynthesizeSpeechResponse;
using google::cloud::texttospeech::v1::AudioEncoding;
using google::cloud::texttospeech::v1::SsmlVoiceGender;

using google::cloud::speech::v1::Speech;
using google::cloud::speech::v1::RecognizeRequest;
using google::cloud::speech::v1::RecognizeResponse;
using google::cloud::speech::v1::RecognitionConfig;
using google::cloud::speech::v1::StreamingRecognitionResult;


//*************************************************************************************
// Transcribe
//*************************************************************************************

std::string GoogleCloudAPI::Transcribe(AudioBuffer const& c_Audio, std::string s_LangCode)
{
    // Audio available?
    if (c_Audio.GetSampleCount() == 0)
    {
        throw Exception("No audio to transcribe added!");
    }
    
    /**
     *  Credentials Setup
     */
    
    // @NOTE: Google speech api is accessed as shown here:
    //        https://github.com/GoogleCloudPlatform/cpp-samples/blob/main/speech/api/transcribe.cc
    
    // Setup google connection with credentials for this request
    auto c_Credentials = grpc::GoogleDefaultCredentials();
    auto c_CloudChannel = grpc::CreateChannel("speech.googleapis.com", c_Credentials);
    std::unique_ptr<Speech::Stub> p_Speech(Speech::NewStub(c_CloudChannel));
    
    /**
     *  Create Request
     */
    
    // Define our request to use for config and audio
    RecognizeRequest c_RecognizeRequest;
    
    // Set recognition configuration
    auto* p_Config = c_RecognizeRequest.mutable_config();
    p_Config->set_language_code(s_LangCode);
    p_Config->set_sample_rate_hertz(c_Audio.GetKHz());
    p_Config->set_encoding(RecognitionConfig::LINEAR16);
    p_Config->set_profanity_filter(true);
    p_Config->set_audio_channel_count(1); // Always mono
    
    // Now add the audio
    c_RecognizeRequest.mutable_audio()->set_content(c_Audio.GetBuffer(),
                                                    c_Audio.GetSampleCount() * sizeof(MRH_Sint16)); // Byte len
    
    /**
     *  Transcribe
     */
    
    grpc::ClientContext c_Context;
    RecognizeResponse c_RecognizeResponse;
    grpc::Status c_RPCStatus = p_Speech->Recognize(&c_Context,
                                                   c_RecognizeRequest,
                                                   &c_RecognizeResponse);
    
    if (c_RPCStatus.ok() == false)
    {
        throw Exception("Failed to transcribe: GRPC streamer error: " + c_RPCStatus.error_message());
    }
    
    /**
     *  Select Transcribed
     */
    
    // Check all results and grab highest confidence
    float f32_Confidence = -1.f;
    std::string s_Transcipt = "";
    
    for (int i = 0; i < c_RecognizeResponse.results_size(); ++i)
    {
        const auto& c_Result = c_RecognizeResponse.results(i);
        
        for (int j = 0; j < c_Result.alternatives_size(); ++j)
        {
            const auto& c_Alternative = c_Result.alternatives(i);
            
            if (f32_Confidence < c_Alternative.confidence())
            {
                f32_Confidence = c_Alternative.confidence();
                s_Transcipt = c_Alternative.transcript();
            }
        }
    }
    
    return s_Transcipt;
}

//*************************************************************************************
// Synthesise
//*************************************************************************************

void GoogleCloudAPI::Synthesise(AudioBuffer& c_Audio, std::string const& s_String, std::string s_LangCode, MRH_Uint8 u8_VoiceGender)
{
    if (s_String.size() == 0)
    {
        throw Exception("Empty string given!");
    }
    
    SsmlVoiceGender c_VoiceGender = SsmlVoiceGender::FEMALE;
    
    if (u8_VoiceGender > 0)
    {
        c_VoiceGender = SsmlVoiceGender::MALE;
    }
    
    /**
     *  Credentials Setup
     */
    
    // Setup google connection with credentials for this request
    auto c_Credentials = grpc::GoogleDefaultCredentials();
    auto c_CloudChannel = grpc::CreateChannel("texttospeech.googleapis.com", c_Credentials);
    std::unique_ptr<TextToSpeech::Stub> p_TextToSpeech(TextToSpeech::NewStub(c_CloudChannel));
    
    /**
     *  Create request
     */
    
    // Define our request to use for config and audio
    SynthesizeSpeechRequest c_SynthesizeRequest;
    
    // Set recognition configuration
    auto* p_AudioConfig = c_SynthesizeRequest.mutable_audio_config();
    p_AudioConfig->set_audio_encoding(AudioEncoding::LINEAR16);
    p_AudioConfig->set_sample_rate_hertz(c_Audio.GetKHz());
    
    // Set output voice info
    auto* p_VoiceConfig = c_SynthesizeRequest.mutable_voice();
    p_VoiceConfig->set_ssml_gender(c_VoiceGender);
    p_VoiceConfig->set_language_code(s_LangCode);
    
    // Set the string
    c_SynthesizeRequest.mutable_input()->set_text(s_String);
    
    /**
     *  Synthesize
     */
    
    grpc::ClientContext c_Context;
    SynthesizeSpeechResponse c_SynthesizeResponse;
    grpc::Status c_RPCStatus = p_TextToSpeech->SynthesizeSpeech(&c_Context,
                                                                c_SynthesizeRequest,
                                                                &c_SynthesizeResponse);
    
    if (c_RPCStatus.ok() == false)
    {
        throw Exception("Failed to synthesise: GRPC streamer error: " + c_RPCStatus.error_message());
    }
    
    /**
     *  Add Synthesized
     */
    
    // Grab the synth data
    MRH_Sint16* p_Buffer = (MRH_Sint16*)c_SynthesizeResponse.audio_content().data();
    size_t us_Elements;
    
    if (p_Buffer == NULL || (us_Elements = c_SynthesizeResponse.audio_content().size() / sizeof(MRH_Sint16)) == 0)
    {
        throw Exception("Invalid synthesized audio!");
    }
    
    try
    {
        c_Audio.AddAudio(p_Buffer, us_Elements);
    }
    catch (...)
    {
        throw;
    }
}
