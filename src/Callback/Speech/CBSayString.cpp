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
    // Is any speech method available?
    if (p_Speech->GetUsable() == false)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Speech not usable!",
                                       "CBSayString.cpp", __LINE__);
        return;
    }
    
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
