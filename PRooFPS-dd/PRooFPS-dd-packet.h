#pragma once

/*
    ###################################################################################
    PRooFPS-dd-packet.h
    Network packets defined for PRooFPS-dd.
    Made by PR00F88
    2022
    ###################################################################################
*/

#include <array>
#include <string>
#include <unordered_map>

#include "Network/PgePacket.h"
#include "Weapons/WeaponManager.h"

#include "GameMode.h"
#include "MapItem.h"
#include "Strafe.h"

namespace proofps_dd
{

    enum class PRooFPSappMsgId : pge_network::MsgApp::TMsgId  /* underlying type should be same as type of pge_network::MsgApp::msgId */
    {
        ServerInfoFromServer = 0,
        GameSessionStateFromServer,
        MapChangeFromServer,
        UserSetupFromServer,
        UserNameChangeAndBootupDone,
        UserCmdFromClient,
        UserUpdateFromServer,
        BulletUpdateFromServer,
        MapItemUpdateFromServer,
        WpnUpdateFromServer,
        CurrentWpnUpdateFromServer,
        DeathNotificationFromServer,
        PlayerEventFromServer,
        UserInGameMenuCmd,
        LastMsgId
    };

    struct PRooFPSappMsgId2ZStringPair
    {
        PRooFPSappMsgId msgId;
        const char* const zstring;
    };

    constexpr auto MapMsgAppId2String = PFL::std_array_of<PRooFPSappMsgId2ZStringPair>
    (
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::ServerInfoFromServer,        "MsgServerInfoFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::GameSessionStateFromServer,  "MsgGameSessionStateFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::MapChangeFromServer,         "MsgMapChangeFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::UserSetupFromServer,         "MsgUserSetupFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::UserNameChangeAndBootupDone, "MsgUserNameChangeAndBootupDone" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::UserCmdFromClient,           "MsgUserCmdFromClient" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::UserUpdateFromServer,        "MsgUserUpdateFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::BulletUpdateFromServer,      "MsgBulletUpdateFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::MapItemUpdateFromServer,     "MsgMapItemUpdateFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::WpnUpdateFromServer,         "MsgWpnUpdateFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::CurrentWpnUpdateFromServer,  "MsgCurrentWpnUpdateFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::DeathNotificationFromServer, "MsgDeathNotificationFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::PlayerEventFromServer,       "MsgPlayerEventFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::UserInGameMenuCmd,           "MsgUserInGameMenuCmd" }
    );

    // this way nobody will forget updating both the enum and the array
    static_assert(static_cast<size_t>(PRooFPSappMsgId::LastMsgId) == MapMsgAppId2String.size());

