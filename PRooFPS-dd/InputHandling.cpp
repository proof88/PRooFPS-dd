/*
    ###################################################################################
    InputHandling.cpp
    Input handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>
#include <chrono>
#include <iomanip>

#include "InputHandling.h"
#include "SharedWithTest.h"


// ############################### PUBLIC ################################


const unsigned int proofps_dd::InputHandling::m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds;
const unsigned int proofps_dd::InputHandling::m_nKeyPressOnceJumpMinumumWaitMilliseconds;
const unsigned int proofps_dd::InputHandling::m_nWeaponActionMinimumWaitMillisecondsAfterSwitch;

proofps_dd::InputHandling::InputHandling(
    PGE& pge,
    proofps_dd::Durations& durations,
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    proofps_dd::Maps& maps,
    proofps_dd::Sounds& sounds) :
    m_pge(pge),
    m_durations(durations),
    m_mapPlayers(mapPlayers),
    m_maps(maps),
    m_sounds(sounds),
    m_bShowGuiDemo(false),
    m_prevStrafe(proofps_dd::Strafe::NONE),
    m_strafe(proofps_dd::Strafe::NONE),
    m_fLastPlayerAngleYSent(-1.f),
    m_fLastWeaponAngleZSent(0.f)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations, mapPlayers, maps, sounds
    // But they can used in other functions.

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


void proofps_dd::InputHandling::handleInputAndSendUserCmdMove(
    proofps_dd::GameMode& gameMode,
    bool& won,
    proofps_dd::Player& player,
    PureObject3D& objXHair,
    const unsigned int nTickrate,
    const unsigned int nClUpdateRate,
    const unsigned int nPhysicsRateMin)
{
    pge_network::PgePacket pkt;
    /* we always init the pkt with the current strafe state so it is correctly sent to server even if we are not setting it
       in keyboard(), this is needed if only mouse() generates reason to send the pkt */
    proofps_dd::MsgUserCmdFromClient::initPkt(pkt, m_strafe, m_bAttack, m_fLastPlayerAngleYSent, m_fLastWeaponAngleZSent);

    keyboard(gameMode, won, pkt, player, nTickrate, nClUpdateRate, nPhysicsRateMin);
    mouse(gameMode, won, pkt, player, objXHair);
    updatePlayerAsPerInputAndSendUserCmdMove(won, pkt, player, objXHair);
}

