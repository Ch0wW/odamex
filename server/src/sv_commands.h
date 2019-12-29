#include <string>

#include "actor.h"
#include "d_player.h"
#include "i_net.h"

// PRINT Messages
void SVCMD_PrintMessageToPlayer(client_t *cl, int level, std::string message);

// WARMUP's "Ready" State
void SVCMD_SetReadyStatus(client_t *cl, int playerid, bool newstatus);
void SVCMD_BroadcastReadyStatus(int playerid, bool newstatus);

// Player Scores Updates
void SVCMD_SetPlayerScores(client_t *cl, player_t &it);
void SVCMD_UpdateScores(player_t &it);

// Spectator State
void SVCMD_SetSpectatorStatus(client_t *cl, int playerid, bool newstatus);
void SVCMD_BroadcastSpectatorStatus(int playerid, bool newstatus);

// Intermission's "Ready" State
void SVCMD_SetIntermissionReadyStatus(client_t *cl, int playerid, bool newstatus);
void SVCMD_BroadcastIntermissionReadyStatus(int playerid, bool newstatus);

// Ping
void SVCMD_SendPingRequest(client_t* cl);
void SVCMD_UpdatePlayersPing(client_t* cl);

// Userinfo
void SVCMD_SendUserInfo(player_t &player, client_t* cl);
void SVCMD_BroadcastUserInfo(player_t &player);