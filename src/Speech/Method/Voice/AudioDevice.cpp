/**
 *  AudioDevice.cpp
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
#include "./AudioDevice.h"
#include "./AudioDeviceOpCode.h"
#include "../../../Configuration.h"

// Pre-defined
#define AVG_SAMPLES_MAX_AMOUNT (SIZE_MAX / INT16_MAX)


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioDevice::AudioDevice(MRH_Uint32 u32_ID,
                         std::string const& s_Name,
                         std::string const& s_Address,
                         int i_Port,
                         bool b_CanRecord,
                         bool b_CanPlay) : e_State(NONE),
                                           b_StateChanged(false),
                                           f32_LastAmplitude(0.f),
                                           u32_ID(u32_ID),
                                           s_Name(s_Name),
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

AudioDevice::~AudioDevice() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Update
//*************************************************************************************

void AudioDevice::Update(AudioDevice* p_Instance) noexcept
{
    while (p_Instance->b_Update == true)
    {
        // @TODO: Run depending on state
    }
}

//*************************************************************************************
// Record
//*************************************************************************************

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

//*************************************************************************************
// Playback
//*************************************************************************************

void AudioStream::AudioDevice::Play() noexcept
{
    // @TODO: Switch if required and wait for response,
    //        Otherwise send
    //        -> Data is correct here, no checking needed!
}

float AudioStream::GetRecordingAmplitude() noexcept
{
    return f32_LastAmplitude;
}

//*************************************************************************************
// Getters
//*************************************************************************************

AudioDevice::DeviceState AudioDevice::GetState() noexcept
{
    return e_State;
}

//*************************************************************************************
// Setters
//*************************************************************************************

void AudioDevice::SetState(DeviceState e_State) noexcept
{
    this->e_State = e_State;
    b_StateChanged = true;
}
