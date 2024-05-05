/*
    ###################################################################################
    PlayerHandling.cpp
    PlayerHandling class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "PlayerHandling.h"
#include "PRooFPS-dd-packet.h"


// ############################### PUBLIC ################################


const char* proofps_dd::PlayerHandling::getLoggerModuleName()
{
    return "PlayerHandling";
}

proofps_dd::PlayerHandling::PlayerHandling(
    PGE& pge,
    proofps_dd::Durations& durations,
    proofps_dd::GUI& gui,
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    proofps_dd::Maps& maps,
    proofps_dd::Sounds& sounds) :
    proofps_dd::Networking(pge, durations),
    m_pge(pge),
    m_gui(gui),
    m_mapPlayers(mapPlayers),
    m_maps(maps),
    m_sounds(sounds)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations, gui, maps, sounds
    // But they can used in other functions.

    // Since this class is used to build up the PRooFPSddPGE class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. PRooFPSddPGE initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be existing at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
    assert(!pge.isGameRunning());
}

CConsole& proofps_dd::PlayerHandling::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}


// ############################## PROTECTED ##############################


void proofps_dd::PlayerHandling::handlePlayerDied(
    Player& player,
    PureObject3D& objXHair,
    pge_network::PgeNetworkConnectionHandle nKillerConnHandleServerSide)
{
    player.die(isMyConnection(player.getServerSideConnectionHandle()), m_pge.getNetwork().isServer());
    if (isMyConnection(player.getServerSideConnectionHandle()))
    {
        m_pge.getAudio().getAudioEngineCore().play(m_sounds.m_sndPlayerDie);
        objXHair.Hide();
        m_gui.showRespawnTimer();
    }

    if (m_pge.getNetwork().isServer())
    {
        // TODO: server should display death notification on gui here (client should display in handleDeathNotificationFromServer())

        // important: killer info is just for informational purpose so we can display it on client side, however
        // if the killer got disconnected already, conn handle will be set as same as player's connhandle, so we can still
        // show the player dieing without a killer, however it is not suicide in such case, that is why we should never
        // decrease frags based only on this value!
        pge_network::PgePacket pktDeathNotificationFromServer;
        proofps_dd::MsgDeathNotificationFromServer::initPkt(
            pktDeathNotificationFromServer,
            player.getServerSideConnectionHandle(),
            nKillerConnHandleServerSide);
        m_pge.getNetwork().getServer().sendToAllClientsExcept(pktDeathNotificationFromServer);
    }
}

void proofps_dd::PlayerHandling::handlePlayerRespawned(Player& player, PureObject3D& objXHair)
{
    const Weapon* const wpnDefaultAvailable = player.getWeaponManager().getWeaponByFilename(
        player.getWeaponManager().getDefaultAvailableWeaponFilename());
    assert(wpnDefaultAvailable);  // cannot be null since it is already verified in handleUserSetupFromServer()
    player.respawn(isMyConnection(player.getServerSideConnectionHandle()), *wpnDefaultAvailable, m_pge.getNetwork().isServer());

    if (isMyConnection(player.getServerSideConnectionHandle()))
    {
        // camera must be repositioned immediately so players can see themselves ASAP
        auto& camera = m_pge.getPure().getCamera();
        camera.getPosVec().SetX(player.getObject3D()->getPosVec().getX());
        camera.getPosVec().SetY(player.getObject3D()->getPosVec().getY());
        camera.getTargetVec().SetX(camera.getPosVec().getX());
        camera.getTargetVec().SetY(camera.getPosVec().getY());

        objXHair.Show();
        m_gui.hideRespawnTimer();
    }
}

void proofps_dd::PlayerHandling::serverRespawnPlayer(Player& player, bool restartGame, const proofps_dd::Config& config)
{
    // to respawn, we just need to set these values, because SendUserUpdates() will automatically send out changes to everyone
    player.getPos() = m_maps.getRandomSpawnpoint();
    player.setHealth(100);
    player.getRespawnFlag() = true;
    player.setInvulnerability(true, config.getPlayerRespawnInvulnerabilityDelaySeconds());
    if (restartGame)
    {
        player.getFrags() = 0;
        player.getDeaths() = 0;
    }
}

void proofps_dd::PlayerHandling::serverUpdateRespawnTimers(
    const proofps_dd::Config& config,
    proofps_dd::GameMode& gameMode,
    proofps_dd::Durations& durations)
{
    if (gameMode.checkWinningConditions())
    {
        return;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        const auto& playerConst = player;

        if (playerConst.getHealth() > 0)
        {
            continue;
        }

        const long long timeDiffSeconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - playerConst.getTimeDied()).count();
        if (static_cast<unsigned long long>(timeDiffSeconds) >= config.getPlayerRespawnDelaySeconds())
        {
            serverRespawnPlayer(player, false, config);
        }
    }

    durations.m_nUpdateRespawnTimersDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
} // serverUpdateRespawnTimers()

void proofps_dd::PlayerHandling::updatePlayersOldValues()
{
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        // From v0.1.5 clients invoke updateOldValues() in handleUserUpdateFromServer() thus they are also good to use old vs new values and isDirty().
        // Server invokes it here in every physics iteration. 2 reasons:
        // - consecutive physics iterations require old and new values to be properly set;
        // - player.isNetDirty() thus serverSendUserUpdates() rely on player.updateOldValues().
        player.updateOldValues();
    }
}

void proofps_dd::PlayerHandling::writePlayerList()
{
    getConsole().OLnOI("PRooFPSddPGE::%s()", __func__);
    for (const auto& playerPair : m_mapPlayers)
    {
        getConsole().OLn("Username: %s; connHandleServerSide: %u; address: %s",
            playerPair.second.getName().c_str(), playerPair.second.getServerSideConnectionHandle(), playerPair.second.getIpAddress().c_str());
    }
    getConsole().OO();
}

bool proofps_dd::PlayerHandling::handleUserConnected(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const pge_network::MsgUserConnectedServerSelf& msg,
    PGEcfgProfiles& cfgProfiles,
    proofps_dd::Config& config,
    proofps_dd::GameMode& gameMode,
    std::function<void(int)>& cbDisplayMapLoadingProgressUpdate)
{
    if (!m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): client received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    pge_network::PgePacket newPktUserUpdate;

    if (msg.m_bCurrentClient)
    {
        // server is processing its own birth
        if (m_mapPlayers.size() == 0)
        {
            // server already loads the map for itself at this point
            if (m_maps.getFilename() != m_maps.getNextMapToBeLoaded())
            {
                // if we fall here with non-empty m_maps.getFilename(), it is an error, and m_maps.load() will fail as expected.
                if (!m_maps.load(m_maps.getNextMapToBeLoaded().c_str(), cbDisplayMapLoadingProgressUpdate))
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): m_maps.load() failed: %s!", __func__, m_maps.getNextMapToBeLoaded().c_str());
                    assert(false);
                    return false;
                }
            }
            else
            {
                getConsole().OLn("PRooFPSddPGE::%s(): map %s already loaded", __func__, m_maps.getNextMapToBeLoaded().c_str());
            }

            getConsole().OLn("PRooFPSddPGE::%s(): first (local) user connected and I'm server, so this is me (connHandleServerSide: %u)",
                __func__, connHandleServerSide);

            pge_network::PgePacket newPktSetup;
            if (proofps_dd::MsgUserSetupFromServer::initPkt(newPktSetup, connHandleServerSide, true, msg.m_szIpAddress, m_maps.getNextMapToBeLoaded().c_str()))
            {
                const PureVector& vecStartPos = cfgProfiles.getVars()["testing"].getAsBool() ?
                    m_maps.getLeftMostSpawnpoint() :
                    m_maps.getRandomSpawnpoint();

                if (proofps_dd::MsgUserUpdateFromServer::initPkt(
                    newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, 0.f, false, 0.f, 100, false, 0, 0, false))
                {
                    // server injects this msg to self so resources for player will be allocated upon processing these
                    m_pge.getNetwork().getServer().send(newPktSetup);
                    m_pge.getNetwork().getServer().send(newPktUserUpdate);
                }
                else
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                    assert(false);
                    return false;
                }
            }
            else
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
                return false;
            }
        }
        else
        {
            // cannot happen
            getConsole().EOLn("PRooFPSddPGE::%s(): user (connHandleServerSide: %u) connected with bCurrentClient as true but it is not me, CANNOT HAPPEN!",
                __func__, connHandleServerSide);
            assert(false);
            return false;
        }
    }
    else
    {
        if (connHandleServerSide == pge_network::ServerConnHandle)
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): server user (connHandleServerSide: %u) connected but map m_bCurrentClient is false, CANNOT HAPPEN!",
                __func__, connHandleServerSide);
            assert(false);
            return false;
        }
        // server is processing another user's birth
        getConsole().OLn("PRooFPSddPGE::%s(): new remote user (connHandleServerSide: %u) connected (from %s) and I'm server",
            __func__, connHandleServerSide, msg.m_szIpAddress);

        pge_network::PgePacket newPktSetup;
        if (!proofps_dd::MsgUserSetupFromServer::initPkt(newPktSetup, connHandleServerSide, false, msg.m_szIpAddress, m_maps.getFilename()))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }

        const PureVector& vecStartPos = cfgProfiles.getVars()["testing"].getAsBool() ?
            m_maps.getRightMostSpawnpoint() :
            m_maps.getRandomSpawnpoint();

        if (!proofps_dd::MsgUserUpdateFromServer::initPkt(
            newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, 0.f, false, 0.f, 100, false, 0, 0, true /* invulnerable by default */))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }

        // server injects this msg to self so resources for player will be allocated upon processing these
        m_pge.getNetwork().getServer().send(newPktSetup);
        m_pge.getNetwork().getServer().send(newPktUserUpdate);

        // inform all other clients about this new user
        m_pge.getNetwork().getServer().sendToAllClientsExcept(newPktSetup, connHandleServerSide);
        m_pge.getNetwork().getServer().sendToAllClientsExcept(newPktUserUpdate, connHandleServerSide);

        // now we send this msg to the client with this bool flag set so client will know it is their connect
        proofps_dd::MsgUserSetupFromServer& msgUserSetup = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserSetupFromServer>(newPktSetup);
        msgUserSetup.m_bCurrentClient = true;
        m_pge.getNetwork().getServer().send(newPktSetup, connHandleServerSide);
        m_pge.getNetwork().getServer().send(newPktUserUpdate, connHandleServerSide);

        pge_network::PgePacket newPktServerInfo;
        // In the future we need something better than GameMode not having some funcs like getFragLimit()
        const proofps_dd::DeathMatchMode* const pDeathMatchMode = dynamic_cast<proofps_dd::DeathMatchMode*>(&gameMode);
        if (!pDeathMatchMode)
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): cast FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }
        if (!proofps_dd::MsgServerInfoFromServer::initPkt(
            newPktServerInfo,
            cfgProfiles.getVars()[CVAR_FPS_MAX].getAsUInt(),
            config.getTickRate(),
            config.getPhysicsRate(),
            config.getClientUpdateRate(),
            gameMode.getGameModeType(),
            pDeathMatchMode->getFragLimit(),
            pDeathMatchMode->getTimeLimitSecs(),
            pDeathMatchMode->getTimeRemainingSecs(),
            config.getPlayerRespawnDelaySeconds(),
            config.getPlayerRespawnInvulnerabilityDelaySeconds()))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }
        m_pge.getNetwork().getServer().send(newPktServerInfo, connHandleServerSide);
    }

    return true;
}  // handleUserConnected()

