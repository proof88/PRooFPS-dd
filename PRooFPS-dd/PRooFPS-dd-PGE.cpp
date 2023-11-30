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
#include <iomanip>     // std::setprecision() for displaying fps
#include <utility>

#include "../../PGE/PGE/Pure/include/external/Render/PureRendererHWfixedPipe.h"  // for rendering hints
#include "../../PGE/PGE/Pure/include/external/PureUiManager.h"
#include "../../PGE/PGE/Pure/include/external/Display/PureWindow.h"
#include "../../PGE/PGE/Pure/include/external/PureCamera.h"
#include "../../Console/CConsole/src/CConsole.h"

using namespace std::chrono_literals;

static constexpr unsigned int GAME_FPS_MEASURE_INTERVAL = 500;
static_assert(GAME_FPS_MEASURE_INTERVAL > 0);

static constexpr float GAME_CAM_Z = -5.0f;
static constexpr float GAME_CAM_SPEED_X = 0.1f;
static constexpr float GAME_CAM_SPEED_Y = 0.3f;

static constexpr char* CVAR_TICKRATE = "tickrate";
static constexpr char* CVAR_PHYSICS_RATE_MIN = "physics_rate_min";
static constexpr char* CVAR_CL_UPDATERATE = "cl_updaterate";
static constexpr char* CVAR_CL_SERVER_IP = "cl_server_ip";
static constexpr char* CVAR_SV_MAP = "sv_map";
static constexpr char* CVAR_SV_ALLOW_STRAFE_MID_AIR = "sv_allow_strafe_mid_air";
static constexpr char* CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL = "sv_allow_strafe_mid_air_full";
static constexpr char* CVAR_SV_RECONNECT_DELAY = "sv_reconnect_delay";
static constexpr char* CVAR_CL_RECONNECT_DELAY = "cl_reconnect_delay";

