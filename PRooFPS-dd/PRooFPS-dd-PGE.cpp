/*
    ###################################################################################
    PRooFPSddPGE.cpp
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022 - 2023
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "PRooFPS-dd-PGE.h"

#include <filesystem>  // requires cpp17
#include <functional>
#include <iomanip>     // std::setprecision() for displaying fps
#include <utility>

#include "Pure/include/external/Render/PureRendererHWfixedPipe.h"  // for rendering hints
#include "Pure/include/external/PureUiManager.h"
#include "Pure/include/external/Display/PureWindow.h"
#include "Pure/include/external/PureCamera.h"
#include "../../Console/CConsole/src/CConsole.h"

using namespace std::chrono_literals;

static constexpr unsigned int GAME_FPS_MEASURE_INTERVAL = 500;
static_assert(GAME_FPS_MEASURE_INTERVAL > 0);

static constexpr float GAME_CAM_Z = -5.0f;
static constexpr float GAME_CAM_SPEED_X = 0.1f;
static constexpr float GAME_CAM_SPEED_Y = 0.3f;


// ############################### PUBLIC ################################


proofps_dd::PRooFPSddPGE* proofps_dd::PRooFPSddPGE::createAndGetPRooFPSddPGEinstance()
{
    static proofps_dd::PRooFPSddPGE pgeInstance((proofps_dd::GAME_NAME + " " + proofps_dd::GAME_VERSION).c_str());
    return &pgeInstance;
}

CConsole& proofps_dd::PRooFPSddPGE::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}


const char* proofps_dd::PRooFPSddPGE::getLoggerModuleName()
{
    return "PRooFPSddPGE";
}


// ############################## PROTECTED ##############################


/**
    This is the only usable ctor, this is used by the static createAndGet().
*/
proofps_dd::PRooFPSddPGE::PRooFPSddPGE(const char* gameTitle) :
    PGE(gameTitle),
    proofps_dd::InputHandling(
        *this, /* Hint: for 1-param ctors, use: static_cast<PGE&>(*this) so it will call the only ctor, not the deleted copy ctor. */
        m_durations,
        m_mapPlayers,
        m_maps,
        m_sounds),
    proofps_dd::Physics(
        *this,
        m_durations,
        m_gui,
        m_mapPlayers,
        m_maps,
        m_sounds),
    proofps_dd::PlayerHandling(
        *this,
        m_durations,
        m_gui,
        m_mapPlayers,
        m_maps,
        m_sounds),
    proofps_dd::WeaponHandling(
        *this,
        m_durations,
        m_gui,
        m_mapPlayers,
        m_maps,
        m_sounds),
    m_config(Config::getConfigInstance(*this, m_maps)),
    m_gui(GUI::getGuiInstance(*this, m_config, m_maps, *this)),
    m_gameMode(nullptr),
    m_deathMatchMode(nullptr),
    m_maps(getConfigProfiles(), getPure()),
    m_fps(GAME_MAXFPS),
    m_fps_counter(0),
    m_fps_lastmeasure(0),
    m_bFpsFirstMeasure(true),
    m_pObjXHair(NULL),
    m_bWon(false),
    m_fCameraMinY(0.0f),
    m_nSendClientUpdatesInEveryNthTick(1),
    m_nSendClientUpdatesCntr(m_nSendClientUpdatesInEveryNthTick)
{
}

proofps_dd::PRooFPSddPGE::~PRooFPSddPGE()
{

}

