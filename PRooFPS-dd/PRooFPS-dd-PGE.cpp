/*
    ###################################################################################
    PRooFPSddPGE.cpp
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022 - 2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "PRooFPS-dd-PGE.h"

#include <filesystem>  // requires cpp17
#include <functional>

#include "../../../PGE/PGE/Pure/include/external/Render/PureRendererHWfixedPipe.h"  // for rendering hints
#include "../../../PGE/PGE/Pure/include/external/PureUiManager.h"
#include "../../../PGE/PGE/Pure/include/external/Display/PureWindow.h"
#include "../../../PGE/PGE/Pure/include/external/PureCamera.h"
#include "../../../CConsole/CConsole/src/CConsole.h"

static const int   GAME_FPS_INTERVAL = 500;  // should be greater than 0
static const int   GAME_MAXFPS = 60;
static const float GAME_CAM_Z = -5.0f;
static const float GAME_CAM_SPEED = 1500.0f;
static const float GAME_PLAYER_SPEED1 = 2.0f;
static const float GAME_PLAYER_SPEED2 = 4.0f;
static const float GAME_FALLING_SPEED = 0.8f;
static const float GAME_JUMPING_SPEED = 2.0f;

static constexpr char* CVAR_CL_SERVER_IP = "cl_server_ip";
static constexpr char* CVAR_SV_MAP = "sv_map";


// ############################### PUBLIC ################################


PRooFPSddPGE* PRooFPSddPGE::createAndGetPRooFPSddPGEinstance()
{
    static PRooFPSddPGE pgeInstance((GAME_NAME + " " + GAME_VERSION).c_str());
    return &pgeInstance;
}

CConsole& PRooFPSddPGE::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}


const char* PRooFPSddPGE::getLoggerModuleName()
{
    return "PRooFPSddPGE";
}


// ############################## PROTECTED ##############################


/**
    This is the only usable ctor, this is used by the static createAndGet().
*/
PRooFPSddPGE::PRooFPSddPGE(const char* gameTitle) :
    PGE(gameTitle),
    m_maps(getPure()),
    m_fps(0),
    m_fps_counter(0),
    m_fps_lastmeasure(0),
    m_fps_ms(0),
    m_pObjXHair(NULL),
    m_bSpaceReleased(true),
    m_bBackSpaceReleased(true),
    m_bCtrlReleased(true),
    m_bShiftReleased(true),
    m_enterreleased(true),
    m_bTeleportReleased(true),
    m_bReloadReleased(true),
    m_bWon(false),
    m_fCameraMinY(0.0f),
    m_bShowGuiDemo(false),
    m_nServerSideConnectionHandle(0),
    m_nFramesElapsedSinceLastDurationsReset(0),
    m_nGravityCollisionDurationUSecs(0),
    m_nActiveWindowStuffDurationUSecs(0),
    m_nUpdateWeaponsDurationUSecs(0),
    m_nUpdateBulletsDurationUSecs(0),
    m_nUpdateRespawnTimersDurationUSecs(0),
    m_nPickupAndRespawnItemsDurationUSecs(0),
    m_nUpdateGameModeDurationUSecs(0),
    m_nSendUserUpdatesDurationUSecs(0),
    m_nFullOnGameRunningDurationUSecs(0),
    m_nHandleUserCmdMoveDurationUSecs(0),
    m_nFullOnPacketReceivedDurationUSecs(0),
    m_nFullRoundtripDurationUSecs(0)
{
    
}

PRooFPSddPGE::~PRooFPSddPGE()
{

}

/**
    Must-have minimal stuff before loading anything.
    Game engine calls this before even finishing its own initialization.
*/
bool PRooFPSddPGE::onGameInitializing()
{
    // Earliest we can enable our own logging
    getConsole().Initialize((GAME_NAME + " " + GAME_VERSION + " log").c_str(), true);
    getConsole().SetLoggingState(getLoggerModuleName(), true);
    getConsole().SetFGColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "999999" );
    getConsole().SetIntsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    getConsole().SetStringsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "FFFFFF" );
    getConsole().SetFloatsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    getConsole().SetBoolsColor( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FFFF" );

    // PRooFPSddPGE (game) logs
    getConsole().SetLoggingState(getLoggerModuleName(), true);

    // Network logs
    getConsole().SetLoggingState("PGESysNET", true);
    getConsole().SetLoggingState(getNetwork().getLoggerModuleName(), true);
    getConsole().SetLoggingState(getNetwork().getServer().getLoggerModuleName(), true);
    getConsole().SetLoggingState(getNetwork().getClient().getLoggerModuleName(), true);

    // Misc engine logs
    getConsole().SetLoggingState("PureWindow", true);

    // Turn everything on for development only
    getConsole().SetLoggingState("4LLM0DUL3S", true);

    // we need PGE::runGame() invoke EVERYTHING even when window is NOT active, and we will decide in onGameRunning() what NOT to do if window is inactive
    SetInactiveLikeActive(true);

    return true;
}

/** 
    Loading game content here.
*/
bool PRooFPSddPGE::onGameInitialized()
{
    getConsole().OLnOI("PRooFPSddPGE::onGameInitialized()");

    getConsole().SetLoggingState("4LLM0DUL3S", false);

    // basically I turn everything off, I could simply set 0, but still want to set bits in a clear way;
    // I need to use legacy rendering path, because if I use occlusion culling methods, it will be slow
    // for ~1000 cubes, since Pure still doesn't implement hierarchical occlusion culling ...
    // And a normal map like Warhouse already contains ~1000 cubes.
    getPure().getRenderer()->SetRenderHints(
        BITF_PREP(PURE_RH_RP_LEGACY_PR00FPS, PURE_RH_RENDER_PATH_BITS, 3) |
        BITF_PREP(PURE_RH_OQ_METHOD_ASYNC, PURE_RH_OQ_METHOD_BITS, 2) |
        PURE_RH_OQ_DRAW_BOUNDING_BOXES_OFF |
        PURE_RH_OQ_DRAW_IF_QUERY_PENDING_OFF |
        PURE_RH_ORDERING_BY_DISTANCE_OFF);
    
    getPure().getScreen().SetVSyncEnabled(true);
    setGameRunningFrequency(GAME_MAXFPS);

    getPure().getUImanager().SetDefaultFontSize(20);

    getPure().getCamera().SetNearPlane(0.1f);
    getPure().getCamera().SetFarPlane(100.0f);
    getPure().getCamera().getPosVec().Set( 0, 0, GAME_CAM_Z );
    getPure().getCamera().getTargetVec().Set( 0, 0, -GAME_BLOCK_SIZE_Z );

    m_gameMode = proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType::DeathMatch);
    if (!m_gameMode)
    {
        getConsole().EOLnOO("ERROR: createGameMode() failed!");
        return false;
    }

    m_deathMatchMode = dynamic_cast<proofps_dd::DeathMatchMode*>(m_gameMode);
    if (!m_deathMatchMode)
    {
        getConsole().EOLnOO("ERROR: m_deathMatchMode null!");
        return false;
    }

    if (!m_maps.initialize())
    {
        getConsole().EOLnOO("ERROR: m_maps.initialize() failed!");
        return false;
    }

    m_pObjXHair = getPure().getObject3DManager().createPlane(32.f, 32.f);
    m_pObjXHair->SetStickedToScreen(true);
    m_pObjXHair->SetDoubleSided(true);
    m_pObjXHair->SetTestingAgainstZBuffer(false);
    m_pObjXHair->SetLit(false);
    // for bitmaps not having proper alpha bits (e.g. saved by irfanview or mspaint), use (PURE_SRC_ALPHA, PURE_ONE)
    // otherwise (bitmaps saved by Flash) just use (PURE_SRC_ALPHA, PURE_ONE_MINUS_SRC_ALPHA) to utilize real alpha
    m_pObjXHair->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE);
    PureTexture* xhairtex = getPure().getTextureManager().createFromFile((std::string(GAME_TEXTURES_DIR)+"hud_xhair.bmp").c_str());
    m_pObjXHair->getMaterial().setTexture( xhairtex );

    getPure().WriteList();

    if (getNetwork().isServer())
    {
        // MsgUserSetup is also processed by server, but it injects this pkt into its own queue when needed.
        // MsgUserSetup MUST NOT be received by server over network!
        // MsgUserSetup is received only by clients over network!
        getNetwork().getServer().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserSetup::id));

        // MsgUserUpdate is also processed by server, but it injects this pkt into its own queue when needed.
        // MsgUserUpdate MUST NOT be received by server over network!
        // MsgUserUpdate is received only by clients over network!
        getNetwork().getServer().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserUpdate::id));

        getNetwork().getServer().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgBulletUpdate::id));
        getNetwork().getServer().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgMapItemUpdate::id));
        getNetwork().getServer().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgWpnUpdate::id));
        getNetwork().getServer().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgWpnUpdateCurrent::id));

        Text("Starting Server ...", 200, getPure().getWindow().getClientHeight() / 2);
        getPure().getRenderer()->RenderScene();

        if (!getNetwork().getServer().startListening())
        {
            PGE::showErrorDialog("Server has FAILED to start listening!");
            getConsole().EOLnOO("ERROR: Server has FAILED to start listening!");
            return false;
        }

        if (getConfigProfiles().getVars()[CVAR_SV_MAP].getAsString().empty())
        {
            m_sServerMapFilenameToLoad = "map_warena.txt";
            getConsole().OLn("Map default: %s", m_sServerMapFilenameToLoad.c_str());
        }
        else
        {
            m_sServerMapFilenameToLoad = getConfigProfiles().getVars()[CVAR_SV_MAP].getAsString();
            getConsole().OLn("Map from config: %s", m_sServerMapFilenameToLoad.c_str());
        }
        // TODO: log level override support: getConsole().SetLoggingState(sTrimmedLine.c_str(), true);
    }
    else
    {
        getNetwork().getClient().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserCmdMove::id));

        std::string sIp = "127.0.0.1";
        if (!getConfigProfiles().getVars()[CVAR_CL_SERVER_IP].getAsString().empty())
        {
            sIp = getConfigProfiles().getVars()[CVAR_CL_SERVER_IP].getAsString();
            getConsole().OLn("IP from config: %s", sIp.c_str());
        }
        // TODO: log level override support: getConsole().SetLoggingState(sTrimmedLine.c_str(), true);

        Text("Connecting to " + sIp + " ...", 200, getPure().getWindow().getClientHeight() / 2);
        getPure().getRenderer()->RenderScene();

        if (!getNetwork().getClient().connectToServer(sIp))
        {
            getConsole().EOLnOO("ERROR: Client has FAILED to establish connection to the server!");
            PGE::showErrorDialog("Client has FAILED to establish connection to the server!");
            return false;
        }
    }

    LoadSound(m_sndLetsgo,         (std::string(GAME_AUDIO_DIR) + "radio/locknload.wav").c_str());
    LoadSound(m_sndReloadStart,    (std::string(GAME_AUDIO_DIR) + "radio/de_clipout.wav").c_str());
    LoadSound(m_sndReloadFinish,   (std::string(GAME_AUDIO_DIR) + "radio/de_clipin.wav").c_str());
    LoadSound(m_sndShootPistol,    (std::string(GAME_AUDIO_DIR) + "radio/deagle-1.wav").c_str());
    LoadSound(m_sndShootMchgun,    (std::string(GAME_AUDIO_DIR) + "radio/m4a1_unsil-1.wav").c_str());
    LoadSound(m_sndShootDryPistol, (std::string(GAME_AUDIO_DIR) + "radio/dryfire_pistol.wav").c_str());
    LoadSound(m_sndShootDryMchgun, (std::string(GAME_AUDIO_DIR) + "radio/dryfire_rifle.wav").c_str());
    LoadSound(m_sndChangeWeapon,   (std::string(GAME_AUDIO_DIR) + "radio/m4a1_deploy.wav").c_str());
    LoadSound(m_sndPlayerDie,      (std::string(GAME_AUDIO_DIR) + "radio/die1.wav").c_str());

    getConsole().OOOLn("PRooFPSddPGE::onGameInitialized() done!");

    getInput().getMouse().SetCursorPos(
        getPure().getWindow().getX() + getPure().getWindow().getWidth()/2,
        getPure().getWindow().getY() + getPure().getWindow().getHeight()/2);
    getPure().getWindow().SetCursorVisible(false);

    m_deathMatchMode->SetFragLimit(10);
    //m_deathMatchMode->SetTimeLimitSecs(500);
    m_gameMode->Reset();
    
    m_fps_lastmeasure = GetTickCount();
    m_fps = 0;

    return true;
}


