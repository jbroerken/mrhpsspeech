/**
 *  PAMicrophone.cpp
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
#include <cstring>
#include <cmath>

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./PAMicrophone.h"
#include "../../../Configuration.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

bool PAMicrophone::b_SetupPA = true;

PAMicrophone::PAMicrophone()
{
    Configuration& c_Configuration = Configuration::Singleton();
    PaError i_Error;
    
    // Setup PortAudio
    if (b_SetupPA == true)
    {
        if ((i_Error = Pa_Initialize()) != paNoError)
        {
            throw Exception("Failed to initialuze PortAudio! " + std::string(Pa_GetErrorText(i_Error)));
        }
        
        b_SetupPA = true;
    }
    
    // Update audio info
    c_Audio.u16_Format = paInt16;
    c_Audio.u32_KHz = c_Configuration.GetPAMicKHz();
    c_Audio.u8_Channels = c_Configuration.GetPAMicChannels();
    c_Audio.u32_StreamLengthS = c_Configuration.GetPAMicStreamLengthS();
    
    // Set input stream info
    PaStreamParameters c_InputParameters;
    MRH_Uint32 u32_DevID = c_Configuration.GetPAMicDeviceID();
    
    bzero(&c_InputParameters, sizeof(c_InputParameters));
    c_InputParameters.channelCount = c_Audio.u8_Channels;
    c_InputParameters.device = u32_DevID;
    c_InputParameters.hostApiSpecificStreamInfo = NULL;
    c_InputParameters.sampleFormat = c_Audio.u16_Format;
    c_InputParameters.suggestedLatency = Pa_GetDeviceInfo(u32_DevID)->defaultLowInputLatency;
    c_InputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
    
    printf("DevInfo: %s\n", Pa_GetDeviceInfo(u32_DevID)->name);
    printf("Channels: %d\n", Pa_GetDeviceInfo(u32_DevID)->maxInputChannels);
    
    // Open audio stream
    i_Error = Pa_OpenStream(&p_Stream,
                            &c_InputParameters,
                            NULL, // No output info
                            c_Audio.u32_KHz,
                            c_Configuration.GetPAMicSamples(),
                            paClipOff,// paNoFlag,
                            PACallback,
                            &c_Audio);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to open PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
    
    // Stream opened, start recording
    if ((i_Error = Pa_StartStream(p_Stream)) != paNoError)
    {
        throw Exception("Failed to start PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

PAMicrophone::~PAMicrophone() noexcept
{
    PaError i_Error;
    
    if ((i_Error = Pa_StopStream(p_Stream)) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to stop PortAudio Stream! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PAMicrophone.cpp", __LINE__);
    }
    else if ((i_Error = Pa_CloseStream(p_Stream)) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to close PortAudio Stream! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PAMicrophone.cpp", __LINE__);
    }
    else if ((i_Error = Pa_Terminate()) != paNoError)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to deinitialize PortAudio! " +
                                                             std::string(Pa_GetErrorText(i_Error)),
                                       "PAMicrophone.cpp", __LINE__);
    }
}

PAMicrophone::Audio::Audio() noexcept : u16_Format(paInt16),
                                        u32_KHz(16000),
                                        u8_Channels(1),
                                        u32_StreamLengthS(5)
{}

PAMicrophone::Audio::~Audio() noexcept
{
    for (auto& Sample : v_Sample)
    {
        delete Sample;
    }
}

PAMicrophone::Sample::Sample() noexcept : u16_Format(paInt16),
                                          u32_KHz(16000),
                                          u8_Channels(1),
                                          f32_Amplitude(0.f),
                                          f32_Peak(0.f),
                                          u64_TimepointS(0)
{}

PAMicrophone::Sample::~Sample() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void PAMicrophone::StartListening()
{
    PaError i_Error = Pa_StartStream(p_Stream);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to start PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

void PAMicrophone::StopListening()
{
    PaError i_Error = Pa_StopStream(p_Stream);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to stop PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
}

/*
void PAMicrophone::PACallback(void* p_UserData, Uint8* p_Buffer, int i_Length) noexcept
{
    PAMicrophone::Stream* p_Stream = static_cast<PAMicrophone::Stream*>(p_UserData);
    
    // Grab the sample to use
    Sample* p_Sample = NULL;
    Uint64 u64_TimepointS = time(NULL);
    
    p_Stream->c_Mutex.lock();
    
    if (p_Stream->v_Sample.size() > 0 &&
        p_Stream->v_Sample[0]->u64_TimepointS < (u64_TimepointS - p_Stream->u32_StreamLengthS))
    {
        // Oldest sample too old, reuse
        p_Sample = p_Stream->v_Sample[0];
        p_Stream->v_Sample.erase(p_Stream->v_Sample.begin());
    }
    
    p_Stream->c_Mutex.unlock();
    
    if (p_Sample == NULL)
    {
        // All samples valid, append new
        p_Sample = new Sample();
    }
    
    p_Sample->u64_TimepointS = u64_TimepointS; // New timepoint
    
    // Sample inherits stream format
    p_Sample->u16_Format = p_Stream->u16_Format;
    p_Sample->u32_KHz = p_Stream->u32_KHz;
    p_Sample->u8_Channels = p_Stream->u8_Channels;
    
    // Add space for all bytes
    if (p_Sample->v_Buffer.size() != i_Length)
    {
        p_Sample->v_Buffer.resize(i_Length, 0);
    }
    
    // Channel RMS
    std::vector<float> v_RMS(p_Sample->u8_Channels, 0.f);
    std::vector<float>::iterator RMS = v_RMS.begin();
    
    // Run variables
    float f32_Value;
    float f32_ABS;
    size_t us_Step = sizeof(float);

    for (size_t i = 0; i < i_Length; i += us_Step)
    {
        // Get float value first
        f32_Value = *((float*)&(p_Buffer[i]));
        
        // Check the current peak (Global for all channels)
        f32_ABS = fabs(f32_Value);
        
        if (f32_ABS > p_Sample->f32_Peak)
        {
            p_Sample->f32_Peak = f32_ABS;
        }
        
        // Grab RMS for this channel
        *RMS += f32_Value * f32_Value;
        
        if ((++RMS) == v_RMS.end())
        {
            RMS = v_RMS.begin();
        }
        
        // Copy bytes
        *((float*)&(p_Sample->v_Buffer[i])) = f32_Value;
    }
    
    // Grab max amplitude
    for (auto& Amplitude : v_RMS)
    {
        if (p_Sample->f32_Amplitude < Amplitude)
        {
            p_Sample->f32_Amplitude = Amplitude;
        }
    }
    
    p_Sample->f32_Amplitude = (sqrt(p_Sample->f32_Amplitude / ((i_Length / v_RMS.size()) / us_Step)));
    
    // Sample was built, add!
    p_Stream->c_Mutex.lock();
    p_Stream->v_Sample.emplace_back(p_Sample);
    printf("Amplitude: %f | Peak: %f | Total Samples: %zu\n", p_Sample->f32_Amplitude, p_Sample->f32_Peak, p_Stream->v_Sample.size());
    p_Stream->c_Mutex.unlock();
}
*/

