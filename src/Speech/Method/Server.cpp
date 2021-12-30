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


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Server::Server() : b_Run(true),
                   b_AppConnected(false)
{
    try
    {
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
// Server
//*************************************************************************************

bool Server::ConnectToServer(MRH_Srv_Context* p_Context, MRH_Srv_Server* p_Server, const char* p_Address, int i_Port) noexcept
{
    /**
     *  Variables
     */
    
    // Define auth messages
    MRH_SRV_C_MSG_AUTH_REQUEST_DATA c_AuthRequest;
    MRH_SRV_S_MSG_AUTH_CHALLENGE_DATA c_AuthChallenge;
    MRH_SRV_C_MSG_AUTH_PROOF_DATA c_AuthProof;
    MRH_SRV_S_MSG_AUTH_RESULT_DATA c_AuthResult;
    
    // Setup things we send
    memset(c_AuthRequest.p_Mail, '\0', MRH_SRV_SIZE_ACCOUNT_MAIL);
    strncpy(c_AuthRequest.p_Mail,
            Configuration::Singleton().GetServerAccountMail().c_str(),
            MRH_SRV_SIZE_ACCOUNT_MAIL);
    c_AuthRequest.u8_Actor = MRH_SRV_CLIENT_PLATFORM;
    c_AuthRequest.u8_Version = MRH_SRV_NET_MESSAGE_VERSION;

    memset(c_AuthProof.p_NonceHash, '\0', MRH_SRV_SIZE_NONCE_HASH);
    memset(c_AuthProof.p_DeviceKey, '\0', MRH_SRV_SIZE_DEVICE_KEY);
    strncpy(c_AuthProof.p_DeviceKey,
            Configuration::Singleton().GetServerDeviceKey().c_str(),
            MRH_SRV_SIZE_DEVICE_KEY);
    
    // Message buffer
    uint8_t p_Buffer[MRH_SRV_SIZE_MESSAGE_BUFFER];
    
    /**
     *  Request & Challenge
     */
    
    if (MRH_SRV_Connect(p_Context, p_Server, p_Address, i_Port) < 0)
    {
        return false;
    }
    
    // Request authentication and recieve challenge
    if (MRH_SRV_SendMessage(p_Server, MRH_SRV_C_MSG_AUTH_REQUEST, &c_AuthRequest, NULL) < 0)
    {
        return false;
    }
    
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        if (MRH_SRV_IsConnected(p_Server) < 0)
        {
            return false;
        }
        else if (MRH_SRV_RecieveMessage(p_Server, p_Buffer, NULL) != MRH_SRV_S_MSG_AUTH_CHALLENGE)
        {
            continue;
        }
        else if (MRH_SRV_SetNetMessage(&c_AuthChallenge, p_Buffer) < 0)
        {
            return false;
        }
        
        break;
    }
    
    /**
     *  Nonce
     */
    
    char p_Password[MRH_SRV_SIZE_ACCOUNT_PASSWORD] = { '\0' };
    uint8_t p_PasswordHash[MRH_SRV_SIZE_ACCOUNT_PASSWORD] = { '\0' };
    
    strncpy(p_Password,
            Configuration::Singleton().GetServerAccountPassword().c_str(),
            MRH_SRV_SIZE_ACCOUNT_PASSWORD);
    
    if (MRH_SRV_CreatePasswordHash(p_PasswordHash, p_Password, (c_AuthChallenge.p_Salt), c_AuthChallenge.u8_HashType) < 0 ||
        MRH_SRV_CreateNonceHash((c_AuthProof.p_NonceHash), c_AuthChallenge.u32_Nonce, p_Password) < 0)
    {
        return false;
    }
    
    /**
     *  Proof & Result
     */
    
    // Now handle proof and result
    if (MRH_SRV_SendMessage(p_Server, MRH_SRV_C_MSG_AUTH_PROOF, &c_AuthProof, NULL) < 0)
    {
        return false;
    }
    
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        if (MRH_SRV_IsConnected(p_Server) < 0)
        {
            return false;
        }
        else if (MRH_SRV_RecieveMessage(p_Server, p_Buffer, NULL) != MRH_SRV_S_MSG_AUTH_RESULT)
        {
            continue;
        }
        else if (MRH_SRV_SetNetMessage(&c_AuthResult, p_Buffer) < 0)
        {
            return false;
        }
        
        break;
    }
    
    // Auth result?
    return c_AuthResult.u8_Result == MRH_SRV_NET_MESSAGE_ERR_NONE ? true : false;
}

