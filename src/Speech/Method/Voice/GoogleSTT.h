/**
 *  GoogleSTT.h
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

#ifndef GoogleSTT_h
#define GoogleSTT_h

// C / C++
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <list>

// External

// Project
#include "./MonoAudio.h"


class GoogleSTT
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    GoogleSTT();
    
    /**
     *  Default destructor.
     */
    
    ~GoogleSTT() noexcept;
    
    //*************************************************************************************
    // Reset
    //*************************************************************************************
    
    /**
     *  Clear speech to text audio buffer.
     */
    
    void ResetAudio() noexcept;
    
    /**
     *  Clear speech to text string results.
     */
    
    void ResetStrings() noexcept;
    
    //*************************************************************************************
    // Audio
    //*************************************************************************************
    
    /**
     *  Add mono audio data for speech to text.
     *
     *  \param c_Audio The audio buffer to add.
     */
    
    void AddAudio(MonoAudio const& c_Audio) noexcept;
    
    /**
     *  Convert stored audio data for speech to text.
     */
    
    void ProcessAudio() noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Recieve all recognized strings.
     *
     *  \return All recognized string.
     */
    
    std::list<std::string> GetStrings() noexcept;
    
private:
    
    //*************************************************************************************
    // Transcribe
    //*************************************************************************************
    
    /**
     *  Update speech to text processing.
     *
     *  \param p_Instance The class instance to use.
     */
    
    static void Transcribe(GoogleSTT* p_Instance) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Thread
    std::thread c_Thread;
    std::atomic<bool> b_Update;
    
    // Audio
    std::mutex c_AudioMutex;
    std::list<std::pair<MRH_Uint32, std::vector<MRH_Sint16>>> l_Audio;
    
    // String
    std::mutex c_TranscribeMutex;
    std::atomic<MRH_Uint64> u64_TranscribeValidAfterMS;
    std::list<std::string> l_Transcribed;
    
protected:
    
};

#endif /* GoogleSTT_h */
