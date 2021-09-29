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
#include <portaudio.h>

// Project
#include "./Callback/Service/CBAvail.h"
#include "./Callback/Service/CBCustomCommand.h"
#include "./Callback/Speech/CBSayString.h"
#include "./Callback/Speech/CBSpeechMethod.h"
#include "./Configuration.h"
#include "./Revision.h"

// Pre-defined
#ifndef MRH_SPEECH_SERVICE_THREAD_COUNT
    #define MRH_SPEECH_SERVICE_THREAD_COUNT 2
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
/*
#include <clocale>
#include <unistd.h>
*/
int main(int argc, const char* argv[])
{
    /*
    std::setlocale(LC_ALL, "en_US.UTF-8");
    Configuration::Singleton().Load();
    PaError i_Err;
    if ((i_Err = Pa_Initialize()) != paNoError)
    {
        return EXIT_FAILURE;
    }
    
    Speech c_Speech;
    sleep(300);
    return 0;
    */
    
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
    
    // Setup PortAudio
    PaError i_Error;
    if ((i_Error = Pa_Initialize()) != paNoError)
    {
        return Exit(p_Context,
                    ("Failed to initialuze PortAudio! " + std::string(Pa_GetErrorText(i_Error))).c_str(),
                    EXIT_FAILURE);
    }
    
    // Setup service specific data
    c_Logger.Log(MRH_PSBLogger::INFO, "Initializing mrhpsspeech (" + std::string(VERSION_NUMBER) + ")...",
                 "Main.cpp", __LINE__);
    
    try
    {
        std::shared_ptr<Speech> p_Speech(new Speech());
        
        std::shared_ptr<MRH_Callback> p_CBAvail(new CBAvail(p_Speech));
        std::shared_ptr<MRH_Callback> p_CBCustomCommand(new CBCustomCommand());
        
        std::shared_ptr<MRH_Callback> p_CBSayString(new CBSayString(p_Speech));
        std::shared_ptr<MRH_Callback> p_CBSpeechMethod(new CBSpeechMethod(p_Speech));
        
        p_Context->AddCallback(p_CBAvail, MRH_EVENT_LISTEN_AVAIL_U);
        p_Context->AddCallback(p_CBAvail, MRH_EVENT_SAY_AVAIL_U);
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