bool Server::RequestChannel(MRH_Srv_Server* p_Server, const char* p_Channel, char* p_Address, int& i_Port) noexcept
{
    /**
     *  Variables
     */
    
    // Define channel messages
    MRH_SRV_C_MSG_CHANNEL_REQUEST_DATA c_Request;
    MRH_SRV_S_MSG_CHANNEL_RESPONSE_DATA c_Response;
    
    // Setup things we send
    memcpy(c_Request.p_Channel, p_Channel, MRH_SRV_SIZE_SERVER_CHANNEL);
    
    // Message buffer
    uint8_t p_Buffer[MRH_SRV_SIZE_MESSAGE_BUFFER];
    
    /**
     *  Request & Response
     */
    
    // Exchange channel request
    if (MRH_SRV_SendMessage(p_Server, MRH_SRV_C_MSG_CHANNEL_REQUEST, &c_Request, NULL) < 0)
    {
        return false;
    }
    
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        if (MRH_SRV_IsConnected(p_Server) < 0)
        {
            return false;
        }
        else if (MRH_SRV_RecieveMessage(p_Server, p_Buffer, NULL) != MRH_SRV_S_MSG_CHANNEL_RESPONSE)
        {
            continue;
        }
        else if (MRH_SRV_SetNetMessage(&c_Response, p_Buffer) < 0)
        {
            return false;
        }
        else if (strncmp(c_Request.p_Channel, c_Response.p_Channel, MRH_SRV_SIZE_SERVER_CHANNEL) != 0)
        {
            continue;
        }
        
        break;
    }
    
    // Set data
    strncpy(p_Address, c_Response.p_Address, MRH_SRV_SIZE_SERVER_ADDRESS);
    i_Port = c_Response.u32_Port;
    
    return true;
}

