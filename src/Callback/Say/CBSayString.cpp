/**
 *  CBSayString.cpp
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

// Project
#include "./CBSayString.h"
#include "../../Speech/OutputStorage.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

CBSayString::CBSayString(std::shared_ptr<Speech>& p_Speech) noexcept : p_Speech(p_Speech)
{}

CBSayString::~CBSayString() noexcept
{}

//*************************************************************************************
// Callback
//*************************************************************************************

void CBSayString::Callback(const MRH_EVBase* p_Event, MRH_Uint32 u32_GroupID) noexcept
{
    // No return, sent on output performed!
    p_Speech->GetOutputStorage().AddEvent(static_cast<const MRH_S_STRING_U*>(p_Event));
}
