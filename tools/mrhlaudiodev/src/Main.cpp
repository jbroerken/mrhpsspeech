/**
 *  Main.cpp
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
#include <stdio.h>
#include <stdlib.h>

// External
#include <portaudio.h>

// Project
#include "./Revision.h"


//*************************************************************************************
// Device
//*************************************************************************************

static int ListDeviceInfo(int i_DeviceID) noexcept
{
    const PaDeviceInfo* p_DeviceInfo = Pa_GetDeviceInfo(i_DeviceID);
    
    if (p_DeviceInfo == NULL)
    {
        return -1;
    }
    
    printf("Device Name: %s\n", p_DeviceInfo->name);
    printf("Device ID: %d\n", i_DeviceID);
    printf("Input Channels (Max): %d\n", p_DeviceInfo->maxInputChannels);
    printf("Output Channels (Max): %d\n", p_DeviceInfo->maxOutputChannels);
    printf("Default Sample Rate: %.2f\n", p_DeviceInfo->defaultSampleRate);
    
    return 0;
}

//*************************************************************************************
// Main
//*************************************************************************************

int main(int argc, char* argv[])
{
    // Greet
    printf("===\n");
    printf("=  mrhlaudiodev (Verion %s)\n", VERSION_NUMBER);
    printf("===\n\n");
    
    // Setup PortAudio
    PaError i_Error;
    if ((i_Error = Pa_Initialize()) != paNoError)
    {
        printf("Failed to setup PortAudio!\n");
        return EXIT_FAILURE;
    }
    
    // List devices
    int i_DeviceCount = Pa_GetDeviceCount();
    if(i_DeviceCount < 0)
    {
        printf("Failed to get device count! PortAudio error %s\n", Pa_GetErrorText(i_DeviceCount));
    }
    
    printf("Available Audio Devices:\n");
    printf("------------------------\n\n");
    
    for (int i = 0; i < i_DeviceCount; ++i)
    {
        if (ListDeviceInfo(i) < 0)
        {
            printf("Failed to list audio device %d!\n", i);
        }
        
        printf("\n");
    }
    
    // Done, exit
    Pa_Terminate();
    return EXIT_SUCCESS;
}