// ############################### PRIVATE ###############################


const unsigned int PRooFPSddPGE::m_nWeaponActionMinimumWaitMillisecondsAfterSwitch;

// Which key should switch to which weapon
std::map<unsigned char, PRooFPSddPGE::KeyReleasedAndWeaponFilenamePair> PRooFPSddPGE::m_mapKeypressToWeapon =
{
    {'2', {true, "pistol.txt"}},
    {'3', {true, "machinegun.txt"}}
};

void PRooFPSddPGE::Text(const std::string& s, int x, int y) const
{
    getPure().getUImanager().text(s, x, y)->SetDropShadow(true);
}

void PRooFPSddPGE::AddText(const std::string& s, int x, int y) const
{
    getPure().getUImanager().addText(s, x, y)->SetDropShadow(true);
}

void PRooFPSddPGE::KeyBoard(int /*fps*/, bool& won, pge_network::PgePacket& pkt, Player& player)
{
    const PGEInputKeyboard& keybd = getInput().getKeyboard();
  
    if ( keybd.isKeyPressed(VK_ESCAPE) )
    {
        getPure().getWindow().Close();
    }

    if (m_gameMode->checkWinningConditions())
    {
        return;
    }

    if (keybd.isKeyPressed(VK_TAB))
    {
        m_gameMode->ShowObjectives(getPure(), getNetwork());
    }

    if (keybd.isKeyPressed(VK_BACK))
    {
        if (m_bBackSpaceReleased)
        {
            m_bShowGuiDemo = !m_bShowGuiDemo;
            getPure().ShowGuiDemo(m_bShowGuiDemo);
            getPure().getWindow().SetCursorVisible(m_bShowGuiDemo);
            m_bBackSpaceReleased = false;
        }
    }
    else
    {
        m_bBackSpaceReleased = true;
    }

    if (m_bShowGuiDemo)
    {
        return;
    }

    if (player.getHealth() == 0)
    {
        return;
    }

    if ( !won )
    {

        if (keybd.isKeyPressed(VK_RETURN))
        {
            if (m_enterreleased)
            {
                m_enterreleased = false;
                if (getConfigProfiles().getVars()["testing"].getAsBool())
                {
                    RegTestDumpToFile();
                }
            }
        }
        else
        {
            m_enterreleased = true;
        }

        if (keybd.isKeyPressed((unsigned char)VkKeyScan('t')))
        {
            if (m_bTeleportReleased)
            {
                m_bTeleportReleased = false;
                
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
                getConsole().OLn("FramesElapsedSinceLastDurationsReset: %d", m_nFramesElapsedSinceLastDurationsReset);
                getConsole().OLn("Avg Durations per Frame:");
                getConsole().OLn(" - FullRoundtripDuration: %f usecs", m_nFullRoundtripDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn(" - FullOnPacketReceivedDuration: %f usecs", m_nFullOnPacketReceivedDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn("   - HandleUserCmdMoveDuration: %f usecs", m_nHandleUserCmdMoveDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn(" - FullOnGameRunningDuration: %f usecs", m_nFullOnGameRunningDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn("   - GravityCollisionDuration: %f usecs", m_nGravityCollisionDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn("   - ActiveWindowStuffDuration: %f usecs", m_nActiveWindowStuffDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn("   - UpdateWeaponDuration: %f usecs", m_nUpdateWeaponsDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn("   - UpdateBulletsDuration: %f usecs", m_nUpdateBulletsDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn("   - UpdateRespawnTimersDuration: %f usecs", m_nUpdateRespawnTimersDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn("   - PickupAndRespawnItemsDuration: %f usecs", m_nPickupAndRespawnItemsDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn("   - UpdateGameModeDuration: %f usecs", m_nUpdateGameModeDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn("   - SendUserUpdatesDuration: %f usecs", m_nSendUserUpdatesDurationUSecs / static_cast<float>(m_nFramesElapsedSinceLastDurationsReset));
                getConsole().OLn("");

                m_nFramesElapsedSinceLastDurationsReset = 0;
                m_nFullRoundtripDurationUSecs = 0;
                m_nFullOnGameRunningDurationUSecs = 0;
                m_nGravityCollisionDurationUSecs = 0;
                m_nActiveWindowStuffDurationUSecs = 0;
                m_nUpdateWeaponsDurationUSecs = 0;
                m_nUpdateBulletsDurationUSecs = 0;
                m_nUpdateRespawnTimersDurationUSecs = 0;
                m_nPickupAndRespawnItemsDurationUSecs = 0;
                m_nUpdateGameModeDurationUSecs = 0;
                m_nSendUserUpdatesDurationUSecs = 0;
                m_nFullOnPacketReceivedDurationUSecs = 0;
                m_nHandleUserCmdMoveDurationUSecs = 0;
            }
        }
        else
        {
            m_bTeleportReleased = true;
        }

        proofps_dd::Strafe strafe = proofps_dd::Strafe::NONE;
        if (keybd.isKeyPressed(VK_LEFT) || keybd.isKeyPressed((unsigned char)VkKeyScan('a')))
        {
            strafe = proofps_dd::Strafe::LEFT;
        }
        if (keybd.isKeyPressed(VK_RIGHT) || keybd.isKeyPressed((unsigned char)VkKeyScan('d')))
        {
            strafe = proofps_dd::Strafe::RIGHT;
        }
    
        bool bSendJumpAction = false;
        if ( keybd.isKeyPressed( VK_SPACE ) )
        {
            if (m_bSpaceReleased)
            {
                bSendJumpAction = true;
                m_bSpaceReleased = false;
            }
        }
        else
        {
            m_bSpaceReleased = true;
        }
    
        bool bToggleRunWalk = false;
        if ( keybd.isKeyPressed( VK_SHIFT ) )
        {
            if ( m_bShiftReleased )
            {
                bToggleRunWalk = true;
                m_bShiftReleased = false;
            }
        }
        else
        {
            m_bShiftReleased = true;
        }

        bool bRequestReload = false;
        if (keybd.isKeyPressed((unsigned char)VkKeyScan('r')))
        {
            if (m_bReloadReleased)
            {
                bRequestReload = true;
                m_bReloadReleased = false;
            }
        }
        else
        {
            m_bReloadReleased = true;
        }

        unsigned char cWeaponSwitch = '\0';
        if (!bRequestReload)
        {   // we dont care about wpn switch if reload is requested
            for (auto& key : m_mapKeypressToWeapon)
            {
                if (keybd.isKeyPressed(key.first))
                {
                    if (key.second.m_bReleased)
                    {
                        key.second.m_bReleased = false;

                        const Weapon* const pTargetWpn = player.getWeaponByFilename(key.second.m_sWpnFilename);
                        if (!pTargetWpn)
                        {
                            getConsole().EOLn("PRooFPSddPGE::%s(): not found weapon by name: %s!",
                                __func__, key.second.m_sWpnFilename.c_str());
                            break;
                        }
                        if (!pTargetWpn->isAvailable())
                        {
                            //getConsole().OLn("PRooFPSddPGE::%s(): weapon %s not available!",
                            //    __func__, key.second.m_sWpnFilename.c_str());
                            break;
                        }
                        if (pTargetWpn != player.getWeapon())
                        {
                            cWeaponSwitch = key.first;
                        }
                    }
                    break;
                }
                else
                {
                    key.second.m_bReleased = true;
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

bool PRooFPSddPGE::Mouse(int /*fps*/, bool& /*won*/, pge_network::PgePacket& pkt, Player& player)
{
    PGEInputMouse& mouse = getInput().getMouse();

    // we should always read the wheel data as often as possible, because this way we can avoid
    // the amount of wheel rotation accumulating too much
    const short int nMouseWheelChange = mouse.getWheel();
    
    if (m_gameMode->checkWinningConditions())
    {
        return false;
    }

    if (m_bShowGuiDemo)
    {
        return false;
    }

    static bool bPrevLeftButtonPressed = false;
    bool bShootActionBeingSent = false;
    if (mouse.isButtonPressed(PGEInputMouse::MouseButton::MBTN_LEFT))
    {
        bPrevLeftButtonPressed = true;

        // sending mouse action is still allowed when player is dead, since server will treat that
        // as respawn request

        const auto nSecsSinceLastWeaponSwitch =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - player.getTimeLastWeaponSwitch()
                ).count();
        if (nSecsSinceLastWeaponSwitch < m_nWeaponActionMinimumWaitMillisecondsAfterSwitch)
        {
            //getConsole().OLn("PRooFPSddPGE::%s(): ignoring too early mouse action!", __func__);
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
        MouseWheel(nMouseWheelChange, pkt, player);
    }

    // TODO: I think xhair update should happen earlier somewhere above, and click/wheel handling should
    // happen after that, so returning from function is an easier thing then ...

    const int oldmx = mouse.getCursorPosX();
    const int oldmy = mouse.getCursorPosY();

    mouse.SetCursorPos(
        getPure().getWindow().getX() + getPure().getWindow().getWidth()/2,
        getPure().getWindow().getY() + getPure().getWindow().getHeight()/2);

    const int dx = oldmx - mouse.getCursorPosX();
    const int dy = oldmy - mouse.getCursorPosY();

    if ((dx == 0) && (dy == 0))
    {
        return false;
    }
    
    static bool bInitialXHairPosForTestingApplied = false;
    if (!bInitialXHairPosForTestingApplied && getConfigProfiles().getVars()["testing"].getAsBool())
    {
        getConsole().OLn("PRooFPSddPGE::%s(): Testing: Initial Mouse Cursor pos applied!", __func__);
        bInitialXHairPosForTestingApplied = true;
        if (getNetwork().isServer())
        {
            m_pObjXHair->getPosVec().Set(100.f, 0.f, m_pObjXHair->getPosVec().getZ());
        }
        else
        {
            m_pObjXHair->getPosVec().Set(-100.f, 0.f, m_pObjXHair->getPosVec().getZ());
        }
    }
    else
    {
        m_pObjXHair->getPosVec().Set(
            m_pObjXHair->getPosVec().getX() + dx,
            m_pObjXHair->getPosVec().getY() - dy,
            0.f);
    }

    return true;
}

void PRooFPSddPGE::MouseWheel(const short int& nMouseWheelChange, pge_network::PgePacket& pkt, Player& player)
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
        
    //getConsole().OLn("PRooFPSddPGE::%s(): mousewheel: %d!", __func__, nMouseWheelChange);

    // not nice but we have to search by value in the map now ...
    // TODO: btw in the future the weapon switch forward/backward functionality will be implemented in WeaponManager
    const Weapon* const pMyCurrentWeapon = player.getWeapon();
    if (!pMyCurrentWeapon)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): pMyCurrentWeapon null, CANNOT HAPPEN!", __func__);
        return;
    }
                
    // TODO: rewrite this search to use lambda, then cCurrentWeaponKeyChar can be const
    unsigned char cCurrentWeaponKeyChar = '\0';
    auto it = m_mapKeypressToWeapon.begin();
    while (it != m_mapKeypressToWeapon.end())
    {
        if (it->second.m_sWpnFilename == pMyCurrentWeapon->getFilename())
        {
            cCurrentWeaponKeyChar = it->first;
            break;
        }
        ++it;
    }
    if (cCurrentWeaponKeyChar == '\0')
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): could not set cCurrentWeaponKeyChar based on %s!",
            __func__, pMyCurrentWeapon->getFilename().c_str());
        return;
    }

    // Now we have to increment or decrement 'it' until:
    // - we find an available weapon;
    // - we reach back to the current weapon.
    // I dont know if it is a good approach to just check only the sign of the change, and based on that,
    // move 1 step forward or backward in weapons list ... or maybe I should move n steps based on the exact amount ...
    
    // This is very ugly as I have to iterate in a map either back or forth ... on the long run I will need
    // a double linked list or similar, so it is easier to go in the list until we end up at the starting element or find an available wpn.
    
    unsigned char cTargetWeapon = cCurrentWeaponKeyChar;
    if (nMouseWheelChange > 0)
    {
        // wheel rotated forward, in CS it means going forward in the list;
        it++;
        while (it != m_mapKeypressToWeapon.end())
        {
            const Weapon* const pTargetWeapon = player.getWeaponByFilename(it->second.m_sWpnFilename);
            if (pTargetWeapon && pTargetWeapon->isAvailable())
            {
                // we dont care about if bullets are loaded, if available then let it be the target!
                cTargetWeapon = it->first;
                break;
            }
            ++it;
        }
        if (it == m_mapKeypressToWeapon.end())
        {
            // try it from the beginning ...
            it = m_mapKeypressToWeapon.begin();
            while (it->first != cCurrentWeaponKeyChar)
            {
                const Weapon* const pTargetWeapon = player.getWeaponByFilename(it->second.m_sWpnFilename);
                if (pTargetWeapon && pTargetWeapon->isAvailable())
                {
                    // we dont care about if bullets are loaded, if available then let it be the target!
                    cTargetWeapon = it->first;
                    break;
                }
                ++it;
            }
        }
    }
    else
    {
        // wheel rotated backward, in CS it means going backward in the list;
        do
        {
            if (it != m_mapKeypressToWeapon.begin())
            {
                --it;
            }
            else
            {
                break;
            }
            const Weapon* const pTargetWeapon = player.getWeaponByFilename(it->second.m_sWpnFilename);
            if (pTargetWeapon && pTargetWeapon->isAvailable())
            {
                // we dont care about if bullets are loaded, if available then let it be the target!
                cTargetWeapon = it->first;
                break;
            }
        } while (true);
        if (cTargetWeapon == cCurrentWeaponKeyChar)
        {
            // try it from the end ...
            it = m_mapKeypressToWeapon.end();
            --it;
            while (it->first != cCurrentWeaponKeyChar)
            {
                const Weapon* const pTargetWeapon = player.getWeaponByFilename(it->second.m_sWpnFilename);
                if (pTargetWeapon && pTargetWeapon->isAvailable())
                {
                    // we dont care about if bullets are loaded, if available then let it be the target!
                    cTargetWeapon = it->first;
                    break;
                }
                --it;
            }
        }
    }

    if (cTargetWeapon == cCurrentWeaponKeyChar)
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): no next available weapon found!", __func__);
    }
    else
    {
        proofps_dd::MsgUserCmdMove::SetWeaponSwitch(pkt, cTargetWeapon);
        //const std::string scTargetWeapon = std::to_string(cTargetWeapon);
        //getConsole().OLn("PRooFPSddPGE::%s(): next weapon is: %s!", __func__, scTargetWeapon.c_str());
    }            
}


bool PRooFPSddPGE::Colliding(const PureObject3D& a, const PureObject3D& b)
{
    return Colliding2(
        a.getPosVec().getX(),  a.getPosVec().getY(),  a.getPosVec().getZ(),
        a.getSizeVec().getX(), a.getSizeVec().getY(), a.getSizeVec().getZ(),
        b.getPosVec().getX(),  b.getPosVec().getY(),  b.getPosVec().getZ(),
        b.getSizeVec().getX(), b.getSizeVec().getY(), b.getSizeVec().getZ()
    );
}       

bool PRooFPSddPGE::Colliding2(
    float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
    float o2px, float o2py, float o2pz, float o2sx, float o2sy, float o2sz )
{
    return (
        (
            (o1px - o1sx / 2 <= o2px + o2sx / 2)
            &&
            (o1px + o1sx / 2 >= o2px - o2sx / 2)
        )
        &&
        (
            (o1py - o1sy / 2 <= o2py + o2sy / 2)
            &&
            (o1py + o1sy / 2 >= o2py - o2sy / 2)
        )
        &&
        (
            (o1pz - o1sz / 2 <= o2pz + o2sz / 2)
            &&
            (o1pz + o1sz / 2 >= o2pz - o2sz / 2)
        )
    );            
}

bool PRooFPSddPGE::Colliding2_NoZ(
    float o1px, float o1py, float o1sx, float o1sy,
    float o2px, float o2py, float o2sx, float o2sy)
{
    return (
        (
            (o1px - o1sx / 2 <= o2px + o2sx / 2)
            &&
            (o1px + o1sx / 2 >= o2px - o2sx / 2)
            )
        &&
        (
            (o1py - o1sy / 2 <= o2py + o2sy / 2)
            &&
            (o1py + o1sy / 2 >= o2py - o2sy / 2)
            )
        );
}

bool PRooFPSddPGE::Colliding3(
    const PureVector& vecPosMin, const PureVector& vecPosMax,
    const PureVector& vecObjPos, const PureVector& vecObjSize)
{
    const PureVector vecSize(
        vecPosMax.getX() - vecPosMin.getX(),
        vecPosMax.getY() - vecPosMin.getY(),
        vecPosMax.getZ() - vecPosMin.getZ()
    );
    const PureVector vecPos(
        vecPosMin.getX() + vecSize.getX() / 2.f,
        vecPosMin.getY() + vecSize.getY() / 2.f,
        vecPosMin.getZ() + vecSize.getZ() / 2.f
    );

    return Colliding2(
        vecPos.getX(),  vecPos.getY(),  vecPos.getZ(),
        vecSize.getX(), vecSize.getY(), vecSize.getZ(),
        vecObjPos.getX(),  vecObjPos.getY(),  vecObjPos.getZ(),
        vecObjSize.getX(), vecObjSize.getY(), vecObjSize.getZ()
    );
}

void PRooFPSddPGE::PlayerCollisionWithWalls(bool& /*won*/)
{ 
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        const PureObject3D* const plobj = player.getObject3D();

        // how to make collision detection even faster:
        // if we dont want to use spatial hierarchy like BVH, just store the map elements in a matrix that we can address with i and j,
        // and based on player's position it is very easy to know which few map elements around matrix[i][j] should be checked ...
        // And I'm also thinking that not pointers but the objects themselves could be stored in matrix, that way the whole matrix
        // could be fetched into cache for even faster iteration on its elements ...

        // at this point, player.getPos().getY() is already updated by Gravity()
        const float fBlockSizeXhalf = GAME_BLOCK_SIZE_X / 2.f;
        const float fBlockSizeYhalf = GAME_BLOCK_SIZE_Y / 2.f;

        const float fPlayerOPos1XMinusHalf = player.getOPos().getX() - plobj->getSizeVec().getX() / 2.f;
        const float fPlayerOPos1XPlusHalf = player.getOPos().getX() + plobj->getSizeVec().getX() / 2.f;
        const float fPlayerPos1YMinusHalf = player.getPos().getY() - plobj->getSizeVec().getY() / 2.f;
        const float fPlayerPos1YPlusHalf = player.getPos().getY() + plobj->getSizeVec().getY() / 2.f;
        if (player.getOPos().getY() != player.getPos().getY())
        {
            for (int i = 0; i < m_maps.getForegroundBlockCount(); i++)
            {
                const PureObject3D* const obj = m_maps.getForegroundBlocks()[i];
                assert(obj);  // we dont store nulls there

                if ((obj->getPosVec().getX() + fBlockSizeXhalf < fPlayerOPos1XMinusHalf) || (obj->getPosVec().getX() - fBlockSizeXhalf > fPlayerOPos1XPlusHalf))
                {
                    continue;
                }

                if ((obj->getPosVec().getY() + fBlockSizeYhalf < fPlayerPos1YMinusHalf) || (obj->getPosVec().getY() - fBlockSizeYhalf > fPlayerPos1YPlusHalf))
                {
                    continue;
                }

                const int nAlignUnderOrAboveWall = obj->getPosVec().getY() < player.getOPos().getY() ? 1 : -1;
                const float fAlignCloseToWall = nAlignUnderOrAboveWall * (fBlockSizeYhalf + GAME_PLAYER_H / 2.0f + 0.01f);
                player.getPos().SetY(obj->getPosVec().getY() + fAlignCloseToWall);

                if (nAlignUnderOrAboveWall == 1)
                {
                    // we fell from above
                    player.SetCanFall(false);
                    player.getForce().Set(0.f, 0.f, 0.f);
                }
                else
                {
                    // we hit ceiling with our head during jumping
                    player.SetCanFall(true);
                    player.StopJumping();
                    player.SetGravity(0.f);
                }

                break;
            }
        }

        player.getPos().SetX(player.getPos().getX() + player.getForce().getX());

        const float fPlayerPos1XMinusHalf = player.getPos().getX() - plobj->getSizeVec().getX() / 2.f;
        const float fPlayerPos1XPlusHalf = player.getPos().getX() + plobj->getSizeVec().getX() / 2.f;
        const float fPlayerPos1YMinusHalf_2 = player.getPos().getY() - plobj->getSizeVec().getY() / 2.f;
        const float fPlayerPos1YPlusHalf_2 = player.getPos().getY() + plobj->getSizeVec().getY() / 2.f;

        if (player.getOPos().getX() != player.getPos().getX())
        {
            for (int i = 0; i < m_maps.getForegroundBlockCount(); i++)
            {
                const PureObject3D* const obj = m_maps.getForegroundBlocks()[i];
                assert(obj);  // we dont store nulls there

                if ((obj->getPosVec().getX() + fBlockSizeXhalf < fPlayerPos1XMinusHalf) || (obj->getPosVec().getX() - fBlockSizeXhalf > fPlayerPos1XPlusHalf))
                {
                    continue;
                }

                if ((obj->getPosVec().getY() + fBlockSizeYhalf < fPlayerPos1YMinusHalf_2) || (obj->getPosVec().getY() - fBlockSizeYhalf > fPlayerPos1YPlusHalf_2))
                {
                    continue;
                }

                // in case of horizontal collision, we should not reposition to previous position, but align next to the wall
                const int nAlignLeftOrRightToWall = obj->getPosVec().getX() < player.getOPos().getX() ? 1 : -1;
                const float fAlignNextToWall = nAlignLeftOrRightToWall * (obj->getSizeVec().getX() / 2 + GAME_PLAYER_W / 2.0f + 0.01f);
                player.getPos().SetX(obj->getPosVec().getX() + fAlignNextToWall);

                break;
            }
        }
    }
}

void PRooFPSddPGE::CameraMovement(int /*fps*/, Player& player)
{
    PureVector campos = getPure().getCamera().getPosVec();
    float celx, cely;
    float speed = GAME_CAM_SPEED / 60.0f;

    /* ne mehessen túlságosan balra vagy jobbra a kamera */
    //if ( m_player.getPos().getX() < m_maps.getStartPos().getX() )
    //    celx = m_maps.getStartPos().getX();
    //else
    //    if ( m_player.getPos().getX() > m_maps.getEndPos().getX() )
    //        celx = m_maps.getEndPos().getX();
    //     else
            celx = player.getObject3D()->getPosVec().getX();

    /* ne mehessen túlságosan le és fel a kamera */
    //if ( m_player.getPos().getY() < m_fCameraMinY )
    //    cely = m_fCameraMinY;
    //else
    //    if ( m_player.getPos().getY() > GAME_CAM_MAX_Y )
    //        cely = GAME_CAM_MAX_Y;
    //    else
            cely = player.getObject3D()->getPosVec().getY();

    /* a játékoshoz igazítjuk a kamerát */
    if (celx != campos.getX() )
    {
        campos.SetX(campos.getX() + ((celx - campos.getX())/speed) );
    }
    if (cely != campos.getY() )
    {
        campos.SetY(campos.getY() + ((cely - campos.getY())/speed) );
    }

    getPure().getCamera().getPosVec().Set( campos.getX(), campos.getY(), GAME_CAM_Z );
    getPure().getCamera().getTargetVec().Set( campos.getX(), campos.getY(), player.getObject3D()->getPosVec().getZ() );

} // CameraMovement()

void PRooFPSddPGE::Gravity(int /*fps*/)
{
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        if (player.isJumping())
        {
            player.SetGravity(player.getGravity() - GAME_JUMPING_SPEED / 60.f/*(float)fps*/);
            if (player.getGravity() < 0.0f)
            {
                player.StopJumping();
            }
        }
        else
        {
            if (player.getGravity() > GAME_GRAVITY_MIN)
            {
                player.SetGravity(player.getGravity() - GAME_FALLING_SPEED / 60.f/*(float)fps*/);
                if (player.getGravity() < GAME_GRAVITY_MIN)
                {
                    player.SetGravity(GAME_GRAVITY_MIN);
                }
            }
        }
        player.getPos().SetY(player.getPos().getY() + player.getGravity());
        
        if ( (player.getHealth() > 0) && (player.getPos().getY() < m_maps.getBlockPosMin().getY() - 5.0f))
        {
            // need to die, out of map lower bound
            HandlePlayerDied(player);
        }
    }
}

void PRooFPSddPGE::UpdateBullets()
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    // on the long run this function needs to be part of the game engine itself, however currently game engine doesn't handle collisions,
    // so once we introduce the collisions to the game engine, it will be an easy move of this function as well there
    pge_network::PgePacket newPktBulletUpdate;
    const float fBlockSizeXhalf = GAME_BLOCK_SIZE_X / 2.f;
    const float fBlockSizeYhalf = GAME_BLOCK_SIZE_Y / 2.f;
    bool bEndGame = m_gameMode->checkWinningConditions();
    std::list<Bullet>& bullets = getWeaponManager().getBullets();
    auto it = bullets.begin();
    while (it != bullets.end())
    {
        auto& bullet = *it;

        bool bDeleteBullet = false;
        if (bEndGame)
        {
            bDeleteBullet = true;
        }
        else
        {
            bullet.Update();
        }

        const float fBulletPosX = bullet.getObject3D().getPosVec().getX();
        const float fBulletPosY = bullet.getObject3D().getPosVec().getY();

        if (!bDeleteBullet)
        {
            // check if bullet is hitting a player
            for (auto& playerPair : m_mapPlayers)
            {
                if (bullet.getOwner() == playerPair.second.getServerSideConnectionHandle())
                {
                    // bullet cannot hit the owner, at least for now ...
                    // in the future, when bullets start in proper position, we won't need this check ...
                    // this check will be bad anyway in future when we will have the guided rockets that actually can hit the owner if guided in suicide way!
                    continue;
                }

                if ((playerPair.second.getHealth() > 0) &&
                    Colliding(*(playerPair.second.getObject3D()), bullet.getObject3D()))
                {
                    bDeleteBullet = true;
                    playerPair.second.DoDamage(bullet.getDamageHp());
                    if (playerPair.second.getHealth() == 0)
                    {
                        const auto itKiller = m_mapPlayers.find(bullet.getOwner());
                        if (itKiller == m_mapPlayers.end())
                        {
                            //getConsole().OLn("PRooFPSddPGE::%s(): Player %s has been killed by a playerPair already left!",
                            //    __func__, playerPair.first.c_str());
                        }
                        else
                        {
                            itKiller->second.getFrags()++;
                            bEndGame = m_gameMode->checkWinningConditions();
                            //getConsole().OLn("PRooFPSddPGE::%s(): Player %s has been killed by %s, who now has %d frags!",
                            //    __func__, playerPair.first.c_str(), itKiller->first.c_str(), itKiller->second.getFrags());
                        }
                        // server handles death here, clients will handle it when they receive MsgUserUpdate
                        HandlePlayerDied(playerPair.second);
                    }
                    break; // we can stop since 1 bullet can touch 1 playerPair only at a time
                }
            }

            if (!bDeleteBullet)
            {
                // check if bullet is out of map bounds
                // we relax map bounds a bit to let the bullets leave map area a bit more before destroying them ...
                const PureVector vRelaxedMapMinBounds(
                    m_maps.getBlocksVertexPosMin().getX() - GAME_BLOCK_SIZE_X * 4,
                    m_maps.getBlocksVertexPosMin().getY() - GAME_BLOCK_SIZE_Y,
                    m_maps.getBlocksVertexPosMin().getZ() - GAME_BLOCK_SIZE_Z); // ah why dont we have vector-scalar subtract operator defined ...
                const PureVector vRelaxedMapMaxBounds(
                    m_maps.getBlocksVertexPosMax().getX() + GAME_BLOCK_SIZE_X * 4,
                    m_maps.getBlocksVertexPosMax().getY() + GAME_BLOCK_SIZE_Y,
                    m_maps.getBlocksVertexPosMax().getZ() + GAME_BLOCK_SIZE_Z);
                if (!Colliding3(vRelaxedMapMinBounds, vRelaxedMapMaxBounds, bullet.getObject3D().getPosVec(), bullet.getObject3D().getSizeVec()))
                {
                    bDeleteBullet = true;
                }
            }

            if (!bDeleteBullet)
            {
                // check if bullet is hitting a map element
                // 
                // TODO: this part slows the game down in debug mode with a few bullets.
                // Originally with only 6 bullets, with VSync FPS went down from 60 to 45 on main dev machine, so
                // I recommend using spatial hierarchy to effectively reduce the number of collision checks ...
                // For now with the small bullet direction optimization in the loop I managed to keep FPS around 45-50
                // with 10-15 bullets.
                for (int i = 0; i < m_maps.getForegroundBlockCount(); i++)
                {
                    const PureObject3D* const obj = m_maps.getForegroundBlocks()[i];
                    const bool bGoingLeft = bullet.getObject3D().getAngleVec().getY() == 0.f; // otherwise it would be 180.f
                    const float fMapObjPosX = obj->getPosVec().getX();
                    const float fMapObjPosY = obj->getPosVec().getY();

                    if ((bGoingLeft && (fMapObjPosX - fBlockSizeXhalf > fBulletPosX)) ||
                        (!bGoingLeft && (fMapObjPosX + fBlockSizeXhalf < fBulletPosX)))
                    {
                        // optimization: rule out those blocks which are not in bullet's direction
                        continue;
                    }

                    const float fBulletPosXMinusHalf = fBulletPosX - bullet.getObject3D().getSizeVec().getX() / 2.f;
                    const float fBulletPosXPlusHalf = fBulletPosX - bullet.getObject3D().getSizeVec().getX() / 2.f;
                    const float fBulletPosYMinusHalf = fBulletPosY - bullet.getObject3D().getSizeVec().getY() / 2.f;
                    const float fBulletPosYPlusHalf = fBulletPosY - bullet.getObject3D().getSizeVec().getY() / 2.f;

                    if ((fMapObjPosX + fBlockSizeXhalf < fBulletPosXMinusHalf) || (fMapObjPosX - fBlockSizeXhalf > fBulletPosXPlusHalf))
                    {
                        continue;
                    }

                    if ((fMapObjPosY + fBlockSizeYhalf < fBulletPosYMinusHalf) || (fMapObjPosY - fBlockSizeYhalf > fBulletPosYPlusHalf))
                    {
                        continue;
                    }

                    bDeleteBullet = true;
                    break; // we can stop since 1 bullet can touch only 1 map element
                }
            }
        }

        if ( bDeleteBullet )
        {
            proofps_dd::MsgBulletUpdate::initPktForDeleting_WithGarbageValues(
                newPktBulletUpdate,
                0,
                bullet.getId()); // clients will also delete this bullet on their side because we set pkt's delete flag here
            it = bullets.erase(it); // delete it right now, otherwise later we would send further updates to clients about this bullet
        }
        else
        {
            proofps_dd::MsgBulletUpdate::initPkt(
                newPktBulletUpdate,
                bullet.getOwner(),
                bullet.getId(),
                fBulletPosX,
                fBulletPosY,
                bullet.getObject3D().getPosVec().getZ(),
                bullet.getObject3D().getAngleVec().getX(),
                bullet.getObject3D().getAngleVec().getY(),
                bullet.getObject3D().getAngleVec().getZ(),
                bullet.getObject3D().getSizeVec().getX(),
                bullet.getObject3D().getSizeVec().getY(),
                bullet.getObject3D().getSizeVec().getZ());

            // bullet didn't touch anything, go to next
            it++;
        }
        
        // 'it' is referring to next bullet, don't use it from here!
        for (const auto& sendToThisPlayer : m_mapPlayers)
        {
            if (sendToThisPlayer.second.getServerSideConnectionHandle() != 0)
            {   // since bullet.Update() updates the bullet position already, server doesn't send this to itself
                getNetwork().getServer().SendPacketToClient(sendToThisPlayer.second.getServerSideConnectionHandle(), newPktBulletUpdate);
            }
        }
    }

    if (bEndGame && (Bullet::getGlobalBulletId() > 0))
    {
        Bullet::ResetGlobalBulletId();
    }

    m_nUpdateBulletsDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void PRooFPSddPGE::UpdateWeapons()
{
    if (m_gameMode->checkWinningConditions())
    {
        return;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    for (auto& playerPair : m_mapPlayers)
    {
        Weapon* const wpn = playerPair.second.getWeapon();
        if (!wpn)
        {
            continue;
        }
        if (wpn->update())
        {
            if (playerPair.first != m_nServerSideConnectionHandle) // server doesn't need to send this msg to itself, it already executed bullet count change by reload()
            {
                pge_network::PgePacket pktWpnUpdate;
                proofps_dd::MsgWpnUpdate::initPkt(
                    pktWpnUpdate,
                    0 /* ignored by client anyway */,
                    wpn->getFilename(),
                    wpn->isAvailable(),
                    wpn->getMagBulletCount(),
                    wpn->getUnmagBulletCount());
                getNetwork().getServer().SendPacketToClient(playerPair.second.getServerSideConnectionHandle(), pktWpnUpdate);
            }
        }
    }

    m_nUpdateWeaponsDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void PRooFPSddPGE::HandlePlayerDied(Player& player)
{
    player.Die(isMyConnection(player.getServerSideConnectionHandle()), getNetwork().isServer());
    if (isMyConnection(player.getServerSideConnectionHandle()))
    {
        getAudio().play(m_sndPlayerDie);
        m_pObjXHair->Hide();
        AddText("Waiting to respawn ...", 200, getPure().getWindow().getClientHeight() / 2);
    }
}

void PRooFPSddPGE::HandlePlayerRespawned(Player& player)
{
    const Weapon* const wpnDefaultAvailable = getWeaponManager().getWeaponByFilename(getWeaponManager().getDefaultAvailableWeaponFilename());
    assert(wpnDefaultAvailable);  // cannot be null since it is already verified in handleUserSetup()
    player.Respawn(isMyConnection(player.getServerSideConnectionHandle()), *wpnDefaultAvailable, getNetwork().isServer());

    if (isMyConnection(player.getServerSideConnectionHandle()))
    {
        m_pObjXHair->Show();
        // well, this won't work if clientHeight is being changed in the meantime, but anyway this supposed to be a temporal feature ...
        getPure().getUImanager().RemoveText(
            "Waiting to respawn ...", 200, getPure().getWindow().getClientHeight() / 2, getPure().getUImanager().getDefaultFontSize());
    }
}

bool PRooFPSddPGE::hasValidConnection() const
{
    return m_mapPlayers.find(m_nServerSideConnectionHandle) != m_mapPlayers.end();
}

bool PRooFPSddPGE::isMyConnection(const pge_network::PgeNetworkConnectionHandle& connHandleServerSide) const
{
    return m_nServerSideConnectionHandle == connHandleServerSide;
}

void PRooFPSddPGE::LoadSound(SoLoud::Wav& snd, const char* fname)
{
    const SoLoud::result resSoloud = snd.load(fname);
    if (resSoloud == SoLoud::SOLOUD_ERRORS::SO_NO_ERROR)
    {
        getConsole().OLn("%s: %s loaded, length: %f secs!", __func__, fname, snd.getLength());
    }
    else
    {
        getConsole().EOLn("%s: %s load error: %d!", __func__, fname, resSoloud);
    }
}

void PRooFPSddPGE::UpdateRespawnTimers()
{
    if (m_gameMode->checkWinningConditions())
    {
        return;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    for (auto& playerPair : m_mapPlayers)
    {
        if (playerPair.second.getHealth() > 0)
        {
            continue;
        }

        const long long timeDiffSeconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - playerPair.second.getTimeDied()).count();
        if (timeDiffSeconds >= GAME_PLAYER_RESPAWN_SECONDS)
        {
            // to respawn, we just need to set these values, because SendUserUpdates() will automatically send out changes to everyone
            playerPair.second.getPos() = m_maps.getRandomSpawnpoint();
            playerPair.second.SetHealth(100);
            playerPair.second.getRespawnFlag() = true;
        }
    }

    m_nUpdateRespawnTimersDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void PRooFPSddPGE::PickupAndRespawnItems()
{
    if (m_gameMode->checkWinningConditions())
    {
        return;
    }

    std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    pge_network::PgePacket newPktMapItemUpdate;
    pge_network::PgePacket newPktWpnUpdate;

    for (auto& itemPair : m_maps.getItems())
    {
        if (!itemPair.second)
        {
            continue;
        }

        MapItem& mapItem = *(itemPair.second);
        bool bSendItemUpdate = false;

        if (mapItem.isTaken())
        {
            const auto nSecsSinceTake = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - mapItem.getTimeTaken()).count();
            if (nSecsSinceTake < MapItem::getItemRespawnTimeSecs(mapItem))
            {
                continue;
            }

            mapItem.UnTake();
            bSendItemUpdate = true;
        }
        else
        {
            for (auto& playerPair : m_mapPlayers)
            {
                auto& player = playerPair.second;
                if (player.getHealth() <= 0)
                {
                    continue;
                }

                const PureObject3D* const plobj = player.getObject3D();

                // TODO: from performance perspective, maybe it would be better to check canTakeItem() first since that might be faster
                // decision than collision check ...
                if (Colliding(*plobj, mapItem.getObject3D()))
                {
                    proofps_dd::MsgWpnUpdate::getAvailable(newPktWpnUpdate) = false;
                    if (player.canTakeItem(mapItem))
                    {
                        player.TakeItem(mapItem, newPktWpnUpdate);  // this also invokes mapItem.Take()
                        bSendItemUpdate = true;
                        // although item update is always sent, wpn update is sent only if TakeItem() flipped the availability of the wpn,
                        // since it can happen the item is not weapon-related at all, or something else, anyway let TakeItem() make the decision!
                        if (proofps_dd::MsgWpnUpdate::getAvailable(newPktWpnUpdate))
                        {
                            getNetwork().getServer().SendPacketToClient(playerPair.second.getServerSideConnectionHandle(), newPktWpnUpdate);
                        }
                        break; // a player can collide with only one item at a time since there are no overlapping items
                    }
                } // colliding
            } // for playerPair
        }

        if (bSendItemUpdate)
        {
            proofps_dd::MsgMapItemUpdate::initPkt(
                newPktMapItemUpdate,
                0,
                mapItem.getId(),
                mapItem.isTaken());

            for (const auto& sendToThisPlayer : m_mapPlayers)
            {
                if (sendToThisPlayer.second.getServerSideConnectionHandle() != 0)
                { 
                    getNetwork().getServer().SendPacketToClient(sendToThisPlayer.second.getServerSideConnectionHandle(), newPktMapItemUpdate);
                }
            }
        }
    } // for item

    m_nPickupAndRespawnItemsDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void PRooFPSddPGE::UpdateGameMode()
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    if (m_gameMode->checkWinningConditions())
    {
        const auto nSecsSinceWin = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_gameMode->getWinTime()).count();
        if (nSecsSinceWin >= 15)
        {
            if (getNetwork().isServer())
            {
                for (auto& playerPair : m_mapPlayers)
                {
                    // to respawn, we just need to set these values, because SendUserUpdates() will automatically send out changes to everyone
                    playerPair.second.getPos() = m_maps.getRandomSpawnpoint();
                    playerPair.second.SetHealth(100);
                    playerPair.second.getFrags() = 0;
                    playerPair.second.getDeaths() = 0;
                    playerPair.second.getRespawnFlag() = true;
                }

                // respawn all map items
                pge_network::PgePacket newPktMapItemUpdate;
                for (auto& itemPair : m_maps.getItems())
                {
                    if (!itemPair.second)
                    {
                        continue;
                    }

                    MapItem& mapItem = *(itemPair.second);
                    if (!mapItem.isTaken())
                    {
                        continue;
                    }

                    mapItem.UnTake();

                    proofps_dd::MsgMapItemUpdate::initPkt(
                        newPktMapItemUpdate,
                        0,
                        mapItem.getId(),
                        mapItem.isTaken());

                    for (const auto& sendToThisPlayer : m_mapPlayers)
                    {
                        if (sendToThisPlayer.second.getServerSideConnectionHandle() != 0)
                        {
                            getNetwork().getServer().SendPacketToClient(sendToThisPlayer.second.getServerSideConnectionHandle(), newPktMapItemUpdate);
                        }
                    }
                } // end for items
            } // end server
            m_gameMode->Reset(); // now both server and clients execute this on their own, in future only server should do this ...
        }
        else
        {
            // these are being executed frame by frame during waiting for game restart, however these are cheap operations so I dont care ...
            m_gameMode->ShowObjectives(getPure(), getNetwork());
            m_pObjXHair->Hide();
            for (auto& playerPair : m_mapPlayers)
            {
                playerPair.second.getObject3D()->Hide();
                playerPair.second.getWeapon()->getObject3D().Hide();
            }
        }
    }

    m_nUpdateGameModeDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void PRooFPSddPGE::SendUserUpdates()
{
    if (!getNetwork().isServer())
    {
        // should not happen, but we log it anyway, if in future we might mess up something during a refactor ...
        getConsole().EOLn("PRooFPSddPGE::%s(): NOT server!", __func__);
        return;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        if ((player.getPos() != player.getOPos()) || (player.getOldAngleY() != player.getAngleY())
            || (player.getWeaponAngle() != player.getOldWeaponAngle())
            || (player.getHealth() != player.getOldHealth())
            || (player.getFrags() != player.getOldFrags())
            || (player.getDeaths() != player.getOldDeaths()))
        {
            pge_network::PgePacket newPktUserUpdate;
            proofps_dd::MsgUserUpdate::initPkt(
                newPktUserUpdate,
                playerPair.second.getServerSideConnectionHandle(),
                player.getPos().getX(),
                player.getPos().getY(),
                player.getPos().getZ(),
                player.getAngleY(),
                player.getWeaponAngle().getY(),
                player.getWeaponAngle().getZ(),
                player.getHealth(),
                player.getRespawnFlag(),
                player.getFrags(),
                player.getDeaths());

            // we always reset respawn flag here
            playerPair.second.getRespawnFlag() = false;

            // Note that health is not needed by server since it already has the updated health, but for convenience
            // we put that into MsgUserUpdate and send anyway like all the other stuff.

            for (const auto& sendToThisPlayer : m_mapPlayers)
            {
                if (sendToThisPlayer.second.getServerSideConnectionHandle() == 0)
                {
                    // server injects this packet to own queue
                    getNetwork().getServer().InjectPacket(newPktUserUpdate);
                }
                else
                {
                    getNetwork().getServer().SendPacketToClient(sendToThisPlayer.second.getServerSideConnectionHandle(), newPktUserUpdate);
                }
            }
        }
    }

    m_nSendUserUpdatesDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

/**
    Game logic right before the engine would do anything.
    This is invoked at the very beginning of the main game loop, before processing window messages and incoming network packets.
*/
void PRooFPSddPGE::onGameFrameBegin()
{
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        if (getNetwork().isServer())
        {
            if (player.getPos().getY() != player.getOPos().getY())
            {   // still could fall in previous frame, so jumping is still disallowed ...
                player.SetJumpAllowed(false);
            }
            else
            {
                player.SetJumpAllowed(true);
            }
        }

        if (hasValidConnection())
        {
            player.UpdateOldPos();
            player.UpdateOldHealth();
            player.UpdateFragsDeaths();
        }
    }
}

/** 
    Game logic here.
    Game engine invokes this in every frame.
    DO NOT make any unnecessary operations here, as this function must always complete below 16 msecs to keep stable 60 fps!
*/
void PRooFPSddPGE::onGameRunning()
{
    const std::chrono::time_point<std::chrono::steady_clock> timeOnGameRunningStart = std::chrono::steady_clock::now();

    if (m_timeFullRoundtripStart.time_since_epoch().count() != 0)
    {
        m_nFullRoundtripDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(timeOnGameRunningStart - m_timeFullRoundtripStart).count();
    }
    m_timeFullRoundtripStart = timeOnGameRunningStart;

    PureWindow& window = getPure().getWindow();

    m_fps_ms = GetTickCount();
    m_nFramesElapsedSinceLastDurationsReset++;

    // having valid connection means that server accepted the connection and we have initialized our player;
    // otherwise m_mapPlayers[connHandle] is dangerous as it implicitly creates entry even ...
    if (hasValidConnection())
    {
        std::chrono::time_point<std::chrono::steady_clock> timeStart;

        if (getNetwork().isServer())
        {
            timeStart = std::chrono::steady_clock::now();
            if (!m_gameMode->checkWinningConditions())
            {
                Gravity(m_fps);
                PlayerCollisionWithWalls(m_bWon);
            }
            m_nGravityCollisionDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
        }

        Player& player = m_mapPlayers.at(m_nServerSideConnectionHandle); // cannot throw, because of bValidConnection

        if (player.getWeapon())
        {
            // very bad: AddText() should be used, but then RemoveText() would be also needed anytime there is a change ...
            Text(
                player.getWeapon()->getVars()["name"].getAsString() + ": " +
                std::to_string(player.getWeapon()->getMagBulletCount()) + " / " +
                std::to_string(player.getWeapon()->getUnmagBulletCount()),
                10, 150);
        }

        Text(std::to_string(m_fps), getPure().getWindow().getClientWidth() - 50, getPure().getWindow().getClientHeight() - 2 * getPure().getUImanager().getDefaultFontSize());

        timeStart = std::chrono::steady_clock::now();
        if (window.isActive())
        {
            pge_network::PgePacket pkt;
            proofps_dd::MsgUserCmdMove::initPkt(pkt);

            KeyBoard(m_fps, m_bWon, pkt, player);
            Mouse(m_fps, m_bWon, pkt, player);

            player.getAngleY() = (m_pObjXHair->getPosVec().getX() < 0.f) ? 0.f : 180.f;
            player.getObject3D()->getAngleVec().SetY(player.getAngleY());
            if (proofps_dd::MsgUserCmdMove::shouldSend(pkt) ||
                (player.getOldAngleY() != player.getAngleY()))
            {
                proofps_dd::MsgUserCmdMove::setAngleY(pkt, player.getAngleY());
            }

            Weapon* const wpn = player.getWeapon();
            if (wpn)
            {
                // my xhair is used to update weapon angle
                wpn->UpdatePositions(player.getObject3D()->getPosVec(), m_pObjXHair->getPosVec());
                player.getWeaponAngle().Set(0.f, wpn->getObject3D().getAngleVec().getY(), wpn->getObject3D().getAngleVec().getZ());

                if (proofps_dd::MsgUserCmdMove::shouldSend(pkt) || (player.getOldWeaponAngle() != player.getWeaponAngle()))
                {
                    proofps_dd::MsgUserCmdMove::setWpnAngles(pkt, player.getWeaponAngle().getY(), player.getWeaponAngle().getZ());
                }
            }

            if (proofps_dd::MsgUserCmdMove::shouldSend(pkt))
            {   // shouldSend() at this point means that there were actual input so MsgUserCmdMove will be sent out
                if (getNetwork().isServer())
                {
                    // inject this packet to server's queue
                    // server will properly update its own position and send update to all clients too based on current state of handleUserCmdMove()
                    getNetwork().getServer().InjectPacket(pkt);
                }
                else
                {
                    getNetwork().getClient().SendToServer(pkt);
                }
            }
        } // window is active
        m_nActiveWindowStuffDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

        CameraMovement(m_fps, player);

        for (auto& otherPlayerPair : m_mapPlayers)
        {
            Weapon* const wpn = otherPlayerPair.second.getWeapon();
            if (!wpn)
            {
                continue;
            }

            if (isMyConnection(otherPlayerPair.first))
            {
                // this should be done for ourselves too, but it is done only when window is active with different logic
            }
            else
            {
                wpn->UpdatePosition(otherPlayerPair.second.getObject3D()->getPosVec());
            }
        }
        
        if (getNetwork().isServer())
        {
            UpdateWeapons();
            UpdateBullets();
            UpdateRespawnTimers();
            PickupAndRespawnItems();
        }

        m_maps.Update();

        UpdateGameMode();  // TODO: on the long run this should be also executed only by server, now for fraglimit every instance executes ...

        if (getNetwork().isServer())
        {
            SendUserUpdates();
        }

        m_maps.UpdateVisibilitiesForRenderer();
    } // endif validConnection

    m_fps_ms = GetTickCount() - m_fps_ms;
    // this is horrible that FPS measuring is still not available from outside of Pure .........
    m_fps_counter++;
    if ( GetTickCount() - GAME_FPS_INTERVAL >= m_fps_lastmeasure )
    {
        m_fps = m_fps_counter * (1000/GAME_FPS_INTERVAL);
        m_fps_counter = 0;
        m_fps_lastmeasure = GetTickCount();

        std::stringstream str;
        str << GAME_NAME << " " << GAME_VERSION << " :: FPS: " << m_fps;
        window.SetCaption(str.str());
    } 

    m_nFullOnGameRunningDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeOnGameRunningStart).count();
}

/**
    Called when a new network packet is received.

    @return True on successful packet handling, false on serious error that should result in terminating the application.
*/
bool PRooFPSddPGE::onPacketReceived(const pge_network::PgePacket& pkt)
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
    bool bRet;

    switch (pkt.pktId)
    {
    case pge_network::MsgUserConnected::id:
        bRet = handleUserConnected(pkt.m_connHandleServerSide, pkt.msg.userConnected);
        break;
    case pge_network::MsgUserDisconnected::id:
        bRet = handleUserDisconnected(pkt.m_connHandleServerSide, pkt.msg.userDisconnected);
        break;
    case pge_network::MsgApp::id:
    {
        switch (static_cast<proofps_dd::ElteFailMsgId>(pkt.msg.app.msgId))
        {
        case proofps_dd::MsgUserSetup::id:
            bRet = handleUserSetup(pkt.m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgUserSetup&>(pkt.msg.app.cData));
            break;
        case proofps_dd::MsgUserCmdMove::id:
            bRet = handleUserCmdMove(pkt.m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData));
            break;
        case proofps_dd::MsgUserUpdate::id:
            bRet = handleUserUpdate(pkt.m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgUserUpdate&>(pkt.msg.app.cData));
            break;
        case proofps_dd::MsgBulletUpdate::id:
            bRet = handleBulletUpdate(pkt.m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgBulletUpdate&>(pkt.msg.app.cData));
            break;
        case proofps_dd::MsgMapItemUpdate::id:
            bRet = handleMapItemUpdate(pkt.m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgMapItemUpdate&>(pkt.msg.app.cData));
            break;
        case proofps_dd::MsgWpnUpdate::id:
            bRet = handleWpnUpdate(pkt.m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgWpnUpdate&>(pkt.msg.app.cData));
            break;
        case proofps_dd::MsgWpnUpdateCurrent::id:
            bRet = handleWpnUpdateCurrent(pkt.m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgWpnUpdateCurrent&>(pkt.msg.app.cData));
            break;
        default:
            bRet = false;
            getConsole().EOLn("CustomPGE::%s(): unknown msgId %u in MsgApp!", __func__, pkt.msg.app.msgId);
        }
        break;
    }
    default:
        bRet = false;
        getConsole().EOLn("CustomPGE::%s(): unknown pktId %u!", __func__, pkt.pktId);
    }

    m_nFullOnPacketReceivedDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
    return bRet;
}

/**
    Freeing up game content here.
    Free up everything that has been allocated in onGameInitialized() and onGameRunning().
*/
void PRooFPSddPGE::onGameDestroying()
{
    getConsole().OLnOI("PRooFPSddPGE::onGameDestroying() ...");

    //getConsole().SetLoggingState("4LLM0DUL3S", true);
    //getPure().WriteList();
    //getConsole().SetLoggingState("4LLM0DUL3S", false);

    m_mapPlayers.clear(); // Dtors of Player instances will be implicitly called
    m_maps.shutdown();
    m_sServerMapFilenameToLoad.clear();
    delete m_pObjXHair;
    delete m_gameMode;
    getPure().getObject3DManager().DeleteAll();
    getPure().getWindow().SetCursorVisible(true);

    getConsole().OOOLn("PRooFPSddPGE::onGameDestroying() done!");
    getConsole().Deinitialize();
}

void PRooFPSddPGE::genUniqueUserName(char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength]) const
{
    bool found = false;
    do
    {
        sprintf_s(szNewUserName, proofps_dd::MsgUserSetup::nUserNameMaxLength, "User%d", 10000 + (rand() % 100000));
        for (const auto& client : m_mapPlayers)
        {
            found = (client.second.getName() == szNewUserName);
            if (found)
            {
                break;
            }
        }
    } while (found);
}

void PRooFPSddPGE::WritePlayerList()
{
    getConsole().OLnOI("PRooFPSddPGE::%s()", __func__);
    for (const auto& playerPair : m_mapPlayers)
    {
        getConsole().OLn("Username: %s; connHandleServerSide: %u; address: %s",
            playerPair.second.getName().c_str(), playerPair.second.getServerSideConnectionHandle(), playerPair.second.getIpAddress().c_str());
    }
    getConsole().OO();
}

bool PRooFPSddPGE::handleUserSetup(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserSetup& msg)
{
    if ((strnlen(msg.m_szUserName, proofps_dd::MsgUserSetup::nUserNameMaxLength) > 0) && (m_mapPlayers.end() != m_mapPlayers.find(connHandleServerSide)))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): cannot happen: connHandleServerSide: %u (rcvd user name: %s) is already present in players list!",
            __func__, connHandleServerSide, msg.m_szUserName);
        assert(false);
        return false;
    }

    if (strnlen(msg.m_szUserName, sizeof(msg.m_szUserName) == 0))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): cannot happen: user name empty (connHandleServerSide: %u)!",
            __func__, connHandleServerSide);
        assert(false);
        return false;
    }

    if (msg.m_bCurrentClient)
    {
        getConsole().OLn("PRooFPSddPGE::%s(): this is me, my name is %s, connHandleServerSide: %u, my IP: %s, map: %s",
            __func__, msg.m_szUserName, connHandleServerSide, msg.m_szIpAddress, msg.m_szMapFilename);
        m_nServerSideConnectionHandle = connHandleServerSide;

        if (getNetwork().isServer())
        {
            AddText("Server, User name: " + std::string(msg.m_szUserName) +
                (getConfigProfiles().getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
                10, 30);
            // server is not loading map here, it already loaded earlier for itself
        }
        else
        {
            AddText("Client, User name: " + std::string(msg.m_szUserName) + "; IP: " + msg.m_szIpAddress +
                (getConfigProfiles().getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
                10, 30);

            Text("Loading Map: " + std::string(msg.m_szMapFilename) + " ...", 200, getPure().getWindow().getClientHeight() / 2);
            getPure().getRenderer()->RenderScene();

            if (!m_maps.load((GAME_MAPS_DIR + std::string(msg.m_szMapFilename)).c_str()))
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): m_maps.load() failed: %s!", __func__, msg.m_szMapFilename);
                assert(false);
                return false;
            }
        }

        getAudio().play(m_sndLetsgo);
    }
    else
    {
        getConsole().OLn("PRooFPSddPGE::%s(): new user %s (connHandleServerSide: %u; IP: %s) connected",
            __func__, msg.m_szUserName, connHandleServerSide, msg.m_szIpAddress);
    }

    const auto insertRes = m_mapPlayers.insert(
        {
            connHandleServerSide,
            Player(getPure(), connHandleServerSide, msg.m_szIpAddress)
        }); // TODO: emplace_back()
    if (!insertRes.second)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to insert user %s into map!", __func__, msg.m_szUserName);
        assert(false);
        return false;
    }
    Player& insertedPlayer = insertRes.first->second;
    insertedPlayer.setName(msg.m_szUserName);
    
    if (!m_gameMode->addPlayer(insertedPlayer))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to insert player %s into GameMode!", __func__, msg.m_szUserName);
        assert(false);
        return false;
    }

    // each client will load all weapons into their weaponManager for their own setup, when they initialie themselves,
    // these will be the reference weapons, never visible, never moving, just to be copied!
    if (msg.m_bCurrentClient)
    {
        for (const auto& entry : std::filesystem::directory_iterator(GAME_WEAPONS_DIR))
        {
            getConsole().OLn("PRooFPSddPGE::%s(): %s!", __func__, entry.path().filename().string().c_str());
            if (entry.path().filename().string().length() >= proofps_dd::MsgWpnUpdate::nWpnNameNameMaxLength)
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): skip wpn due to long filename: %s!", __func__, entry.path().string().c_str());
                continue; // otherwise multiple weapons with same first nWpnNameNameMaxLength-1 chars would be mixed up in pkts
            }

            if (entry.path().extension().string() == ".txt")
            {
                if (!getWeaponManager().load(entry.path().string().c_str(), connHandleServerSide))
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): failed to load weapon: %s!", __func__, entry.path().string().c_str());
                    assert(false);
                    return false;
                }
            }
        }

        // TODO: server should send the default weapon to client in this setup message, but for now we set same hardcoded
        // value on both side ... cheating is not possible anyway, since on server side server will always know what is
        // the default weapon for the player, so there is no use of overriding it on client side ...
        if (!getWeaponManager().setDefaultAvailableWeaponByFilename(GAME_WPN_DEFAULT))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): failed to set default weapon: %s!", __func__, GAME_WPN_DEFAULT);
            assert(false);
            return false;
        }
    }

    Weapon* const wpnDefaultAvailable = getWeaponManager().getWeaponByFilename(getWeaponManager().getDefaultAvailableWeaponFilename());
    if (!wpnDefaultAvailable)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to get default weapon!", __func__);
        assert(false);
        return false;
    }

    // and here the actual weapons for the specific player, these can be visible when active, moving with player, etc.
    // client doesnt do anything else with these, server also uses these to track and validate player weapons and related actions.
    for (const auto pWpn : getWeaponManager().getWeapons())
    {
        if (!pWpn)
        {
            continue;
        }

        Weapon* const pNewWeapon = new Weapon(*pWpn);
        insertedPlayer.getWeapons().push_back(pNewWeapon);
        pNewWeapon->SetOwner(connHandleServerSide);
        pNewWeapon->getObject3D().SetName(pNewWeapon->getObject3D().getName() + " (copied Weapon for user " + msg.m_szUserName + ")");
        if (pNewWeapon->getFilename() == wpnDefaultAvailable->getFilename())
        {
            pNewWeapon->SetAvailable(true);
            insertedPlayer.SetWeapon(pNewWeapon, false, getNetwork().isServer());
        }
    }

    if (!insertedPlayer.getWeapon())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): no default weapon selected for user %s!", __func__, msg.m_szUserName);
        assert(false);
        return false;
    }

    // Note that this is a waste of resources this way.
    // Because clients also store the full weapon instances for themselves, even though they dont use weapon cvars at all!
    // Task: On the long run, there should be a WeaponProxy or WeaponClient or something for the clients which are basically
    // just the image for their current weapon.
    // Task: And I also think that each Player should have a WeaponManager instance, so player's weapons would be loaded there.
    // Task: the only problem here would be that the bullets list should be extracted into separate entity, so all WeaponManager
    // instances would refer to the same bullets list. And some functions may be enough to be static, but thats all!

    getNetwork().WriteList();
    WritePlayerList();

    return true;
}

