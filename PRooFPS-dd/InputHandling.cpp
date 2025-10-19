/*
    ###################################################################################
    InputHandling.cpp
    Input handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "InputHandling.h"

// there is no unistd.h in VS, we use process.h instead
//#include <unistd.h>   // for getpid()
#include <process.h>  // for getpid()

#include "SharedWithTest.h"

#include "Test.h"
#include "Benchmarks.h"

static constexpr float SndWpnDryFireDistMin = 6.f;
static constexpr float SndWpnDryFireDistMax = 14.f;

static constexpr std::chrono::milliseconds::rep nPlayerRespawnCountdownFastForwardByClickingMillisecs = 250;


// ############################### PUBLIC ################################


proofps_dd::InputHandling::InputHandling(
    PGE& pge,
    proofps_dd::Durations& durations,
    proofps_dd::GUI& gui,
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    proofps_dd::Maps& maps,
    proofps_dd::Sounds& sounds) :
    m_pge(pge),
    m_durations(durations),
    m_gui(gui),
    m_mapPlayers(mapPlayers),
    m_maps(maps),
    m_sounds(sounds),
    m_prevStrafe(proofps_dd::Strafe::NONE),
    m_strafe(proofps_dd::Strafe::NONE),
    m_bPrevAttack(false),
    m_bAttack(false),
    m_bPrevCrouch(false),
    m_bCrouch(false),
    m_fLastPlayerAngleYSent(-1.f),
    m_fLastWeaponAngleZSent(0.f)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations, gui, mapPlayers, maps, sounds
    // But they can be used in other functions.

    // Since this class is used to build up the PRooFPSddPGE class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. PRooFPSddPGE initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be existing at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
    assert(!pge.isGameRunning());
}

CConsole& proofps_dd::InputHandling::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* proofps_dd::InputHandling::getLoggerModuleName()
{
    return "InputHandling";
}


// ############################## PROTECTED ##############################


proofps_dd::InputHandling::PlayerAppActionRequest proofps_dd::InputHandling::clientHandleInputWhenConnectedAndSendUserCmdMoveToServer(
    proofps_dd::GameMode& gameMode,
    proofps_dd::Player& player,
    proofps_dd::XHair& xhair,
    const unsigned int nTickrate,
    const unsigned int nClUpdateRate,
    const unsigned int nPhysicsRateMin,
    proofps_dd::WeaponHandling& wpnHandling /* this design is really bad this way as it is explained in serverHandleUserCmdMoveFromClient() */)
{
    pge_network::PgePacket pkt;
    /* we always init the pkt with the current strafe state so it is correctly sent to server even if we are not setting it
       in keyboard(), this is needed if only clientMouseWhenConnectedToServer() generates reason to send the pkt */
    proofps_dd::MsgUserCmdFromClient::initPkt(pkt, m_strafe, m_bAttack, m_bCrouch, m_fLastPlayerAngleYSent, m_fLastWeaponAngleZSent);

    const proofps_dd::InputHandling::PlayerAppActionRequest playerAppActionReq =
        clientKeyboardWhenConnectedToServer(gameMode, pkt, player, nTickrate, nClUpdateRate, nPhysicsRateMin, wpnHandling);
    if (playerAppActionReq == proofps_dd::InputHandling::PlayerAppActionRequest::None)
    {
        clientMouseWhenConnectedToServer(gameMode, pkt, player, xhair.getObject3D());
        clientUpdatePlayerAsPerInputAndSendUserCmdMoveToServer(pkt, player, xhair.getObject3D(), gameMode);
    }
    return playerAppActionReq;
}

proofps_dd::InputHandling::PlayerAppActionRequest proofps_dd::InputHandling::clientHandleInputWhenDisconnectedFromServer()
{
    return clientKeyboardWhenDisconnectedFromServer();
}

