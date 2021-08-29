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
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

// External
#include <MRH_Typedefs.h>

// Project
#include "./Revision.h"

// Pre-defined
#ifndef MRH_SPEECH_CLI_SOCKET_PATH
    #define MRH_SPEECH_CLI_SOCKET_PATH "/tmp/mrhpsspeech_cli.socket"
#endif
#define MRH_SPEECH_CLI_READ_SIZE sizeof(MRH_Uint32)
#define MRH_SPEECH_CLI_WRITE_SIZE sizeof(MRH_Uint32)


//*************************************************************************************
// Main
//*************************************************************************************

int main(int argc, char* argv[])
{
    return 0;
}
