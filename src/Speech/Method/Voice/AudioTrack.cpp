/**
 *  AudioTrack.cpp
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
#include <cmath>
#include <cstring>

// External

// Project
#include "./AudioTrack.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

AudioTrack::AudioTrack(MRH_Uint32 u32_KHz,
                       size_t us_ChunkElements,
                       MRH_Uint8 u8_StorageSizeS,
                       bool b_CanGrow) : u32_KHz(u32_KHz),
                                         us_ChunkElements(us_ChunkElements),
                                          b_CanGrow(b_CanGrow)
{
    // Create all chunks needed
    size_t us_TotalSize = u8_StorageSizeS * u32_KHz;
    size_t us_ChunkCount = (us_TotalSize / us_ChunkElements) + (us_TotalSize % us_ChunkElements > 0 ? 1 : 0);
    
    try
    {
        for (size_t i = 0; i < us_ChunkCount; ++i)
        {
            l_Chunk.emplace_back(us_ChunkElements);
        }
    }
    catch (...)
    {
        throw;
    }
    
    FreeChunk = l_Chunk.begin();
}

AudioTrack::~AudioTrack() noexcept
{}

AudioTrack::Chunk::Chunk(size_t us_Elements) : us_ElementsTotal(us_Elements),
                                               us_ElementsCurrent(0)
{
    if ((p_Buffer = new MRH_Sint16[us_Elements]) == NULL)
    {
        throw Exception("Failed to allocate chunk buffer!");
    }
    
    std::memset(p_Buffer, 0, us_Elements * sizeof(MRH_Sint16));
}

AudioTrack::Chunk::~Chunk() noexcept
{
    if (p_Buffer != NULL)
    {
        delete[] p_Buffer;
    }
}

//*************************************************************************************
// Update
//*************************************************************************************

void AudioTrack::Clear() noexcept
{
    for (auto& Chunk : l_Chunk)
    {
        // Only clear written
        std::memset(Chunk.p_Buffer, 0, Chunk.us_ElementsCurrent * sizeof(MRH_Sint16));
        Chunk.us_ElementsCurrent = 0;
    }
    
    // Start is free again
    FreeChunk = l_Chunk.begin();
}

void AudioTrack::AddAudio(const MRH_Sint16* p_Buffer, size_t us_Elements)
{
    size_t us_WriteSize;
    size_t us_ExistingElements;
    
    for (size_t us_Written = 0; us_Written < us_Elements;)
    {
        // We are full, add a new chunk
        // Needs to be checked in the loop, we don't know how many
        // free chunks are following
        if (FreeChunk == l_Chunk.end())
        {
            if (b_CanGrow == false)
            {
                throw Exception("Audio track chunk limit reached!");
            }
            
            l_Chunk.emplace_back(us_ChunkElements);
        }
        
        // Write chunk
        us_ExistingElements = FreeChunk->us_ElementsCurrent;
        us_WriteSize = us_ChunkElements - us_ExistingElements;
        
        if (us_WriteSize > (us_Elements - us_Written))
        {
            us_WriteSize = (us_Elements - us_Written);
        }
        
        std::memcpy(&(FreeChunk->p_Buffer[us_ExistingElements]),
                    &(p_Buffer[us_Written]),
                    us_WriteSize);
        
        us_Written += us_WriteSize;
        
        // Select next chunk if needed
        if ((FreeChunk->us_ElementsCurrent += us_Written) == us_ChunkElements)
        {
            ++FreeChunk;
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

MRH_Sint16* AudioTrack::Chunk::GetBuffer() noexcept
{
    return p_Buffer;
}

const MRH_Sint16* AudioTrack::Chunk::GetBufferConst() const noexcept
{
    return p_Buffer;
}

size_t AudioTrack::Chunk::GetElementsCurrent() const noexcept
{
    return us_ElementsCurrent;
}

std::list<AudioTrack::Chunk> const& AudioTrack::GetChunksConst() const noexcept
{
    return l_Chunk;
}

bool AudioTrack::GetAudioExists() const noexcept
{
    if (FreeChunk != l_Chunk.begin() || FreeChunk->us_ElementsCurrent > 0)
    {
        return true;
    }
    
    return false;
}