/**
    Must-have minimal stuff before loading anything.
    Game engine calls this before even finishing its own initialization.
*/
bool proofps_dd::PRooFPSddPGE::onGameInitializing()
{
    // Earliest we can enable our own logging
    getConsole().Initialize((proofps_dd::GAME_NAME + " " + proofps_dd::GAME_VERSION + " log").c_str(), true);
    getConsole().SetLoggingState(getLoggerModuleName(), true);
    getConsole().SetFGColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "999999" );
    getConsole().SetIntsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    getConsole().SetStringsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "FFFFFF" );
    getConsole().SetFloatsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    getConsole().SetBoolsColor( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FFFF" );

    // proofps_dd::PRooFPSddPGE (game) logs
    getConsole().SetLoggingState(getLoggerModuleName(), true);

    // Network logs
    getConsole().SetLoggingState("PgeGnsWrapper", true);
    getConsole().SetLoggingState("PgeGnsServer", true);
    getConsole().SetLoggingState("PgeGnsClient", true);
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
bool proofps_dd::PRooFPSddPGE::onGameInitialized()
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
    
    setGameRunningFrequency(GAME_MAXFPS);
    getConsole().OLn("Game running frequency: %u Hz", getGameRunningFrequency());

    getPure().getCamera().SetNearPlane(0.1f);
    getPure().getCamera().SetFarPlane(100.0f);
    getPure().getCamera().getPosVec().Set( 0, 0, GAME_CAM_Z );
    getPure().getCamera().getTargetVec().Set( 0, 0, -proofps_dd::GAME_BLOCK_SIZE_Z );

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

    // make the xhair earlier than the loading screen, so whenever loading screen is visible, xhair stays behind it!
    // this is needed because it is not trivial when to show/hide the xhair for the server.
    m_pObjXHair = getPure().getObject3DManager().createPlane(32.f, 32.f);
    m_pObjXHair->SetStickedToScreen(true);
    m_pObjXHair->SetDoubleSided(true);
    m_pObjXHair->SetTestingAgainstZBuffer(false);
    m_pObjXHair->SetLit(false);
    // for bitmaps not having proper alpha bits (e.g. saved by irfanview or mspaint), use (PURE_SRC_ALPHA, PURE_ONE)
    // otherwise (bitmaps saved by Flash) just use (PURE_SRC_ALPHA, PURE_ONE_MINUS_SRC_ALPHA) to utilize real alpha
    m_pObjXHair->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE);
    PureTexture* xhairtex = getPure().getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "hud_xhair.bmp").c_str());
    m_pObjXHair->getMaterial().setTexture(xhairtex);
    m_pObjXHair->Hide();

    // let the GUI create loading screen AFTER we created the xhair because otherwise in some situations the xhair
    // might appear ABOVE the loading screen ... this is still related to the missing PURE feature: custom Z-ordering of 2D objects.
    m_gui.initialize();

    m_cbDisplayMapLoadingProgressUpdate = [this](int nProgress)
    {
        showLoadingScreen(nProgress);
    };

    getPure().WriteList();

    if (!initializeWeaponHandling())
    {
        getConsole().EOLnOO("ERROR: initializeWeaponHandling() failed!");
        return false;
    }

    // We tell the names of our app messages to the network engine so it can properly log message stats with message names
    for (const auto& msgAppId2StringPair : MapMsgAppId2String)
    {
        getNetwork().getServerClientInstance()->getMsgAppId2StringMap().insert(
            { static_cast<pge_network::MsgApp::TMsgId>(msgAppId2StringPair.msgId),
              std::string(msgAppId2StringPair.zstring) }
        );
    }

    // TODO: log level override support: getConsole().SetLoggingState(sTrimmedLine.c_str(), true);

    allowListAppMessages();

    if (getNetwork().isServer() && !getConfigProfiles().getVars()[proofps_dd::GUI::CVAR_GUI_MAINMENU].getAsBool())
    {
        if (m_maps.serverDecideFirstMapAndUpdateNextMapToBeLoaded().empty())
        {
            getConsole().EOLnOO("ERROR: Server is unable to select first map!");
            PGE::showErrorDialog("Server is unable to select first map!");
            return false;
        }
    }

    m_config.validate();

    getConsole().OLn("");
    getConsole().OLn("size of PgePacket: %u Bytes", sizeof(pge_network::PgePacket));
    getConsole().OLn("  size of MsgUserCmdFromClient: %u Bytes", sizeof(proofps_dd::MsgUserCmdFromClient));
    getConsole().OLn("  size of MsgUserUpdateFromServer: %u Bytes", sizeof(proofps_dd::MsgUserUpdateFromServer));
    getConsole().OLn("  size of MsgBulletUpdateFromServer: %u Bytes", sizeof(proofps_dd::MsgBulletUpdateFromServer));
    getConsole().OLn("  size of MsgWpnUpdateFromServer: %u Bytes", sizeof(proofps_dd::MsgWpnUpdateFromServer));
    getConsole().OLn("  size of MsgCurrentWpnUpdateFromServer: %u Bytes", sizeof(proofps_dd::MsgCurrentWpnUpdateFromServer));
    getConsole().OLn("  size of MsgMapItemUpdateFromServer: %u Bytes", sizeof(proofps_dd::MsgMapItemUpdateFromServer));
    getConsole().OLn("");

    LoadSound(m_sounds.m_sndLetsgo,         (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/locknload.wav").c_str());
    LoadSound(m_sounds.m_sndReloadStart,    (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/de_clipout.wav").c_str());
    LoadSound(m_sounds.m_sndReloadFinish,   (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/de_clipin.wav").c_str());
    LoadSound(m_sounds.m_sndShootPistol,    (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/deagle-1.wav").c_str());
    LoadSound(m_sounds.m_sndShootMchgun,    (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/m4a1_unsil-1.wav").c_str());
    LoadSound(m_sounds.m_sndShootBazooka,   (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/bazooka.wav").c_str());
    LoadSound(m_sounds.m_sndReloadBazooka,  (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/bazooka_reload.wav").c_str());
    LoadSound(m_sounds.m_sndShootDryPistol, (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/dryfire_pistol.wav").c_str());
    LoadSound(m_sounds.m_sndShootDryMchgun, (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/dryfire_rifle.wav").c_str());
    LoadSound(m_sounds.m_sndChangeWeapon,   (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/m4a1_deploy.wav").c_str());
    LoadSound(m_sounds.m_sndPlayerDie,      (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/die1.wav").c_str());
    LoadSound(m_sounds.m_sndExplosion,      (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/xplosion.wav").c_str());

    getConsole().OOOLn("PRooFPSddPGE::onGameInitialized() done!");

    getInput().getMouse().SetCursorPos(
        getPure().getWindow().getX() + getPure().getWindow().getWidth()/2,
        getPure().getWindow().getY() + getPure().getWindow().getHeight()/2);
    getPure().getWindow().SetCursorVisible(false);

    m_deathMatchMode->setFragLimit(10);
    //m_deathMatchMode->setTimeLimitSecs(500);
    m_gameMode->restart();
    
    m_fps_lastmeasure = GetTickCount();
    m_fps = GAME_MAXFPS;

    return true;
}

/**
    Game logic right before the engine would do anything.
    This is invoked at the very beginning of the main game loop, before processing window messages and incoming network packets.
*/
void proofps_dd::PRooFPSddPGE::onGameFrameBegin()
{
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        if (getNetwork().isServer())
        {
            if (player.getPos().getNew().getY() != player.getPos().getOld().getY())
            {   // still could fall in previous frame, so jumping is still disallowed ...
                player.SetJumpAllowed(false);
            }
            else
            {
                player.SetJumpAllowed(true);
            }
        }
    }
}

/**
    Game logic here.
    Game engine invokes this in every frame.
    DO NOT make any unnecessary operations here, as this function must always complete below 16 msecs to keep stable 60 fps!
*/
void proofps_dd::PRooFPSddPGE::onGameRunning()
{
    const std::chrono::time_point<std::chrono::steady_clock> timeOnGameRunningStart = std::chrono::steady_clock::now();

    if (m_durations.m_timeFullRoundtripStart.time_since_epoch().count() != 0)
    {
        m_durations.m_nFullRoundtripDurationUSecs +=
            std::chrono::duration_cast<std::chrono::microseconds>(timeOnGameRunningStart - m_durations.m_timeFullRoundtripStart).count();
    }
    m_durations.m_timeFullRoundtripStart = timeOnGameRunningStart;

    PureWindow& window = getPure().getWindow();

    m_durations.m_nFramesElapsedSinceLastDurationsReset++;

    // onGameRunning() is invoked by PGE in every frame no matter if we are in main menu or a game session.
    // If we are in main menu, then basically GUI::drawMainMenuCb() is operating, since PURE is calling it back in every frame.
    // In that case, there is not much to do here in onGameRunning().
    // We expect the GUI to set MenuState::None as soon as the user wants to enter a game (either by creating or joining).

    // if we want to handle Create or Join event initiated from Main menu here, we should maintain a prevMenuState also, then
    // the condition would look like this:
    // if ((m_gui.getMenuState() == proofps_dd::GUI::MenuState::None) && (m_gui.getPrevMenuState() != proofps_dd::GUI::MenuState::None))
    // This way we could decouple this logic from GUI code.

    if (m_gui.getMenuState() == proofps_dd::GUI::MenuState::None)
    {
        // having valid connection means that server accepted the connection and we have initialized our player;
        // otherwise m_mapPlayers[connHandle] is dangerous as it implicitly creates entry ...
        if (hasValidConnection())
        {
            if (getNetwork().isServer())
            {
                // TODO: very bad: physics stuff should not be set every frame, it should be done in config.validate(), however
                // in that case Config would need the Physics definition which overall leads to circular including each other,
                // leading to GameMode.cpp unable to compile.
                // 1 way of fixing this would be to implement the prevMenuState stuff explained a few lines above, so that
                // the main loop itself would be able to detect exiting from the menu and invoke stuff only once!
                serverSetAllowStrafeMidAir(getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR].getAsBool());
                serverSetAllowStrafeMidAirFull(getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].getAsBool());
                serverUpdateWeapons(*m_gameMode);
            }

            // 1 TICK START
            const auto DurationSimulationStepMicrosecsPerTick = std::chrono::microseconds((1000 * 1000) / m_config.getTickRate());
            const auto timeNow = std::chrono::steady_clock::now();
            if (m_timeSimulation.time_since_epoch().count() == 0)
            {
                m_timeSimulation = std::chrono::steady_clock::now() - DurationSimulationStepMicrosecsPerTick;
            }
            while (m_timeSimulation < timeNow)
            {
                // @TICKRATE
                m_timeSimulation += DurationSimulationStepMicrosecsPerTick;
                if (getNetwork().isServer())
                {
                    mainLoopConnectedServerOnlyOneTick(DurationSimulationStepMicrosecsPerTick.count());
                }
                else
                {
                    mainLoopConnectedClientOnlyOneTick(DurationSimulationStepMicrosecsPerTick.count());
                }
            }
            // 1 TICK END

            mainLoopConnectedShared(window);
        } // endif validConnection
        else
        {
            // try connecting back
            // we also need to wait for m_mapPlayers to become empty, it is important for server otherwise it will fail in handleUserConnected();
            // it might take a while to bring down all clients one-by-one and let m_mapPlayers be empty, so we also need to check that here!
            if (m_mapPlayers.empty() &&
                (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_timeConnectionStateChangeInitiated).count() >= m_config.getReconnectDelaySeconds()))
            {
                if (connect())
                {
                    showXHairInCenter();
                    // Config::validate() makes sure neither getTickRate() nor getClientUpdateRate() return 0
                    m_nSendClientUpdatesInEveryNthTick = m_config.getTickRate() / m_config.getClientUpdateRate();
                    m_nSendClientUpdatesCntr = m_nSendClientUpdatesInEveryNthTick;
                    m_timeSimulation = {};  // reset tick-based simulation time as well
                }
                else
                {
                    // try again a bit later
                    m_timeConnectionStateChangeInitiated = std::chrono::steady_clock::now();
                }
            }
            else
            {
                mainLoopDisconnectedShared(window);

                m_gui.textForNextFrame("Waiting for restoring connection (pending clients to be disconnected: " + std::to_string(m_mapPlayers.size()) + ") ...",
                    200,
                    getPure().getWindow().getClientHeight() / 2);
                if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_timeLastPrintWaitConnection).count() >= 1)
                {
                    m_timeLastPrintWaitConnection = std::chrono::steady_clock::now();
                    getConsole().EOLn("Waiting a bit before trying to connect back (pending clients to be disconnected: %u) ... ", m_mapPlayers.size());
                }
            }
        } // end else validConnection
    } // m_gui.getMenuState()

    updateFramesPerSecond(window);

    m_durations.m_nFullOnGameRunningDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeOnGameRunningStart).count();
}

/**
    Called when a new network packet is received.

    @return True on successful packet handling, false on serious error that should result in terminating the application.
*/
bool proofps_dd::PRooFPSddPGE::onPacketReceived(const pge_network::PgePacket& pkt)
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
    bool bRet;

    const pge_network::PgePktId& pgePktId = pge_network::PgePacket::getPacketId(pkt);
    switch (pgePktId)
    {
    case pge_network::MsgUserConnectedServerSelf::id:
        bRet = handleUserConnected(pge_network::PgePacket::getServerSideConnectionHandle(pkt), pge_network::PgePacket::getMessageAsUserConnected(pkt));
        break;
    case pge_network::MsgUserDisconnectedFromServer::id:
        bRet = handleUserDisconnected(pge_network::PgePacket::getServerSideConnectionHandle(pkt), pge_network::PgePacket::getMessageAsUserDisconnected(pkt));
        break;
    case pge_network::MsgApp::id:
    {
        // for now we support only 1 app msg per pkt
        assert(pge_network::PgePacket::getMessageAppCount(pkt) == 1);
        assert(pge_network::PgePacket::getMessageAppsTotalActualLengthBytes(pkt) > 0);  // for now we dont have empty messages
        
        // TODO: here we will need to iterate over all app msg but for now there is only 1 inside!

        const proofps_dd::PRooFPSappMsgId& proofpsAppMsgId = static_cast<proofps_dd::PRooFPSappMsgId>(pge_network::PgePacket::getMsgAppIdFromPkt(pkt));
        switch (proofpsAppMsgId)
        {
        case proofps_dd::MsgMapChangeFromServer::id:
            bRet = handleMapChangeFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgMapChangeFromServer>(pkt));
            break;
        case proofps_dd::MsgUserSetupFromServer::id:
            bRet = handleUserSetupFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserSetupFromServer>(pkt));
            break;
        case proofps_dd::MsgUserNameChange::id:
            bRet = handleUserNameChange(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserNameChange>(pkt));
            break;
        case proofps_dd::MsgUserCmdFromClient::id:
            bRet = handleUserCmdMoveFromClient(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt));
            break;
        case proofps_dd::MsgUserUpdateFromServer::id:
            bRet = handleUserUpdateFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserUpdateFromServer>(pkt),
                *m_pObjXHair,
                *m_gameMode);
            break;
        case proofps_dd::MsgBulletUpdateFromServer::id:
            bRet = handleBulletUpdateFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgBulletUpdateFromServer>(pkt),
                m_vecCamShakeForce);
            break;
        case proofps_dd::MsgMapItemUpdateFromServer::id:
            bRet = handleMapItemUpdateFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgMapItemUpdateFromServer>(pkt));
            break;
        case proofps_dd::MsgWpnUpdateFromServer::id:
            bRet = handleWpnUpdateFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgWpnUpdateFromServer>(pkt));
            break;
        case proofps_dd::MsgCurrentWpnUpdateFromServer::id:
            bRet = handleWpnUpdateCurrentFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgCurrentWpnUpdateFromServer>(pkt));
            break;
        case proofps_dd::MsgDeathNotificationFromServer::id:
            bRet = handleDeathNotificationFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgDeathNotificationFromServer>(pkt));
            break;
        default:
            bRet = false;
            getConsole().EOLn("CustomPGE::%s(): unknown msgId %u in MsgApp!", __func__, proofpsAppMsgId);
        }
        break;
    }
    default:
        bRet = false;
        getConsole().EOLn("CustomPGE::%s(): unknown pktId %u!", __func__, pgePktId);
    }

    m_durations.m_nFullOnPacketReceivedDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
    return bRet;
}

/**
    Freeing up game content here.
    Free up everything that has been allocated in onGameInitialized() and onGameRunning().
*/
void proofps_dd::PRooFPSddPGE::onGameDestroying()
{
    getConsole().OLnOI("proofps_dd::PRooFPSddPGE::onGameDestroying() ...");

    m_gui.textForNextFrame("Exiting game ...", 200, getPure().getWindow().getClientHeight() / 2);
    getPure().getRenderer()->RenderScene();

    //getConsole().SetLoggingState("4LLM0DUL3S", true);
    //getPure().WriteList();
    //getConsole().SetLoggingState("4LLM0DUL3S", false);

    // TODO: check common parts with disconnect()
    m_mapPlayers.clear();       // Dtors of Player instances will be implicitly called
    deleteWeaponHandlingAll();  // Dtors of Bullet instances will be implicitly called
    m_maps.shutdown();
    m_gui.shutdown();
    delete m_pObjXHair;
    m_pObjXHair = nullptr;
    delete m_gameMode;
    m_gameMode = nullptr;
    getPure().getObject3DManager().DeleteAll();
    getPure().getWindow().SetCursorVisible(true);

    getConsole().OOOLn("proofps_dd::PRooFPSddPGE::onGameDestroying() done!");
    getConsole().Deinitialize();
}


// ############################### PRIVATE ###############################


void proofps_dd::PRooFPSddPGE::showLoadingScreen(int nProgress)
{
    m_gui.showLoadingScreen(nProgress, m_maps.getNextMapToBeLoaded());
}

void proofps_dd::PRooFPSddPGE::hideLoadingScreen()
{
    m_gui.hideLoadingScreen();
    m_maps.UpdateVisibilitiesForRenderer();
}

void proofps_dd::PRooFPSddPGE::showXHairInCenter()
{
    if (!m_pObjXHair)
    {
        return;
    }

    // this is to get rid of all mouse move messages that were probably queued up in the meantime (e.g. during map loading), otherwise
    // there is no use of setting cursor pos to center if enqueued messages will reposition it when PURE runs the window's processMessages().
    getPure().getWindow().ProcessMessages();

    // getInput().getMouse().SetCursorPos() is not triggering any mouse move event and nulls out pending raw input events as well!
    getInput().getMouse().SetCursorPos(
        getPure().getWindow().getX() + getPure().getWindow().getWidth() / 2,
        getPure().getWindow().getY() + getPure().getWindow().getHeight() / 2);
    
    m_pObjXHair->getPosVec().Set(0, 0, 0); // reposition to viewport center so it won't appear at random places
    m_pObjXHair->Show();
}

bool proofps_dd::PRooFPSddPGE::hasValidConnection() const
{
    // This is why it is very important to clear out m_nServerSideConnectionHandle when we are disconnecting:
    // to make sure this conditiona fails, so the main loop is aware of not having a valid connection.
    // And it is also very important to clear m_mapPlayers in that case, otherwise 2 bugs would occur:
    // - server: it would still be able to find itself in the map, obviously since 0 is its own handle anyway;
    // - client: it would still be able to find the server in the map, which is a false conclusion of having a valid connection.
    // We must also wait for a non-empty player name because it means that all 3 must-have messages were processed properly:
    // MsgUserConnected, MsgUserSetup, MsgUserNameChange.
    // A properly set unique name is important for gamemode. And handleUserUpdateFromServer() would also update gamemode by valid user name.
    const auto itPlayer = m_mapPlayers.find(m_nServerSideConnectionHandle);
    return (itPlayer != m_mapPlayers.end()) && (!itPlayer->second.getName().empty());
}

bool proofps_dd::PRooFPSddPGE::connect()
{
    initializeWeaponHandling();
    bool bRet;
    if (getNetwork().isServer())
    {
        m_gui.textForNextFrame("Starting Server ...", 200, getPure().getWindow().getClientHeight() / 2);
        getPure().getRenderer()->RenderScene();

        bRet = getNetwork().getServer().startListening();
        if (!bRet)
        {
            getConsole().EOLn("ERROR: Server has FAILED to start listening!");
        }
    }
    else
    {
        std::string sIp = "127.0.0.1";
        if (!getConfigProfiles().getVars()[CVAR_CL_SERVER_IP].getAsString().empty())
        {
            sIp = getConfigProfiles().getVars()[CVAR_CL_SERVER_IP].getAsString();
            getConsole().OLn("IP from config: %s", sIp.c_str());
        }

        m_gui.textForNextFrame("Connecting to Server @ " + sIp + " ...", 200, getPure().getWindow().getClientHeight() / 2);
        getPure().getRenderer()->RenderScene();

        bRet = getNetwork().getClient().connectToServer(sIp);
        if (!bRet)
        {
            getConsole().EOLn("ERROR: Client has FAILED to establish connection to the server!");
        }
    }
    return bRet;
}

void proofps_dd::PRooFPSddPGE::disconnect(bool bExitFromGameSession, const std::string& sExtraDebugText)
{
    getPure().getUImanager().removeAllTextPermanentLegacy(); // cannot find better way to get rid of permanent texts
    const std::string sPrintText =
        sExtraDebugText.empty() ?
        "Unloading resources ..." :
        "Unloading resources ... Reason: " + sExtraDebugText;
    m_gui.textForNextFrame(sPrintText, 200, getPure().getWindow().getClientHeight() / 2);
    getPure().getRenderer()->RenderScene();

    getConsole().SetLoggingState("4LLM0DUL3S", true);
    getNetwork().disconnect(sExtraDebugText);
    m_nServerSideConnectionHandle = pge_network::ServerConnHandle; // default it back
    getConsole().SetLoggingState("4LLM0DUL3S", false);
    
    // As server, dont need to remove players because we already disconnected above, this will cause all connection states to transition to
    // disconnected while handling state changes in engine main loop (getNetwork.Update()), invoking our handleUserDisconnected() as well.
    // The only user who will stay in m_mapPlayers is the server player but for that server is injecting a userDisconnected message so it
    // will be also removed from the map, leading to an empty map as expected.
    // As client, there will be a userDisconnected message for the server, for which we should delete all players.
    // So eventually all players will be also removed not only from m_mapPlayers but also from m_gameMode.
    // We need m_mapPlayers to be cleared out by the end of processing all disconnections, the reason is explained in hasValidConnection().

    // we should hide all the players because actual deleting them will happen later once
    // game is processing each player disconnect, however they should not be visible from now as they would be visible
    // during map loading.
    m_pObjXHair->Hide();
    for (auto& connHandlePlayerPair : m_mapPlayers)
    {
        connHandlePlayerPair.second.getObject3D()->Hide();
        if (connHandlePlayerPair.second.getWeaponManager().getCurrentWeapon())
        {
            connHandlePlayerPair.second.getWeaponManager().getCurrentWeapon()->getObject3D().Hide();
        }
    }

    deleteWeaponHandlingAll();  // Dtors of Bullet instances will be implicitly called
    m_maps.unload();

    if (bExitFromGameSession)
    {
        m_gui.resetMenuState(bExitFromGameSession);
    }
}

/**
    Only server executes this.
    Good for either dedicated- or listen- server.
*/
void proofps_dd::PRooFPSddPGE::mainLoopConnectedServerOnlyOneTick(
    const long long& /*durElapsedMicrosecs*/)
{
    /*
    * This function is executed every tick.
    * If executed rarely i.e. with very low tickrate e.g. 1 tick/sec, players and bullets might "jump" over walls.
    * To solve this, we might run multiple physics iterations (nPhysicsIterationsPerTick) so movements are
    * calculated in smaller steps, resulting in more precise results.
    * However, it is highly recommended to keep tickrate high, because even though input is sampled at framerate, the
    * sampled input for player movement is evaluated per tick, which with a low tickrate and high physics rate combo
    * leads to less precise player movement. Considering a quick player able to hold key for strafing for only 18 ms,
    * we need at least ~55 Hz fine-grade tickrate to make sure both the keypress and key up events are evaluated
    * precisely by the ticks and not treated as longer strafe by longer ticks caused by lower tickrate.
    * I think the ideal is 60 Hz tickrate with 60 Hz minPhysicsRate, where the latter could be changed to higher only
    * if it is really required.
    */
    const unsigned int nPhysicsIterationsPerTick = std::max(1u, m_config.getPhysicsRate() / m_config.getTickRate());
    if (!m_gameMode->checkWinningConditions())
    {
        for (unsigned int iPhyIter = 1; iPhyIter <= nPhysicsIterationsPerTick; iPhyIter++)
        {
            const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
            serverGravity(*m_pObjXHair, m_config.getPhysicsRate());
            serverPlayerCollisionWithWalls(m_bWon, m_config.getPhysicsRate());
            m_durations.m_nGravityCollisionDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
            serverUpdateBullets(*m_gameMode, *m_pObjXHair, m_config.getPhysicsRate(), m_vecCamShakeForce);
            serverUpdateExplosions(*m_gameMode, m_config.getPhysicsRate());
            serverPickupAndRespawnItems();
            updatePlayersOldValues();
        }  // for iPhyIter
    }  // checkWinningConditions()
    serverUpdateRespawnTimers();
    serverSendUserUpdates();
}

/**
    Only client executes this.
*/
void proofps_dd::PRooFPSddPGE::mainLoopConnectedClientOnlyOneTick(
    const long long& /*durElapsedMicrosecs*/)
{
    /*
    * This function is executed every tick.
    * Since this is executed by client, we dont care about physics-related concerns explained in comments in mainLoopConnectedServerOnlyOneTick(). 
    */
    const unsigned int nPhysicsIterationsPerTick = std::max(1u, m_config.getPhysicsRate() / m_config.getTickRate());
    for (unsigned int iPhyIter = 1; iPhyIter <= nPhysicsIterationsPerTick; iPhyIter++)
    {
        clientUpdateBullets(m_config.getPhysicsRate());
        clientUpdateExplosions(*m_gameMode, m_config.getPhysicsRate());
    }
}

/**
    Both clients and listen-server executes this.
    Dedicated server won't need this.
*/
void proofps_dd::PRooFPSddPGE::mainLoopConnectedShared(PureWindow& window)
{
    std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
    Player& player = m_mapPlayers.at(m_nServerSideConnectionHandle); // cannot throw, because of bValidConnection
    if (window.isActive())
    {
        if (handleInputWhenConnectedAndSendUserCmdMove(*m_gameMode, m_bWon, player, *m_pObjXHair, m_config.getTickRate(), m_config.getClientUpdateRate(), m_config.getPhysicsRate()) ==
            proofps_dd::InputHandling::PlayerAppActionRequest::Exit)
        {
            disconnect(true);
            return;
        }
    } // window is active
    m_durations.m_nActiveWindowStuffDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

    CameraMovement(player, m_config.getCameraFollowsPlayerAndXHair(), m_config.getCameraTilting());
    UpdateGameMode();  // TODO: on the long run this should be also executed only by server, now for fraglimit every instance executes ...
    m_maps.Update(m_fps);
    m_maps.UpdateVisibilitiesForRenderer();
    if (player.getWeaponManager().getCurrentWeapon())
    {
        // very bad: m_gui.textPermanent() should be used, but then removeTextPermanentLegacy() would be also needed anytime there is a change ...
        m_gui.textForNextFrame(
            player.getWeaponManager().getCurrentWeapon()->getVars()["name"].getAsString() + ": " +
            std::to_string(player.getWeaponManager().getCurrentWeapon()->getMagBulletCount()) + " / " +
            std::to_string(player.getWeaponManager().getCurrentWeapon()->getUnmagBulletCount()),
            10, 150);
    }
}

/**
    Both clients and listen-server executes this.
    Dedicated server won't need this.
*/
void proofps_dd::PRooFPSddPGE::mainLoopDisconnectedShared(PureWindow& window)
{
    if (window.isActive())
    {
        if (handleInputWhenDisconnected() == proofps_dd::InputHandling::PlayerAppActionRequest::Exit)
        {
            disconnect(true);
            return;
        }
    } // window is active
}

void proofps_dd::PRooFPSddPGE::updateFramesPerSecond(PureWindow& window)
{
    // this is horrible that FPS measuring is still not available from outside of Pure .........
    std::stringstream ssFps;
    ssFps << std::fixed << std::setprecision(1) << m_fps;
    m_fps_counter++;
    const DWORD nGetTickCount = GetTickCount();  // TODO: switch to chrono ...
    if (nGetTickCount - GAME_FPS_MEASURE_INTERVAL >= m_fps_lastmeasure)
    {
        if (!m_bFpsFirstMeasure)
        {
            // too much time might elapse between onGameInitialized() and onGameRunning() so we don't update fps
            // when onGameRunning() is invoked for the first time
            m_fps = m_fps_counter * (1000.f / (nGetTickCount - m_fps_lastmeasure));
        }
        else
        {
            m_bFpsFirstMeasure = false;
        }

        m_fps_counter = 0;
        m_fps_lastmeasure = nGetTickCount;

        std::stringstream str;
        if (getNetwork().isServer())
        {
            str << proofps_dd::GAME_NAME << " " << proofps_dd::GAME_VERSION <<
                " Server :: Tickrate : " << m_config.getTickRate() <<
                " Hz :: MinPhyRate : " << m_config.getPhysicsRate() <<
                " Hz :: ClUpdRate : " << m_config.getClientUpdateRate() <<
                " Hz :: FPS : " << ssFps.str();
        }
        else
        {
            str << proofps_dd::GAME_NAME << " " << proofps_dd::GAME_VERSION <<
                " Client :: Tickrate : " << m_config.getTickRate() <<
                " Hz :: FPS : " << ssFps.str();
        }
        window.SetCaption(str.str());

        if (m_fps < 0.01f)
        {
            m_fps = 0.01f; // make sure nobody tries division by zero
        }
    }
    m_gui.textForNextFrame(ssFps.str(), window.getClientWidth() - 50, window.getClientHeight() - 2 * getPure().getUImanager().getDefaultFontSizeLegacy());
}

void proofps_dd::PRooFPSddPGE::LoadSound(SoLoud::Wav& snd, const char* fname)
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

void proofps_dd::PRooFPSddPGE::CameraMovement(
    const Player& player,
    bool bCamFollowsXHair,
    bool bCamTilting)
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    constexpr unsigned int nBlocksToKeepCameraWithinMapBoundsHorizontally = 3;
    constexpr unsigned int nBlocksToKeepCameraWithinMapBottom = 5;
    constexpr unsigned int nBlocksToKeepCameraWithinMapTop = 1;
    
    const float fCamMinAllowedPosX =
        m_maps.width() < (nBlocksToKeepCameraWithinMapBoundsHorizontally*2 + 1) ?
        m_maps.getBlocksVertexPosMin().getX() :
        m_maps.getBlocksVertexPosMin().getX() + (proofps_dd::GAME_BLOCK_SIZE_X * (nBlocksToKeepCameraWithinMapBoundsHorizontally));
    const float fCamMaxAllowedPosX =
        m_maps.width() < (nBlocksToKeepCameraWithinMapBoundsHorizontally*2 + 1) ?
        m_maps.getBlocksVertexPosMax().getX() :
        m_maps.getBlocksVertexPosMin().getX() + (proofps_dd::GAME_BLOCK_SIZE_X * (m_maps.width() - nBlocksToKeepCameraWithinMapBoundsHorizontally));
    
    const float fCamMinAllowedPosY =
        m_maps.height() < (nBlocksToKeepCameraWithinMapBottom + 1) ?
        m_maps.getBlocksVertexPosMin().getY() :
        m_maps.getBlocksVertexPosMin().getY() + (proofps_dd::GAME_BLOCK_SIZE_Y * (nBlocksToKeepCameraWithinMapBottom - 1));
    const float fCamMaxAllowedPosY =
        m_maps.height() < (nBlocksToKeepCameraWithinMapTop + 1) ?
        m_maps.getBlocksVertexPosMax().getY() :
        m_maps.getBlocksVertexPosMin().getY() + (proofps_dd::GAME_BLOCK_SIZE_Y * (m_maps.height() - nBlocksToKeepCameraWithinMapTop + 1));

    auto& camera = getPure().getCamera();
    float fCamPosXTarget, fCamPosYTarget;
    if (bCamFollowsXHair)
    {
        /* Unsure about if we really need to unproject the 2D xhair's position.
           Because of matrix multiplication and inversion, maybe this way is too expensive.
           
           I see 2 other ways of doing this:
            - keep the xhair 2D, and anytime mouse is moved, we should also change 2 variables: fCamPosOffsetX and fCamPosOffsetY
              based on the dx and dy variables in InputHandling::mouse(). Camera will have this position offset applied relative to
              player's position. This looks to be the easiest solution.
              However, on the long run we will need 3D position of xhair since we want to use pick/select method, for example,
              when hovering the xhair over other player, we should be able to tell which player is that.
              
            - change the xhair to 3D, so in InputHandling::mouse() the dx and dy variable changes will be applied to xhair's 3D position.
              With this method, pick/select can be implemented in a different way: no need to unproject, we just need collision
              logic to find out which player object collides with our xhair object!
        */
        PureVector vecUnprojected;
        if (!camera.project2dTo3d(
            static_cast<TPureUInt>(roundf(m_pObjXHair->getPosVec().getX()) + camera.getViewport().size.width/2),
            static_cast<TPureUInt>(roundf(m_pObjXHair->getPosVec().getY()) + camera.getViewport().size.height/2),
            /* in v0.1.5 this is player's Z mapped to depth buffer: 0.9747f,*/
            0.96f,
            vecUnprojected))
        {
            //getConsole().EOLn("PRooFPSddPGE::%s(): project2dTo3d() failed!", __func__);
        }
        else
        {
            //getConsole().EOLn("obj X: %f, Y: %f, vecUnprojected X: %f, Y: %f",
            //    player.getObject3D()->getPosVec().getX(), player.getObject3D()->getPosVec().getY(),
            //    vecUnprojected.getX(), vecUnprojected.getY());
        }

        fCamPosXTarget = std::min(
            fCamMaxAllowedPosX,
            std::max(fCamMinAllowedPosX, (player.getObject3D()->getPosVec().getX() + vecUnprojected.getX())/2));
        
        fCamPosYTarget = std::min(
            fCamMaxAllowedPosY,
            std::max(fCamMinAllowedPosY, (player.getObject3D()->getPosVec().getY() + vecUnprojected.getY())/2));
    }
    else
    {
        fCamPosXTarget = std::min(
            fCamMaxAllowedPosX,
            std::max(fCamMinAllowedPosX, player.getObject3D()->getPosVec().getX()));

        fCamPosYTarget = std::min(
            fCamMaxAllowedPosY,
            std::max(fCamMinAllowedPosY, player.getObject3D()->getPosVec().getY()));
    }

    static float fShakeDegree = 0.f;
    assert(m_fps > 0.f);  // updateFramesPerSecond() makes sure m_fps is never 0
    fShakeDegree += 1200 / m_fps;
    while (fShakeDegree >= 360.f)
    {
        fShakeDegree -= 360.f;
    }
    float fShakeSine = sin(fShakeDegree * PFL::PI / 180.f);

    const float GAME_FPS_RATE_LERP_FACTOR = (m_fps - GAME_TICKRATE_MIN) / static_cast<float>(GAME_TICKRATE_MAX - GAME_TICKRATE_MIN);
    const float GAME_IMPACT_FORCE_CHANGE = PFL::lerp(2150.f, 2160.f, GAME_FPS_RATE_LERP_FACTOR);
    assert(m_fps > 0.f);  // updateFramesPerSecond() makes sure m_fps is never 0
    const float fCamShakeForceChangePerFrame = GAME_IMPACT_FORCE_CHANGE / 36.f / m_fps; /* smaller number means longer shaking in time */
    if (m_vecCamShakeForce.getX() > 0.f)
    {
        m_vecCamShakeForce.SetX(m_vecCamShakeForce.getX() - fCamShakeForceChangePerFrame);
        if (m_vecCamShakeForce.getX() < 0.f)
        {
            m_vecCamShakeForce.SetX(0.f);
        }
    }
    if (m_vecCamShakeForce.getY() > 0.f)
    {
        m_vecCamShakeForce.SetY(m_vecCamShakeForce.getY() - fCamShakeForceChangePerFrame);
        if (m_vecCamShakeForce.getY() < 0.f)
        {
            m_vecCamShakeForce.SetY(0.f);
        }
    }

    float fShakeFactorX = fShakeSine * m_vecCamShakeForce.getX() / m_fps;
    float fShakeFactorY = fShakeSine * m_vecCamShakeForce.getY() / m_fps;

    fCamPosXTarget += fShakeFactorX;
    fCamPosYTarget += fShakeFactorY;

    PureVector vecCamPos{
        PFL::smooth(
            camera.getPosVec().getX() + fShakeFactorX, fCamPosXTarget, GAME_CAM_SPEED_X * m_fps),
        PFL::smooth(
            camera.getPosVec().getY() + fShakeFactorY,
            fCamPosYTarget,
            /* if we are not following xhair, we want an eased vertical camera movement because it looks nice */
            (bCamFollowsXHair ? (GAME_CAM_SPEED_X * m_fps) : (GAME_CAM_SPEED_Y * m_fps))),
        GAME_CAM_Z
    };

    camera.getPosVec() = vecCamPos;
    camera.getTargetVec().Set(
        bCamTilting ? ((vecCamPos.getX() + fCamPosXTarget) / 2.f) : vecCamPos.getX(),
        bCamTilting ? ((vecCamPos.getY() + fCamPosYTarget) / 2.f) : vecCamPos.getY(),
        player.getObject3D()->getPosVec().getZ()
    );

    m_durations.m_nCameraMovementDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

} // CameraMovement()

void proofps_dd::PRooFPSddPGE::serverSendUserUpdates()
{
    if (!getNetwork().isServer())
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

        if (bSendUserUpdates && player.isNetDirty())
        {
            pge_network::PgePacket newPktUserUpdate;
            //getConsole().EOLn("PRooFPSddPGE::%s(): send 1!", __func__);
            if (proofps_dd::MsgUserUpdateFromServer::initPkt(
                newPktUserUpdate,
                playerPair.second.getServerSideConnectionHandle(),
                player.getPos().getNew().getX(),
                player.getPos().getNew().getY(),
                player.getPos().getNew().getZ(),
                player.getAngleY(),
                player.getWeaponAngle().getNew().getZ(),
                player.getCrouchStateCurrent(),
                player.getHealth(),
                player.getRespawnFlag(),
                player.getFrags(),
                player.getDeaths()))
            {
                player.clearNetDirty();

                // we always reset respawn flag here
                playerPair.second.getRespawnFlag() = false;

                // Note that health is not needed by server since it already has the updated health, but for convenience
                // we put that into MsgUserUpdateFromServer and send anyway like all the other stuff.
                getNetwork().getServer().sendToAll(newPktUserUpdate);
                //getConsole().EOLn("PRooFPSddPGE::%s(): send 2!", __func__);
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
        m_durations.m_nSendUserUpdatesDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
    }

    ++m_nSendClientUpdatesCntr;
}

void proofps_dd::PRooFPSddPGE::RestartGame()
{
    if (getNetwork().isServer())
    {
        for (auto& playerPair : m_mapPlayers)
        {
            ServerRespawnPlayer(playerPair.second, true);
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

            if (proofps_dd::MsgMapItemUpdateFromServer::initPkt(
                newPktMapItemUpdate,
                0,
                mapItem.getId(),
                mapItem.isTaken()))
            {
                getNetwork().getServer().sendToAllClientsExcept(newPktMapItemUpdate);
            }
            else
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
            }
        } // end for items
    } // end server

    m_gameMode->restartWithoutRemovingPlayers(); // now both server and clients execute this on their own, in future only server should do this ...
}

void proofps_dd::PRooFPSddPGE::serverUpdateRespawnTimers()
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
        if (timeDiffSeconds >= proofps_dd::GAME_PLAYER_RESPAWN_SECONDS)
        {
            ServerRespawnPlayer(playerPair.second, false);
        }
    }

    m_durations.m_nUpdateRespawnTimersDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void proofps_dd::PRooFPSddPGE::UpdateGameMode()
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    if (m_gameMode->checkWinningConditions())
    {
        const auto nSecsSinceWin = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_gameMode->getWinTime()).count();
        if (nSecsSinceWin >= 15)
        {
            RestartGame();
        }
        else
        {
            // these are being executed frame by frame during waiting for game restart, however these are cheap operations so I dont care ...
            m_gameMode->showObjectives(getPure(), getNetwork());
            m_pObjXHair->Hide();
            for (auto& playerPair : m_mapPlayers)
            {
                playerPair.second.getObject3D()->Hide();
                playerPair.second.getWeaponManager().getCurrentWeapon()->getObject3D().Hide();
            }
        }
    }

    m_durations.m_nUpdateGameModeDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void proofps_dd::PRooFPSddPGE::serverPickupAndRespawnItems()
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
                // To avoid Z-fighting, players and items have different Z-coords. This makes Colliding() unable to work, so we use Colliding_NoZ().
                // And it is faster too, no use of Z anyway since theoretically players and items have the same Z.
                if (Colliding_NoZ(*plobj, mapItem.getObject3D()))
                {
                    proofps_dd::MsgWpnUpdateFromServer::getAvailable(newPktWpnUpdate) = false;
                    if (player.canTakeItem(mapItem))
                    {
                        player.TakeItem(mapItem, newPktWpnUpdate);  // this also invokes mapItem.Take()
                        bSendItemUpdate = true;
                        // although item update is always sent, wpn update is sent only if TakeItem() flipped the availability of the wpn,
                        // since it can happen the item is not weapon-related at all, or something else, anyway let TakeItem() make the decision!
                        if (proofps_dd::MsgWpnUpdateFromServer::getAvailable(newPktWpnUpdate))
                        {
                            if (playerPair.second.getServerSideConnectionHandle() != pge_network::ServerConnHandle) // server doesnt send this to itself
                            {
                                getNetwork().getServer().send(newPktWpnUpdate, playerPair.second.getServerSideConnectionHandle());
                            }
                        }
                        break; // a player can collide with only one item at a time since there are no overlapping items
                    }
                } // colliding
            } // for playerPair
        }

        if (bSendItemUpdate)
        {
            if (proofps_dd::MsgMapItemUpdateFromServer::initPkt(
                newPktMapItemUpdate,
                pge_network::ServerConnHandle,
                mapItem.getId(),
                mapItem.isTaken()))
            {
                getNetwork().getServer().sendToAllClientsExcept(newPktMapItemUpdate);
            }
            else
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
            }
        }
    } // for item

    m_durations.m_nPickupAndRespawnItemsDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}  // serverPickupAndRespawnItems()

