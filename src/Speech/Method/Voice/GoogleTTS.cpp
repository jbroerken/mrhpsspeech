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

GoogleTTS::GoogleTTS() : b_Update(true)
{
    try
    {
        c_Thread = std::thread(Process, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start Google Cloud TTS thread!" + std::string(e.what()));
    }
}

GoogleTTS::~GoogleTTS() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Reset
//*************************************************************************************

void GoogleTTS::ResetStrings() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_StringMutex);
    l_String.clear();
}

void GoogleTTS::ResetAudio() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_AudioMutex);
    l_Audio.clear();
}

//*************************************************************************************
// String
//*************************************************************************************

void GoogleTTS::AddString(std::string const& s_String)
{
    std::lock_guard<std::mutex> c_Guard(c_StringMutex);
    l_String.emplace_back(s_String);
}

//*************************************************************************************
// Process
//*************************************************************************************

void GoogleTTS::Process(GoogleTTS* p_Instance) noexcept
{
    // Service vars
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    MRH_Uint32 u32_KHz = Configuration::Singleton().GetPASpeakerKHz();
    std::string s_String;
    
    std::mutex& c_StringMutex = p_Instance->c_StringMutex;
    std::mutex& c_AudioMutex = p_Instance->c_AudioMutex;
    std::list<std::string>& l_String = p_Instance->l_String;
    std::list<MonoAudio>& l_Audio = p_Instance->l_Audio;
    
    // Google vars
    std::string s_LangCode = Configuration::Singleton().GetGoogleLanguageCode();
    SsmlVoiceGender c_VoiceGender = SsmlVoiceGender::FEMALE;
    
    if (Configuration::Singleton().GetGoogleVoiceGender() > 0)
    {
        c_VoiceGender = SsmlVoiceGender::MALE;
    }
    
    while (p_Instance->b_Update == true)
    {
        /**
         *  Data Grab
         */
        
        // Grab string to process
        c_StringMutex.lock();
        
        if (l_String.size() > 0)
        {
            s_String = l_String.front();
            l_String.pop_front();
            c_StringMutex.unlock();
        }
        else
        {
            c_StringMutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
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
        p_AudioConfig->set_sample_rate_hertz(u32_KHz);
        
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
            c_Logger.Log(MRH_PSBLogger::ERROR, "GRPC Streamer error: " +
                                               c_RPCStatus.error_message(),
                         "GoogleTTS.cpp", __LINE__);
            continue;
        }
        
        /**
         *  Add Synthesized
         */
        
        // Grab the synth data
        MRH_Sint16* p_Buffer = (MRH_Sint16*)c_SynthesizeResponse.audio_content().data();
        size_t us_Elements;
        
        if (p_Buffer == NULL || (us_Elements = c_SynthesizeResponse.audio_content().size() / sizeof(MRH_Sint16)) == 0)
        {
            c_Logger.Log(MRH_PSBLogger::ERROR, "Invalid synthesized audio!",
                         "GoogleTTS.cpp", __LINE__);
            continue;
        }
        
        // Create mono audio from it
        c_AudioMutex.lock();
        l_Audio.emplace_back(p_Buffer,
                             us_Elements,
                             u32_KHz);
        c_AudioMutex.unlock();
    }
}

//*************************************************************************************
// Audio
//*************************************************************************************

bool GoogleTTS::GetAudioAvailable() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_AudioMutex);
    return l_Audio.size() > 0 ? true : false;
}

MonoAudio GoogleTTS::GetAudio()
{
    c_AudioMutex.lock();
    
    MonoAudio c_Result = std::move(l_Audio.front());
    l_Audio.pop_front();
    
    c_AudioMutex.unlock();
    
    return c_Result;
}
