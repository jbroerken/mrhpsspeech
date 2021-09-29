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
#include <google/longrunning/operations.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./GoogleSTT.h"
#include "../../../Configuration.h"

// Pre-defined
#define AUDIO_WRITE_SIZE_ELEMENTS 32 * 1024 // Google recommends 64 * 1024 in bytes, so /2 for PCM16 elements

using google::cloud::speech::v1::Speech;
using google::cloud::speech::v1::LongRunningRecognizeRequest;
using google::cloud::speech::v1::LongRunningRecognizeResponse;
using google::cloud::speech::v1::RecognitionConfig;
using google::cloud::speech::v1::StreamingRecognitionResult;


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

GoogleSTT::GoogleSTT() : b_Update(true)
{
    try
    {
        // Set initial audio to write
        l_Audio.emplace_back(std::make_pair(0, std::vector<MRH_Sint16>()));
        
        c_Thread = std::thread(Transcribe, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start Google Cloud STT thread!" + std::string(e.what()));
    }
}

GoogleSTT::~GoogleSTT() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Reset
//*************************************************************************************

void GoogleSTT::ResetAudio() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_AudioMutex);
    l_Audio.clear();
}

void GoogleSTT::ResetStrings() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_TranscribeMutex);
    l_Transcribed.clear();
}

//*************************************************************************************
// Audio
//*************************************************************************************

void GoogleSTT::AddAudio(MonoAudio const& c_Audio) noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_AudioMutex);
    
    // Reset for changes
    if (l_Audio.back().first != c_Audio.u32_KHz)
    {
        l_Audio.back().first = c_Audio.u32_KHz;
        l_Audio.back().second = c_Audio.v_Buffer;
    }
    else
    {
        l_Audio.back().second.insert(l_Audio.back().second.end(),
                                     c_Audio.v_Buffer.begin(),
                                     c_Audio.v_Buffer.end());
    }
}

void GoogleSTT::ProcessAudio() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_AudioMutex);
    l_Audio.emplace_back(std::make_pair(0, std::vector<MRH_Sint16>()));
}

//*************************************************************************************
// Transcribe
//*************************************************************************************

void GoogleSTT::Transcribe(GoogleSTT* p_Instance) noexcept
{
    // Service vars
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    
    // Audio vars
    std::pair<MRH_Uint32, std::vector<MRH_Sint16>> c_Audio;
    
    std::mutex& c_AudioMutex = p_Instance->c_AudioMutex;
    std::mutex& c_TranscribeMutex = p_Instance->c_TranscribeMutex;
    std::list<std::pair<MRH_Uint32, std::vector<MRH_Sint16>>>& l_Audio = p_Instance->l_Audio;
    std::list<std::string>& l_Transcribed = p_Instance->l_Transcribed;
    
    // Google vars
    std::string s_LangCode = Configuration::Singleton().GetGoogleLanguageCode();
    
    while (p_Instance->b_Update == true)
    {
        /**
         *  Data Grab
         */
        
        // Grab allowed audio buffer
        c_AudioMutex.lock();
        
        if (l_Audio.size() > 1) // (size() - 1) is the audio being added
        {
            c_Audio = std::move(l_Audio.front());
            l_Audio.pop_front();
            c_AudioMutex.unlock();
        }
        else
        {
            c_AudioMutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        /**
         *  Credentials Setup
         */
        
        // @NOTE: Google speech api is accessed as shown here:
        //        https://github.com/GoogleCloudPlatform/cpp-samples/blob/main/speech/api/async_transcribe.cc
        
        // Setup google connection with credentials for this request
        auto c_Credentials = grpc::GoogleDefaultCredentials();
        auto c_CloudChannel = grpc::CreateChannel("speech.googleapis.com", c_Credentials);
        std::unique_ptr<Speech::Stub> p_Speech(Speech::NewStub(c_CloudChannel));
        
        /**
         *  Create Long operation Request
         */
        
        // Create the long running operation
        std::unique_ptr<google::longrunning::Operations::Stub> p_LongOperation(google::longrunning::Operations::NewStub(c_CloudChannel));
        
        // Define our request to use for config and audio
        LongRunningRecognizeRequest c_RecognizeRequest;
        
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
         *  Run Long Operation
         */
        
        grpc::ClientContext c_Context;
        google::longrunning::Operation c_Operation;
        grpc::Status c_RPCStatus = p_Speech->LongRunningRecognize(&c_Context,
                                                                  c_RecognizeRequest,
                                                                  &c_Operation);
        
        if (c_RPCStatus.ok() == false)
        {
            c_Logger.Log(MRH_PSBLogger::ERROR, "GRPC Streamer error: " +
                                               c_RPCStatus.error_message(),
                         "GoogleSTT.cpp", __LINE__);
            continue;
        }
        
        /**
         *  Wait For Result
         */
        
        google::longrunning::GetOperationRequest c_OperationRequest;
        c_OperationRequest.set_name(c_Operation.name());
        bool b_RPCError = false;
        
        while (c_Operation.done() == false)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            
            grpc::ClientContext c_OperationContext;
            c_RPCStatus = p_LongOperation->GetOperation(&c_OperationContext,
                                                        c_OperationRequest,
                                                        &c_Operation);
            
            if (c_RPCStatus.ok() == false)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "GRPC Streamer error: " +
                                                   c_RPCStatus.error_message(),
                             "GoogleSTT.cpp", __LINE__);
                
                // Something went wrong, return to loop start
                b_RPCError = false;
                break;
            }
        }
        
        // End here, can't continue
        if (b_RPCError == true)
        {
            continue;
        }
        
        // Unpack the response
        if (c_Operation.response().Is<LongRunningRecognizeResponse>() == false)
        {
            c_Logger.Log(MRH_PSBLogger::ERROR, "Operation completed with wrong response!",
                         "GoogleSTT.cpp", __LINE__);
            continue;
        }
        
        LongRunningRecognizeResponse c_RecognizeResponse;
        c_Operation.response().UnpackTo(&c_RecognizeResponse);
        
        /**
         *  Add Transcribed
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
        
        // Add to strings
        if (s_Transcipt.size() > 0)
        {
            c_TranscribeMutex.lock();
            l_Transcribed.emplace_back(s_Transcipt);
            c_TranscribeMutex.unlock();
        }
    } // Loop End
}

//*************************************************************************************
// Getters
//*************************************************************************************

std::list<std::string> GoogleSTT::GetStrings() noexcept
{
    c_TranscribeMutex.lock();
    
    std::list<std::string> l_Result = std::move(l_Transcribed);
    l_Transcribed = {};
    
    c_TranscribeMutex.unlock();
    
    return l_Result;
}