bool proofps_dd::InputHandling::serverHandleUserCmdMoveFromClient(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const proofps_dd::MsgUserCmdFromClient& pktUserCmdMove,
    proofps_dd::GameMode& gameMode,
    proofps_dd::WeaponHandling& wpnHandling /* this design is really bad this way */)
{
    //const int nRandom = PFL::random(0, 100);
    //getConsole().EOLn("InputHandling::%s(): new msg from connHandleServerSide: %u, strafe: %d, %d, shoot: %b, crouch: %b!",
    //    __func__, connHandleServerSide, pktUserCmdMove.m_strafe, nRandom, pktUserCmdMove.m_bShootAction, pktUserCmdMove.m_bCrouch);

    if (!m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("InputHandling::%s(): client received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    if (gameMode.isGameWon())
    {
        // no input shall be accepted from any client when game is already finished
        return true;
    }

    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("InputHandling::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        assert(false);  // in debug mode this terminates server: same reason as in PlayerHandling::serverHandleUserInGameMenuCmd()
        return true;    // in release mode, we dont terminate the server, just silently ignore: same reason as in PlayerHandling::serverHandleUserInGameMenuCmd()
    }

    const std::string& sClientUserName = it->second.getName();

    if ((!pktUserCmdMove.m_bJumpAction) && (!pktUserCmdMove.m_bCrouch) && (!pktUserCmdMove.m_bSendSwitchToRunning) &&
        (pktUserCmdMove.m_fPlayerAngleY == -1.f) && (!pktUserCmdMove.m_bRequestReload) && (!pktUserCmdMove.m_bShouldSend))
    {
        getConsole().EOLn("InputHandling::%s(): user %s sent invalid cmdMove!", __func__, sClientUserName.c_str());
        assert(false);  // in debug mode this terminates server
        return false;   // in release mode, we dont terminate the server, just silently ignore
        // TODO: I might disconnect this client!
    }

    auto& player = it->second;
    const auto& playerConst = player;

    if (!gameMode.isPlayerAllowedForGameplay(playerConst))
    {
        // not error, valid state, maybe player not selected team yet
        return true;
    }

    if (playerConst.getHealth() == 0)
    {
        if (pktUserCmdMove.m_bShootAction)
        {
            //getConsole().EOLn("InputHandling::%s(): user %s is requesting respawn", __func__, sClientUserName.c_str());

            // this is for server's player timer respawn logic, independent of GUI respawn countdown, which is handled in clientMouseWhenConnectedToServer()
            player.moveTimeDiedEarlier(nPlayerRespawnCountdownFastForwardByClickingMillisecs);

            return true;
        }

        // for dead player, only the shoot action is allowed which is treated as respawn request
        //getConsole().OLn("InputHandling::%s(): ignoring cmdMove for user %s due to health is 0!", __func__, sClientUserName.c_str());
        return true;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    if (pktUserCmdMove.m_bSendSwitchToRunning)
    {
        const auto nMillisecsSinceLastToggleRunning =
            std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - player.getTimeLastToggleRun()).count();
        if (nMillisecsSinceLastToggleRunning < m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds)
        {
            // should NOT had received this from client this early
            getConsole().OLn("InputHandling::%s(): player %s sent run toggle request too early, ignoring (actual: %d, req: %d)!",
                __func__, sClientUserName.c_str(), nMillisecsSinceLastToggleRunning, m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds);
            // Dont terminate for now, just log. Reason explained below at handling jumping.
            //assert(false);  // in debug mode, terminate the game
        }
        else
        {
            player.setRun(!player.isRunning());
        }
    }

    // make sure we have an up-to-date angle Y so startSomersaultServer() has the up-to-date data to decide things
    if ((pktUserCmdMove.m_fPlayerAngleY != -1.f) && (!player.isSomersaulting()))
    {
        player.getAngleY() = pktUserCmdMove.m_fPlayerAngleY;
        player.getObject3D()->getAngleVec().SetY(pktUserCmdMove.m_fPlayerAngleY);
    }

    const auto nMillisecsSinceLastStrafe =
        std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - player.getTimeLastActualStrafe()).count();
    const auto prevStrafeState = player.getStrafe();
    // since v0.1.3 strafe is a continuous operation until client explicitly requests server to stop simulating it, so Strafe::NONE is always accepted.
    player.setStrafe(pktUserCmdMove.m_strafe);
    if ((prevStrafeState == Strafe::NONE) && (player.getStrafe() != Strafe::NONE) && (player.getPreviousActualStrafe() == player.getStrafe()))
    {
        // at this point, player triggered either a LEFT-NONE-LEFT or a RIGHT-NONE-RIGHT combo
        if (!player.isInAir() && !player.isSomersaulting() && (nMillisecsSinceLastStrafe <= m_nKeyPressSomersaultMaximumWaitMilliseconds))
        {
            //getConsole().EOLn("InputHandling::%s(): player %s somersault initiated!", __func__, sClientUserName.c_str());
            // unlike with mid-air somersaulting that can be triggered in InputHandling, the on-ground somersaulting should be postponed to Physics:
            // because Physics can determine if there is horizontal collision occuring at the moment, and if not, then it can trigger the somersault!
            player.setWillSomersaultInNextTick(true);
        }
    }

    //static std::chrono::time_point<std::chrono::steady_clock> timeStrafeStarted;
    //static float fPlayerPosXStarted = 0.f;
    //if (player.getStrafe() != Strafe::NONE)
    //{
    //    timeStrafeStarted = timeStart;
    //    fPlayerPosXStarted = player.getPos().getNew().getX();
    //}
    //else
    //{
    //    const auto nMillisecsStrafing =
    //        std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - timeStrafeStarted).count();
    //    const float fStrafeDistance = player.getPos().getNew().getX() - fPlayerPosXStarted;
    //
    //    getConsole().EOLn("Strafe duration: %d msecs, dist.: %f", nMillisecsStrafing, fStrafeDistance);
    //}

    // crouching is also continuous op
    player.getCrouchInput().set(pktUserCmdMove.m_bCrouch);
    // crouch-induced player scaling and repositioning are handled in the physics class
    if (player.getCrouchInput().getOld() && !player.getCrouchInput().getNew())
    {
        player.getWantToStandup() = true;  // this stays permanent across frames, getCrouchInput() old and new is valid only this frame
        //getConsole().EOLn("%s player %s just signaled wanna stand up", __func__, sClientUserName.c_str());
    }
    else if (!player.getCrouchInput().getOld() && player.getCrouchInput().getNew())
    {
        //getConsole().EOLn("%s player %s just signaled wanna go down crouch", __func__, sClientUserName.c_str());
        player.getWantToStandup() = false;  // this stays permanent across frames, getCrouchInput() old and new is valid only this frame
    }

    if (pktUserCmdMove.m_bJumpAction)
    {
        //getConsole().EOLn("InputHandling::%s(): asd 1", __func__);

        // jump-induced actions can be initiated only if we cannot fall at the moment (I always forget though what does "cannot fall" mean)
        if (!player.canFall())
        {
            const auto nMillisecsSinceLastJump =
                std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - player.getTimeLastSetWillJump()).count();
            if (player.isJumping())
            {
                //getConsole().EOLn("jumping");

                // isJumping() is set to true by the Physics class when jumping is really initiated, and stays true until losing upwards jump force, so
                // if we are here, we can be 100% sure that an actual ongoing jumping is happening now.
                if (/*player.getCrouchInput().getNew() &&*/ !player.isSomersaulting() && (nMillisecsSinceLastJump <= m_nKeyPressSomersaultMaximumWaitMilliseconds))
                {
                    //getConsole().EOLn("InputHandling::%s(): player %s somersault initiated!", __func__, sClientUserName.c_str());
                    player.startSomersaultServer(true);
                }

                // starting somersault and wall jump at the same time is allowed, and in fact, they need to be handled with their separate conditions,
                // because even if player is late to initiate somersaulting, possibility of initiating wall jumping should be still available.
                // Player angle Y and strafe must be also updated for this to work properly, both of them are updated in earlier lines above.
                player.setWillWallJumpInNextTick();
            }
            else
            {
                //getConsole().EOLn("not jumping");
                if (nMillisecsSinceLastJump < m_nKeyPressOnceJumpMinumumWaitMilliseconds)
                {
                    // should NOT had received this from client this early (actually could, see explanation below)
                    getConsole().EOLn("InputHandling::%s(): player %s sent jump request too early, ignoring (actual: %d, req: %d)!",
                        __func__, sClientUserName.c_str(), nMillisecsSinceLastJump, m_nKeyPressOnceJumpMinumumWaitMilliseconds);
                    // For now, dont terminate. Reason: since client does the rate limit on its side, it can happen that the required time elapsed
                    // at client-side but did not elapse at server-side. Imagine client sends a packet to server, the ping is a bit high. Client
                    // already starts to wait the required delay before sending next packet. Server receives the packet 30 ms later and starts
                    // measuring time. In the meantime client sends next packet, ping is lower, delay is much less, so server receives the packet
                    // a few msecs earlier than the required delay elapsed on server-side.
                    // So we should not terminate but log these occurrences to understand how many such occasions are there on LAN party.
                    // A way to solve this issue in reliable way is that client must always send its timestamp as well to the server, so
                    // server can check if required time elapsed based on client's sent timestamp compared to the previously sent timestamp.
                    // To avoid cheating, server must also decide if client timestamp is valid: server must save the initial client timestamp
                    // upon client connect, and it can check if game session time duration is actually matching the real elapsed time with
                    // client's elapsed time.
                    //assert(false);  // in debug mode, terminate the game
                }
                else
                {
                    // Since we are doing the actual strafe movement in the Physics class, the forces we would like to record at the moment
                    // of jumping up are available there, not here. So here we are just recording that we will do the jump: delaying it to the
                    // Physics class, so inside there at the correct place jump() will be invoked and correct forces will be saved.
                    player.setWillJumpInNextTick(1.f, 0.f);
                }
            }
        }
    }

    Weapon* const wpn = player.getWeaponManager().getCurrentWeapon();
    if (!wpn)
    {
        getConsole().EOLn("InputHandling::%s(): getWeapon() failed!", __func__);
        assert(false);
        return false;
    }

    if (!pktUserCmdMove.m_bShootAction)
    {
        if (!wpn->isTriggerReleased())
        {
            //getConsole().OLn("InputHandling::%s(): player %s released trigger!", 
            //    __func__, sClientUserName.c_str());
        }
        wpn->releaseTrigger();
        player.getAttack() = false;
    }

    if (pktUserCmdMove.m_bRequestReload)
    {
        static std::chrono::time_point<std::chrono::steady_clock> timeLastWpnReload;
        const auto nMillisecsSinceLastWpnReload =
            std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - timeLastWpnReload).count();
        if (nMillisecsSinceLastWpnReload < m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds)
        {
            // should NOT had received this from client this early
            getConsole().OLn("InputHandling::%s(): player %s sent wpn reload request too early, ignoring (actual: %d, req: %d)!",
                __func__, sClientUserName.c_str(), nMillisecsSinceLastWpnReload, m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds);
            // Dont terminate for now, just log. Reason explained above at handling jumping.
            //assert(false);  // in debug mode, terminate the game
        }
        else
        {
            timeLastWpnReload = std::chrono::steady_clock::now();
            if (wpn->reload())
            {

                //getConsole().OLn("InputHandling::%s(): player %s reloading the weapon!",
                //    __func__, sClientUserName.c_str());
            }
            else
            {
                //getConsole().OLn("InputHandling::%s(): player %s requested reload but we ignore it!",
                //    __func__, sClientUserName.c_str());
            }
        }
    }

    if (!pktUserCmdMove.m_bRequestReload && (wpn->getState() == Weapon::State::WPN_READY) && (pktUserCmdMove.m_cWeaponSwitch != '\0'))
    {
        const auto nMillisecsSinceLastWpnSwitch =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStart - player.getWeaponManager().getTimeLastWeaponSwitch()
            ).count();
        if (nMillisecsSinceLastWpnSwitch < m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds)
        {
            // should NOT had received this from client this early
            getConsole().OLn("InputHandling::%s(): player %s sent wpn switch request too early, ignoring (actual: %d, req: %d)!",
                __func__, sClientUserName.c_str(), nMillisecsSinceLastWpnSwitch, m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds);
            // Dont terminate for now, just log. Reason explained above at handling jumping.
            //assert(false);  // in debug mode, terminate the game
        }
        else
        {
            const auto itTargetWpn = WeaponManager::getKeypressToWeaponMap().find(pktUserCmdMove.m_cWeaponSwitch);
            if (itTargetWpn == WeaponManager::getKeypressToWeaponMap().end())
            {
                const std::string sc = std::to_string(pktUserCmdMove.m_cWeaponSwitch); // because CConsole still doesnt support %c!
                getConsole().EOLn("InputHandling::%s(): weapon not found for char %s!", __func__, sc.c_str());
                assert(false);
                return false;
            }

            Weapon* const pTargetWpn = player.getWeaponManager().getWeaponByFilename(itTargetWpn->second);
            if (!pTargetWpn)
            {
                getConsole().EOLn("InputHandling::%s(): weapon not found for name %s!", __func__, itTargetWpn->second.c_str());
                assert(false);
                return false;
            }

            if (!pTargetWpn->isAvailable())
            {
                getConsole().EOLn("InputHandling::%s(): weapon not available: %s!", __func__, itTargetWpn->second.c_str());
                return true;    // just silently ignore, maybe it was a message sent from client earlier when that weapon was still available!
            }

            if (pTargetWpn != player.getWeaponManager().getCurrentWeapon())
            {
                if (connHandleServerSide == pge_network::ServerConnHandle)
                {   // server plays for itself because it doesnt inject the MsgCurrentWpnUpdateFromServer to itself
                    m_pge.getAudio().playSound(m_sounds.m_sndChangeWeapon);
                    assert(m_gui.getPlayerAmmoChangeEvents());
                    m_gui.getPlayerAmmoChangeEvents()->clear();
                }
                const auto prevWpn = player.getWeaponManager().getCurrentWeapon();
                if (player.getWeaponManager().setCurrentWeapon(pTargetWpn, true, m_pge.getNetwork().isServer()))
                {
                    if (connHandleServerSide == pge_network::ServerConnHandle)
                    {
                         // !!!BADDESIGN!!!
                        // this is ridiculous.
                        // InputHandling should not use wpnHandling.
                        // All Weapon-related messages should be injected by server to self, thus the processing of such events
                        // would be the same as clients process them.
                        // Would be nice to think about a design change.
                        // Player movement is fine, but weapon handling is a bit different between server and client, leading to the need of
                        // invoking some functions at multiple places, such as:
                        // - handleCurrentWeaponBulletCountsChangeShared();
                        // - handleWeaponStateChangeShared().
                        wpnHandling.handleCurrentPlayersCurrentWeaponBulletCountsChangeShared(
                            player,
                            *pTargetWpn,
                            prevWpn->getMagBulletCount(),
                            pTargetWpn->getMagBulletCount(),
                            prevWpn->getUnmagBulletCount(),
                            pTargetWpn->getUnmagBulletCount(),
                            pTargetWpn->getState().getOld(),
                            pTargetWpn->getState().getNew()
                        );
                    }
                }
                else
                {
                    getConsole().EOLn("InputHandling::%s(): player %s switching to %s failed due to setCurrentWeapon() failed!",
                        __func__, sClientUserName.c_str(), itTargetWpn->second.c_str());
                    assert(false);  // in debug mode, terminate the game
                    return true;   // in release mode, dont terminate the server, just silently ignore!
                }
                it->second.getWeaponManager().getCurrentWeapon()->UpdatePosition(it->second.getObject3D()->getPosVec(), player.isSomersaulting());

                getConsole().OLn("InputHandling::%s(): player %s switching to %s!",
                    __func__, sClientUserName.c_str(), itTargetWpn->second.c_str());

                // all clients must be updated about this player's weapon switch
                pge_network::PgePacket pktWpnUpdateCurrent;
                proofps_dd::MsgCurrentWpnUpdateFromServer::initPkt(
                    pktWpnUpdateCurrent,
                    connHandleServerSide,
                    pTargetWpn->getFilename(),
                    pTargetWpn->getState().getNew());
                m_pge.getNetwork().getServer().sendToAllClientsExcept(pktWpnUpdateCurrent);
            }
            //else
            //{
            //    // UPDATE 2024-08-01: this can happen if player has at least 2 weapons, dies, and during the respawn
            //    // countdown, we rapidly scroll up&down or keep pressing the button for switching to the another weapon,
            //    // so I just commented this out, not much use of this else block anyway.
            // 
            //    // should not happen because client should NOT send message in such case
            //    getConsole().OLn("InputHandling::%s(): player %s already has target wpn %s, CLIENT SHOULD NOT SEND THIS!",
            //        __func__, sClientUserName.c_str(), itTargetWpn->second.c_str());
            //    assert(false);  // in debug mode, terminate the game
            //    return true;   // in release mode, dont terminate the server, just silently ignore!
            //}
        }
    }

    // TODO: this should be moved up, so returning from function is easier for rest of action handling code
    if (!player.isSomersaulting())
    {
        player.getWeaponAngle().set(PureVector(0.f, player.getAngleY(), pktUserCmdMove.m_fWpnAngleZ));
        wpn->getObject3D().getAngleVec().SetY(player.getAngleY());
        wpn->getObject3D().getAngleVec().SetZ(pktUserCmdMove.m_fWpnAngleZ);
    }

    if (pktUserCmdMove.m_bRequestReload || (pktUserCmdMove.m_cWeaponSwitch != '\0'))
    {
        // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
        m_durations.m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
        return true; // don't check anything related to shooting in case of either of these actions
    }

    // we should be allowed to come here if current wpn's state is NOT ready, because in some cases it is allowed to shoot: for example,
    // if wpn has per-bullet reload, shooting can cancel the reloading state!
    if (pktUserCmdMove.m_bShootAction)
    {
        const auto nSecsSinceLastWeaponSwitch =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStart - player.getWeaponManager().getTimeLastWeaponSwitch()
            ).count();
        if (nSecsSinceLastWeaponSwitch < m_nWeaponActionMinimumWaitMillisecondsAfterSwitch)
        {
            //getConsole().EOLn("InputHandling::%s(): ignoring too early mouse action!", __func__);
            // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
            m_durations.m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
            return true;
        }
        else
        {
            //getConsole().EOLn("InputHandling::%s(): m_bShootAction true and accepted!", __func__);
            player.getAttack() = true;
        }
    }
    // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
    m_durations.m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

    return true;
}


