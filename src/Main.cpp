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
#include <SDL2/SDL.h>
/*
std::vector<Uint8> v_Buffer;

static void SDLCallback(void* p_UserData, Uint8* p_Buffer, int i_Length) noexcept
{
    static size_t us_Pos = 0;
    
    if (us_Pos + i_Length >= v_Buffer.size())
    {
        size_t us_Avail = (v_Buffer.size() - us_Pos);
        memset(p_Buffer, 0, i_Length);
        
        if (us_Avail > 0)
        {
            memcpy(p_Buffer, &(v_Buffer[us_Pos]), us_Avail);
            us_Pos += us_Avail;
        }
    }
    else
    {
        memcpy(p_Buffer, &(v_Buffer[us_Pos]), i_Length);
        us_Pos += i_Length;
    }
}

void PlayAudio()
{
    SDL_AudioSpec c_Want;
    SDL_AudioSpec c_Have;

    SDL_zero(c_Want);
    
    c_Want.freq = 16000;
    c_Want.format = AUDIO_S16SYS;
    c_Want.channels = 1;
    c_Want.samples = 1024;
    c_Want.callback = &SDLCallback;
    c_Want.userdata = NULL;
    
    if (SDL_GetNumAudioDevices(0) == 0)
    {
        printf("No Audio Devices!\n");
        return;
    }
    
    SDL_AudioDeviceID u32_DevID;
    
    if ((u32_DevID = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(3, 0), 0, &c_Want, &c_Have, 0)) == 0)
    {
        printf("Failed to open audio device!\n");
        return;
    }
    
    if (c_Want.format != c_Have.format ||
        c_Want.freq != c_Have.freq ||
        c_Want.channels != c_Have.channels)
    {
        SDL_CloseAudioDevice(u32_DevID);
        u32_DevID = 0;
        
        printf("Format mismatch!\n");
        return;
    }
    
    SDL_PauseAudioDevice(u32_DevID, 0);
}

void RunSphinx()
{
    PocketSphinx c_Sphinx(Configuration::Singleton().GetSphinxTriggerModelDirPath());
    std::vector<MRH_Uint8> v_Chunk(256, 0);
    
    c_Sphinx.StartRecognition();
    
    for (size_t i = 0; i < v_Buffer.size();)
    {
        MRH_Uint32 u32_Size;
        
        if (i < (v_Buffer.size() - 256))
        {
            u32_Size = 256;
            memcpy(&(v_Chunk[0]), &(v_Buffer[i]), u32_Size);
        }
        else
        {
            u32_Size = (v_Buffer.size() - i);
            memcpy(&(v_Chunk[0]), &(v_Buffer[i]), u32_Size);
        }
        
        i += u32_Size;
        c_Sphinx.AddSample(&(v_Chunk[0]), u32_Size);
    }
    
    std::string s_Result = c_Sphinx.Recognize();
    printf("%s\n", s_Result.c_str());
}
*/

int main(int argc, const char* argv[])
{
    std::setlocale(LC_ALL, "en_US.UTF-8");
    /*
    SDLMicrophone c_Audio;
    
    sleep(15);
    
    c_Audio.PauseListening();
    
    sleep(5);
    
    while (c_Audio.GetSampleCount() > 0)
    {
        SDLMicrophone::Sample* p_Sample = c_Audio.GrabSample(0);
        //c_Audio.ConvertTo(p_Sample, AUDIO_S16SYS, 16000, 1);
        v_Buffer.insert(v_Buffer.end(), p_Sample->v_Buffer.begin(), p_Sample->v_Buffer.end());
    }
    
    PlayAudio();
    //RunSphinx();
    
    sleep(30);
    */
    Configuration::Singleton().Load();
    PAMicrophone c_Mic;
    sleep(30);
    return 0;
    
    // Setup service base
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    libmrhpsb* p_Context;
    
    try
    {
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
