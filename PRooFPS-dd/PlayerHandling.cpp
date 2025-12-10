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
#include "Physics.h"
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


bool proofps_dd::PlayerHandling::hasPlayerBootedUp(const pge_network::PgeNetworkConnectionHandle& connHandle) const
{
    const auto myPlayerIt = m_mapPlayers.find(connHandle);
    if (m_mapPlayers.end() == myPlayerIt)
    {
        return false;
    }

    return myPlayerIt->second.hasBootedUp() /* means I have already received MY MsgUserNameChangeAndBootupDone */;
}

void proofps_dd::PlayerHandling::handlePlayerDied(
    Player& player,
    XHair& xhair,
    const pge_network::PgeNetworkConnectionHandle& nKillerConnHandleServerSide)
{
    // here design is good because server and client share the same code
    player.die(isMyConnection(player.getServerSideConnectionHandle()), m_pge.getNetwork().isServer());
    
    // TODO: other player-related sounds are played by Player instance, e.g. Player::handleLanded(), this should be consolidated in the future
    m_pge.getAudio().play3dSound(m_sounds.m_sndPlayerDie, player.getPos().getNew());
    //getConsole().EOLn("PlayerHandling::%s() playing die sound", __func__);

    if (isMyConnection(player.getServerSideConnectionHandle()))
    {
        xhair.hide();
    }

    // design was good until this line, in the future we should change MsgDeathNotificationFromServer to be injected also to server, thus
    // the same message handling would do everything for both server and client
    // !!! BADDESIGN !!!
    if (m_pge.getNetwork().isServer())
    {
        // important: killer info is just for informational purpose so we can display it, however
        // if the killer got disconnected already, conn handle is set as same as player's connhandle, so we can still
        // show the player dieing without a killer, however it is not suicide in such case, that is why we should never
        // decrease frags based only on this value!

        // server displays death notification on gui here, client displays in handleDeathNotificationFromServer() as we send the pkt out below
        assert(m_gui.getDeathKillEvents());
        std::string sKillerName;
        unsigned int iKillerTeamId = 0;
        const Player* pPlayerKiller = nullptr;
        if (nKillerConnHandleServerSide != player.getServerSideConnectionHandle())
        {
            const auto itPlayerKiller = m_mapPlayers.find(nKillerConnHandleServerSide);
            if (m_mapPlayers.end() != itPlayerKiller)
            {
                pPlayerKiller = &(itPlayerKiller->second);
                sKillerName = pPlayerKiller->getName();
                iKillerTeamId = pPlayerKiller->getTeamId();
            }
        }
        m_gui.getDeathKillEvents()->addDeathKillEvent(
            sKillerName,
            GUI::getImVec4fromPureColor( TeamDeathMatchMode::getTeamColor( iKillerTeamId ) ),
            player.getName(),
            GUI::getImVec4fromPureColor( TeamDeathMatchMode::getTeamColor( player.getTeamId() ) ));

        pge_network::PgePacket pktDeathNotificationFromServer;
        proofps_dd::MsgDeathNotificationFromServer::initPkt(
            pktDeathNotificationFromServer,
            player.getServerSideConnectionHandle(),
            nKillerConnHandleServerSide);
        m_pge.getNetwork().getServer().sendToAllClientsExcept(pktDeathNotificationFromServer);

        // from v0.2.5, server shows respawn timer here for themselves, client shows upon receiving MsgDeathNotificationFromServer
        if (isMyConnection(player.getServerSideConnectionHandle()))
        {
            m_gui.showRespawnTimer(pPlayerKiller);
        }
    }
}

void proofps_dd::PlayerHandling::handlePlayerRespawned(
    Player& player,
    XHair& xhair)
{
    // Remember this function is NOT invoked when just connected to a new game!
    // For setting initial stuff after connect, use handleUserUpdateFromServer(), where it checks for isJustCreatedAndExpectingStartPos()!
    
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

        xhair.show();
        xhair.handleMagLoaded();
        m_gui.hideRespawnTimer();
        m_gui.hideGameObjectives(); // just in case player was checking it during respawn countdown, OR game just restarted
        m_gui.getMinimap()->show(); // even though we dont hide it when player dies, game restart eventually respawns players so we need to show because game end hides it
    }
}

void proofps_dd::PlayerHandling::serverRespawnPlayer(Player& player, bool restartGame, const proofps_dd::Config& config)
{
    // to respawn, we just need to set these values, because SendUserUpdates() will automatically send out changes to everyone
    player.getPos() = m_maps.getRandomSpawnpoint(GameMode::getGameMode()->isTeamBasedGame(), player.getTeamId());
    player.setHealth(100);
    player.getRespawnFlag() = true;
    player.setInvulnerability(true, config.getPlayerRespawnInvulnerabilityDelaySeconds());
    if (restartGame)
    {
        player.getFrags() = 0;
        player.getDeaths() = 0;
        player.getSuicides() = 0;
        player.getFiringAccuracy() = 0.f;
        player.getShotsFiredCount() = 0;
        player.getShotsHitTarget() = 0;
    }
}

void proofps_dd::PlayerHandling::serverUpdateRespawnTimers(
    const proofps_dd::Config& config,
    proofps_dd::GameMode& gameMode,
    proofps_dd::Durations& durations)
{
    if (gameMode.isGameWon())
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

        const auto timeDiffSeconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - playerConst.getTimeDied()).count();
        if (timeDiffSeconds >= static_cast<std::chrono::seconds::rep>(config.getPlayerRespawnDelaySeconds()))
        {
            serverRespawnPlayer(player, false, config);
        }
    }

    durations.m_nUpdateRespawnTimersDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
} // serverUpdateRespawnTimers()