static constexpr char* CVAR_GFX_CAM_FOLLOWS_XHAIR = "gfx_cam_follows_xhair";
static constexpr char* CVAR_GFX_CAM_TILTING = "gfx_cam_tilting";


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
        m_mapPlayers,
        m_maps,
        m_sounds),
    proofps_dd::UserInterface(static_cast<PGE&>(*this) /* static_cast so it will call the only ctor, not the deleted copy ctor */),
    proofps_dd::WeaponHandling(
        *this,
        m_durations,
        m_mapPlayers,
        m_maps,
        m_sounds),
    m_gameMode(nullptr),
    m_deathMatchMode(nullptr),
    m_nSecondsReconnectDelay(GAME_NETWORK_RECONNECT_SECONDS),
    m_maps(getPure()),
    m_nTickrate(GAME_TICKRATE_DEF),
    m_nPhysicsRateMin(GAME_PHYSICS_RATE_MIN_DEF),
    m_nClientUpdateRate(GAME_CL_UPDATERATE_DEF),
    m_bCamFollowsXHair(true),
    m_bCamTilting(true),
    m_fps(GAME_MAXFPS),
    m_fps_counter(0),
    m_fps_lastmeasure(0),
    m_bFpsFirstMeasure(true),
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
    
    getPure().getScreen().SetVSyncEnabled(true);
    setGameRunningFrequency(GAME_MAXFPS);
    getConsole().OLn("Game running frequency: %u Hz", getGameRunningFrequency());

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
    m_pObjXHair->Hide();

    getPure().WriteList();

    // Which key should switch to which weapon
    WeaponManager::getKeypressToWeaponMap() = {
        {'2', "pistol.txt"},
        {'3', "machinegun.txt"}
    };

    // We tell the names of our app messages to the network engine so it can properly log message stats with message names
    for (const auto& msgAppId2StringPair : MapMsgAppId2String)
    {
        getNetwork().getServerClientInstance()->getMsgAppId2StringMap().insert(
            { static_cast<pge_network::MsgApp::TMsgId>(msgAppId2StringPair.msgId),
              std::string(msgAppId2StringPair.zstring) }
        );
    }

    if (getNetwork().isServer())
    {
        getNetwork().getServer().getAllowListedAppMessages().insert(static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgUserCmdFromClient::id));

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
        // MsgMapChangeFromServer is also processed by server, but it injects this pkt into its own queue when needed.
        // MsgMapChangeFromServer MUST NOT be received by server over network!
        // MsgMapChangeFromServer is received only by clients over network!
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgMapChangeFromServer::id));

        // MsgUserSetupFromServer is also processed by server, but it injects this pkt into its own queue when needed.
        // MsgUserSetupFromServer MUST NOT be received by server over network!
        // MsgUserSetupFromServer is received only by clients over network!
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgUserSetupFromServer::id));

        // MsgUserUpdateFromServer is also processed by server, but it injects this pkt into its own queue when needed.
        // MsgUserUpdateFromServer MUST NOT be received by server over network!
        // MsgUserUpdateFromServer is received only by clients over network!
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgUserUpdateFromServer::id));

        // following messages are received only by clients over network:
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgBulletUpdateFromServer::id));
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgMapItemUpdateFromServer::id));
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgWpnUpdateFromServer::id));
        getNetwork().getClient().getAllowListedAppMessages().insert(static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgCurrentWpnUpdateFromServer::id));

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

    // TODO: too much validation here, validation probably should be done by CVARS themselves.
    // Update this code after implementing: https://github.com/proof88/PRooFPS-dd/issues/251 .
    if (!getConfigProfiles().getVars()[CVAR_TICKRATE].getAsString().empty())
    {
        if ((getConfigProfiles().getVars()[CVAR_TICKRATE].getAsInt() >= GAME_TICKRATE_MIN) &&
            (getConfigProfiles().getVars()[CVAR_TICKRATE].getAsUInt() <= getGameRunningFrequency()))
        {
            m_nTickrate = getConfigProfiles().getVars()[CVAR_TICKRATE].getAsUInt();
            getConsole().OLn("Tickrate from config: %u Hz", m_nTickrate);
        }
        else
        {
            getConsole().EOLn("ERROR: Invalid Tickrate in config: %s, forcing default: %u Hz",
                getConfigProfiles().getVars()[CVAR_TICKRATE].getAsString().c_str(),
                m_nTickrate);
        }
    }
    else
    {
        getConsole().OLn("Missing Tickrate in config, forcing default: %u Hz", m_nTickrate);
    }

    if (!getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsString().empty())
    {
        if ((getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsUInt() >= m_nTickrate) &&
            (getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsUInt() <= GAME_PHYSICS_RATE_MIN_MAX) &&
            /* Physics update distribution in time must be constant/even if we do it more frequently than tickrate. */
            (getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsUInt() % m_nTickrate == 0u))
        {
            m_nPhysicsRateMin = getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsUInt();
            getConsole().OLn("Min. physics rate from config: %u Hz", m_nPhysicsRateMin);
        }
        else
        {
            m_nPhysicsRateMin = m_nTickrate;
            getConsole().EOLn("ERROR: Invalid Min. physics rate in config: %s, forcing tickrate: %u Hz",
                getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsString().c_str(),
                m_nTickrate);
        }
    }
    else
    {
        getConsole().OLn("Missing Min. physics rate in config, forcing default: %u Hz", m_nPhysicsRateMin);
    }

    if (!getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsString().empty())
    {
        if ((getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsInt() >= GAME_CL_UPDATERATE_MIN) &&
            (getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsUInt() <= m_nTickrate) &&
            /* Clients should receive UPDATED physics results evenly distributed in time. */
            (m_nTickrate % getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsUInt() == 0u))
        {
            m_nClientUpdateRate = getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsUInt();
            getConsole().OLn("Client update rate from config: %u Hz", m_nClientUpdateRate);
        }
        else
        {
            m_nClientUpdateRate = m_nTickrate;
            getConsole().EOLn("ERROR: Invalid Client update rate in config: %s, forcing tickrate: %u Hz",
                getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsString().c_str(),
                m_nClientUpdateRate);
        }
    }
    else
    {
        getConsole().OLn("Missing Client update rate in config, forcing default: %u Hz", m_nClientUpdateRate);
    }

    const char* const szCVarReconnectDelay = getNetwork().isServer() ? CVAR_SV_RECONNECT_DELAY : CVAR_CL_RECONNECT_DELAY;

    if (!getConfigProfiles().getVars()[szCVarReconnectDelay].getAsString().empty())
    {
        if (getConfigProfiles().getVars()[szCVarReconnectDelay].getAsInt() >= 0)
        {
            m_nSecondsReconnectDelay = getConfigProfiles().getVars()[szCVarReconnectDelay].getAsUInt();
            getConsole().OLn("%s rate from config: %u seconds", szCVarReconnectDelay, m_nSecondsReconnectDelay);
        }
        else
        {
            getConsole().EOLn("ERROR: Invalid %s in config: %s, forcing: %u seconds",
                szCVarReconnectDelay, getConfigProfiles().getVars()[szCVarReconnectDelay].getAsString().c_str(),
                m_nSecondsReconnectDelay);
        }
    }
    else
    {
        getConsole().OLn("Missing %s, forcing default: %u seconds", szCVarReconnectDelay, m_nSecondsReconnectDelay);
    }

    serverSetAllowStrafeMidAir( getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR].getAsBool() );
    
    if (getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].getAsBool() &&
        !getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR].getAsBool())
    {
        getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].Set(false);
        getConsole().EOLn("ERROR: %s cannot be true when %s is false, forcing false!",
            CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL, CVAR_SV_ALLOW_STRAFE_MID_AIR);
    }
    serverSetAllowStrafeMidAirFull( getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].getAsBool() );

    m_bCamFollowsXHair = getConfigProfiles().getVars()[CVAR_GFX_CAM_FOLLOWS_XHAIR].getAsBool();
    m_bCamTilting = getConfigProfiles().getVars()[CVAR_GFX_CAM_TILTING].getAsBool();

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
    LoadSound(m_sounds.m_sndShootDryPistol, (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/dryfire_pistol.wav").c_str());
    LoadSound(m_sounds.m_sndShootDryMchgun, (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/dryfire_rifle.wav").c_str());
    LoadSound(m_sounds.m_sndChangeWeapon,   (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/m4a1_deploy.wav").c_str());
    LoadSound(m_sounds.m_sndPlayerDie,      (std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/die1.wav").c_str());

    getConsole().OOOLn("PRooFPSddPGE::onGameInitialized() done!");

    getInput().getMouse().SetCursorPos(
        getPure().getWindow().getX() + getPure().getWindow().getWidth()/2,
        getPure().getWindow().getY() + getPure().getWindow().getHeight()/2);
    getPure().getWindow().SetCursorVisible(false);
    m_pObjXHair->Show();

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

    // having valid connection means that server accepted the connection and we have initialized our player;
    // otherwise m_mapPlayers[connHandle] is dangerous as it implicitly creates entry ...
    if (hasValidConnection())
    {
        if (getNetwork().isServer())
        {
            serverUpdateWeapons(*m_gameMode);
        }

        // 1 TICK START
        static const auto DurationSimulationStepMicrosecsPerTick = std::chrono::microseconds((1000 * 1000) / m_nTickrate);
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
                mainLoopServerOnlyOneTick(DurationSimulationStepMicrosecsPerTick.count());
            }
            else
            {
                mainLoopClientOnlyOneTick(DurationSimulationStepMicrosecsPerTick.count());
            }
        }
        // 1 TICK END

        mainLoopShared(window);
    } // endif validConnection
    else
    {
        // try connecting back
        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_timeConnectionStateChangeInitiated).count() >= m_nSecondsReconnectDelay)
        {
            // now try to connect back
            // TODO: we will need to think about the possible outcomes of when server is loading slowly, thus client trying connecting might time out.
            // So client should try connecting multiple times for this reason.
            if (getNetwork().isServer())
            {
                Text("Starting Server ...", 200, getPure().getWindow().getClientHeight() / 2);
                getPure().getRenderer()->RenderScene();

                if (!getNetwork().getServer().startListening())
                {
                    getConsole().EOLn("ERROR: Server has FAILED to start listening!");

                    // try again a bit later
                    m_timeConnectionStateChangeInitiated = std::chrono::steady_clock::now();
                }
                else
                {
                    m_pObjXHair->Show();
                    m_timeSimulation = {};  // reset tick-based simulation time as well
                }
            }
            else
            {
                std::string sIp = "127.0.0.1";
                if (!getConfigProfiles().getVars()[CVAR_CL_SERVER_IP].getAsString().empty())
                {
                    sIp = getConfigProfiles().getVars()[CVAR_CL_SERVER_IP].getAsString();
                }
            
                Text("Connecting to " + sIp + " ...", 200, getPure().getWindow().getClientHeight() / 2);
                getPure().getRenderer()->RenderScene();
            
                if (!getNetwork().getClient().connectToServer(sIp))
                {
                    getConsole().EOLn("ERROR: Client has FAILED to establish connection to the server!");

                    // try again a bit later
                    m_timeConnectionStateChangeInitiated = std::chrono::steady_clock::now();
                }
                else
                {
                    m_pObjXHair->Show();
                    m_timeSimulation = {};  // reset tick-based simulation time as well
                }
            }
        }
        else
        {
            Text("Waiting for restoring connection ...", 200, getPure().getWindow().getClientHeight() / 2);
            if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_timeLastPrintWaitConnection).count() >= 1)
            {
                m_timeLastPrintWaitConnection = std::chrono::steady_clock::now();
                getConsole().EOLn("Waiting a bit before trying to connect back ... ");
            }
        }
    }

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
        case proofps_dd::MsgUserCmdFromClient::id:
            bRet = handleUserCmdMoveFromClient(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt));
            break;
        case proofps_dd::MsgUserUpdateFromServer::id:
            bRet = handleUserUpdateFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserUpdateFromServer>(pkt));
            break;
        case proofps_dd::MsgBulletUpdateFromServer::id:
            bRet = handleBulletUpdateFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgBulletUpdateFromServer>(pkt));
            break;
        case proofps_dd::MsgMapItemUpdateFromServer::id:
            bRet = handleMapItemUpdateFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgMapItemUpdateFromServer>(pkt));
            break;
        case proofps_dd::MsgWpnUpdateFromServer::id:
            bRet = handleWpnUpdateFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgWpnUpdateFromServer>(pkt),
                hasValidConnection());
            break;
        case proofps_dd::MsgCurrentWpnUpdateFromServer::id:
            bRet = handleWpnUpdateCurrentFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgCurrentWpnUpdateFromServer>(pkt));
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

    Text("Exiting game ...", 200, getPure().getWindow().getClientHeight() / 2);
    getPure().getRenderer()->RenderScene();

    //getConsole().SetLoggingState("4LLM0DUL3S", true);
    //getPure().WriteList();
    //getConsole().SetLoggingState("4LLM0DUL3S", false);

    // TODO: check common parts with disconnect()
    m_mapPlayers.clear();       // Dtors of Player instances will be implicitly called
    deleteWeaponHandlingAll();  // Dtors of Bullet instances will be implicitly called
    m_sServerMapFilenameToLoad.clear();
    m_maps.shutdown();
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
    // This is why it is very important to clear out m_nServerSideConnectionHandle when we are disconnecting:
    // to make sure this conditiona fails, so the main loop is aware of not having a valid connection.
    // And it is also very important to clear m_mapPlayers in that case, otherwise 2 bugs would occur:
    // - server: it would still be able to find itself in the map, obviously since 0 is its own handle anyway;
    // - client: it would still be able to find the server in the map, which is a false conclusion of having a valid connection.
    return m_mapPlayers.find(m_nServerSideConnectionHandle) != m_mapPlayers.end();
}

