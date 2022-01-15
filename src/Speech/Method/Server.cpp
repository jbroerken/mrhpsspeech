/**
 *  Server.cpp
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

// External
#include <libmrhpsb/MRH_PSBLogger.h>
#include <sodium.h>

// Project
#include "./Server.h"
#include "../../Configuration.h"

// Pre-defined
#ifndef SERVER_RECONNECT_WAIT_TIME_MS
    #define SERVER_RECONNECT_WAIT_TIME_MS 300 * 1000
#endif
#ifndef SERVER_NEXT_UPDATE_WAIT_TIME_MS
    #define SERVER_NEXT_UPDATE_WAIT_TIME_MS 250
#endif

namespace
{
    enum ConnectionState
    {
        CONNECTION_CONNECT = 0,
        CONNECTION_REQUEST_AUTH = 1,
        CONNECTION_PERFORM_AUTH = 2,
        
        CHANNEL_REQUEST = 3,
        CHANNEL_RESPONSE = 4,
        
        COMMUNICATION_CONNECT = 5,
        COMMUNICATION_REQUEST_AUTH = 6,
        COMMUNICATION_PERFORM_AUTH = 7,
        
        SERVICE_WAIT_FOR_APP = 8,
        SERVICE_APP_CONNECTED = 9,
        
        CONNECTION_STATE_MAX = SERVICE_APP_CONNECTED,
        
        CONNECTION_STATE_COUNT = CONNECTION_STATE_MAX + 1
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Server::Server() : b_Run(true),
                   b_AppPaired(false)
{
    // Setup data
    Configuration& c_Config = Configuration::Singleton();
    
    memset(p_AccountMail, '\0', MRH_SRV_SIZE_ACCOUNT_MAIL);
    strncpy(p_AccountMail,
            c_Config.GetServerAccountMail().c_str(),
            MRH_SRV_SIZE_ACCOUNT_MAIL);
    
    memset(p_AccountPassword, '\0', MRH_SRV_SIZE_ACCOUNT_PASSWORD);
    strncpy(p_AccountPassword,
            c_Config.GetServerAccountPassword().c_str(),
            MRH_SRV_SIZE_ACCOUNT_PASSWORD);
    
    memset(p_DeviceKey, '\0', MRH_SRV_SIZE_DEVICE_KEY);
    strncpy(p_DeviceKey,
            c_Config.GetServerDeviceKey().c_str(),
            MRH_SRV_SIZE_DEVICE_KEY);
    
    memset(p_DevicePassword, '\0', MRH_SRV_SIZE_DEVICE_PASSWORD);
    strncpy(p_DevicePassword,
            c_Config.GetServerDevicePassword().c_str(),
            MRH_SRV_SIZE_DEVICE_PASSWORD);
    
    memset(p_ConServerAddress, '\0', MRH_SRV_SIZE_SERVER_ADDRESS);
    strncpy(p_ConServerAddress,
            c_Config.GetServerConnectionAddress().c_str(),
            MRH_SRV_SIZE_SERVER_ADDRESS);
    i_ConServerPort = c_Config.GetServerConnectionPort();
    
    memset(p_ComServerChannel, '\0', MRH_SRV_SIZE_SERVER_CHANNEL);
    strncpy(p_ComServerChannel,
            c_Config.GetServerCommunicationChannel().c_str(),
            MRH_SRV_SIZE_SERVER_CHANNEL);
    
    memset(p_ComServerAddress, '\0', MRH_SRV_SIZE_SERVER_CHANNEL);
    i_ComServerPort = -1;
    
    
    // Data setup, now run thread
    try
    {
        c_Thread = std::thread(Update, this);
        
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Server speech now available.",
                                       "Server.cpp", __LINE__);
    }
    catch (std::exception& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Failed to start server thread: " +
                                                            std::string(e.what()),
                                       "Server.cpp", __LINE__);
    }
}

Server::~Server() noexcept
{
    b_Run = false;
    c_Thread.join();
}

//*************************************************************************************
// Switch
//*************************************************************************************

void Server::Start()
{
    std::lock_guard<std::mutex> c_RecieveGuard(c_RecieveMutex);
    l_Recieve.clear();
    
    std::lock_guard<std::mutex> c_SendGuard(c_SendMutex);
    l_Send.clear();
}

void Server::Stop()
{}

//*************************************************************************************
// Update
//*************************************************************************************

void Server::Update(Server* p_Instance) noexcept
{
    // Define used variables
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    Configuration& c_Config = Configuration::Singleton();
    
    thread_local int i_State = CONNECTION_CONNECT; // Initially we connect to the connection server
    int i_NextState = -1;
    
    MRH_Srv_Context* p_Context;
    MRH_Srv_Server* p_Server;
    
    // Setup lib objects for communication
    if ((p_Context = MRH_SRV_Init(MRH_SRV_CLIENT_PLATFORM, 1, c_Config.GetServerTimeoutS() * 1000)) == NULL)
    {
        c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetServerErrorString(),
                     "Server.cpp", __LINE__);
        return;
    }
    else if ((p_Server = MRH_SRV_CreateServer(p_Context, "speech")) == NULL) // @NOTE: Channel doesn't matter, we replace this server
    {
        c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetServerErrorString(),
                     "Server.cpp", __LINE__);
        MRH_SRV_Destroy(p_Context);
        return;
    }
    
    // Run until service terminates
    while (p_Instance->b_Run == true)
    {
        // Are we connected to our current server?
        if (MRH_SRV_IsConnected(p_Server) < 0)
        {
            c_Logger.Log(MRH_PSBLogger::INFO, "Disconnected from server, connecting...",
                         "Server.cpp", __LINE__);
            
            // Reset state, we need to reconnect
            i_State = CONNECTION_CONNECT;
            
            // Set app disconnected
            if (p_Instance->b_AppPaired == true)
            {
                p_Instance->b_AppPaired = false;
            }
        }
        
        // Handle depending on connection state
        switch (i_State)
        {
            /**
             *  Connection Server
             */
                
            case CONNECTION_CONNECT:
            {
                c_Logger.Log(MRH_PSBLogger::INFO, "Connecting to connection server...",
                             "Server.cpp", __LINE__);
                
                // Now connect to connection server
                if (MRH_SRV_Connect(p_Context, p_Server, p_Instance->p_ConServerAddress, p_Instance->i_ConServerPort, 60) < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to connect to connection server!",
                                 "Server.cpp", __LINE__);
                    
                    // Wait before retry
                    std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_RECONNECT_WAIT_TIME_MS));
                }
                else
                {
                    c_Logger.Log(MRH_PSBLogger::INFO, "Connected to connection server!",
                                 "Server.cpp", __LINE__);
                    
                    i_State = CONNECTION_REQUEST_AUTH;
                }
                
                break;
            }
            case CONNECTION_REQUEST_AUTH:
            {
                if (p_Instance->SendMessage(p_Server, i_State) == true)
                {
                    c_Logger.Log(MRH_PSBLogger::INFO, "Sent auth request to connection server!",
                              "Server.cpp", __LINE__);
                    
                    // Auth request sent, now perform auth
                    i_State = CONNECTION_PERFORM_AUTH;
                }
                
                break;
            }
            case CONNECTION_PERFORM_AUTH:
            {
                // Recieve server messages
                while ((i_NextState = p_Instance->RecieveMessage(p_Server, i_State)) != -1)
                {
                    if (i_NextState == CHANNEL_REQUEST)
                    {
                        c_Logger.Log(MRH_PSBLogger::INFO, "Authenticated with connection server!",
                                     "Server.cpp", __LINE__);
                        
                        i_State = CHANNEL_REQUEST;
                        break;
                    }
                }
                
                break;
            }
                
            /**
             *  Channel
             */
                
            case CHANNEL_REQUEST:
            {
                if (p_Instance->SendMessage(p_Server, i_State) == true)
                {
                    c_Logger.Log(MRH_PSBLogger::INFO, "Sent communication server channel request!",
                                 "Server.cpp", __LINE__);
                    
                    // Channel request sent, wait for response
                    i_State = CHANNEL_RESPONSE;
                }
                break;
            }
            case CHANNEL_RESPONSE:
            {
                // Recieve server messages
                while ((i_NextState = p_Instance->RecieveMessage(p_Server, i_State)) != -1)
                {
                    if (i_NextState == COMMUNICATION_CONNECT)
                    {
                        c_Logger.Log(MRH_PSBLogger::INFO, "Recieved communication server channel response!",
                                     "Server.cpp", __LINE__);
                        
                        i_State = COMMUNICATION_CONNECT;
                        break;
                    }
                }
                
                break;
            }
                
            /**
             *  Communication Server
             */
                
            case COMMUNICATION_CONNECT:
            {
                c_Logger.Log(MRH_PSBLogger::INFO, "Connecting to communication server...",
                             "Server.cpp", __LINE__);
                
                // Disconnect from connection server
                MRH_SRV_Disconnect(p_Server, 60);
                
                // Connect to communication server
                if (MRH_SRV_Connect(p_Context, p_Server, p_Instance->p_ComServerAddress, p_Instance->i_ComServerPort, 60) < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to connect to communication server!",
                                 "Server.cpp", __LINE__);
                    
                    // Not connectable, wait and return to connection server
                    std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_RECONNECT_WAIT_TIME_MS));
                    i_State = CONNECTION_CONNECT;
                }
                else
                {
                    c_Logger.Log(MRH_PSBLogger::INFO, "Connected to communication server!",
                                 "Server.cpp", __LINE__);
                    
                    i_State = COMMUNICATION_REQUEST_AUTH;
                }
                
                break;
            }
            case COMMUNICATION_REQUEST_AUTH:
            {
                if (p_Instance->SendMessage(p_Server, i_State) == true)
                {
                    c_Logger.Log(MRH_PSBLogger::INFO, "Sent auth request to communication server!",
                              "Server.cpp", __LINE__);
                    
                    // Auth request sent, now perform auth
                    i_State = COMMUNICATION_PERFORM_AUTH;
                }
                
                break;
            }
            case COMMUNICATION_PERFORM_AUTH:
            {
                // Recieve server messages
                while ((i_NextState = p_Instance->RecieveMessage(p_Server, i_State)) != -1)
                {
                    if (i_NextState == SERVICE_WAIT_FOR_APP)
                    {
                        c_Logger.Log(MRH_PSBLogger::INFO, "Authenticated with communication server!",
                                     "Server.cpp", __LINE__);
                        
                        i_State = SERVICE_WAIT_FOR_APP;
                        break;
                    }
                }
                
                break;
            }
                
            /**
             *  Client Communication
             */
               
            case SERVICE_WAIT_FOR_APP:
            {
                // Recieve server messages
                while ((i_NextState = p_Instance->RecieveMessage(p_Server, i_State)) != -1)
                {
                    if (i_NextState == SERVICE_APP_CONNECTED)
                    {
                        c_Logger.Log(MRH_PSBLogger::INFO, "App client paired!",
                                  "Server.cpp", __LINE__);
                        
                        // Set app connected
                        if (p_Instance->b_AppPaired == false)
                        {
                            p_Instance->b_AppPaired = true;
                        }
                        
                        i_State = SERVICE_APP_CONNECTED;
                        break;
                    }
                }
                
                break;
            }
            case SERVICE_APP_CONNECTED:
            {
                // Recieve first
                while ((i_NextState = p_Instance->RecieveMessage(p_Server, i_State)) != -1)
                {
                    if (i_NextState == SERVICE_WAIT_FOR_APP)
                    {
                        c_Logger.Log(MRH_PSBLogger::INFO, "App client disconnected, waiting for reconnect...",
                                  "Server.cpp", __LINE__);
                        
                        i_State = SERVICE_WAIT_FOR_APP;
                        break;
                    }
                }
            
                // Send if app client still connected
                if (i_State != SERVICE_WAIT_FOR_APP)
                {
                    while (p_Instance->SendMessage(p_Server, i_State) == true);
                }
                
                break;
            }
                
            /**
             *  Not Handled
             */
                
            default: { break; }
        }
        
        // Wait for next message
        std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_NEXT_UPDATE_WAIT_TIME_MS));
    }
}

