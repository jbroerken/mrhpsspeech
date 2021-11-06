/**
 *  MessagePacket.h
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

#ifndef MessagePacket_h
#define MessagePacket_h

// C / C++

// External

// Project

// Pre-defined
#define MESSAGE_PACKET_SIZE 256

#define MESSAGE_PACKET_STREAM_ID_POS 0
#define MESSAGE_PACKET_STREAM_ID_SIZE sizeof(MRH_Uint8)

#define MESSAGE_PACKET_TYPE_POS MESSAGE_PACKET_STREAM_ID_POS + MESSAGE_PACKET_STREAM_ID_SIZE
#define MESSAGE_PACKET_TYPE_SIZE sizeof(MRH_Uint8)

#define MESSAGE_PACKET_PAYLOAD_SIZE_POS MESSAGE_PACKET_TYPE_POS + MESSAGE_PACKET_TYPE_SIZE
#define MESSAGE_PACKET_PAYLOAD_SIZE_SIZE sizeof(MRH_Uint16) // 2 + 1 + 1 = 4, 252 / 4 is without rem

#define MESSAGE_PACKET_PAYLOAD_POS MESSAGE_PACKET_PAYLOAD_SIZE_POS + MESSAGE_PACKET_PAYLOAD_SIZE_SIZE


enum MessagePacketType
{
    START = 0,                                              // New data start
    CONT = 1,                                               // Continue of data
    END = 2,                                                // Last data packet
    SINGLE = 3,                                             // Single packet (Start + End)
    
    MESSAGE_PACKET_TYPE_MAX = SINGLE,
    
    MESSAGE_PACKET_TYPE_COUNT = MESSAGE_PACKET_TYPE_MAX + 1
};


#endif /* MessagePacket_h */
