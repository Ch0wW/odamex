//
// OdaTV - Allow multiple clients to watch the same server
// without creating extra traffic on that server
//
// This proxy will create the first connection transparently,
// but other connections will be hidden from the server
//

#include "odatv_protocol.h"
#include "odatv_connection.h"
#include "odatv_server.h"

#include "../../common/info.h"

buf_t challenge_message(MAX_UDP_PACKET), first_message(MAX_UDP_PACKET);
buf_t local_message(MAX_UDP_PACKET);

buf_t spread_message(MAX_UDP_PACKET);

OdaTV tr;

#if 0
	#define DEBUG
#endif

void debugmsg(std::string text) {
	#ifdef DEBUG
	std::cout << "[DEBUG] " << text << std::endl;
	#endif
}

void OdaTV::Proxy_PlayerInfo(buf_t &in, buf_t &out) {
	debugmsg("Received svc_playerinfo marker");
	CopyMarker(in, out);			// svc_loadmap
	CopyShort(in, out);				// booleans

	for (int i = 0; i < 4; i++)	{	// NUMAMMO
		CopyShort(in, out);
		CopyShort(in, out);
	}

	CopyByte(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
	CopyByte(in, out);

	for (int i = 0; i < 6; i++)	// NUMPOWERS
		CopyShort(in, out);

}
void OdaTV::Proxy_UserInfo(buf_t &in, buf_t &out) {
	debugmsg("Received svc_userinfo marker");
	CopyMarker(in, out);
	CopyByte(in, out);
	CopyString(in, out);
	CopyByte(in, out);
	CopyLong(in, out);

	for (int i = 0 ; i < 3 ; i++)
		CopyByte(in, out);

	CopyString(in, out, false);
	CopyShort(in, out);
}
void OdaTV::Proxy_LoadMap(buf_t &in, buf_t &out) {
	debugmsg("Received svc_loadmap marker");
	CopyMarker(in, out);			// svc_loadmap
	CopyString(in, out);		// mapname
}
void OdaTV::Proxy_UpdateConsolePlayer(buf_t &in, buf_t &out) {
	debugmsg("Received svc_consoleplayer marker");
	CopyMarker(in, out);		// svc_consoleplayer
	CopyByte(in, out);			// playerID
	CopyString(in, out);		// Fragcount/Killcount
}
void OdaTV::Proxy_UpdateFrags(buf_t &in, buf_t &out) {
	debugmsg("Received svc_updatefrags marker");
	CopyMarker(in, out);			// svc_updatefrags
	CopyByte(in, out);			// playerID
	CopyShort(in, out);			// Fragcount/Killcount
	CopyShort(in, out);			// Deathcount
	CopyShort(in, out);			// Points
}
void OdaTV::Proxy_MovePlayer(buf_t &in, buf_t &out) {
	//debugmsg("Received svc_moveplayer marker");
	CopyMarker(in, out);			
	CopyByte(in, out);
	CopyLong(in, out);
	for (int i = 0 ; i < 3 ; i++) {
		CopyLong(in, out);			// x, y, z
	}
	CopyShort(in, out);			// Angle
	CopyShort(in, out);			// Pitch
	CopyByte(in, out);			// Frame
	for (int j = 0 ; j < 3 ; j++) {
		CopyLong(in, out);		// momx, momy, momz
	}
	CopyByte(in, out);

}
void OdaTV::Proxy_MoveLocalPlayer(buf_t &in, buf_t &out) {
	debugmsg("Update svc_updatelocalplayer");
	CopyMarker(in, out);				// svc_updatelocalplayer
	CopyLong(in, out);				// player.tic
	for (int i = 0 ; i < 3 ; i++) {
		CopyLong(in, out);			// x, y, z
	}
	for (int j = 0 ; j < 3 ; j++) {
		CopyLong(in, out);			// momx, momy, momz
	}
	CopyByte(in, out);				// Waterlevel

}
void OdaTV::Proxy_WadChunk(buf_t &in) {
	debugmsg("Received svc_wadchunk marker");
	in.ReadByte();				// svc_updatelocalplayer
	int left = in.ReadLong();				// player.tic
	in.ReadShort();				// x, y, z
	in.ReadChunk(left);
}
void OdaTV::Proxy_CTFEvent(buf_t &in, buf_t &out) {
	debugmsg("Received svc_ctfevent marker");
	CopyMarker(in, out);			// svc_ctfevent
	
	int event = in.ReadByte();			// event
	out.WriteByte(event);

	if (event == SCORE_NONE) {
		for (int i = 0 ; i < 2 ; i++) {	// NUMFLAGS
			CopyByte(in, out);
			CopyByte(in, out);
		}
	} else {
		CopyByte(in, out);			// f
		CopyLong(in, out);			// f

		for (int i = 0 ; i < 2 ; i++) { 
			CopyLong(in, out);
		}
	}
}
void OdaTV::Proxy_UpdatePing(buf_t &in, buf_t &out) {
	//debugmsg("Received svc_updateping marker");
	CopyMarker(in, out);		// svc_updateping
	CopyByte(in, out);			// event
	CopyLong(in, out);			// f
}
void OdaTV::Proxy_Corpse(buf_t &in, buf_t &out) {
	debugmsg("Received svc_corpse marker");
	CopyMarker(in, out);
	CopyShort(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
}
void OdaTV::TimeLeft(buf_t &in, buf_t &out) {
	//debugmsg("Update Time left");
	CopyMarker(in, out);
	CopyShort(in, out);
}
void OdaTV::Proxy_TeamPoints(buf_t &in, buf_t &out){
	debugmsg("Update Teampoints");
	CopyMarker(in, out);
	for (int i = 0 ; i < 2 ; i++)
		CopyShort(in, out);
}
void OdaTV::Proxy_DamagePlayer(buf_t &in, buf_t &out) {
	debugmsg("Update Damageplayer");
	CopyMarker(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
	CopyShort(in, out);
}
void OdaTV::Proxy_KillMOBJ(buf_t &in, buf_t &out) {
	debugmsg("Update KillMObj");
	CopyMarker(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
	CopyLong(in, out);
	CopyByte(in, out);
}
void OdaTV::Proxy_ServerSettings(buf_t &in, buf_t &out) {
	CopyMarker(in, out);
	std::string cvarname, cvarvalue;
	while(true)
	{
		byte type = in.ReadByte();
		out.WriteByte(type);
		if(type == 1)
		{
			#ifdef DEBUG
			cvarname = in.ReadString();
			out.WriteString(cvarname.c_str());
			cvarvalue = in.ReadString();
			out.WriteString(cvarvalue.c_str());
			std::cout << "-->" << "CVAR: " << cvarname << " = " << cvarvalue << std::endl;
			#else
			CopyString(in, out);
			CopyString(in, out);
			#endif

		}
		else break;
	}
}
void OdaTV::Proxy_MidPrint(buf_t &in, buf_t &out) {
	CopyMarker(in, out);
	std::string msg = in.ReadString();
	std::cout << msg << std::endl;
	out.WriteString(msg.c_str());
	CopyShort(in, out);
}
void OdaTV::Proxy_PingRequest(buf_t &in, buf_t &out) {
		//debugmsg("Received svc_pingrequest marker");
		CopyMarker(in, out);
		int pingtime = in.ReadLong();
		out.WriteLong(pingtime);

		// Since we need to report the ping, give them a long value, even if it's not true
		remote.message.WriteByte(clc_pingreply);
		remote.message.WriteLong(pingtime);
		remote.SendToServer(true);
}
void OdaTV::Proxy_SpawnPlayer(buf_t &in, buf_t &out){
	debugmsg("Received svc_spawnplayer marker");
	CopyMarker(in, out);	// svc_spawnplayer
	CopyByte(in, out);		// PlayerID
	CopyShort(in, out);		// NetID
	CopyLong(in, out);		// Angle

	for (int i = 0 ; i < 3 ; i++)
		CopyLong(in, out);		// x, y, z
}
void OdaTV::Proxy_MoveMObj(buf_t &in, buf_t &out) {
	debugmsg("Received svc_movemobj marker");
	CopyMarker(in, out);	// svc_movemobj	
	CopyShort(in, out);		// NetID
	CopyByte(in, out);		// RNDIndex

	for (int i = 0 ; i < 3 ; i++)
		CopyLong(in, out);		// x, y, z
}
void OdaTV::Proxy_MObjSpeedAngle(buf_t &in, buf_t &out) {
	debugmsg("Received svc_mobjspeedangle marker");
	CopyMarker(in, out);	// svc_movemobj	
	CopyShort(in, out);		// NetID
	CopyLong(in, out);		// angle

	for (int i = 0 ; i < 3 ; i++)
		CopyLong(in, out);		// momx, momy, momz
}
void OdaTV::Proxy_DamageMOBJ(buf_t &in, buf_t &out) {
	debugmsg("Received svc_damagemobj marker");
	CopyMarker(in, out);	// svc_damagemobj	
	CopyShort(in, out);		// NetID
	CopyShort(in, out);		// Health
	CopyByte(in, out);		// pain
}
void OdaTV::Proxy_SpawnMObj(buf_t &in, buf_t &out) {
	//debugmsg("Received svc_spawnmobj marker. Good luck...");
	CopyMarker(in, out);	// svc_spawnmobj	

	for (int i = 0 ; i < 3 ; i++)
		CopyLong(in, out);		// x, y, z
	CopyLong(in, out);		// angle
	
	byte type = in.ReadShort();
	out.WriteShort(type);
	CopyShort(in, out);
	CopyByte(in, out);
	CopyShort(in, out);

	if (type == MT_FOUNTAIN) 
		CopyByte(in, out);
	
	if (type == MT_ZDOOMBRIDGE) {
		CopyByte(in, out);
		CopyByte(in, out);
	}

	// Ch0wW : I don't know now how to spread it, so do it as is.
	Copy(in, out, in.BytesLeftToRead());
}
void OdaTV::Proxy_Switch(buf_t &in, buf_t &out) {
	debugmsg("Received svc_switch marker");
	CopyMarker(in, out);
	CopyLong(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
	CopyShort(in, out);
	CopyLong(in, out);
}
void OdaTV::Proxy_Print(buf_t &in, buf_t &out) {
	debugmsg("Received svc_print marker");
	CopyMarker(in, out);
	CopyByte(in, out);
	CopyString(in, out);
}
void OdaTV::Proxy_Say(buf_t &in, buf_t &out) {
	debugmsg("Received svc_say marker");
	CopyMarker(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
	CopyString(in, out);
}
void OdaTV::Proxy_MovingSector(buf_t &in, buf_t &out){
	//debugmsg("Received svc_movingsector marker");
	CopyMarker(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
	CopyByte(in, out);

	// Ch0wW : since I don't know how to parse the rest...
	Copy(in, out, in.BytesLeftToRead());
}
void OdaTV::Proxy_PlayerState(buf_t &in, buf_t &out){
	debugmsg("Received svc_movingsector marker");
	CopyMarker(in, out);
	CopyByte(in, out);
	CopyShort(in, out);
	CopyByte(in, out);
	CopyShort(in, out);

	CopyByte(in, out);

	for (int i = 0; i < 4 ; i++) // NUMAMMO
		CopyShort(in,out);

	for (int i = 0; i < 2 ; i++) // NUMPSPRITES
		CopyByte(in,out);

	Copy(in, out, in.BytesLeftToRead());
}
void OdaTV::Proxy_PlayerReadyState(buf_t &in, buf_t &out){
	debugmsg("Received svc_readystate marker");
	CopyMarker(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
}
void OdaTV::Proxy_Spectate(buf_t &in, buf_t &out){
	debugmsg("Received svc_spectate marker");
	CopyMarker(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
}
void OdaTV::Proxy_Actor_Tracer(buf_t &in, buf_t &out){
	debugmsg("Received svc_actor_tracer marker");
	CopyMarker(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
}
void OdaTV::Proxy_UpdateSector(buf_t &in, buf_t &out){
	debugmsg("Received svc_sector marker");
	CopyMarker(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
	CopyShort(in, out);
}
void OdaTV::Proxy_TouchSpecial(buf_t &in, buf_t &out) {
	debugmsg("Received svc_touchspecial marker");
	CopyMarker(in, out);
	CopyShort(in, out);
}
void OdaTV::Proxy_RailTrail(buf_t &in, buf_t &out){
	debugmsg("Received svc_railtrail marker");
	CopyMarker(in, out);

	for (int i = 0; i < 3 ; i++)
		CopyShort(in, out);			// start x/y/z

	for (int i = 0; i < 3 ; i++)
		CopyShort(in, out);			// end x/y/z
}
void OdaTV::Proxy_WarmupState(buf_t &in, buf_t &out){
	debugmsg("Received svc_warmupstate marker");
	CopyMarker(in, out);
	CopyByte(in, out);			// start x/y/z

	// Ch0wW : At this point, we can guess how many bytes are read, depending on the gamemode
	// But I'm lazy.
	Copy(in, out, in.BytesLeftToRead());
}

void OdaTV::Proxy_UpdateVote(buf_t &in, buf_t &out){
	debugmsg("Received svc_vote_update marker");
	CopyMarker(in, out);
	CopyByte(in, out);
	CopyString(in, out);
	CopyShort(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
	CopyByte(in, out);
}

//---------------------------------------------------

void OdaTV::Proxy_ERROR_Disconnect() {
	// ToDo : send a disconnect to every client
}

//---------------------------------------------------


void OdaTV::CopyProcessPackets(buf_t &in, buf_t &out)
{
	while(in.BytesLeftToRead())
	{
	//	std::cout << "BYTES LEFT --> " << in.BytesLeftToRead() << std::endl; 
		byte cmd = in.NextByte();
	//	std::cout << "Reading Byte #" << (int)cmd << " (" << cmd << ")" << std::endl;
		
		switch(cmd)
		{
			case svc_loadmap: 			Proxy_LoadMap(in, out); break;
			case svc_playerinfo: 		Proxy_PlayerInfo(in, out); break;
			case svc_consoleplayer: 	Proxy_UpdateConsolePlayer(in, out); break;
			case svc_updatefrags: 		Proxy_UpdateFrags(in, out); break;
			case svc_moveplayer: 		Proxy_MovePlayer(in, out); break;
			case svc_updatelocalplayer: Proxy_MoveLocalPlayer(in, out); break;
			case svc_userinfo: 			Proxy_UserInfo(in, out); break;
			case svc_teampoints:		Proxy_TeamPoints(in, out); break;
			case svc_svgametic: 		CopyMarker(in, out); CopyByte(in, out); break;
			case svc_updateping:		Proxy_UpdatePing(in, out); break;
			case svc_spawnmobj:			Proxy_SpawnMObj(in, out); break;
			
			case svc_mobjspeedangle: 	Proxy_MObjSpeedAngle(in, out); break;
			case svc_mobjinfo:			CopyMarker(in, out); CopyShort(in, out); CopyLong(in, out); break;
			case svc_explodemissile: 	CopyMarker(in, out); CopyShort(in, out); break;
			case svc_removemobj: 		CopyMarker(in, out); CopyShort(in, out); break;
			case svc_killmobj: 			Proxy_KillMOBJ(in, out); break;
			case svc_movemobj: 			Proxy_MoveMObj(in, out); break;
			case svc_damagemobj: 		Proxy_DamageMOBJ(in, out); break;
			case svc_corpse: 			Proxy_Corpse(in, out); break;
			case svc_spawnplayer:		Proxy_SpawnPlayer(in, out); break;
			case svc_damageplayer: 		Proxy_DamagePlayer(in, out); break;
			case svc_playerstate: 		Proxy_PlayerState(in, out); break;
				
			case svc_firepistol:
			case svc_fireshotgun:
			case svc_firessg:
			case svc_firechaingun: 
				CopyMarker(in, out); 
				CopyByte(in, out); 
				break;

			case svc_fireweapon: 		CopyMarker(in, out); CopyByte(in, out); CopyLong(in, out); break;
			case svc_railtrail:			Proxy_RailTrail(in, out);

			case svc_activateline:
				CopyMarker(in, out);
				CopyLong(in, out);
				CopyShort(in, out);
				CopyByte(in, out);
				CopyByte(in, out);
				break;
			case svc_sector:			Proxy_UpdateSector(in,out);	break;
			case svc_movingsector: 		Proxy_MovingSector(in, out); break;
			case svc_switch: 			Proxy_Switch(in, out);	break;
			case svc_touchspecial:		Proxy_TouchSpecial(in, out); break;
			case svc_print: 			Proxy_Print(in, out); break;
			case svc_startsound:
			{
				CopyMarker(in, out);
				CopyShort(in, out);
				CopyLong(in, out);
				CopyLong(in, out);
				CopyByte(in, out);
				CopyByte(in, out);
				CopyByte(in, out);
				CopyByte(in, out);
			}
				break;
			case svc_soundorigin:
			{
				CopyMarker(in, out);
				CopyLong(in, out);
				CopyLong(in, out);
				CopyByte(in, out);
				CopyByte(in, out);
				CopyByte(in, out);
				CopyByte(in, out);
			}
					break;
				case svc_mobjstate: CopyMarker(in, out); CopyShort(in, out); CopyShort(in, out); break;

				case svc_actor_movedir:
					CopyMarker(in, out); CopyShort(in, out); 
					CopyByte(in, out); CopyLong(in, out);
					break;

				case svc_actor_target:
				case svc_actor_tracer:
					Proxy_Actor_Tracer(in, out);
					break;

				case svc_missedpacket:
					Copy(in, out, 1+ 6);
					break;
				case svc_forceteam: 			CopyByte(in, out); CopyByte(in, out); break;
				case svc_ctfevent: 				Proxy_CTFEvent(in, out); break;
				case svc_serversettings: 		Proxy_ServerSettings(in, out); break;
				case svc_reconnect: 			CopyByte(in, out); 	break;
				case svc_exitlevel: 			CopyByte(in, out); 	break;
				case svc_resetmap:				CopyMarker(in, out); break;
				case svc_wadinfo: 				in.ReadLong();		break;
				case svc_wadchunk: 				Proxy_WadChunk(in); 	break;
				case svc_challenge:				ReadEmpty(in); break;
				case svc_launcher_challenge:	ReadEmpty(in); break;
				case svc_connectclient:			CopyMarker(in, out); CopyByte(in, out); break;
				case svc_disconnectclient: 		CopyMarker(in, out); CopyByte(in, out);  break;
				case svc_readystate:			Proxy_PlayerReadyState(in, out); break;
				case svc_warmupstate:			Proxy_WarmupState(in, out); break;
				case svc_spectate: 				Proxy_Spectate(in, out);	break;
				case svc_say:					Proxy_Say(in, out); break;

				case svc_timeleft:
				case svc_inttimeleft:
					TimeLeft(in, out);
					break;

				case svc_compressed:		// Ch0wW : THIS CAN BE HANDLED BETTER
				{
					buf_t	newone;
					CopyMarker(in, out);
					CopyByte(in, out);
					MSG_DecompressMinilzo(in);
					int iLeft = in.BytesLeftToRead();
					CopyProcessPackets(in, newone);
					Copy(in, out, iLeft);
					return;
				}
					break;

				case svc_pingrequest:		Proxy_PingRequest(in, out); break;	
				case svc_midprint:			Proxy_MidPrint(in, out); break;	
				case svc_vote_update:		Proxy_UpdateVote(in, out); break;
				case svc_maplist:			debugmsg("MAPLIST"); break;
				case svc_maplist_index:		debugmsg("MAPLIST_INDEX"); break;
				case svc_maplist_update:	debugmsg("MAPLIST_UPDATE"); break;

				case svc_fullupdatedone:	CopyMarker(in, out); break;

				case svc_disconnect:		
				case svc_full:
					Proxy_ERROR_Disconnect();
					break;
				case svc_abort:
					std::cout << "abort #" << (int)cmd << std::endl;
					in.ReadByte();
				default:
					int iLeft = in.BytesLeftToRead();
					Copy(in, out, iLeft);
					std::cout << "Skipping " << iLeft << " bytes" << std::endl;
					break;
			}
		if(in.overflowed) {
			std::cout << "overflowed on cmd " << (int)cmd << std::endl;
		}
	}
}



void OnClientTV(int i)
{
	if(i == 0)
	{
		// only forward the first client's packet
		NET_SendPacket(net_message.cursize, net_message.data, net_from);
	}
	else	
	{
		// TODO should keep client's tick info
		// TODO and also catch client disconnect messages
	}
}

void OnPacketTV()
{
	if(NET_CompareAdr(net_from, remote.server))
	{
		buf_t out = net_message;

		// if this is a server connect message, keep a copy for other clients
		int t = net_message.ReadLong();		
		if(t == CHALLENGE)
		{
			std::cout << "Query received..." << std::endl;
			challenge_message = net_message;
			remote.ProcessQueryAnswer(challenge_message);
		}
		else if(t == 0)
		{
			first_message = net_message;
			buf_t tmp;
			tr.CopyProcessPackets(first_message, spread_message);
		}
		else
		{
			//std::cout << "Transfering message.............." << std::endl;
			out.clear();
			tr.CopyProcessPackets(net_message, spread_message);
		}
		// replicate server packet to all connected clients
		for(int i = 0; i < tvserver.spectators.size(); i++)
		{
			//std::cout << "Transfering message to specs..." << std::endl;
			NET_SendPacket(spread_message.cursize, spread_message.data, tvserver.spectators[i].address);
		}
	}
	else
	{
		// is this an existing client?
		for(int i = 0; i < tvserver.spectators.size(); i++)
		{
			if(NET_CompareAdr(net_from, tvserver.spectators[i].address))
			{
				OnClientTV(i);
				return;
			}
		}

		// must be a new client - perform checks on him.
		tvserver.OnNewClient();
	}
}