int PAMicrophone::PACallback(const void* p_Input,
                             void* p_Output,
                             unsigned long u32_FrameCount,
                             const PaStreamCallbackTimeInfo* p_TimeInfo,
                             PaStreamCallbackFlags e_StatusFlags,
                             void* p_UserData) noexcept
{
    PAMicrophone::Audio* p_Audio = static_cast<PAMicrophone::Audio*>(p_UserData);
    
    printf("Frame Count: %lu\n", u32_FrameCount);
    
    for (size_t i = 0; i < u32_FrameCount; ++i)
    {
        printf("Data: %d\n", ((MRH_Sint16*)p_Output)[i]);
    }
    
    return 0;
    
    /*
    paTestData *data = (paTestData*)userData;
      100     const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
      101     SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
      102     long framesToCalc;
      103     long i;
      104     int finished;
      105     unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;
      106
      107     (void) outputBuffer; // Prevent unused variable warnings.
      108     (void) timeInfo;
      109     (void) statusFlags;
      110     (void) userData;
      111
      112     if( framesLeft < framesPerBuffer )
      113     {
      114         framesToCalc = framesLeft;
      115         finished = paComplete;
      116     }
      117     else
      118     {
      119         framesToCalc = framesPerBuffer;
      120         finished = paContinue;
      121     }
      122
      123     if( inputBuffer == NULL )
      124     {
      125         for( i=0; i<framesToCalc; i++ )
      126         {
      127             *wptr++ = SAMPLE_SILENCE;  // left
      128             if( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;  // right
      129         }
      130     }
      131     else
      132     {
      133         for( i=0; i<framesToCalc; i++ )
      134         {
      135             *wptr++ = *rptr++;  // left
      136             if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  // right
      137         }
      138     }
      139     data->frameIndex += framesToCalc;
      140     return finished;
    return -1;
     */
}

//*************************************************************************************
// Sample
//*************************************************************************************

void PAMicrophone::ConvertTo(Sample* p_Sample, PaSampleFormat u16_Format, MRH_Uint32 u32_KHz, MRH_Uint8 u8_Channels) noexcept
{
    // TODO: Convert
}

size_t PAMicrophone::GetSampleCount() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Audio.c_Mutex);
    return c_Audio.v_Sample.size();
}

PAMicrophone::Sample* PAMicrophone::GrabSample(size_t us_Sample)
{
    std::lock_guard<std::mutex> c_Guard(c_Audio.c_Mutex);
    
    std::vector<Sample*>& v_Sample = c_Audio.v_Sample;
    
    if (v_Sample.size() <= us_Sample)
    {
        throw Exception("Invalid sample requested!");
    }
    
    Sample* p_Sample = v_Sample[us_Sample];
    v_Sample.erase(v_Sample.begin() + us_Sample);
    
    return p_Sample;
}

void PAMicrophone::ReturnSample(Sample* p_Sample)
{
    std::lock_guard<std::mutex> c_Guard(c_Audio.c_Mutex);
    
    auto It = c_Audio.v_Sample.begin();
    auto End = c_Audio.v_Sample.end();
    
    for (; It != End; ++It)
    {
        if ((*It)->u64_TimepointS >= p_Sample->u64_TimepointS)
        {
            c_Audio.v_Sample.insert(It, p_Sample);
            return;
        }
    }
    
    // Newest, add at end
    c_Audio.v_Sample.emplace_back(p_Sample);
}
