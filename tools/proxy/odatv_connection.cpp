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

#include "../../common/version.h"
#include "../../common/minilzo.h"

#define NUMWEAPONS 9    // Change it accordingly too 

#define ODATV_HOSTNAME      "Untitled OdaTV server - TESTING! ^_^"

ProxyToServer remote;
serverquery_s query;

//
// Init
//
void ProxyToServer::Init() {
    message = buf_t(MAX_UDP_PACKET);
}

//
// QueryServer
// The Client first queries the Server to get the Token.
//
void ProxyToServer::QueryServer()
{
    message.WriteLong(LAUNCHER_CHALLENGE);
    SendToServer(false);
}

//
// ProcessQueryAnswer
// The Server got our request, process its answer.
//
void ProxyToServer::ProcessQueryAnswer(buf_t msg)
{
    #define SV tvserver.p2s_challenge
    tvserver.p2s_challenge = buf_t(MAX_UDP_PACKET);

    Token = msg.ReadLong();
    std::cout << "Token: " << Token << std::endl;

    // ===
    // At this point, get all the data from the server,
    // So that the proxy gives all the data back to the client.
    // ===
    query.hostname = ODATV_HOSTNAME; 
    msg.ReadString();
    SV.WriteString(query.hostname.c_str());
    //tr.CopyString(msg, SV);

    query.playersingame = msg.ReadByte();
    SV.WriteByte(query.playersingame);

    query.sv_maxclients = msg.ReadByte();
    SV.WriteByte(query.sv_maxclients);

    query.level_mapname = msg.ReadString();
    SV.WriteString(query.level_mapname.c_str());

    /*query.numwads = msg.ReadByte();
    SV.WriteByte(query.numwads);
    std::cout << "[WADS] Getting " << query.numwads << " PWADs" << std::endl;

     for (int i = 0 ; i < query.numwads ; i++)
        tr.CopyString(msg, SV);

   query.isDeathmatch = msg.ReadByte();
    SV.WriteByte(query.isDeathmatch);

    query.skill = msg.ReadByte();
    SV.WriteByte(query.skill);

    query.isTeamDeathmatch = msg.ReadByte();
    SV.WriteByte(query.isTeamDeathmatch);

    query.isCTF = msg.ReadByte();
    SV.WriteByte(query.isCTF);

    for (int i = 0 ; i < query.playersingame; i++) {
       tr.CopyString(msg, SV);
       tr.CopyShort(msg, SV);
       tr.CopyLong(msg, SV);

       tr.CopyByte(msg, SV);
    }

    for (int i = 1 ; i < query.numwads ; i++)
        tr.CopyString(msg, SV);

    if (query.isCTF || query.isTeamDeathmatch) {
        if (query.isCTF) {
            tr.CopyByte(msg, SV);
            tr.CopyLong(msg, SV);
        } else 
            tr.CopyByte(msg, SV);
    }

    query.version = msg.ReadShort();


    // - (bond)
    query.mail  = msg.ReadString();

    query.timelimit = msg.ReadShort();
    query.timeleft = msg.ReadShort();
    query.fraglimit = msg.ReadShort();*/

    // ===

    tr.Copy(msg, tvserver.p2s_challenge, msg.BytesLeftToRead());
    std::cout << "Copied " << tvserver.p2s_challenge.BytesLeftToRead() << " bytes." << std::endl;

 //   query.gamever = msg.ReadShort();

    PrepareConnect();
    SendToServer(true);
}

//
// Connect
// Connects to a server
//
void ProxyToServer::PrepareConnect()
{
    message.clear();
    
    message.WriteLong(CHALLENGE);
    message.WriteLong(Token);
    message.WriteShort(65);     // Ch0wW : If this is modified, look in common/i_net.h - VERSION

    message.WriteByte(0);
    message.WriteLong(GAMEVER);

    SendUserInfo();

    message.WriteLong(10000);
    message.WriteString("");
}

//
// SendUserInfo
// Sends a stub version of the UserInfo data
//
void ProxyToServer::SendUserInfo()
{
    std::cout << "Preparing userinfo... " << std::endl;
    message.WriteByte(OdaTV::clc_userinfo); // CLC_USERINFO - MARKER
    message.WriteString("OdaTV");           // UserName
    message.WriteByte(0);                   // Team
    message.WriteLong(2);                   // Gender

    // Color: We don't really care so... WHITE.
    for (int i = 3; i >= 0; i--)
		message.WriteByte(0);   

    message.WriteString("");    // Obsolete : skins

    message.WriteLong(1);       // Aim Dist
    message.WriteByte(1);       // Unlag
    message.WriteByte(1);       // Predict Weapon
    message.WriteByte(1);       // Switch Weapon

	for (size_t i = 0; i < NUMWEAPONS; i++)
		message.WriteByte (0);
}

void ProxyToServer::SendToServer(bool bEmptyMsg)
{
    NET_SendPacket(message.cursize, message.data, server);

    if (bEmptyMsg)
        message.clear();
}


//
// MSG_DecompressMinilzo
//
bool MSG_DecompressMinilzo (buf_t &in)
{
    buf_t decompressed;
    
	// decompress back onto the receive buffer
	size_t left = in.BytesLeftToRead();

	if(decompressed.maxsize() < in.maxsize())
		decompressed.resize(in.maxsize());

	lzo_uint newlen = in.maxsize();

	unsigned int r = lzo1x_decompress_safe (in.ptr() + in.BytesRead(), left, decompressed.ptr(), &newlen, NULL);

	if(r != LZO_E_OK)
	{
		std::cout << "Error: minilzo packet decompression failed with error " << r << std::endl;
		return false;
	}

	in.clear();
	memcpy(in.ptr(), decompressed.ptr(), newlen);

	in.cursize = newlen;

	return true;
}