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
    proofps_dd::Durations& durations,
    proofps_dd::Maps& maps) :
    /* due to virtual inheritance, we don't invoke ctor of PGE, PRooFPSddPGE invokes it only */
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
    if (getInput().getKeyboard().isKeyPressedOnce(VK_ESCAPE))
    {
        getPure().getWindow().Close();
    }

    if (gameMode.checkWinningConditions())
    {
        return;
    }

    if (getInput().getKeyboard().isKeyPressed(VK_TAB))
    {
        gameMode.showObjectives(getPure(), getNetwork());
    }

    if (getInput().getKeyboard().isKeyPressedOnce(VK_BACK))
    {
        m_bShowGuiDemo = !m_bShowGuiDemo;
        getPure().ShowGuiDemo(m_bShowGuiDemo);
        getPure().getWindow().SetCursorVisible(m_bShowGuiDemo);
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

        if (getInput().getKeyboard().isKeyPressedOnce(VK_RETURN))
        {
            if (getConfigProfiles().getVars()["testing"].getAsBool())
            {
                RegTestDumpToFile(gameMode, player);
            }
        }

        if (getInput().getKeyboard().isKeyPressedOnce((unsigned char)VkKeyScan('t')))
        {
            if (getNetwork().isServer())
            {
                // for testing purpose only, we can teleport server player to random spawn point
                player.getPos() = m_maps.getRandomSpawnpoint();
                player.getRespawnFlag() = true;
            }

            // log some stats
            getConsole().SetLoggingState("PureRendererHWfixedPipe", true);
            getPure().getRenderer()->ResetStatistics();
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
        if (getInput().getKeyboard().isKeyPressed(VK_LEFT) || getInput().getKeyboard().isKeyPressed((unsigned char)VkKeyScan('a')))
        {
            strafe = proofps_dd::Strafe::LEFT;
        }
        if (getInput().getKeyboard().isKeyPressed(VK_RIGHT) || getInput().getKeyboard().isKeyPressed((unsigned char)VkKeyScan('d')))
        {
            strafe = proofps_dd::Strafe::RIGHT;
        }

        bool bSendJumpAction = false;
        if (getInput().getKeyboard().isKeyPressedOnce(VK_SPACE))
        {
            bSendJumpAction = true;
        }

        bool bToggleRunWalk = false;
        if (getInput().getKeyboard().isKeyPressedOnce(VK_SHIFT))
        {
            bToggleRunWalk = true;
        }

        bool bRequestReload = false;
        if (getInput().getKeyboard().isKeyPressedOnce((unsigned char)VkKeyScan('r')))
        {
            bRequestReload = true;
        }

        unsigned char cWeaponSwitch = '\0';
        if (!bRequestReload)
        {   // we dont care about wpn switch if reload is requested
            for (const auto& keyWpnPair : WeaponManager::getKeypressToWeaponMap())
            {
                if (getInput().getKeyboard().isKeyPressedOnce(keyWpnPair.first))
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
    const short int nMouseWheelChange = getInput().getMouse().getWheel();

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
    if (getInput().getMouse().isButtonPressed(PGEInputMouse::MouseButton::MBTN_LEFT))
    {
        bPrevLeftButtonPressed = true;

        // sending getInput().getMouse() action is still allowed when player is dead, since server will treat that
        // as respawn request

        const auto nSecsSinceLastWeaponSwitch =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - player.getWeaponManager().getTimeLastWeaponSwitch()
            ).count();
        if (nSecsSinceLastWeaponSwitch < m_nWeaponActionMinimumWaitMillisecondsAfterSwitch)
        {
            //getConsole().OLn("InputHandling::%s(): ignoring too early getInput().getMouse() action!", __func__);
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

    const int oldmx = getInput().getMouse().getCursorPosX();
    const int oldmy = getInput().getMouse().getCursorPosY();

    getInput().getMouse().SetCursorPos(
        getPure().getWindow().getX() + getPure().getWindow().getWidth() / 2,
        getPure().getWindow().getY() + getPure().getWindow().getHeight() / 2);

    const int dx = oldmx - getInput().getMouse().getCursorPosX();
    const int dy = oldmy - getInput().getMouse().getCursorPosY();

    if ((dx == 0) && (dy == 0))
    {
        return false;
    }

    static bool bInitialXHairPosForTestingApplied = false;
    if (!bInitialXHairPosForTestingApplied && getConfigProfiles().getVars()["testing"].getAsBool())
    {
        getConsole().OLn("InputHandling::%s(): Testing: Initial Mouse Cursor pos applied!", __func__);
        bInitialXHairPosForTestingApplied = true;
        if (getNetwork().isServer())
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
    std::ofstream fRegTestDump(getNetwork().isServer() ? proofps_dd::GAME_REG_TEST_DUMP_FILE_SERVER : proofps_dd::GAME_REG_TEST_DUMP_FILE_CLIENT);
    if (fRegTestDump.fail())
    {
        getConsole().EOLn("%s ERROR: couldn't create file: %s", __func__, getNetwork().isServer() ? proofps_dd::GAME_REG_TEST_DUMP_FILE_SERVER : proofps_dd::GAME_REG_TEST_DUMP_FILE_CLIENT);
        return;
    }

    fRegTestDump << "Tx: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << getNetwork().getServerClientInstance()->getTxPacketCount() << std::endl;
    fRegTestDump << "  " << getNetwork().getServerClientInstance()->getTxPacketPerSecondCount() << std::endl;
    fRegTestDump << "Rx: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << getNetwork().getServerClientInstance()->getRxPacketCount() << std::endl;
    fRegTestDump << "  " << getNetwork().getServerClientInstance()->getRxPacketPerSecondCount() << std::endl;
    fRegTestDump << "Inject: Total Pkt Count, Pkt/Second" << std::endl;
    fRegTestDump << "  " << getNetwork().getServerClientInstance()->getInjectPacketCount() << std::endl;
    fRegTestDump << "  " << getNetwork().getServerClientInstance()->getInjectPacketPerSecondCount() << std::endl;

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