bool proofps_dd::PlayerHandling::handleUserDisconnected(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const pge_network::MsgUserDisconnectedFromServer&,
    proofps_dd::GameMode& gameMode)
{
    const auto playerIt = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == playerIt)
    {
        // TEMPORARILY COMMENTED DUE TO: https://github.com/proof88/PRooFPS-dd/issues/261
        // When we are trying to join a server but we get bored and user presses ESCAPE, client's disconnect is invoked, which
        // actually starts disconnecting because it thinks we are connected to server, and injects this userDisconnected pkt.
        // 
        //getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        //assert(false); // in debug mode, try to understand this scenario
        return true; // in release mode, dont terminate
    }

    // Server should not remove all players when it it notified with its connHandle being disconnected, because in that case
    // all players are expected to be removed by subsequent calls into this function with their connHandle as their connection state transitions.
    // There will be userDisconnected message for all players, including the server as well, so eventually this way m_mapPlayers will be
    // cleared out in multiple steps on server-side.
    // However, client won't be notified about all clients disconnecting in this case, but it is quite trivial when the server itself disconnects.
    // So that is why we manually get rid of all players in case of client.
    // We need m_mapPlayers to be cleared out by the end of processing all disconnections, the reasion is explained in hasValidConnection().
    const bool bClientShouldRemoveAllPlayers = !m_pge.getNetwork().isServer() && (connHandleServerSide == pge_network::ServerConnHandle);
    if (m_pge.getNetwork().isServer())
    {
        getConsole().OLn(
            "PRooFPSddPGE::%s(): user %s with connHandleServerSide %u disconnected and I'm server",
            __func__, playerIt->second.getName().c_str(), connHandleServerSide);
    }
    else
    {
        getConsole().OLn(
            "PRooFPSddPGE::%s(): user %s with connHandleServerSide %u disconnected and I'm client",
            __func__, playerIt->second.getName().c_str(), connHandleServerSide);
    }

    gameMode.removePlayer(playerIt->second);
    m_mapPlayers.erase(playerIt);

    if (bClientShouldRemoveAllPlayers)
    {
        getConsole().OLn("PRooFPSddPGE::%s(): it was actually the server disconnected so I'm removing every player including myself", __func__);
        gameMode.restart();
        m_mapPlayers.clear();
    }

    m_pge.getNetwork().WriteList();
    writePlayerList();

    return true;
}  // handleUserDisconnected()

