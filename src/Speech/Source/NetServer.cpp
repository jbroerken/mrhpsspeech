/**
 *  NetServer.cpp
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
#include "./NetServer.h"
#include "../SpeechEvent.h"

// Pre-defined
#ifndef MRH_SPEECH_NET_SERVER_UPDATE_TIME_S
    #define MRH_SPEECH_NET_SERVER_UPDATE_TIME_S 5
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

NetServer::NetServer(Configuration const& c_Configuration) noexcept : b_RunThread(true),
                                                                      u64_RecieveTimestampS(0),
                                                                      u32_RecieveTimeoutS(30)
{
    MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Using remote net server.",
                                   "NetServer.cpp", __LINE__);
    
    // Grab connection info
    memset(p_AccountMail, '\0', MRH_SRV_SIZE_ACCOUNT_MAIL);
    strncpy(p_AccountMail,
            c_Configuration.GetServerAccountMail().c_str(),
            MRH_SRV_SIZE_ACCOUNT_MAIL);
    
    memset(p_AccountPassword, '\0', MRH_SRV_SIZE_ACCOUNT_PASSWORD);
    strncpy(p_AccountPassword,
            c_Configuration.GetServerAccountPassword().c_str(),
            MRH_SRV_SIZE_ACCOUNT_PASSWORD);
    
    memset(p_DeviceKey, '\0', MRH_SRV_SIZE_DEVICE_KEY);
    strncpy(p_DeviceKey,
            c_Configuration.GetServerDeviceKey().c_str(),
            MRH_SRV_SIZE_DEVICE_KEY);
    
    memset(p_DevicePassword, '\0', MRH_SRV_SIZE_DEVICE_PASSWORD);
    strncpy(p_DevicePassword,
            c_Configuration.GetServerDevicePassword().c_str(),
            MRH_SRV_SIZE_DEVICE_PASSWORD);
    
    memset(p_ServerAddress, '\0', MRH_SRV_SIZE_SERVER_ADDRESS);
    strncpy(p_ServerAddress,
            c_Configuration.GetServerAddress().c_str(),
            MRH_SRV_SIZE_SERVER_ADDRESS);
    i_ServerPort = c_Configuration.GetServerPort();
    
    u32_ConnectionTimeoutS = c_Configuration.GetServerConnectionTimeoutS();
    u32_ConnectionRetryS = c_Configuration.GetServerRetryWaitS();
    u32_RecieveTimeoutS = c_Configuration.GetServerRecieveTimeoutS();
    
    // Got connection info, now start updating client
    try
    {
        c_Thread = std::thread(ClientUpdate, this);
        
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::INFO, "Started client update thread!",
                                       "Server.cpp", __LINE__);
    }
    catch (std::exception& e)
    {
        MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to start client update thread: " +
                                                             std::string(e.what()),
                                       "Server.cpp", __LINE__);
    }
}

NetServer::~NetServer() noexcept
{
    b_RunThread = false;
    c_Thread.join();
}

//*************************************************************************************
// Client
//*************************************************************************************

void NetServer::ClientUpdate(NetServer* p_Instance) noexcept
{
    MRH_PSBLogger& c_Logger = MRH_PSBLogger::Singleton();
    
    /**
     *  Library Init
     */
    
    MRH_Srv_Context* p_Context = MRH_SRV_Init(MRH_SRV_CLIENT_PLATFORM,
                                              1, /* Connection server is replaced by communication server */
                                              p_Instance->u32_ConnectionTimeoutS * 1000);
    
    if (p_Context == NULL)
    {
        c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetServerErrorString(),
                     "Server.cpp", __LINE__);
        return;
    }
    
    MRH_Srv_Server* p_Server = MRH_SRV_CreateServer(p_Context);
    
    if (p_Server == NULL)
    {
        c_Logger.Log(MRH_PSBLogger::ERROR, MRH_ERR_GetServerErrorString(),
                     "Server.cpp", __LINE__);
        
        MRH_SRV_Destroy(p_Context);
        return;
    }
    
    /**
     *  Client Update
     */
    
    // Define locally used connection info
    ConnectionState e_State = CONNECT;
    
    uint8_t p_MessageBuffer[MRH_SRV_SIZE_MESSAGE_BUFFER_MAX];
    MRH_Srv_NetMessage e_Recieved;
    int i_Result = -1;
    
    char p_Salt[MRH_SRV_SIZE_ACCOUNT_PASSWORD_SALT];
    uint32_t u32_Nonce = 0;
    uint8_t u8_HashType = 0;
    
    // Start updating the client
    while (p_Instance->b_RunThread == true)
    {
        // Update connection depending on state
        switch (e_State)
        {
            case CONNECT:
            {
                break;
            }
                
            default:
            {
                if (MRH_SRV_IsConnected(p_Server) < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::INFO, "Connection lost, reconnecting...",
                                 "Server.cpp", __LINE__);
                
                    e_State = CONNECT;
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::seconds(MRH_SPEECH_NET_SERVER_UPDATE_TIME_S));
                }
                break;
            }
        }
        
        // State update for client connection
        switch (e_State)
        {
            /**
             *  Server Connection
             */
                
            case CONNECT:
            {
                c_Logger.Log(MRH_PSBLogger::INFO, "Connecting to connection server!",
                             "Server.cpp", __LINE__);
                
                MRH_SRV_Disconnect(p_Server, -1); // Reset server object
                
                i_Result = MRH_SRV_Connect(p_Context,
                                           p_Server,
                                           p_Instance->p_ServerAddress,
                                           p_Instance->i_ServerPort,
                                           p_Instance->u32_ConnectionTimeoutS);
                
                if (i_Result < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to connect to connection server!",
                                 "Server.cpp", __LINE__);
                    
                    std::this_thread::sleep_for(std::chrono::seconds(p_Instance->u32_ConnectionRetryS));
                }
                
                e_State = NextState(e_State, (i_Result < 0));
                break;
            }
            
            /**
             *  Authentication
             */
                
            case AUTH_SEND_REQUEST:
            {
                c_Logger.Log(MRH_PSBLogger::INFO, "Sending authentication request message...",
                             "Server.cpp", __LINE__);
                
                MRH_SRV_MSG_AUTH_REQUEST_DATA c_Request;
                strncpy(c_Request.p_Mail, p_Instance->p_AccountMail, MRH_SRV_SIZE_ACCOUNT_MAIL);
                strncpy(c_Request.p_DeviceKey, p_Instance->p_DeviceKey, MRH_SRV_SIZE_DEVICE_KEY);
                c_Request.u8_ClientType = MRH_SRV_CLIENT_PLATFORM;
                c_Request.u8_Version = MRH_SRV_NET_MESSAGE_VERSION;
                
                i_Result = MRH_SRV_SendMessage(p_Server, MRH_SRV_MSG_AUTH_REQUEST, &c_Request, NULL);
                
                if (i_Result < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to send auth request message: " +
                                                       std::string(MRH_ERR_GetServerErrorString()),
                                 "Server.cpp", __LINE__);
                }
                
                e_State = NextState(e_State, (i_Result < 0));
                break;
            }
            case AUTH_RECIEVE_CHALLENGE:
            {
                e_Recieved = RecieveServerMessage(p_Server,
                                                  { MRH_SRV_MSG_AUTH_RESULT,
                                                    MRH_SRV_MSG_AUTH_CHALLENGE },
                                                  p_MessageBuffer,
                                                  NULL);
                
                if (e_Recieved == MRH_SRV_MSG_AUTH_CHALLENGE)
                {
                    c_Logger.Log(MRH_PSBLogger::INFO, "Recieved authentication challenge message!",
                                 "Server.cpp", __LINE__);
                    
                    MRH_SRV_MSG_AUTH_CHALLENGE_DATA c_Challenge;
                    
                    if (MRH_SRV_SetNetMessage(&c_Challenge, p_MessageBuffer) < 0)
                    {
                        c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to set auth challenge message!",
                                     "Server.cpp", __LINE__);
                        
                        e_State = NextState(e_State, true);
                    }
                    else
                    {
                        // Copy to use in next step
                        memcpy(p_Salt, c_Challenge.p_Salt, MRH_SRV_SIZE_ACCOUNT_PASSWORD_SALT);
                        u32_Nonce = c_Challenge.u32_Nonce;
                        u8_HashType = c_Challenge.u8_HashType;
                        
                        e_State = NextState(e_State, false);
                    }
                }
                else if (e_Recieved == MRH_SRV_MSG_AUTH_RESULT)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to authenticate!",
                                 "Server.cpp", __LINE__);
                    
                    // Log error if possible
                    MRH_SRV_MSG_AUTH_RESULT_DATA c_Result;
                    if (MRH_SRV_SetNetMessage(&c_Result, p_MessageBuffer) == 0)
                    {
                        c_Logger.Log(MRH_PSBLogger::ERROR, "Auth request returned error: " +
                                                           std::to_string(c_Result.u8_Result),
                                     "Server.cpp", __LINE__);
                    }
                    
                    // Auth result here always means failure
                    e_State = NextState(e_State, true);
                }
                break;
            }
            case AUTH_SEND_PROOF:
            {
                c_Logger.Log(MRH_PSBLogger::INFO, "Sending authentication proof message...",
                             "Server.cpp", __LINE__);
                
                MRH_SRV_MSG_AUTH_PROOF_DATA c_Proof;
                uint8_t p_PasswordHash[MRH_SRV_SIZE_ACCOUNT_PASSWORD] = { '\0' };
                
                if ((i_Result = MRH_SRV_CreatePasswordHash(p_PasswordHash, p_Instance->p_AccountPassword, p_Salt, u8_HashType)) < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to create account password hash: " +
                                                       std::string(MRH_ERR_GetServerErrorString()),
                                 "Server.cpp", __LINE__);
                }
                else if ((i_Result = MRH_SRV_EncryptNonce(c_Proof.p_NonceHash, u32_Nonce, p_PasswordHash)) < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to encrypt recieved nonce: " +
                                                       std::string(MRH_ERR_GetServerErrorString()),
                                 "Server.cpp", __LINE__);
                }
                else if ((i_Result = MRH_SRV_SendMessage(p_Server, MRH_SRV_MSG_AUTH_PROOF, &c_Proof, NULL)) < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to send auth proof net message: " +
                                                       std::string(MRH_ERR_GetServerErrorString()),
                                 "Server.cpp", __LINE__);
                }
                
                e_State = NextState(e_State, (i_Result < 0));
                break;
            }
            case AUTH_RECIEVE_RESULT:
            {
                e_Recieved = RecieveServerMessage(p_Server,
                                                  { MRH_SRV_MSG_AUTH_RESULT },
                                                  p_MessageBuffer,
                                                  NULL);
                
                if (e_Recieved == MRH_SRV_MSG_UNK)
                {
                    break;
                }
                
                c_Logger.Log(MRH_PSBLogger::INFO, "Recieved authentication result message!",
                             "Server.cpp", __LINE__);
                
                MRH_SRV_MSG_AUTH_RESULT_DATA c_Result;
                
                if ((i_Result = MRH_SRV_SetNetMessage(&c_Result, p_MessageBuffer)) < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to set auth result message!",
                                 "Server.cpp", __LINE__);
                }
                else if (c_Result.u8_Result != 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Auth request returned error: " +
                                                       std::to_string(c_Result.u8_Result),
                                 "Server.cpp", __LINE__);
                    i_Result = -1; // Set error
                }
                
                e_State = NextState(e_State, (i_Result < 0));
                break;
            }
                
            /**
             *  Exchange Text
             */
               
            case REQUEST_TEXT:
            {
                MRH_SRV_MSG_DATA_AVAIL_DATA c_Avail;
                c_Avail.u8_Data = MRH_SRV_MSG_TEXT;
                
                i_Result = MRH_SRV_SendMessage(p_Server, MRH_SRV_MSG_DATA_AVAIL, &c_Avail, NULL);
                
                if (i_Result < 0)
                {
                    c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to send data avail message: " +
                                                       std::string(MRH_ERR_GetServerErrorString()),
                                 "CBGetLocation.cpp", __LINE__);
                }
                
                e_State = NextState(e_State, (i_Result < 0));
                break;
            }
            case EXCHANGE_TEXT:
            {
                // Set next state
                e_State = NextState(e_State, true);
                
                // First, recieve
                e_Recieved = RecieveServerMessage(p_Server,
                                                  { MRH_SRV_MSG_TEXT },
                                                  p_MessageBuffer,
                                                  p_Instance->p_DevicePassword);
                
                // Now recieve text messages
                MRH_SRV_MSG_TEXT_DATA c_Text;
                
                if (e_Recieved == MRH_SRV_MSG_TEXT)
                {
                    if (MRH_SRV_SetNetMessage(&c_Text, p_MessageBuffer) < 0)
                    {
                        c_Logger.Log(MRH_PSBLogger::ERROR, "Failed to set text message!",
                                     "Server.cpp", __LINE__);
                    }
                    else if (CommunicationActive(c_Text.u64_TimestampS, p_Instance->u32_RecieveTimeoutS) == false)
                    {
                        c_Logger.Log(MRH_PSBLogger::ERROR, "Recieved outdated message!",
                                     "Server.cpp", __LINE__);
                    }
                    else
                    {
                        // Check time stamp
                        if (p_Instance->u64_RecieveTimestampS < c_Text.u64_TimestampS)
                        {
                            p_Instance->u64_RecieveTimestampS = c_Text.u64_TimestampS;
                        }
                        
                        std::lock_guard<std::mutex> c_Guard(p_Instance->c_RecievedMutex);
                        p_Instance->l_Recieved.emplace_back(c_Text.p_String);
                    }
                }
                
                // Are we communicating?
                if (CommunicationActive(p_Instance->u64_RecieveTimestampS, p_Instance->u32_RecieveTimeoutS) == false)
                {
                    break;
                }
                
                // Recieved, now send
                std::lock_guard<std::mutex> c_Guard(p_Instance->c_SendMutex);
                
                if (p_Instance->l_Send.size() == 0)
                {
                    break;
                }
                
                i_Result = 0;
                while (i_Result == 0 && p_Instance->l_Send.size() > 0)
                {
                    std::string s_Full = p_Instance->l_Send.front();
                    std::string s_Part;
                    
                    p_Instance->l_Send.pop_front();
                    
                    while (s_Full.size() > 0)
                    {
                        if (s_Full.size() > MRH_SRV_SIZE_TEXT_STRING)
                        {
                            s_Part = s_Full.substr(0, MRH_SRV_SIZE_TEXT_STRING);
                            s_Full.erase(0, MRH_SRV_SIZE_TEXT_STRING);
                        }
                        else
                        {
                            s_Part = s_Full;
                            s_Full.clear();
                        }
                        
                        strcpy(c_Text.p_String, s_Part.c_str());
                        
                        i_Result = MRH_SRV_SendMessage(p_Server, MRH_SRV_MSG_TEXT, &c_Text, p_Instance->p_DevicePassword);
                        
                        if (i_Result < 0)
                        {
                            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to send text net message!",
                                                           "Server.cpp", __LINE__);
                            break;
                        }
                    }
                }
                break;
            }
                
            /**
             *  Unk
             */
                
            // Failed, unknown, etc
            default:
            {
                c_Logger.Log(MRH_PSBLogger::ERROR, "Unknown client state, stopping client!",
                             "Server.cpp", __LINE__);
                
                p_Instance->b_RunThread = false;
                break;
            }
        }
    }
    
    // Clean up
    e_State = CONNECTION_STATE_COUNT;
    
    MRH_SRV_DestroyServer(p_Context, p_Server);
    MRH_SRV_Destroy(p_Context);
}