bool PRooFPSddPGE::handleUserConnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserConnected& msg)
{
    if (!getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): client received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    const char* szConnectedUserName = nullptr;
    pge_network::PgePacket newPktUserUpdate;

    if (msg.bCurrentClient)
    {
        // server is processing its own birth
        if (m_mapPlayers.size() == 0)
        {
            Text("Loading Map: " + m_sServerMapFilenameToLoad + " ...", 200, getPure().getWindow().getClientHeight() / 2);
            getPure().getRenderer()->RenderScene();

            Bullet::ResetGlobalBulletId();

            // server already loads the map for itself at this point, so no need for map filename in PktSetup, but we fill it anyway ...
            //const bool mapLoaded = m_maps.load("gamedata/maps/map_test_good.txt");
            if (!m_maps.load((std::string(GAME_MAPS_DIR) + m_sServerMapFilenameToLoad).c_str()))
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): m_maps.load() failed: %s!", __func__, m_sServerMapFilenameToLoad.c_str());
                assert(false);
                return false;
            }

            char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength];
            genUniqueUserName(szNewUserName);
            getConsole().OLn("PRooFPSddPGE::%s(): first (local) user %s connected and I'm server, so this is me (connHandleServerSide: %u)",
                __func__, szNewUserName, connHandleServerSide);

            szConnectedUserName = szNewUserName;

            pge_network::PgePacket newPktSetup;
            proofps_dd::MsgUserSetup::initPkt(newPktSetup, connHandleServerSide, true, szConnectedUserName, msg.szIpAddress, m_maps.getFilename());

            const PureVector& vecStartPos = getConfigProfiles().getVars()["testing"].getAsBool() ?
                m_maps.getLeftMostSpawnpoint() :
                m_maps.getRandomSpawnpoint();

            proofps_dd::MsgUserUpdate::initPkt(
                newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, 0.f, 100, false, 0, 0);

            // server injects this msg to self so resources for player will be allocated
            getNetwork().getServer().InjectPacket(newPktSetup);
            getNetwork().getServer().InjectPacket(newPktUserUpdate);
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
        // server is processing another user's birth
        if (m_mapPlayers.empty())
        {
            // cannot happen because at least the user of the server should be in the map!
            // this should happen only if we are dedicated server but currently only listen-server is supported!
            getConsole().EOLn("PRooFPSddPGE::%s(): non-server user (connHandleServerSide: %u) connected but map of players is still empty, CANNOT HAPPEN!",
                __func__, connHandleServerSide);
            assert(false);
            return false;
        }

        char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength];
        genUniqueUserName(szNewUserName);
        szConnectedUserName = szNewUserName;
        getConsole().OLn("PRooFPSddPGE::%s(): new remote user %s (connHandleServerSide: %u) connected (from %s) and I'm server",
            __func__, szConnectedUserName, connHandleServerSide, msg.szIpAddress);

        pge_network::PgePacket newPktSetup;
        proofps_dd::MsgUserSetup::initPkt(newPktSetup, connHandleServerSide, false, szConnectedUserName, msg.szIpAddress, m_maps.getFilename());

        const PureVector& vecStartPos = getConfigProfiles().getVars()["testing"].getAsBool() ?
            m_maps.getRightMostSpawnpoint() :
            m_maps.getRandomSpawnpoint();

        proofps_dd::MsgUserUpdate::initPkt(
            newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, 0.f, 100, false, 0, 0);

        // server injects this msg to self so resources for player will be allocated
        getNetwork().getServer().InjectPacket(newPktSetup);
        getNetwork().getServer().InjectPacket(newPktUserUpdate);

        // inform all other clients about this new user
        getNetwork().getServer().SendPacketToAllClients(newPktSetup, connHandleServerSide);
        getNetwork().getServer().SendPacketToAllClients(newPktUserUpdate, connHandleServerSide);

        // now we send this msg to the client with this bool flag set so client will know it is their connect
        proofps_dd::MsgUserSetup& msgUserSetup = reinterpret_cast<proofps_dd::MsgUserSetup&>(newPktSetup.msg.app.cData);
        msgUserSetup.m_bCurrentClient = true;
        getNetwork().getServer().SendPacketToClient(connHandleServerSide, newPktSetup);
        getNetwork().getServer().InjectPacket(newPktUserUpdate);

        // we also send as many MsgUserSetup pkts to the client as the number of already connected players,
        // otherwise client won't know about them, so this way the client will detect them as newly connected users;
        // we also send MsgUserUpdate about each player so new client will immediately have their positions and other data updated.
        for (const auto& it : m_mapPlayers)
        {
            proofps_dd::MsgUserSetup::initPkt(
                newPktSetup,
                it.second.getServerSideConnectionHandle(),
                false,
                it.second.getName(), it.second.getIpAddress(),
                "" /* here mapFilename is irrelevant */);
            getNetwork().getServer().SendPacketToClient(connHandleServerSide, newPktSetup);

            proofps_dd::MsgUserUpdate::initPkt(
                newPktUserUpdate,
                it.second.getServerSideConnectionHandle(),
                it.second.getObject3D()->getPosVec().getX(),
                it.second.getObject3D()->getPosVec().getY(),
                it.second.getObject3D()->getPosVec().getZ(),
                it.second.getObject3D()->getAngleVec().getY(),
                it.second.getWeapon()->getObject3D().getAngleVec().getY(),
                it.second.getWeapon()->getObject3D().getAngleVec().getZ(),
                it.second.getHealth(),
                false,
                it.second.getFrags(),
                it.second.getDeaths());
            getNetwork().getServer().SendPacketToClient(connHandleServerSide, newPktUserUpdate);
        }

        // we also send the state of all map items
        pge_network::PgePacket newPktMapItemUpdate;
        for (auto& itemPair : m_maps.getItems())
        {
            if (!itemPair.second)
            {
                continue;
            }

            proofps_dd::MsgMapItemUpdate::initPkt(
                newPktMapItemUpdate,
                0,
                itemPair.first,
                itemPair.second->isTaken());
            getNetwork().getServer().SendPacketToClient(connHandleServerSide, newPktMapItemUpdate);
        }
    }

    return true;
}

