/**
 *  CBSpeechMethod.cpp
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
#include "./CBSpeechMethod.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

CBSpeechMethod::CBSpeechMethod(std::shared_ptr<Speech>& p_Speech) noexcept : p_Speech(p_Speech)
{}

CBSpeechMethod::~CBSpeechMethod() noexcept
{}

//*************************************************************************************
// Callback
//*************************************************************************************

void CBSpeechMethod::Callback(const MRH_EVBase* p_Event, MRH_Uint32 u32_GroupID) noexcept
{
    try
    {
        switch (p_Event->GetType())
        {
            case MRH_EVENT_LISTEN_GET_METHOD_U:
                MRH_EventStorage::Singleton().Add(MRH_L_GET_METHOD_S(p_Speech->GetMethod()),
                                                  u32_GroupID);
                break;
            case MRH_EVENT_SAY_GET_METHOD_U:
                MRH_EventStorage::Singleton().Add(MRH_S_GET_METHOD_S(p_Speech->GetMethod()),
                                                  u32_GroupID);
                break;
        }
    }
    catch (MRH_PSBException& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                       "CBSpeechMethod.cpp", __LINE__);
    }
}