void proofps_dd::PlayerHandling::handlePlayerTeamIdChanged(
    Player& player,
    const unsigned int& iTeamId,
    const proofps_dd::Config& config,
    PGEcfgProfiles& cfgProfiles)
{
    // both server and client comes here

    assert(m_gui.getXHair());

    const auto iPrevTeamId = player.getTeamId();

    // need to trigger updating player teamId here because that is needed for serverRespawnPlayer() to work properly
    player.handleTeamIdChanged(iTeamId);
    player.getObject3D()->getMaterial(false).getTextureEnvColor() = TeamDeathMatchMode::getTeamColor(iTeamId);

    if (m_pge.getNetwork().isServer())
    {
        if (iPrevTeamId == 0u /* i.e. the 1st team selection right after connecting to server */)
        {
            // even tho player is already on a random global spawn point selected in handleUserConnected(), now
            // with proper team id respawn is needed to deal with team spawn groups;
            // but do this only if NOT regression test is running because it messes with positioning players to the leftmost/rightmost spawnpoints!
            if (!cfgProfiles.getVars()["testing"].getAsBool())
            {
                serverRespawnPlayer(player, false, config);
            }
        }
        else
        {
            // any consecutive team changes shall kill the player
            handlePlayerDied(player, *m_gui.getXHair(), player.getServerSideConnectionHandle());
        }
    }

    // suppress such events if we are still not fully loaded and connected
    if (hasPlayerBootedUp(getMyServerSideConnectionHandle()))
    {
        m_gui.getServerEvents()->addTeamChangedEvent(
            player.getName(),
            GUI::getImVec4fromPureColor(TeamDeathMatchMode::getTeamColor(iPrevTeamId)),
            iTeamId,
            GUI::getImVec4fromPureColor(TeamDeathMatchMode::getTeamColor(iTeamId)));
    }
}

