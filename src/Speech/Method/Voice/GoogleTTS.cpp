/**
 *  GoogleTTS.cpp
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
#include "./GoogleTTS.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

GoogleTTS::GoogleTTS() : b_Update(true)
{
    try
    {
        c_Thread = std::thread(Process, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start Google Cloud TTS thread!" + std::string(e.what()));
    }
}

GoogleTTS::~GoogleTTS() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Reset
//*************************************************************************************

void GoogleTTS::ResetStrings() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_StringMutex);
    l_String.clear();
}

void GoogleTTS::ResetAudio() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_AudioMutex);
    l_Audio.clear();
}

//*************************************************************************************
// String
//*************************************************************************************

void GoogleTTS::AddString(std::string const& s_String)
{
    std::lock_guard<std::mutex> c_Guard(c_StringMutex);
    l_String.emplace_back(s_String);
}

//*************************************************************************************
// Process
//*************************************************************************************

void GoogleTTS::Process(GoogleTTS* p_Instance) noexcept
{
    std::string s_String;
    
    std::mutex& c_StringMutex = p_Instance->c_StringMutex;
    std::mutex& c_AudioMutex = p_Instance->c_AudioMutex;
    std::list<std::string>& l_String = p_Instance->l_String;
    std::list<MonoAudio>& l_Audio = p_Instance->l_Audio;
    
    while (p_Instance->b_Update == true)
    {
        /**
         *  Data Grab
         */
        
        // Grab string to process
        c_StringMutex.lock();
        
        if (l_Audio.size() > 0)
        {
            s_String = l_String.front();
            l_String.pop_front();
            c_StringMutex.unlock();
        }
        else
        {
            c_StringMutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        /**
         *  Data Process
         */
        
        MonoAudio c_Audio({}, 0, 0);
        
        /**
         *  Add Audio
         */
        
        c_AudioMutex.lock();
        l_Audio.emplace_back(std::move(c_Audio));
        c_AudioMutex.unlock();
        
        // Run processing
        // @TODO: Run, add each chunk, set audio info, Set tts audio set
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

//*************************************************************************************
// Audio
//*************************************************************************************

bool GoogleTTS::GetAudioAvailable() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_AudioMutex);
    return l_Audio.size() > 0 ? true : false;
}

MonoAudio GoogleTTS::GetAudio()
{
    c_AudioMutex.lock();
    
    MonoAudio c_Result = std::move(l_Audio.front());
    l_Audio.pop_front();
    
    c_AudioMutex.unlock();
    
    return c_Result;
}
