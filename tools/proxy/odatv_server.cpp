#include <set>
#include <list>
#include "odatv_server.h"
#include "odatv_protocol.h"
#include "odatv_connection.h"

#define MAXCLIENTS 10

#define AUTH_TOKEN 666666

OdaTVServer tvserver;
std::set<byte> free_player_ids;
extern serverquery_s query;


void OdaTVServer::PrintMessage(clientreq_t client, std::string text)
{
    client.answer.clear();
    client.answer.WriteByte(OdaTV::svc_print);
    client.answer.WriteString(text.c_str());
    NET_SendPacket(client.answer.cursize, client.answer.data, client.address);
}

//
// StripColorCodes
//
// Removes any color code markup from the given string.
//
void OdaTVServer::StripColorCodes(std::string& str)
{
	size_t pos = 0;
	while (pos < str.length())
	{
		if (str.c_str()[pos] == '\\' && str.c_str()[pos + 1] == 'c' && str.c_str()[pos + 2] != '\0')
			str.erase(pos, 3);
		else
			pos++;
	}
}

// Ensure that a string only has valid viewable ASCII in it.
bool OdaTVServer::isValidString(const std::string& s)
{
	for (std::string::const_iterator it = s.begin();it != s.end();++it)
	{
		const char c = *it;
		if (c < ' ' || c > '~')
			return false;
	}
	return true;
}

//Get next free player. Will use the lowest available player id.
OdaTVServer::Spectators::iterator OdaTVServer::GetFreeClient(void)
{
	if (spectators.size() >= MAXCLIENTS)
		return spectators.end();

	if (free_player_ids.empty())
	{
		// list of free ids needs to be initialized
		for (int i = 1; i < MAXCLIENTS; i++)
			free_player_ids.insert(i);
	}

	spectators.push_back(clientreq_t());

	// generate player id
	std::set<byte>::iterator id = free_player_ids.begin();
	spectators.back().id = *id;
	free_player_ids.erase(id);

	// update tracking cvar
	//sv_clientcount.ForceSet(players.size());

	// Return iterator pointing to the just-inserted player
	Spectators::iterator it = spectators.end();
	return --it;
}

void OdaTVServer::ReadClientChallenge (clientreq_t client)
{
    client.answer.clear();

    int requestCode = client.challenge.ReadLong();

    // During the initial
    if (requestCode == LAUNCHER_CHALLENGE) {
        std::cout << "Sending Launcher Challenge with Token..." << std::endl;
        SendChallengeRequest(client);
        return;
    }

    if (requestCode != CHALLENGE) {
        std::cout << "Received unknown Challenge code " << requestCode << std::endl;
        return;
    }

    int auth = client.challenge.ReadLong();
    if (auth != AUTH_TOKEN) {
        std::cout << "Received unknown Token " << auth << std::endl;
        return;
    }

    PrintMessage(client, "YO THIS WORKS\n");

    std::cout << requestCode << " // " << auth << std::endl;
    std::cout << "TRYING ?" << std::endl;
    
    // Check how many clients can connect, and kick any who cross the line
    Spectators::iterator it = GetFreeClient();

    if (it == spectators.end()) {
        client.answer.WriteByte(OdaTV::svc_full);
		NET_SendPacket(client.answer.cursize, client.answer.data, client.address);
        return;
    }

    std::cout << "past Free Client" << std::endl;

    // Read version
    int version = client.challenge.ReadShort();

    // Read if needing to download
    int conn_type = client.challenge.ReadByte();
    if (conn_type == 1) {
        client.answer.WriteByte(OdaTV::svc_disconnect);
		NET_SendPacket(client.answer.cursize, client.answer.data, client.address);
        return;
    }

    PrintMessage(client, "HEY BIATCH\n");

    client.challenge.ReadLong();    // Gamever - unused

    // Userinfo shit
    {
        OdaTV::clc_t userinfo = (OdaTV::clc_t)client.challenge.ReadByte();
        if (userinfo != OdaTV::clc_userinfo ) {
            client.answer.WriteByte(OdaTV::svc_disconnect);
		    NET_SendPacket(client.answer.cursize, client.answer.data, client.address);
            return;
        }
        std::cout << "PAST USERINFO" << std::endl;
        if (!ReadClientUserInfo(client)) {      
            client.answer.WriteByte(OdaTV::svc_disconnect);
            NET_SendPacket(client.answer.cursize, client.answer.data, client.address);
            return;
        }
    }
    
    std::cout << "PAST READ USERINFO" << std::endl;
    client.challenge.ReadLong();    // userrate, ignore it for now

    std::string passhash = client.challenge.ReadString();    // for now, we don't plan to add passwords

    // Send Consoleplayer data
    {
        client.answer = buf_t(1024);
        client.answer.WriteByte(OdaTV::svc_consoleplayer);
        client.answer.WriteByte(client.id);
        client.answer.WriteString("azazazazazaza"); // digest hash - ToDo : add it
        NET_SendPacket(client.answer.cursize, client.answer.data, client.address);
    }

    std::cout << "END " << auth << std::endl;
}

// SendChallengeRequest
// Send a byte with OUR token, and the data originally from the server.
void OdaTVServer::SendChallengeRequest(OdaTVServer::clientreq_t client)
{
    buf_t chal(MAX_UDP_PACKET);

    chal.WriteLong(CHALLENGE);
    chal.WriteLong(AUTH_TOKEN);
    tr.Copy(tvserver.p2s_challenge, chal, tvserver.p2s_challenge.BytesLeftToRead());

    NET_SendPacket(chal.cursize, chal.data, client.address);
}


void OdaTVServer::OnNewClient()
{
	OdaTVServer::clientreq_t cl = OdaTVServer::clientreq_t { net_from, net_message };

    std::cout << "Connexion requested from " << NET_AdrToString(net_from) << std::endl;

    ReadClientChallenge(cl);

	//spectators.push_back(net_from);

		//NET_SendPacket(spread_message.cursize, spread_message.data, net_from);
	//	std::cout << "shadow client connect from #" << (int)tr.consoleplayer << " " << NET_AdrToString(net_from) << std::endl;
}

bool OdaTVServer::ReadClientUserInfo(clientreq_t client) {

    client.userinfo.netname = client.challenge.ReadString();
    std::cout << client.userinfo.netname << std::endl;

    if (client.userinfo.netname.length() > 15)
        client.userinfo.netname.erase(15);

    if (!isValidString(client.userinfo.netname))
        return false;

    client.challenge.ReadByte();        // Team
    client.userinfo.team = 0;           

    // Gender
    client.challenge.ReadLong();
    client.userinfo.gender = 0;

    // Color
    for (int i = 0 ; i < 3 ; i++){
        client.challenge.ReadByte();
        client.userinfo.color[i] = 0;
    }

    client.challenge.ReadString();  // Depreciated skins
    client.challenge.ReadLong();
    client.userinfo.aimdist = 0;    // Aimdist
    client.challenge.ReadByte();
    client.userinfo.unlag = 1;      // Unlag
    client.challenge.ReadByte();
    client.userinfo.predict_weapons = 1; // predict weapons

    client.challenge.ReadByte();
    client.userinfo.switchweapon = 1; // Switch weapons

    for (int i = 0 ; i < 9 ; i++) {
        client.challenge.ReadByte();
        client.userinfo.weapon_prefs[i] = 1; // Switch weapons
    }

    return true;
}