    // server -> clients
    // sent to any connecting client, or in the future even before connecting e.g. when listing servers.
    // Client initialization/player bringup SHALL NOT depend on this, this is just for informational purpose so that
    // server config can be seen on client-side as well. Some values might be used to alter client's behavior, e.g.
    // nRespawnTimeSecs is actually needed by client for proper visualization of the respawn time countdown.
    // 
    // Also, since this message teaches the client about the actual game mode, client shall receive it soon after
    // connecting so depending on game mode client might do additional setup, such as asking user to select team.
    // 
    // As stated above, "client initialization/player bringup SHALL NOT depend on this", so it does not interfere with
    // the MsgUserConnected / MsgUserSetupFromServer / MsgUserNameChangeAndBootupDone trio.
    // Before v0.5 this was sent out after MsgUserNameChangeAndBootupDone.
    // But from v0.5 I'm sending this msg to connecting client right in server handling MsgUserConnected, before sending
    // out any player setup stuff to client. This way, client can reconfigure GameMode instance before receiving any
    // player info that needs to be added to the GameMode instance.
    // 
    // nTimeRemainingSecs is used by client to start its own countdown on its side from server's nTimeRemainingSecs.
    // Thus this message needs to be sent out at least once to new client AFTER client bringup is done.
    // This is still valid in v0.5, this msg is also sent out one more time to client after player bringup (bootup) is done.
    // It won't be exactly the same countdown as on server's side, this is just for ROUGH informational purpose so that
    // client can also show remaining time without any proper synchronization with server.
    struct MsgServerInfoFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::ServerInfoFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const unsigned int& nMaxFps,
            const unsigned int& nTickrate,
            const unsigned int& nPhysicsRateMin,
            const unsigned int& nClientUpdateRate,
            const GameModeType& iGameModeType,
            const unsigned int& nFragLimit,
            const unsigned int& nTimeLimitSecs,
            const unsigned int& nTimeRemainingMillisecs,
            const int& nFallDamageMultiplier,
            const unsigned int& nRespawnTimeSecs,
            const unsigned int& nRespawnInvulnerabilityTimeSecs)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgServerInfoFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, 0u /*m_connHandleServerSide is ignored in this message*/);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgServerInfoFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgServerInfoFromServer& msgServerInfo = reinterpret_cast<proofps_dd::MsgServerInfoFromServer&>(*pMsgAppData);

            msgServerInfo.m_nMaxFps = nMaxFps;
            msgServerInfo.m_nTickrate = nTickrate;
            msgServerInfo.m_nPhysicsRateMin = nPhysicsRateMin;
            msgServerInfo.m_nClientUpdateRate = nClientUpdateRate;

            msgServerInfo.m_iGameModeType = iGameModeType;
            msgServerInfo.m_nFragLimit = nFragLimit;
            msgServerInfo.m_nTimeLimitSecs = nTimeLimitSecs;
            msgServerInfo.m_nTimeRemainingMillisecs = nTimeRemainingMillisecs;

            msgServerInfo.m_nFallDamageMultiplier = nFallDamageMultiplier;

            msgServerInfo.m_nRespawnTimeSecs = nRespawnTimeSecs;
            msgServerInfo.m_nRespawnInvulnerabilityTimeSecs = nRespawnInvulnerabilityTimeSecs;

            return true;
        }

        unsigned int m_nMaxFps;
        unsigned int m_nTickrate;
        unsigned int m_nPhysicsRateMin;
        unsigned int m_nClientUpdateRate;

        GameModeType m_iGameModeType;
        unsigned int m_nFragLimit;
        unsigned int m_nTimeLimitSecs;
        unsigned int m_nTimeRemainingMillisecs;

        int m_nFallDamageMultiplier;

        unsigned int m_nRespawnTimeSecs;
        unsigned int m_nRespawnInvulnerabilityTimeSecs;

    };  // struct MsgServerInfoFromServer
    static_assert(std::is_trivial_v<MsgServerInfoFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgServerInfoFromServer>);
    static_assert(std::is_standard_layout_v<MsgServerInfoFromServer>);

    // server -> clients
    // sent to all clients when game session state is changed (e.g. frag limit reached or whatever goal is defined in GameMode object).
    struct MsgGameSessionStateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::GameSessionStateFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            bool bGameSessionEnd)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgGameSessionStateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, 0u /*m_connHandleServerSide is ignored in this message*/);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgGameSessionStateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgGameSessionStateFromServer& msgGameSessionState = reinterpret_cast<proofps_dd::MsgGameSessionStateFromServer&>(*pMsgAppData);
            
            msgGameSessionState.m_bGameSessionEnd = bGameSessionEnd;

            return true;
        }

        bool m_bGameSessionEnd;
    };  // struct MsgGameSessionStateFromServer
    static_assert(std::is_trivial_v<MsgGameSessionStateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgGameSessionStateFromServer>);
    static_assert(std::is_standard_layout_v<MsgGameSessionStateFromServer>);

    // server -> self (inject) and clients
    // sent to all clients when map is changing
    // So currently this is NOT used at bootup.
    struct MsgMapChangeFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::MapChangeFromServer;
        static const uint8_t nMapFilenameMaxLength = 64;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const std::string& sMapFilename)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgMapChangeFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, 0u /*m_connHandleServerSide is ignored in this message*/);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgMapChangeFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgMapChangeFromServer& msgUserSetup = reinterpret_cast<proofps_dd::MsgMapChangeFromServer&>(*pMsgAppData);
            strncpy_s(msgUserSetup.m_szMapFilename, sizeof(msgUserSetup.m_szMapFilename), sMapFilename.c_str(), sMapFilename.length());

            return true;
        }

        char m_szMapFilename[nMapFilenameMaxLength];
    };  // struct MsgMapChangeFromServer
    static_assert(std::is_trivial_v<MsgMapChangeFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgMapChangeFromServer>);
    static_assert(std::is_standard_layout_v<MsgMapChangeFromServer>);

    /*
     * As of v0.1.6.1, there are 3 messages that are needed to be processed by server and clients to bring up a new player successfully.
     * The order of these messages is defined as:
     *  - pge_network::MsgUserConnectedServerSelf;
     *  - proofps_dd::MsgUserSetupFromServer;
     *  - proofps_dd::MsgUserNameChangeAndBootupDone.
     * 
     * However, when multiple players are connecting simultaneously to the server, messages for different players can come
     * interleaved, thus handling of these messages must take this into account.
     * 
     * 1.) pge_network::MsgUserConnectedServerSelf
     *     This message is present only on the server, thus it cannot cause any trouble on client-side.
     *     It is guaranteed to be the first message for server app level to receive for itself right after listening is started.
     *     It is also guaranteed that at GNS level the connection is already transitioned from Connecting to Connected state when
     *     this message is delivered to application level because this message is directly injected into the message queue, and
     *     in the next game loop iteration PgeServer::Update() runs connection transition callbacks again, and only after that
     *     we start delivering the received messages from the queue.
     * 
     *     When we get this message at application level, the connected client is already added to PgeGnsServer::m_mapClients.
     *     This is important to understand because from this point sendToAllClientsExcept() includes this newly connected instance too.
     *     
     *     PRooFPS-dd-PGE::m_mapPlayers is still empty at this point, so at app level we still don't have any player.
     *     
     *     When processing this message, server sends the following messages regarding the connecting instance to itself, other clients
     *     based on PgeGnsServer::m_mapClients and then to the connecting client too:
     *     - MsgUserSetupFromServer;
     *     - MsgUserUpdateFromServer.
     * 
     * 2.) proofps_dd::MsgUserSetupFromServer
     *     If the server is setuping itself, then only a MsgUserNameChangeAndBootupDone will be injected, assuming there are no other players at this point
     *     to be notified. The assumption is good, since we learnt above that when server starts listening, the first message will be
     *     the MsgUserConnectedServerSelf for the server itself, which generates this MsgUserSetupFromServer too.
     * 
     *     If the server is setuping a client, then the following messages will be sent:
     *      - iterating over PRooFPS-dd-PGE::m_mapPlayers, sending out messages about the players already in the container to the client being set up:
     *        - MsgUserSetupFromServer;
     *        - MsgUserNameChangeAndBootupDone (if that player's name is not still empty);
     *        - MsgUserUpdateFromServer;
     *        - MsgCurrentWpnUpdateFromServer;
     *      - iterating over map items, sending out state of all map items to the client being set up:
     *        - MsgMapItemUpdateFromServer (but I leave this out from the analysis below).
     * 
     *     If we are client then we are sending a MsgUserNameChangeAndBootupDone to the server.
     * 
     *     Only after this the new player is added to PRooFPS-dd-PGE::m_mapPlayers.
     * 
     * 3.) proofps_dd::MsgUserNameChangeAndBootupDone
     *     Server reacts to this with the same kind of message, client doesn't react to it.
     *     This is when the player is added to PRooFPS-dd-PGE::m_gameMode.
     * 
     * In the following paragraph I analyse how messages come when MsgUserConnectedServerSelf is kicking in for all players at the same time, with 3 players:
     * 1 server (connection handle 0) and 2 client players with connection handles 100 and 400.
     * We are right after a map change, so server just started to listen again and clients are connecting simulatenously.
     * We are monitoring also PgeGnsServer::m_mapClients in the server, and also PRooFPS-dd-PGE::m_mapPlayers and PRooFPS-dd-PGE::m_gameMode in all iterations.
     * Time is elapsing from top to bottom in column 1 first, then in column 2, and so on, so the whole procedure finishes somewhere in the bottom right.
     * 
     *            I T E R A T I O N  1         |         I T E R A T I O N  2               |         I T E R A T I O N  3          |         I T E R A T I O N  4
     *                                         |                                            |                                       |
     * PgeGnsServer::m_mapClients  : {}        | PgeGnsServer::m_mapClients:                |  PgeGnsServer::m_mapClients:          |
     *                                         |    {0, 100, 400}                           |     {0, 100, 400}                     |
     *                                         | PRooFPS-dd-PGE(0)::m_mapPlayers: {}        |  PRooFPS-dd-PGE(0)::m_mapPlayers:     |
     *                                         |                                            |  {0, 100, 400}                        |
     *                                         | PRooFPS-dd-PGE(100)::m_mapPlayers: {}      |  PRooFPS-dd-PGE(100)::m_mapPlayers:   |
     *                                         |                                            |  {100, 400}                           |
     *                                         | PRooFPS-dd-PGE(400)::m_mapPlayers: {}      |  PRooFPS-dd-PGE(400)::m_mapPlayers:   |
     *                                         |                                            |  {400}                                |
     * Server connecting:                      |                                            |                                       |
     * - MsgUserConnectedServerSelf(0)         |                                            |                                       |
     *   PgeGnsServer::m_mapClients: {0}       |                                            |                                       |
     *   -> MsgUserSetupFromServer(0)  ------->| 0 (self)                                   |                                       |
     *                                         |   -> MsgUserNameChangeAndBootupDone(0) --->|  0 (self)                             |
     *                                         |   PRooFPS-dd-PGE(0)::m_mapPlayers: {0}     |                                       |
     *                                         |                                            |                                       |
     *   -> MsgUserUpdateFromServer(0) ------->| 0 (self)                                   |                                       |
     *                                         |                                            |                                       |
     *                                         |                                            |                                       |
     * Client with handle 100 connecting:      |                                            |                                       |
     * - MsgUserConnectedServerSelf(100)       |                                            |                                       |
     *   PgeGnsServer::m_mapClients: {0, 100}  |                                            |                                       |
     *   -> MsgUserSetupFromServer(100) ------>| 0 (self)                                   |                                       |
     *                                         |   -> MsgUserSetupFromServer(0) ----------->|  100                                  |
     *                                         |                                            |    PRooFPS-dd-PGE(100)::m_mapPlayers: |
     *                                         |                                            |    {0, 100, 400}                      |
     *                                         |   -> MsgUserNameChangeAndBootupDone(0) --->|  100                                  |
     *                                         |   -> MsgUserUpdateFromServer(0) ---------->|  100                                  |
     *                                         |   -> MsgCurrentWpnUpdateFromServer(0) ---->|  100                                  |
     *                                         |   PRooFPS-dd-PGE(0)::m_mapPlayers:         |                                       |
     *                                         |      {0, 100}                              |                                       |
     *                                         |                                            |                                       |
     *   -> MsgUserSetupFromServer(100) ------>| 100                                        |                                       |
     *                                         |   -> MsgUserNameChangeAndBootupDone(100)-->|  0                                    |
     *                                         |                                            |    -> MsgUserNameChange ------------->|  100
     *                                         |   PRooFPS-dd-PGE(100)::m_mapPlayers:       |       AndBootupDone(100)              |
     *                                         |      {100}                                 |                                       |
     *                                         |                                            |                                       |
     *   -> MsgUserUpdateFromServer(100) ----->| 0 (self)                                   |                                       |
     *   -> MsgUserUpdateFromServer(100) ----->| 100                                        |                                       |
     *                                         |                                            |                                       |
     *                                         |                                            |                                       |
     * Client with handle 400 connecting:      |                                            |                                       |
     * - MsgUserConnectedServerSelf(400)       |                                            |                                       |
     *   PgeGnsServer::m_mapClients:           |                                            |                                       |
     *   {0, 100, 400}                         |                                            |                                       |
     *   -> MsgUserSetupFromServer(400) ------>| 0 (self)                                   |                                       |
     *                                         |   -> MsgUserSetupFromServer(0) ----------->|  400                                  |
     *                                         |                                            |    PRooFPS-dd-PGE(400)::m_mapPlayers: |
     *                                         |                                            |    {0, 400}                           |
     *                                         |   -> MsgUserNameChangeAndBootupDone(0) --->|  400                                  |
     *                                         |   -> MsgUserUpdateFromServer(0) ---------->|  400                                  |
     *                                         |   -> MsgCurrentWpnUpdateFromServer(0) ---->|  400                                  |
     *                                         |   -> MsgUserSetupFromServer(100) --------->|  400                                  |
     *                                         |                                            |    PRooFPS-dd-PGE(400)::m_mapPlayers: |
     *                                         |                                            |    {0, 100, 400}                      |
     *                                         |   -> MsgUserNameChangeAndBootupDone(100)-->|  400                                  |
     *                                         |   -> MsgUserUpdateFromServer(100) -------->|  400                                  |
     *                                         |   -> MsgCurrentWpnUpdateFromServer(100)--->|  400                                  |
     *                                         |   PRooFPS-dd-PGE(0)::m_mapPlayers:         |                                       |
     *                                         |      {0, 100, 400}                         |                                       |
     *                                         |                                            |                                       |
     *   -> MsgUserSetupFromServer(400) ------>| 100                                        |                                       |
     *                                         |   PRooFPS-dd-PGE(100)::m_mapPlayers:       |                                       |
     *                                         |      {100, 400}                            |                                       |
     *                                         |                                            |                                       |
     *   -> MsgUserSetupFromServer(400) ------>| 400                                        |                                       |
     *                                         |   -> MsgUserNameChangeAndBootupDone(400)-->|  0                                    |
     *                                         |                                            |    -> MsgUserNameChange ------------->|  400
     *                                         |   PRooFPS-dd-PGE(400)::m_mapPlayers:       |       AndBootupDone(400)              |
     *                                         |      {400}                                 |                                       |
     *                                         |                                            |                                       |
     *   -> MsgUserUpdateFromServer(400) ----->| 0 (self)                                   |                                       |
     *   -> MsgUserUpdateFromServer(400) ----->| 400                                        |                                       |
     *
     * 
     * Some conclusions:
     * - PgeGnsServer::m_mapClients already contains every instance after iteration 1 but PRooFPS-dd-PGE::m_mapPlayers is still empty;
     * - the situation is also good with a different sequence when both MsgUserConnectedServerSelf(0) and MsgUserConnectedServerSelf(100) and
     *   both MsgUserSetupFromServer(0) and MsgUserSetupFromServer(100) are processed: no duplicate/redundant message will be received
     *   by any client about any other client;
     * - however what is not seen above is that when multiple MsgUserConnectedServerSelf messages are injected by server to itself,
     *   it actually means that the connection state of multiple clients has changed to Connected, so they are in PgeGnsServer's
     *   m_mapClients map, leading to sending some redundant messages by the app without knowing it.
     *   Because of this, we must NOT fail in handleUserSetupFromServer() when the received connection handle is already present
     *   in m_mapPlayers. A ticket has been filed for this: https://github.com/proof88/PRooFPS-dd/issues/268 .
     *
     */

    // server -> self (inject) and clients
    // sent to all clients after the connecting client has been accepted by server
    // This message is also used to tell the client which map to load at bootup.
    // However, in case of map change, MsgMapChangeFromServer contains this info, but this message also contains it again.
    // A unique pre-generated user name is also in the message, which must be accepted by clients, however clients are
    // also encouraged to request user name change using MsgUserNameChangeAndBootupDone message.
    struct MsgUserSetupFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserSetupFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            bool bCurrentClient,
            const std::string& sIpAddress,
            const std::string& sMapFilename)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserSetupFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgUserSetupFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserSetupFromServer& msgUserSetup = reinterpret_cast<proofps_dd::MsgUserSetupFromServer&>(*pMsgAppData);
            msgUserSetup.m_bCurrentClient = bCurrentClient;
            strncpy_s(msgUserSetup.m_szIpAddress, sizeof(msgUserSetup.m_szIpAddress), sIpAddress.c_str(), sIpAddress.length());
            strncpy_s(msgUserSetup.m_szMapFilename, sizeof(msgUserSetup.m_szMapFilename), sMapFilename.c_str(), sMapFilename.length());

            return true;
        }

        bool m_bCurrentClient;
        char m_szIpAddress[pge_network::MsgUserConnectedServerSelf::nIpAddressMaxLength];
        char m_szMapFilename[MsgMapChangeFromServer::nMapFilenameMaxLength];
    };  // struct MsgUserSetupFromServer
    static_assert(std::is_trivial_v<MsgUserSetupFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgUserSetupFromServer>);
    static_assert(std::is_standard_layout_v<MsgUserSetupFromServer>);

    // client -> server -> clients
    // MsgUserNameChangeAndBootupDone messages are first sent by client to server to request user name change, for which the server will reply back the
    // same kind of message. Since server is in charge for ensuring unique user names for all players, it might generate a unique user name
    // for the user in case of name collision. A response message is always sent back to the clients, containing either the accepted or the
    // newly generated user name, which must be accepted by the clients.
    // Server also injects this message to itself, names are handled in the same code path.
    // Client sends this message to server after it already loaded all resources including the map, this is why we are treating this as "bootup done"
    // message, so on server side we can start the respawn-invulnerability timer also at this point.
    struct MsgUserNameChangeAndBootupDone
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserNameChangeAndBootupDone;
        static const uint8_t nUserNameBufferLength = sizeof("megszentsegtelenithetetlensegeskedeseitekert");

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            bool bCurrentClient,
            const std::string& sUserName)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserNameChangeAndBootupDone) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgUserNameChangeAndBootupDone));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserNameChangeAndBootupDone& msgUserNameChange = reinterpret_cast<proofps_dd::MsgUserNameChangeAndBootupDone&>(*pMsgAppData);
            msgUserNameChange.m_bCurrentClient = bCurrentClient;
            strncpy_s(msgUserNameChange.m_szUserName, nUserNameBufferLength, sUserName.c_str(), sUserName.length());

            return true;
        }  // initPkt()

        bool m_bCurrentClient; // TODO: this could be removed once every instance saves its server-side connection handle
        char m_szUserName[nUserNameBufferLength];
    };  // MsgUserNameChangeAndBootupDone
    static_assert(std::is_trivial_v<MsgUserNameChangeAndBootupDone>);
    static_assert(std::is_trivially_copyable_v<MsgUserNameChangeAndBootupDone>);
    static_assert(std::is_standard_layout_v<MsgUserNameChangeAndBootupDone>);

    // clients -> server + server self (inject)
    // MsgUserCmdFromClient messages are sent from clients to server, so server will do sg and then update all the clients with MsgUserUpdateFromServer.
    // Actually, server player also sends (injects) this to server, so server processes both server and client player inputs at the same place.
    struct MsgUserCmdFromClient
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserCmdFromClient;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            proofps_dd::Strafe strafe,
            bool bAttack,
            bool bCrouch,
            TPureFloat fPlayerAngleY,
            TPureFloat fWeaponAngleZ)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserCmdFromClient) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, 0u /*m_connHandleServerSide is ignored in this message*/);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgUserCmdFromClient));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdFromClient&>(*pMsgAppData);
            msgUserCmdMove.m_bShouldSend = false;
            msgUserCmdMove.m_strafe = strafe;
            msgUserCmdMove.m_bShootAction = bAttack;
            msgUserCmdMove.m_bCrouch = bCrouch;
            msgUserCmdMove.m_fPlayerAngleY = fPlayerAngleY;
            msgUserCmdMove.m_fWpnAngleZ = fWeaponAngleZ;

            return true;
        }

        static void setKeybd(
            pge_network::PgePacket& pkt,
            const Strafe& strafe,
            bool bJump,
            bool bSwitchToRunning,
            bool bCrouch,
            bool bRequestReload,
            unsigned char cWeaponSwitch)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_strafe = strafe;
            msgUserCmdMove.m_bJumpAction = bJump;
            msgUserCmdMove.m_bSendSwitchToRunning = bSwitchToRunning;
            msgUserCmdMove.m_bCrouch = bCrouch;
            msgUserCmdMove.m_bRequestReload = bRequestReload;
            msgUserCmdMove.m_cWeaponSwitch = cWeaponSwitch;
        }

        static unsigned char getWeaponSwitch(const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            return msgUserCmdMove.m_cWeaponSwitch;
        }

        static void SetWeaponSwitch(pge_network::PgePacket& pkt, unsigned char cTargetWpnKey)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_cWeaponSwitch = cTargetWpnKey;
        }

        static bool getReloadRequest(const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            return msgUserCmdMove.m_bRequestReload;
        }

        static void setMouse(
            pge_network::PgePacket& pkt,
            bool bShootAction)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_bShootAction = bShootAction;
        }

        static void setAngleY(
            pge_network::PgePacket& pkt,
            TPureFloat fPlayerAngleY)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_fPlayerAngleY = fPlayerAngleY;
        }

        static void setWpnAngles(
            pge_network::PgePacket& pkt,
            TPureFloat fWpnAngleZ)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_fWpnAngleZ = fWpnAngleZ;
        }

        static bool shouldSend(
            const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            return msgUserCmdMove.m_bShouldSend;
        }

        bool m_bShouldSend;
        Strafe m_strafe;                 // continuous op
        bool m_bJumpAction;
        bool m_bSendSwitchToRunning;
        bool m_bRequestReload;
        unsigned char m_cWeaponSwitch;
        bool m_bShootAction;             // continuous op
        bool m_bCrouch;                  // continuous op
        TPureFloat m_fPlayerAngleY;
        TPureFloat m_fWpnAngleZ;
    };  // struct MsgUserCmdFromClient
    static_assert(std::is_trivial_v<MsgUserCmdFromClient>);
    static_assert(std::is_trivially_copyable_v<MsgUserCmdFromClient>);
    static_assert(std::is_standard_layout_v<MsgUserCmdFromClient>);

    // server -> self (inject) and clients
    // sent regularly to all clients
    struct MsgUserUpdateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserUpdateFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const TPureFloat x,
            const TPureFloat y,
            const TPureFloat z,
            TPureFloat fPlayerAngleY,
            TPureFloat fPlayerAngleZ,
            TPureFloat fWpnAngleZ,
            float fWpnMomentaryAccuracy,
            bool bActuallyRunningOnGround,
            bool bCrouch,
            float fSomersaultAngle,
            int nArmor,
            int nHealth,
            bool bRespawn,
            int nFrags,
            int nDeaths,
            unsigned int nSuicides,
            float fFiringAccuracy,
            unsigned int nShotsFired,
            bool bInvulnerability)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgUserUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserUpdateFromServer& msgUserCmdUpdate = reinterpret_cast<proofps_dd::MsgUserUpdateFromServer&>(*pMsgAppData);
            msgUserCmdUpdate.m_pos.x = x;
            msgUserCmdUpdate.m_pos.y = y;
            msgUserCmdUpdate.m_pos.z = z;
            msgUserCmdUpdate.m_fPlayerAngleY = fPlayerAngleY;
            msgUserCmdUpdate.m_fPlayerAngleZ = fPlayerAngleZ;
            msgUserCmdUpdate.m_fWpnAngleZ = fWpnAngleZ;
            msgUserCmdUpdate.m_fWpnMomentaryAccuracy = fWpnMomentaryAccuracy;
            msgUserCmdUpdate.m_bActuallyRunningOnGround = bActuallyRunningOnGround;
            msgUserCmdUpdate.m_bCrouch = bCrouch;
            // currently this is redundant: this is the same angle as fPlayerAngleZ, however in the future they might not be always the same,
            // this is why I'm sending both now: on client-side, client must set player's angle Z to fPlayerAngleZ, and set somersault angle
            // to m_fSomersaultAngle, logically they mean different thing, but as of v0.2.2.0 they are the same.
            msgUserCmdUpdate.m_fSomersaultAngle = fSomersaultAngle;
            msgUserCmdUpdate.m_nArmor = nArmor;
            msgUserCmdUpdate.m_nHealth = nHealth;
            msgUserCmdUpdate.m_bRespawn = bRespawn;
            msgUserCmdUpdate.m_nFrags = nFrags;
            msgUserCmdUpdate.m_nDeaths = nDeaths;
            msgUserCmdUpdate.m_nSuicides = nSuicides;
            msgUserCmdUpdate.m_fFiringAccuracy = fFiringAccuracy;
            msgUserCmdUpdate.m_nShotsFired = nShotsFired;
            msgUserCmdUpdate.m_bInvulnerability = bInvulnerability;

            return true;
        }

        // important: the data members here should be kept in sync with the PgeOldNewValue data members of Player class!
        // basically what we have here should be the data evaluated by Player.isDirty() and handled in handleUserUpdateFromServer().
        TXYZ m_pos;
        TPureFloat m_fPlayerAngleY;
        TPureFloat m_fPlayerAngleZ;
        TPureFloat m_fWpnAngleZ;
        float m_fWpnMomentaryAccuracy;
        bool m_bActuallyRunningOnGround;
        bool m_bCrouch;
        float m_fSomersaultAngle;
        int m_nArmor;
        int m_nHealth;
        bool m_bRespawn;
        int m_nFrags;
        int m_nDeaths;
        unsigned int m_nSuicides;
        float m_fFiringAccuracy;
        unsigned int m_nShotsFired;
        bool m_bInvulnerability;
    };  // struct MsgUserUpdateFromServer
    static_assert(std::is_trivial_v<MsgUserUpdateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgUserUpdateFromServer>);
    static_assert(std::is_standard_layout_v<MsgUserUpdateFromServer>);

    // server -> clients
    // sent to all clients
    struct MsgBulletUpdateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::BulletUpdateFromServer;

        enum class BulletDelete : pge_network::TByte
        {
            No = 0,
            Yes,
            YesHitWall,
            YesHitPlayer
        };

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const Bullet::BulletId bulletId,
            const WeaponId weaponId,
            const TPureFloat px,
            const TPureFloat py,
            const TPureFloat pz,
            const TPureFloat ax,
            const TPureFloat ay,
            const TPureFloat az,
            const int& nDamageHp,
            const TPureFloat& damageAreaSize,
            const Bullet::DamageAreaEffect& damageAreaEffect,
            const TPureFloat& damageAreaPulse)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgBulletUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide, pge_network::PgePacket::AutoFill::NONE);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgBulletUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgBulletUpdateFromServer& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdateFromServer&>(*pMsgAppData);
            msgBulletUpdate.m_bulletId = bulletId;
            msgBulletUpdate.m_weaponId = weaponId;
            msgBulletUpdate.m_pos.x = px;
            msgBulletUpdate.m_pos.y = py;
            msgBulletUpdate.m_pos.z = pz;
            msgBulletUpdate.m_angle.x = ax;
            msgBulletUpdate.m_angle.y = ay;
            msgBulletUpdate.m_angle.z = az;
            msgBulletUpdate.m_nDamageHp = nDamageHp;
            msgBulletUpdate.m_fDamageAreaSize = damageAreaSize;
            msgBulletUpdate.m_eDamageAreaEffect = damageAreaEffect;
            msgBulletUpdate.m_fDamageAreaPulse = damageAreaPulse;
            msgBulletUpdate.m_delete = BulletDelete::No;

            return true;
        }

        static BulletDelete& getDelete(pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgBulletUpdateFromServer& msgBulletUpdate = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgBulletUpdateFromServer>(pkt);
            return msgBulletUpdate.m_delete;
        }

        // even though we have the WeaponId since v0.3.0, we still need to have some data members in this message.
        // For example, m_nDamageHp could be also retrieved on client-side from Weapon data using WeaponId, BUT
        // the damage might be modified at the moment of firing the bullet, for example, if shooter has quad damage.
        // And, even if quad damage expires in the meantime, bullet must maintain it.
        Bullet::BulletId m_bulletId;
        WeaponId m_weaponId;
        TXYZ m_pos;
        TXYZ m_angle;
        int m_nDamageHp; // v0.3.0: could deduct by WeaponId but might be different under different circumstances so keeping it now ...
        TPureFloat m_fDamageAreaSize; // v0.3.0: could deduct by WeaponId but might be different under different circumstances so keeping it now ...
        Bullet::DamageAreaEffect m_eDamageAreaEffect; // v0.3.0: could deduct by WeaponId but might be different under different circumstances so keeping it now ...
        TPureFloat m_fDamageAreaPulse; // v0.3.0: could deduct by WeaponId but might be different under different circumstances so keeping it now ...
        BulletDelete m_delete;
    };  // struct MsgBulletUpdateFromServer
    static_assert(std::is_trivial_v<MsgBulletUpdateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgBulletUpdateFromServer>);
    static_assert(std::is_standard_layout_v<MsgBulletUpdateFromServer>);

    // server -> clients
    // sent to all clients after specific event, e.g. picking up an item
    struct MsgMapItemUpdateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::MapItemUpdateFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const MapItem::MapItemId mapItemId,
            const bool bTaken)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgMapItemUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgMapItemUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgMapItemUpdateFromServer& msgMapItemUpdate = reinterpret_cast<proofps_dd::MsgMapItemUpdateFromServer&>(*pMsgAppData);
            msgMapItemUpdate.m_mapItemId = mapItemId;
            msgMapItemUpdate.m_bTaken = bTaken;

            return true;
        }

        MapItem::MapItemId m_mapItemId;
        bool m_bTaken;
    };  // struct MsgMapItemUpdateFromServer
    static_assert(std::is_trivial_v<MsgMapItemUpdateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgMapItemUpdateFromServer>);
    static_assert(std::is_standard_layout_v<MsgMapItemUpdateFromServer>);

    // server -> clients
    // availability and bullet counts update to a single client after specific events, e.g. shoot.
    // In contrast to MsgCurrentWpnUpdateFromServer, this is private data, i.e. not related to any other client.
    struct MsgWpnUpdateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::WpnUpdateFromServer;
        static const uint8_t nWpnNameNameMaxLength = 64;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const std::string& sWpnName /* eMapItemType makes it obsolete */,
            const MapItemType& eMapItemType,
            bool bAvailable,
            const Weapon::State& state,
            unsigned int nMagBulletCount,
            unsigned int nUnmagBulletCount,
            unsigned int nAmmoIncrease)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgWpnUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgWpnUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgWpnUpdateFromServer& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgWpnUpdateFromServer&>(*pMsgAppData);
            strncpy_s(msgWpnUpdate.m_szWpnName, nWpnNameNameMaxLength, sWpnName.c_str(), sWpnName.length());
            msgWpnUpdate.m_eMapItemType = eMapItemType;
            msgWpnUpdate.m_bAvailable = bAvailable;
            msgWpnUpdate.m_state = state;
            msgWpnUpdate.m_nMagBulletCount = nMagBulletCount;
            msgWpnUpdate.m_nUnmagBulletCount = nUnmagBulletCount;
            msgWpnUpdate.m_nAmmoIncrease = nAmmoIncrease;

            return true;
        }

        static bool& getAvailable(pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgWpnUpdateFromServer& msgWpnUpdate = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgWpnUpdateFromServer>(pkt);
            return msgWpnUpdate.m_bAvailable;
        }

        char m_szWpnName[nWpnNameNameMaxLength];  /* eMapItemType makes it obsolete */
        MapItemType m_eMapItemType; /* probably this should be replaced by WeaponId already! */
        bool m_bAvailable;
        Weapon::State m_state;
        unsigned int m_nMagBulletCount;
        unsigned int m_nUnmagBulletCount;
        unsigned int m_nAmmoIncrease; // it is not always easy for client to find out the actual increase so always include it too
    };  // struct MsgWpnUpdateFromServer
    static_assert(std::is_trivial_v<MsgWpnUpdateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgWpnUpdateFromServer>);
    static_assert(std::is_standard_layout_v<MsgWpnUpdateFromServer>);

    // server -> clients
    // current weapon of a specific client, sent to all clients after specific events, e.g. weapon switch, state update, etc.
    // In contrast to MsgWpnUpdateFromServer, this is public data, i.e. might related to any other client.
    // We need to send state update to all clients because only this way clients can properly audio-visualize other players' weapon state on their side.
    struct MsgCurrentWpnUpdateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::CurrentWpnUpdateFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const std::string& sWpnCurrentName,
            const Weapon::State& state)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgCurrentWpnUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgCurrentWpnUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgCurrentWpnUpdateFromServer& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgCurrentWpnUpdateFromServer&>(*pMsgAppData);
            strncpy_s(msgWpnUpdate.m_szWpnCurrentName, MsgWpnUpdateFromServer::nWpnNameNameMaxLength, sWpnCurrentName.c_str(), sWpnCurrentName.length());
            msgWpnUpdate.m_state = state;

            return true;
        }

        char m_szWpnCurrentName[MsgWpnUpdateFromServer::nWpnNameNameMaxLength];  /* should be replaced by WeaponId already! */
        Weapon::State m_state;
    };  // struct MsgCurrentWpnUpdateFromServer
    static_assert(std::is_trivial_v<MsgCurrentWpnUpdateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgCurrentWpnUpdateFromServer>);
    static_assert(std::is_standard_layout_v<MsgCurrentWpnUpdateFromServer>);

    // server -> clients
    // If someone dies, server notifies everyone about it.
    // This is required because for clients it is not straightforward to find out who killed who.
    // Although clients can recognize someone becoming dead by processing MsgUserUpdateFromServer, the killer is still unknown.
    // And this also cannot be easily found out even from MsgBulletUpdateFromServer since clients dont do collision detection with bullets.
    // So it is better if we introduce a specific message for this.
    struct MsgDeathNotificationFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::DeathNotificationFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& nDeadConnHandleServerSide,
            const pge_network::PgeNetworkConnectionHandle& nKillerConnHandleServerSide)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgDeathNotificationFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, nDeadConnHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgDeathNotificationFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgDeathNotificationFromServer& msgDeathNotify = reinterpret_cast<proofps_dd::MsgDeathNotificationFromServer&>(*pMsgAppData);
            msgDeathNotify.m_nKillerConnHandleServerSide = nKillerConnHandleServerSide;

            return true;
        }

        pge_network::PgeNetworkConnectionHandle m_nKillerConnHandleServerSide;
    };  // struct MsgDeathNotificationFromServer
    static_assert(std::is_trivial_v<MsgDeathNotificationFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgDeathNotificationFromServer>);
    static_assert(std::is_standard_layout_v<MsgDeathNotificationFromServer>);

    // server -> clients
    // This kind of message is to inform clients about Player-specific events which are less frequent than the MsgUserUpdateFromServer stuff.
    // Also, while on the long run I'm planning to send MsgUserUpdateFromServer unreliable, this MsgPlayerEventFromServer will stay reliable. 
    // This should be some kind of RPC stuff, basically I want to invoke a function of Player class on the other side, however
    // I'm not exactly sure of all details, so I just started using this with simple PlayerEventIds and we will see where it goes on the long run.
    struct MsgPlayerEventFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::PlayerEventFromServer;

        // This is weird, but I have to do like this, because I cannot include Player.h into this header, so I'm using this general data type
        // that can hold different values, Player class will know exactly what they mean.
        // Not planning to use std::optional, since Player class exactly knows what data it should expect, based on m_iPlayerEventId, I don't need to call
        // has_value() or similar stuff.
        // Also, I need to keep this struct trivial (i.e. std::is_trivial_v must be true) so it can be easily memcpy'd, I cannot do that with std::optional member.
        union UOptionalData
        {
            bool  m_bValue;
            int   m_nValue;
            float m_fValue;

            // struct stays trivial with these operator= defines, it helps a lot by allowing simple data types to be directly assigned
            // as it is done in initPkt()
            
            UOptionalData& operator=(const bool& b)
            {
                m_bValue = b;
                return *this;
            }

            UOptionalData& operator=(const int& n)
            {
                m_nValue = n;
                return *this;
            }

            UOptionalData& operator=(const float& f)
            {
                m_fValue = f;
                return *this;
            }
        };

        // With this optional template argument list, you don't even need to specify those extra parameters.
        // What I'm thinking of as an improvement is to have arbitrary number of arguments and then those are loaded into an array in the pkt
        // instead of those specific m_optData# members, but for now this is good enough!
        template<typename OptSimpleData1 = int, typename OptSimpleData2 = OptSimpleData1, typename OptSimpleData3 = OptSimpleData1>
        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const pge_network::TByte& iPlayerEventId,
            const OptSimpleData1& optData1 = {},
            const OptSimpleData2& optData2 = {}, 
            const OptSimpleData3& optData3 = {})
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgPlayerEventFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgPlayerEventFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgPlayerEventFromServer& msgPlayerEvent = reinterpret_cast<proofps_dd::MsgPlayerEventFromServer&>(*pMsgAppData);
            msgPlayerEvent.m_iPlayerEventId = iPlayerEventId;
            msgPlayerEvent.m_optData1 = optData1;
            msgPlayerEvent.m_optData2 = optData2;
            msgPlayerEvent.m_optData3 = optData3;

            return true;
        }

        pge_network::TByte m_iPlayerEventId;

        // meaning of these members is determined by m_iPlayerEventId
        UOptionalData m_optData1;
        UOptionalData m_optData2;
        UOptionalData m_optData3;
    };  // struct MsgPlayerEventFromServer
    static_assert(std::is_trivial_v<MsgPlayerEventFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgPlayerEventFromServer>);
    static_assert(std::is_standard_layout_v<MsgPlayerEventFromServer>);

    // clients -> server + server self (inject)
    // MsgUserInGameMenuCmd messages are sent from clients to server, so server will do sg and then update all the clients in different ways.
    // Actually, server player also sends (injects) this to server, so server processes both server and client player inputs at the same place.
    // Typical example: player selects the team, this message is sent to server, for which server will emit MsgPlayerEventFromServer to all clients.
    struct MsgUserInGameMenuCmd
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserInGameMenuCmd;

        template<typename OptSimpleData1 = int, typename OptSimpleData2 = OptSimpleData1, typename OptSimpleData3 = OptSimpleData1>
        static bool initPkt(
            pge_network::PgePacket& pkt,
            const int& iInGameMenu,
            const OptSimpleData1& optData1 = {},
            const OptSimpleData2& optData2 = {},
            const OptSimpleData3& optData3 = {})
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserInGameMenuCmd) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, 0u /*m_connHandleServerSide is ignored in this message*/);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgUserInGameMenuCmd));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserInGameMenuCmd& msgUserInGameMenuCmd = reinterpret_cast<proofps_dd::MsgUserInGameMenuCmd&>(*pMsgAppData);
            msgUserInGameMenuCmd.m_iInGameMenu = iInGameMenu;
            msgUserInGameMenuCmd.m_optData1 = optData1;
            msgUserInGameMenuCmd.m_optData2 = optData2;
            msgUserInGameMenuCmd.m_optData3 = optData3;

            return true;
        }

        static unsigned int getSelectedTeamId(const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const proofps_dd::MsgUserInGameMenuCmd& msgUserInGameMenuCmd = pge_network::PgePacket::getMsgAppDataFromPkt<const proofps_dd::MsgUserInGameMenuCmd>(pkt);
            return static_cast<unsigned int>(msgUserInGameMenuCmd.m_optData1.m_nValue);
        }

        static void setSelectedTeamId(pge_network::PgePacket& pkt, const unsigned int& iTeamId)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserInGameMenuCmd& msgUserInGameMenuCmd = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserInGameMenuCmd>(pkt);
            msgUserInGameMenuCmd.m_optData1 = static_cast<int>(iTeamId);
        }

        // idea is similar to as how MsgPlayerEventFromServer works
        int m_iInGameMenu; // see GUI::InGameMenuState

        // meaning of these members is determined by m_iInGameMenu
        MsgPlayerEventFromServer::UOptionalData m_optData1;
        MsgPlayerEventFromServer::UOptionalData m_optData2;
        MsgPlayerEventFromServer::UOptionalData m_optData3;
    }; // MsgUserInGameMenuCmd
    static_assert(std::is_trivial_v<MsgUserInGameMenuCmd>);
    static_assert(std::is_trivially_copyable_v<MsgUserInGameMenuCmd>);
    static_assert(std::is_standard_layout_v<MsgUserInGameMenuCmd>);

} // namespace proofps_dd