bool proofps_dd::PlayerHandling::handleUserNameChange(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const proofps_dd::MsgUserNameChangeAndBootupDone& msg,
    proofps_dd::Config& config,
    proofps_dd::GameMode& gameMode,
    PGEcfgProfiles& cfgProfiles)
{
    // TODO: make sure received user name is properly null-terminated! someone else could had sent that, e.g. malicious client or server

    const auto playerIt = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == playerIt)
    {
        getConsole().EOLn("PlayerHandling::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return true;
    }

    if (m_pge.getNetwork().isServer())
    {
        // sanity check: connHandle should be server's if bCurrentClient is set
        if ((connHandleServerSide == pge_network::ServerConnHandle) && (!msg.m_bCurrentClient))
        {
            getConsole().EOLn("PlayerHandling::%s(): cannot happen: connHandleServerSide != pge_network::ServerConnHandle: %u != %u, programming error!",
                __func__, connHandleServerSide, pge_network::ServerConnHandle);
            assert(false);
            return false;
        }

        // this is a requested name so this is the place where we make sure name is unique!
        char szNewUserName[sizeof(msg.m_szUserName)];
        Player::genUniqueUserName(szNewUserName, msg.m_szUserName, m_mapPlayers);

        if (strncmp(szNewUserName, msg.m_szUserName, sizeof(msg.m_szUserName)) == 0)
        {
            getConsole().OLn("PlayerHandling::%s(): name change request accepted for connHandleServerSide: %u, old name: %s, new name: %s!",
                __func__, connHandleServerSide, playerIt->second.getName().c_str(), szNewUserName);
        }
        else
        {
            getConsole().OLn("PlayerHandling::%s(): name change request denied for connHandleServerSide: %u, old name: %s, requested: %s, new name: %s!",
                __func__, connHandleServerSide, playerIt->second.getName().c_str(), msg.m_szUserName, szNewUserName);
        }

        // server updates player's name first

        playerIt->second.setName(szNewUserName);
        // TODO: these commented lines below will be needed when we are allowing player name change WHILE already connected to the server
        // TODO: check if such name is already present in frag table, if so, then rename
        //if (!gameMode.renamePlayer(playerIt->second.getName().c_str(), szNewUserName))
        //{
        //    getConsole().EOLn("PlayerHandling::%s(): gameMode.renamePlayer() FAILED!", __func__);
        //    assert(false);
        //    return false;
        //}
        if (!gameMode.addPlayer(playerIt->second))
        {
            getConsole().EOLn("PlayerHandling::%s(): failed to insert player %s (%u) into GameMode!", __func__, szNewUserName, connHandleServerSide);
            assert(false);
            return false;
        }

        // then we let all clients except this one know about the name change
        pge_network::PgePacket newPktUserNameChange;
        if (!proofps_dd::MsgUserNameChangeAndBootupDone::initPkt(newPktUserNameChange, connHandleServerSide, false, szNewUserName))
        {
            getConsole().EOLn("PlayerHandling::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }
        m_pge.getNetwork().getServer().sendToAllClientsExcept(newPktUserNameChange, connHandleServerSide);

        if (connHandleServerSide != pge_network::ServerConnHandle)
        {
            m_pge.getNetwork().getServer().setDebugNickname(connHandleServerSide, szNewUserName);
            // we also let this one know its own name change (only if this is not server)
            proofps_dd::MsgUserNameChangeAndBootupDone& msgUserNameChange = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserNameChangeAndBootupDone>(newPktUserNameChange);
            msgUserNameChange.m_bCurrentClient = true;
            m_pge.getNetwork().getServer().send(newPktUserNameChange, connHandleServerSide);
        }

        if (msg.m_bCurrentClient)
        {
            m_gui.textPermanent("Server, User name: " + std::string(szNewUserName) +
                (cfgProfiles.getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
                10, 30);
        }

        // from our point of view, client player has now fully booted up so NOW we are arming the respawn invulnerability
        getConsole().EOLn("PlayerHandling::%s(): arming 1st-spawn invulnerability for connHandleServerSide: %u!", __func__, connHandleServerSide);
        playerIt->second.setInvulnerability(true, config.getPlayerRespawnInvulnerabilityDelaySeconds());
    }
    else
    {
        // if we are client, we MUST NOT receive empty user name from server, so in such case just terminate because there is something fishy!
        if (strnlen(msg.m_szUserName, sizeof(msg.m_szUserName)) == 0)
        {
            getConsole().EOLn("PlayerHandling::%s(): cannot happen: connHandleServerSide: %u, received empty user name from server!",
                __func__, connHandleServerSide);
            assert(false);  // in debug mode, raise the debugger
            return false;   // for release mode
        }


        getConsole().OLn("PlayerHandling::%s(): accepting new name from server for connHandleServerSide: %u (%s), old name: %s, new name: %s!",
            __func__, connHandleServerSide, msg.m_bCurrentClient ? "me" : "not me", playerIt->second.getName().c_str(), msg.m_szUserName);

        playerIt->second.setName(msg.m_szUserName);
        // TODO: these commented lines below will be needed when we are allowing player name change WHILE already connected to the server
        // TODO: check if such name is already present in frag table, if so, then rename
        //if (!gameMode.renamePlayer(playerIt->second.getName(), msg.m_szUserName))
        //{
        //    getConsole().EOLn("PlayerHandling::%s(): gameMode.renamePlayer() FAILED!", __func__);
        //    assert(false);
        //    return false;
        //}
        if (!gameMode.addPlayer(playerIt->second))
        {
            getConsole().EOLn("PlayerHandling::%s(): failed to insert player %s (%u) into GameMode!", __func__, msg.m_szUserName, connHandleServerSide);
            assert(false);
            return false;
        }

        if (msg.m_bCurrentClient)
        {
            // due to difficulties caused by m_gui.textPermanent() it is easier to use it here than in handleUserSetupFromServer()
            m_gui.textPermanent("Client, User name: " + playerIt->second.getName() + "; IP: " + playerIt->second.getIpAddress() +
                (cfgProfiles.getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
                10, 30);
        }
    }

    m_pge.getNetwork().WriteList();
    writePlayerList();

    return true;

}  // handleUserNameChange()

void proofps_dd::PlayerHandling::resetSendClientUpdatesCounter(proofps_dd::Config& config)
{
    // Config::validate() makes sure neither getTickRate() nor getClientUpdateRate() return 0
    m_nSendClientUpdatesInEveryNthTick = config.getTickRate() / config.getClientUpdateRate();
    m_nSendClientUpdatesCntr = m_nSendClientUpdatesInEveryNthTick;
}

void proofps_dd::PlayerHandling::serverSendUserUpdates(proofps_dd::Durations& durations)
{
    if (!m_pge.getNetwork().isServer())
    {
        // should not happen, but we log it anyway, if in future we might mess up something during a refactor ...
        getConsole().EOLn("PRooFPSddPGE::%s(): NOT server!", __func__);
        assert(false);
        return;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
    const bool bSendUserUpdates = (m_nSendClientUpdatesCntr == m_nSendClientUpdatesInEveryNthTick);

    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        const auto& playerConst = player;

        if (bSendUserUpdates && player.isNetDirty())
        {
            pge_network::PgePacket newPktUserUpdate;
            //getConsole().EOLn("PRooFPSddPGE::%s(): send 1!", __func__);
            if (proofps_dd::MsgUserUpdateFromServer::initPkt(
                newPktUserUpdate,
                playerPair.second.getServerSideConnectionHandle(),
                playerConst.getPos().getNew().getX(),
                playerConst.getPos().getNew().getY(),
                playerConst.getPos().getNew().getZ(),
                playerConst.getAngleY(),
                playerConst.getAngleZ(),
                player.getWeaponAngle().getNew().getZ(),
                player.getCrouchStateCurrent(),
                player.getSomersaultAngle(),
                playerConst.getHealth().getNew(),
                player.getRespawnFlag(),
                playerConst.getFrags(),
                playerConst.getDeaths(),
                playerConst.getInvulnerability()))
            {
                player.clearNetDirty();

                // we always reset respawn flag here
                playerPair.second.getRespawnFlag() = false;

                // Note that health is not needed by server since it already has the updated health, but for convenience
                // we put that into MsgUserUpdateFromServer and send anyway like all the other stuff.
                m_pge.getNetwork().getServer().sendToAll(newPktUserUpdate);
                //getConsole().EOLn("PRooFPSddPGE::%s(): send 2, invul: %b!", __func__, playerConst.getInvulnerability());
            }
            else
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
            }
        }
    }  // for playerPair

    if (bSendUserUpdates)
    {
        m_nSendClientUpdatesCntr = 0;
        // measure duration only if we really sent the user updates to clients
        durations.m_nSendUserUpdatesDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
    }

    ++m_nSendClientUpdatesCntr;
} // serverSendUserUpdates()

bool proofps_dd::PlayerHandling::handleUserUpdateFromServer(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const proofps_dd::MsgUserUpdateFromServer& msg,
    PureObject3D& objXHair,
    const proofps_dd::Config& /*config*/,
    proofps_dd::GameMode& gameMode)
{
    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return true;  // might NOT be fatal error in some circumstances, although I cannot think about any, but dont terminate the app for this ...
    }

    auto& player = it->second;
    const auto& playerConst = player;
    //getConsole().OLn("PRooFPSddPGE::%s(): user %s received MsgUserUpdateFromServer: %f", __func__, player.getName().c_str(), msg.m_pos.x);

    const bool bOriginalExpectingStartPos = player.isExpectingStartPos();
    if (player.isExpectingStartPos())
    {
        // Player object is recently constructed and this is the 1st MsgUserUpdateFromServer for this Player

        player.setExpectingStartPos(false);
        // PPPKKKGGGGGG
        player.getPos().set(PureVector(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z));
        player.getPos().commit(); // both server and client commits in this case

        if (m_pge.getNetwork().isServer())
        {
            // When Player is spawned for the 1st time, it is not a "re-"spawn, but we should still apply invulnerability since
            // this player has not yet fully booted up but gets visible to other players so we keep protected from other players.
            // I could not find a better place for this (MsgUserConnected, MsgUserSetupFromServer, etc.), so here we are forcing it.
            getConsole().EOLn("PRooFPSddPGE::%s(): 1st spawn: forced invulnerability for connHandleServerSide: %u!", __func__, connHandleServerSide);
            player.setInvulnerability(true, 999);
        }
    }
    else
    {
        if (!m_pge.getNetwork().isServer() && (msg.m_bInvulnerability != player.getInvulnerability()))
        {
            // only clients should fall here, server sets invulnerability in other locations and doesnt need to update itself here!
            
            getConsole().EOLn("PRooFPSddPGE::%s(): new invulnerability state %b for connHandleServerSide: %u!", __func__, msg.m_bInvulnerability, connHandleServerSide);
            // no need to set time for clients even if state is true, since player.update() is not allowed to stop invulnerability on client-side.
            player.setInvulnerability(msg.m_bInvulnerability);
        }
    }

    // server has already set this in input handling and/or physics, however probably this is still faster than with condition: if (!m_pge.getNetwork().isServer())
    player.setSomersaultClient(msg.m_fSomersaultAngle);

    //getConsole().OLn("PRooFPSddPGE::%s(): rcvd crouch: %b", __func__, msg.m_bCrouch);
    if (msg.m_bCrouch)
    {
        // server had already set stuff since it relayed this to clients, however
        // there is no use of adding extra condition for checking if we are server or client
        player.doCrouchShared();
    }
    else
    {
        player.doStandupShared();
    }

    player.getPos().set(PureVector(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z)); // server does not commit here, client commits few lines below by invoking updateOldValues()
    player.getObject3D()->getPosVec().Set(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);
    player.getWeaponManager().getCurrentWeapon()->UpdatePosition(player.getObject3D()->getPosVec(), player.isSomersaulting());

    if (msg.m_fPlayerAngleY != -1.f)
    {
        //player.getAngleY() = msg.m_fPlayerAngleY;  // not sure why this is commented
        player.getObject3D()->getAngleVec().SetY(msg.m_fPlayerAngleY);
    }
    player.getObject3D()->getAngleVec().SetZ(msg.m_fPlayerAngleZ);

    player.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().SetY(player.getObject3D()->getAngleVec().getY());
    player.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().SetZ(msg.m_fWpnAngleZ);

    player.getFrags() = msg.m_nFrags;
    player.getDeaths() = msg.m_nDeaths;

    //getConsole().OLn("PRooFPSddPGE::%s(): rcvd health: %d, health: %d, old health: %d",
    //    __func__, msg.m_nHealth, player.getHealth(), player.getHealth().getOld());
    player.setHealth(msg.m_nHealth);

    if (msg.m_bRespawn)
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): player %s has respawned!", __func__, player.getName().c_str());
        handlePlayerRespawned(player, objXHair);
    }
    else
    {
        if ((playerConst.getHealth().getOld() > 0) && (playerConst.getHealth() == 0))
        {
            // only clients fall here, since server already set oldhealth to 0 at the beginning of this frame
            // because it had already set health to 0 in previous frame
            //getConsole().OLn("PRooFPSddPGE::%s(): player %s has died!", __func__, player.getName().c_str());
            handlePlayerDied(player, objXHair, player.getServerSideConnectionHandle() /* ignored by client anyway */);

            // TODO: until v0.2.0.0 this was the only location where client could figure out if any player died, however
            // now we have handleDeathNotificationFromServer(), we could simply move this code to there!
            // Client does not invoke HandlePlayerDied() anywhere else.
        }
    }

    // the only situation when game mode does not contain the player but we already receive update for the player is
    // when isExpectingStartPos() is true, because the userNameChange will be handled a bit later;
    // note that it can also happen that we receive update here for a player who has not yet handshaked its name
    // with the server, in that case the name is empty, that is why we also need to check emptiness!
    if (!player.getName().empty() && !bOriginalExpectingStartPos && !gameMode.updatePlayer(player))
    {
        getConsole().EOLn("%s: failed to update player %s in GameMode!", __func__, player.getName().c_str());
    }

    if (!m_pge.getNetwork().isServer())
    {
        // server already invoked updateOldValues() when it sent out this update message
        player.updateOldValues();
    }

    return true;
}  // handleUserUpdateFromServer()