bool proofps_dd::InputHandling::handleUserCmdMoveFromClient(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const proofps_dd::MsgUserCmdFromClient& pktUserCmdMove)
{
    //const int nRandom = PFL::random(0, 100);
    //getConsole().EOLn("InputHandling::%s(): new msg from connHandleServerSide: %u, strafe: %d, %d, shoot: %b!",
    //    __func__, connHandleServerSide, pktUserCmdMove.m_strafe, nRandom, pktUserCmdMove.m_bShootAction);

    if (!m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("InputHandling::%s(): client received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("InputHandling::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        assert(false);  // in debug mode this terminates server
        return true;    // in release mode, we dont terminate the server, just silently ignore
    }

    const std::string& sClientUserName = it->second.getName();

    if ((!pktUserCmdMove.m_bJumpAction) && (!pktUserCmdMove.m_bSendSwitchToRunning) &&
        (pktUserCmdMove.m_fPlayerAngleY == -1.f) && (!pktUserCmdMove.m_bRequestReload) && (!pktUserCmdMove.m_bShouldSend))
    {
        getConsole().EOLn("InputHandling::%s(): user %s sent invalid cmdMove!", __func__, sClientUserName.c_str());
        assert(false);  // in debug mode this terminates server
        return false;   // in release mode, we dont terminate the server, just silently ignore
        // TODO: I might disconnect this client!
    }

    auto& player = it->second;

    if (player.getHealth() == 0)
    {
        if (pktUserCmdMove.m_bShootAction)
        {
            //getConsole().OLn("InputHandling::%s(): user %s is requesting respawn", __func__, sClientUserName.c_str());
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
            static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - player.getTimeLastToggleRun()).count());
        if (nMillisecsSinceLastToggleRunning < m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds)
        {
            // should NOT had received this from client this early
            getConsole().OLn("InputHandling::%s(): player %s sent run toggle request too early, ignoring (actual: %u, req: %u)!",
                __func__, sClientUserName.c_str(), nMillisecsSinceLastToggleRunning, m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds);
            // Dont terminate for now, just log. Reason explained below at handling jumping.
            //assert(false);  // in debug mode, terminate the game
        }
        else
        {
            player.SetRun(!player.isRunning());
        }
    }

    // since v0.1.3 strafe is a continuous operation until client explicitly requests server to stop simulating it, so Strafe::NONE is always accepted.
    player.setStrafe(pktUserCmdMove.m_strafe);

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
    //        static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - timeStrafeStarted).count());
    //    const float fStrafeDistance = player.getPos().getNew().getX() - fPlayerPosXStarted;
    //
    //    getConsole().EOLn("Strafe duration: %u msecs, dist.: %f", nMillisecsStrafing, fStrafeDistance);
    //}

    if (pktUserCmdMove.m_bJumpAction)
    {
        if (!player.isJumping() &&
            !player.canFall())
        {
            const auto nMillisecsSinceLastJump =
                static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - player.getTimeLastSetWillJump()).count());
            if (nMillisecsSinceLastJump < m_nKeyPressOnceJumpMinumumWaitMilliseconds)
            {
                // should NOT had received this from client this early (actually could, see explanation below)
                getConsole().EOLn("InputHandling::%s(): player %s sent jump request too early, ignoring (actual: %u, req: %u)!",
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
                // Physics class, so inside there at the correct place Jump() will be invoked and correct forces will be saved.
                player.setWillJumpInNextTick(true);
            }
        }
    }

    if (pktUserCmdMove.m_fPlayerAngleY != -1.f)
    {
        player.getAngleY() = pktUserCmdMove.m_fPlayerAngleY;
        player.getObject3D()->getAngleVec().SetY(pktUserCmdMove.m_fPlayerAngleY);
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
                getConsole().EOLn("InputHandling::%s(): weapon not found for name %s!", __func__, itTargetWpn->second.c_str());
                assert(false);  // in debug mode, must abort because CLIENT should had not sent weapon switch request if they don't have this wpn!
                return true;    // in release mode, dont terminate the server, just silently ignore!
                // TODO: I might disconnect this client!
            }

            if (pTargetWpn != player.getWeaponManager().getCurrentWeapon())
            {
                if (connHandleServerSide == pge_network::ServerConnHandle)
                {   // server plays for itself because it doesnt inject the MsgCurrentWpnUpdateFromServer to itself
                    m_pge.getAudio().play(m_sounds.m_sndChangeWeapon);
                }
                if (!player.getWeaponManager().setCurrentWeapon(pTargetWpn, true, m_pge.getNetwork().isServer()))
                {
                    getConsole().EOLn("InputHandling::%s(): player %s switching to %s failed due to setCurrentWeapon() failed!",
                        __func__, sClientUserName.c_str(), itTargetWpn->second.c_str());
                    assert(false);  // in debug mode, terminate the game
                    return true;   // in release mode, dont terminate the server, just silently ignore!
                }
                it->second.getWeaponManager().getCurrentWeapon()->UpdatePosition(it->second.getObject3D()->getPosVec());

                //getConsole().OLn("InputHandling::%s(): player %s switching to %s!",
                //    __func__, sClientUserName.c_str(), itTargetWpn->second.c_str());

                // all clients must be updated about this player's weapon switch
                pge_network::PgePacket pktWpnUpdateCurrent;
                proofps_dd::MsgCurrentWpnUpdateFromServer::initPkt(
                    pktWpnUpdateCurrent,
                    connHandleServerSide,
                    pTargetWpn->getFilename());
                m_pge.getNetwork().getServer().sendToAllClientsExcept(pktWpnUpdateCurrent);
            }
            else
            {
                // should not happen because client should NOT send message in such case
                getConsole().OLn("InputHandling::%s(): player %s already has target wpn %s, CLIENT SHOULD NOT SEND THIS!",
                    __func__, sClientUserName.c_str(), itTargetWpn->second.c_str());
                assert(false);  // in debug mode, terminate the game
                return true;   // in release mode, dont terminate the server, just silently ignore!
                // TODO: I might disconnect this client!
            }
        }
    }

    // TODO: this should be moved up, so returning from function is easier for rest of action handling code
    player.getWeaponAngle().set(PureVector(0.f, player.getAngleY(), pktUserCmdMove.m_fWpnAngleZ));
    wpn->getObject3D().getAngleVec().SetY(player.getAngleY());
    wpn->getObject3D().getAngleVec().SetZ(pktUserCmdMove.m_fWpnAngleZ);

    if (pktUserCmdMove.m_bRequestReload || (wpn->getState() != Weapon::State::WPN_READY) || (pktUserCmdMove.m_cWeaponSwitch != '\0'))
    {
        // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
        m_durations.m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
        return true; // don't check anything related to shooting in case of either of these actions
    }

    if (pktUserCmdMove.m_bShootAction)
    {
        const auto nSecsSinceLastWeaponSwitch =
            static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStart - player.getWeaponManager().getTimeLastWeaponSwitch()
            ).count());
        if (nSecsSinceLastWeaponSwitch < m_nWeaponActionMinimumWaitMillisecondsAfterSwitch)
        {
            //getConsole().EOLn("InputHandling::%s(): ignoring too early mouse action!", __func__);
            // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
            m_durations.m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
            return true;
        }
        else
        {
            player.getAttack() = true;
        }
    }
    // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
    m_durations.m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

    return true;
}


