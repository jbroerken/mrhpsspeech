/**
 *  PocketSphinx.cpp
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
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./PocketSphinx.h"

// Pre-defined
#ifndef MRH_SPEECH_SPHINX_LISTEN_PATH
    #define MRH_SPEECH_SPHINX_LISTEN_PATH "/var/mrh/mrhpsspeech/sphinx/listen/"
#endif
#define MRH_SPEECH_SPHINX_LISTEN_LM_EXT ".lm.bin"
#define MRH_SPEECH_SPHINX_LISTEN_DICT_EXT "-dict.dict"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

PocketSphinx::PocketSphinx() : b_Update(true)
{
    // @TODO: Init sphinx
    
    // Run
    try
    {
        c_Thread = std::thread(Update, this);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to start Pocket Sphinx update thread: " + std::string(e.what()));
    }
    
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Pocket sphinx now available.",
                                   "PocketSphinx.cpp", __LINE__);
}

PocketSphinx::~PocketSphinx() noexcept
{
    b_Update = false;
    c_Thread.join();
}

//*************************************************************************************
// Update
//*************************************************************************************

void PocketSphinx::Update(PocketSphinx* p_Instance) noexcept
{
    while (p_Instance->b_Update == true)
    {
        
    }
}

//*************************************************************************************
// Listen
//*************************************************************************************

void PocketSphinx::Listen()
{
    
}

//*************************************************************************************
// Say
//*************************************************************************************

void PocketSphinx::Say(OutputStorage& c_OutputStorage)
{
    
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool PocketSphinx::IsUsable() noexcept
{
    return false;
}
