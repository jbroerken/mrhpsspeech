/**
 *  PocketSphinx.h
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

#ifndef PocketSphinx_h
#define PocketSphinx_h

// C / C++

// External
#include <MRH_Typedefs.h>
#include <pocketsphinx/pocketsphinx.h>

// Project
#include "../../../Exception.h"


class PocketSphinx
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param s_ModelDir The full path to the model directory to use.
     */
    
    PocketSphinx(std::string const& s_ModelDir);
    
    /**
     *  Default destructor.
     */
    
    virtual ~PocketSphinx() noexcept;
    
    //*************************************************************************************
    // Recognize
    //*************************************************************************************
    
    /**
     *  Start speech recognition.
     */
    
    void StartRecognition() noexcept;
    
    /**
     *  Add a audio sample to the decoder. The sample bytes will be interpreted as
     *  Signed PCM 16 with one Channel (Mono) at 16000 KHz.
     *
     *  \param p_Buffer The data buffer.
     *  \param u32_Length The length of the data buffer.
     */
    
    void AddSample(const MRH_Uint8* p_Buffer, MRH_Uint32 u32_Length);
    
    /**
     *  Convert given audio samples to a string.
     *
     *  \return The recognized string.
     */
    
    std::string Recognize() noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    ps_decoder_t* p_Decoder;
    cmd_ln_t* p_Config;
    
protected:
    
};

#endif /* PocketSphinx_h */
