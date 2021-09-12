/**
 *  Main.cpp
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
#include <cstdlib>

// External
#include <libmrhpsb.h>

// Project
#include "./Callback/Service/CBAvail.h"
#include "./Callback/Service/CBReset.h"
#include "./Callback/Service/CBCustomCommand.h"
#include "./Callback/Speech/CBSayString.h"
#include "./Callback/Speech/CBSpeechMethod.h"
#include "./Revision.h"

// Pre-defined
#ifndef MRH_SPEECH_SERVICE_THREAD_COUNT
    #define MRH_SPEECH_SERVICE_THREAD_COUNT 1
#endif


//*************************************************************************************
// Exit
//*************************************************************************************

static int Exit(libmrhpsb* p_Context, const char* p_Exception, int i_Result)
{
    if (p_Context != NULL)
    {
        delete p_Context;
    }
    
    if (p_Exception != NULL)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, p_Exception,
                                       "Main.cpp", __LINE__);
    }
    
    return i_Result;
}

//*************************************************************************************
// Main
//*************************************************************************************

#include <clocale>
#include "PAMicrophone.h"
#include "PocketSphinx.h"
#include "Configuration.h"
#include <unistd.h>

AudioSample c_Sample(NULL,
                     0,
                     0,
                     0,
                     0);

static int AudioCB(const void *input,
                   void *output,
                   unsigned long frameCount,
                   const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags statusFlags,
                   void *userData)
{
    static size_t us_Pos = 0;
    
    memset(output, 0, frameCount);
    
    if (us_Pos + frameCount < c_Sample.v_Buffer.size())
    {
        memcpy(output, c_Sample.v_Buffer.data() + us_Pos, frameCount);
        us_Pos += frameCount;
        
        return paContinue;
    }
    else
    {
        return paComplete;
    }
}

void PlayAudio()
{
    PaStream* p_Stream;
    PaError i_Error;
    
    // Set output stream info
    PaStreamParameters c_OutputParameters;
    MRH_Uint32 u32_DevID = Configuration::Singleton().GetPAMicDeviceID();
    
    bzero(&c_OutputParameters, sizeof(c_OutputParameters));
    c_OutputParameters.channelCount = Configuration::Singleton().GetPAMicChannels();
    c_OutputParameters.device = u32_DevID;
    c_OutputParameters.hostApiSpecificStreamInfo = NULL;
    c_OutputParameters.sampleFormat = paInt16;
    c_OutputParameters.suggestedLatency = Pa_GetDeviceInfo(u32_DevID)->defaultLowInputLatency;
    c_OutputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
    
    // Open audio stream
    i_Error = Pa_OpenStream(&p_Stream,
                            NULL,
                            &c_OutputParameters,
                            Configuration::Singleton().GetPAMicKHz(),
                            Configuration::Singleton().GetPAMicSamples(),
                            paClipOff,// paNoFlag,
                            AudioCB,
                            NULL);
    
    if (i_Error != paNoError)
    {
        throw Exception("Failed to open PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
    
    if ((i_Error = Pa_StartStream(p_Stream)) != paNoError)
    {
        throw Exception("Failed to start PortAudio stream! " + std::string(Pa_GetErrorText(i_Error)));
    }
    
    sleep(10);
}

int main(int argc, const char* argv[])
{
    std::setlocale(LC_ALL, "en_US.UTF-8");
    Configuration::Singleton().Load();
    Speech c_Speech;
    
    sleep(30);
    
    //c_Audio.StopListening();
    
    //sleep(1);
    
    //c_Sample = c_Audio.GetAudioSample();
    
    //PlayAudio();
    //RunSphinx();
    
    return 0;
    
    // Setup service base
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    libmrhpsb* p_Context;
    
    try
    {
        Configuration::Singleton().Load();
        p_Context = new libmrhpsb("mrhpsspeech",
                                  argc,
                                  argv,
                                  MRH_SPEECH_SERVICE_THREAD_COUNT,
                                  true);
    }
    catch (MRH_PSBException& e)
    {
        return Exit(NULL, e.what(), EXIT_FAILURE);
    }
    catch (std::exception& e)
    {
        return Exit(NULL, e.what(), EXIT_FAILURE);
    }
    
    // Setup service specific data
    c_Logger.Log(MRH_PSBLogger::INFO, "Initializing mrhpsspeech (" + std::string(VERSION_NUMBER) + ")...",
                 "Main.cpp", __LINE__);
    
    try
    {
        std::shared_ptr<Speech> p_Speech(new Speech());
        
        std::shared_ptr<MRH_Callback> p_CBAvail(new CBAvail(p_Speech));
        std::shared_ptr<MRH_Callback> p_CBReset(new CBReset(p_Speech));
        std::shared_ptr<MRH_Callback> p_CBCustomCommand(new CBCustomCommand());
        
        std::shared_ptr<MRH_Callback> p_CBSayString(new CBSayString(p_Speech));
        std::shared_ptr<MRH_Callback> p_CBSpeechMethod(new CBSpeechMethod(p_Speech));
        
        p_Context->AddCallback(p_CBAvail, MRH_EVENT_LISTEN_AVAIL_U);
        p_Context->AddCallback(p_CBAvail, MRH_EVENT_SAY_AVAIL_U);
        p_Context->AddCallback(p_CBReset, MRH_EVENT_PS_RESET_REQUEST_U);
        p_Context->AddCallback(p_CBCustomCommand, MRH_EVENT_LISTEN_CUSTOM_COMMAND_U);
        p_Context->AddCallback(p_CBCustomCommand, MRH_EVENT_SAY_CUSTOM_COMMAND_U);
        
        p_Context->AddCallback(p_CBSayString, MRH_EVENT_SAY_STRING_U);
        p_Context->AddCallback(p_CBSpeechMethod, MRH_EVENT_LISTEN_GET_METHOD_U);
        p_Context->AddCallback(p_CBSpeechMethod, MRH_EVENT_SAY_GET_METHOD_U);
    }
    catch (MRH_PSBException& e)
    {
        return Exit(p_Context, e.what(), EXIT_FAILURE);
    }
    catch (std::exception& e)
    {
        return Exit(p_Context, e.what(), EXIT_FAILURE);
    }
    
    c_Logger.Log(MRH_PSBLogger::INFO, "Successfully intialized mrhpsspeech service!",
                 "Main.cpp", __LINE__);
    
    // Update service until termination
    p_Context->Update();
    
    // Exit
    c_Logger.Log(MRH_PSBLogger::INFO, "Terminating service.",
                 "Main.cpp", __LINE__);
    delete p_Context;
    return EXIT_SUCCESS;
}
