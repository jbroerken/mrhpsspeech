/**
 *  CBReset.cpp
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
#include "./CBReset.h"
#include "../../Speech/OutputStorage.h"
#include "../../Configuration.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

CBReset::CBReset(std::shared_ptr<Speech>& p_Speech) noexcept : p_Speech(p_Speech)
{}

CBReset::~CBReset() noexcept
{}

//*************************************************************************************
// Callback
//*************************************************************************************

void CBReset::Callback(const MRH_EVBase* p_Event, MRH_Uint32 u32_GroupID) noexcept
{
    // Reset config
    try
    {
        Configuration::Singleton().Load();
    }
    catch (Exception& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::WARNING, e.what(),
                                       "CBReset.cpp", __LINE__);
    }
    
    // Simply reset output
    p_Speech->GetOutputStorage().ResetUnfinished();
    p_Speech->GetOutputStorage().ResetFinished();
}