// ############################### PRIVATE ###############################


void proofps_dd::InputHandling::keyboard(
    proofps_dd::GameMode& gameMode,
    bool& won,
    pge_network::PgePacket& pkt,
    proofps_dd::Player& player,
    const unsigned int nTickrate,
    const unsigned int nClUpdateRate,
    const unsigned int nPhysicsRateMin)
{
    if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_ESCAPE))
    {
        m_pge.getPure().getWindow().Close();
    }

    if (gameMode.checkWinningConditions())
    {
        return;
    }

    if (m_pge.getInput().getKeyboard().isKeyPressed(VK_TAB))
    {
        gameMode.showObjectives(m_pge.getPure(), m_pge.getNetwork());
    }

    if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_BACK))
    {
        m_bShowGuiDemo = !m_bShowGuiDemo;
        m_pge.getPure().ShowGuiDemo(m_bShowGuiDemo);
        m_pge.getPure().getWindow().SetCursorVisible(m_bShowGuiDemo);
    }

    if (m_bShowGuiDemo)
    {
        return;
    }

    if (player.getHealth() == 0)
    {
        return;
    }

    if (!won)
    {

        if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_RETURN))
        {
            if (m_pge.getConfigProfiles().getVars()["testing"].getAsBool())
            {
                getConsole().SetLoggingState("4LLM0DUL3S", true);
                RegTestDumpToFile(gameMode, player, nTickrate, nClUpdateRate, nPhysicsRateMin);
                getConsole().SetLoggingState("4LLM0DUL3S", false);
            }
        }

        if (m_pge.getInput().getKeyboard().isKeyPressedOnce((unsigned char)VkKeyScan('t')))
        {
            if (m_pge.getNetwork().isServer())
            {
                // for testing purpose only, we can teleport server player to random spawn point
                player.getPos() = m_maps.getRandomSpawnpoint();
                player.getRespawnFlag() = true;
            }

            // log some stats
            getConsole().SetLoggingState("PureRendererHWfixedPipe", true);
            m_pge.getPure().getRenderer()->ResetStatistics();
            getConsole().SetLoggingState("PureRendererHWfixedPipe", false);

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
            getConsole().OLn("   - SendUserUpdatesDuration: %f usecs", m_durations.m_nSendUserUpdatesDurationUSecs / static_cast<float>(m_durations.m_nFramesElapsedSinceLastDurationsReset));
            getConsole().OLn("");

            m_durations.reset();
        }

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

        bool bRequestReload = false;
        if (m_pge.getInput().getKeyboard().isKeyPressedOnce((unsigned char)VkKeyScan('r'), m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds))
        {
            bRequestReload = true;
        }

        unsigned char cWeaponSwitch = '\0';
        if (!bRequestReload)
        {   // we dont care about wpn switch if reload is requested
            const auto nSecsSinceLastWeaponSwitchMillisecs =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - player.getWeaponManager().getTimeLastWeaponSwitch()
                ).count();
            if (nSecsSinceLastWeaponSwitchMillisecs >= m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds)
            {
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

        if ((m_prevStrafe != m_strafe) || bSendJumpAction || bToggleRunWalk || bRequestReload || (cWeaponSwitch != '\0'))
        {
            // strafe is a continuous operation: once started, server is strafing the player in every tick until client explicitly says so, thus
            // we need to send Strafe::NONE as well to server if user released the key. Other keyboard operations are non-continuous hence we handle them
            // as one-time actions.
            proofps_dd::MsgUserCmdFromClient::setKeybd(pkt, m_strafe, bSendJumpAction, bToggleRunWalk, bRequestReload, cWeaponSwitch);
        }
        m_prevStrafe = m_strafe;
    }
    else
    {

    } // won
}

