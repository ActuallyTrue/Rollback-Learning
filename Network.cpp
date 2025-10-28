#include "Network.h"
#include <plog/Log.h>

#define NBNET_IMPL

#define NBN_LogInfo(...) PLOG_INFO << __VA_ARGS__;
#define NBN_LogError(...) PLOG_ERROR << __VA_ARGS__;
#define NBN_LogDebug(...) PLOG_DEBUG << __VA_ARGS__;
#define NBN_LogTrace(...) PLOG_INFO << __VA_ARGS__;
#define NBN_LogWarning(...) PLOG_WARNING << __VA_ARGS__;

#define HOST_PORT 50101
#define PROTOCOL_NAME "Rollback Test"


#include "nbnet.h"
#include "net_drivers/udp.h"

static NBN_ConnectionHandle client = 0;

bool InitializeHost()
{
    NBN_UDP_Register();
    //Start the server
    if (NBN_GameServer_Start(PROTOCOL_NAME, HOST_PORT) < 0)
    {
        NBN_LogInfo("Failed to start the server!");

        //Deinit server
        NBN_GameServer_Stop();

        //Error, quit the server application
        return false;
    }

    NBN_LogInfo("Started Hosting\n");

    return true;
}

bool InitializeClient()
{
    NBN_UDP_Register();

    if (NBN_GameClient_Start( PROTOCOL_NAME, "127.0.0.1", HOST_PORT) < 0)
    {
        NBN_LogError("Failed to start the client!");
        NBN_GameClient_Stop();
        return false;
    }

    NBN_LogInfo("Connected to Host!\n");

    return true;
}

void UpdateNetworkHost()
{
    int ev;

    // Poll for server events
    while ((ev = NBN_GameServer_Poll()) != NBN_NO_EVENT)
    {
        if (ev < 0)
        {
            NBN_LogError("Network Error");

            // Error, quit the server application
            exit(-1);
            break;
        }

        switch (ev)
        {
            // New connection request...
            case NBN_NEW_CONNECTION:
                if (client == 0)
                {
                    NBN_GameServer_AcceptIncomingConnection();
                    client = NBN_GameServer_GetIncomingConnection();
                    NBN_LogInfo("Accepted Client Connection!");
                }
                break;
                //
                //     // The client has disconnected
                // case NBN_CLIENT_DISCONNECTED:
                //     assert(NBN_GameServer_GetDisconnectedClient() == client);
                //
                //     client = 0;
                //     break;
                //
                //     // A message has been received from the client
                // case NBN_CLIENT_MESSAGE_RECEIVED:
                //     if (EchoReceivedMessage() < 0)
                //     {
                //         NBN_LogError("Failed to echo received message");
                //
                //         // Error, quit the server application
                //         exit(-1);
                //     }
                //     break;
        }
    }

    // Pack all enqueued messages as packets and send them
    if (NBN_GameServer_SendPackets() < 0)
    {
        NBN_LogError("Failed to send packets");

        // Error, quit the server application
        exit(-1);
    }
}

void UpdateNetworkClient()
{
    int ev;

    // Poll for client events
    while ((ev = NBN_GameClient_Poll()) != NBN_NO_EVENT)
    {
        if (ev < 0)
        {
            NBN_LogError("An error occured while polling client events. Exit");
            exit(-1);
        }

        switch (ev)
        {
            // Client is connected to the server
            case NBN_CONNECTED:
                //OnConnected();
                NBN_LogInfo("Connected to host!");
                break;
            //
            //     // Client has disconnected from the server
            // case NBN_DISCONNECTED:
            //     OnDisconnected();
            //     break;
            //
            //     // A message has been received from the server
            // case NBN_MESSAGE_RECEIVED:
            //     OnMessageReceived();
            //     break;
        }
    }

    // Pack all enqueued messages as packets and send them
    if (NBN_GameClient_SendPackets() < 0)
    {
        NBN_LogError("Failed to send packets. Exit");
    }
}

void HandleMessage()
{
    //Get info about the received message
    NBN_MessageInfo msg_info = NBN_GameServer_GetMessageInfo();

    assert(msg_info.sender == client);
    assert(msg_info.type == NBN_BYTE_ARRAY_MESSAGE_TYPE);

    //retrieve the received message
    auto* msg = (NBN_ByteArrayMessage*)msg_info.data;

    // If the send fails the client will be disconnected and a NBN_CLIENT_DISCONNECTED event
    // will be received (see event polling in main)
    if (NBN_GameServer_SendReliableByteArrayTo(client, msg->bytes, msg->length) < 0)
    {
        exit(-1);
    }

}