void proofps_dd::PRooFPSddPGE::WritePlayerList()
{
    getConsole().OLnOI("PRooFPSddPGE::%s()", __func__);
    for (const auto& playerPair : m_mapPlayers)
    {
        getConsole().OLn("Username: %s; connHandleServerSide: %u; address: %s",
            playerPair.second.getName().c_str(), playerPair.second.getServerSideConnectionHandle(), playerPair.second.getIpAddress().c_str());
    }
    getConsole().OO();
}

bool proofps_dd::PRooFPSddPGE::handleUserSetupFromServer(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserSetupFromServer& msg)
{
    // TODO: make sure received user name is properly null-terminated! someone else could had sent that, e.g. malicious server
    // TODO: make sure received map name is properly null-terminated! someone else could had sent that, e.g. malicious server
    // TODO: make sure received IP address is properly null-terminated! someone else could had sent that, e.g. malicious server

    // TEMPORARY COMMENTED DUE TO: https://github.com/proof88/PRooFPS-dd/issues/268
    //if (m_mapPlayers.end() != m_mapPlayers.find(connHandleServerSide))
    //{
    //    getConsole().EOLn("PRooFPSddPGE::%s(): cannot happen: connHandleServerSide: %u is already present in players list!",
    //        __func__, connHandleServerSide);
    //    assert(false);
    //    return false;
    //}

    // TEMPORAL WORKAROUND DUE TO: https://github.com/proof88/PRooFPS-dd/issues/268
    if (m_mapPlayers.end() != m_mapPlayers.find(connHandleServerSide))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): connHandleServerSide: %u is already present in players list, allowed temporarily!",
            __func__, connHandleServerSide);
        return true;
    }

    if (msg.m_bCurrentClient)
    {
        getConsole().OLn("PRooFPSddPGE::%s(): this is me, connHandleServerSide: %u, my IP: %s, map: %s",
            __func__, connHandleServerSide, msg.m_szIpAddress, msg.m_szMapFilename);
        m_nServerSideConnectionHandle = connHandleServerSide;

        if (getNetwork().isServer())
        {
            // if we are server and current client is us, then our m_nServerSideConnectionHandle should be pge_network::ServerConnHandle
            if (m_nServerSideConnectionHandle != pge_network::ServerConnHandle)
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): cannot happen: m_nServerSideConnectionHandle != pge_network::ServerConnHandle: %u != %u, programming error!",
                    __func__, m_nServerSideConnectionHandle, pge_network::ServerConnHandle);
                assert(false);
                return false;
            }

            // now we should ask ourselves to set our wanted name
            pge_network::PgePacket newPktUserNameChange;
            if (!proofps_dd::MsgUserNameChange::initPkt(
                newPktUserNameChange,
                pge_network::ServerConnHandle,
                true,
                getConfigProfiles().getVars()[CVAR_CL_NAME].getAsString()))
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
                return false;
            }
            getNetwork().getServer().send(newPktUserNameChange);

            // Server receives map name also in MsgUserSetupFromServer.
            // However, the server MUST have the correct map loaded already at this point:
            //  - if this is a bootup, it loaded already in handleUserConnected();
            //  - if this is a map change, it loaded already in handleMapChangeFromServer().
            // So if file name is mismatching then there must be a huge logic error somewhere and we should terminate now.
            if (m_maps.getFilename() != msg.m_szMapFilename)
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): unexpected map name received by self: %s, loaded map: %s!", __func__, msg.m_szMapFilename, m_maps.getFilename().c_str());
                assert(false);
                return false;
            }
        }
        else
        {
            // now we should ask the server to set our wanted name
            pge_network::PgePacket newPktUserNameChange;
            if (!proofps_dd::MsgUserNameChange::initPkt(
                newPktUserNameChange,
                connHandleServerSide,
                false,
                getConfigProfiles().getVars()[CVAR_CL_NAME].getAsString()))
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
                return false;
            }
            getNetwork().getClient().send(newPktUserNameChange);

            // Client receives map name also in MsgUserSetupFromServer.
            // If this is a bootup, then we need to load map here, only if it is different than what we have already loaded (map change case).
            // Because we also get here in case of reconnecting after a map change, we should load the map only if it is different than server just asked for.
            if (m_maps.getFilename() != msg.m_szMapFilename)
            {
                // if we fall here with non-empty m_maps.getFilename(), it is an error, and m_maps.load() will fail as expected.
                if (!m_maps.load(msg.m_szMapFilename, m_cbDisplayMapLoadingProgressUpdate))
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): m_maps.load() failed: %s!", __func__, msg.m_szMapFilename);
                    assert(false);
                    return false;
                }
            }
            else
            {
                getConsole().OLn("PRooFPSddPGE::%s(): map %s already loaded", __func__, msg.m_szMapFilename);
            }
        }

        // at this point we can be sure we have the proper map loaded, camera must start from the center of the map
        getPure().getCamera().getPosVec().Set(
            (m_maps.getBlockPosMin().getX() + m_maps.getBlockPosMax().getX()) / 2.f,
            (m_maps.getBlockPosMin().getY() + m_maps.getBlockPosMax().getY()) / 2.f,
            GAME_CAM_Z);
        getPure().getCamera().getTargetVec().Set(
            getPure().getCamera().getPosVec().getX(),
            getPure().getCamera().getPosVec().getY(),
            -proofps_dd::GAME_BLOCK_SIZE_Z);

        hideLoadingScreen();
        showXHairInCenter();
        
        getAudio().play(m_sounds.m_sndLetsgo);
    }
    else
    {
        getConsole().OLn("PRooFPSddPGE::%s(): new user (connHandleServerSide: %u; IP: %s) connected",
            __func__, connHandleServerSide, msg.m_szIpAddress);

        if (getNetwork().isServer())
        {
            // we send as many MsgUserSetupFromServer pkts to the client as the number of already connected players,
            // otherwise client won't know about them, so this way the client will detect them as newly connected users;
            // we also send MsgUserUpdateFromServer about each player so new client will immediately have their positions and other data updated,
            // and MsgCurrentWpnUpdateFromServer so their current weapon is also correctly display instead of the default wpn.
            
            for (const auto& it : m_mapPlayers)
            {
                pge_network::PgePacket newPktSetup;
                if (!proofps_dd::MsgUserSetupFromServer::initPkt(
                    newPktSetup,
                    it.second.getServerSideConnectionHandle(),
                    false,
                    it.second.getIpAddress(),
                    "" /* here mapFilename is irrelevant */))
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                    assert(false);
                    continue;
                }
                getNetwork().getServer().send(newPktSetup, connHandleServerSide);

                if (!it.second.getName().empty())
                {
                    // send out only confirmed non-empty names, as handleUserNameChange() expects confirmed, unique, non-empty names!
                    // Once this client has a confirmed name, server will send out this info to all clients anyway!
                    pge_network::PgePacket newPktUserNameChange;
                    if (!proofps_dd::MsgUserNameChange::initPkt(
                        newPktUserNameChange,
                        it.second.getServerSideConnectionHandle(),
                        false,
                        it.second.getName()))
                    {
                        getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                        assert(false);
                        return false;
                    }
                    getNetwork().getServer().send(newPktUserNameChange, connHandleServerSide);
                }

                pge_network::PgePacket newPktUserUpdate;
                if (!proofps_dd::MsgUserUpdateFromServer::initPkt(
                    newPktUserUpdate,
                    it.second.getServerSideConnectionHandle(),
                    it.second.getObject3D()->getPosVec().getX(),
                    it.second.getObject3D()->getPosVec().getY(),
                    it.second.getObject3D()->getPosVec().getZ(),
                    it.second.getObject3D()->getAngleVec().getY(),
                    it.second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().getZ(),
                    false,
                    it.second.getHealth(),
                    false,
                    it.second.getFrags(),
                    it.second.getDeaths()))
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                    assert(false);
                    continue;
                }
                getNetwork().getServer().send(newPktUserUpdate, connHandleServerSide);

                pge_network::PgePacket pktWpnUpdateCurrent;
                if (!proofps_dd::MsgCurrentWpnUpdateFromServer::initPkt(
                    pktWpnUpdateCurrent,
                    it.second.getServerSideConnectionHandle(),
                    it.second.getWeaponManager().getCurrentWeapon()->getFilename()))
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                    assert(false);
                    continue;
                }
                getNetwork().getServer().send(pktWpnUpdateCurrent, connHandleServerSide);
            }

            // we also send the state of all map items
            pge_network::PgePacket newPktMapItemUpdate;
            for (auto& itemPair : m_maps.getItems())
            {
                if (!itemPair.second)
                {
                    continue;
                }

                if (proofps_dd::MsgMapItemUpdateFromServer::initPkt(
                    newPktMapItemUpdate,
                    pge_network::ServerConnHandle,
                    itemPair.first,
                    itemPair.second->isTaken()))
                {
                    getNetwork().getServer().send(newPktMapItemUpdate, connHandleServerSide);
                }
                else
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                    assert(false);
                }
            }
        } // end server processing birth of another user
    }

    const auto insertRes = m_mapPlayers.insert(
        {
            connHandleServerSide,
            Player(getConfigProfiles(), getBullets(), getPure(), connHandleServerSide, msg.m_szIpAddress)
        }); // TODO: emplace_back()
    if (!insertRes.second)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to insert user %u into map!", __func__, connHandleServerSide);
        assert(false);
        return false;
    }
    Player& insertedPlayer = insertRes.first->second;

    for (const auto& entry : std::filesystem::directory_iterator(proofps_dd::GAME_WEAPONS_DIR))
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): %s!", __func__, entry.path().filename().string().c_str());
        if (entry.path().filename().string().length() >= proofps_dd::MsgWpnUpdateFromServer::nWpnNameNameMaxLength)
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): skip wpn due to long filename: %s!", __func__, entry.path().string().c_str());
            continue; // otherwise multiple weapons with same first nWpnNameNameMaxLength-1 chars would be mixed up in pkts
        }

        if (entry.path().extension().string() == ".txt")
        {
            Weapon* const loadedWpn = insertedPlayer.getWeaponManager().load(entry.path().string().c_str(), connHandleServerSide);
            if (!loadedWpn)
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): failed to load weapon: %s!", __func__, entry.path().string().c_str());
                assert(false);
                return false;
            }
            loadedWpn->SetOwner(connHandleServerSide);
            loadedWpn->getObject3D().SetName(loadedWpn->getObject3D().getName() + " (for user " + std::to_string(connHandleServerSide) + ")");
        }
    }

    // TODO: server should send the default weapon to client in this setup message, but for now we set same hardcoded
    // value on both side ... cheating is not possible anyway, since on server side server will always know what is
    // the default weapon for the player, so there is no use of overriding it on client side ...
    if (!insertedPlayer.getWeaponManager().setDefaultAvailableWeaponByFilename(proofps_dd::GAME_WPN_DEFAULT))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to set default weapon: %s!", __func__, proofps_dd::GAME_WPN_DEFAULT);
        assert(false);
        return false;
    }

    Weapon* const wpnDefaultAvailable = insertedPlayer.getWeaponManager().getWeaponByFilename(insertedPlayer.getWeaponManager().getDefaultAvailableWeaponFilename());
    if (!wpnDefaultAvailable)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to get default weapon!", __func__);
        assert(false);
        return false;
    }

    wpnDefaultAvailable->SetAvailable(true);
    if (!insertedPlayer.getWeaponManager().setCurrentWeapon(wpnDefaultAvailable, false, getNetwork().isServer()))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): player %u switching to %s failed due to setCurrentWeapon() failed!",
            __func__, connHandleServerSide, wpnDefaultAvailable->getFilename().c_str());
        assert(false);
        return false;
    }

    if (!insertedPlayer.getWeaponManager().getCurrentWeapon())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): no default weapon selected for user %u!", __func__, connHandleServerSide);
        assert(false);
        return false;
    }

    // Note that this is a waste of resources this way.
    // Because clients also store the full weapon instances for themselves, even though they dont use weapon cvars at all!
    // Task: On the long run, there should be a WeaponProxy or WeaponClient or something for the clients which are basically
    // just the image for their current weapon.

    return true;
}  // handleUserSetupFromServer()