void proofps_dd::PlayerHandling::handleExplosionMultiKill(
    int nPlayersDiedByExplosion)
{
    // both server and client comes here

    // suppress such events if we are still not fully loaded and connected
    if (hasPlayerBootedUp(getMyServerSideConnectionHandle()))
    {
        m_gui.getServerEvents()->addExplosionMultiKillEvent(nPlayersDiedByExplosion);
        m_gui.getSlidingProof88Laugh().show(m_pge.getAudio());
        //getConsole().EOLn("PlayerHandling::%s() nPlayersDiedByExplosion: %d", __func__, nPlayersDiedByExplosion);
    }
}

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
    getConsole().OLnOI("PlayerHandling::%s()", __func__);
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
    std::function<void(int)>& cbDisplayMapLoadingProgressUpdate)
{
    if (!m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("PlayerHandling::%s(): client received, CANNOT HAPPEN!", __func__);
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
                // save it just for error log, because upon error even getNextMapToBeLoaded() will be reset
                const std::string sNextMapToBeLoaded = m_maps.getNextMapToBeLoaded();
                // if we fall here with non-empty m_maps.getFilename(), it is an error, and m_maps.load() will fail as expected.
                if (!m_maps.load(m_maps.getNextMapToBeLoaded().c_str(), cbDisplayMapLoadingProgressUpdate))
                {
                    getConsole().EOLn("PlayerHandling::%s(): m_maps.load() failed: %s!", __func__, sNextMapToBeLoaded.c_str());
                    assert(false);
                    return false;
                }
            }
            else
            {
                getConsole().OLn("PlayerHandling::%s(): map %s already loaded", __func__, m_maps.getNextMapToBeLoaded().c_str());
            }

            getConsole().OLn("PlayerHandling::%s(): first (local) user connected and I'm server, so this is me (connHandleServerSide: %u)",
                __func__, connHandleServerSide);

            pge_network::PgePacket newPktSetup;
            if (proofps_dd::MsgUserSetupFromServer::initPkt(newPktSetup, connHandleServerSide, true, msg.m_szIpAddress, m_maps.getNextMapToBeLoaded().c_str()))
            {
                const PureVector& vecStartPos = cfgProfiles.getVars()["testing"].getAsBool() ?
                    m_maps.getLeftMostSpawnpoint() :
                    m_maps.getRandomSpawnpoint(GameMode::getGameMode()->isTeamBasedGame(), 0);

                if (proofps_dd::MsgUserUpdateFromServer::initPkt(
                    newPktUserUpdate,
                    connHandleServerSide,
                    vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(),
                    0.f /* player angle Y */, 0.f /* player angle Z */,
                    0.f /* weapon angle Z */,
                    0.f /* weapon momentary accuracy */,
                    false /* bActuallyRunningOnGround*/, false /* bCrouch */, 0.f /* fSomersaultAngle */,
                    0 /* AP */, 100 /* HP */,
                    false /* bRespawn */,
                    0 /* nFrags */, 0 /* nDeaths */,
                    0 /* nSuicides */,
                    0.f /* fFiringAccuracy */,
                    0 /* nShotsFiredCount */,
                    false /* bInvulnerability */,
                    0.f /* fCurrentInventoryItemPower */))
                {
                    // server injects this msg to self so resources for player will be allocated upon processing these
                    m_pge.getNetwork().getServer().send(newPktSetup);
                    m_pge.getNetwork().getServer().send(newPktUserUpdate);
                }
                else
                {
                    getConsole().EOLn("PlayerHandling::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                    assert(false);
                    return false;
                }
            }
            else
            {
                getConsole().EOLn("PlayerHandling::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
                return false;
            }
        }
        else
        {
            // cannot happen
            getConsole().EOLn("PlayerHandling::%s(): user (connHandleServerSide: %u) connected with bCurrentClient as true but it is not me, CANNOT HAPPEN!",
                __func__, connHandleServerSide);
            assert(false);
            return false;
        }
    }
    else
    {
        if (connHandleServerSide == pge_network::ServerConnHandle)
        {
            getConsole().EOLn("PlayerHandling::%s(): server user (connHandleServerSide: %u) connected but map m_bCurrentClient is false, CANNOT HAPPEN!",
                __func__, connHandleServerSide);
            assert(false);
            return false;
        }
        // server is processing another user's birth
        getConsole().OLn("PlayerHandling::%s(): new remote user (connHandleServerSide: %u) connected (from %s) and I'm server",
            __func__, connHandleServerSide, msg.m_szIpAddress);

        // In the future we need something better than GameMode not having some funcs like getFragLimit()
        const DeathMatchMode* const pDeathMatchMode = dynamic_cast<proofps_dd::DeathMatchMode*>(GameMode::getGameMode());
        if (!pDeathMatchMode)
        {
            getConsole().EOLn("PlayerHandling::%s(): cast FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }

        // Send server config now, for example remaining game time on client side will start from server's remaining time
        // upon receiving this message, and also it is crucial that BEFORE client receives any player info, it recreates GameMode instance if needed, so
        // that any player info can be added to the proper GameMode instance at client-side.
        // This message will be received by client late enough to make the timeRemainingSecs annoying delayed, so we send updated message a bit later
        // as well in serverSendUserUpdates().
        if (!config.serverSendServerInfo(connHandleServerSide))
        {
            getConsole().EOLn("PlayerHandling::%s(): serverSendServerInfo() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }

        pge_network::PgePacket newPktSetup;
        if (!proofps_dd::MsgUserSetupFromServer::initPkt(newPktSetup, connHandleServerSide, false, msg.m_szIpAddress, m_maps.getFilename()))
        {
            getConsole().EOLn("PlayerHandling::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }

        const PureVector& vecStartPos = cfgProfiles.getVars()["testing"].getAsBool() ?
            m_maps.getRightMostSpawnpoint() :
            m_maps.getRandomSpawnpoint(GameMode::getGameMode()->isTeamBasedGame(), 0);

        if (!proofps_dd::MsgUserUpdateFromServer::initPkt(
            newPktUserUpdate,
            connHandleServerSide,
            vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(),
            0.f /* player angle Y */, 0.f /* player angle Z */,
            0.f /* weapon angle Z */,
            0.f /* weapon momentary accuracy */,
            false /* bActuallyRunningOnGround*/, false /* bCrouch */, 0.f /* fSomersaultAngle*/,
            0 /* AP */, 100 /* HP */,
            false /* bRespawn */,
            0 /* nFrags */, 0 /* nDeaths */,
            0 /* nSuicides */,
            0.f /* fFiringAccuracy */,
            0 /* nShotsFiredCount */,
            true /* invulnerable by default */,
            0.f /* fCurrentInventoryItemPower */))
        {
            getConsole().EOLn("PlayerHandling::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
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
    }

    return true;
}  // handleUserConnected()

bool proofps_dd::PlayerHandling::handleUserDisconnected(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const pge_network::MsgUserDisconnectedFromServer&,
    proofps_dd::GameMode& gameMode)
{
    // Due to https://github.com/proof88/PRooFPS-dd/issues/268, we need to apply WA here.
    // Explained in details in handlePlayerEventFromServer().
    // Due to this, now this WA is applied: return true from non-serveronly msg handling functions if connHandle is not found in m_mapPlayers.
    const auto playerIt = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == playerIt)
    {
        // ANOTHER CASE ALSO NEEDS WA:
        // TEMPORARILY COMMENTED DUE TO: https://github.com/proof88/PRooFPS-dd/issues/261
        // When we are trying to join a server but we get bored and user presses ESCAPE, client's disconnect is invoked, which
        // actually starts disconnecting because it thinks we are connected to server, and injects this userDisconnected pkt.
        // 
        //getConsole().EOLn("PlayerHandling::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
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
            "PlayerHandling::%s(): user %s with connHandleServerSide %u disconnected and I'm server",
            __func__, playerIt->second.getName().c_str(), connHandleServerSide);
    }
    else
    {
        getConsole().OLn(
            "PlayerHandling::%s(): user %s with connHandleServerSide %u disconnected and I'm client",
            __func__, playerIt->second.getName().c_str(), connHandleServerSide);
    }

    // display disconnect event only if this is not due to server disconnect, that is displayed with different text anyway;
    // using hasPlayerBootedUp(getMyServerSideConnectionHandle()) is for same reason as in handleUserNameChange().
    if ((connHandleServerSide != pge_network::ServerConnHandle) && hasPlayerBootedUp(getMyServerSideConnectionHandle()))
    {
        m_gui.getServerEvents()->addDisconnectedEvent(
            playerIt->second.getName(),
            GUI::getImVec4fromPureColor( TeamDeathMatchMode::getTeamColor(playerIt->second.getTeamId()) ));
    }

    gameMode.removePlayer(playerIt->second);
    m_mapPlayers.erase(playerIt);

    if (bClientShouldRemoveAllPlayers)
    {
        getConsole().OLn("PlayerHandling::%s(): it was actually the server disconnected so I'm removing every player including myself", __func__);
        m_gui.hideRespawnTimer();
        m_gui.hideGameObjectives();
        assert(m_gui.getDeathKillEvents());
        assert(m_gui.getItemPickupEvents());
        assert(m_gui.getPlayerInventoryChangeEvents());
        assert(m_gui.getPlayerHpChangeEvents());
        assert(m_gui.getPlayerApChangeEvents());
        assert(m_gui.getPlayerAmmoChangeEvents());
        m_gui.getDeathKillEvents()->clear();
        m_gui.getItemPickupEvents()->clear();
        m_gui.getPlayerInventoryChangeEvents()->clear();
        m_gui.getPlayerHpChangeEvents()->clear();
        m_gui.getPlayerApChangeEvents()->clear();
        m_gui.getPlayerAmmoChangeEvents()->clear();
        gameMode.restart(m_pge.getNetwork());
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
    PGEcfgProfiles& cfgProfiles)
{
    // TODO: make sure received user name is properly null-terminated! someone else could had sent that, e.g. malicious client or server

    GameMode* gameMode = GameMode::getGameMode();

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
        if (!gameMode->addPlayer(playerIt->second, m_pge.getNetwork()))
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

        // In the future we need something better than GameMode not having some funcs like getFragLimit()
        const DeathMatchMode* const pDeathMatchMode = dynamic_cast<proofps_dd::DeathMatchMode*>(GameMode::getGameMode());
        if (!pDeathMatchMode)
        {
            getConsole().EOLn("PlayerHandling::%s(): cast FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }
        if (msg.m_bCurrentClient)
        {
            // this is the moment when server player has fully booted up
            // 
            // server saves this config into the same struct as clients so GUI can show server config on server side as well, from the same struct as clients do!
            config.serverSaveServerInfo(
                cfgProfiles.getVars()[CVAR_FPS_MAX].getAsUInt(),
                config.getTickRate(),
                config.getPhysicsRate(),
                config.getClientUpdateRate(),
                gameMode->getGameModeType(),
                pDeathMatchMode->getFragLimit(),
                gameMode->getTimeLimitSecs(),
                config.getFallDamageMultiplier(),
                config.getPlayerRespawnDelaySeconds(),
                config.getPlayerRespawnInvulnerabilityDelaySeconds());
            
            m_pge.getAudio().stopSoundInstance(m_sounds.m_sndMenuMusicHandle);

            // as last step, restart the game mode now, this is important to be last step, for example remaining game time starts to count down now!
            gameMode->restartWithoutRemovingPlayers(m_pge.getNetwork());

            // UPDATE: commented out due to text is now added in GUI::drawCurrentPlayerInfo(), just kept comment here in case we want some other actions in the future
            //m_gui.textPermanent("Server, User name: " + std::string(szNewUserName) +
            //    (cfgProfiles.getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
            //    10, 30);
        }
        else
        {
            // this is the moment when client player has fully booted up
            m_gui.getServerEvents()->addConnectedEvent(playerIt->second.getName());
        }

        playerIt->second.setTimeBootedUp();

        // from our point of view, player has now fully booted up so NOW we are arming the respawn invulnerability
        getConsole().OLn("PlayerHandling::%s(): Player BOOTED UP, arming 1st-spawn invulnerability for connHandleServerSide: %u!", __func__, connHandleServerSide);
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

        // TEMPORAL WORKAROUND DUE TO: https://github.com/proof88/PRooFPS-dd/issues/268
        // we have the same WA in handleUserSetupFromServer().
        const auto itPlayerInGamemode = std::find_if(
            gameMode->getPlayersTable().begin(),
            gameMode->getPlayersTable().end(),
            [&msg](const proofps_dd::PlayersTableRow& row) { return row.m_sName == msg.m_szUserName; });
        if (itPlayerInGamemode != gameMode->getPlayersTable().end())
        {
            getConsole().EOLn("PlayerHandling::%s(): WA: connHandleServerSide: %u is already present in GameMode, allowed temporarily due to issue #268 (received: %s; set: %s)!",
                __func__, connHandleServerSide, msg.m_szUserName, playerIt->second.getName().c_str());
            return true;
        }

        playerIt->second.setTimeBootedUp();
        getConsole().OLn("PlayerHandling::%s(): Player BOOTED UP, accepting new name from server for connHandleServerSide: %u (%s), old name: %s, new name: %s!",
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
        if (!gameMode->addPlayer(playerIt->second, m_pge.getNetwork()))
        {
            getConsole().EOLn("PlayerHandling::%s(): failed to insert player %s (%u) into GameMode!", __func__, msg.m_szUserName, connHandleServerSide);
            assert(false);
            return false;
        }

        if (msg.m_bCurrentClient)
        {
            m_pge.getAudio().stopSoundInstance(m_sounds.m_sndMenuMusicHandle);

            // due to difficulties caused by m_gui.textPermanent() it is easier to use it here than in handleUserSetupFromServer()
            // UPDATE: commented out due to text is now added in GUI::drawCurrentPlayerInfo(), just kept comment here in case we want some other actions in the future
            //m_gui.textPermanent("Client, User name: " + playerIt->second.getName() + "; IP: " + playerIt->second.getIpAddress() +
            //    (cfgProfiles.getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
            //    10, 30);
        }
        else
        {
            // To avoid flooding of events about other players when we have just recently connected to server, let's suppress these events.
            // Due to the scheduling of player bringup messages (i.e. server sends out MsgUserSetupFromServer and MsgUserNameChangeAndBootupDone messages about other
            // players to me right after I'm establishing connection to the server, but for MY relevant MsgUserNameChangeAndBootupDone to receive from server, first
            // I need to process MY MsgUserSetupFromServer, for which I send MsgUserNameChangeAndBootupDone to the server, and server replies the same), I as new client
            // will receive MY relevant MsgUserNameChangeAndBootupDone from the server AFTER the other MsgUserNameChangeAndBootupDone messages about the other clients.
            // Therefore, my Player::getTimeBootedUp() will still not be set when I receive MsgUserNameChangeAndBootupDone messages about the other clients.
            // We can utilize this info to suppress messages received from server about the other players when I have just connected to the server.

            if (hasPlayerBootedUp(getMyServerSideConnectionHandle()))
            {
                m_gui.getServerEvents()->addConnectedEvent(playerIt->second.getName());
            }
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

void proofps_dd::PlayerHandling::serverSendUserUpdates(
    PGEcfgProfiles& /*cfgProfiles*/,
    proofps_dd::Config& config,
    proofps_dd::Durations& durations)
{
    if (!m_pge.getNetwork().isServer())
    {
        // should not happen, but we log it anyway, if in future we might mess up something during a refactor ...
        getConsole().EOLn("PlayerHandling::%s(): NOT server!", __func__);
        assert(false);
        return;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
    const bool bSendUserUpdates = (m_nSendClientUpdatesCntr == m_nSendClientUpdatesInEveryNthTick);

    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        const auto& playerConst = player;

        static constexpr std::chrono::seconds::rep nAfterBootUpDelayedUpdateSeconds = 3;
        if (playerConst.isExpectingAfterBootUpDelayedUpdate() &&
            (playerConst.getTimeBootedUp().time_since_epoch().count() != 0) &&
            ((std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - playerConst.getTimeBootedUp()).count()) >= nAfterBootUpDelayedUpdateSeconds))
        {
            player.setExpectingAfterBootUpDelayedUpdate(false);

            if (!config.serverSendServerInfo(playerPair.first))
            {
                getConsole().EOLn("PlayerHandling::%s(): serverSendServerInfo() FAILED at line %d!", __func__, __LINE__);
                assert(false);
                return;
            }
            getConsole().OLn("PlayerHandling::%s(): WA: sent out after-bootup delayed update to: %u", __func__, playerPair.first);
        } // isExpectingAfterBootUpDelayedUpdate()

        if (bSendUserUpdates && player.isNetDirty())
        {
            pge_network::PgePacket newPktUserUpdate;
            //getConsole().EOLn("PlayerHandling::%s(): send 1!", __func__);
            if (proofps_dd::MsgUserUpdateFromServer::initPkt(
                newPktUserUpdate,
                playerPair.second.getServerSideConnectionHandle(),
                playerConst.getPos().getNew().getX(),
                playerConst.getPos().getNew().getY(),
                playerConst.getPos().getNew().getZ(),
                playerConst.getAngleY(),
                playerConst.getAngleZ(),
                player.getWeaponAngle().getNew().getZ(),
                playerConst.getWeaponMomentaryAccuracy(),
                player.getActuallyRunningOnGround(),
                player.getCrouchStateCurrent(),
                player.getSomersaultAngle(),
                playerConst.getArmor().getNew(),
                playerConst.getHealth().getNew(),
                player.getRespawnFlag(),
                playerConst.getFrags(),
                playerConst.getDeaths(),
                playerConst.getSuicides(),
                playerConst.getFiringAccuracy(),
                playerConst.getShotsFiredCount(),
                playerConst.getInvulnerability(),
                playerConst.getCurrentInventoryItemPower()))
            {
                player.clearNetDirty();

                //if (playerPair.second.getRespawnFlag())
                //{
                //    getConsole().EOLn(
                //        "%s(): player XY: %f, %f",
                //        __func__,
                //        player.getPos().getNew().getX(), player.getPos().getNew().getY());
                //}

                // we always reset respawn flag here
                playerPair.second.getRespawnFlag() = false;

                // Note that health is not needed by server since it already has the updated health, but for convenience
                // we put that into MsgUserUpdateFromServer and send anyway like all the other stuff.
                m_pge.getNetwork().getServer().sendToAll(newPktUserUpdate);
                //getConsole().EOLn("PlayerHandling::%s(): send 2, invul: %b!", __func__, playerConst.getInvulnerability());
            }
            else
            {
                getConsole().EOLn("PlayerHandling::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
            }
        } // bSendUserUpdates
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
    XHair& xhair,
    const proofps_dd::Config& /*config*/,
    proofps_dd::GameMode& gameMode)
{
    // Due to https://github.com/proof88/PRooFPS-dd/issues/268, we need to apply WA here.
    // Explained in details in handlePlayerEventFromServer().
    // Due to this, now this WA is applied: return true from non-serveronly msg handling functions if connHandle is not found in m_mapPlayers.
    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PlayerHandling::%s(): failed to find user with connHandleServerSide: %u, WA in place, ignoring error!", __func__, connHandleServerSide);
        return true; // WA
    }

    const bool bCurrentClient = isMyConnection(connHandleServerSide);
    auto& player = it->second;
    const auto& playerConst = player;
    //getConsole().OLn("PlayerHandling::%s(): user %s received MsgUserUpdateFromServer: %f", __func__, player.getName().c_str(), msg.m_pos.x);

    const bool bOriginalExpectingStartPos = player.isJustCreatedAndExpectingStartPos();
    if (player.isJustCreatedAndExpectingStartPos())
    {
        // Player object is recently constructed and this is the 1st MsgUserUpdateFromServer for this Player
        player.setJustCreatedAndExpectingStartPos(false);

        // PPPKKKGGGGGG
        player.getPos().set(PureVector(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z));
        player.getPos().commit(); // both server and client commits in this case
        player.setHasJustStartedFallingNaturallyInThisTick(true);  // make sure vars for calculating high fall are reset

        if (m_pge.getNetwork().isServer())
        {
            // When Player is spawned for the 1st time, it is not a "re-"spawn, but we should still apply invulnerability since
            // this player has not yet fully booted up but gets visible to other players so we keep protected from other players.
            // I could not find a better place for this (MsgUserConnected, MsgUserSetupFromServer, etc.), so here we are forcing it.
            //getConsole().EOLn("PlayerHandling::%s(): 1st spawn: forced invulnerability for connHandleServerSide: %u!", __func__, connHandleServerSide);
            player.setInvulnerability(true, 999);
        }
        else
        {
            player.setInvulnerability(msg.m_bInvulnerability);
            //getConsole().EOLn("PlayerHandling::%s(): 1st spawn: initial invulnerability for connHandleServerSide: %u!", __func__, connHandleServerSide);
        }
    }
    else
    {
        if (!m_pge.getNetwork().isServer() && (msg.m_bInvulnerability != player.getInvulnerability()))
        {
            // only clients should fall here, server sets invulnerability in other locations and doesnt need to update itself here!           
            //getConsole().EOLn("PlayerHandling::%s(): new invulnerability state %b for connHandleServerSide: %u!", __func__, msg.m_bInvulnerability, connHandleServerSide);
            // no need to set time for clients even if state is true, since player.update() is not allowed to stop invulnerability on client-side.
            player.setInvulnerability(msg.m_bInvulnerability);
        }
    }

    // server has already set this in input handling and/or physics, however probably this is still faster than with condition: if (!m_pge.getNetwork().isServer())
    player.setSomersaultClient(msg.m_fSomersaultAngle);

    //getConsole().OLn("PlayerHandling::%s(): rcvd crouch: %b", __func__, msg.m_bCrouch);
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

    player.setWeaponMomentaryAccuracy(msg.m_fWpnMomentaryAccuracy);
    if (bCurrentClient)
    {
        // note that if we change values here, then setBaseScaling() might also need to be adjusted in GUI init!
        m_gui.getXHair()->setRelativeScaling(
            PFL::lerp(0.5f, 1.2f, msg.m_fWpnMomentaryAccuracy / player.getWeaponManager().getCurrentWeapon()->getLowestAccuracyPossible())
        );
    }

    // server has already set this in physics, however probably this is still faster than with condition: if (!m_pge.getNetwork().isServer())
    player.getActuallyRunningOnGround() = msg.m_bActuallyRunningOnGround;
    // note that I still don't know if this is the right way to trigger this sound playing ... this is one way, but the other way can be seen in
    // handleFallingFromHigh(), invoked by handlePlayerEventFromServer(). However, those sounds there are triggered non-continuously, while
    // running sound is repeating, and implicitly tied together with posxy change which is sent anyway here in this msg, so for running sound, I think
    // this is the proper location. For non-repeating, less frequent sounds that are not implicitly tied to anything in this message, can be handled
    // in handlePlayerEventFromServer().
    
    // we are not supposed to keep sending this value if it is unchanged, still the footstep sound is kept repeating, why?
    // because if there is true strafing, player position is being updated continuously ... hence we also fall into this function, hehe.
    if (player.getActuallyRunningOnGround())
    {
        player.handleActuallyRunningOnGround();
    }

    player.getFrags() = msg.m_nFrags;
    player.getDeaths() = msg.m_nDeaths;
    player.getSuicides() = msg.m_nSuicides;
    player.getFiringAccuracy() = msg.m_fFiringAccuracy;
    player.getShotsFiredCount() = msg.m_nShotsFired;
    player.setCurrentInventoryItemPower(msg.m_fCurrentInventoryItemPower);

    //getConsole().EOLn("PlayerHandling::%s(): rcvd health: %d, health: %d, old health: %d",
    //    __func__, msg.m_nHealth, std::as_const(player).getHealth(), std::as_const(player).getHealth().getOld());
    player.setArmor(msg.m_nArmor);
    player.setHealth(msg.m_nHealth);

    if (bCurrentClient)
    {
        // !!!BESTPRACTICE!!!
        // Server already has both new and old HP updated when we get here, and msg.m_nHealth contains the same value obviously.
        // So the easiest trick is to have a static var so we always know the old HP, no matter if we are server or client.
        // This way server and client can have this same shared code, instead of only client doing it here, and server doing it
        // at some other place.
        // Obviously it would be better if server would NOT update these values before injecting MsgUserUpdate but now this is
        // how it is.
        static int nPrevHP = 100;
        static int nPrevAP = 0;

        if (bOriginalExpectingStartPos || msg.m_bRespawn)
        {
            // We might be after map change, do not show any HP change for refilled HP!
            // Even though Player instances are recreated, this static var remembers, so we need to reset it.
            nPrevHP = 100;
            nPrevAP = 0;
        }
        else
        {
            if (std::as_const(player).getHealth() != nPrevHP)
            {
                assert(m_gui.getPlayerHpChangeEvents());
                const int nHpChange = std::as_const(player).getHealth() - nPrevHP;

                // As of v0.2.8 the max HP is 100, and we should not list +100% as HP change because that is basically respawn, and
                // when it happens, it is obvious to the player so don't make HP change event for that.
                if (nHpChange < 100)
                {
                    m_gui.getPlayerHpChangeEvents()->addEvent(std::to_string(nHpChange));
                }

                nPrevHP = std::as_const(player).getHealth();
            }
            if (std::as_const(player).getArmor() != nPrevAP)
            {
                assert(m_gui.getPlayerApChangeEvents());
                const int nApChange = std::as_const(player).getArmor() - nPrevAP;

                m_gui.getPlayerApChangeEvents()->addEvent(std::to_string(nApChange));

                nPrevAP = std::as_const(player).getArmor();
            }
        }
    }

    if (msg.m_bRespawn)
    {
        //getConsole().OLn("PlayerHandling::%s(): player %s has respawned!", __func__, player.getName().c_str());
        handlePlayerRespawned(player, xhair);
    }
    else
    {
        if ((playerConst.getHealth().getOld() > 0) && (playerConst.getHealth() == 0))
        {
            // only clients fall here, since server already set oldhealth to 0 at the beginning of this frame
            // because it had already set health to 0 in previous frame
            //getConsole().OLn("PlayerHandling::%s(): player %s has died!", __func__, player.getName().c_str());
            handlePlayerDied(player, xhair, player.getServerSideConnectionHandle() /* ignored by client anyway */);

            // TODO: until v0.2.0.0 this was the only location where client could figure out if any player died, however
            // now we have handleDeathNotificationFromServer(), we could simply move this code to there!
            // Client does not invoke HandlePlayerDied() anywhere else.
        }
    }

    // We might receive update for another player who has not yet handshaked its name with the server, in that case the name is empty, that
    // is why we also need to check emptiness!
    // Also, we should not try update ourselves in GameMode if bOriginalExpectingStartPos is set because that also means that
    // we not yet handshaked our own name with the server (but on our side our name is not empty, so we need this complicated logic below).
    if (!player.getName().empty())
    {
        if (!bCurrentClient || (bCurrentClient && !bOriginalExpectingStartPos))
        {
            if (!gameMode.updatePlayer(player, m_pge.getNetwork()))
            {
                getConsole().EOLn("%s: failed to update player %s in GameMode!", __func__, player.getName().c_str());
            }
        }
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

    // Due to https://github.com/proof88/PRooFPS-dd/issues/268, we need to apply WA here.
    // Explained in details in handlePlayerEventFromServer().
    // Due to this, now this WA is applied: return true from non-serveronly msg handling functions if connHandle is not found in m_mapPlayers.
    const auto itPlayerDied = m_mapPlayers.find(nDeadConnHandleServerSide);
    if (m_mapPlayers.end() == itPlayerDied)
    {
        getConsole().EOLn("PlayerHandling::%s(): failed to find dead with connHandleServerSide: %u, WA in place, ignoring error!", __func__, nDeadConnHandleServerSide);
        //assert(false);
        return true; // WA
    }

    std::string sKillerName;
    unsigned int iKillerTeamId = 0;
    const auto itPlayerKiller = m_mapPlayers.find(msg.m_nKillerConnHandleServerSide);
    if (m_mapPlayers.end() == itPlayerKiller)
    {
        getConsole().EOLn("PlayerHandling::%s(): failed to find killer with connHandleServerSide: %u!", __func__, msg.m_nKillerConnHandleServerSide);
        assert(false); // crash in debug, ignore in release mode: a bullet killing someone might be shot by a killer already disconnected before the impact,
        // however this is not likely to happen in debug mode when I'm testing regular gameplay!
    }
    else
    {
        // killer connhandle is set to player's connhandle also if killer got disconnected in the meantime, so that is not necessarily suicide!
        if (msg.m_nKillerConnHandleServerSide != nDeadConnHandleServerSide)
        {
            sKillerName = itPlayerKiller->second.getName();
            iKillerTeamId = itPlayerKiller->second.getTeamId();
        }
    }

    // from v0.2.5, client shows respawn timer here instead of in handlePlayerDied()
    if (isMyConnection(nDeadConnHandleServerSide))
    {
        m_gui.showRespawnTimer(
            sKillerName.empty() ? nullptr : &(itPlayerKiller->second));
    }

    // Server does death notification on GUI in HandlePlayerDied(), clients do here.
    assert(m_gui.getDeathKillEvents());
    m_gui.getDeathKillEvents()->addDeathKillEvent(
        sKillerName,
        GUI::getImVec4fromPureColor(TeamDeathMatchMode::getTeamColor(iKillerTeamId)),
        itPlayerDied->second.getName(),
        GUI::getImVec4fromPureColor(TeamDeathMatchMode::getTeamColor(itPlayerDied->second.getTeamId())));

    return true;
}

bool proofps_dd::PlayerHandling::handlePlayerEventFromServer(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const proofps_dd::MsgPlayerEventFromServer& msg,
    PureVector& vecCamShakeForce,
    const proofps_dd::Config& config,
    PGEcfgProfiles& cfgProfiles)
{
    //getConsole().EOLn("PlayerHandling::%s(): received event id: %u about player with connHandleServerSide: %u!", __func__, msg.m_iPlayerEventId, connHandleServerSide);

    // !!!BADDESIGN!!!
    // https://github.com/proof88/PRooFPS-dd/issues/268
    // Because of this issue, it can happen that a newly connecting client who not yet has received the list of other players, receives a player related update
    // like this. This typically happens when server uses sendToAllClientsExcept():
    // Flow is like:
    // - new client is connecting, accepted at server GNS/PGE level;
    // - server PGE invokes server APP packet handling callbacks, APP handles MsgUserConnected, injects MsgUserSetup;
    // - server PGE invokes onGameRunning(), server APP identifies a new event, informs players about it using sendToAllClientsExcept();
    // - new client receives message about a player it does NOT yet know, here the error can occur;
    // - server would send the list of other players to the new client only in next loop's packet handling, when processing the injected MsgUserSetup.
    // Until a good solution is provided, workaround is to NOT to treat such error on client side as critical, just ignoring that message.
    // One solution I can think about is that APP layer must use its own layer for sending out packets, for those who are present in the m_mapPlayers list.
    // Not directly calling PGE networking functions.
    // This would be a thin layer over PGE networking functions.
    // Due to this, now this WA is applied: return true from non-serveronly msg handling functions if connHandle is not found in m_mapPlayers.
    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PlayerHandling::%s(): failed to find user with connHandleServerSide: %u, WA in place, ignoring error!", __func__, connHandleServerSide);
        return true; // WA
    }

    const bool bCurrentClient = isMyConnection(connHandleServerSide);
    
    auto& player = it->second;

    // I made a comment in handleUserUpdateFromServer() about why the running footstep sound is handled there, and not here in this message.

    // PlayerEventId
    switch (msg.m_iPlayerEventId)
    {
    case PlayerEventId::FallingFromHigh:
        player.handleFallingFromHigh(msg.m_optData1.m_nValue);
        break;
    case PlayerEventId::Landed:
        player.handleLanded(msg.m_optData1.m_fValue, msg.m_optData2.m_bValue, msg.m_optData3.m_bValue, vecCamShakeForce, bCurrentClient);
        break;
    case PlayerEventId::ItemTake:
        player.handleTakeNonWeaponItem(static_cast<MapItemType>(msg.m_optData1.m_nValue), bCurrentClient);
        break;
    case PlayerEventId::InventoryItemToggle:
        player.handleToggleInventoryItem(
            static_cast<MapItemType>(msg.m_optData1.m_nValue),
            msg.m_optData2.m_bValue);
        break;
    case PlayerEventId::ItemUntake:
        player.handleUntakeInventoryItem(static_cast<MapItemType>(msg.m_optData1.m_nValue));
        break;
    case PlayerEventId::JumppadActivated:
        assert(bCurrentClient);
        player.handleJumppadActivated();
        break;
    case PlayerEventId::TeamIdChanged:
        handlePlayerTeamIdChanged(player, static_cast<unsigned int>(msg.m_optData1.m_nValue), config, cfgProfiles);
        break;
    case PlayerEventId::ExplosionMultiKill:
        handleExplosionMultiKill(msg.m_optData1.m_nValue);
        break;
    default:
        getConsole().EOLn("PlayerHandling::%s(): bad event id: %u about player with connHandleServerSide: %u!", __func__, msg.m_iPlayerEventId, connHandleServerSide);
        assert(false);  // crash in debug
        return false;
    }

    return true;
}

bool proofps_dd::PlayerHandling::serverHandleUserInGameMenuCmd(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const proofps_dd::MsgUserInGameMenuCmd& msg,
    const proofps_dd::Config& config,
    PGEcfgProfiles& cfgProfiles)
{
    //getConsole().EOLn("PlayerHandling::%s(): received event id: %d from player with connHandleServerSide: %u!", __func__, msg.m_iInGameMenu, connHandleServerSide);

    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PlayerHandling::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        assert(false);  // shall fail in debug mode private testing because no corner cases then
        return true;  // silent ignore this message in release mode: valid scenario, if server already deleted the player due to server shutdown, but this msg was still in queue!
    }

    auto& player = it->second;

    // GUI::InGameMenuState
    switch (msg.m_iInGameMenu)
    {
    case static_cast<int>(GUI::InGameMenuState::TeamSelect):
        handlePlayerTeamIdChanged(player, static_cast<unsigned int>(msg.m_optData1.m_nValue), config, cfgProfiles);
        break;
    default:
        getConsole().EOLn("PlayerHandling::%s(): bad event id: %d about player with connHandleServerSide: %u!", __func__, msg.m_iInGameMenu, connHandleServerSide);
        assert(false);  // crash in debug
        return false;
    }

    return true;
}

/**
* Invoked in every frame by all instances.
*/
void proofps_dd::PlayerHandling::updatePlayersVisuals(
    const proofps_dd::Config& config,
    proofps_dd::GameMode& gameMode)
{
    // both client and server come here, so this is a great place to HIDE respawn countdown for player if game has been won in the meantime.
    // as of v0.2.4 this is invoked in every frame so this is great!
    if (gameMode.isGameWon())
    {
        m_gui.hideRespawnTimer();
    }

    const bool bXHairIdentifiesPlayers = m_pge.getConfigProfiles().getVars()[XHair::szCvarGuiXHairIdentifiesPlayers].getAsBool();
    m_gui.getXHair()->hideIdText();
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        if (GameMode::getGameMode()->isGameWon())
        {
            player.getObject3D()->Hide();
            continue;
        }

        player.updateAudioVisuals(
            config, m_pge.getNetwork().isServer(), gameMode.isPlayerAllowedForGameplay(player));
        
        if (bXHairIdentifiesPlayers)
        {
            // it does not matter if we iterate from the back or the front, because players are ordered in the map by their conn handle, which
            // is not continuously increasing, so the latest added player might not be the last player in the map.
            // At the same time, when players are overlapping, the later added player is rendered over the earlier added player, which would imply
            // that in such situation the later added player's name is preferred to be shown, but it is not the case due to the order in the map.
            // To solve this, there should be also a different map where players are ordered by their connection time, and we should iterate that.
            // Opened ticket for this bug: https://github.com/proof88/PRooFPS-dd/issues/323
            if (!gameMode.isGameWon() &&
                m_gui.getXHair()->getIdText().empty() &&
                !isMyConnection(playerPair.first) &&
                (std::as_const(player).getObject3D()->isRenderingAllowed()) &&
                (std::as_const(player).getHealth() > 0) &&
                Physics::colliding2_NoZ(
                    player.getObject3D()->getPosVec().getX(), player.getObject3D()->getPosVec().getY(),
                    player.getObject3D()->getScaledSizeVec().getX(), player.getObject3D()->getScaledSizeVec().getY(),
                    m_gui.getXHair()->getUnprojectedCoords().getX(), m_gui.getXHair()->getUnprojectedCoords().getY(),
                    /* virtual 3D size of xhair */ 0.02f, 0.02f))
            {
                m_gui.getXHair()->showIdText(
                    player.getName() +
                    " | (" + std::to_string(std::as_const(player).getHealth().getNew()) + 
                    " / " + std::to_string(std::as_const(player).getArmor().getNew()) + ")",
                    GUI::getImVec4fromPureColor(TeamDeathMatchMode::getTeamColor(player.getTeamId()))
                );
                //getConsole().EOLn("PlayerHandling::%s(): xhair hit player: %s!", __func__, player.getName().c_str());
            }
        }
    }
}


// ############################### PRIVATE ###############################
