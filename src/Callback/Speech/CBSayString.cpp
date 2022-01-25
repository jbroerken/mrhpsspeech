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
#include <libmrhpsb/MRH_PSBLogger.h>

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

void CBSayString::Callback(const MRH_Event* p_Event, MRH_Uint32 u32_GroupID) noexcept
{
    MRH_EvD_S_String_U c_Data;
    
    if (MRH_EVD_ReadEvent(&c_Data, p_Event->u32_Type, p_Event) < 0)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to read request event!",
                                       "CBSayString.cpp", __LINE__);
        return;
    }
    
    // No return, sent on output performed!
    p_Speech->GetOutputStorage().AddString(c_Data, u32_GroupID);
}