NetServer::ConnectionState NetServer::NextState(ConnectionState e_State, bool b_Failed) noexcept
{
    switch (e_State)
    {
        // Server Connection
        case CONNECT:
            return b_Failed ? CONNECT : AUTH_SEND_REQUEST;
            
        // Authentication
        case AUTH_SEND_REQUEST:
            return b_Failed ? CONNECT : AUTH_RECIEVE_CHALLENGE;
        case AUTH_RECIEVE_CHALLENGE:
            return b_Failed ? CONNECT : AUTH_SEND_PROOF;
        case AUTH_SEND_PROOF:
            return b_Failed ? CONNECT : AUTH_RECIEVE_RESULT;
        case AUTH_RECIEVE_RESULT:
            return b_Failed ? CONNECT : REQUEST_TEXT;
            
        // Exchange Text
        case REQUEST_TEXT:
            return EXCHANGE_TEXT;
        case EXCHANGE_TEXT:
            return REQUEST_TEXT;
            
        // Default
        default:
            return CONNECT;
    }
}

MRH_Srv_NetMessage NetServer::RecieveServerMessage(MRH_Srv_Server* p_Server, std::vector<MRH_Srv_NetMessage> v_Message, uint8_t* p_Buffer, const char* p_Password) noexcept
{
    MRH_Srv_NetMessage e_Message;
    
    while (true)
    {
        if ((e_Message = MRH_SRV_RecieveMessage(p_Server, p_Buffer, p_Password)) == MRH_SRV_MSG_UNK)
        {
            return e_Message;
        }
        
        for (auto& Message : v_Message)
        {
            if (Message == e_Message)
            {
                return e_Message;
            }
        }
    }
}

