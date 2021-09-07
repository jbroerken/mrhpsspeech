/**
 *  HandleAudio.cpp
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
#include <clocale>

// External
#include <libmrhpsb/MRH_PSBLogger.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// Project
#include "./HandleAudio.h"

// Pre-defined
namespace
{
    std::string s_DefaultLocale = "en_US";
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

HandleAudio::HandleAudio() : s_Locale(s_DefaultLocale)
{
    // Grab locale first
    if (std::setlocale(LC_CTYPE, NULL) == NULL)
    {
        s_Locale = std::setlocale(LC_CTYPE, NULL);
        s_Locale = s_Locale.substr(0, s_Locale.find_first_of('.'));
    }
    
    // Setup SDL
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        throw Exception("Failed to initialize SDL! " + std::string(SDL_GetError()));
    }
    else if (Mix_Init(0) != 0)
    {
        throw Exception("Failed to initialize SDL_Mixer! " + std::string(Mix_GetError()));
    }
    
    // Setup Audio
    try
    {
        ReadAudio::Setup(s_Locale);
    }
    catch (Exception& e)
    {
        throw;
    }
}

HandleAudio::~HandleAudio() noexcept
{
    SDL_Quit();
}

//*************************************************************************************
// Getters
//*************************************************************************************

std::string HandleAudio::GetLocale() const noexcept
{
    return s_Locale;
}