// ############################### PRIVATE ###############################


proofps_dd::InputHandling::PlayerAppActionRequest proofps_dd::InputHandling::clientKeyboardWhenConnectedToServer(
    proofps_dd::GameMode& gameMode,
    pge_network::PgePacket& pkt,
    proofps_dd::Player& player,
    const unsigned int nTickrate,
    const unsigned int nClUpdateRate,
    const unsigned int nPhysicsRateMin,
    proofps_dd::WeaponHandling& wpnHandling /* this design is really bad this way as it is explained in serverHandleUserCmdMoveFromClient() */)
{
    // fetch auto-behaviors into vars so we can clear them right away before any early return, this is to make sure we always clear them!
    bool bWeaponAutoReloadWasRequested = wpnHandling.getWeaponAutoReloadRequest();
    bool bAutoSwitchToBestLoadedWasRequested = wpnHandling.getWeaponAutoSwitchToBestLoadedRequest();
    bool bAutoSwitchToBestWithAnyKindOfAmmoWasRequested = wpnHandling.getWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest();
    Weapon* pWeaponPickupInducedAutoSwitchWasRequested = wpnHandling.getWeaponPickupInducedAutoSwitchRequest();

    wpnHandling.clearWeaponAutoReloadRequest();
    wpnHandling.clearWeaponAutoSwitchToBestLoadedRequest();
    wpnHandling.clearWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest();
    wpnHandling.clearWeaponPickupInducedAutoSwitchRequest();

    if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_ESCAPE))
    {
        m_gui.hideInGameMenu();
        return proofps_dd::InputHandling::PlayerAppActionRequest::Exit;
    }

    // allow server to operate server admin menu even if game is already in won state or when player is dead.
    if (m_pge.getInput().getKeyboard().isKeyPressedOnce(static_cast<unsigned char>(VkKeyScan(GAME_INPUT_KEY_MENU_SERVERADMIN))))
    {
        if (m_pge.getNetwork().isServer())
        {
            // avoid key function if we are in a differen in-game menu!
            if ((m_gui.getInGameMenuState() == GUI::InGameMenuState::None) ||
                (m_gui.getInGameMenuState() == GUI::InGameMenuState::ServerAdmin))
            {
                m_gui.showHideInGameServerAdminMenu();
            }
        }
    }

    if (gameMode.isGameWon())
    {
        return proofps_dd::InputHandling::PlayerAppActionRequest::None;
    }

    if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_TAB))
    {
        // dont allow unassigned players to move away from frag table, simply because I want only frag table to be shown to them
        if (!gameMode.isTeamBasedGame() || (player.getTeamId() != 0u))
        {
            m_gui.showAndLoopGameInfoPages();
        }
    }

    const auto& playerConst = player;

    if (playerConst.getHealth() == 0)
    {
        return proofps_dd::InputHandling::PlayerAppActionRequest::None;
    }

    // put this here so dead player i.e. waiting to respawn cannot change team during respawn countdown!
    if (m_pge.getInput().getKeyboard().isKeyPressedOnce(static_cast<unsigned char>(VkKeyScan(GAME_INPUT_KEY_MENU_TEAMSELECTION))) &&
        gameMode.isTeamBasedGame())
    {
        // avoid key function if we are in a differen in-game menu!
        if ((m_gui.getInGameMenuState() == GUI::InGameMenuState::None) ||
            (m_gui.getInGameMenuState() == GUI::InGameMenuState::TeamSelect))
        {
            if (playerConst.getTeamId() == 0u)
            {
                if (m_gui.getInGameMenuState() == GUI::InGameMenuState::TeamSelect)
                {
                    // we dont have spectator mode, so if unassigned player hides team selection menu, frag table shall be automatically visible
                    m_gui.showGameObjectives();
                }
            }
            m_gui.showHideInGameTeamSelectMenu();
        }
    }

    // accept this ENTER key for testing before returning if we are in an in-game menu so that reg-tests can dump to file here
    if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_RETURN))
    {
        if (m_pge.getConfigProfiles().getVars()["testing"].getAsBool())
        {
            getConsole().SetLoggingState("4LLM0DUL3S", true);
            regTestDumpToFile(gameMode, player, nTickrate, nClUpdateRate, nPhysicsRateMin);
            getConsole().SetLoggingState("4LLM0DUL3S", false);
        }
    }

    // active in-game menu cancels any player control, and
    // in-game menu input is ignored here after in-game menu is closed
    static bool bRequireKeyUpBeforeAcceptingPlayerInput = false;
    if (bRequireKeyUpBeforeAcceptingPlayerInput ||
        (m_gui.getInGameMenuState() != GUI::InGameMenuState::None))
    {
        if (m_pge.getInput().getKeyboard().isKeyPressed())
        {
            bRequireKeyUpBeforeAcceptingPlayerInput = true; // sticky until all keys are released
            return proofps_dd::InputHandling::PlayerAppActionRequest::None;
        }
        bRequireKeyUpBeforeAcceptingPlayerInput = false;
    }
    if (bRequireKeyUpBeforeAcceptingPlayerInput ||
        (m_gui.getInGameMenuState() != GUI::InGameMenuState::None))
    {
        return proofps_dd::InputHandling::PlayerAppActionRequest::None;
    }

    if (m_pge.getInput().getKeyboard().isKeyPressedOnce((unsigned char)VkKeyScan('t')))
    {
        if (m_pge.getNetwork().isServer())
        {
            // for testing purpose only, we can teleport server player to random spawn point
            // TODO: probably we should just call serverRespawnPlayer() but cannot access that from here :/

            player.getPos() = m_maps.getRandomSpawnpoint(gameMode.isTeamBasedGame(), player.getTeamId());
            // no need to commit, otherwise server wont even inject userupdatemsg to itself!
            //player.getPos().commit();
            getConsole().EOLn(
                "%s(): server forced respawn to XY: %f, %f",
                __func__,
                player.getPos().getNew().getX(), player.getPos().getNew().getY());
            player.getRespawnFlag() = true;
        }

        // log some stats
        getConsole().SetLoggingState("PureRendererHWfixedPipe", true);
        m_pge.getPure().getRenderer()->ResetStatistics();
        getConsole().SetLoggingState("PureRendererHWfixedPipe", false);
        
        getConsole().SetLoggingState(getLoggerModuleName(), true);
        getConsole().OLn("");
        getConsole().OLn("FramesElapsedSinceLastDurationsReset: %d", m_durations.m_nFramesElapsedSinceLastDurationsReset);
        getConsole().OLn("Avg Durations per Frame:");
        getConsole().OLn(" - FullRoundtripDuration: %f usecs", m_durations.m_nFullRoundtripDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn(" - FullOnPacketReceivedDuration: %f usecs", m_durations.m_nFullOnPacketReceivedDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("   - HandleUserCmdMoveDuration: %f usecs", m_durations.m_nHandleUserCmdMoveDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn(" - FullOnGameRunningDuration: %f usecs", m_durations.m_nFullOnGameRunningDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("   - GravityCollisionDuration: %f usecs", m_durations.m_nGravityCollisionDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("   - ActiveWindowStuffDuration: %f usecs", m_durations.m_nActiveWindowStuffDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("   - UpdateWeaponDuration: %f usecs", m_durations.m_nUpdateWeaponsDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("   - UpdateBulletsDuration: %f usecs", m_durations.m_nUpdateBulletsDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("   - UpdateRespawnTimersDuration: %f usecs", m_durations.m_nUpdateRespawnTimersDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("   - PickupAndRespawnItemsDuration: %f usecs", m_durations.m_nPickupAndRespawnItemsDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("   - UpdateGameModeDuration: %f usecs", m_durations.m_nUpdateGameModeDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("   - CameraMovementDuration: %f usecs", m_durations.m_nCameraMovementDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("   - SendUserUpdatesDuration: %f usecs", m_durations.m_nSendUserUpdatesDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
        getConsole().OLn("");

        getConsole().OLn("ScopeBenchmarkers:");
        for (const auto& bmData : ScopeBenchmarkerDataStore::getAllData())
        {
            std::string sBmLog =
                ("    " +
                    bmData.second.m_name +
                    " Iterations: " + std::to_string(bmData.second.m_iterations) +
                    ", Durations: Min/Max/Avg: " +
                    std::to_string(bmData.second.m_durationsMin) + "/" +
                    std::to_string(bmData.second.m_durationsMax) + "/" +
                    /* Test::toString() for getting rid of unneeded zeros after decimal point */
                    Test::toString(bmData.second.getAverageDuration()) +
                    " " + bmData.second.getUnitString() +
                    ", Total: " +
                    std::to_string(bmData.second.m_durationsTotal) +
                    " " + bmData.second.getUnitString());
            getConsole().OLn("%s", sBmLog.c_str());
        }
        getConsole().OLn("");
        getConsole().SetLoggingState(getLoggerModuleName(), false);
        
        m_durations.reset();
        ScopeBenchmarkerDataStore::clear(); // since ScopeBenchmarker works with static data, make sure we dont leave anything there
    }

    // For now we dont need rate limit for strafe, but in future if FPS limit can be disable we probably will want to limit this!
    if (m_pge.getInput().getKeyboard().isKeyPressed(VK_LEFT) || m_pge.getInput().getKeyboard().isKeyPressed((unsigned char)VkKeyScan('a')))
    {
        m_strafe = proofps_dd::Strafe::LEFT;
    }
    else if (m_pge.getInput().getKeyboard().isKeyPressed(VK_RIGHT) || m_pge.getInput().getKeyboard().isKeyPressed((unsigned char)VkKeyScan('d')))
    {
        m_strafe = proofps_dd::Strafe::RIGHT;
    }
    else
    {
        m_strafe = proofps_dd::Strafe::NONE;
    }

    // isKeyPressed() detects left SHIFT or CONTROL keys only, detecting the right-side stuff requires engine update.
    // For now we dont need rate limit for this, but in future if FPS limit can be disable we probably will want to limit this!
    m_bCrouch = m_pge.getInput().getKeyboard().isKeyPressed(VK_CONTROL);

    bool bSendJumpAction = false;
    if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_SPACE, m_nKeyPressOnceJumpMinumumWaitMilliseconds))
    {
        bSendJumpAction = true;
    }

    bool bToggleRunWalk = false;
    if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_SHIFT, m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds))
    {
        bToggleRunWalk = true;
    }

    const bool bFireButtonPressed = m_pge.getInput().getMouse().isButtonPressed(PGEInputMouse::MouseButton::MBTN_LEFT);
    if (bFireButtonPressed)
    {
        // neither of the firing-induced auto-reload or auto-switch behaviors can be executed if user is still pressing fire button, however
        // whichever is requested by WeaponHandling, should be postponed to later when fire button is finally released.
        if (bWeaponAutoReloadWasRequested)
        {
            //getConsole().EOLn("InputHandling::%s(): scheduled auto-reload for next time!", __func__);
            wpnHandling.scheduleWeaponAutoReloadRequest();
            bWeaponAutoReloadWasRequested = false;
        }
        else if (bAutoSwitchToBestLoadedWasRequested)
        {
            //getConsole().EOLn("InputHandling::%s(): scheduled auto-switch to best loaded for next time!", __func__);
            wpnHandling.scheduleWeaponAutoSwitchToBestLoadedRequest();
            bAutoSwitchToBestLoadedWasRequested = false;
        }
        else if (bAutoSwitchToBestWithAnyKindOfAmmoWasRequested)
        {
            //getConsole().EOLn("InputHandling::%s(): scheduled auto-switch to best any for next time!", __func__);
            wpnHandling.scheduleWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest();
            bAutoSwitchToBestWithAnyKindOfAmmoWasRequested = false;
        }
    }

    if (bFireButtonPressed ||
        (player.getWeaponManager().getCurrentWeapon() && (player.getWeaponManager().getCurrentWeapon()->getState() != Weapon::WPN_READY)))
    {
        // cannot auto-switch to picked up weapon now if user is still pressing fire button, or the current weapon is still doing something
        if (pWeaponPickupInducedAutoSwitchWasRequested)
        {
            //getConsole().EOLn("InputHandling::%s(): scheduled auto-switch to picked up for next time!", __func__);
            wpnHandling.scheduleWeaponPickupInducedAutoSwitchRequest(pWeaponPickupInducedAutoSwitchWasRequested);
            pWeaponPickupInducedAutoSwitchWasRequested = nullptr;
        }
    }

    bool bRequestReload = bWeaponAutoReloadWasRequested; // might be false even tho bWeaponAutoReloadWasRequested is rescheduled, but it is not a problem, server will ignore anyway
    if (m_pge.getInput().getKeyboard().isKeyPressedOnce((unsigned char)VkKeyScan('r'), m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds))
    {
        bRequestReload = true;
    }

    //if (bRequestReload)
    //{
    //    getConsole().EOLn("InputHandling::%s(): bRequestReload became true!", __func__);
    //}

    unsigned char cWeaponSwitch = '\0';
    if (!bRequestReload)
    {   // we dont care about wpn switch if reload is requested
        const auto nSecsSinceLastWeaponSwitchMillisecs =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - player.getWeaponManager().getTimeLastWeaponSwitch()
            ).count();
        if (nSecsSinceLastWeaponSwitchMillisecs >= m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds)
        {
            const Weapon* pNextBestWpnFound = nullptr;
            if (bAutoSwitchToBestLoadedWasRequested)
            {
                //getConsole().EOLn("InputHandling::%s(): trying auto switch to best with mag ammo ...", __func__);
                pNextBestWpnFound = player.getWeaponManager().getNextBestAvailableWeapon(cWeaponSwitch, true /* must have mag bullet */);
                if (pNextBestWpnFound == nullptr)
                {
                    getConsole().EOLn("InputHandling::%s(): SHOULD NOT HAPPEN: getNextBestAvailableWeapon() 1 returned nullptr!", __func__);
                    assert(false);  // crash in debug
                    return proofps_dd::InputHandling::PlayerAppActionRequest::Exit;  // graceful exit in release mode
                }
                if (pNextBestWpnFound == player.getWeaponManager().getCurrentWeapon())
                {
                    m_gui.getItemPickupEvents()->addEvent("Auto-Switch: No better loaded weapon found!");
                    //getConsole().EOLn("InputHandling::%s(): auto switch to best with mag ammo: did not find better!", __func__);
                    cWeaponSwitch = '\0'; // did not found better wpn so set key back to null, so we dont send switch request to server
                }
                else
                {
                    const auto itCVarWpnName = pNextBestWpnFound->getVars().find("name");
                    if (itCVarWpnName != pNextBestWpnFound->getVars().end())
                    {
                        m_gui.getItemPickupEvents()->addEvent("Auto-Switch to loaded " + itCVarWpnName->second.getAsString());
                    }
                }
            }

            if (bAutoSwitchToBestWithAnyKindOfAmmoWasRequested ||
                /* 2nd part of condition is: could not find better loaded wpn, so with best-effort we are trying to find a reloadable */
                (bAutoSwitchToBestLoadedWasRequested && (pNextBestWpnFound == player.getWeaponManager().getCurrentWeapon())))
            {
                //getConsole().EOLn("InputHandling::%s(): trying auto switch to best with any ammo ...", __func__);
                pNextBestWpnFound = player.getWeaponManager().getNextBestAvailableWeapon(cWeaponSwitch, false /* must have either mag or unmag bullet */);
                if (pNextBestWpnFound == nullptr)
                {
                    getConsole().EOLn("InputHandling::%s(): SHOULD NOT HAPPEN: getNextBestAvailableWeapon() 2 returned nullptr!", __func__);
                    assert(false);  // crash in debug
                    return proofps_dd::InputHandling::PlayerAppActionRequest::Exit;  // graceful exit in release mode
                }
                if (pNextBestWpnFound == player.getWeaponManager().getCurrentWeapon())
                {
                    m_gui.getItemPickupEvents()->addEvent("Auto-Switch: No better reloadable weapon found!");
                    //getConsole().EOLn("InputHandling::%s(): auto switch to best with any ammo: did not find better!", __func__);
                    cWeaponSwitch = '\0'; // did not found better wpn so set key back to null, so we dont send switch request to server
                }
                else
                {
                    const auto itCVarWpnName = pNextBestWpnFound->getVars().find("name");
                    if (itCVarWpnName != pNextBestWpnFound->getVars().end())
                    {
                        m_gui.getItemPickupEvents()->addEvent("Auto-Switch to reloadable " + itCVarWpnName->second.getAsString());
                    }
                }
            }

            if ((bAutoSwitchToBestWithAnyKindOfAmmoWasRequested || bAutoSwitchToBestLoadedWasRequested) &&
                /* either of auto-switches is set but still no better wpn found, check if current could be auto-reloaded?
                   Remember, we are here because reload was not requested. Thus, this reload is really last-resort. */
                (pNextBestWpnFound == player.getWeaponManager().getCurrentWeapon()))
            {
                // pNextBestWpnFound cannot be null, because we have explicit null-handling blocks in above cases
                assert(pNextBestWpnFound != nullptr);
                if (pNextBestWpnFound->getUnmagBulletCount() != 0)
                {
                    m_gui.getItemPickupEvents()->addEvent("Auto-Switch: Last-resort Auto-Reload");
                    //getConsole().EOLn("InputHandling::%s(): auto switch: did not find better, doing last-resort auto-reload!", __func__);
                    bRequestReload = true;
                }
            }
            else if ((cWeaponSwitch == '\0') && !bFireButtonPressed)
            {
                /* none of above firing-induced auto-switch methods selected any weapon, and no last-resort auto-reload was initiated either */

                if (pWeaponPickupInducedAutoSwitchWasRequested)
                {
                    /* we picked up ammo/weapon, and per config auto-switch to it was requested */

                    const auto itKeypressToWeaponMapCurrentWeapon = std::find_if(
                        WeaponManager::getKeypressToWeaponMap().begin(),
                        WeaponManager::getKeypressToWeaponMap().end(),
                        [pWeaponPickupInducedAutoSwitchWasRequested](const auto& keyWpnPair) { return keyWpnPair.second == pWeaponPickupInducedAutoSwitchWasRequested->getFilename(); });

                    if (itKeypressToWeaponMapCurrentWeapon == WeaponManager::getKeypressToWeaponMap().end())
                    {
                        getConsole().EOLn("PRooFPSddPGE::%s(): could not set cWeaponSwitch based on %s!",
                            __func__, pWeaponPickupInducedAutoSwitchWasRequested->getFilename().c_str());
                    }
                    else
                    {
                        m_gui.getItemPickupEvents()->addEvent("Auto-Switch: Picked Up " + pWeaponPickupInducedAutoSwitchWasRequested->getVars()["name"].getAsString());
                        //getConsole().EOLn("InputHandling::%s(): auto switch to picked up weapon: %s!", __func__, pWeaponPickupInducedAutoSwitchWasRequested->getFilename().c_str());
                        cWeaponSwitch = itKeypressToWeaponMapCurrentWeapon->first;
                    }
                }
                else
                {
                    // scan for any manual switch request
                    for (const auto& keyWpnPair : WeaponManager::getKeypressToWeaponMap())
                    {
                        if (m_pge.getInput().getKeyboard().isKeyPressedOnce(keyWpnPair.first, m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds))
                        {
                            const Weapon* const pTargetWpn = player.getWeaponManager().getWeaponByFilename(keyWpnPair.second);
                            if (!pTargetWpn)
                            {
                                getConsole().EOLn("InputHandling::%s(): not found weapon by name: %s!",
                                    __func__, keyWpnPair.second.c_str());
                                break;
                            }
                            if (!pTargetWpn->isAvailable())
                            {
                                //getConsole().OLn("InputHandling::%s(): weapon %s not available!",
                                //    __func__, key.second.c_str());
                                break;
                            }
                            if (pTargetWpn != player.getWeaponManager().getCurrentWeapon())
                            {
                                cWeaponSwitch = keyWpnPair.first;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    // at this point, if cWeaponSwitch is NOT nullchar, we really want to switch to something else either manually or auto
    //if (cWeaponSwitch != '\0')
    //{
    //    getConsole().EOLn("InputHandling::%s(): send request to switch to: %s!", __func__, std::to_string(cWeaponSwitch).c_str());
    //}

    if ((m_prevStrafe != m_strafe) || (m_bPrevCrouch != m_bCrouch) || bSendJumpAction || bToggleRunWalk || bRequestReload || (cWeaponSwitch != '\0'))
    {
        // strafe is a continuous operation: once started, server is strafing the player in every tick until client explicitly says so, thus
        // we need to send Strafe::NONE as well to server if user released the key. Other keyboard operations are non-continuous hence we handle them
        // as one-time actions.
        proofps_dd::MsgUserCmdFromClient::setKeybd(pkt, m_strafe, bSendJumpAction, bToggleRunWalk, m_bCrouch, bRequestReload, cWeaponSwitch);
    }
    m_prevStrafe = m_strafe;
    m_bPrevCrouch = m_bCrouch;

    return proofps_dd::InputHandling::PlayerAppActionRequest::None;
}

proofps_dd::InputHandling::PlayerAppActionRequest proofps_dd::InputHandling::clientKeyboardWhenDisconnectedFromServer()
{
    if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_ESCAPE))
    {
        return proofps_dd::InputHandling::PlayerAppActionRequest::Exit;
    }
    return proofps_dd::InputHandling::PlayerAppActionRequest::None;
}

/**
    @return True in case there was mouse (xhair) movement, false otherwise.
*/
bool proofps_dd::InputHandling::clientMouseWhenConnectedToServer(
    proofps_dd::GameMode& gameMode,
    pge_network::PgePacket& pkt,
    proofps_dd::Player& player,
    PureObject3D& objXHair)
{
    // we should always read the wheel data as often as possible, because this way we can avoid
    // the amount of wheel rotation accumulating too much
    const short int nMouseWheelChange = m_pge.getInput().getMouse().getWheel();

    // active in-game menu cancels any player control, and
    // in-game menu input is ignored here after in-game menu is closed
    static bool bRequireMouseUpBeforeAcceptingPlayerInput = false;
    if (bRequireMouseUpBeforeAcceptingPlayerInput ||
        (m_gui.getInGameMenuState() != GUI::InGameMenuState::None))
    {
        if (m_pge.getInput().getMouse().isButtonPressed())
        {
            bRequireMouseUpBeforeAcceptingPlayerInput = true; // sticky until all mouse buttons are released
            return false;
        }
        bRequireMouseUpBeforeAcceptingPlayerInput = false;
    }
    if (bRequireMouseUpBeforeAcceptingPlayerInput ||
        (m_gui.getInGameMenuState() != GUI::InGameMenuState::None))
    {
        return false;
    }

    if (!gameMode.isPlayerAllowedForGameplay(player))
    {
        // not error, valid state, maybe player not selected team yet
        return false;
    }

    if (gameMode.isGameWon())
    {
        return false;
    }

    const bool bFireButtonPressed = m_pge.getInput().getMouse().isButtonPressed(PGEInputMouse::MouseButton::MBTN_LEFT);

    bool bShootActionBeingSent = false;
    const auto nSecsSinceLastWeaponSwitchMillisecs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - player.getWeaponManager().getTimeLastWeaponSwitch()
        ).count();
    if (bFireButtonPressed)
    {
        // sending m_pge.getInput().getMouse() action is still allowed when player is dead, since server will treat that
        // as respawn request
        if (nSecsSinceLastWeaponSwitchMillisecs < m_nWeaponActionMinimumWaitMillisecondsAfterSwitch)
        {
            //getConsole().OLn("InputHandling::%s(): ignoring too early m_pge.getInput().getMouse() action!", __func__);
        }
        else
        {
            m_bAttack = true;
            if (m_bAttack != m_bPrevAttack)
            {
                proofps_dd::MsgUserCmdFromClient::setMouse(pkt, m_bAttack);
                bShootActionBeingSent = true;

                if (std::as_const(player).getHealth() > 0)
                {
                    // this is very bad, but I decided to play weapon dry fire here, because:
                    // - client does not get response from server for dry fire;
                    // - client also has enough data to decide if fire will be dry or not.
                    // Ideally, the pullTrigger() executed on server side should generate transparent traffic towards client which
                    // would trigger the dry fire sound, but such mechanism does not exist currently.
                    // Downside of this approach is that this can be heard only by current player and not by other players around.
                    Weapon* const wpn = player.getWeaponManager().getCurrentWeapon();
                    if (wpn && (wpn->getState() == Weapon::WPN_READY) && (wpn->getMagBulletCount() == 0))
                    {
                        const auto sndWpnDryFireHandle = m_pge.getAudio().play3dSound(wpn->getDryFiringSound(), player.getPos().getNew());
                        m_pge.getAudio().getAudioEngineCore().set3dSourceMinMaxDistance(sndWpnDryFireHandle, SndWpnDryFireDistMin, SndWpnDryFireDistMax);
                        m_pge.getAudio().getAudioEngineCore().set3dSourceAttenuation(sndWpnDryFireHandle, SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
                    }
                }
                else
                {
                    m_gui.fastForwardRespawnTimer(nPlayerRespawnCountdownFastForwardByClickingMillisecs);
                }
            }
        }
    }
    else
    {
        m_bAttack = false;
        if (m_bAttack != m_bPrevAttack)
        {
            proofps_dd::MsgUserCmdFromClient::setMouse(pkt, m_bAttack);
        }
    }
    m_bPrevAttack = m_bAttack;

    const auto& playerConst = player;
    if (playerConst.getHealth() == 0)
    {
        return false;
    }

    if (!bFireButtonPressed && !proofps_dd::MsgUserCmdFromClient::getReloadRequest(pkt))
    {
        if (nSecsSinceLastWeaponSwitchMillisecs >= m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds)
        {
            clientMouseWheel(nMouseWheelChange, pkt, player);
        }
    }

    const int oldmx = m_pge.getInput().getMouse().getCursorPosX();
    const int oldmy = m_pge.getInput().getMouse().getCursorPosY();

    auto& window = m_pge.getPure().getWindow();
    m_pge.getInput().getMouse().SetCursorPos(
        window.getX() + window.getWidth() / 2,
        window.getY() + window.getHeight() / 2);

    const int dx = oldmx - m_pge.getInput().getMouse().getCursorPosX();
    const int dy = oldmy - m_pge.getInput().getMouse().getCursorPosY();

    static bool bInitialXHairPosForTestingApplied = false;
    static std::chrono::time_point<std::chrono::steady_clock> timeInitialXHairPosForTestingApplied;
    if (!bInitialXHairPosForTestingApplied && m_pge.getConfigProfiles().getVars()["testing"].getAsBool())
    {
        const auto nSecsSinceInitialXHairPosForTestingApplied =
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - timeInitialXHairPosForTestingApplied).count();
        // if we immediately apply cursor pos, it might be changed to center a few moments later at startup so we need to wait a bit
        if (nSecsSinceInitialXHairPosForTestingApplied >= 2)
        {
            getConsole().EOLn("InputHandling::%s(): Testing: Initial Mouse Cursor pos applied!", __func__);
            bInitialXHairPosForTestingApplied = true;
            timeInitialXHairPosForTestingApplied = std::chrono::steady_clock::now();
            if (m_pge.getNetwork().isServer())
            {
                objXHair.getPosVec().Set(100.f, -100.f, objXHair.getPosVec().getZ());
            }
            else
            {
                objXHair.getPosVec().Set(-100.f, -100.f, objXHair.getPosVec().getZ());
            }
            return true;
        }
        return false;
    }

    if ((dx == 0) && (dy == 0))
    {
        return false;
    }

    // I'm thinking we should not come here at all if testing CVAR is set.

    // at this point, we are not under testing, there was real mouse move we need to apply to xhair
    const float fCursorNewX = std::min(
        static_cast<float>(window.getClientWidth() / 2),
        std::max(-static_cast<float>(window.getClientWidth() / 2), objXHair.getPosVec().getX() + dx));

    const float fCursorNewY = std::min(
        static_cast<float>(window.getClientHeight() / 2),
        std::max(-static_cast<float>(window.getClientHeight() / 2), objXHair.getPosVec().getY() - dy));

    //getConsole().EOLn("InputHandling::%s(): objXHair x: %f, y: %f!", __func__, fCursorNewX, fCursorNewY);
    objXHair.getPosVec().Set(fCursorNewX, fCursorNewY, 0.f);

    return true;
}

void proofps_dd::InputHandling::clientUpdatePlayerAsPerInputAndSendUserCmdMoveToServer(
    pge_network::PgePacket& pkt,
    proofps_dd::Player& player,
    PureObject3D& objXHair,
    proofps_dd::GameMode& gameMode)
{
    if (!gameMode.isPlayerAllowedForGameplay(player))
    {
        // not error, valid state, maybe player not selected team yet
        return;
    }

    static std::chrono::time_point<std::chrono::steady_clock> timeLastMsgUserCmdFromClientSent;
    const auto nMillisecsSinceLastMsgUserCmdFromClientSent =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timeLastMsgUserCmdFromClientSent).count();

    Weapon* const wpn = player.getWeaponManager().getCurrentWeapon();
    if (player.isSomersaulting())
    {
        if (wpn)
        {
            // during somersaulting, weapon and player angles are NOT controlled by player input but by Physics class
            wpn->UpdatePosition(player.getObject3D()->getPosVec(), true);
        }
    }
    else
    {
        if (wpn)
        {
            // my xhair is used to update weapon angle
            wpn->UpdatePositions(player.getObject3D()->getPosVec(), objXHair.getPosVec());
            // PPPKKKGGGGGG
            player.getWeaponAngle().set(
                PureVector(0.f, wpn->getObject3D().getAngleVec().getY(), wpn->getObject3D().getAngleVec().getZ())
            );

            // TODO: on the long run there should be a more general function that calculates angle because now player angle also depends on
            // weapon angle, however in future we might end up not having a weapon!

            // player should also look in the same horizontal direction as the weapon
            player.getAngleY() = wpn->getObject3D().getAngleVec().getY();
        }
        else
        {
            // this is just a fallback case if for any reason we dont have a weapon
            player.getAngleY() = (objXHair.getPosVec().getX() < 0.f) ? 0.f : 180.f;
        }
        player.getObject3D()->getAngleVec().SetY(player.getAngleY());
    }

    /* following condition is example of simple rate-limiting with time interval */
    const bool bMustSendPlayerAngleY =
        (m_fLastPlayerAngleYSent != player.getAngleY().getNew()) &&
        (nMillisecsSinceLastMsgUserCmdFromClientSent >= m_nPlayerAngleYSendIntervalMilliseconds);

    /* following condition is example of a more sophisticated rate-limiting, with time interval combined with threshold */
    const bool bMustSendWeaponAngleZ =
        (m_fLastWeaponAngleZSent != player.getWeaponAngle().getNew().getZ()) &&
        (((nMillisecsSinceLastMsgUserCmdFromClientSent >= m_nWeaponAngleZBigChangeSendIntervalMilliseconds) &&
            (abs(m_fLastWeaponAngleZSent - player.getWeaponAngle().getNew().getZ()) >= m_fWeaponAngleZBigChangeThreshold)) ||
            (nMillisecsSinceLastMsgUserCmdFromClientSent >= m_nWeaponAngleZSmallChangeSendIntervalMilliseconds));
    
    // This condition is the combined form of the same multiple conditions in previous commit, and this is a step towards generalizing.
    // TODO: In the future we can introduce a generalized way of defining the rate-limited variables, and then we won't need these special conditions
    // and time difference calculations to be written here in game logic.
    if (proofps_dd::MsgUserCmdFromClient::shouldSend(pkt) ||
        bMustSendPlayerAngleY ||
        bMustSendWeaponAngleZ)
    {
        m_fLastPlayerAngleYSent = player.getAngleY().getNew();
        proofps_dd::MsgUserCmdFromClient::setAngleY(pkt, m_fLastPlayerAngleYSent);

        m_fLastWeaponAngleZSent = player.getWeaponAngle().getNew().getZ();
        proofps_dd::MsgUserCmdFromClient::setWpnAngles(pkt, m_fLastWeaponAngleZSent);

        //getConsole().EOLn("InputHandling::%s(): sending pkt with bRequestReload %b", __func__, proofps_dd::MsgUserCmdFromClient::getReloadRequest(pkt));

        // shouldSend() at this point means that there were actual change in user input so MsgUserCmdFromClient will be sent out.
        // Instead of using sendToServer() of getClient() or inject() of getServer() instances, we use the send() of
        // their common interface which always points to the initialized instance, which is either client or server.
        // Btw send() in case of server instance and server as target is implemented as an inject() as of May 2023.
        m_pge.getNetwork().getServerClientInstance()->send(pkt);
        timeLastMsgUserCmdFromClientSent = std::chrono::steady_clock::now();
    }
}

void proofps_dd::InputHandling::clientMouseWheel(
    const short int& nMouseWheelChange,
    pge_network::PgePacket& pkt,
    proofps_dd::Player& player)
{
    if (proofps_dd::MsgUserCmdFromClient::getWeaponSwitch(pkt) != '\0')
    {
        return;
    }

    if (nMouseWheelChange == 0)
    {
        return;
    }

    // if we dont shoot, and weapon switch not yet initiated by keyboard, we
    // are allowed to process mousewheel event for changing weapon
    //getConsole().OLn("InputHandling::%s(): mousewheel: %d!", __func__, nMouseWheelChange);

    unsigned char cTargetWeapon;
    const Weapon* pWpnTarget;
    // I dont know if it is a good approach to just check only the sign of the change, and based on that,
    // move 1 step forward or backward in weapons list ... or maybe I should move n steps based on the exact amount ...
    if (nMouseWheelChange > 0)
    {
        // wheel rotated forward, in CS it means going forward in the list;
        pWpnTarget = player.getWeaponManager().getNextAvailableWeapon(cTargetWeapon);
    }
    else
    {
        // wheel rotated backward, in CS it means going backward in the list;
        pWpnTarget = player.getWeaponManager().getPrevAvailableWeapon(cTargetWeapon);
    }

    if (pWpnTarget == player.getWeaponManager().getCurrentWeapon())
    {
        getConsole().OLn("InputHandling::%s(): no next available weapon found!", __func__);
    }
    else
    {
        proofps_dd::MsgUserCmdFromClient::SetWeaponSwitch(pkt, cTargetWeapon);
        getConsole().OLn("InputHandling::%s(): next weapon is: %s!", __func__, pWpnTarget->getFilename().c_str());
    }
}

const char* proofps_dd::InputHandling::getMsgAppIdName(const proofps_dd::PRooFPSappMsgId& id)
{
    if (static_cast<size_t>(id) < proofps_dd::MapMsgAppId2String.size())
    {
        return proofps_dd::MapMsgAppId2String[static_cast<size_t>(id)].zstring;
    }
    return "UNKNOWN_MSG";
}

const size_t proofps_dd::InputHandling::getLongestMsgAppIdNameLength()
{
    size_t nLongestLength = 0;
    for (const auto& msgId2ZStringPair : proofps_dd::MapMsgAppId2String)
    {
        const size_t currLen = strlen(msgId2ZStringPair.zstring);
        if (currLen > nLongestLength)
        {
            nLongestLength = currLen;
        }
    }
    return nLongestLength;
}

void proofps_dd::InputHandling::regTestDumpToFile(
    proofps_dd::GameMode& gameMode,
    proofps_dd::Player& player,
    const unsigned int nTickrate,
    const unsigned int nClUpdateRate,
    const unsigned int nPhysicsRateMin)
{
    const std::string sRegTestDumpFilename = proofps_dd::generateTestDumpFilename(
        m_pge.getNetwork().isServer(), static_cast<unsigned long>(_getpid()), nTickrate, nClUpdateRate, nPhysicsRateMin);
    std::ofstream fRegTestDump(sRegTestDumpFilename);
    if (fRegTestDump.fail())
    {
        getConsole().EOLn("%s ERROR: couldn't create file: %s", __func__, sRegTestDumpFilename.c_str());
        return;
    }

    getConsole().OLn("%s Dumping to file: %s ...", __func__, sRegTestDumpFilename.c_str());

    fRegTestDump << "Tx: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << std::setw(3) << m_pge.getNetwork().getServerClientInstance()->getTxPacketCount() << std::endl;
    fRegTestDump << "  " << std::setw(3) << m_pge.getNetwork().getServerClientInstance()->getTxPacketPerSecondCount() << std::endl;
    fRegTestDump << "Rx: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << std::setw(3) << m_pge.getNetwork().getServerClientInstance()->getRxPacketCount() << std::endl;
    fRegTestDump << "  " << std::setw(3) << m_pge.getNetwork().getServerClientInstance()->getRxPacketPerSecondCount() << std::endl;
    fRegTestDump << "Inject: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << std::setw(3) << m_pge.getNetwork().getServerClientInstance()->getInjectPacketCount() << std::endl;
    fRegTestDump << "  " << std::setw(3) << m_pge.getNetwork().getServerClientInstance()->getInjectPacketPerSecondCount() << std::endl;

    fRegTestDump << std::endl;
    fRegTestDump << "Tx: Total Byte Count" << std::endl;
    fRegTestDump << "  " << std::setw(10) << m_pge.getNetwork().getServerClientInstance()->getTxByteCount() << std::endl;

    fRegTestDump << "Rx: Total Byte Count" << std::endl;
    fRegTestDump << "  " << std::setw(10) << m_pge.getNetwork().getServerClientInstance()->getRxByteCount() << std::endl;

    fRegTestDump << "Inject: Total Byte Count" << std::endl;
    fRegTestDump << "  " << std::setw(10) << m_pge.getNetwork().getServerClientInstance()->getInjectByteCount() << std::endl;

    const size_t nLongestMsgAppIdNameLength = getLongestMsgAppIdNameLength();
    fRegTestDump << std::endl;
    fRegTestDump << "Total Tx'd App Msg Count per AppMsgId:" << std::endl;
    for (const auto& txMsgCount : m_pge.getNetwork().getServerClientInstance()->getTxMsgCount())
    {
        fRegTestDump << "  Id " << std::right << std::setw(2) << txMsgCount.first << " " << std::left << std::setw(nLongestMsgAppIdNameLength + 1)
            << getMsgAppIdName(static_cast<proofps_dd::PRooFPSappMsgId>(txMsgCount.first))
            << ": " << std::right << std::setw(3) << txMsgCount.second << std::endl;
    }
    // add an extra empty line, so the regression test can easily detect end of AppMsgId count list
    fRegTestDump << std::endl;

    fRegTestDump << "Total Rx'd App Msg Count per AppMsgId:" << std::endl;
    for (const auto& rxMsgCount : m_pge.getNetwork().getServerClientInstance()->getRxMsgCount())
    {
        fRegTestDump << "  Id " << std::right << std::setw(2) << rxMsgCount.first << " " << std::left << std::setw(nLongestMsgAppIdNameLength + 1)
            << getMsgAppIdName(static_cast<proofps_dd::PRooFPSappMsgId>(rxMsgCount.first))
            << ": " << std::right << std::setw(3) << rxMsgCount.second << std::endl;
    }
    // add an extra empty line, so the regression test can easily detect end of AppMsgId count list
    fRegTestDump << std::endl;

    fRegTestDump << "Total Inj'd App Msg Count per AppMsgId:" << std::endl;
    for (const auto& injectMsgCount : m_pge.getNetwork().getServerClientInstance()->getInjectMsgCount())
    {
        fRegTestDump << "  Id " << std::right << std::setw(2) << injectMsgCount.first << " " << std::left << std::setw(nLongestMsgAppIdNameLength + 1)
            << getMsgAppIdName(static_cast<proofps_dd::PRooFPSappMsgId>(injectMsgCount.first))
            << ": " << std::right << std::setw(3) << injectMsgCount.second << std::endl;
    }
    // add an extra empty line, so the regression test can easily detect end of AppMsgId count list
    fRegTestDump << std::endl;

    if (gameMode.isTeamBasedGame())
    {
        fRegTestDump << "Frag Table: Team Total Frags, then for each Player: [Player Name, Team, Frags, Deaths, Suicides, Aim Accuracy, Shots Fired]" << std::endl;
        const proofps_dd::TeamDeathMatchMode* const pTeamDeathMatchMode = dynamic_cast<proofps_dd::TeamDeathMatchMode*>(&gameMode);
        if (!pTeamDeathMatchMode)
        {
            getConsole().EOLnOO("ERROR: pTeamDeathMatchMode null!");
            return;
        }
        fRegTestDump << pTeamDeathMatchMode->getTeamFrags(1) << std::endl;
        fRegTestDump << pTeamDeathMatchMode->getTeamFrags(2) << std::endl;
    }
    else
    {
        fRegTestDump << "Frag Table: for each Player: [Player Name, Team, Frags, Deaths, Suicides, Aim Accuracy, Shots Fired]" << std::endl;
    }
    
    for (const auto& fragTableRow : gameMode.getPlayersTable())
    {
        fRegTestDump << "  " << fragTableRow.m_sName << std::endl;
        fRegTestDump << "  " << std::right << std::setw(fragTableRow.m_sName.length()) << fragTableRow.m_iTeamId << std::endl;
        fRegTestDump << "  " << std::right << std::setw(fragTableRow.m_sName.length()) << fragTableRow.m_nFrags << std::endl;
        fRegTestDump << "  " << std::right << std::setw(fragTableRow.m_sName.length()) << fragTableRow.m_nDeaths << std::endl;
        fRegTestDump << "  " << std::right << std::setw(fragTableRow.m_sName.length()) << fragTableRow.m_nSuicides << std::endl;
        fRegTestDump << "  " << std::right << std::setw(fragTableRow.m_sName.length()) <<
            std::to_string(std::lroundf(fragTableRow.m_fFiringAcc * 100)) << std::endl;
        fRegTestDump << "  " << std::right << std::setw(fragTableRow.m_sName.length()) << fragTableRow.m_nShotsFired << std::endl;
    }

    // add an extra empty line, so the regression test can easily detect end of frag table
    fRegTestDump << std::endl;

    fRegTestDump << "Weapons Available: Weapon Filename, Mag Bullet Count, Unmag Bullet Count" << std::endl;
    for (const auto& wpn : player.getWeaponManager().getWeapons())
    {
        if (wpn->isAvailable())
        {
            fRegTestDump << "  " << wpn->getFilename() << std::endl;
            fRegTestDump << "  " << std::right << std::setw(wpn->getFilename().length()) << wpn->getMagBulletCount() << std::endl;
            fRegTestDump << "  " << std::right << std::setw(wpn->getFilename().length()) << wpn->getUnmagBulletCount() << std::endl;
        }
    }

    // add an extra empty line, so the regression test can easily detect end of weapon list
    fRegTestDump << std::endl;

    fRegTestDump << "Player Info: Health, Armor" << std::endl;
    const auto& playerConst = player;
    fRegTestDump << "  " << playerConst.getHealth().getNew() << std::endl;
    fRegTestDump << "  " << playerConst.getArmor().getNew() << std::endl;

    fRegTestDump.flush();
    fRegTestDump.close();

    getConsole().OLn("%s Dumping to file: %s FINISHED!", __func__, sRegTestDumpFilename.c_str());
}