bool PRooFPSddPGE::handleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnected&)
{
    const auto playerIt = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == playerIt)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        assert(false); // in debug mode, try to understand this scenario
        return true; // in release mode, dont terminate
    }

    if (getNetwork().isServer())
    {
        getConsole().OLn("PRooFPSddPGE::%s(): user %s disconnected and I'm server", __func__, playerIt->second.getName().c_str());
    }
    else
    {
        getConsole().OLn("PRooFPSddPGE::%s(): user %s disconnected and I'm client", __func__, playerIt->second.getName().c_str());
    }

    m_gameMode->removePlayer(playerIt->second);
    m_mapPlayers.erase(playerIt);

    getNetwork().WriteList();
    WritePlayerList();

    return true;
}

bool PRooFPSddPGE::handleUserCmdMove(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserCmdMove& pktUserCmdMove)
{
    if (!getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): client received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        assert(false);  // in debug mode this terminates server
        return true;    // in release mode, we dont terminate the server, just silently ignore
    }

    const std::string& sClientUserName = it->second.getName();

    if ((pktUserCmdMove.m_strafe == proofps_dd::Strafe::NONE) &&
        (!pktUserCmdMove.m_bJumpAction) && (!pktUserCmdMove.m_bSendSwitchToRunning) &&
        (pktUserCmdMove.m_fPlayerAngleY == -1.f) && (!pktUserCmdMove.m_bRequestReload) && (!pktUserCmdMove.m_bShouldSend))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): user %s sent invalid cmdMove!", __func__, sClientUserName.c_str());
        assert(false);  // in debug mode this terminates server
        return false;   // in release mode, we dont terminate the server, just silently ignore
        // TODO: I might disconnect this client!
    }

    auto& player = it->second;

    if (player.getHealth() == 0)
    {
        if (pktUserCmdMove.m_bShootAction)
        {
            //getConsole().OLn("PRooFPSddPGE::%s(): user %s is requesting respawn", __func__, sClientUserName.c_str());
            return true;
        }

        // for dead player, only the shoot action is allowed which is treated as respawn request
        //getConsole().OLn("PRooFPSddPGE::%s(): ignoring cmdMove for user %s due to health is 0!", __func__, sClientUserName.c_str());
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

    if ( pktUserCmdMove.m_strafe == proofps_dd::Strafe::LEFT )
    {
        if (!player.isJumping() && !player.isFalling() && player.jumpAllowed())
        {
            player.getPos().SetX(player.getPos().getX() - fSpeed);
        }
    }
    if ( pktUserCmdMove.m_strafe == proofps_dd::Strafe::RIGHT )
    {
        if (!player.isJumping() && !player.isFalling() && player.jumpAllowed())
        {
            player.getPos().SetX(player.getPos().getX() + fSpeed);
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

    Weapon* const wpn = player.getWeapon();
    if (!wpn)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): getWeapon() failed!", __func__);
        assert(false);
        return false;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    if (!pktUserCmdMove.m_bShootAction)
    {
        if (!wpn->isTriggerReleased())
        {
            //getConsole().OLn("PRooFPSddPGE::%s(): player %s released trigger!", 
            //    __func__, sClientUserName.c_str());
        }
        wpn->releaseTrigger();
    }

    if (pktUserCmdMove.m_bRequestReload)
    {
        if (wpn->reload())
        {
            //getConsole().OLn("PRooFPSddPGE::%s(): player %s reloading the weapon!",
            //    __func__, sClientUserName.c_str());
        }
        else
        {
            //getConsole().OLn("PRooFPSddPGE::%s(): player %s requested reload but we ignore it!",
            //    __func__, sClientUserName.c_str());
        }
    }

    if (!pktUserCmdMove.m_bRequestReload && (wpn->getState() == Weapon::State::WPN_READY) && (pktUserCmdMove.m_cWeaponSwitch != '\0'))
    {
        const auto itTargetWpn = m_mapKeypressToWeapon.find(pktUserCmdMove.m_cWeaponSwitch);
        if (itTargetWpn == m_mapKeypressToWeapon.end())
        {
            const std::string sc = std::to_string(pktUserCmdMove.m_cWeaponSwitch); // because CConsole still doesnt support %c!
            getConsole().EOLn("PRooFPSddPGE::%s(): weapon not found for char %s!", __func__, sc.c_str());
            assert(false);
            return false;
        }

        Weapon* const pTargetWpn = player.getWeaponByFilename(itTargetWpn->second.m_sWpnFilename);
        if (!pTargetWpn)
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): weapon not found for name %s!", __func__, itTargetWpn->second.m_sWpnFilename.c_str());
            assert(false);
            return false;
        }

        if (!pTargetWpn->isAvailable())
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): weapon not found for name %s!", __func__, itTargetWpn->second.m_sWpnFilename.c_str());
            assert(false);  // in debug mode, must abort because CLIENT should had not sent weapon switch request if they don't have this wpn!
            return true;    // in release mode, dont terminate the server, just silently ignore!
            // TODO: I might disconnect this client!
        }

        if (pTargetWpn != player.getWeapon())
        {
            if (isMyConnection(connHandleServerSide))
            {   // server plays for itself
                getAudio().play(m_sndChangeWeapon);
            }
            player.SetWeapon(pTargetWpn, true, getNetwork().isServer());
            //getConsole().OLn("PRooFPSddPGE::%s(): player %s switching to %s!",
            //    __func__, sClientUserName.c_str(), itTargetWpn->second.m_sWpnFilename.c_str());

            // all clients must be updated about this player's weapon switch
            for (const auto& client : m_mapPlayers)
            {
                if (client.first != m_nServerSideConnectionHandle)
                {   // server doesn't need to send this msg to itself, it already executed weapon change by SetWeapon()
                    pge_network::PgePacket pktWpnUpdateCurrent;
                    proofps_dd::MsgWpnUpdateCurrent::initPkt(
                        pktWpnUpdateCurrent,
                        connHandleServerSide,
                        pTargetWpn->getFilename());
                    getNetwork().getServer().SendPacketToClient(client.first, pktWpnUpdateCurrent);
                    //getConsole().OLn("PRooFPSddPGE::%s(): sent MsgWpnUpdateCurrent to %u about new wpn of %u!", __func__, client.first, connHandleServerSide);
                }
            }
        }
        else
        {
            // should not happen because client should NOT send message in such case
            getConsole().OLn("PRooFPSddPGE::%s(): player %s already has target wpn %s, CLIENT SHOULD NOT SEND THIS!",
                __func__, sClientUserName.c_str(), itTargetWpn->second.m_sWpnFilename.c_str());
            assert(false);  // in debug mode, terminate the game
            return true;   // in release mode, dont terminate the server, just silently ignore!
            // TODO: I might disconnect this client!
        }
    }

    // TODO: this should be moved up, so returning from function is easier for rest of action handling code
    player.getWeaponAngle().Set(0.f, pktUserCmdMove.m_fWpnAngleY, pktUserCmdMove.m_fWpnAngleZ);
    wpn->getObject3D().getAngleVec().SetY(pktUserCmdMove.m_fWpnAngleY);
    wpn->getObject3D().getAngleVec().SetZ(pktUserCmdMove.m_fWpnAngleZ);

    if (pktUserCmdMove.m_bRequestReload || (wpn->getState() != Weapon::State::WPN_READY) || (pktUserCmdMove.m_cWeaponSwitch != '\0'))
    {
        // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
        m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
        return true; // don't check anything related to shooting in case of either of these actions
    }

    if (pktUserCmdMove.m_bShootAction)
    {
        const auto nSecsSinceLastWeaponSwitch =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - player.getTimeLastWeaponSwitch()
                ).count();
        if (nSecsSinceLastWeaponSwitch < m_nWeaponActionMinimumWaitMillisecondsAfterSwitch)
        {
            //getConsole().OLn("PRooFPSddPGE::%s(): ignoring too early mouse action!", __func__);
            // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
            m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
            return true;
        }

        // server will have the new bullet, clients will learn about the new bullet when server is sending out
        // the regular bullet updates;
        if (wpn->pullTrigger())
        {
            // but we send out the wpn update for bullet count change here for that single client
            if (connHandleServerSide != m_nServerSideConnectionHandle) // server doesn't need to send this msg to itself, it already executed bullet count change by pullTrigger()
            {
                pge_network::PgePacket pktWpnUpdate;
                proofps_dd::MsgWpnUpdate::initPkt(
                    pktWpnUpdate,
                    0 /* ignored by client anyway */,
                    wpn->getFilename(),
                    wpn->isAvailable(),
                    wpn->getMagBulletCount(),
                    wpn->getUnmagBulletCount());
                getNetwork().getServer().SendPacketToClient(it->second.getServerSideConnectionHandle(), pktWpnUpdate);
            }
            else
            {
                // here server plays the firing sound, clients play for themselves when they receive newborn bullet update
                // not nice, but this is just some temporal solution for private beta
                if (wpn->getFilename() == "pistol.txt")
                {
                    getAudio().play(m_sndShootPistol);
                }
                else if (wpn->getFilename() == "machinegun.txt")
                {
                    getAudio().play(m_sndShootMchgun);
                }
                else
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): did not find correct weapon name for: %s!", __func__, wpn->getFilename().c_str());
                    assert(false);
                    return false;
                }
            }
        }
    }
    // TODO: not nice: an object should be used, which is destructed upon return, its dtor adds the time elapsed since its ctor!
    m_nHandleUserCmdMoveDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

    return true;
}

