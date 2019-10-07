
#include <string.h>
#include <vector>
#include <iostream>

#include "../../master/i_net.h"

typedef void (*fp)();

#define SIZE_BYTE 	1
#define SIZE_MARKER SIZE_MARKER
#define SIZE_SHORT 	2
#define SIZE_LONG	4

struct protocol_t
{
	fp onInit;
	fp onPacket;
};

// events for CTF
enum flag_score_t
{
	SCORE_NONE,
	SCORE_REFRESH,
	SCORE_KILL,
	SCORE_BETRAYAL,
	SCORE_GRAB,
	SCORE_FIRSTGRAB,
	SCORE_CARRIERKILL,
	SCORE_RETURN,
	SCORE_CAPTURE,
	SCORE_DROP,
	SCORE_MANUALRETURN,
	NUM_CTF_SCORE
};


class MessageTranslator
{
    public:
        void Copy(buf_t &in, buf_t &out, int len) {
            byte *p = in.ReadChunk(len);
            if(p)
                out.WriteChunk((const char *)p, len);
        }

		void CopyMarker(buf_t &in, buf_t &out) {
            CopyByte(in, out);
        }
	
		void CopyByte(buf_t &in, buf_t &out) {
            byte p = in.ReadByte();
            if(p)
                out.WriteByte(p);
        }

		void CopyBool(buf_t &in, buf_t &out) {
			CopyByte(in, out);
        }

		void CopyShort(buf_t &in, buf_t &out)  {
            byte p = in.ReadShort();
            if(p)
                out.WriteShort(p);
        }

		void CopyLong(buf_t &in, buf_t &out) {
            byte p = in.ReadLong();
            if(p)
                out.WriteLong(p);
        }
        
		void CopyString(buf_t &in, buf_t &out, bool bEmptyString = false)  {
            const char *p = in.ReadString();
            if(p)
                out.WriteString(p);
			else if (bEmptyString)
				out.WriteString("");
        }

		void CopyEmptyString(buf_t &in, buf_t &out)  {
            in.ReadString();
			out.WriteString("");
        }

		void ReadEmpty(buf_t &in) {
			size_t left = in.BytesLeftToRead();
			in.ReadChunk(left);
		}
};

class OdaTV : public MessageTranslator
{
public:
// network messages
// Ch0wW : PLEASE, BE ABLE TO SYNC THEM WITH COMMON/I_NET.H
    enum svc_t 
    {
	svc_abort,
	svc_full,
	svc_disconnect,
	svc_reserved3,
	svc_playerinfo,			// weapons, ammo, maxammo, raisedweapon for local player
	svc_moveplayer,			// [byte] [int] [int] [int] [int] [byte]
	svc_updatelocalplayer,	// [int] [int] [int] [int] [int]
	svc_pingrequest,		// [SL] 2011-05-11 [long:timestamp]
	svc_updateping,			// [byte] [byte]
	svc_spawnmobj,			//
	svc_disconnectclient,
	svc_loadmap,
	svc_consoleplayer,
	svc_mobjspeedangle,
	svc_explodemissile,		// [short] - netid
	svc_removemobj,
	svc_userinfo,
	svc_movemobj,			// [short] [byte] [int] [int] [int]
	svc_spawnplayer,
	svc_damageplayer,
	svc_killmobj,
	svc_firepistol,			// [byte] - playernum
	svc_fireshotgun,		// [byte] - playernum
	svc_firessg,			// [byte] - playernum
	svc_firechaingun,		// [byte] - playernum
	svc_fireweapon,			// [byte]
	svc_sector,
	svc_print,
	svc_mobjinfo,
	svc_updatefrags,		// [byte] [short]
	svc_teampoints,
	svc_activateline,
	svc_movingsector,
	svc_startsound,
	svc_reconnect,
	svc_exitlevel,
	svc_touchspecial,
	svc_changeweapon,
	svc_reserved42,
	svc_corpse,
	svc_missedpacket,
	svc_soundorigin,
	svc_reserved46,
	svc_reserved47,
	svc_forceteam,			// [Toke] Allows server to change a clients team setting.
	svc_switch,
	svc_say,				// [AM] Similar to a broadcast print except we know who said it.
	svc_reserved51,
	svc_spawnhiddenplayer,	// [denis] when client can't see player
	svc_updatedeaths,		// [byte] [short]
	svc_ctfevent,			// [Toke - CTF] - [int]
	svc_serversettings,		// 55 [Toke] - informs clients of server settings
	svc_spectate,			// [Nes] - [byte:state], [short:playernum]
	svc_connectclient,
    svc_midprint,
	svc_svgametic,			// [SL] 2011-05-11 - [byte]
	svc_timeleft,
	svc_inttimeleft,		// [ML] For intermission timer
	svc_mobjtranslation,	// [SL] 2011-09-11 - [byte]
	svc_fullupdatedone,		// [SL] Inform client the full update is over
	svc_railtrail,			// [SL] Draw railgun trail and play sound
	svc_readystate,			// [AM] Broadcast ready state to client
	svc_playerstate,		// [SL] Health, armor, and weapon of a player
	svc_warmupstate,		// [AM] Broadcast warmup state to client
	svc_resetmap,			// [AM] Server is resetting the map

	// for co-op
	svc_mobjstate = 70,
	svc_actor_movedir,
	svc_actor_target,
	svc_actor_tracer,
	svc_damagemobj,