bool proofps_dd::PRooFPSddPGE::handleUserNameChange(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserNameChange& msg)
{
    // TODO: make sure received user name is properly null-terminated! someone else could had sent that, e.g. malicious client or server

    const auto playerIt = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == playerIt)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return true;
    }

    if (getNetwork().isServer())
    {
        // sanity check: connHandle should be server's if bCurrentClient is set
        if ((connHandleServerSide == pge_network::ServerConnHandle) && (!msg.m_bCurrentClient))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): cannot happen: connHandleServerSide != pge_network::ServerConnHandle: %u != %u, programming error!",
                __func__, connHandleServerSide, pge_network::ServerConnHandle);
            assert(false);
            return false;
        }

        // this is a requested name so this is the place where we make sure name is unique!
        char szNewUserName[sizeof(msg.m_szUserName)];
        Player::genUniqueUserName(szNewUserName, msg.m_szUserName, m_mapPlayers);

        if (strncmp(szNewUserName, msg.m_szUserName, sizeof(msg.m_szUserName)) == 0)
        {
            getConsole().OLn("PRooFPSddPGE::%s(): name change request accepted for connHandleServerSide: %u, old name: %s, new name: %s!",
                __func__, connHandleServerSide, playerIt->second.getName().c_str(), szNewUserName);
        }
        else
        {
            getConsole().OLn("PRooFPSddPGE::%s(): name change request denied for connHandleServerSide: %u, old name: %s, requested: %s, new name: %s!",
                __func__, connHandleServerSide, playerIt->second.getName().c_str(), msg.m_szUserName, szNewUserName);
        }

        // server updates player's name first
        
        playerIt->second.setName(szNewUserName);
        // TODO: these commented lines below will be needed when we are allowing player name change WHILE already connected to the server
        // TODO: check if such name is already present in frag table, if so, then rename
        //if (!m_gameMode->renamePlayer(playerIt->second.getName().c_str(), szNewUserName))
        //{
        //    getConsole().EOLn("PRooFPSddPGE::%s(): m_gameMode->renamePlayer() FAILED!", __func__);
        //    assert(false);
        //    return false;
        //}
        if (!m_gameMode->addPlayer(playerIt->second))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): failed to insert player %s (%u) into GameMode!", __func__, szNewUserName, connHandleServerSide);
            assert(false);
            return false;
        }

        // then we let all clients except this one know about the name change
        pge_network::PgePacket newPktUserNameChange;
        if (!proofps_dd::MsgUserNameChange::initPkt(newPktUserNameChange, connHandleServerSide, false, szNewUserName))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }
        getNetwork().getServer().sendToAllClientsExcept(newPktUserNameChange, connHandleServerSide);

        if (connHandleServerSide != pge_network::ServerConnHandle)
        {
            getNetwork().getServer().setDebugNickname(connHandleServerSide, szNewUserName);
            // we also let this one know its own name change (only if this is not server)
            proofps_dd::MsgUserNameChange& msgUserNameChange = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserNameChange>(newPktUserNameChange);
            msgUserNameChange.m_bCurrentClient = true;
            getNetwork().getServer().send(newPktUserNameChange, connHandleServerSide);
        }

        if (msg.m_bCurrentClient)
        {
            m_gui.textPermanent("Server, User name: " + std::string(szNewUserName) +
                (getConfigProfiles().getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
                10, 30);
        }
    }
    else
    {
        // if we are client, we MUST NOT receive empty user name from server, so in such case just terminate because there is something fishy!
        if (strnlen(msg.m_szUserName, sizeof(msg.m_szUserName)) == 0)
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): cannot happen: connHandleServerSide: %u, received empty user name from server!",
                __func__, connHandleServerSide);
            assert(false);  // in debug mode, raise the debugger
            return false;   // for release mode
        }


        getConsole().OLn("PRooFPSddPGE::%s(): accepting new name from server for connHandleServerSide: %u (%s), old name: %s, new name: %s!",
            __func__, connHandleServerSide, msg.m_bCurrentClient ? "me" : "not me", playerIt->second.getName().c_str(), msg.m_szUserName);
        
        playerIt->second.setName(msg.m_szUserName);
        // TODO: these commented lines below will be needed when we are allowing player name change WHILE already connected to the server
        // TODO: check if such name is already present in frag table, if so, then rename
        //if (!m_gameMode->renamePlayer(playerIt->second.getName(), msg.m_szUserName))
        //{
        //    getConsole().EOLn("PRooFPSddPGE::%s(): m_gameMode->renamePlayer() FAILED!", __func__);
        //    assert(false);
        //    return false;
        //}
        if (!m_gameMode->addPlayer(playerIt->second))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): failed to insert player %s (%u) into GameMode!", __func__, msg.m_szUserName, connHandleServerSide);
            assert(false);
            return false;
        }

        if (msg.m_bCurrentClient)
        {
            // due to difficulties caused by m_gui.textPermanent() it is easier to use it here than in handleUserSetupFromServer()
            m_gui.textPermanent("Client, User name: " + playerIt->second.getName() + "; IP: " + playerIt->second.getIpAddress() +
                (getConfigProfiles().getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
                10, 30);
        }
    }

    getNetwork().WriteList();
    WritePlayerList();

    return true;

}  // handleUserNameChange()