void proofps_dd::PRooFPSddPGE::disconnect(const std::string& sExtraDebugText)
{
    getPure().getUImanager().RemoveAllPermanentTexts(); // cannot find better way to get rid of permanent texts
    const std::string sPrintText =
        sExtraDebugText.empty() ?
        "Unloading resources ..." :
        "Unloading resources ... Reason: " + sExtraDebugText;
    Text(sPrintText, 200, getPure().getWindow().getClientHeight() / 2);
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
    // We need m_mapPlayers to be cleared out by the end of processing all disconnections, the reasion is explained in hasValidConnection().

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
    m_sServerMapFilenameToLoad.clear();
    m_maps.unload();
}

/**
    Only server executes this.
    Good for either dedicated- or listen- server.
*/
void proofps_dd::PRooFPSddPGE::mainLoopServerOnlyOneTick(
    const long long& /*durElapsedMicrosecs*/)
{
    /*
    * This function is executed every tick.
    * If executed rarely i.e. with very low tickrate e.g. 1 tick/sec, players and bullets might "jump" over walls.
    * To solve this, we might run multiple physics iterations (nPhysicsIterationsPerTick) so movements are
    * calculated in smaller steps, resulting in more precise results.
    */
    static const unsigned int nPhysicsIterationsPerTick = std::max(1u, m_nPhysicsRateMin / m_nTickrate);
    if (!m_gameMode->checkWinningConditions())
    {
        for (unsigned int iPhyIter = 1; iPhyIter <= nPhysicsIterationsPerTick; iPhyIter++)
        {
            const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
            serverGravity(*m_pObjXHair, m_nPhysicsRateMin);
            serverPlayerCollisionWithWalls(m_bWon, m_nPhysicsRateMin);
            m_durations.m_nGravityCollisionDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
            serverUpdateBullets(*m_gameMode, *m_pObjXHair, m_nPhysicsRateMin);
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
void proofps_dd::PRooFPSddPGE::mainLoopClientOnlyOneTick(
    const long long& /*durElapsedMicrosecs*/)
{
    /*
    * This function is executed every tick.
    * Since this is executed by client, we dont care about physics-related concerns explained in comments in mainLoopServerOnlyOneTick(). 
    */
    static const unsigned int nPhysicsIterationsPerTick = std::max(1u, m_nPhysicsRateMin / m_nTickrate);
    for (unsigned int iPhyIter = 1; iPhyIter <= nPhysicsIterationsPerTick; iPhyIter++)
    {
        clientUpdateBullets(m_nPhysicsRateMin);
    }
}

/**
    Both clients and listen-server executes this.
    Dedicated server won't need this.
*/
void proofps_dd::PRooFPSddPGE::mainLoopShared(PureWindow& window)
{
    std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
    Player& player = m_mapPlayers.at(m_nServerSideConnectionHandle); // cannot throw, because of bValidConnection
    if (window.isActive())
    {
        handleInputAndSendUserCmdMove(*m_gameMode, m_bWon, player, *m_pObjXHair, m_nTickrate, m_nClientUpdateRate, m_nPhysicsRateMin);
    } // window is active
    m_durations.m_nActiveWindowStuffDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

    CameraMovement(player, m_bCamFollowsXHair, m_bCamTilting);
    UpdateGameMode();  // TODO: on the long run this should be also executed only by server, now for fraglimit every instance executes ...
    m_maps.Update(m_fps);
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
                " Server :: Tickrate : " << m_nTickrate <<
                " Hz :: MinPhyRate : " << m_nPhysicsRateMin <<
                " Hz :: ClUpdRate : " << m_nClientUpdateRate <<
                " Hz :: FPS : " << ssFps.str();
        }
        else
        {
            str << proofps_dd::GAME_NAME << " " << proofps_dd::GAME_VERSION <<
                " Client :: Tickrate : " << m_nTickrate <<
                " Hz :: FPS : " << ssFps.str();
        }
        window.SetCaption(str.str());

        if (m_fps < 0.01f)
        {
            m_fps = 0.01f; // make sure nobody tries division by zero
        }
    }
    Text(ssFps.str(), window.getClientWidth() - 50, window.getClientHeight() - 2 * getPure().getUImanager().getDefaultFontSize());
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
    float fCamTargetX, fCamTargetY;
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

        fCamTargetX = std::min(
            fCamMaxAllowedPosX,
            std::max(fCamMinAllowedPosX, (player.getObject3D()->getPosVec().getX() + vecUnprojected.getX())/2));
        
        fCamTargetY = std::min(
            fCamMaxAllowedPosY,
            std::max(fCamMinAllowedPosY, (player.getObject3D()->getPosVec().getY() + vecUnprojected.getY())/2));
    }
    else
    {
        fCamTargetX = std::min(
            fCamMaxAllowedPosX,
            std::max(fCamMinAllowedPosX, player.getObject3D()->getPosVec().getX()));

        fCamTargetY = std::min(
            fCamMaxAllowedPosY,
            std::max(fCamMinAllowedPosY, player.getObject3D()->getPosVec().getY()));
    }

    const PureVector vecCamPos{
        PFL::smooth(
            camera.getPosVec().getX(), fCamTargetX, GAME_CAM_SPEED_X * m_fps),
        PFL::smooth(
            camera.getPosVec().getY(),
            fCamTargetY,
            /* if we are not following xhair, we want an eased vertical camera movement because it looks nice */
            (bCamFollowsXHair ? (GAME_CAM_SPEED_X * m_fps) : (GAME_CAM_SPEED_Y * m_fps))),
        GAME_CAM_Z
    };

    camera.getPosVec() = vecCamPos;
    camera.getTargetVec().Set(
        bCamTilting ? ((vecCamPos.getX() + fCamTargetX) / 2.f) : vecCamPos.getX(),
        bCamTilting ? ((vecCamPos.getY() + fCamTargetY) / 2.f) : vecCamPos.getY(),
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

    static const unsigned int nSendClientUpdatesInEveryNthTick = m_nTickrate / m_nClientUpdateRate;
    static unsigned int nSendClientUpdatesCntr = nSendClientUpdatesInEveryNthTick;
    const bool bSendUserUpdates = (nSendClientUpdatesCntr == nSendClientUpdatesInEveryNthTick);

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
        nSendClientUpdatesCntr = 0;
        // measure duration only if we really sent the user updates to clients
        m_durations.m_nSendUserUpdatesDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
    }

    ++nSendClientUpdatesCntr;
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
}

