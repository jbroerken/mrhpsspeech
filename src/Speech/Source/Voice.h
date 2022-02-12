/**
 *  Voice.h
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

#ifndef Voice_h
#define Voice_h

// C / C++

// External
#include <libmrhpsb/MRH_Callback.h>
#include <libmrhmstream/MRH_MessageStream.h>

// Project
#include "./APIProvider/APIProvider.h"
#include "./Audio/AudioBuffer.h"
#include "../../Configuration.h"
#include "../OutputStorage.h"


class Voice
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
    
    bool GetSourceConnected() const noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Stream
    MRH_MessageStream c_Stream;
    
    // Input
    AudioBuffer c_Input;
    MRH_Uint32 u32_RecordingTimeoutS;
    MRH_Uint64 u64_LastAudioTimePointS;
    
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
