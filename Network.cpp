#include "Network.h"
#include <plog/Log.h>

#define NBNET_IMPL

#define NBN_LogInfo(...) LOG_INFO << __VA_ARGS__;
#define NBN_LogError(...) LOG_ERROR << __VA_ARGS__;
#define NBN_LogDebug(...) LOG_DEBUG << __VA_ARGS__;
#define NBN_LogTrace(...) LOG_INFO << __VA_ARGS__;
#define NBN_LogWarning(...) LOG_WARNING << __VA_ARGS__;

#define HOST_PORT 50101


#include "nbnet.h"
#include "net_drivers/udp.h"

static NBN_Connection* client = NULL;

bool InitializeHost()
{
    //Start the server
    if (NBN_GameServer_Start("Rollback Test", HOST_PORT) < 0)
    {
        //NET_LOG("Failed to start the server!");

        //Deinit server
        NBN_GameServer_Stop();

        //Error, quit the server application
        return true;
    }

    return false;
}