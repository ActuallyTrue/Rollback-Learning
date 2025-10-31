#pragma once

/*
 * Start hosting a game.
 * @return True when the server failed to initialize.
 */
bool InitializeHost();
bool InitializeClient();
void UpdateNetworkHost();
void UpdateNetworkClient();
void HandleMessage();
void SendDebugMessage();