/**
 *  GoogleTTS.h
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

#ifndef GoogleTTS_h
#define GoogleTTS_h

// C / C++
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <list>

// External

// Project
#include "./MonoAudio.h"


class GoogleTTS
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    GoogleTTS();
    
    /**
     *  Default destructor.
     */
    
    ~GoogleTTS() noexcept;
    
    //*************************************************************************************
    // Reset
    //*************************************************************************************
    
    /**
     *  Clear text to speech string buffer.
     */
    
    void ResetStrings() noexcept;
    
    /**
     *  Clear text to speech audio results.
     */
    
    void ResetAudio() noexcept;
    
    //*************************************************************************************
    // String
    //*************************************************************************************
    
    /**
     *  Add string data for text to speech.
     *
     *  \param s_String The UTF-8 string to add.
     */
    
    void AddString(std::string const& s_String);
    
    //*************************************************************************************
    // Audio
    //*************************************************************************************
    
    /**
     *  Check if a audio buffer for speech if available.
     *
     *  \return true if audio is available, false if not.
     */
    
    bool GetAudioAvailable() noexcept;
    
    /**
     *  Get the oldest audio buffer containing speech.
     *
     *  \return The speech audio buffer.
     */
    
    MonoAudio GetAudio();
    
private:
    
    //*************************************************************************************
    // Process
    //*************************************************************************************
    
    /**
     *  Update text to speech processing.
     *
     *  \param p_Instance The class instance to use.
     */
    
    static void Process(GoogleTTS* p_Instance) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Thread
    std::thread c_Thread;
    std::atomic<bool> b_Update;
    
    // String
    std::mutex c_StringMutex;
    std::list<std::string> l_String;
    
    // Audio
    std::mutex c_AudioMutex;
    std::list<MonoAudio> l_Audio;
    
protected:
    
};

#endif /* GoogleTTS_h */
