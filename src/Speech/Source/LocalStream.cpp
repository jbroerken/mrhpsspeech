/**
 *  LocalStream.cpp
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
#include "./LocalStream.h"
#include "./APIProvider/APIProvider.h"
#include "./APIProvider/GoogleCloudAPI.h"
#include "../SpeechEvent.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

LocalStream::LocalStream(Configuration const& c_Configuration) : c_Input(c_Configuration.GetVoiceRecordingKHz()),
                                                                 c_Output(c_Configuration.GetVoicePlaybackKHz())
{}

LocalStream::~LocalStream() noexcept
{}

//*************************************************************************************
// Reset
//*************************************************************************************

void LocalStream::Reset() noexcept
{

}

//*************************************************************************************
// Exchange
//*************************************************************************************

MRH_Uint32 LocalStream::Retrieve(MRH_Uint32 u32_StringID)
{
    return 0;
}

void LocalStream::Send(OutputStorage& c_OutputStorage)
{

}
