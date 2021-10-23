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
#include <limits.h>
#include <cmath>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./AudioStream.h"
#include "./AudioStreamOpCode.h"
#include "./RateConverter.h"
#include "../../../Configuration.h"

// Pre-defined
#define AVG_SAMPLES_MAX_AMOUNT (SIZE_MAX / INT16_MAX)


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

AudioStream::AudioDevice::AudioDevice(bool b_CanRecord,
                                      bool b_CanPlay) : e_State(NONE),
                                                        b_StateChanged(false),
                                                        f32_LastAmplitude(0.f),
                                                        b_CanPlay(b_CanPlay),
                                                        b_CanRecord(b_CanRecord)
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
    MRH_Sint16 p_Data[2048] = { 0 };
    size_t testsize = 2048;
    
    // @TODO: Switch if required and wait for response,
    //        Otherwise recieve
    //
    //        CALCULATE SAMPLE -> AVERAGE <- AMPLITUDE HERE!
    //        Also, set last average
    
    // Do we need to use arrays for the average?
    size_t us_TotalSamples = 2048; // TODO: Buffer Size
    size_t us_SampleAverage = 0;
    
    if (us_TotalSamples > AVG_SAMPLES_MAX_AMOUNT)
    {
        // First, store all samples in groups for size constraints
        size_t us_Iterations = us_TotalSamples / AVG_SAMPLES_MAX_AMOUNT;
        us_Iterations += (us_TotalSamples % AVG_SAMPLES_MAX_AMOUNT > 0 ? 1 : 0);
        
        size_t us_Pos = 0; // Pos in buffer
        
        for (size_t i = 0; i < us_Iterations; ++i)
        {
            size_t us_GroupTotal = 0;
            size_t us_End;
            size_t us_ProcessedSamples;
            
            if (us_TotalSamples > (us_Pos + AVG_SAMPLES_MAX_AMOUNT))
            {
                us_ProcessedSamples = AVG_SAMPLES_MAX_AMOUNT;
                us_End = us_Pos + AVG_SAMPLES_MAX_AMOUNT;
            }
            else
            {
                us_ProcessedSamples = us_TotalSamples - us_Pos;
                us_End = us_TotalSamples;
            }
            
            for (; us_Pos < us_End; ++us_Pos)
            {
                us_GroupTotal += abs(p_Data[us_Pos]); // TODO: Buffer read
            }
            
            us_SampleAverage += (us_GroupTotal / us_ProcessedSamples);
        }
        
        // Now, create average for all
        us_SampleAverage /= us_Iterations;
    }
    else
    {
        for (size_t i = 0; i < us_TotalSamples; ++i)
        {
            us_SampleAverage += abs(p_Data[i]); // TODO: Buffer Read
        }
        
        us_SampleAverage /= us_TotalSamples;
    }
    
    // Create float
    float f32_Average = static_cast<double>(us_SampleAverage) / 32768.f;
    
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
    // Record on all which are able to
    for (auto& Device : l_Device)
    {
        if (Device.b_CanRecord == true)
        {
            Device.SetState(RECORDING);
        }
        else
        {
            Device.SetState(NONE);
        }
    }
}

//*************************************************************************************
// Output
//*************************************************************************************

void AudioStream::Playback()
{
    // First, pick the device with the highest last average amp
    std::list<AudioDevice>::iterator PlaybackDevice = l_Device.end();
    
    for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
    {
        if (It->b_CanPlay == false) // Skip if not possible to playback
        {
            continue;
        }
        else if (PlaybackDevice == l_Device.end() || /* No device set yet */
                 PlaybackDevice->f32_LastAmplitude < It->f32_LastAmplitude) /* Device set, but quiter */
        {
            PlaybackDevice = It;
        }
    }
    
    // No devices for playback, keep recording
    if (PlaybackDevice == l_Device.end())
    {
        return;
    }
    
    // Disable recording for all and set the active device for playback
    for (auto It = l_Device.begin(); It != l_Device.end(); ++It)
    {
        // Playback on the active playback device, stop all others
        if (It == PlaybackDevice)
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
            if (It->b_CanRecord == false || It->v_Recieved.size() >= us_Sample)
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
    }
    
    return MonoAudio(v_Buffer.data(),
                     v_Buffer.size(),
                     c_RecordingFormat.first);
}

bool AudioStream::GetPlayback() noexcept
{
    for (auto& Device : l_Device)
    {
        if (Device.GetState() == PLAYBACK)
        {
            return true;
        }
    }
    
    return false;
}

bool AudioStream::GetRecording() noexcept
{
    for (auto& Device : l_Device)
    {
        if (Device.GetState() == RECORDING)
        {
            return true;
        }
    }
    
    return false;
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
