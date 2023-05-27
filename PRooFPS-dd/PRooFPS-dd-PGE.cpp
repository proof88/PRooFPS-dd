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

static const int   GAME_FPS_INTERVAL = 500;
static_assert(GAME_FPS_INTERVAL > 0);

static const int   GAME_MAXFPS = 60;
static const float GAME_CAM_Z = -5.0f;
static const float GAME_CAM_SPEED = 1500.0f;

static constexpr char* CVAR_CL_SERVER_IP = "cl_server_ip";
static constexpr char* CVAR_SV_MAP = "sv_map";


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
        *this,
        m_durations,
        m_mapPlayers,
        m_maps,
        m_sounds),
    proofps_dd::Physics(
        *this,
        m_durations,
        m_mapPlayers,
        m_maps,
        m_sounds),
    proofps_dd::PlayerHandling(
        *this,
        m_durations,
        m_maps,
        m_sounds),
    proofps_dd::UserInterface(static_cast<PGE&>(*this) /* static_cast so it will call the only ctor, not the deleted copy ctor */),
    proofps_dd::WeaponHandling(
        *this,
        m_durations,
        m_mapPlayers,
        m_maps,
        m_sounds),
    m_maps(getPure()),
    m_fps(0),
    m_fps_counter(0),
    m_fps_lastmeasure(0),
    m_pObjXHair(NULL),
    m_bWon(false),
    m_fCameraMinY(0.0f)
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
    getConsole().SetLoggingState("PgeGsnWrapper", true);
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
    
    getPure().getScreen().SetVSyncEnabled(true);
    setGameRunningFrequency(GAME_MAXFPS);

    getPure().getUImanager().SetDefaultFontSize(20);

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

    m_pObjXHair = getPure().getObject3DManager().createPlane(32.f, 32.f);
    m_pObjXHair->SetStickedToScreen(true);
    m_pObjXHair->SetDoubleSided(true);
    m_pObjXHair->SetTestingAgainstZBuffer(false);
    m_pObjXHair->SetLit(false);
    // for bitmaps not having proper alpha bits (e.g. saved by irfanview or mspaint), use (PURE_SRC_ALPHA, PURE_ONE)
    // otherwise (bitmaps saved by Flash) just use (PURE_SRC_ALPHA, PURE_ONE_MINUS_SRC_ALPHA) to utilize real alpha
    m_pObjXHair->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE);
    PureTexture* xhairtex = getPure().getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR)+"hud_xhair.bmp").c_str());
    m_pObjXHair->getMaterial().setTexture( xhairtex );

    getPure().WriteList();

    // Which key should switch to which weapon
    WeaponManager::getKeypressToWeaponMap() = {
        {'2', "pistol.txt"},
        {'3', "machinegun.txt"}
    };

    if (getNetwork().isServer())
    {
        getNetwork().getServer().getAllowListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserCmdMove::id));

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
        // MsgUserSetup is also processed by server, but it injects this pkt into its own queue when needed.
        // MsgUserSetup MUST NOT be received by server over network!
        // MsgUserSetup is received only by clients over network!
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserSetup::id));

        // MsgUserUpdate is also processed by server, but it injects this pkt into its own queue when needed.
        // MsgUserUpdate MUST NOT be received by server over network!
        // MsgUserUpdate is received only by clients over network!
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserUpdate::id));

        // following messages are received only by clients over network:
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgBulletUpdate::id));
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgMapItemUpdate::id));
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgWpnUpdate::id));
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgWpnUpdateCurrent::id));

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

    LoadSound(m_sounds.m_sndLetsgo,         (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/locknload.wav").c_str());
    LoadSound(m_sounds.m_sndReloadStart,    (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/de_clipout.wav").c_str());
    LoadSound(m_sounds.m_sndReloadFinish,   (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/de_clipin.wav").c_str());
    LoadSound(m_sounds.m_sndShootPistol,    (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/deagle-1.wav").c_str());
    LoadSound(m_sounds.m_sndShootMchgun,    (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/m4a1_unsil-1.wav").c_str());
    LoadSound(m_sounds.m_sndShootDryPistol, (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/dryfire_pistol.wav").c_str());
    LoadSound(m_sounds.m_sndShootDryMchgun, (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/dryfire_rifle.wav").c_str());
    LoadSound(m_sounds.m_sndChangeWeapon,   (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/m4a1_deploy.wav").c_str());
    LoadSound(m_sounds.m_sndPlayerDie,      (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/die1.wav").c_str());

    getConsole().OOOLn("PRooFPSddPGE::onGameInitialized() done!");

    getInput().getMouse().SetCursorPos(
        getPure().getWindow().getX() + getPure().getWindow().getWidth()/2,
        getPure().getWindow().getY() + getPure().getWindow().getHeight()/2);
    getPure().getWindow().SetCursorVisible(false);

    m_deathMatchMode->setFragLimit(10);
    //m_deathMatchMode->setTimeLimitSecs(500);
    m_gameMode->restart();
    
    m_fps_lastmeasure = GetTickCount();
    m_fps = 0;

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

        // TODO: why am I checking for valid connection here if player is already in the map?
        // Probably this is a leftover from before the m_sUserName refactor. Remove this condition and test!
        if (hasValidConnection())
        {
            player.updateOldValues();
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
        m_durations.m_nFullRoundtripDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(timeOnGameRunningStart - m_durations.m_timeFullRoundtripStart).count();
    }
    m_durations.m_timeFullRoundtripStart = timeOnGameRunningStart;

    PureWindow& window = getPure().getWindow();

    m_durations.m_nFramesElapsedSinceLastDurationsReset++;

    // having valid connection means that server accepted the connection and we have initialized our player;
    // otherwise m_mapPlayers[connHandle] is dangerous as it implicitly creates entry even ...
    if (hasValidConnection())
    {
        std::chrono::time_point<std::chrono::steady_clock> timeStart;
        if (getNetwork().isServer())
        {
            mainLoopServerOnly(timeStart);
        }

        mainLoopShared(timeStart, window);
    } // endif validConnection

    // this is horrible that FPS measuring is still not available from outside of Pure .........
    m_fps_counter++;
    if (GetTickCount() - GAME_FPS_INTERVAL >= m_fps_lastmeasure)
    {
        m_fps = m_fps_counter * (1000 / GAME_FPS_INTERVAL);
        m_fps_counter = 0;
        m_fps_lastmeasure = GetTickCount();

        std::stringstream str;
        str << proofps_dd::GAME_NAME << " " << proofps_dd::GAME_VERSION << " :: FPS: " << m_fps;
        window.SetCaption(str.str());
    }
    Text(std::to_string(m_fps), window.getClientWidth() - 50, window.getClientHeight() - 2 * getPure().getUImanager().getDefaultFontSize());

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
            bRet = handleUserCmdMove(
                pkt.m_connHandleServerSide,
                reinterpret_cast<const proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData));
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
            bRet = handleWpnUpdate(pkt.m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgWpnUpdate&>(pkt.msg.app.cData), hasValidConnection());
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

    getConsole().OOOLn("proofps_dd::PRooFPSddPGE::onGameDestroying() done!");
    getConsole().Deinitialize();
}


// ############################### PRIVATE ###############################


bool proofps_dd::PRooFPSddPGE::hasValidConnection() const
{
    return m_mapPlayers.find(m_nServerSideConnectionHandle) != m_mapPlayers.end();
}

/**
    Only server executes this.
    Good for either dedicated- or listen- server.
*/
void proofps_dd::PRooFPSddPGE::mainLoopServerOnly(std::chrono::steady_clock::time_point& timeStart)
{
    timeStart = std::chrono::steady_clock::now();
    if (!m_gameMode->checkWinningConditions())
    {
        Gravity(m_fps, *m_pObjXHair);
        PlayerCollisionWithWalls(m_bWon);
    }
    m_durations.m_nGravityCollisionDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
    UpdateWeapons(*m_gameMode);
    UpdateBullets(*m_gameMode, *m_pObjXHair);
    UpdateRespawnTimers();
    PickupAndRespawnItems();
    SendUserUpdates();
}

/**
    Both clients and listen-server executes this.
    Dedicated server won't need this.
*/
void proofps_dd::PRooFPSddPGE::mainLoopShared(std::chrono::steady_clock::time_point& timeStart, PureWindow& window)
{
    Player& player = m_mapPlayers.at(m_nServerSideConnectionHandle); // cannot throw, because of bValidConnection
    timeStart = std::chrono::steady_clock::now();
    if (window.isActive())
    {
        handleInputAndSendUserCmdMove(*m_gameMode, m_fps, m_bWon, player, *m_pObjXHair);
    } // window is active
    m_durations.m_nActiveWindowStuffDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

    CameraMovement(m_fps, player);
    UpdateGameMode();  // TODO: on the long run this should be also executed only by server, now for fraglimit every instance executes ...
    m_maps.Update();
    m_maps.UpdateVisibilitiesForRenderer();
    if (player.getWeaponManager().getCurrentWeapon())
    {
        // very bad: AddText() should be used, but then RemoveText() would be also needed anytime there is a change ...
        Text(
            player.getWeaponManager().getCurrentWeapon()->getVars()["name"].getAsString() + ": " +
            std::to_string(player.getWeaponManager().getCurrentWeapon()->getMagBulletCount()) + " / " +
            std::to_string(player.getWeaponManager().getCurrentWeapon()->getUnmagBulletCount()),
            10, 150);
    }
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

void proofps_dd::PRooFPSddPGE::CameraMovement(int /*fps*/, Player& player)
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
    if (celx != campos.getX())
    {
        campos.SetX(campos.getX() + ((celx - campos.getX()) / speed));
    }
    if (cely != campos.getY())
    {
        campos.SetY(campos.getY() + ((cely - campos.getY()) / speed));
    }

    getPure().getCamera().getPosVec().Set(campos.getX(), campos.getY(), GAME_CAM_Z);
    getPure().getCamera().getTargetVec().Set(campos.getX(), campos.getY(), player.getObject3D()->getPosVec().getZ());

} // CameraMovement()

void proofps_dd::PRooFPSddPGE::SendUserUpdates()
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

        if (player.isDirty())
        {
            pge_network::PgePacket newPktUserUpdate;
            proofps_dd::MsgUserUpdate::initPkt(
                newPktUserUpdate,
                playerPair.second.getServerSideConnectionHandle(),
                player.getPos().getNew().getX(),
                player.getPos().getNew().getY(),
                player.getPos().getNew().getZ(),
                player.getAngleY(),
                player.getWeaponAngle().getNew().getY(),
                player.getWeaponAngle().getNew().getZ(),
                player.getHealth(),
                player.getRespawnFlag(),
                player.getFrags(),
                player.getDeaths());

            // player.updateOldValues() might be invoked here, however this code is only executed by server, and
            // currently onGameFrameBegin() invokes player.updateOldValues() for all players even by clients, I'm not
            // sure if there would be any difference in behavior, but logically I would call that function here ...
            // Since I assume clients should not take care of old-new values anyway, only server does that I think ...

            // we always reset respawn flag here
            playerPair.second.getRespawnFlag() = false;

            // Note that health is not needed by server since it already has the updated health, but for convenience
            // we put that into MsgUserUpdate and send anyway like all the other stuff.
            getNetwork().getServer().sendToAll(newPktUserUpdate);
        }
    }

    m_durations.m_nSendUserUpdatesDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
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

            proofps_dd::MsgMapItemUpdate::initPkt(
                newPktMapItemUpdate,
                0,
                mapItem.getId(),
                mapItem.isTaken());

            getNetwork().getServer().sendToAllClientsExcept(newPktMapItemUpdate);
        } // end for items
    } // end server

    m_gameMode->restart(); // now both server and clients execute this on their own, in future only server should do this ...
}

void proofps_dd::PRooFPSddPGE::UpdateRespawnTimers()
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

void proofps_dd::PRooFPSddPGE::PickupAndRespawnItems()
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
            proofps_dd::MsgMapItemUpdate::initPkt(
                newPktMapItemUpdate,
                pge_network::ServerConnHandle,
                mapItem.getId(),
                mapItem.isTaken());

            getNetwork().getServer().sendToAllClientsExcept(newPktMapItemUpdate);
        }
    } // for item

    m_durations.m_nPickupAndRespawnItemsDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void proofps_dd::PRooFPSddPGE::genUniqueUserName(char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength]) const
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

bool proofps_dd::PRooFPSddPGE::handleUserSetup(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserSetup& msg)
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

            if (!m_maps.load((proofps_dd::GAME_MAPS_DIR + std::string(msg.m_szMapFilename)).c_str()))
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): m_maps.load() failed: %s!", __func__, msg.m_szMapFilename);
                assert(false);
                return false;
            }
        }

        getAudio().play(m_sounds.m_sndLetsgo);
    }
    else
    {
        getConsole().OLn("PRooFPSddPGE::%s(): new user %s (connHandleServerSide: %u; IP: %s) connected",
            __func__, msg.m_szUserName, connHandleServerSide, msg.m_szIpAddress);
    }

    const auto insertRes = m_mapPlayers.insert(
        {
            connHandleServerSide,
            Player(getConfigProfiles(), getBullets(), getPure(), connHandleServerSide, msg.m_szIpAddress)
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

    for (const auto& entry : std::filesystem::directory_iterator(proofps_dd::GAME_WEAPONS_DIR))
    {
        getConsole().OLn("PRooFPSddPGE::%s(): %s!", __func__, entry.path().filename().string().c_str());
        if (entry.path().filename().string().length() >= proofps_dd::MsgWpnUpdate::nWpnNameNameMaxLength)
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
            loadedWpn->getObject3D().SetName(loadedWpn->getObject3D().getName() + " (for user " + msg.m_szUserName + ")");
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
        getConsole().EOLn("PRooFPSddPGE::%s(): player %s switching to %s failed due to setCurrentWeapon() failed!",
            __func__, msg.m_szUserName, wpnDefaultAvailable->getFilename().c_str());
        assert(false);
        return false;
    }

    if (!insertedPlayer.getWeaponManager().getCurrentWeapon())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): no default weapon selected for user %s!", __func__, msg.m_szUserName);
        assert(false);
        return false;
    }

    // Note that this is a waste of resources this way.
    // Because clients also store the full weapon instances for themselves, even though they dont use weapon cvars at all!
    // Task: On the long run, there should be a WeaponProxy or WeaponClient or something for the clients which are basically
    // just the image for their current weapon.

    getNetwork().WriteList();
    WritePlayerList();

    return true;
}

bool proofps_dd::PRooFPSddPGE::handleUserConnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserConnected& msg)
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
            if (!m_maps.load((std::string(proofps_dd::GAME_MAPS_DIR) + m_sServerMapFilenameToLoad).c_str()))
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
            getNetwork().getServer().send(newPktSetup);
            getNetwork().getServer().send(newPktUserUpdate);
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
        getNetwork().getServer().send(newPktSetup);
        getNetwork().getServer().send(newPktUserUpdate);

        // inform all other clients about this new user
        getNetwork().getServer().sendToAllClientsExcept(newPktSetup, connHandleServerSide);
        getNetwork().getServer().sendToAllClientsExcept(newPktUserUpdate, connHandleServerSide);

        // now we send this msg to the client with this bool flag set so client will know it is their connect
        proofps_dd::MsgUserSetup& msgUserSetup = reinterpret_cast<proofps_dd::MsgUserSetup&>(newPktSetup.msg.app.cData);
        msgUserSetup.m_bCurrentClient = true;
        getNetwork().getServer().send(newPktSetup, connHandleServerSide);
        getNetwork().getServer().send(newPktUserUpdate);

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
            getNetwork().getServer().send(newPktSetup, connHandleServerSide);

            proofps_dd::MsgUserUpdate::initPkt(
                newPktUserUpdate,
                it.second.getServerSideConnectionHandle(),
                it.second.getObject3D()->getPosVec().getX(),
                it.second.getObject3D()->getPosVec().getY(),
                it.second.getObject3D()->getPosVec().getZ(),
                it.second.getObject3D()->getAngleVec().getY(),
                it.second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().getY(),
                it.second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().getZ(),
                it.second.getHealth(),
                false,
                it.second.getFrags(),
                it.second.getDeaths());
            getNetwork().getServer().send(newPktUserUpdate, connHandleServerSide);
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
                pge_network::ServerConnHandle,
                itemPair.first,
                itemPair.second->isTaken());
            getNetwork().getServer().send(newPktMapItemUpdate, connHandleServerSide);
        }
    }

    return true;
}

