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
    // Reset
    //*************************************************************************************
    
    /**
     *  Reset the decoder.
     */
    
    void ResetDecoder() noexcept;
    
    //*************************************************************************************
    // Recognize
    //*************************************************************************************
    
    /**
     *  Add audio data to the decoder.
     *
     *  \param p_Buffer The audio buffer to add.
     *  \param us_Length The length of the audio buffer.
     */
    
    void AddAudio(const MRH_Sint16* p_Buffer, size_t us_Length) noexcept;
    
    /**
     *  Check if the last added audio buffer contains speech.
     *
     *  \return true if contained, false if not.
     */
    
    bool AudioContainsSpeech() noexcept;
    
    /**
     *  Convert a given audio sample to a string.
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
    
    bool b_DecoderRunning;
    
protected:
    
};

#endif /* PocketSphinx_h */
