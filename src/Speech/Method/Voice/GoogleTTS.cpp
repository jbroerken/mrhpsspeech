/**
 *  GoogleTTS.cpp
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
#include <google/cloud/texttospeech/v1/cloud_tts.grpc.pb.h>
#include <google/longrunning/operations.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./GoogleTTS.h"
#include "../../../Configuration.h"

// Pre-defined
using google::cloud::texttospeech::v1::TextToSpeech;
using google::cloud::texttospeech::v1::SynthesizeSpeechRequest;
using google::cloud::texttospeech::v1::SynthesizeSpeechResponse;
using google::cloud::texttospeech::v1::AudioEncoding;
using google::cloud::texttospeech::v1::SsmlVoiceGender;


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

GoogleTTS::GoogleTTS() : c_Audio(Configuration::Singleton().GetPlaybackKHz(),
                                 Configuration::Singleton().GetPlaybackFrameSamples(),
                                 0, /* Grows on need, no initial data */
                                 true)
{}

GoogleTTS::~GoogleTTS() noexcept
{}

//*************************************************************************************
// Synthesise
//*************************************************************************************

AudioTrack const& GoogleTTS::Synthesise(std::string const& s_String)
{
    if (s_String.size() == 0)
    {
        throw Exception("Empty string given!");
    }
    
    std::string s_LangCode = Configuration::Singleton().GetGoogleLanguageCode();
    SsmlVoiceGender c_VoiceGender = SsmlVoiceGender::FEMALE;
    
    if (Configuration::Singleton().GetGoogleVoiceGender() > 0)
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
    p_AudioConfig->set_sample_rate_hertz(c_Audio.u32_KHz);
    
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
    
    // Create mono audio from it
    try
    {
        c_Audio.Clear();
        c_Audio.AddAudio(p_Buffer, us_Elements);
    
        return c_Audio;
    }
    catch (...)
    {
        throw;
    }
}
