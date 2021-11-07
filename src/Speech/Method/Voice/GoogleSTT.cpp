/**
 *  GoogleSTT.cpp
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
#include <google/cloud/speech/v1/cloud_speech.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./GoogleSTT.h"
#include "../../../Configuration.h"

// Pre-defined
#define AUDIO_WRITE_SIZE_ELEMENTS 32 * 1024 // Google recommends 64 * 1024 in bytes, so /2 for PCM16 elements

using google::cloud::speech::v1::Speech;
using google::cloud::speech::v1::RecognizeRequest;
using google::cloud::speech::v1::RecognizeResponse;
using google::cloud::speech::v1::RecognitionConfig;
using google::cloud::speech::v1::StreamingRecognitionResult;


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

GoogleSTT::GoogleSTT() noexcept
{}

GoogleSTT::~GoogleSTT() noexcept
{}

//*************************************************************************************
// Reset
//*************************************************************************************

void GoogleSTT::ResetAudio() noexcept
{
    c_Audio.first = 0;
    c_Audio.second = {};
}

//*************************************************************************************
// Audio
//*************************************************************************************

void GoogleSTT::AddAudio(AudioTrack const& c_Audio) noexcept
{
    // Audio empty?
    if (c_Audio.GetSampleCount() == 0)
    {
        return;
    }
    
    // New KHz?
    if (this->c_Audio.first != c_Audio.u32_KHz)
    {
        this->c_Audio.first = c_Audio.u32_KHz;
        this->c_Audio.second = {};
    }
    
    // Add audio
    this->c_Audio.second.insert(this->c_Audio.second.end(),
                                c_Audio.GetBufferConst(),
                                c_Audio.GetBufferConst() + c_Audio.GetSampleCount());
}

//*************************************************************************************
// Transcribe
//*************************************************************************************

std::string GoogleSTT::Transcribe()
{
    // Audio available?
    if (c_Audio.first == 0 || c_Audio.second.size() == 0)
    {
        throw Exception("No audio to transcribe added!");
    }
    
    std::string s_LangCode = Configuration::Singleton().GetGoogleLanguageCode();
    
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
    p_Config->set_sample_rate_hertz(c_Audio.first);
    p_Config->set_encoding(RecognitionConfig::LINEAR16);
    p_Config->set_profanity_filter(true);
    p_Config->set_audio_channel_count(1); // Always mono
    
    // Now add the audio
    c_RecognizeRequest.mutable_audio()->set_content(c_Audio.second.data(),
                                                    c_Audio.second.size() * sizeof(MRH_Sint16)); // Byte len
    
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
    
    ResetAudio(); // Clear old
    
    return s_Transcipt;
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool GoogleSTT::GetAudioAvailable() noexcept
{
    return (c_Audio.first != 0 && c_Audio.second.size() > 0) ? true : false;
}