bool PRooFPSddPGE::handleUserUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserUpdate& msg)
{
    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return true;  // might NOT be fatal error in some circumstances, although I cannot think about any, but dont terminate the app for this ...
    }

    //getConsole().OLn("PRooFPSddPGE::%s(): user %s received MsgUserUpdate: %f", __func__, it->first.c_str(), msg.m_pos.x);

    if (it->second.isExpectingStartPos())
    {
        it->second.SetExpectingStartPos(false);
        it->second.getPos().Set(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);
        it->second.getOPos().Set(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);
    }

    it->second.getObject3D()->getPosVec().Set(
        msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);

    if (msg.m_fPlayerAngleY != -1.f)
    {
        //it->second.getAngleY() = msg.m_fPlayerAngleY;
        it->second.getObject3D()->getAngleVec().SetY(msg.m_fPlayerAngleY);
    }

    it->second.getWeapon()->getObject3D().getAngleVec().SetY(msg.m_fWpnAngleY);
    it->second.getWeapon()->getObject3D().getAngleVec().SetZ(msg.m_fWpnAngleZ);

    //getConsole().OLn("PRooFPSddPGE::%s(): rcvd health: %d, health: %d, old health: %d",
    //    __func__, msg.m_nHealth, it->second.getHealth(), it->second.getOldHealth());

    it->second.getFrags() = msg.m_nFrags;
    it->second.getDeaths() = msg.m_nDeaths;

    it->second.SetHealth(msg.m_nHealth);

    if (msg.m_bRespawn)
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): player %s has respawned!", __func__, it->first.c_str());
        HandlePlayerRespawned(it->second);
    }
    else
    {
        if ((it->second.getOldHealth() > 0) && (it->second.getHealth() == 0))
        {
            // only clients fall here, since server already set oldhealth to 0 at the beginning of this frame
            // because it had already set health to 0 in previous frame
            //getConsole().OLn("PRooFPSddPGE::%s(): player %s has died!", __func__, it->first.c_str());
            HandlePlayerDied(it->second);
        }
    }

    if (!m_gameMode->updatePlayer(it->second))
    {
        getConsole().EOLn("%s: failed to update player %s in GameMode!", __func__, it->second.getName().c_str());
    }

    return true;
}

