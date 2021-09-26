/**
 *  GoogleSTT.cpp
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
#include "./GoogleSTT.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

GoogleSTT::GoogleSTT() : b_Update(true)
{
    try
    {
        // Set initial audio to write
        l_Audio.emplace_back(std::make_pair(0, std::vector<MRH_Sint16>()));
        
        c_Thread = std::thread(Transcribe, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start Google Cloud STT thread!" + std::string(e.what()));
    }
}

GoogleSTT::~GoogleSTT() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Reset
//*************************************************************************************

void GoogleSTT::ResetAudio() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_AudioMutex);
    l_Audio.clear();
}

void GoogleSTT::ResetStrings() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_TranscribeMutex);
    l_Transcribed.clear();
}

//*************************************************************************************
// Audio
//*************************************************************************************

void GoogleSTT::AddAudio(MonoAudio const& c_Audio) noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_AudioMutex);
    
    // Reset for changes
    if (l_Audio.back().first != c_Audio.u32_KHz)
    {
        l_Audio.back().first = c_Audio.u32_KHz;
        l_Audio.back().second = c_Audio.v_Buffer;
    }
    else
    {
        l_Audio.back().second.insert(l_Audio.back().second.end(),
                                     c_Audio.v_Buffer.begin(),
                                     c_Audio.v_Buffer.end());
    }
}

void GoogleSTT::ProcessAudio() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_AudioMutex);
    l_Audio.emplace_back(std::make_pair(0, std::vector<MRH_Sint16>()));
}

//*************************************************************************************
// Transcribe
//*************************************************************************************

void GoogleSTT::Transcribe(GoogleSTT* p_Instance) noexcept
{
    std::pair<MRH_Uint32, std::vector<MRH_Sint16>> c_Audio;
    
    std::mutex& c_AudioMutex = p_Instance->c_AudioMutex;
    std::mutex& c_TranscribeMutex = p_Instance->c_TranscribeMutex;
    std::list<std::pair<MRH_Uint32, std::vector<MRH_Sint16>>>& l_Audio = p_Instance->l_Audio;
    std::list<std::string>& l_Transcribed = p_Instance->l_Transcribed;
    
    while (p_Instance->b_Update == true)
    {
        /**
         *  Data Grab
         */
        
        // Grab allowed audio buffer
        c_AudioMutex.lock();
        
        if (l_Audio.size() > 1) // (size() - 1) is the audio being added
        {
            c_Audio = std::move(l_Audio.front());
            l_Audio.pop_front();
            c_AudioMutex.unlock();
        }
        else
        {
            c_AudioMutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        /**
         *  Data Process
         */
        
        // Run processing
        std::string s_String = "";
        
        // @TODO: Run, add if finished
        
        
        /**
         *  Add Transcribed
         */
        
        c_TranscribeMutex.lock();
        l_Transcribed.emplace_back(std::move(s_String));
        c_TranscribeMutex.unlock();
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

std::list<std::string> GoogleSTT::GetStrings() noexcept
{
    c_TranscribeMutex.lock();
    
    std::list<std::string> l_Result = std::move(l_Transcribed);
    l_Transcribed = {};
    
    c_TranscribeMutex.unlock();
    
    return l_Result;
}
