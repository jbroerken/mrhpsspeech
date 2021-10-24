/**
 *  AudioTrack.h
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

#ifndef AudioTrack_h
#define AudioTrack_h

// C / C++
#include <list>

// External
#include <MRH_Typedefs.h>

// Project
#include "../../../Exception.h"


class AudioTrack
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    class Chunk
    {
        friend class AudioTrack;
        
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         *
         *  \param us_Elements The amount of elements storable in the chunk.
         */
        
        Chunk(size_t us_Elements);
        
        /**
         *  Default destructor.
         */
        
        ~Chunk() noexcept;
        
        //*************************************************************************************
        // Update
        //*************************************************************************************
        
        /**
         *  Calculate the average amplitude.
         */
        
        void CalculateAvgAmplitude() noexcept;
        
        //*************************************************************************************
        // Getters
        //*************************************************************************************
        
        /**
         *  Get the chunk buffer.
         *
         *  \return The chunk buffer.
         */
        
        MRH_Sint16* GetBuffer() noexcept;
        
        /**
         *  Get the const chunk buffer.
         *
         *  \return The const chunk buffer.
         */
        
        const MRH_Sint16* GetBufferConst() const noexcept;
        
        /**
         *  Get the amount of elements currently stored.
         *
         *  \return The currently stored element count.
         */
        
        size_t GetElementsCurrent() const noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        const size_t us_ElementsTotal;
        
    private:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        MRH_Sint16* p_Buffer;
        size_t us_ElementsCurrent;
        
    public:
        
    };
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param u32_KHz The audio track KHz for all chunks.
     *  \param us_ChunkElements The elements storable in each chunk.
     *  \param u8_StorageSizeS The amount of audio storable in seconds.
     *  \param b_CanGrow If the track can be expanded beyond the initial size.
     */
    
    AudioTrack(MRH_Uint32 u32_KHz,
               size_t us_ChunkElements,
               MRH_Uint8 u8_StorageSizeS,
               bool b_CanGrow);
    
    /**
     *  Default destructor.
     */
    
    ~AudioTrack() noexcept;
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Clear all audio chunks.
     */
    
    void Clear() noexcept;
    
    /**
     *  Add a new audio chunk.
     *
     *  \param p_Buffer The audio buffer.
     *  \param us_Elements The elements in the audio buffer.
     */
    
    void AddChunk(const MRH_Sint16* p_Buffer, size_t us_Elements);
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the const chunk list.
     *
     *  \return The const chunk list.
     */
    
    std::list<Chunk> const& GetChunksConst() const noexcept;
    
    /**
     *  Check if this audio track has any audio.
     *
     *  \return true if audio exists, false if not.
     */
    
    bool GetAudioExists() const noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    const MRH_Uint32 u32_KHz;
    const size_t us_ChunkElements;
    const bool b_CanGrow;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::list<Chunk> l_Chunk;
    std::list<Chunk>::iterator FreeChunk;
    
protected:
    
};

#endif /* AudioTrack_h */