bool proofps_dd::InputHandling::mouse(
    proofps_dd::GameMode& gameMode,
    bool& /*won*/,
    pge_network::PgePacket& pkt,
    proofps_dd::Player& player,
    PureObject3D& objXHair)
{
    // we should always read the wheel data as often as possible, because this way we can avoid
    // the amount of wheel rotation accumulating too much
    const short int nMouseWheelChange = m_pge.getInput().getMouse().getWheel();

    if (gameMode.checkWinningConditions())
    {
        return false;
    }

    if (m_bShowGuiDemo)
    {
        return false;
    }

    bool bShootActionBeingSent = false;
    const auto nSecsSinceLastWeaponSwitchMillisecs =
        static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - player.getWeaponManager().getTimeLastWeaponSwitch()
        ).count());
    if (m_pge.getInput().getMouse().isButtonPressed(PGEInputMouse::MouseButton::MBTN_LEFT))
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

    if (player.getHealth() == 0)
    {
        return false;
    }

    if (!bShootActionBeingSent && !proofps_dd::MsgUserCmdFromClient::getReloadRequest(pkt))
    {
        if (nSecsSinceLastWeaponSwitchMillisecs >= m_nKeyPressOnceWpnHandlingMinumumWaitMilliseconds)
        {
            mouseWheel(nMouseWheelChange, pkt, player);
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

    if ((dx == 0) && (dy == 0))
    {
        return false;
    }

    static bool bInitialXHairPosForTestingApplied = false;
    if (!bInitialXHairPosForTestingApplied && m_pge.getConfigProfiles().getVars()["testing"].getAsBool())
    {
        getConsole().OLn("InputHandling::%s(): Testing: Initial Mouse Cursor pos applied!", __func__);
        bInitialXHairPosForTestingApplied = true;
        if (m_pge.getNetwork().isServer())
        {
            objXHair.getPosVec().Set(100.f, 0.f, objXHair.getPosVec().getZ());
        }
        else
        {
            objXHair.getPosVec().Set(-100.f, 0.f, objXHair.getPosVec().getZ());
        }
    }
    else
    {
        const float fCursorNewX = std::min(
            static_cast<float>(window.getClientWidth() / 2),
            std::max(-static_cast<float>(window.getClientWidth() / 2), objXHair.getPosVec().getX() + dx));

        const float fCursorNewY = std::min(
            static_cast<float>(window.getClientHeight() / 2),
            std::max(-static_cast<float>(window.getClientHeight() / 2), objXHair.getPosVec().getY() - dy));

        objXHair.getPosVec().Set(
            fCursorNewX,
            fCursorNewY,
            0.f);
    }

    return true;
}

void proofps_dd::InputHandling::updatePlayerAsPerInputAndSendUserCmdMove(
    bool& /*won*/,
    pge_network::PgePacket& pkt,
    proofps_dd::Player& player, PureObject3D& objXHair)
{
    player.getAngleY() = (objXHair.getPosVec().getX() < 0.f) ? 0.f : 180.f;
    player.getObject3D()->getAngleVec().SetY(player.getAngleY());
    
    static std::chrono::time_point<std::chrono::steady_clock> timeLastMsgUserCmdFromClientSent;
    const auto nMillisecsSinceLastMsgUserCmdFromClientSent =
        static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timeLastMsgUserCmdFromClientSent).count());

    Weapon* const wpn = player.getWeaponManager().getCurrentWeapon();
    if (wpn)
    {
        // my xhair is used to update weapon angle
        wpn->UpdatePositions(player.getObject3D()->getPosVec(), objXHair.getPosVec());
        // PPPKKKGGGGGG
        player.getWeaponAngle().set(
            PureVector(0.f, wpn->getObject3D().getAngleVec().getY(), wpn->getObject3D().getAngleVec().getZ())
        );
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

        // shouldSend() at this point means that there were actual change in user input so MsgUserCmdFromClient will be sent out.
        // Instead of using sendToServer() of getClient() or inject() of getServer() instances, we use the send() of
        // their common interface which always points to the initialized instance, which is either client or server.
        // Btw send() in case of server instance and server as target is implemented as an inject() as of May 2023.
        m_pge.getNetwork().getServerClientInstance()->send(pkt);
        timeLastMsgUserCmdFromClientSent = std::chrono::steady_clock::now();
    }
}