bool proofps_dd::PRooFPSddPGE::handleMapChangeFromServer(pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/, const proofps_dd::MsgMapChangeFromServer& msg)
{
    getConsole().OLn("PRooFPSddPGE::%s(): map: %s", __func__, msg.m_szMapFilename);

    // TODO: make sure received map name is properly null-terminated! someone else could had sent that, e.g. malicious server

    // map change request may come anytime, so first we disconnect and clean up
    disconnect(false, "Map change: " + m_maps.getFilename() + " -> " + msg.m_szMapFilename);
    // reason for disconnecting from the server (in case of client) or clients (in case of server) is the following:
    // - in case of server we stop listening because we are busy anyway with map loading, cannot process messages on the main thread,
    //   so even if we could turn off GNS heartbeat supervisioning we still could not receive client messages;
    // - in case of client we disconnect because being busy with map loading on the main thread would cause the GNS connection to be
    //   torn down anyway due to not answering for heartbeats.
    // We could do the map loading on a separate thread and keep the connections active by letting the main thread loop run without blocking,
    // however the Pure graphics engine is not thread-safe thus letting both the main thread and the map loading thread call Pure would cause
    // hell.
    // Another issue with keeping the connections active would be: some clients would finish with loading faster, others later, so we would
    // need to maintain a flag per client if they are finished with loading or not: if not, we should NOT send any message to them since
    // their main thread is blocked temporarily, would cause congestion and dropped packets, also their player object should be kept hidden
    // for this temporal period. Also, if we want to keep the connections open, and load the map on main thread, we need to turn GNS heartbeat
    // supervising off for that period temporarily.
    // Sticking to the first idea: disconnecting and reconnecting later looks to be a less error-prone design where we need to handle less
    // corner cases in the future.

    // Note that when server sends this message to us, it also closes down all connections.
    // We would receive as many userDisconnected messages from the server as the number of players, however we don't receive it
    // because we also already closed the connection above.
    // But programmatically client's disconnect injects a MsgUserDisconnected pkt with server's connection handle, so the client will
    // get informed about server's disconnect anyway in handleUserDisconnected() where it should delete all players.

    if (!m_maps.load(msg.m_szMapFilename, m_cbDisplayMapLoadingProgressUpdate))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): m_maps.load() failed: %s!", __func__, msg.m_szMapFilename);
        assert(false);
        return false;
    }
    
    // Camera must start from the center of the map.
    // This is also done in both server and client in handleUserSetupFromServer(), they will get that pkg from server
    // after they successfully reconnect to server later. However due to rendering again before that, camera should already positioned now.
    getPure().getCamera().getPosVec().Set(
        (m_maps.getBlockPosMin().getX() + m_maps.getBlockPosMax().getX()) / 2.f,
        (m_maps.getBlockPosMin().getY() + m_maps.getBlockPosMax().getY()) / 2.f,
        GAME_CAM_Z);
    getPure().getCamera().getTargetVec().Set(
        getPure().getCamera().getPosVec().getX(),
        getPure().getCamera().getPosVec().getY(),
        -proofps_dd::GAME_BLOCK_SIZE_Z);

    hideLoadingScreen();
    showXHairInCenter();

    // Since we are here from a message callback, it is not really good to try building up a connection again, since
    // we already disconnected above, and we should let the main loop handle all pending messages and connection state changes,
    // only after that we should try connect back. This is true for server also, not only for clients.
    // So we just save the time here, and then try connect back when a predefined amount of time elapsed.
    // This is a temporal solution. On the long run the engine's network system should support querying of real connection state so we can
    // just query it and reconnect when it really became disconnected.
    m_timeConnectionStateChangeInitiated = std::chrono::steady_clock::now();

    // TODO: there are things that are the same as in onGameInitialized(), put them into a common function!
    m_timeSimulation = {};  // reset tick-based simulation time as well
    m_fps_lastmeasure = GetTickCount();
    m_fps = GAME_MAXFPS;

    return true;
}  // handleMapChangeFromServer()