//*************************************************************************************
// Listen
//*************************************************************************************

int Server::RecieveMessage(MRH_Srv_Server* p_Server, int i_State) noexcept
{
    // Define used data
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    
    uint8_t p_Buffer[MRH_SRV_SIZE_MESSAGE_BUFFER];
    MRH_Srv_NetMessage e_Message;
    thread_local uint32_t u32_Nonce;
    
    // First try to read a message to work with
    if ((e_Message = MRH_SRV_RecieveMessage(p_Server, p_Buffer, p_DevicePassword)) == MRH_SRV_CS_MSG_UNK)
    {
        return -1;
    }
    
    // Got a message, handle depending on type and state
    switch (e_Message)
    {
        /**
         *  Availability
         */
            
        case MRH_SRV_S_MSG_PARTNER_CLOSED:
        {
            if (i_State != SERVICE_APP_CONNECTED)
            {
                break;
            }
            
            // Reset usable, no app client
            return SERVICE_WAIT_FOR_APP;
        }
            
        /**
         *  Server Auth
         */
        
        case MRH_SRV_S_MSG_AUTH_CHALLENGE:
        {
            if (i_State != CONNECTION_PERFORM_AUTH && i_State != COMMUNICATION_PERFORM_AUTH)
            {
                break;
            }
            
            MRH_SRV_S_MSG_AUTH_CHALLENGE_DATA c_Challenge;
            MRH_SRV_C_MSG_AUTH_PROOF_DATA c_Proof;
            
            uint8_t p_PasswordHash[MRH_SRV_SIZE_ACCOUNT_PASSWORD] = { '\0' };
            
            if (MRH_SRV_SetNetMessage(&c_Challenge, p_Buffer) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to set auth challenge net message: " +
                                                   std::string(MRH_ERR_GetServerErrorString()),
                             "Server.cpp", __LINE__);
            }
            else if (MRH_SRV_CreatePasswordHash(p_PasswordHash, p_AccountPassword, (c_Challenge.p_Salt), c_Challenge.u8_HashType) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to hash account password!",
                             "Server.cpp", __LINE__);
            }
            else if (MRH_SRV_EncryptNonce((uint8_t*)(c_Proof.p_NonceHash), c_Challenge.u32_Nonce, p_PasswordHash) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to encrypt recieved challenge nonce!",
                             "Server.cpp", __LINE__);
            }
            else if (MRH_SRV_SendMessage(p_Server, MRH_SRV_C_MSG_AUTH_PROOF, &c_Proof, NULL) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to send pair challenge net message: " +
                                                   std::string(MRH_ERR_GetServerErrorString()),
                             "Server.cpp", __LINE__);
            }
            
            break;
        }
        case MRH_SRV_S_MSG_AUTH_RESULT:
        {
            if (i_State != CONNECTION_PERFORM_AUTH && i_State != COMMUNICATION_PERFORM_AUTH)
            {
                break;
            }
            
            MRH_SRV_S_MSG_AUTH_RESULT_DATA c_Result;
            
            if (MRH_SRV_SetNetMessage(&c_Result, p_Buffer) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to set auth result net message: " +
                                                   std::string(MRH_ERR_GetServerErrorString()),
                             "Server.cpp", __LINE__);
            }
            else if (c_Result.u8_Result != MRH_SRV_NET_MESSAGE_ERR_NONE)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to authenticate: Error: " +
                                                   std::to_string(c_Result.u8_Result) +
                                                   "!",
                             "Server.cpp", __LINE__);
            }
            else // Auth OK
            {
                if (i_State == CONNECTION_PERFORM_AUTH)
                {
                    return CHANNEL_REQUEST;
                }
                else
                {
                    return SERVICE_WAIT_FOR_APP;
                }
            }
            
            break;
        }
            
        /**
         *  Device Pairing
         */
            
        case MRH_SRV_C_MSG_PAIR_REQUEST:
        {
            if (i_State != SERVICE_WAIT_FOR_APP)
            {
                // Already paired?
                break;
            }
            
            MRH_SRV_C_MSG_PAIR_REQUEST_DATA c_Request;
            
            if (MRH_SRV_SetNetMessage(&c_Request, p_Buffer) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to set pair request net message: " +
                                                   std::string(MRH_ERR_GetServerErrorString()),
                             "Server.cpp", __LINE__);
                break;
            }
            
            // Set next nonce bytes
            randombytes_buf(&u32_Nonce, sizeof(u32_Nonce));
            
            // Build and send pair challenge to app client
            MRH_SRV_C_MSG_PAIR_CHALLENGE_DATA c_Challenge;
            
            c_Challenge.u32_Nonce = u32_Nonce;
            c_Challenge.u8_Actor = MRH_SRV_CLIENT_PLATFORM;
            
            if (MRH_SRV_SendMessage(p_Server, MRH_SRV_C_MSG_PAIR_CHALLENGE, &c_Challenge, NULL) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to send pair challenge net message: " +
                                                   std::string(MRH_ERR_GetServerErrorString()),
                             "Server.cpp", __LINE__);
            }
            
            break;
        }
        case MRH_SRV_C_MSG_PAIR_PROOF:
        {
            if (i_State != SERVICE_WAIT_FOR_APP)
            {
                break;
            }
            
            MRH_SRV_C_MSG_PAIR_PROOF_DATA c_Proof;
            MRH_SRV_C_MSG_PAIR_RESULT_DATA c_Result;
            uint32_t u32_ProofNonce;
            
            c_Result.u8_Result = MRH_SRV_NET_MESSAGE_ERR_DA_PAIR;
            
            // Now grab proof, decrypt and compare nonce, and check device key
            if (MRH_SRV_SetNetMessage(&c_Proof, p_Buffer) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to set pair proof net message: " +
                                                   std::string(MRH_ERR_GetServerErrorString()),
                             "Server.cpp", __LINE__);
            }
            else if (MRH_SRV_DecryptNonce(&u32_ProofNonce, c_Proof.p_NonceHash, (uint8_t*)p_DevicePassword) < 0 ||
                     u32_Nonce != u32_ProofNonce)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Pair proof has invalid nonce!",
                             "Server.cpp", __LINE__);
            }
            else if (strncmp(c_Proof.p_DeviceKey, p_DeviceKey, MRH_SRV_SIZE_DEVICE_KEY) != 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Pair proof has invalid device key!",
                             "Server.cpp", __LINE__);
            }
            else
            {
                // Worked!
                c_Result.u8_Result = MRH_SRV_NET_MESSAGE_ERR_NONE;
            }
            
            // Whatever the result, send
            if (MRH_SRV_SendMessage(p_Server, MRH_SRV_C_MSG_PAIR_RESULT, &c_Result, NULL) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to send pair result net message: " +
                                                   std::string(MRH_ERR_GetServerErrorString()),
                             "Server.cpp", __LINE__);
            }
            else if (c_Result.u8_Result == MRH_SRV_NET_MESSAGE_ERR_NONE)
            {
                // Sending success and auth info matched, set connected
                return SERVICE_APP_CONNECTED;
            }
            
            break;
        }
        
        /**
         *  Channel
         */
            
        case MRH_SRV_S_MSG_CHANNEL_RESPONSE:
        {
            // Do we actually need this?
            if (i_State != CHANNEL_RESPONSE)
            {
                break;
            }
            
            // Get channel info
            MRH_SRV_S_MSG_CHANNEL_RESPONSE_DATA c_Response;
            
            if (MRH_SRV_SetNetMessage(&c_Response, p_Buffer) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to set channel response net message: " +
                                                   std::string(MRH_ERR_GetServerErrorString()),
                             "Server.cpp", __LINE__);
            }
            else if (strncmp(p_ComServerChannel, c_Response.p_Channel, MRH_SRV_SIZE_SERVER_CHANNEL) != 0)
            {
                c_Logger.Log(MRH_PSBLogger::WARNING, "Unexpected channel response net message recieved!",
                             "Server.cpp", __LINE__);
            }
            else // All OK
            {
                memset(p_ComServerAddress, '\0', MRH_SRV_SIZE_SERVER_CHANNEL);
                strncpy(p_ComServerAddress, c_Response.p_Address, MRH_SRV_SIZE_SERVER_ADDRESS);
                i_ComServerPort = c_Response.u32_Port;
                
                // Got communication server info, next state
                return COMMUNICATION_CONNECT;
            }
            
            break;
        }
        
        /**
         *  Text
         */
            
        case MRH_SRV_C_MSG_TEXT:
        {
            // Do not accept if not authenticated
            if (i_State != SERVICE_APP_CONNECTED)
            {
                break;
            }
            
            MRH_SRV_C_MSG_TEXT_DATA c_Text;
            
            if (MRH_SRV_SetNetMessage(&c_Text, p_Buffer) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to set net text message: " +
                                                   std::string(MRH_ERR_GetServerErrorString()),
                             "Server.cpp", __LINE__);
                break;
            }
                
            std::string s_Text(c_Text.p_String,
                               c_Text.p_String + strnlen(c_Text.p_String, MRH_SRV_SIZE_MESSAGE_BUFFER - 1));
            
            std::lock_guard<std::mutex> c_Guard(c_RecieveMutex);
            l_Recieve.emplace_back(s_Text);
            
            break;
        }
            
        /**
         *  Not Handled
         */
            
        default: { break; }
    }
    
    // Default return
    return i_State;
}

