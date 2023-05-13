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

#include "InputHandling.h"

static const float GAME_PLAYER_SPEED1 = 2.0f;
static const float GAME_PLAYER_SPEED2 = 4.0f;


// ############################### PUBLIC ################################


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
    m_bShowGuiDemo(false)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations, mapPlayers, maps, sounds
    // But they can used in other functions.

    // Since this class is used to build up the PRooFPSddPGE class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. PRooFPSddPGE initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be extisting at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
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


void proofps_dd::InputHandling::keyboard(
    proofps_dd::GameMode& gameMode,
    int /*fps*/,
    bool& won,
    pge_network::PgePacket& pkt,
    proofps_dd::Player& player)
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
                RegTestDumpToFile(gameMode, player);
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

        proofps_dd::Strafe strafe = proofps_dd::Strafe::NONE;
        if (m_pge.getInput().getKeyboard().isKeyPressed(VK_LEFT) || m_pge.getInput().getKeyboard().isKeyPressed((unsigned char)VkKeyScan('a')))
        {
            strafe = proofps_dd::Strafe::LEFT;
        }
        if (m_pge.getInput().getKeyboard().isKeyPressed(VK_RIGHT) || m_pge.getInput().getKeyboard().isKeyPressed((unsigned char)VkKeyScan('d')))
        {
            strafe = proofps_dd::Strafe::RIGHT;
        }

        bool bSendJumpAction = false;
        if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_SPACE))
        {
            bSendJumpAction = true;
        }

        bool bToggleRunWalk = false;
        if (m_pge.getInput().getKeyboard().isKeyPressedOnce(VK_SHIFT))
        {
            bToggleRunWalk = true;
        }

        bool bRequestReload = false;
        if (m_pge.getInput().getKeyboard().isKeyPressedOnce((unsigned char)VkKeyScan('r')))
        {
            bRequestReload = true;
        }

        unsigned char cWeaponSwitch = '\0';
        if (!bRequestReload)
        {   // we dont care about wpn switch if reload is requested
            for (const auto& keyWpnPair : WeaponManager::getKeypressToWeaponMap())
            {
                if (m_pge.getInput().getKeyboard().isKeyPressedOnce(keyWpnPair.first))
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

        if ((strafe != proofps_dd::Strafe::NONE) || bSendJumpAction || bToggleRunWalk || bRequestReload || (cWeaponSwitch != '\0'))
        {
            proofps_dd::MsgUserCmdMove::setKeybd(pkt, strafe, bSendJumpAction, bToggleRunWalk, bRequestReload, cWeaponSwitch);
        }
    }
    else
    {

    } // won
}

bool proofps_dd::InputHandling::mouse(
    proofps_dd::GameMode& gameMode,
    int /*fps*/,
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

    static bool bPrevLeftButtonPressed = false; // I guess we could get rid of this if we introduced isButtonPressedOnce()
    bool bShootActionBeingSent = false;
    if (m_pge.getInput().getMouse().isButtonPressed(PGEInputMouse::MouseButton::MBTN_LEFT))
    {
        bPrevLeftButtonPressed = true;

        // sending m_pge.getInput().getMouse() action is still allowed when player is dead, since server will treat that
        // as respawn request

        const auto nSecsSinceLastWeaponSwitch =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - player.getWeaponManager().getTimeLastWeaponSwitch()
            ).count();
        if (nSecsSinceLastWeaponSwitch < m_nWeaponActionMinimumWaitMillisecondsAfterSwitch)
        {
            //getConsole().OLn("InputHandling::%s(): ignoring too early m_pge.getInput().getMouse() action!", __func__);
        }
        else
        {
            proofps_dd::MsgUserCmdMove::setMouse(pkt, true);
            bShootActionBeingSent = true;
        }
    }
    else
    {
        if (!proofps_dd::MsgUserCmdMove::getReloadRequest(pkt) && bPrevLeftButtonPressed)
        {
            bPrevLeftButtonPressed = false;
            proofps_dd::MsgUserCmdMove::setMouse(pkt, false);
        }
    }

    if (player.getHealth() == 0)
    {
        return false;
    }

    if (!bShootActionBeingSent && !proofps_dd::MsgUserCmdMove::getReloadRequest(pkt))
    {
        mouseWheel(nMouseWheelChange, pkt, player);
    }

    const int oldmx = m_pge.getInput().getMouse().getCursorPosX();
    const int oldmy = m_pge.getInput().getMouse().getCursorPosY();

    m_pge.getInput().getMouse().SetCursorPos(
        m_pge.getPure().getWindow().getX() + m_pge.getPure().getWindow().getWidth() / 2,
        m_pge.getPure().getWindow().getY() + m_pge.getPure().getWindow().getHeight() / 2);

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
        objXHair.getPosVec().Set(
            objXHair.getPosVec().getX() + dx,
            objXHair.getPosVec().getY() - dy,
            0.f);
    }

    return true;
}