bool proofps_dd::PRooFPSddPGE::handleUserConnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserConnectedServerSelf& msg)
{
    if (!getNetwork().isServer())
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
                if (!m_maps.load(m_maps.getNextMapToBeLoaded().c_str(), m_cbDisplayMapLoadingProgressUpdate))
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
                const PureVector& vecStartPos = getConfigProfiles().getVars()["testing"].getAsBool() ?
                    m_maps.getLeftMostSpawnpoint() :
                    m_maps.getRandomSpawnpoint();

                if (proofps_dd::MsgUserUpdateFromServer::initPkt(
                    newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, false, 100, false, 0, 0))
                {
                    // server injects this msg to self so resources for player will be allocated
                    getNetwork().getServer().send(newPktSetup);
                    getNetwork().getServer().send(newPktUserUpdate);
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

        const PureVector& vecStartPos = getConfigProfiles().getVars()["testing"].getAsBool() ?
            m_maps.getRightMostSpawnpoint() :
            m_maps.getRandomSpawnpoint();

        if (!proofps_dd::MsgUserUpdateFromServer::initPkt(
            newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, false, 100, false, 0, 0))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }

        // server injects this msg to self so resources for player will be allocated
        getNetwork().getServer().send(newPktSetup);
        getNetwork().getServer().send(newPktUserUpdate);

        // inform all other clients about this new user
        getNetwork().getServer().sendToAllClientsExcept(newPktSetup, connHandleServerSide);
        getNetwork().getServer().sendToAllClientsExcept(newPktUserUpdate, connHandleServerSide);

        // now we send this msg to the client with this bool flag set so client will know it is their connect
        proofps_dd::MsgUserSetupFromServer& msgUserSetup = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserSetupFromServer>(newPktSetup);
        msgUserSetup.m_bCurrentClient = true;
        getNetwork().getServer().send(newPktSetup, connHandleServerSide);
        getNetwork().getServer().send(newPktUserUpdate);   // TODO: why is this here? we already sent it few lines earlier.
    }

    return true;
}  // handleUserConnected()

bool proofps_dd::PRooFPSddPGE::handleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnectedFromServer&)
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
    const bool bClientShouldRemoveAllPlayers = !getNetwork().isServer() && (connHandleServerSide == pge_network::ServerConnHandle);
    if (getNetwork().isServer())
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

    m_gameMode->removePlayer(playerIt->second);
    m_mapPlayers.erase(playerIt);

    if (bClientShouldRemoveAllPlayers)
    {
        getConsole().OLn("PRooFPSddPGE::%s(): it was actually the server disconnected so I'm removing every player including myself", __func__);
        m_gameMode->restart();
        m_mapPlayers.clear();
    }

    getNetwork().WriteList();
    WritePlayerList();

    return true;
}  // handleUserDisconnected()

bool proofps_dd::PRooFPSddPGE::handleMapItemUpdateFromServer(pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/, const proofps_dd::MsgMapItemUpdateFromServer& msg)
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
}  // handleMapItemUpdateFromServer()