	// for downloading
	svc_wadinfo,			// denis - [ulong:filesize]
	svc_wadchunk,			// denis - [ulong:offset], [ushort:len], [byte[]:data]
		
	// netdemos - NullPoint
	svc_netdemocap = 100,
	svc_netdemostop = 101,
	svc_netdemoloadsnap = 102,

	svc_vote_update = 150, // [AM] - Send the latest voting state to the client.
	svc_maplist = 155, // [AM] - Return a maplist status.
	svc_maplist_update = 156, // [AM] - Send the entire maplist to the client in chunks.
	svc_maplist_index = 157, // [AM] - Send the current and next map index to the client.

	// for compressed packets
	svc_compressed = 200,

	// for when launcher packets go astray
	svc_launcher_challenge = 212,
	svc_challenge = 163,
	svc_max = 255
};

// client messages
// Important to send messages
    enum clc_t {
	clc_abort,
	clc_reserved1,
	clc_disconnect,
	clc_say,
	clc_move,			// send cmds
	clc_userinfo,		// send userinfo
	clc_pingreply,		// [SL] 2011-05-11 - [long: timestamp]
	clc_rate,
	clc_ack,
	clc_rcon,
	clc_rcon_password,
	clc_changeteam,		// [NightFang] - Change your team [Toke - Teams] Made this actualy work
	clc_ctfcommand,
	clc_spectate,			// denis - [byte:state]
	clc_wantwad,			// denis - string:name, string:hash
	clc_kill,				// denis - suicide
	clc_cheat,				// denis - god, pumpkins, etc
    clc_cheatpulse,         // Russell - one off cheats (idkfa, idfa etc)
	clc_callvote,			// [AM] - Calling a vote
	clc_vote,				// [AM] - Casting a vote
	clc_maplist,			// [AM] - Maplist status request.
	clc_maplist_update,     // [AM] - Request the entire maplist from the server.
	clc_getplayerinfo,
	clc_ready,				// [AM] Toggle ready state.
	clc_spy,				// [SL] Tell server to send info about this player
	clc_privmsg,			// [AM] Targeted chat to a specific player.

	// for when launcher packets go astray
	clc_launcher_challenge = 212,
	clc_challenge = 163,
	clc_max = 255
};

	std::string map, digest;
	byte consoleplayer;
	int playermobj;
	int spawnpos[3];
	unsigned int spawnang;

void CopyProcessPackets(buf_t &in, buf_t &out);


// Proxy commands, because I'm seriously lazy
void Proxy_ServerSettings(buf_t &in, buf_t &out);
void Proxy_UserInfo(buf_t &in, buf_t &out);
void Proxy_LoadMap(buf_t &in, buf_t &out);
void Proxy_UpdateConsolePlayer(buf_t &in, buf_t &out);
void Proxy_PlayerReadyState(buf_t &in, buf_t &out);

// Player stuff
void Proxy_PlayerInfo(buf_t &in, buf_t &out);
void Proxy_SpawnPlayer(buf_t &in, buf_t &out);
void Proxy_MovePlayer(buf_t &in, buf_t &out);
void Proxy_MoveLocalPlayer(buf_t &in, buf_t &out);
void Proxy_DamagePlayer(buf_t &in, buf_t &out);
void Proxy_PlayerState(buf_t &in, buf_t &out);

// All related to game scores
void Proxy_UpdateFrags(buf_t &in, buf_t &out);
void Proxy_TeamPoints(buf_t &in, buf_t &out);
void Proxy_CTFEvent(buf_t &in, buf_t &out);
void Proxy_WarmupState(buf_t &in, buf_t &out);


// All related to messages
void Proxy_MidPrint(buf_t &in, buf_t &out);
void Proxy_Print(buf_t &in, buf_t &out);
void Proxy_Say(buf_t &in, buf_t &out);

// MOBJ related stuff
void Proxy_SpawnMObj(buf_t &in, buf_t &out);
void Proxy_MoveMObj(buf_t &in, buf_t &out);
void Proxy_MObjSpeedAngle(buf_t &in, buf_t &out);
void Proxy_KillMOBJ(buf_t &in, buf_t &out);
void Proxy_Actor_Tracer(buf_t &in, buf_t &out);
void Proxy_DamageMOBJ(buf_t &in, buf_t &out);

// World-related stuff
void Proxy_Switch(buf_t &in, buf_t &out);
void Proxy_MovingSector(buf_t &in, buf_t &out);
void Proxy_UpdateSector(buf_t &in, buf_t &out);
void Proxy_TouchSpecial(buf_t &in, buf_t &out);
void Proxy_RailTrail(buf_t &in, buf_t &out);

void Proxy_Spectate(buf_t &in, buf_t &out);

// Pings !
void Proxy_PingRequest(buf_t &in, buf_t &out);

// More less useless functions to note here...
void TimeLeft(buf_t &in, buf_t &out);
void Proxy_WadChunk(buf_t &in);
void Proxy_UpdatePing(buf_t &in, buf_t &out);
void Proxy_Corpse(buf_t &in, buf_t &out);
void Proxy_UpdateVote(buf_t &in, buf_t &out);

void Proxy_ERROR_Disconnect();

};



void OnPacketTV();
void OnInitTV();

extern OdaTV tr;
