/**
 *  GoogleAPI.h
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

#ifndef GoogleAPI_h
#define GoogleAPI_h

// C / C++
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <list>

// External

// Project
#include "./VoiceAudio.h"


class GoogleAPI
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    GoogleAPI();
    
    /**
     *  Default destructor.
     */
    
    ~GoogleAPI() noexcept;
    
    //*************************************************************************************
    // Speech to Text
    //*************************************************************************************
    
    /**
     *  Clear speech to text audio buffer.
     */
    
    void ClearSTTAudio() noexcept;
    
    /**
     *  Add mono audio data for speech to text.
     *
     *  \param v_Buffer The audio buffer to add.
     *  \param u32_KHz The audio buffer KHz.
     */
    
    void AddAudioSTT(std::vector<MRH_Sint16> const& v_Buffer, MRH_Uint32 u32_KHz) noexcept;
    
    /**
     *  Convert stored audio data for speech to text.
     */
    
    void ProcessAudioSTT() noexcept;
    
    /**
     *  Recieve all recognized strings.
     *
     *  \return All recognized string.
     */
    
    std::list<std::string> RecieveStringsSTT() noexcept;
    
    //*************************************************************************************
    // Text to Speech
    //*************************************************************************************
    
    /**
     *  Add string data for text to speech.
     *
     *  \param s_String The UTF-8 string to add.
     */
    
    void AddStringTTS(std::string const& s_String);
    
    /**
     *  Check if a audio buffer for speech if available.
     *
     *  \return true if a buffer is available, false if not.
     */
    
    bool TTSAudioAvailable() noexcept;
    
    /**
     *  Get the audio buffer containing speech.
     *
     *  \return The speech audio buffer.
     */
    
    VoiceAudio GrabTTSAudio();
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    class Audio
    {
    public:
        
        //*************************************************************************************
        // Constructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         *
         *  \param v_Buffer The audio buffer.
         *  \param u32_KHz The audio KHz.
         *  \param b_Finished If the audio is complete or not.
         */
        
        Audio(std::vector<MRH_Sint16> const& v_Buffer,
              MRH_Uint32 u32_KHz,
              bool b_Finished);
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::vector<MRH_Sint16> v_Buffer;
        MRH_Uint32 u32_KHz;
        std::atomic<bool> b_Finished;
    };
    
    //*************************************************************************************
    // Speech to Text
    //*************************************************************************************
    
    /**
     *  Update speech to text connection.
     *
     *  \param p_Instance The class instance to use.
     */
    
    static void UpdateSTT(GoogleAPI* p_Instance) noexcept;
    
    //*************************************************************************************
    // Text to Speech
    //*************************************************************************************
    
    /**
     *  Update text to speech connection.
     *
     *  \param p_Instance The class instance to use.
     */
    
    static void UpdateTTS(GoogleAPI* p_Instance) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Speech to Text
    std::thread c_STTThread;
    std::mutex c_STTMutex;
    std::atomic<bool> b_STTUpdate;
    std::list<std::string> l_STTTranscribed;
    Audio c_STTAudio;
    
    // Text to Speech
    std::thread c_TTSThread;
    std::mutex c_TTSMutex;
    std::atomic<bool> b_TTSUpdate;
    std::string s_ToAudio;
    Audio c_TTSAudio;
    
protected:
    
};

#endif /* GoogleAPI_h */
