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
                         u32_KHz(0),
                         b_TTSUpdate(true),
                         s_ToAudio(""),
                         c_TTSAudio(NULL, 0, 0, 0, 0)
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

//*************************************************************************************
// Speech to Text
//*************************************************************************************

void GoogleAPI::ClearSTTAudio() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_STTMutex);
    v_Buffer.clear();
    u32_KHz = 0;
}

void GoogleAPI::UpdateSTT(GoogleAPI* p_Instance) noexcept
{
    std::vector<MRH_Sint16> v_Buffer;
    MRH_Uint32 u32_KHz;
    
    while (p_Instance->b_STTUpdate == true)
    {
        /**
         *  Data Grab
         */
        
        // Grab allowed audio buffer
        p_Instance->c_STTMutex.lock();
        
        if (p_Instance->b_CanProcess == true)
        {
            v_Buffer = p_Instance->v_Buffer;
            u32_KHz = p_Instance->u32_KHz;
            p_Instance->b_CanProcess = false; // Reset, now grabbed
        }
        
        p_Instance->c_STTMutex.unlock();
        
        /**
         *  Data Process
         */
        
        // Run processing
        // @TODO: Run, add if finished
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void GoogleAPI::AddAudioSTT(const MRH_Sint16* p_Buffer, size_t us_Elements, MRH_Uint32 u32_KHz) noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_STTMutex);
    
    // Reset for changes
    if (this->u32_KHz != 0 && this->u32_KHz != u32_KHz)
    {
        v_Buffer.clear();
        this->u32_KHz = u32_KHz;
    }
    
    v_Buffer.insert(v_Buffer.end(),
                    p_Buffer,
                    p_Buffer + (sizeof(MRH_Sint16) * us_Elements));
}

void GoogleAPI::ProcessAudioSTT() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_STTMutex);
    b_CanProcess = true;
}

std::list<std::string> GoogleAPI::RecieveStringsSTT() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_STTMutex);
    std::list<std::string> l_Result = l_Transcribed;
    l_Transcribed.clear();
    return l_Result;
}

//*************************************************************************************
// Text to Speech
//*************************************************************************************

void GoogleAPI::UpdateTTS(GoogleAPI* p_Instance) noexcept
{
    std::string s_String;
    
    while (p_Instance->b_TTSUpdate == true)
    {
        /**
         *  Data Grab
         */
        
        // Grab string to process
        p_Instance->c_TTSMutex.lock();
        
        if (p_Instance->s_ToAudio.size() > 0)
        {
            s_String = p_Instance->s_ToAudio;
        }
        
        p_Instance->c_STTMutex.unlock();
        
        /**
         *  Data Process
         */
        
        // Run processing
        // @TODO: Run, add each chunk, set audio info, reset string if finished
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void GoogleAPI::AddStringTTS(std::string const& s_String)
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
    std::lock_guard<std::mutex> c_Guard(c_TTSMutex);
    return s_ToAudio.size() == 0 ? true : false;
}

VoiceAudio GoogleAPI::GrabTTSAudio()
{
    return c_TTSAudio;
}