void proofps_dd::InputHandling::mouseWheel(
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

void proofps_dd::InputHandling::RegTestDumpToFile(
    proofps_dd::GameMode& gameMode,
    proofps_dd::Player& player,
    const unsigned int nTickrate,
    const unsigned int nClUpdateRate,
    const unsigned int nPhysicsRateMin)
{
    const std::string sRegTestDumpFilename = proofps_dd::generateTestDumpFilename(
        m_pge.getNetwork().isServer(), nTickrate, nClUpdateRate, nPhysicsRateMin);
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

    proofps_dd::DeathMatchMode* const pDeathMatchMode = dynamic_cast<proofps_dd::DeathMatchMode*>(&gameMode);
    if (!pDeathMatchMode)
    {
        getConsole().EOLnOO("ERROR: pDeathMatchMode null!");
        return;
    }

    fRegTestDump << "Frag Table: Player Name, Frags, Deaths" << std::endl;
    for (const auto& fragTableRow : pDeathMatchMode->getFragTable())
    {
        fRegTestDump << "  " << fragTableRow.m_sName << std::endl;
        fRegTestDump << "  " << std::right << std::setw(fragTableRow.m_sName.length()) << fragTableRow.m_nFrags << std::endl;
        fRegTestDump << "  " << std::right << std::setw(fragTableRow.m_sName.length()) << fragTableRow.m_nDeaths << std::endl;
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

    fRegTestDump << "Player Info: Health" << std::endl;
    fRegTestDump << "  " << player.getHealth().getNew() << std::endl;

    fRegTestDump.flush();
    fRegTestDump.close();

    getConsole().OLn("%s Dumping to file: %s FINISHED!", __func__, sRegTestDumpFilename.c_str());
}
