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

// C / C++
#include <cstdlib>

// External
#include <libmrhpsb.h>

// Project
#include "./Callback/Service/CBAvail.h"
#include "./Callback/Service/CBCustomCommand.h"
#include "./Callback/Speech/CBSayString.h"
#include "./Callback/Speech/CBSpeechMethod.h"
#include "./Callback/Speech/CBNotification.h"
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

int main(int argc, const char* argv[])
{
    // Setup service base
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    libmrhpsb* p_Context;
    
    try
    {
        p_Context = new libmrhpsb("mrhpsspeech",
                                  argc,
                                  argv,
                                  MRH_SPEECH_SERVICE_THREAD_COUNT);
        
        c_Logger.Log(MRH_PSBLogger::INFO, "Initializing mrhpsspeech (" + std::string(VERSION_NUMBER) + ")...",
                     "Main.cpp", __LINE__);
        
        Configuration c_Configuration;
        
        std::shared_ptr<Speech> p_Speech(new Speech(c_Configuration));
        
        std::shared_ptr<MRH_Callback> p_CBAvail(new CBAvail(p_Speech));
        std::shared_ptr<MRH_Callback> p_CBCustomCommand(new CBCustomCommand());
        
        std::shared_ptr<MRH_Callback> p_CBSayString(new CBSayString(p_Speech));
        std::shared_ptr<MRH_Callback> p_CBSpeechMethod(new CBSpeechMethod(p_Speech));
        std::shared_ptr<MRH_Callback> p_CBNotification(new CBNotification());
        
        p_Context->AddCallback(p_CBAvail, MRH_EVENT_LISTEN_AVAIL_U);
        p_Context->AddCallback(p_CBAvail, MRH_EVENT_SAY_AVAIL_U);
        p_Context->AddCallback(p_CBCustomCommand, MRH_EVENT_LISTEN_CUSTOM_COMMAND_U);
        p_Context->AddCallback(p_CBCustomCommand, MRH_EVENT_SAY_CUSTOM_COMMAND_U);
        
        p_Context->AddCallback(p_CBSayString, MRH_EVENT_SAY_STRING_U);
        p_Context->AddCallback(p_CBSpeechMethod, MRH_EVENT_LISTEN_GET_METHOD_U);
        p_Context->AddCallback(p_CBSpeechMethod, MRH_EVENT_SAY_GET_METHOD_U);
        p_Context->AddCallback(p_CBNotification, MRH_EVENT_SAY_NOTIFICATION_APP_U);
        p_Context->AddCallback(p_CBNotification, MRH_EVENT_SAY_NOTIFICATION_SERVICE_U);
    }
    catch (Exception& e)
    {
        return Exit(p_Context, e.what(), EXIT_FAILURE);
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
