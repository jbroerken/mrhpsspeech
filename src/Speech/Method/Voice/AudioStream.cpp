/**
 *  AudioStream.cpp
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
#include <cmath>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./AudioStream.h"
#include "./AudioStreamOpCode.h"
#include "./RateConverter.h"
#include "../../../Configuration.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioStream::AudioStream()
{
    // @TODO: Read Device List, add connections for all
    //        Individual exception for all devices
    //          -> Set Active to list end on no devices
    //          -> Also set formats from config!
}

AudioStream::~AudioStream() noexcept
{}

AudioStream::AudioDevice::AudioDevice() : e_State(NONE),
                                          b_StateChanged(false)
{
    // @TODO: Socket connection to device, throw on failed
    
    try
    {
        c_Thread = std::thread(AudioDevice::Update, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start audio device thread!" + std::string(e.what()));
    }
}

AudioStream::AudioDevice::~AudioDevice() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Update
//*************************************************************************************

void AudioStream::AudioDevice::Update(AudioDevice* p_Instance) noexcept
{
    while (p_Instance->b_Update == true)
    {
        // @TODO: Run depending on state
    }
}

void AudioStream::AudioDevice::Record() noexcept
{
    // @TODO: Switch if required and wait for response,
    //        Otherwise recieve
    //
    //        CALCULATE SAMPLE PEAK AMPLITUDE HERE!
    
    
    /*
    // Calculate peak amplitude
    us_Elements = v_Buffer.size();
    float f32_Value;

    for (size_t i = 0; i < us_Elements; ++i)
    {
        // Get float value first
        f32_Value = v_Buffer[i] / 32768.f;
        f32_Value = fabs(f32_Value);
        
        // Check the current peak
        if (f32_Value > f32_Peak)
        {
            f32_Peak = f32_Value;
        }
    }
    */
}

void AudioStream::AudioDevice::Play() noexcept
{
    // @TODO: Switch if required and wait for response,
    //        Otherwise send
    //        -> Data is correct here, no checking needed!
}

//*************************************************************************************
// Stream
//*************************************************************************************

void AudioStream::StopAll() noexcept
{
    // Stop all for everything
    for (auto& Device : l_Device)
    {
        Device.SetState(NONE);
    }
}

//*************************************************************************************
// Input
//*************************************************************************************

void AudioStream::Record()
{
    // Record on all
    for (auto& Device : l_Device)
    {
        Device.SetState(RECORDING);
    }
}

//*************************************************************************************
// Output
//*************************************************************************************

void AudioStream::Playback()
{
    // Disable recording for all and set the active device for playback
    for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
    {
        if (It == ActiveDevice)
        {
            // Copy audio before sending and pad if wierd order
            It->c_SendMutex.lock();
            It->v_Send = v_Send; // @NOTE: No clearing, expect override
            It->c_SendMutex.unlock();
            
            // Now start playback
            It->SetState(PLAYBACK);
        }
        else
        {
            // Expected to be further away, do nothing
            // We want the speaker closest to the input
            It->SetState(NONE);
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

AudioStream::AudioState AudioStream::AudioDevice::GetState() noexcept
{
    return e_State;
}

MonoAudio AudioStream::GetRecordedAudio() noexcept
{
    // We have to check all samples on which to use
    std::list<AudioDevice>::iterator Choice;
    std::vector<MRH_Sint16> v_Buffer;
    size_t us_Sample = 0;
    
    while (true)
    {
        float f32_PeakAmplitude = -1.f;
        Choice = l_Device.end();
        
        // We need to check each device for the highest sample amplitude
        // We then add the loudest one
        for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
        {
            // No samples?
            if (It->v_Recieved.size() >= us_Sample)
            {
                continue;
            }
            
            // Is this louder than the other one?
            if (f32_PeakAmplitude < It->v_Recieved[us_Sample].first)
            {
                f32_PeakAmplitude = It->v_Recieved[us_Sample].first;
                Choice = It;
            }
        }
        
        // Any choice to add?
        if (Choice == l_Device.end())
        {
            break;
        }
        
        // Found, add
        v_Buffer.insert(v_Buffer.end(),
                        Choice->v_Recieved[us_Sample].second.begin(),
                        Choice->v_Recieved[us_Sample].second.end());
        ++us_Sample;
        
        // Set the active device
        // @NOTE: This may look a bit wierd, but it ensures that the
        //        active device is set way less
        if (us_Sample + 1 == Choice->v_Recieved.size())
        {
            ActiveDevice = Choice;
        }
    }
    
    return MonoAudio(v_Buffer.data(),
                     v_Buffer.size(),
                     c_RecordingFormat.first);
}

bool AudioStream::GetPlayback() noexcept
{
    // Check the state of the active device, active is always
    // the device which got the loudest recording
    if (ActiveDevice == l_Device.end())
    {
        return false;
    }
    
    return ActiveDevice->GetState() == PLAYBACK ? true : false;
}

bool AudioStream::GetRecording() noexcept
{
    // Check the state of the active device, active is always
    // the device which got the loudest recording
    if (ActiveDevice == l_Device.end())
    {
        return false;
    }
    
    return ActiveDevice->GetState() == RECORDING ? true : false;
}

//*************************************************************************************
// Setters
//*************************************************************************************

void AudioStream::AudioDevice::SetState(AudioState e_State) noexcept
{
    this->e_State = e_State;
    b_StateChanged = true;
}

void AudioStream::SetPlaybackAudio(MonoAudio const& c_Audio)
{
    if (GetPlayback() == true)
    {
        throw Exception("Playback in progress!");
    }
    else if (c_Audio.v_Buffer.size() == 0)
    {
        throw Exception("Recieved empty playback buffer!");
    }
    
    // Copy to buffer
    if (c_Audio.u32_KHz != c_PlaybackFormat.first)
    {
        // Difference, we need to convert
        try
        {
            v_Send = RateConverter(c_Audio.u32_KHz).Convert(c_Audio.v_Buffer,
                                                            c_Audio.u32_KHz);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                           "AudioStream.cpp", __LINE__);
            return;
        }
    }
    else
    {
        // Same KHz, simply copy
        v_Send = c_Audio.v_Buffer;
    }
    
    // Check if padding is needed
    size_t us_Padding = v_Send.size() % c_PlaybackFormat.second;
    
    if (us_Padding != 0)
    {
        v_Send.insert(v_Send.end(), us_Padding, 0);
    }
}