bool proofps_dd::InputHandling::handleUserCmdMove(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const proofps_dd::MsgUserCmdMove& pktUserCmdMove)
{
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

    if ((pktUserCmdMove.m_strafe == proofps_dd::Strafe::NONE) &&
        (!pktUserCmdMove.m_bJumpAction) && (!pktUserCmdMove.m_bSendSwitchToRunning) &&
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

    if (pktUserCmdMove.m_bSendSwitchToRunning)
    {
        player.SetRun(!player.isRunning());
    }

    float fSpeed;
    if (player.isRunning())
    {
        fSpeed = GAME_PLAYER_SPEED2 / 60.0f;
    }
    else
    {
        fSpeed = GAME_PLAYER_SPEED1 / 60.0f;
    }

    if (pktUserCmdMove.m_strafe == proofps_dd::Strafe::LEFT)
    {
        if (!player.isJumping() && !player.isFalling() && player.jumpAllowed())
        {
            // PPPKKKGGGGGG
            player.getPos().set(
                PureVector(
                    player.getPos().getNew().getX() - fSpeed,
                    player.getPos().getNew().getY(),
                    player.getPos().getNew().getZ()
                ));
        }
    }
    if (pktUserCmdMove.m_strafe == proofps_dd::Strafe::RIGHT)
    {
        if (!player.isJumping() && !player.isFalling() && player.jumpAllowed())
        {
            // PPPKKKGGGGGG
            player.getPos().set(
                PureVector(
                    player.getPos().getNew().getX() + fSpeed,
                    player.getPos().getNew().getY(),
                    player.getPos().getNew().getZ()
                ));
        }
    }

    if (pktUserCmdMove.m_bJumpAction)
    {
        if (!player.isJumping() &&
            !player.isFalling())
        {
            player.Jump();
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

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    if (!pktUserCmdMove.m_bShootAction)
    {
        if (!wpn->isTriggerReleased())
        {
            //getConsole().OLn("InputHandling::%s(): player %s released trigger!", 
            //    __func__, sClientUserName.c_str());
        }
        wpn->releaseTrigger();
    }

    if (pktUserCmdMove.m_bRequestReload)
    {
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

    if (!pktUserCmdMove.m_bRequestReload && (wpn->getState() == Weapon::State::WPN_READY) && (pktUserCmdMove.m_cWeaponSwitch != '\0'))
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
            {   // server plays for itself
                m_pge.getAudio().play(m_sounds.m_sndChangeWeapon);
            }
            if (!player.getWeaponManager().setCurrentWeapon(pTargetWpn, true, m_pge.getNetwork().isServer()))
            {
                getConsole().EOLn("InputHandling::%s(): player %s switching to %s failed due to setCurrentWeapon() failed!",
                    __func__, sClientUserName.c_str(), itTargetWpn->second.c_str());
                assert(false);  // in debug mode, terminate the game
                return true;   // in release mode, dont terminate the server, just silently ignore!
            }

            //getConsole().OLn("InputHandling::%s(): player %s switching to %s!",
            //    __func__, sClientUserName.c_str(), itTargetWpn->second.c_str());

            // all clients must be updated about this player's weapon switch
            pge_network::PgePacket pktWpnUpdateCurrent;
            proofps_dd::MsgWpnUpdateCurrent::initPkt(
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

    // TODO: this should be moved up, so returning from function is easier for rest of action handling code
    player.getWeaponAngle().set(PureVector(0.f, pktUserCmdMove.m_fWpnAngleY, pktUserCmdMove.m_fWpnAngleZ));
    wpn->getObject3D().getAngleVec().SetY(pktUserCmdMove.m_fWpnAngleY);
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
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - player.getWeaponManager().getTimeLastWeaponSwitch()
            ).count();
        if (nSecsSinceLastWeaponSwitch < m_nWeaponActionMinimumWaitMillisecondsAfterSwitch)
        {
            //getConsole().OLn("InputHandling::%s(): ignoring too early mouse action!", __func__);
            // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
            m_durations.m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
            return true;
        }

        // server will have the new bullet, clients will learn about the new bullet when server is sending out
        // the regular bullet updates;
        if (wpn->pullTrigger())
        {
            // but we send out the wpn update for bullet count change here for that single client
            if (connHandleServerSide != pge_network::ServerConnHandle) // server doesn't need to send this msg to itself, it already executed bullet count change by pullTrigger()
            {
                pge_network::PgePacket pktWpnUpdate;
                proofps_dd::MsgWpnUpdate::initPkt(
                    pktWpnUpdate,
                    pge_network::ServerConnHandle /* ignored by client anyway */,
                    wpn->getFilename(),
                    wpn->isAvailable(),
                    wpn->getMagBulletCount(),
                    wpn->getUnmagBulletCount());
                m_pge.getNetwork().getServer().send(pktWpnUpdate, it->second.getServerSideConnectionHandle());
            }
            else
            {
                // here server plays the firing sound, clients play for themselves when they receive newborn bullet update
                // not nice, but this is just some temporal solution for private beta
                if (wpn->getFilename() == "pistol.txt")
                {
                    m_pge.getAudio().play(m_sounds.m_sndShootPistol);
                }
                else if (wpn->getFilename() == "machinegun.txt")
                {
                    m_pge.getAudio().play(m_sounds.m_sndShootMchgun);
                }
                else
                {
                    getConsole().EOLn("InputHandling::%s(): did not find correct weapon name for: %s!", __func__, wpn->getFilename().c_str());
                    assert(false);
                    return false;
                }
            }
        }
    }
    // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
    m_durations.m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

    return true;
}


// ############################### PRIVATE ###############################


void proofps_dd::InputHandling::mouseWheel(
    const short int& nMouseWheelChange,
    pge_network::PgePacket& pkt,
    proofps_dd::Player& player)
{
    if (proofps_dd::MsgUserCmdMove::getWeaponSwitch(pkt) != '\0')
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
        proofps_dd::MsgUserCmdMove::SetWeaponSwitch(pkt, cTargetWeapon);
        getConsole().OLn("InputHandling::%s(): next weapon is: %s!", __func__, pWpnTarget->getFilename().c_str());
    }
}

void proofps_dd::InputHandling::RegTestDumpToFile(
    proofps_dd::GameMode& gameMode,
    proofps_dd::Player& player)
{
    std::ofstream fRegTestDump(m_pge.getNetwork().isServer() ? proofps_dd::GAME_REG_TEST_DUMP_FILE_SERVER : proofps_dd::GAME_REG_TEST_DUMP_FILE_CLIENT);
    if (fRegTestDump.fail())
    {
        getConsole().EOLn("%s ERROR: couldn't create file: %s", __func__, m_pge.getNetwork().isServer() ? proofps_dd::GAME_REG_TEST_DUMP_FILE_SERVER : proofps_dd::GAME_REG_TEST_DUMP_FILE_CLIENT);
        return;
    }

    fRegTestDump << "Tx: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << m_pge.getNetwork().getServerClientInstance()->getTxPacketCount() << std::endl;
    fRegTestDump << "  " << m_pge.getNetwork().getServerClientInstance()->getTxPacketPerSecondCount() << std::endl;
    fRegTestDump << "Rx: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << m_pge.getNetwork().getServerClientInstance()->getRxPacketCount() << std::endl;
    fRegTestDump << "  " << m_pge.getNetwork().getServerClientInstance()->getRxPacketPerSecondCount() << std::endl;
    fRegTestDump << "Inject: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << m_pge.getNetwork().getServerClientInstance()->getInjectPacketCount() << std::endl;
    fRegTestDump << "  " << m_pge.getNetwork().getServerClientInstance()->getInjectPacketPerSecondCount() << std::endl;

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
        fRegTestDump << "  " << fragTableRow.m_nFrags << std::endl;
        fRegTestDump << "  " << fragTableRow.m_nDeaths << std::endl;
    }

    // add an extra empty line, so the regression test can easily detect end of frag table
    fRegTestDump << std::endl;

    fRegTestDump << "Weapons Available: Weapon Filename, Mag Bullet Count, Unmag Bullet Count" << std::endl;
    for (const auto& wpn : player.getWeaponManager().getWeapons())
    {
        if (wpn->isAvailable())
        {
            fRegTestDump << "  " << wpn->getFilename() << std::endl;
            fRegTestDump << "  " << wpn->getMagBulletCount() << std::endl;
            fRegTestDump << "  " << wpn->getUnmagBulletCount() << std::endl;
        }
    }

    // add an extra empty line, so the regression test can easily detect end of weapon list
    fRegTestDump << std::endl;

    fRegTestDump << "Player Info: Health" << std::endl;
    fRegTestDump << "  " << player.getHealth().getNew() << std::endl;

    fRegTestDump.flush();
    fRegTestDump.close();
}