bool NetServer::CommunicationActive(MRH_Uint64 u64_TimestampS, MRH_Uint32 u32_TimeoutS) noexcept
{
    if (u64_TimestampS < (time(NULL) + u32_TimeoutS))
    {
        return false;
    }
    
    return true;
}

//*************************************************************************************
// Retrieve
//*************************************************************************************

MRH_Uint32 NetServer::Retrieve(MRH_Uint32 u32_StringID)
{
    while (true)
    {
        std::lock_guard<std::mutex> c_Guard(c_RecievedMutex);
        
        if (l_Recieved.size() == 0)
        {
            break;
        }
        
        try
        {
            SpeechEvent::InputRecieved(u32_StringID, l_Recieved.front());
            l_Recieved.pop_front();
            
            ++u32_StringID;
        }
        catch (Exception& e)
        {
            MRH_PSBLogger::Singleton().Log(MRH_PSBLogger::ERROR, "Failed to recieve input: " +
                                                                 std::string(e.what()),
                                           "Server.cpp", __LINE__);
        }
    }
    
    return u32_StringID;
}

//*************************************************************************************
// Send
//*************************************************************************************

void NetServer::Send(OutputStorage& c_OutputStorage)
{
    std::lock_guard<std::mutex> c_Guard(c_SendMutex);
    
    while (c_OutputStorage.GetFinishedAvailable() == true)
    {
        auto String = c_OutputStorage.GetFinishedString();
        
        l_Send.emplace_back(String.s_String);
        
        try
        {
            SpeechEvent::OutputPerformed(String.u32_StringID, String.u32_GroupID);
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

bool NetServer::GetAppClientConnected() const noexcept
{
    return CommunicationActive(u64_RecieveTimestampS, u32_RecieveTimeoutS);
}