bool proofps_dd::PRooFPSddPGE::handleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnected&)
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

bool proofps_dd::PRooFPSddPGE::handleUserUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserUpdate& msg)
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
        // PPPKKKGGGGGG
        it->second.getPos().set(
            PureVector(
                msg.m_pos.x, msg.m_pos.y, msg.m_pos.z
            ));
        it->second.getPos().commit();
    }

    it->second.getObject3D()->getPosVec().Set(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);
    it->second.getWeaponManager().getCurrentWeapon()->UpdatePosition(it->second.getObject3D()->getPosVec());

    if (msg.m_fPlayerAngleY != -1.f)
    {
        //it->second.getAngleY() = msg.m_fPlayerAngleY;
        it->second.getObject3D()->getAngleVec().SetY(msg.m_fPlayerAngleY);
    }

    it->second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().SetY(msg.m_fWpnAngleY);
    it->second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().SetZ(msg.m_fWpnAngleZ);

    //getConsole().OLn("PRooFPSddPGE::%s(): rcvd health: %d, health: %d, old health: %d",
    //    __func__, msg.m_nHealth, it->second.getHealth(), it->second.getOldHealth());

    it->second.getFrags() = msg.m_nFrags;
    it->second.getDeaths() = msg.m_nDeaths;

    it->second.SetHealth(msg.m_nHealth);

    if (msg.m_bRespawn)
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): player %s has respawned!", __func__, it->first.c_str());
        HandlePlayerRespawned(it->second, *m_pObjXHair);
    }
    else
    {
        if ((it->second.getHealth().getOld() > 0) && (it->second.getHealth() == 0))
        {
            // only clients fall here, since server already set oldhealth to 0 at the beginning of this frame
            // because it had already set health to 0 in previous frame
            //getConsole().OLn("PRooFPSddPGE::%s(): player %s has died!", __func__, it->first.c_str());
            HandlePlayerDied(it->second, *m_pObjXHair);
        }
    }

    if (!m_gameMode->updatePlayer(it->second))
    {
        getConsole().EOLn("%s: failed to update player %s in GameMode!", __func__, it->second.getName().c_str());
    }

    return true;
}

bool proofps_dd::PRooFPSddPGE::handleMapItemUpdate(pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/, const proofps_dd::MsgMapItemUpdate& msg)
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
