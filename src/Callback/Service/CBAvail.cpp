/**
 *  CBAvail.cpp
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
#include "./CBAvail.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

CBAvail::CBAvail(std::shared_ptr<Speech>& p_Speech) noexcept : p_Speech(p_Speech)
{}

CBAvail::~CBAvail() noexcept
{}

//*************************************************************************************
// Callback
//*************************************************************************************

void CBAvail::Callback(const MRH_Event* p_Event, MRH_Uint32 u32_GroupID) noexcept
{
    MRH_Uint32 u32_ResponseType;
    
    switch (p_Event->u32_Type)
    {
        case MRH_EVENT_LISTEN_AVAIL_U:
            u32_ResponseType = MRH_EVENT_LISTEN_AVAIL_S;
            break;
        case MRH_EVENT_SAY_AVAIL_U:
            u32_ResponseType = MRH_EVENT_SAY_AVAIL_S;
            break;
            
        default:
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Unknown request event type!",
                                           "CBAvail.cpp", __LINE__);
            return;
    }
    
    MRH_EvD_Base_ServiceAvail_S_t c_Data;
    c_Data.u8_Available = (p_Speech->GetUsable() ? MRH_EVD_BASE_RESULT_SUCCESS : MRH_EVD_BASE_RESULT_FAILED);
    c_Data.u32_SupplierID = 0x4d524800;
    c_Data.u32_BinaryID = 0x54414c4b;
    c_Data.u32_Version = 1;
    
    MRH_Event* p_Result = MRH_EVD_CreateSetEvent(u32_ResponseType, &c_Data);
    
    if (p_Result == NULL)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to create response event!",
                                       "CBAvail.cpp", __LINE__);
        return;
    }
    
    p_Result->u32_GroupID = u32_GroupID;
    
    try
    {
        MRH_EventStorage::Singleton().Add(p_Result);
    }
    catch (MRH_PSBException& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, e.what(),
                                       "CBAvail.cpp", __LINE__);
        MRH_EVD_DestroyEvent(p_Result);
    }
}