void proofps_dd::PRooFPSddPGE::genUniqueUserName(char szNewUserName[proofps_dd::MsgUserSetupFromServer::nUserNameMaxLength]) const
{
    bool found = false;
    do
    {
        sprintf_s(szNewUserName, proofps_dd::MsgUserSetupFromServer::nUserNameMaxLength, "User%d", 10000 + (rand() % 100000));
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

bool proofps_dd::PRooFPSddPGE::handleUserSetupFromServer(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserSetupFromServer& msg)
{
    if ((strnlen(msg.m_szUserName, proofps_dd::MsgUserSetupFromServer::nUserNameMaxLength) > 0) && (m_mapPlayers.end() != m_mapPlayers.find(connHandleServerSide)))
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
            // if we are server and current client is us, then our m_nServerSideConnectionHandle should be pge_network::ServerConnHandle
            if (m_nServerSideConnectionHandle != pge_network::ServerConnHandle)
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): cannot happen: m_nServerSideConnectionHandle != pge_network::ServerConnHandle: %u != %u, programming error!",
                    __func__, m_nServerSideConnectionHandle, pge_network::ServerConnHandle);
                assert(false);
                return false;
            }

            AddText("Server, User name: " + std::string(msg.m_szUserName) +
                (getConfigProfiles().getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
                10, 30);

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
            AddText("Client, User name: " + std::string(msg.m_szUserName) + "; IP: " + msg.m_szIpAddress +
                (getConfigProfiles().getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
                10, 30);

            // Client receives map name also in MsgUserSetupFromServer.
            // If this is a bootup, then we need to load map here, only if it is different than what we have already loaded (map change case).
            // Because we also get here in case of reconnecting after a map change, we should load the map only if it is different than server just asked for.
            if (m_maps.getFilename() != msg.m_szMapFilename)
            {
                // if we fall here with non-empty m_maps.getFilename(), it is an error, and m_maps.load() will fail as expected.
                Text("Loading Map: " + std::string(msg.m_szMapFilename) + " ...", 200, getPure().getWindow().getClientHeight() / 2);
                getPure().getRenderer()->RenderScene();

                if (!m_maps.load(msg.m_szMapFilename))
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

bool proofps_dd::PRooFPSddPGE::handleMapChangeFromServer(pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/, const proofps_dd::MsgMapChangeFromServer& msg)
{
    getConsole().OLn("PRooFPSddPGE::%s(): map: %s", __func__, msg.m_szMapFilename);

    // map change request may come anytime, so first we disconnect and clean up
    disconnect("Map change: " + m_maps.getFilename() + " -> " + msg.m_szMapFilename);
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
    // get informed about server's disconnect anyway in handleUserDisconnected() where it can delete all players.

    m_sServerMapFilenameToLoad = msg.m_szMapFilename;

    Text("Loading Map: " + std::string(msg.m_szMapFilename) + " ...", 200, getPure().getWindow().getClientHeight() / 2);
    getPure().getRenderer()->RenderScene();
    
    //const bool mapLoaded = m_maps.load("gamedata/maps/map_test_good.txt");
    if (!m_maps.load((msg.m_szMapFilename)))
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
}

bool proofps_dd::PRooFPSddPGE::handleUserConnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserConnectedServerSelf& msg)
{
    if (!getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): client received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    const char* szConnectedUserName = nullptr;
    pge_network::PgePacket newPktUserUpdate;

    if (msg.m_bCurrentClient)
    {
        // server is processing its own birth
        if (m_mapPlayers.size() == 0)
        {
            Text("Loading Map: " + m_sServerMapFilenameToLoad + " ...", 200, getPure().getWindow().getClientHeight() / 2);
            getPure().getRenderer()->RenderScene();

            // server already loads the map for itself at this point
            //const bool mapLoaded = m_maps.load("gamedata/maps/map_test_good.txt");
            if (m_maps.getFilename() != m_sServerMapFilenameToLoad)
            {
                // if we fall here with non-empty m_maps.getFilename(), it is an error, and m_maps.load() will fail as expected.
                if (!m_maps.load(m_sServerMapFilenameToLoad.c_str()))
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): m_maps.load() failed: %s!", __func__, m_sServerMapFilenameToLoad.c_str());
                    assert(false);
                    return false;
                }
            }
            else
            {
                getConsole().OLn("PRooFPSddPGE::%s(): map %s already loaded", __func__, m_sServerMapFilenameToLoad.c_str());
            }

            char szNewUserName[proofps_dd::MsgUserSetupFromServer::nUserNameMaxLength];
            genUniqueUserName(szNewUserName);
            getConsole().OLn("PRooFPSddPGE::%s(): first (local) user %s connected and I'm server, so this is me (connHandleServerSide: %u)",
                __func__, szNewUserName, connHandleServerSide);

            szConnectedUserName = szNewUserName;

            pge_network::PgePacket newPktSetup;
            if (proofps_dd::MsgUserSetupFromServer::initPkt(newPktSetup, connHandleServerSide, true, szConnectedUserName, msg.m_szIpAddress, m_sServerMapFilenameToLoad.c_str()))
            {
                const PureVector& vecStartPos = getConfigProfiles().getVars()["testing"].getAsBool() ?
                    m_maps.getLeftMostSpawnpoint() :
                    m_maps.getRandomSpawnpoint();

                if (proofps_dd::MsgUserUpdateFromServer::initPkt(
                    newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, 100, false, 0, 0))
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

        char szNewUserName[proofps_dd::MsgUserSetupFromServer::nUserNameMaxLength];
        genUniqueUserName(szNewUserName);
        szConnectedUserName = szNewUserName;
        getConsole().OLn("PRooFPSddPGE::%s(): new remote user %s (connHandleServerSide: %u) connected (from %s) and I'm server",
            __func__, szConnectedUserName, connHandleServerSide, msg.m_szIpAddress);

        pge_network::PgePacket newPktSetup;
        if (!proofps_dd::MsgUserSetupFromServer::initPkt(newPktSetup, connHandleServerSide, false, szConnectedUserName, msg.m_szIpAddress, m_maps.getFilename()))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }

        const PureVector& vecStartPos = getConfigProfiles().getVars()["testing"].getAsBool() ?
            m_maps.getRightMostSpawnpoint() :
            m_maps.getRandomSpawnpoint();

        if (!proofps_dd::MsgUserUpdateFromServer::initPkt(
            newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, 100, false, 0, 0))
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

        // we also send as many MsgUserSetupFromServer pkts to the client as the number of already connected players,
        // otherwise client won't know about them, so this way the client will detect them as newly connected users;
        // we also send MsgUserUpdateFromServer about each player so new client will immediately have their positions and other data updated,
        // and MsgCurrentWpnUpdateFromServer so their current weapon is also correctly display instead of the default wpn.
        for (const auto& it : m_mapPlayers)
        {
            if (!proofps_dd::MsgUserSetupFromServer::initPkt(
                newPktSetup,
                it.second.getServerSideConnectionHandle(),
                false,
                it.second.getName(), it.second.getIpAddress(),
                "" /* here mapFilename is irrelevant */))
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
                continue;
            }
            getNetwork().getServer().send(newPktSetup, connHandleServerSide);

            if (!proofps_dd::MsgUserUpdateFromServer::initPkt(
                newPktUserUpdate,
                it.second.getServerSideConnectionHandle(),
                it.second.getObject3D()->getPosVec().getX(),
                it.second.getObject3D()->getPosVec().getY(),
                it.second.getObject3D()->getPosVec().getZ(),
                it.second.getObject3D()->getAngleVec().getY(),
                it.second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().getZ(),
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
    }

    return true;
}

bool proofps_dd::PRooFPSddPGE::handleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnectedFromServer&)
{
    const auto playerIt = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == playerIt)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        assert(false); // in debug mode, try to understand this scenario
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
}

bool proofps_dd::PRooFPSddPGE::handleUserUpdateFromServer(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserUpdateFromServer& msg)
{
    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return true;  // might NOT be fatal error in some circumstances, although I cannot think about any, but dont terminate the app for this ...
    }

    //getConsole().OLn("PRooFPSddPGE::%s(): user %s received MsgUserUpdateFromServer: %f", __func__, it->second.getName().c_str(), msg.m_pos.x);

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

    it->second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().SetY(it->second.getObject3D()->getAngleVec().getY());
    it->second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().SetZ(msg.m_fWpnAngleZ);

    //getConsole().OLn("PRooFPSddPGE::%s(): rcvd health: %d, health: %d, old health: %d",
    //    __func__, msg.m_nHealth, it->second.getHealth(), it->second.getOldHealth());

    it->second.getFrags() = msg.m_nFrags;
    it->second.getDeaths() = msg.m_nDeaths;

    it->second.SetHealth(msg.m_nHealth);

    if (msg.m_bRespawn)
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): player %s has respawned!", __func__, it->second.getName().c_str());
        HandlePlayerRespawned(it->second, *m_pObjXHair);
    }
    else
    {
        if ((it->second.getHealth().getOld() > 0) && (it->second.getHealth() == 0))
        {
            // only clients fall here, since server already set oldhealth to 0 at the beginning of this frame
            // because it had already set health to 0 in previous frame
            //getConsole().OLn("PRooFPSddPGE::%s(): player %s has died!", __func__, it->second.getName().c_str());
            HandlePlayerDied(it->second, *m_pObjXHair);
        }
    }

    if (!m_gameMode->updatePlayer(it->second))
    {
        getConsole().EOLn("%s: failed to update player %s in GameMode!", __func__, it->second.getName().c_str());
    }

    if (!getNetwork().isServer())
    {
        // server already invoked updateOldValues() when it sent out this update message
        it->second.updateOldValues();
    }

    return true;
}

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
}
