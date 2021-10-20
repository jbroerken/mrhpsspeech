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
#include <thread>
#include <atomic>

// External

// Project
#include "../SpeechMethod.h"
#include "./Voice/MonoAudio.h"
#include "./Voice/RateConverter.h"
//#include "./Voice/PADevice.h"
#include "./Voice/AudioStream.h"
#include "./Voice/PocketSphinx.h"
#include "./Voice/GoogleSTT.h"
#include "./Voice/GoogleTTS.h"


class Voice : public SpeechMethod
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    Voice();
    
    /**
     *  Default destructor.
     */
    
    ~Voice() noexcept;
    
    //*************************************************************************************
    // Useage
    //*************************************************************************************
    
    /**
     *  Resume speech method.
     */
    
    void Resume() override;
    
    /**
     *  Reset speech method.
     */
    
    void Reset() override;
    
    /**
     *  Pause speech method.
     */
    
    void Pause() override;
    
    //*************************************************************************************
    // Listen
    //*************************************************************************************
    
    /**
     *  Listen to speech input.
     */
    
    void Listen() override;
    
    //*************************************************************************************
    // Say
    //*************************************************************************************
    
    /**
     *  Perform speech output.
     *
     *  \param c_OutputStorage The output storage to use.
     */
    
    void Say(OutputStorage& c_OutputStorage) override;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if this speech method is usable.
     *
     *  \return true if usable, false if not.
     */
    
    bool IsUsable() noexcept override;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Components
    AudioStream c_AudioStream;
    PocketSphinx c_PocketSphinx;
    GoogleSTT c_GoogleSTT;
    GoogleTTS c_GoogleTTS;
    
    // Trigger
    MonoAudio c_TriggerSound;
    MRH_Uint64 u64_TriggerValidS;
    bool b_PlayTriggerSound;
    
    // Listen Input
    bool b_ListenAudioAvailable;
    size_t us_ListenWaitSamples;
    
    // Speech output
    bool b_StringSet;
    MRH_Uint32 u32_SayStringID;
    MRH_Uint32 u32_SayGroupID;
    
    // Convert
    RateConverter c_Converter;
    
protected:

};

#endif /* Voice_h */