bool proofps_dd::PlayerHandling::handleDeathNotificationFromServer(pge_network::PgeNetworkConnectionHandle nDeadConnHandleServerSide, const proofps_dd::MsgDeathNotificationFromServer& msg)
{
    if (m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("PlayerHandling::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    //getConsole().EOLn("PlayerHandling::%s(): player %u killed by %u",  __func__, nDeadConnHandleServerSide, msg.m_nKillerConnHandleServerSide);

    const auto itPlayerDied = m_mapPlayers.find(nDeadConnHandleServerSide);
    if (m_mapPlayers.end() == itPlayerDied)
    {
        getConsole().EOLn("PlayerHandling::%s(): failed to find dead with connHandleServerSide: %u!", __func__, nDeadConnHandleServerSide);
        assert(false);
        return false; // crash in both debug and release mode: if someone dies on server side, must still exist at that moment in every network instance
    }

    const auto itPlayerKiller = m_mapPlayers.find(msg.m_nKillerConnHandleServerSide);
    if (m_mapPlayers.end() == itPlayerKiller)
    {
        getConsole().EOLn("PlayerHandling::%s(): failed to find killer with connHandleServerSide: %u!", __func__, msg.m_nKillerConnHandleServerSide);
        assert(false); // crash in debug, ignore in release mode: a bullet killing someone might be shot by a killer already disconnected before the impact,
        // however this is not likely to happen in debug mode when I'm testing regular gameplay!
    }

    // Server does death notification on GUI in HandlePlayerDied(), clients do here.
    // TODO: add notification to GUI!

    return true;
}

void proofps_dd::PlayerHandling::updatePlayers(const proofps_dd::Config& config)
{
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        player.update(config, m_pge.getNetwork().isServer());
    }
}


// ############################### PRIVATE ###############################
