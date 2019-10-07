#include <string.h>
#include <vector>

#ifdef UNIX
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#endif

#ifdef WIN32
#include <winsock.h>
#include <time.h>
#define usleep(n) Sleep(n/1000)
#endif

#include "odatv_protocol.h"
#include "odatv_connection.h"
#include "odatv_server.h"

netadr_t net_spread;


void OnInitTV()
{
	remote.Init();
	NET_StringToAdr("127.0.0.1:10666", &remote.server);		// Ch0wW : need to be customisable
	remote.QueryServer();
	
}

int main()
{
	protocol_t protocol = {
		OnInitTV, 
		OnPacketTV
	};

	std::cout << "OdaTV - TESTING - Copyright (C) 2006-2019 The Odamex Team" << std::endl;

	// Create a UDP socket
	localport = 10999;
	InitNetCommon();
	protocol.onInit();
	
	while (true) {
		while (NET_GetPacket()) {
			//std::cout << "New packet" << std::endl;
			protocol.onPacket();
		}
		usleep(1);
	}

	remote.message.WriteByte(OdaTV::svc_disconnect);
	remote.SendToServer(true);
	
	CloseNetwork();

}

