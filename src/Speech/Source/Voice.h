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

#ifndef Voice_h
#define Voice_h

// C / C++

// External
#include <libmrhpsb/MRH_Callback.h>

// Project
#include "./APIProvider/APIProvider.h"
#include "./Audio/AudioBuffer.h"
#include "../../Configuration.h"
#include "../LocalStream.h"
#include "../OutputStorage.h"


class Voice : private LocalStream
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param c_Configuration The configuration to construct with.
     */
    
    Voice(Configuration const& c_Configuration);
    
    /**
     *  Default destructor.
     */
    
    ~Voice() noexcept;
    
    //*************************************************************************************
    // Recording
    //*************************************************************************************
    
    /**
     *  Start recording.
     */
    
    void StartRecording() noexcept;
    
    /**
     *  Stop recording.
     */
    
    void StopRecording() noexcept;
    
    //*************************************************************************************
    // Retrieve
    //*************************************************************************************
    
    /**
     *  Retrieve recieved data from the voice source.
     *
     *  \param u32_StringID The string id to use for the first input.
     *  \param b_DiscardInput If recieved input should be discarded.
     *
     *  \return The new string id after retrieving.
     */
    
    MRH_Uint32 Retrieve(MRH_Uint32 u32_StringID, bool b_DiscardInput);
    
    //*************************************************************************************
    // Send
    //*************************************************************************************
    
    /**
     *  Add output to send to the voice source.
     *
     *  \param c_OutputStorage The output storage to send from.
     */
    
    void Send(OutputStorage& c_OutputStorage);
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the voice source connection status.
     *
     *  \return true if connected, false if not.
     */
    
    bool GetSourceConnected() noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Input
    AudioBuffer c_Input;
    MRH_Uint32 u32_RecordingTimeoutS;
    MRH_Uint64 u64_LastAudioTimePointS;
    bool b_InitialRecording;
    
    // Output
    AudioBuffer c_Output;
    bool b_OutputSet;
    MRH_Uint32 u32_OutputID;
    MRH_Uint32 u32_OutputGroup;
    
    // Google Cloud API
    APIProvider e_APIProvider;
#if MRH_API_PROVIDER_GOOGLE_CLOUD_API > 0
    std::string s_GoogleLangCode;
    MRH_Uint8 u8_GoogleVoiceGender;
#endif
    
protected:

};

#endif /* Voice_h */