void Server::Listen()
{
    while (true)
    {
        std::lock_guard<std::mutex> c_Guard(c_RecieveMutex);
        
        if (l_Recieve.size() == 0)
        {
            break;
        }
        
        try
        {
            SendInput(l_Recieve.front());
            l_Recieve.pop_front();
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to send input: " +
                                                                 std::string(e.what()),
                                           "Server.cpp", __LINE__);
        }
    }
}

//*************************************************************************************
// Say
//*************************************************************************************

bool Server::SendMessage(MRH_Srv_Server* p_Server, int i_State) noexcept
{
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    
    switch (i_State)
    {
        /**
         *  Server Auth
         */
            
        case CONNECTION_REQUEST_AUTH:
        case COMMUNICATION_REQUEST_AUTH:
        {
            MRH_SRV_C_MSG_AUTH_REQUEST_DATA c_Request;
            
            memcpy((c_Request.p_Mail), p_AccountMail, MRH_SRV_SIZE_ACCOUNT_MAIL);
            memcpy((c_Request.p_DeviceKey), p_DeviceKey, MRH_SRV_SIZE_DEVICE_KEY);
            c_Request.u8_Actor = MRH_SRV_CLIENT_PLATFORM;
            c_Request.u8_Version = MRH_SRV_NET_MESSAGE_VERSION;
            
            if (MRH_SRV_SendMessage(p_Server, MRH_SRV_C_MSG_AUTH_REQUEST, &c_Request, NULL) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to send auth request net message!",
                             "Server.cpp", __LINE__);
                return false;
            }
            
            return true;
        }
            
        /**
         *  Channel
         */
        
        case CHANNEL_REQUEST:
        {
            MRH_SRV_C_MSG_CHANNEL_REQUEST_DATA c_Request;
            
            memcpy(c_Request.p_Channel, p_ComServerChannel, MRH_SRV_SIZE_SERVER_CHANNEL);
            
            if (MRH_SRV_SendMessage(p_Server, MRH_SRV_C_MSG_CHANNEL_REQUEST, &c_Request, NULL) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to send channel request net message!",
                             "Server.cpp", __LINE__);
                return false;
            }
            
            return true;
        }
            
        /**
         *  Text
         */
            
        case SERVICE_APP_CONNECTED:
        {
            std::lock_guard<std::mutex> c_Guard(c_SendMutex);
            
            if (l_Send.size() == 0)
            {
                return false;
            }
            
            MRH_SRV_C_MSG_TEXT_DATA c_Text;
            bool b_Send = true;
            
            // We send until all is cleared or if sending failed
            // @NOTE: Failing to send might not be a broken connection, so keep going
            while (b_Send == true && l_Send.size() > 0)
            {
                std::string& s_String = l_Send.front();
                
                while (s_String.size() > 0)
                {
                    if (s_String.size() > (MRH_SRV_SIZE_MESSAGE_BUFFER - 1))
                    {
                        strncpy(c_Text.p_String, s_String.c_str(), (MRH_SRV_SIZE_MESSAGE_BUFFER - 1));
                        s_String.erase(0, (MRH_SRV_SIZE_MESSAGE_BUFFER - 1));
                    }
                    else
                    {
                        memset(c_Text.p_String, '\0', (MRH_SRV_SIZE_MESSAGE_BUFFER - 1));
                        strncpy(c_Text.p_String, s_String.c_str(), s_String.size());
                        s_String.clear();
                    }
                    
                    if (MRH_SRV_SendMessage(p_Server, MRH_SRV_C_MSG_TEXT, &c_Text, p_DevicePassword) < 0)
                    {
                        c_Logger.Log(MRH_PSBLogger::WARNING, "Failed to send net message: " +
                                                             std::string(MRH_ERR_GetServerErrorString()),
                                     "Server.cpp", __LINE__);
                        
                        // End early, no streams maybe?
                        b_Send = false;
                    }
                }
                
                l_Send.pop_front();
            }
        }
            
        /**
         *  Not Handled
         */
            
        default: { return false; }
    }
}

void Server::Say(OutputStorage& c_OutputStorage)
{
    while (c_OutputStorage.GetFinishedAvailable() == true)
    {
        auto String = c_OutputStorage.GetFinishedString();
        
        std::lock_guard<std::mutex> c_Guard(c_SendMutex);
        l_Send.emplace_back(String.s_String);
        
        try
        {
            OutputPerformed(String.u32_StringID, String.u32_GroupID);
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to signal output performed: " +
                                                                 std::string(e.what()),
                                           "Server.cpp", __LINE__);
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool Server::IsUsable() noexcept
{
    return b_AppPaired;
}
