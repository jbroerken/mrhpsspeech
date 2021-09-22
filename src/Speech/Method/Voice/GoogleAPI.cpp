/**
 *  GoogleAPI.cpp
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

// External

// Project
#include "./GoogleAPI.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

GoogleAPI::GoogleAPI() : b_STTUpdate(true),
                         b_TTSUpdate(true),
                         s_ToAudio(""),
                         c_STTAudio({}, 0, false),
                         c_TTSAudio({}, 0, false)
{
    try
    {
        c_STTThread = std::thread(UpdateSTT, this);
        c_TTSThread = std::thread(UpdateTTS, this);
    }
    catch (std::exception& e)
    {
        b_STTUpdate = false;
        c_STTThread.join();
    }
}

GoogleAPI::~GoogleAPI() noexcept
{
    b_STTUpdate = false;
    b_TTSUpdate = false;
    
    c_STTThread.join();
    c_TTSThread.join();
}

GoogleAPI::Audio::Audio(std::vector<MRH_Sint16> const& v_Buffer,
                        MRH_Uint32 u32_KHz,
                        bool b_Finished) : v_Buffer(v_Buffer),
                                           u32_KHz(u32_KHz),
                                           b_Available(b_Finished)
{}

//*************************************************************************************
// Speech to Text
//*************************************************************************************

void GoogleAPI::ClearSTTAudio() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_STTMutex);
    
    c_STTAudio.v_Buffer.clear();
    c_STTAudio.b_Available = false;
}

void GoogleAPI::UpdateSTT(GoogleAPI* p_Instance) noexcept
{
    std::mutex& c_Mutex = p_Instance->c_STTMutex;
    GoogleAPI::Audio& c_Audio = p_Instance->c_STTAudio;
    
    std::vector<MRH_Sint16> v_Buffer;
    MRH_Uint32 u32_KHz;
    
    while (p_Instance->b_STTUpdate == true)
    {
        /**
         *  Data Grab
         */
        
        // Grab allowed audio buffer
        c_Mutex.lock();
        
        if (c_Audio.b_Available == true)
        {
            v_Buffer = c_Audio.v_Buffer;
            u32_KHz = c_Audio.u32_KHz;
            
            c_Audio.v_Buffer.clear(); // Clear otherwise buffer grows endlessly
            c_Audio.b_Available = false; // Reset, now grabbed
            
            c_Mutex.unlock();
        }
        else
        {
            c_Mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        /**
         *  Data Process
         */
        
        // Run processing
        // @TODO: Run, add if finished
    }
}

void GoogleAPI::AddAudioSTT(std::vector<MRH_Sint16> const& v_Buffer, MRH_Uint32 u32_KHz) noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_STTMutex);
    
    // Reset for changes
    if (c_STTAudio.u32_KHz != u32_KHz)
    {
        c_STTAudio.v_Buffer = v_Buffer;
        c_STTAudio.u32_KHz = u32_KHz;
        c_STTAudio.b_Available = false;
    }
    else
    {
        c_STTAudio.v_Buffer.insert(c_STTAudio.v_Buffer.end(),
                                   v_Buffer.begin(),
                                   v_Buffer.end());
    }
}

void GoogleAPI::ProcessAudioSTT() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_STTMutex);
    c_STTAudio.b_Available = true;
}

std::list<std::string> GoogleAPI::RecieveStringsSTT() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_STTMutex);
    
    std::list<std::string> l_Result = l_STTTranscribed;
    l_STTTranscribed.clear();
    
    return l_Result;
}

//*************************************************************************************
// Text to Speech
//*************************************************************************************

void GoogleAPI::UpdateTTS(GoogleAPI* p_Instance) noexcept
{
    std::mutex& c_Mutex = p_Instance->c_TTSMutex;
    GoogleAPI::Audio& c_Audio = p_Instance->c_TTSAudio;
    std::string& s_ToAudio = p_Instance->s_ToAudio;
    
    std::string s_String;
    
    while (p_Instance->b_TTSUpdate == true)
    {
        /**
         *  Data Grab
         */
        
        // Grab string to process
        c_Mutex.lock();
        
        if (s_ToAudio.size() > 0)
        {
            c_Audio.v_Buffer.clear();
            c_Audio.b_Available = false;
            
            s_String = s_ToAudio;
            s_ToAudio = "";
            
            c_Mutex.unlock();
        }
        else
        {
            c_Mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        /**
         *  Data Process
         */
        
        // Run processing
        // @TODO: Run, add each chunk, set audio info, Set tts audio set
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void GoogleAPI::UpdateStringTTS(std::string const& s_String)
{
    std::lock_guard<std::mutex> c_Guard(c_TTSMutex);
    
    if (s_ToAudio.size() != 0)
    {
        throw Exception("Current TTS string not yet processed!");
    }
    
    s_ToAudio = s_String;
}

bool GoogleAPI::TTSAudioAvailable() noexcept
{
    return c_TTSAudio.b_Available;
}

VoiceAudio GoogleAPI::GrabTTSAudio()
{
    std::lock_guard<std::mutex> c_Guard(c_TTSMutex);
    
    // Reset audio available, this audio file is now old!
    c_TTSAudio.b_Available = false;
    
    return VoiceAudio(c_TTSAudio.v_Buffer.data(),
                      c_TTSAudio.v_Buffer.size(),
                      c_TTSAudio.u32_KHz);
}
