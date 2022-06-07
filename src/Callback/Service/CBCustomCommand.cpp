/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// C / C++

// External
#include <libmrhpsb/MRH_PSBLogger.h>

// Project
#include "./CBCustomCommand.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

CBCustomCommand::CBCustomCommand() noexcept
{}

CBCustomCommand::~CBCustomCommand() noexcept
{}

//*************************************************************************************
// Callback
//*************************************************************************************

void CBCustomCommand::Callback(const MRH_Event* p_Event, MRH_Uint32 u32_GroupID) noexcept
{
    MRH_EvD_Sys_NotImplemented_S c_Data;
    c_Data.u32_Type = p_Event->u32_Type;
    
    MRH_Event* p_Result = MRH_EVD_CreateSetEvent(MRH_EVENT_NOT_IMPLEMENTED_S, &c_Data);
    
    if (p_Result == NULL)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to create response event!",
                                       "CBCustomCommand.cpp", __LINE__);
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
                                       "CBCustomCommand.cpp", __LINE__);
        MRH_EVD_DestroyEvent(p_Result);
    }
}