bool PRooFPSddPGE::handleBulletUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgBulletUpdate& msg)
{
    if (getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    Bullet* pBullet = nullptr;

    auto it = getWeaponManager().getBullets().begin();
    while (it != getWeaponManager().getBullets().end())
    {
        if (it->getId() == msg.m_bulletId)
        {
            break;
        }
        it++;
    }

    if (getWeaponManager().getBullets().end() == it)
    {
        if (msg.m_bDelete)
        {
            // this is valid scenario: when 2 players are at almost same position (partially overlapping), the bullet will immediately hit the other player
            // when being fired. In such case, we can just ignore doing anything here on client side.
            // TODO: btw why does sender send the message like this anyway to clients?!
            return true;
        }
        // need to create this new bullet first on our side
        //getConsole().OLn("PRooFPSddPGE::%s(): user %s received MsgBulletUpdate: NEW bullet id %u", __func__, m_sUserName.c_str(), msg.m_bulletId);

        const auto playerIt = m_mapPlayers.find(m_nServerSideConnectionHandle);
        if (playerIt == m_mapPlayers.end())
        {
            // must always find self player
            return false;
        }

        Player& player = playerIt->second;
        if (player.getServerSideConnectionHandle() == connHandleServerSide)
        {
            // this is my newborn bullet
            // I'm playing the sound associated to my current weapon, although it might happen that with BIG latency, when I receive this update from server,
            // I have already switched to another weapon ... but I think this cannot happen since my inputs are processed and responded by server in order.
            const Weapon* const wpn = player.getWeapon();
            if (!wpn)
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): getWeapon() failed!", __func__);
                assert(false);
                return false;
            }
            else
            {
                // not nice, but this is just some temporal solution for private beta
                if (wpn->getFilename() == "pistol.txt")
                {
                    getAudio().play(m_sndShootPistol);
                }
                else if (wpn->getFilename() == "machinegun.txt")
                {
                    getAudio().play(m_sndShootMchgun);
                }
                else
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): did not find correct weapon name for: %s!", __func__, wpn->getFilename().c_str());
                    assert(false);
                    return false;
                }
            }
        }

        // TODO: here it is okay to get all the properties of the bullet, but if it is not a new bullet, it is not nice to
        // send every property in all BulletUpdate, this should be improved in future ...
        getWeaponManager().getBullets().push_back(
            Bullet(
                getPure(),
                msg.m_bulletId,
                msg.m_pos.x, msg.m_pos.y, msg.m_pos.z,
                msg.m_angle.x, msg.m_angle.y, msg.m_angle.z,
                msg.m_size.x, msg.m_size.y, msg.m_size.z) );
        pBullet = &(getWeaponManager().getBullets().back());
        it = getWeaponManager().getBullets().end();
        it--; // iterator points to this newly inserted last bullet
    }
    else
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): user %s received MsgBulletUpdate: old bullet id %u", __func__, m_sUserName.c_str(), msg.m_bulletId);
        pBullet = &(*it);
    }

    if (msg.m_bDelete)
    {
        getWeaponManager().getBullets().erase(it);
        return true;
    }

    pBullet->getObject3D().getPosVec().Set(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);

    return true;
}

