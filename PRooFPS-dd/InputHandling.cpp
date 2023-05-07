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

#include "InputHandling.h"


// ############################### PUBLIC ################################


const unsigned int proofps_dd::InputHandling::m_nWeaponActionMinimumWaitMillisecondsAfterSwitch;

proofps_dd::InputHandling::InputHandling(
    PGEcfgProfiles& cfg,
    PGEInputKeyboard& keybd,
    PGEInputMouse& mouse,
    pge_network::PgeNetwork& network,
    PR00FsUltimateRenderingEngine& gfx,
    proofps_dd::Durations& durations,
    proofps_dd::Maps& maps) :
    m_cfgProfiles(cfg),
    m_keyboard(keybd),
    m_mouse(mouse),
    m_network(network),
    m_gfx(gfx),
    m_durations(durations),
    m_maps(maps),
    m_bShowGuiDemo(false)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // durations, maps
    // But they can used in other functions.
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
    if (m_keyboard.isKeyPressedOnce(VK_ESCAPE))
    {
        m_gfx.getWindow().Close();
    }

    if (gameMode.checkWinningConditions())
    {
        return;
    }

    if (m_keyboard.isKeyPressed(VK_TAB))
    {
        gameMode.showObjectives(m_gfx, m_network);
    }

    if (m_keyboard.isKeyPressedOnce(VK_BACK))
    {
        m_bShowGuiDemo = !m_bShowGuiDemo;
        m_gfx.ShowGuiDemo(m_bShowGuiDemo);
        m_gfx.getWindow().SetCursorVisible(m_bShowGuiDemo);
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

        if (m_keyboard.isKeyPressedOnce(VK_RETURN))
        {
            if (m_cfgProfiles.getVars()["testing"].getAsBool())
            {
                RegTestDumpToFile(gameMode, player);
            }
        }

        if (m_keyboard.isKeyPressedOnce((unsigned char)VkKeyScan('t')))
        {
            if (m_network.isServer())
            {
                // for testing purpose only, we can teleport server player to random spawn point
                player.getPos() = m_maps.getRandomSpawnpoint();
                player.getRespawnFlag() = true;
            }

            // log some stats
            getConsole().SetLoggingState("PureRendererHWfixedPipe", true);
            m_gfx.getRenderer()->ResetStatistics();
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
        if (m_keyboard.isKeyPressed(VK_LEFT) || m_keyboard.isKeyPressed((unsigned char)VkKeyScan('a')))
        {
            strafe = proofps_dd::Strafe::LEFT;
        }
        if (m_keyboard.isKeyPressed(VK_RIGHT) || m_keyboard.isKeyPressed((unsigned char)VkKeyScan('d')))
        {
            strafe = proofps_dd::Strafe::RIGHT;
        }

        bool bSendJumpAction = false;
        if (m_keyboard.isKeyPressedOnce(VK_SPACE))
        {
            bSendJumpAction = true;
        }

        bool bToggleRunWalk = false;
        if (m_keyboard.isKeyPressedOnce(VK_SHIFT))
        {
            bToggleRunWalk = true;
        }

        bool bRequestReload = false;
        if (m_keyboard.isKeyPressedOnce((unsigned char)VkKeyScan('r')))
        {
            bRequestReload = true;
        }

        unsigned char cWeaponSwitch = '\0';
        if (!bRequestReload)
        {   // we dont care about wpn switch if reload is requested
            for (const auto& keyWpnPair : WeaponManager::getKeypressToWeaponMap())
            {
                if (m_keyboard.isKeyPressedOnce(keyWpnPair.first))
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
    const short int nMouseWheelChange = m_mouse.getWheel();

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
    if (m_mouse.isButtonPressed(PGEInputMouse::MouseButton::MBTN_LEFT))
    {
        bPrevLeftButtonPressed = true;

        // sending m_mouse action is still allowed when player is dead, since server will treat that
        // as respawn request

        const auto nSecsSinceLastWeaponSwitch =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - player.getWeaponManager().getTimeLastWeaponSwitch()
            ).count();
        if (nSecsSinceLastWeaponSwitch < m_nWeaponActionMinimumWaitMillisecondsAfterSwitch)
        {
            //getConsole().OLn("InputHandling::%s(): ignoring too early m_mouse action!", __func__);
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

    const int oldmx = m_mouse.getCursorPosX();
    const int oldmy = m_mouse.getCursorPosY();

    m_mouse.SetCursorPos(
        m_gfx.getWindow().getX() + m_gfx.getWindow().getWidth() / 2,
        m_gfx.getWindow().getY() + m_gfx.getWindow().getHeight() / 2);

    const int dx = oldmx - m_mouse.getCursorPosX();
    const int dy = oldmy - m_mouse.getCursorPosY();

    if ((dx == 0) && (dy == 0))
    {
        return false;
    }

    static bool bInitialXHairPosForTestingApplied = false;
    if (!bInitialXHairPosForTestingApplied && m_cfgProfiles.getVars()["testing"].getAsBool())
    {
        getConsole().OLn("InputHandling::%s(): Testing: Initial Mouse Cursor pos applied!", __func__);
        bInitialXHairPosForTestingApplied = true;
        if (m_network.isServer())
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
    std::ofstream fRegTestDump(m_network.isServer() ? proofps_dd::GAME_REG_TEST_DUMP_FILE_SERVER : proofps_dd::GAME_REG_TEST_DUMP_FILE_CLIENT);
    if (fRegTestDump.fail())
    {
        getConsole().EOLn("%s ERROR: couldn't create file: %s", __func__, m_network.isServer() ? proofps_dd::GAME_REG_TEST_DUMP_FILE_SERVER : proofps_dd::GAME_REG_TEST_DUMP_FILE_CLIENT);
        return;
    }

    fRegTestDump << "Tx: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << m_network.getServerClientInstance()->getTxPacketCount() << std::endl;
    fRegTestDump << "  " << m_network.getServerClientInstance()->getTxPacketPerSecondCount() << std::endl;
    fRegTestDump << "Rx: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << m_network.getServerClientInstance()->getRxPacketCount() << std::endl;
    fRegTestDump << "  " << m_network.getServerClientInstance()->getRxPacketPerSecondCount() << std::endl;
    fRegTestDump << "Inject: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << m_network.getServerClientInstance()->getInjectPacketCount() << std::endl;
    fRegTestDump << "  " << m_network.getServerClientInstance()->getInjectPacketPerSecondCount() << std::endl;

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
