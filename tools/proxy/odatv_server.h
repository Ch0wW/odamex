#include <list>
#include <string.h>
#include <vector>
#include <iostream>

#include "../../master/i_net.h"

class OdaTVServer {

    public: 
    struct UserInfo {
        std::string		netname;
        int			team; // [Toke - Teams]
        int			aimdist;
        bool			unlag;
        bool			predict_weapons;
        byte			color[4];
        int     		gender;
        int         	switchweapon;
        byte			weapon_prefs[9];
    };

    struct clientreq_t {
        netadr_t address;
        buf_t challenge, answer;
        byte id;
        UserInfo userinfo;
    };

    buf_t p2s_challenge;
    typedef std::vector<clientreq_t> Spectators;
    Spectators spectators;

    void OnNewClient();
    void ReadClientChallenge (clientreq_t client);
    void SendChallengeRequest(clientreq_t client);

    Spectators::iterator GetFreeClient(void);
    bool ReadClientUserInfo(clientreq_t client);

    // Commands sent by the server, just in case
    void PrintMessage(clientreq_t client, std::string text);

    private:
        bool isValidString(const std::string& s);
        void StripColorCodes(std::string& str);
};

struct serverquery_s {
    std::string hostname;
    size_t      playersingame;
    size_t      sv_maxclients;

    std::string level_mapname;
    size_t      numwads;

    bool        isDeathmatch;
    int         skill;
    bool        isTeamDeathmatch;
    bool        isCTF;

    int         version;
    std::string mail;

    int         timelimit, timeleft, fraglimit;

    int         gamever;

};

extern OdaTVServer tvserver; 