bool PRooFPSddPGE::handleMapItemUpdate(pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/, const proofps_dd::MsgMapItemUpdate& msg)
{
    if (getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    const auto it = m_maps.getItems().find(msg.m_mapItemId);
    if (it == m_maps.getItems().end())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): unknown map item id %u, CANNOT HAPPEN!", __func__, msg.m_mapItemId);
        assert(false);
        return false;
    }

    MapItem* const pMapItem = it->second;

    if (msg.m_bTaken)
    {
        pMapItem->Take();
    }
    else
    {
        pMapItem->UnTake();
    }

    return true;
}

bool PRooFPSddPGE::handleWpnUpdate(
    pge_network::PgeNetworkConnectionHandle /* connHandleServerSide, not filled properly by server so we ignore it */,
    const proofps_dd::MsgWpnUpdate& msg)
{
    if (getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    //getConsole().OLn("PRooFPSddPGE::%s(): received: %s, available: %s, mag: %u, unmag: %u!",
    //    __func__, msg.m_szWpnName, msg.m_bAvailable ? "yes" : "no", msg.m_nMagBulletCount, msg.m_nUnmagBulletCount);

    if (!hasValidConnection())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): my connection is invalid, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    const auto playerIt = m_mapPlayers.find(m_nServerSideConnectionHandle);
    if (playerIt == m_mapPlayers.end())
    {
        // must always find self player
        return false;
    }

    Player& player = playerIt->second;
    Weapon* const wpn = player.getWeaponByFilename(msg.m_szWpnName);
    if (!wpn)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): did not find wpn: %s!", __func__, msg.m_szWpnName);
        assert(false);
        return false;
    }

    wpn->SetAvailable(msg.m_bAvailable);
    wpn->SetMagBulletCount(msg.m_nMagBulletCount);
    wpn->SetUnmagBulletCount(msg.m_nUnmagBulletCount);

    return true;
}

