#include "../../master/i_net.h"

class ProxyToServer
{
    public: 
    buf_t message;
    netadr_t server;

    void Init();

    // Step 1 : QUERY THE SERVER
    int Token;
    int Version;
    void QueryServer();         
    void ProcessQueryAnswer(buf_t msg);

    void PrepareConnect();
    void SendUserInfo();

    void SendToServer(bool bEmptyMsg);
};

extern ProxyToServer remote;

bool MSG_DecompressMinilzo (buf_t &in);