void Server::Update(Server* p_Instance) noexcept
{
    /**
     *  Variables
     */
    
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    Configuration& c_Config = Configuration::Singleton();
    
    MRH_Uint32 u32_TimeoutS = c_Config.GetServerTimeoutS();
    
    uint8_t p_Buffer[MRH_SRV_SIZE_MESSAGE_BUFFER];
    MRH_Srv_NetMessage e_Message;
    uint32_t u32_Nonce;
    randombytes_buf(&u32_Nonce, sizeof(u32_Nonce));
    time_t u32_HelloTimerS = time(NULL);
    
    int i_ConPort = c_Config.GetServerConnectionPort();
    char p_ConAddress[MRH_SRV_SIZE_SERVER_ADDRESS] = { '\0' };
    strncpy(p_ConAddress,
            c_Config.GetServerConnectionAddress().c_str(),
            MRH_SRV_SIZE_SERVER_ADDRESS);
    
    char p_Channel[MRH_SRV_SIZE_SERVER_CHANNEL] = { '\0' };
    strncpy(p_Channel,
            c_Config.GetServerCommunicationChannel().c_str(),
            MRH_SRV_SIZE_SERVER_CHANNEL);
    
    char p_DeviceKey[MRH_SRV_SIZE_DEVICE_KEY] = { '\0' };
    strncpy(p_DeviceKey,
            c_Config.GetServerDeviceKey().c_str(),
            MRH_SRV_SIZE_DEVICE_KEY);
    
    char p_DevicePassword[MRH_SRV_SIZE_DEVICE_PASSWORD] = { '\0' };
    strncpy(p_DevicePassword,
            c_Config.GetServerDevicePassword().c_str(),
            MRH_SRV_SIZE_DEVICE_PASSWORD);
    
    /**
     *  Context
     */
    
    MRH_Srv_Context* p_Context = MRH_SRV_Init(MRH_SRV_CLIENT_PLATFORM, 2, u32_TimeoutS);
    
    if (p_Context == NULL)
    {
        c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetServerErrorString(),
                     "Server.cpp", __LINE__);
        return;
    }
    
    /**
     *  Servers
     */
    
    MRH_Srv_Server* p_Connection = MRH_SRV_CreateServer(p_Context, "connection");
    
    if (p_Connection == NULL)
    {
        c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetServerErrorString(),
                     "Server.cpp", __LINE__);
        MRH_SRV_Destroy(p_Context);
        return;
    }
    
    MRH_Srv_Server* p_Speech = MRH_SRV_CreateServer(p_Context, p_Channel);
    
    if (p_Speech == NULL)
    {
        c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetServerErrorString(),
                     "Server.cpp", __LINE__);
        MRH_SRV_DestroyServer(p_Context, p_Connection);
        MRH_SRV_Destroy(p_Context);
        return;
    }
    
    /**
     *  Communication Update
     */
    
    while (p_Instance->b_Run == true)
    {
        /**
         *  Connection
         */
        
        // @NOTE: Wait if no connection or to let messages be recieved by library
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Check connection
        if (MRH_SRV_IsConnected(p_Speech) < 0)
        {
            char p_ComAddress[MRH_SRV_SIZE_SERVER_ADDRESS] = { '\0' };
            int i_ComPort;
            
            if (p_Instance->ConnectToServer(p_Context, p_Connection,
                                            p_ConAddress, i_ConPort) == false ||
                p_Instance->RequestChannel(p_Connection, p_Channel,
                                           p_ComAddress, i_ComPort) == false ||
                p_Instance->ConnectToServer(p_Context, p_Speech,
                                            p_ComAddress, i_ComPort) == false)
            {
                MRH_SRV_Disconnect(p_Connection);
                std::this_thread::sleep_for(std::chrono::seconds(60));
                continue;
            }
            else
            {
                MRH_SRV_Disconnect(p_Connection);
            }
            
            u32_HelloTimerS = time(NULL) + (u32_TimeoutS * 0.75f);
        }
        
        /**
         *  Recieve
         */
        
        while ((e_Message = MRH_SRV_RecieveMessage(p_Speech, p_Buffer, p_DevicePassword)) != MRH_SRV_CS_MSG_UNK)
        {
            switch (e_Message)
            {
                // Availability
                case MRH_SRV_S_MSG_PARTNER_CLOSED:
                {
                    // Reset usable, no app client
                    p_Instance->b_AppConnected = false;
                    break;
                }
                    
                // Device Pairing
                case MRH_SRV_C_MSG_PAIR_REQUEST:
                {
                    randombytes_buf(&u32_Nonce, sizeof(u32_Nonce));
                    
                    MRH_SRV_C_MSG_PAIR_CHALLENGE_DATA c_Challenge;
                    c_Challenge.u32_Nonce = u32_Nonce;
                    c_Challenge.u8_Actor = MRH_SRV_CLIENT_PLATFORM;
                    
                    if (MRH_SRV_SendMessage(p_Speech, MRH_SRV_C_MSG_PAIR_CHALLENGE, &c_Challenge, NULL) < 0)
                    {
                        c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to send net message: " +
                                                           std::string(MRH_ERR_GetServerErrorString()),
                                     "Server.cpp", __LINE__);
                    }
                    
                    break;
                }
                case MRH_SRV_C_MSG_PAIR_PROOF:
                {
                    MRH_SRV_C_MSG_PAIR_PROOF_DATA c_Proof;
                    uint8_t u8_Result = MRH_SRV_NET_MESSAGE_ERR_NONE;
                    uint8_t p_NonceBuffer[MRH_SRV_SIZE_NONCE_HASH];
                    
                    if (MRH_SRV_CreateNonceHash(p_NonceBuffer, u32_Nonce, p_DevicePassword) < 0 ||
                        MRH_SRV_SetNetMessage(&c_Proof, p_Buffer) < 0 ||
                        strncmp(c_Proof.p_DeviceKey, p_DeviceKey, MRH_SRV_SIZE_DEVICE_KEY) != 0 ||
                        memcmp(p_NonceBuffer, c_Proof.p_NonceHash, MRH_SRV_SIZE_NONCE_HASH) != 0)
                    {
                        c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to check proof: " +
                                                           std::string(MRH_ERR_GetServerErrorString()),
                                     "Server.cpp", __LINE__);
                        u8_Result = MRH_SRV_NET_MESSAGE_ERR_DA_PAIR;
                    }
                    
                    MRH_SRV_C_MSG_PAIR_RESULT_DATA c_Result;
                    c_Result.u8_Result = u8_Result;
                    
                    if (MRH_SRV_SendMessage(p_Speech, MRH_SRV_C_MSG_PAIR_RESULT, &c_Result, NULL) < 0)
                    {
                        c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to send net message: " +
                                                           std::string(MRH_ERR_GetServerErrorString()),
                                     "Server.cpp", __LINE__);
                    }
                    else
                    {
                        // Sending success, set connected
                        p_Instance->b_AppConnected = true;
                    }
                    
                    break;
                }
                
                // Text
                case MRH_SRV_C_MSG_TEXT:
                {
                    // Do not accept if not authenticated
                    if (p_Instance->b_AppConnected == false)
                    {
                        break;
                    }
                    
                    MRH_SRV_C_MSG_TEXT_DATA c_Text;
                    
                    if (MRH_SRV_SetNetMessage(&c_Text, p_Buffer) == 0)
                    {
                        std::string s_Text(c_Text.p_String,
                                           c_Text.p_String + strnlen(c_Text.p_String, MRH_SRV_SIZE_MESSAGE_BUFFER - 1));
                        
                        std::lock_guard<std::mutex> c_Guard(p_Instance->c_RecieveMutex);
                        p_Instance->l_Recieve.emplace_back(s_Text);
                    }
                    
                    break;
                }
                    
                // Not handled
                default: { break; }
            }
        }
        
        /**
         *  Send
         */
        
        p_Instance->c_SendMutex.lock();
        
        if (p_Instance->l_Send.size() > 0)
        {
            MRH_SRV_C_MSG_TEXT_DATA c_Text;
            bool b_Send = true;
            
            // We send until all is cleared or if sending failed
            // @NOTE: Failing to send might not be a broken connection, so keep going
            while (b_Send == true && p_Instance->l_Send.size() > 0)
            {
                std::string& s_String = p_Instance->l_Send.front();
                
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
                    
                    if (MRH_SRV_SendMessage(p_Speech, MRH_SRV_C_MSG_PAIR_RESULT, &c_Text, p_DevicePassword) < 0)
                    {
                        c_Logger.Log(MRH_PSBLogger::WARNING, "Failed to send net message: " +
                                                             std::string(MRH_ERR_GetServerErrorString()),
                                     "Server.cpp", __LINE__);
                        
                        // End early, no streams maybe?
                        b_Send = false;
                    }
                    else
                    {
                        // Next hello timeout update
                        u32_HelloTimerS = time(NULL) + (u32_TimeoutS * 0.75f);
                    }
                }
            }
            
            p_Instance->l_Send.clear();
        }
        else if (u32_HelloTimerS <= time(NULL))
        {
            // Send Hello to keep connection active
            if (MRH_SRV_SendMessage(p_Speech, MRH_SRV_C_MSG_HELLO, NULL, NULL) < 0)
            {
                c_Logger.Log(MRH_PSBLogger::WARNING, "Failed to send net message: " +
                                                     std::string(MRH_ERR_GetServerErrorString()),
                             "Server.cpp", __LINE__);
            }
            else
            {
                u32_HelloTimerS += u32_TimeoutS;
            }
        }
        
        p_Instance->c_SendMutex.unlock();
    }
    
    // Clean up
    MRH_SRV_DestroyServer(p_Context, p_Connection);
    MRH_SRV_DestroyServer(p_Context, p_Speech);
    MRH_SRV_Destroy(p_Context);
}

//*************************************************************************************
// Listen
//*************************************************************************************

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

void Server::Say(OutputStorage& c_OutputStorage)
{
    while (c_OutputStorage.GetFinishedAvailable() == false)
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
    return b_AppConnected;
}