bool PRooFPSddPGE::handleWpnUpdateCurrent(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgWpnUpdateCurrent& msg)
{
    if (getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    //getConsole().OLn("PRooFPSddPGE::%s(): received: %s for player %u",  __func__, msg.m_szWpnCurrentName, connHandleServerSide);

    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        assert(false);
        return false;
    }

    Weapon* const wpn = it->second.getWeaponByFilename(msg.m_szWpnCurrentName);
    if (!wpn)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): did not find wpn: %s!", __func__, msg.m_szWpnCurrentName);
        assert(false);
        return false;
    }

    if (isMyConnection(it->first))
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): this current weapon update is changing my current weapon!", __func__);
        getAudio().play(m_sndChangeWeapon);
    }

    it->second.SetWeapon(
        wpn,
        true /* even client should record last switch time to be able to check it on client side too */,
        getNetwork().isServer());

    return true;
}

void PRooFPSddPGE::RegTestDumpToFile() 
{
    std::ofstream fRegTestDump(getNetwork().isServer() ? GAME_REG_TEST_DUMP_FILE_SERVER : GAME_REG_TEST_DUMP_FILE_CLIENT);
    if (fRegTestDump.fail())
    {
        getConsole().EOLn("%s ERROR: couldn't create file: %s", __func__, getNetwork().isServer() ? GAME_REG_TEST_DUMP_FILE_SERVER : GAME_REG_TEST_DUMP_FILE_CLIENT);
        return;
    }

    // ahhh this is nonsense, we should really refactor client and server to have the same ancestor class!
    if (getNetwork().isServer())
    {
        fRegTestDump << "Tx: Total Pkt Count, Pkt/Second" << std::endl;
        fRegTestDump << "  " << getNetwork().getServer().getTxPacketCount() << std::endl;
        fRegTestDump << "  " << getNetwork().getServer().getTxPacketPerSecondCount() << std::endl;
        fRegTestDump << "Rx: Total Pkt Count, Pkt/Second" << std::endl;
        fRegTestDump << "  " << getNetwork().getServer().getRxPacketCount() << std::endl;
        fRegTestDump << "  " << getNetwork().getServer().getRxPacketPerSecondCount() << std::endl;
        fRegTestDump << "Inject: Total Pkt Count, Pkt/Second" << std::endl;
        fRegTestDump << "  " << getNetwork().getServer().getInjectPacketCount() << std::endl;
        fRegTestDump << "  " << getNetwork().getServer().getInjectPacketPerSecondCount() << std::endl;
    }
    else
    {
        fRegTestDump << "Tx: Total Pkt Count, Pkt/Second" << std::endl;
        fRegTestDump << "  " << getNetwork().getClient().getTxPacketCount() << std::endl;
        fRegTestDump << "  " << getNetwork().getClient().getTxPacketPerSecondCount() << std::endl;
        fRegTestDump << "Rx: Total Pkt Count, Pkt/Second" << std::endl;
        fRegTestDump << "  " << getNetwork().getClient().getRxPacketCount() << std::endl;
        fRegTestDump << "  " << getNetwork().getClient().getRxPacketPerSecondCount() << std::endl;
        fRegTestDump << "Inject: Total Pkt Count, Pkt/Second" << std::endl;
        fRegTestDump << "  " << getNetwork().getClient().getInjectPacketCount() << std::endl;
        fRegTestDump << "  " << getNetwork().getClient().getInjectPacketPerSecondCount() << std::endl;
    }

    fRegTestDump << "Frag Table: Player Name, Frags, Deaths" << std::endl;
    for (const auto& player : m_deathMatchMode->getFragTable())
    {
        fRegTestDump << "  " << player.m_sName << std::endl;
        fRegTestDump << "  " << player.m_nFrags << std::endl;
        fRegTestDump << "  " << player.m_nDeaths << std::endl;
    }

    // add an extra empty line, so the regression test can easily detect end of frag table
    fRegTestDump << std::endl;

    const auto selfPlayerIt = m_mapPlayers.find(m_nServerSideConnectionHandle);
    if (selfPlayerIt == m_mapPlayers.end())
    {
        // must always find self player
        return;
    }

    Player& selfPlayer = selfPlayerIt->second;

    fRegTestDump << "Weapons Available: Weapon Filename, Mag Bullet Count, Unmag Bullet Count" << std::endl;
    for (const auto& wpn : selfPlayer.getWeapons())
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
    fRegTestDump << "  " << selfPlayer.getHealth() << std::endl;

    fRegTestDump.flush();
    fRegTestDump.close();
}
