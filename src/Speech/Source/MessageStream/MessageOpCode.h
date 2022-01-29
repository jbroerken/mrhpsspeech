/**
 *  MessageOpCode.h
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

#ifndef MessageOpCode_h
#define MessageOpCode_h

// C / C++
#include <vector>
#include <string>

// External
#include <MRH_Typedefs.h>

// Project

// Pre-defined
#define OPCODE_OPCODE_POS 0
#define OPCODE_OPCODE_SIZE sizeof(MRH_Uint8)

#define OPCODE_DATA_POS OPCODE_OPCODE_POS + OPCODE_OPCODE_SIZE


namespace MessageOpCode
{
    //*************************************************************************************
    // OpCodes
    //*************************************************************************************
    
    enum OpCodeList
    {
        /**
         *  UNK
         */
        
        UNK_CS_UNK = 0,                         // ???
        
        /**
         *  Hello
         */
        
        HELLO_C_HELLO = 1,                      // Signal client still in use
        
        /**
         *  String
         */
        
        STRING_CS_STRING = 2,                   // String data for input / output
        
        /**
         *  Audio
         */
        
        AUDIO_CS_AUDIO = 3,                     // Audio data recorded / to play
        
        AUDIO_C_PLAYBACK_FINISHED = 4,          // FinishMessageOpCode::STRING_CS_STRING_DATA
        
        AUDIO_S_START_RECORDING = 5,            // Start recording audio
        AUDIO_S_STOP_RECORDING = 6,             // Stop recording audio
        
        /**
         *  Bounds
         */
        
        OPCODE_MAX = AUDIO_S_STOP_RECORDING,
        
        OPCODE_COUNT = OPCODE_MAX + 1
    };
    
    //*************************************************************************************
    // OpCode Data - Base
    //*************************************************************************************
    
    class OpCodeData
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Value constructor.
         *
         *  \param u8_OpCode The opcode represented.
         */
        
        OpCodeData(MRH_Uint8 u8_OpCode) noexcept;
        
        /**
         *  Default destructor.
         */
        
        virtual ~OpCodeData() noexcept;
        
        //*************************************************************************************
        // Getters
        //*************************************************************************************
        
        /**
         *  Get the opcode identifier.
         *
         *  \return The opcode identifier.
         */
        
        MRH_Uint8 GetOpCode() noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::vector<MRH_Uint8> v_Data;
        
    private:
        
    protected:
        
        //*************************************************************************************
        // Constructor
        //*************************************************************************************
        
        /**
         *  Data constructor.
         *
         *  \param v_Data The data for the opcode.
         */
        
        OpCodeData(std::vector<MRH_Uint8>& v_Data) noexcept;
    };
    
    //*************************************************************************************
    // OpCode Data - STRING_CS_STRING
    //*************************************************************************************
    
    class STRING_CS_STRING_DATA : public OpCodeData
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Data constructor.
         *
         *  \param v_Data The data for the opcode.
         */
        
        STRING_CS_STRING_DATA(std::vector<MRH_Uint8>& v_Data) noexcept;
        
        /**
         *  Value constructor.
         *
         *  \param s_String The message string.
         */
        
        STRING_CS_STRING_DATA(std::string const& s_String) noexcept;
        
        /**
         *  Default destructor.
         */
        
        ~STRING_CS_STRING_DATA() noexcept;
        
        //*************************************************************************************
        // Getters
        //*************************************************************************************
        
        /**
         *  Get the message string.
         *
         *  \return The message string.
         */
        
        std::string GetString() noexcept;
        
    private:
        
    protected:
        
    };
    
    //*************************************************************************************
    // OpCode Data - AUDIO_CS_AUDIO
    //*************************************************************************************
    
    class AUDIO_CS_AUDIO_DATA : public OpCodeData
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Data constructor.
         *
         *  \param v_Data The data for the opcode.
         */
        
        AUDIO_CS_AUDIO_DATA(std::vector<MRH_Uint8>& v_Data) noexcept;
        
        /**
         *  Value constructor.
         *
         *  \param p_Samples The samples to store.
         *  \param u32_Count The amount of samples to store.
         */
        
        AUDIO_CS_AUDIO_DATA(const MRH_Sint16* p_Samples,
                            MRH_Uint32 u32_Count) noexcept;
        
        /**
         *  Default destructor.
         */
        
        ~AUDIO_CS_AUDIO_DATA() noexcept;
        
        //*************************************************************************************
        // Getters
        //*************************************************************************************
        
        /**
         *  Get the audio buffer.
         *
         *  \return The audio buffer.
         */
        
        const MRH_Sint16* GetAudioBuffer() noexcept;
        
        /**
         *  Get the amount of audio samples.
         *
         *  \return The amount of audio samples.
         */
        
        MRH_Uint32 GetSampleCount() noexcept;
        
    private:
        
    protected:
        
    };
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the opcode for a data buffer.
     *
     *  \param v_Data The data buffer.
     *
     *  \return The opcode.
     */
    
    MRH_Uint8 GetOpCode(std::vector<MRH_Uint8> const& v_Data) noexcept;
    
    /**
     *  Get a data value.
     *
     *  \param v_Data The message data.
     *  \param us_Pos The start byte position.
     *
     *  \return The casted value.
     */
    
    template <typename T> inline static T GetValue(std::vector<MRH_Uint8> const& v_Data, size_t us_Pos) noexcept
    {
        return *(T*)&(v_Data[us_Pos]);
    }
    
    //*************************************************************************************
    // Setters
    //*************************************************************************************
    
    /**
     *  Set a data value.
     *
     *  \param v_Data The message data.
     *  \param us_Pos The start byte position.
     *  \param Value The value to set.
     */
    
    template <typename T> inline void SetValue(std::vector<MRH_Uint8>& v_Data, size_t us_Pos, const T* Value) noexcept
    {
        std::memcpy(&(v_Data[us_Pos]),
                    Value,
                    sizeof(T));
    }
}

#endif /* MessageOpCode_h */
