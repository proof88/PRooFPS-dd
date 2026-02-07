/*
    ###################################################################################
    PRooFPSddPGE.cpp
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022 - 2024
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
    proofps_dd::CameraHandling(
        this->getPure() /* cannot be null since PGE is initialized in above line */,
        m_durations,
        m_maps),
    proofps_dd::InputHandling(
        *this, /* Hint: for 1-param ctors, use: static_cast<PGE&>(*this) so it will call the only ctor, not the deleted copy ctor. */
        m_durations,
        m_gui,
        m_mapPlayers,
        m_maps,
        m_sounds,
        *this),
    proofps_dd::Physics(
        *this,
        m_durations,
        m_gui,
        m_mapPlayers,
        m_maps,
        m_sounds,
        *this),
    proofps_dd::PlayerHandling(
        *this,
        m_durations,
        m_gui,
        m_mapPlayers,
        m_maps,
        m_sounds,
        *this),
    proofps_dd::Sounds(),
    proofps_dd::WeaponHandling(
        *this,
        Config::getConfigInstance(*this, m_maps),
        m_durations,
        m_gui,
        m_mapPlayers,
        m_maps,
        m_sounds,
        *this),
    m_config(Config::getConfigInstance(*this, m_maps)),
    m_gui(GUI::getGuiInstance(*this, *this, m_config, m_maps, *this, m_mapPlayers, this->getSmokePool())),
    m_maps(getAudio(), getConfigProfiles(), getPure()),
    m_fps(GAME_MAXFPS_DEF),
    m_fps_counter(0),
    m_fps_lastmeasure(0),
    m_bFpsFirstMeasure(true)
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
    
    m_config.validate();
    setGameRunningFrequency( getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsUInt() );
    getConsole().OLn("Game running frequency: %u Hz", getGameRunningFrequency());

    getConsole().SetLoggingState("4LLM0DUL3S", false);

    cameraInitForGameStart();

    // create GameMode right away here, based on config, will be recreated later anyway if config is changed.
    if (!proofps_dd::GameMode::createGameMode(GameMode::getGameModeTypeFromConfig(getConfigProfiles())))
    {
        getConsole().EOLnOO("ERROR: createGameMode() failed!");
        return false;
    }

    if (!m_maps.initialize())
    {
        getConsole().EOLnOO("ERROR: m_maps.initialize() failed!");
        return false;
    }

    // BAD: physics stuff should not be set here, it should be done in config.validate(), however
    // in that case Config would need the Physics definition which overall leads to circular including each other,
    // leading to GameMode.cpp unable to compile.
    serverSetCollisionModeBvh(getConfigProfiles().getVars()[Maps::szCVarSvMapCollisionMode].getAsInt() == 1);

    m_gui.initialize();
    m_gui.setServerRestartGameCallback([this]() { serverRestartGame(); });

    m_cbDisplayMapLoadingProgressUpdate = [this](int nProgress)
    {
        showLoadingScreen(nProgress);
    };

    getPure().WriteList();

    // TODO: log level override support: getConsole().SetLoggingState(sTrimmedLine.c_str(), true);

    reinitializeNetworking();

    if (getNetwork().isServer() && !getConfigProfiles().getVars()[proofps_dd::GUI::CVAR_GUI_MAINMENU].getAsBool())
    {
        if (m_maps.serverDecideFirstMapAndUpdateNextMapToBeLoaded().empty())
        {
            getConsole().EOLnOO("ERROR: Server is unable to select first map!");
            PGE::showErrorDialog("Server is unable to select first map!");
            return false;
        }
    }

    getConsole().OLn("");
    getConsole().OLn("size of PgePacket: %u Bytes", sizeof(pge_network::PgePacket));
    getConsole().OLn("  size of MsgUserCmdFromClient: %u Bytes", sizeof(proofps_dd::MsgUserCmdFromClient));
    getConsole().OLn("  size of MsgUserUpdateFromServer: %u Bytes", sizeof(proofps_dd::MsgUserUpdateFromServer));
    getConsole().OLn("  size of MsgBulletUpdateFromServer: %u Bytes", sizeof(proofps_dd::MsgBulletUpdateFromServer));
    getConsole().OLn("  size of MsgWpnUpdateFromServer: %u Bytes", sizeof(proofps_dd::MsgWpnUpdateFromServer));
    getConsole().OLn("  size of MsgCurrentWpnUpdateFromServer: %u Bytes", sizeof(proofps_dd::MsgCurrentWpnUpdateFromServer));
    getConsole().OLn("  size of MsgMapItemUpdateFromServer: %u Bytes", sizeof(proofps_dd::MsgMapItemUpdateFromServer));
    getConsole().OLn("  size of MsgPlayerEventFromServer: %u Bytes", sizeof(proofps_dd::MsgPlayerEventFromServer));
    getConsole().OLn("  size of MsgDeathNotificationFromServer: %u Bytes", sizeof(proofps_dd::MsgDeathNotificationFromServer));
    getConsole().OLn("");

    getAudio().loadSound(m_sounds.m_sndMenuMusic,     std::string(proofps_dd::GAME_AUDIO_DIR) + "menu/Monkeys_Spinning_Monkeys.mp3");
    m_sounds.m_sndMenuMusic.setSingleInstance(true);
    getAudio().loadSound(m_sounds.m_sndEndgameMusic, std::string(proofps_dd::GAME_AUDIO_DIR) + "menu/Fart_with_Musical_Instrument_XSoundEffect.mp3");
    m_sounds.m_sndEndgameMusic.setSingleInstance(true);
    getAudio().loadSound(m_sounds.m_sndLetsgo,        std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/locknload.wav");
    getAudio().loadSound(m_sounds.m_sndChangeWeapon,  std::string(proofps_dd::GAME_AUDIO_DIR) + "weapons/m4a1_deploy.wav");
    getAudio().loadSound(m_sounds.m_sndPlayerDie,     std::string(proofps_dd::GAME_AUDIO_DIR) + "radio/die1.wav");

    static constexpr float SndPlayerDieDistMin = 6.f;
    static constexpr float SndPlayerDieDistMax = 12.f;

    m_sounds.m_sndPlayerDie.set3dMinMaxDistance(SndPlayerDieDistMin, SndPlayerDieDistMax);
    m_sounds.m_sndPlayerDie.set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);

    getConsole().OOOLn("PRooFPSddPGE::onGameInitialized() done!");

    getInput().getMouse().SetCursorPos(
        getPure().getWindow().getX() + getPure().getWindow().getWidth()/2,
        getPure().getWindow().getY() + getPure().getWindow().getHeight()/2);
    getPure().getWindow().SetCursorVisible(false);
    
    m_fps_lastmeasure = GetTickCount();
    m_fps = GAME_MAXFPS_DEF;

    return true;
}

/**
    Game logic right before the engine would do anything.
    This is invoked at the very beginning of the main game loop, before processing window messages and incoming network packets.
*/
void proofps_dd::PRooFPSddPGE::onGameFrameBegin()
{
    //for (auto& playerPair : m_mapPlayers)
    //{
    //    auto& player = playerPair.second;
    //    if (getNetwork().isServer())
    //    {
    //        // before v0.7 this controlled setJumpAllowed(), however since v0.7 we have a more sophisticated way based on the
    //        // physics calculations in Physics::serverGravity().
    //        //if (player.getPos().getNew().getY() != player.getPos().getOld().getY())
    //        //{   // still could fall in previous frame, so jumping is still disallowed ...
    //        //    player.setJumpAllowed(false);
    //        //}
    //        //else
    //        //{
    //        //    player.setJumpAllowed(true);
    //        //}
    //    }
    //}
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
    // PURE calls drawDearImGuiCb() in every frame.
    // If we are in main menu, then basically GUI::drawWindowForMainMenu() is operating, since drawDearImGuiCb() is calling it back in every frame.
    // In that case, there is not much to do here in onGameRunning().
    // We expect the GUI to set MainMenuState::None as soon as the user wants to enter a game (either by creating or joining).

    // if we want to handle Create or Join event initiated from Main menu here, we should maintain a prevMenuState also, then
    // the condition would look like this:
    // if ((m_gui.getMainMenuState() == proofps_dd::GUI::MainMenuState::None) && (m_gui.getPrevMenuState() != proofps_dd::GUI::MainMenuState::None))
    // This way we could decouple this logic from GUI code.

    if (m_gui.getMainMenuState() == proofps_dd::GUI::MainMenuState::None)
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
                serverSetFallDamageMultiplier(m_config.getFallDamageMultiplier());
                serverUpdateWeapons(*GameMode::getGameMode());
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
                (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_timeConnectionStateChangeInitiated).count()
                    >= static_cast<std::chrono::seconds::rep>(m_config.getReconnectDelaySeconds())))
            {
                if (connect())
                {
                    m_gui.getXHair()->showInCenter();
                    //m_gui.getMinimap()->show();
                    resetSendClientUpdatesCounter(m_config);
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
    }
    else
    {
        getAudio().stopSoundInstance(m_sounds.m_sndEndgameMusicHandle);
        if (!getAudio().getAudioEngineCore().isValidVoiceHandle(m_sounds.m_sndMenuMusicHandle))
        {
            m_sounds.m_sndMenuMusicHandle = getAudio().playSound(m_sounds.m_sndMenuMusic);
        }
    }// m_gui.getMainMenuState()

    updateFramesPerSecond(window);

    m_durations.m_nFullOnGameRunningDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeOnGameRunningStart).count();
}

/**
    Called when a new network packet is received.

    @return True on successful packet handling, false on serious error that should result in terminating the application.
*/
bool proofps_dd::PRooFPSddPGE::onPacketReceived(const pge_network::PgePacket& pkt)
{
    assert(GameMode::getGameMode());

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
    bool bRet;

    const pge_network::PgePktId& pgePktId = pge_network::PgePacket::getPacketId(pkt);
    switch (pgePktId)
    {
    case pge_network::MsgUserConnectedServerSelf::id:
        bRet = handleUserConnected(
            pge_network::PgePacket::getServerSideConnectionHandle(pkt),
            pge_network::PgePacket::getMessageAsUserConnected(pkt),
            getConfigProfiles(),
            m_config,
            m_cbDisplayMapLoadingProgressUpdate);
        break;
    case pge_network::MsgUserDisconnectedFromServer::id:
        bRet = handleUserDisconnected(
            pge_network::PgePacket::getServerSideConnectionHandle(pkt),
            pge_network::PgePacket::getMessageAsUserDisconnected(pkt),
            *GameMode::getGameMode());
        break;
    case pge_network::MsgApp::id:
    {
        // for now we support only 1 app msg per pkt
        assert(pge_network::PgePacket::getMessageAppCount(pkt) == 1);
        assert(pge_network::PgePacket::getMessageAppsTotalActualLengthBytes(pkt) > 0);  // for now we dont have empty messages
        
        // TODO: here we will need to iterate over all app msg but for now there is only 1 inside!

        const proofps_dd::PRooFPSappMsgId& proofpsAppMsgId = static_cast<proofps_dd::PRooFPSappMsgId>(pge_network::PgePacket::getMsgAppIdFromPkt(pkt));

        //if (m_nServerSideConnectionHandle == pge_network::PgePacket::getServerSideConnectionHandle(pkt))
        //{
        //    const auto playerIt = m_mapPlayers.find(m_nServerSideConnectionHandle);
        //    if (playerIt != m_mapPlayers.end())
        //    {
        //        const Player& player = playerIt->second;
        //        if (player.getHealth() == 0)
        //        {
        //            //getConsole().EOLn("Got message during being dead: %u", proofpsAppMsgId);
        //            if (proofpsAppMsgId == proofps_dd::MsgCurrentWpnUpdateFromServer::id)
        //            {
        //                getConsole().EOLn("Wpn update");
        //            }
        //        }
        //    }
        //}

        switch (proofpsAppMsgId)
        {
        case proofps_dd::MsgServerInfoFromServer::id:
            bRet = m_config.clientHandleServerInfoFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgServerInfoFromServer>(pkt));
            break;
        case proofps_dd::MsgGameSessionStateFromServer::id:
            bRet = clientHandleGameSessionStateFromServer(
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgGameSessionStateFromServer>(pkt));
            break;
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
        case proofps_dd::MsgUserNameChangeAndBootupDone::id:
            bRet = handleUserNameChange(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserNameChangeAndBootupDone>(pkt),
                m_config,
                getConfigProfiles());
            break;
        case proofps_dd::MsgUserCmdFromClient::id:
            bRet = serverHandleUserCmdMoveFromClient(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt),
                *GameMode::getGameMode(),
                *this);
            break;
        case proofps_dd::MsgUserUpdateFromServer::id:
            assert(m_gui.getXHair());
            bRet = handleUserUpdateFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserUpdateFromServer>(pkt),
                *m_gui.getXHair(),
                m_config,
                *GameMode::getGameMode(),
                getSmokePool());
            break;
        case proofps_dd::MsgBulletUpdateFromServer::id:
            bRet = handleBulletUpdateFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgBulletUpdateFromServer>(pkt),
                cameraGetShakeForce());
            break;
        case proofps_dd::MsgMapItemUpdateFromServer::id:
            // TODO: this check should not be here, in future a big packet table should solve this as well:
            // https://github.com/proof88/PRooFPS-dd/issues/220
            if (getNetwork().isServer())
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): server received, CANNOT HAPPEN!", __func__);
                assert(false);
                return false;
            }
            bRet = m_maps.handleMapItemUpdateFromServer(
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
        case proofps_dd::MsgPlayerEventFromServer::id:
            bRet = handlePlayerEventFromServer(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgPlayerEventFromServer>(pkt),
                cameraGetShakeForce(),
                m_config,
                getConfigProfiles());
            break;
        case proofps_dd::MsgUserInGameMenuCmd::id:
            bRet = serverHandleUserInGameMenuCmd(
                pge_network::PgePacket::getServerSideConnectionHandle(pkt),
                pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserInGameMenuCmd>(pkt),
                m_config,
                getConfigProfiles());
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
    m_mapPlayers.clear();           // Dtors of Player instances will be implicitly called
    deleteWeaponHandlingAll(true);  // Dtors of Bullet instances will be implicitly called
    m_maps.shutdown();
    m_gui.shutdown();
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
    m_maps.updateVisibilitiesForRenderer();
}

bool proofps_dd::PRooFPSddPGE::hasValidConnection() const
{
    // This is why it is very important to clear out m_nServerSideConnectionHandle when we are disconnecting:
    // to make sure this conditiona fails, so the main loop is aware of not having a valid connection.
    // And it is also very important to clear m_mapPlayers in that case, otherwise 2 bugs would occur:
    // - server: it would still be able to find itself in the map, obviously since 0 is its own handle anyway;
    // - client: it would still be able to find the server in the map, which is a false conclusion of having a valid connection.
    // We must also wait for a non-empty player name because it means that all 3 must-have messages were processed properly:
    // MsgUserConnected, MsgUserSetup, MsgUserNameChangeAndBootupDone.
    // A properly set unique name is important for gamemode. And handleUserUpdateFromServer() would also update gamemode by valid user name.
    const auto itPlayer = m_mapPlayers.find(m_nServerSideConnectionHandle);
    return (itPlayer != m_mapPlayers.end()) && (!itPlayer->second.getName().empty());
}

bool proofps_dd::PRooFPSddPGE::connect()
{
    if (!initializeWeaponHandling(getConfigProfiles()))
    {
        getConsole().EOLnOO("ERROR: initializeWeaponHandling() failed!");
        return false;
    }

    bool bRet;
    const std::string sAppVersion = std::string(GAME_NAME) + " " + std::string(GAME_VERSION);
    if (getNetwork().isServer())
    {
        m_gui.textForNextFrame("Starting Server ...", 200, getPure().getWindow().getClientHeight() / 2);
        getPure().getRenderer()->RenderScene();

        bRet = getNetwork().getServer().startListening(sAppVersion);
        if (!bRet)
        {
            getConsole().EOLn("ERROR: Server has FAILED to start listening!");
        }
    }
    else
    {
        std::string sIp = "127.0.0.1";
        if (!getConfigProfiles().getVars()[Networking::szCVarClServerIp].getAsString().empty())
        {
            sIp = getConfigProfiles().getVars()[Networking::szCVarClServerIp].getAsString();
            getConsole().OLn("IP from config: %s", sIp.c_str());
        }

        m_gui.textForNextFrame("Connecting to Server @ " + sIp + " ...", 200, getPure().getWindow().getClientHeight() / 2);
        getPure().getRenderer()->RenderScene();

        bRet = getNetwork().getClient().connectToServer(sIp, sAppVersion);
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
        "Thinking ..." :
        "Thinking ... Reason: " + sExtraDebugText;
    m_gui.textForNextFrame(sPrintText, 200, getPure().getWindow().getClientHeight() / 2);
    m_gui.hideRespawnTimer();
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
    m_gui.hideGameObjectives();
    m_gui.getDeathKillEvents()->clear();
    m_gui.getItemPickupEvents()->clear();
    m_gui.getPlayerHpChangeEvents()->clear();
    m_gui.getPlayerApChangeEvents()->clear();
    m_gui.getPlayerAmmoChangeEvents()->clear();
    m_gui.getXHair()->hide();
    m_gui.getMinimap()->hide();
    m_gui.getSlidingProof88Laugh().hide(getAudio(), true /* forceStopAudio */);
    for (auto& connHandlePlayerPair : m_mapPlayers)
    {
        connHandlePlayerPair.second.forceDeactivateCurrentInventoryItem();
        connHandlePlayerPair.second.getObject3D()->Hide();
        if (connHandlePlayerPair.second.getWeaponManager().getCurrentWeapon())
        {
            connHandlePlayerPair.second.getWeaponManager().getCurrentWeapon()->getObject3D().Hide();
        }
    }

    m_config.setServerInfoNotReceived();
    deleteWeaponHandlingAll(
        false /* no need for bulletpool dealloc, it is unnecessary and slow anyway, and if we are changing map, alloc again will be slow too*/);
    m_maps.unload();

    if (bExitFromGameSession)
    {
        m_gui.resetMenuStates(bExitFromGameSession);
    }
}

/**
    Only server executes this.
    Good for either dedicated- or listen- server.
*/
void proofps_dd::PRooFPSddPGE::mainLoopConnectedServerOnlyOneTick(
    const long long& /*durElapsedMicrosecs*/)
{
    // @TICK-RATE START

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
    assert(GameMode::getGameMode());
    assert(m_gui.getXHair());
    const bool bWin = GameMode::getGameMode()->serverCheckAndUpdateWinningConditions(getNetwork());
    for (unsigned int iPhyIter = 1; iPhyIter <= nPhysicsIterationsPerTick; iPhyIter++)
    {
        // @PHYSICS-RATE START

        if (!bWin)
        {
            const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
            serverGravity(*m_gui.getXHair(), m_config.getPhysicsRate(), *GameMode::getGameMode());
            serverPlayerCollisionWithWalls(m_config.getPhysicsRate(), *m_gui.getXHair(), *GameMode::getGameMode(), cameraGetShakeForce());
            m_durations.m_nGravityCollisionDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
        }
        serverUpdateBulletsAndHandleHittingWallsAndPlayers(*GameMode::getGameMode(), *m_gui.getXHair(), m_config.getPhysicsRate(), cameraGetShakeForce());
        serverHandleBulletsVsBullets(*GameMode::getGameMode(), *m_gui.getXHair(), m_config.getPhysicsRate(), cameraGetShakeForce());
        serverUpdateExplosions(*GameMode::getGameMode(), m_config.getPhysicsRate());
        updateSmokes(*GameMode::getGameMode(), m_config.getPhysicsRate());
        serverPickupAndRespawnItems();
        serverUpdatePlayersOldValues(m_config, getSmokePool());

        // @PHYSICS-RATE END
    }  // for iPhyIter
    serverUpdateRespawnTimers(m_config, *GameMode::getGameMode(), m_durations);
    serverSendUserUpdates(getConfigProfiles(), m_config, m_durations);

    // @TICK-RATE END
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
    GameMode* const gm = GameMode::getGameMode();
    assert(gm);

    const unsigned int nPhysicsIterationsPerTick = std::max(1u, m_config.getPhysicsRate() / m_config.getTickRate());
    for (unsigned int iPhyIter = 1; iPhyIter <= nPhysicsIterationsPerTick; iPhyIter++)
    {
        clientUpdateBullets(m_config.getPhysicsRate());
        clientUpdateExplosions(*gm, m_config.getPhysicsRate());
        updateSmokes(*gm, m_config.getPhysicsRate());
    }
}

/**
    Both clients and listen-server executes this.
    Called back by PRooFPSddPGE::onGameRunning() in every frame.
    Note that periodical update of Dear ImGui elements shall be done in proofps_dd::GUI::drawDearImGuiCb() instead.

    Dedicated server won't need this.
*/
void proofps_dd::PRooFPSddPGE::mainLoopConnectedShared(PureWindow& window)
{
    std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
    assert(m_gui.getXHair());
    Player& player = m_mapPlayers.at(m_nServerSideConnectionHandle); // cannot throw, because of bValidConnection
    if (window.isActive())
    {
        if (clientHandleInputWhenConnectedAndSendUserCmdMoveToServer(
            *GameMode::getGameMode(), player, *m_gui.getXHair(), m_config.getTickRate(), m_config.getClientUpdateRate(), m_config.getPhysicsRate(), *this
        ) == proofps_dd::InputHandling::PlayerAppActionRequest::Exit)
        {
            disconnect(true);
            return;
        }
    } // window is active
    m_durations.m_nActiveWindowStuffDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

    // after we have adjusted xhair 2D coords based on user input above, update 3D coords for this frame
    // TODO: get rid of this by changing XHair design as explained within the function itself!
    m_gui.getXHair()->updateUnprojectedCoords(getPure().getCamera());
    // TODO: basically 1 single public setPosVec() should be added to XHair, which will automatically invoke these functions too!
    // Everywhere objXHair.getPosVec().Set() should be replaced by a new function: m_gui.getXHair().setPosVec().
    m_gui.getXHair()->updateVisuals();

    m_gui.updateNonDearImGuiElements();

    //if (!isServer())
    //{
    //    // debugging how much the client is aware of some of its states
    //    static int i = 0;
    //    i++;
    //    
    //    if (i > 100)
    //    {
    //        i = 0;
    //        getConsole().EOLn(
    //            "%s(): bMoving: %b, isSomersaulting(): %b, isInAir(): %b, (getStrafe() != Strafe::NONE): %b, (getPos().getOld() != getPos().getNew()): %b, bRunning: %b, bDuck: %b, recoil: %f",
    //            __func__,
    //            player.isMoving(),
    //            player.isSomersaulting(),
    //            player.isInAir(),
    //            (player.getStrafe() != Strafe::NONE),
    //            (player.getPos().getOld() != player.getPos().getNew()),
    //            player.isRunning(),
    //            player.getCrouchStateCurrent(),
    //            player.getWeaponManager().getCurrentWeapon()->getMomentaryRecoilMultiplier());
    //    }
    //}
    
    cameraUpdatePosAndAngle(
        getAudio(),
        m_mapPlayers,
        player,
        *m_gui.getXHair(),
        m_fps,
        m_config.getCameraFollowsPlayerAndXHair(),
        m_config.getCameraTilting(),
        m_config.getCameraRolling());
    
    updatePlayersVisuals(m_config, *GameMode::getGameMode()); // maybe we should do this per-tick instead of per-frame in the future
    updateAudioVisualsForGameModeShared();
    m_maps.update(m_fps, *player.getObject3D());
    m_maps.updateVisibilitiesForRenderer();

    GameMode* const gm = GameMode::getGameMode();
    assert(gm);
    if (!getNetwork().isServer())
    {
        gm->clientTickUpdateWinningConditions();
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
        if (clientHandleInputWhenDisconnectedFromServer() == proofps_dd::InputHandling::PlayerAppActionRequest::Exit)
        {
            disconnect(true);
            return;
        }
    } // window is active
}

void proofps_dd::PRooFPSddPGE::updateFramesPerSecond(PureWindow& window)
{
    // this is horrible that FPS measuring is still not available from outside of PURE .........
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

// TODO: RFR: Shall be moved this to where we are able to access: playerhandling, maps, gui, gamemode, and
// gui shall be able to invoke it. For now we just register this as callback in gui so gui can call it back.
void proofps_dd::PRooFPSddPGE::serverRestartGame()
{
    assert(getNetwork().isServer());
    
    for (auto& playerPair : m_mapPlayers)
    {
        serverRespawnPlayer(playerPair.second, true, m_config);
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
            // just to avoid unnecessary server -> client network traffic
            continue;
        }

        mapItem.unTake();

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

    m_gui.hideGameObjectives();
    m_gui.getDeathKillEvents()->clear();
    getAudio().stopSoundInstance(m_sounds.m_sndEndgameMusicHandle); // server stops it here, clients stop it when they process MsgGameSessionStateFromServer
    m_gui.getServerEvents()->addGameRestartedEvent(); // server adds it here, clients add when they process MsgGameSessionStateFromServer
    GameMode::getGameMode()->restartWithoutRemovingPlayers(getNetwork());
    m_gui.showMandatoryGameModeConfigMenuOnlyIfGameModeIsNotYetConfiguredForCurrentPlayer(); // server does it here, clients add when they process MsgGameSessionStateFromServer 
}

void proofps_dd::PRooFPSddPGE::updateAudioVisualsForGameModeShared()
{
    // both server and client instances execute this
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    GameMode* const gm = GameMode::getGameMode();
    assert(gm);

    if (GameMode::getGameMode()->hasJustBeenWonThisTick())
    {
        // come here only once
        //getConsole().EOLn("PRooFPSddPGE::%s() detected game has just been won in this frame or tick", __func__);

        if (!getAudio().getAudioEngineCore().isValidVoiceHandle(m_sounds.m_sndEndgameMusicHandle))
        {
            m_sounds.m_sndEndgameMusicHandle = getAudio().playSound(m_sounds.m_sndEndgameMusic);
        }
        m_gui.hideInGameMenu();
        m_gui.showGameObjectives();
        m_gui.getXHair()->hide();
        m_gui.getMinimap()->hide();
        for (auto& playerPair : m_mapPlayers)
        {
            playerPair.second.hide();
            playerPair.second.forceDeactivateCurrentInventoryItem();
        }
    }

    if (gm->isGameWon())
    {
        if (getNetwork().isServer())
        {
            const auto nSecsSinceWin = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - GameMode::getGameMode()->getWinTime()).count();
            if (nSecsSinceWin >= 60)
            {
                serverRestartGame();
            }
        }
    }

    m_durations.m_nUpdateGameModeDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void proofps_dd::PRooFPSddPGE::serverPickupAndRespawnItems()
{
    assert(GameMode::getGameMode());
    if (GameMode::getGameMode()->isGameWon())
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
            if (nSecsSinceTake < static_cast<std::chrono::seconds::rep>(MapItem::getItemRespawnTimeSecs(mapItem)))
            {
                continue;
            }

            mapItem.unTake();
            bSendItemUpdate = true;
        }
        else
        {
            for (auto& playerPair : m_mapPlayers)
            {
                auto& player = playerPair.second;
                const auto& playerConst = player;
                if ((playerConst.getHealth() <= 0) || (player.getRespawnFlag()))
                {
                    continue;
                }
                if (!GameMode::getGameMode()->isPlayerAllowedForGameplay(player))
                {
                    continue;
                }

                const PureObject3D* const plobj = player.getObject3D();

                // TODO: from performance perspective, maybe it would be better to check canTakeItem() first since that might be faster
                // decision than collision check ...
                // To avoid Z-fighting, players and items have different Z-coords. This makes Colliding() unable to work, so we use Colliding_NoZ().
                // And it is faster too, no use of Z anyway since theoretically players and items have the same Z.
                if (colliding_NoZ(*plobj, mapItem.getObject3D()))
                {
                    proofps_dd::MsgWpnUpdateFromServer::getAvailable(newPktWpnUpdate) = false;
                    if (player.canTakeItem(mapItem))
                    {
                        bool bHasJustBecomeAvailable = false;
                        player.takeItem(mapItem, newPktWpnUpdate, bHasJustBecomeAvailable);  // this also invokes mapItem.Take()
                        bSendItemUpdate = true;
                        // although item update is always sent, wpn update is sent only if takeItem() flipped the availability of the wpn,
                        // since it can happen the item is not weapon-related at all, or something else, anyway let takeItem() make the decision!
                        if (proofps_dd::MsgWpnUpdateFromServer::getAvailable(newPktWpnUpdate))
                        {
                            if (playerPair.second.getServerSideConnectionHandle() != pge_network::ServerConnHandle) // server doesnt send this to itself
                            {
                                getNetwork().getServer().send(newPktWpnUpdate, playerPair.second.getServerSideConnectionHandle());
                            }
                            else
                            {
                                // server handles pickup-induced switch here, clients do that in MsgWpnUpdateFromServer handler
                                // TODO: pWpnPicked maybe const!
                                auto* pWpnPicked = player.getWeaponInstanceByMapItemType(mapItem.getType());
                                if (pWpnPicked && player.getWeaponManager().getCurrentWeapon())
                                {
                                    handleAutoSwitchUponWeaponPickupShared(player, *player.getWeaponManager().getCurrentWeapon(), *pWpnPicked, bHasJustBecomeAvailable);
                                }
                                else
                                {
                                    getConsole().EOLn("PRooFPSddPGE::%s(): either current or picked wpn is nullptr around line %d!", __func__, __LINE__);
                                }
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

bool proofps_dd::PRooFPSddPGE::clientHandleGameSessionStateFromServer(const proofps_dd::MsgGameSessionStateFromServer& msg)
{
    /* this function should be in GameMode, however currently I cannot include PRooFPS-dd-packet.h in GameMode.h due to
       circular include dependency, thus here I'm manually passing values from the msg to GameMode */
    
    if (getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    assert(GameMode::getGameMode());
    GameMode::getGameMode()->clientReceiveAndUpdateWinningConditions(getNetwork(), msg.m_bGameSessionEnd);

    if (msg.m_bGameSessionEnd)
    {
        // just in case a client has just joined the server where game is already ended, do not show team selection even
        // if it was shown automatically in some previous frames as supposed to be by default
        m_gui.hideInGameMenu();
    }
    else
    {
        // Note: although clients stop endgame music here, starting the music is in common code in updateAudioVisualsForGameModeShared()
        getAudio().stopSoundInstance(m_sounds.m_sndEndgameMusicHandle);
        // !!! BADDESIGN !!!
        // GUI should be invoked in GameMode's restartWithoutRemovingPlayers(), however I cannot include GUI.h in GameMode now ...
        m_gui.hideGameObjectives();
        m_gui.getDeathKillEvents()->clear();
    }

    if (msg.m_bGameRestarted)
    {
        for (auto& playerPair : m_mapPlayers)
        {
            // server handled this in PlayerHandling::serverRespawnPlayer() already
            auto& player = playerPair.second;
            player.forceDeactivateCurrentInventoryItem();
        }
        getAudio().stopSoundInstance(m_sounds.m_sndEndgameMusicHandle);
        m_gui.getServerEvents()->addGameRestartedEvent();
        m_gui.showMandatoryGameModeConfigMenuOnlyIfGameModeIsNotYetConfiguredForCurrentPlayer();
    }
    
    return true;
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
        getConsole().EOLn("PRooFPSddPGE::%s(): WA: connHandleServerSide: %u is already present in players list, allowed temporarily due to issue #268!",
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
            if (!proofps_dd::MsgUserNameChangeAndBootupDone::initPkt(
                newPktUserNameChange,
                pge_network::ServerConnHandle,
                true,
                getConfigProfiles().getVars()[Player::szCVarClName].getAsString()))
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

            // Now we should ask the server to set our wanted name,
            // this is how we actually signal the server that we have loaded all the stuff and are READY!
            // Until this point, server has been keeping us in a forever-invulnerable state, since we have been already visible for other players from the moment
            // of our connection has been established. This is NOT related to the "respawn invulnerability timer".
            // For this message, server will actually start the "respawn invulnerability timer" for us, since from NOW we are able to see ourselves on client-side!
            // See handleUserNameChange() also!
            pge_network::PgePacket newPktUserNameChange;
            if (!proofps_dd::MsgUserNameChangeAndBootupDone::initPkt(
                newPktUserNameChange,
                connHandleServerSide,
                false,
                getConfigProfiles().getVars()[Player::szCVarClName].getAsString()))
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
                return false;
            }
            getNetwork().getClient().send(newPktUserNameChange);
        }

        // at this point we can be sure we have the proper map loaded, camera must start from the center of the map
        cameraPositionToMapCenter();
        hideLoadingScreen();
        m_gui.getXHair()->showInCenter();
        m_gui.getMinimap()->show();
        m_gui.hideGameObjectives(); // just in case it would had stuck from a previous game session
        if (!GameMode::getGameMode()->isGameWon())
        {
            m_gui.showMandatoryGameModeConfigMenu();
        }
        
        getAudio().playSound(m_sounds.m_sndLetsgo);
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
                    if (!proofps_dd::MsgUserNameChangeAndBootupDone::initPkt(
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

                bool bSentImplicitExitSpectatorMode = false;
                if (GameMode::getGameMode()->isTeamBasedGame() &&
                    /* do not send team id if spectating because it would implicitly toggle spectator mode */
                    !it.second.isInSpectatorMode())
                {
                    bSentImplicitExitSpectatorMode = true;
                    pge_network::PgePacket pktPlayerEventTeamSelect;
                    proofps_dd::MsgPlayerEventFromServer::initPkt(
                        pktPlayerEventTeamSelect,
                        it.second.getServerSideConnectionHandle(),
                        PlayerEventId::TeamIdChanged,
                        static_cast<int>(it.second.getTeamId()));
                    getNetwork().getServer().send(pktPlayerEventTeamSelect, connHandleServerSide);
                }

                // send item availability BEFORE MsgUserUpdateFromServer because latter has the up-to-date getCurrentInventoryItemPower()
                if (it.second.hasJetLax())
                {
                    pge_network::PgePacket pktPlayerInventoryItemAvailable;
                    if (!proofps_dd::MsgPlayerEventFromServer::initPkt(
                        pktPlayerInventoryItemAvailable,
                        it.second.getServerSideConnectionHandle(),
                        PlayerEventId::ItemTake,
                        static_cast<int>(MapItemType::ITEM_JETLAX)))
                    {
                        getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                        assert(false);
                        continue;
                    }
                    getNetwork().getServer().send(pktPlayerInventoryItemAvailable, connHandleServerSide);
                }

                if (it.second.hasAntiGravityActive())
                {
                    pge_network::PgePacket pktPlayerInventoryItemActive;
                    if (!proofps_dd::MsgPlayerEventFromServer::initPkt(
                        pktPlayerInventoryItemActive,
                        it.second.getServerSideConnectionHandle(),
                        PlayerEventId::InventoryItemToggle,
                        static_cast<int>(MapItemType::ITEM_JETLAX),
                        true /* bSyncHistory */))
                    {
                        getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                        assert(false);
                        continue;
                    }
                    getNetwork().getServer().send(pktPlayerInventoryItemActive, connHandleServerSide);
                }

                pge_network::PgePacket newPktUserUpdate;
                if (!proofps_dd::MsgUserUpdateFromServer::initPkt(
                    newPktUserUpdate,
                    it.second.getServerSideConnectionHandle(),
                    it.second.getObject3D()->getPosVec().getX(),
                    it.second.getObject3D()->getPosVec().getY(),
                    it.second.getObject3D()->getPosVec().getZ(),
                    it.second.getObject3D()->getAngleVec().getY(),
                    it.second.getObject3D()->getAngleVec().getZ(),
                    it.second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().getZ(),
                    it.second.getWeaponManager().getCurrentWeapon()->getMomentaryAccuracy(it.second.isMoving(), it.second.isRunning(), it.second.getCrouchStateCurrent()),
                    it.second.getActuallyRunningOnGround(),
                    false /* TODO: why are we not sending out the current crouch state??? */,
                    it.second.getSomersaultAngle(),
                    it.second.getArmor(),
                    it.second.getHealth(),
                    false /* bRespawn */,
                    it.second.getFrags(),
                    it.second.getDeaths(),
                    it.second.getSuicides(),
                    it.second.getFiringAccuracy(),
                    it.second.getShotsFiredCount(),
                    it.second.getInvulnerability(),
                    it.second.getCurrentInventoryItemPower()))
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
                    it.second.getWeaponManager().getCurrentWeapon()->getFilename(),
                    it.second.getWeaponManager().getCurrentWeapon()->getState().getNew()))
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                    assert(false);
                    continue;
                }
                getNetwork().getServer().send(pktWpnUpdateCurrent, connHandleServerSide);

                // by default spectator mode is enabled for players, send packet to toggle it
                // if a player is not spectating
                if (!it.second.isInSpectatorMode() && !bSentImplicitExitSpectatorMode)
                {
                    bSentImplicitExitSpectatorMode = true;
                    pge_network::PgePacket pktPlayerEventToggleSpectator;
                    proofps_dd::MsgPlayerEventFromServer::initPkt(
                        pktPlayerEventToggleSpectator,
                        it.second.getServerSideConnectionHandle(),
                        PlayerEventId::ToggledSpectatorMode);
                    getNetwork().getServer().send(pktPlayerEventToggleSpectator, connHandleServerSide);
                }
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

    assert(m_gui.getItemPickupEvents());
    const auto insertRes = m_mapPlayers.insert(
        {
            connHandleServerSide,
            Player(
                getAudio(),
                getConfigProfiles(),
                getBullets(),
                *m_gui.getItemPickupEvents(), *m_gui.getPlayerInventoryChangeEvents(), *m_gui.getPlayerAmmoChangeEvents(),
                getPure(), getNetwork(), connHandleServerSide, msg.m_szIpAddress)
        }); // TODO: emplace_back()
    if (!insertRes.second)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to insert user %u into map!", __func__, connHandleServerSide);
        assert(false);
        return false;
    }
    Player& insertedPlayer = insertRes.first->second;

    // We dont hide player here:
    // updatePlayersVisuals() is responsible for player visibility updates, thus also for hiding player not allowed for gameplay!

    if (getNetwork().isServer() && msg.m_bCurrentClient)
    {
        // server player is obviously not expecting after bootup delayed update from server
        insertedPlayer.setExpectingAfterBootUpDelayedUpdate(false);
    }

    float fMaxBulletRatePerSec = 0.f;
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

            if (m_mapPlayers.size() == 1)
            {
                // this is where we make sure any kind of explosion has the proper reference created
                if (loadedWpn->getVars()["damage_area_size"].getAsFloat() > 0.f)
                {
                    // cannot be empty, Weapon ctor makes it sure!
                    assert(!loadedWpn->getVars()["damage_area_gfx_obj"].getAsString().empty());
                    assert(!loadedWpn->getVars()["damage_area_snd"].getAsString().empty());
                    Explosion::updateReferenceExplosions(
                        *this,
                        loadedWpn->getVars()["damage_area_gfx_obj"].getAsString(),
                        loadedWpn->getVars()["damage_area_snd"].getAsString());
                }

                // bullet pool capacity is determined using the fastest bullet firing weapon
                if (loadedWpn->getBulletRate() > fMaxBulletRatePerSec)
                {
                    fMaxBulletRatePerSec = loadedWpn->getBulletRate();
                }

                // just log some weapon info only when 1st player is created to avoid spamming
                getConsole().OLn("PRooFPSddPGE::%s(): weapon %s br: %f, dpfr: %f, fr: %f, dpsr: %f",
                    __func__,
                    loadedWpn->getFilename().c_str(),
                    loadedWpn->getBulletRate(),
                    loadedWpn->getDamagePerFireRating(),
                    loadedWpn->getFiringRate(),
                    loadedWpn->getDamagePerSecondRating());
            }
        }
    }

    if (getBullets().capacity() == 0)
    {
        // considering a bullet lifetime up to 10 secs, a player can have up to 10 * fMaxBulletRatePerSec active bullets on the map, and
        // max expected number of players is coming from config
        getConsole().OLn("PRooFPSddPGE::%s(): highest bullet rate: %f", __func__, fMaxBulletRatePerSec);
        CConsole::getConsoleInstance().SetLoggingState(getBullets().getLoggerModuleName(), true);
        getBullets().reserve("bullets", static_cast<size_t>(
            std::ceil(getConfigProfiles().getVars()[Player::szCVarPlayersMax].getAsUInt() * 10 * fMaxBulletRatePerSec)), getPure()
        );
        CConsole::getConsoleInstance().SetLoggingState(getBullets().getLoggerModuleName(), false);
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

    // and this is when the "default available weapon" concept is not necessarily correct because from v0.3.0 knife also becomes available by default:
    Weapon* const wpnKnife = insertedPlayer.getWeaponManager().getWeaponByFilename("knife.txt");
    wpnKnife->SetAvailable(true);

    // Note that this is a waste of resources this way.
    // Because clients also store the full weapon instances for themselves, even though they dont use weapon cvars at all!
    // Task: On the long run, there should be a WeaponProxy or WeaponClient or something for the clients which are basically
    // just the image for their current weapon.

    return true;
}  // handleUserSetupFromServer()

bool proofps_dd::PRooFPSddPGE::handleMapChangeFromServer(pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/, const proofps_dd::MsgMapChangeFromServer& msg)
{
    getConsole().OLn("PRooFPSddPGE::%s(): map: %s", __func__, msg.m_szMapFilename);

    // TODO: make sure received map name is properly null-terminated! someone else could had sent that, e.g. malicious server

    getAudio().stopSoundInstance(m_sounds.m_sndEndgameMusicHandle);
    if (!getAudio().getAudioEngineCore().isValidVoiceHandle(m_sounds.m_sndMenuMusicHandle))
    {
        m_sounds.m_sndMenuMusicHandle = getAudio().playSound(m_sounds.m_sndMenuMusic);
    }

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
    cameraPositionToMapCenter();
    hideLoadingScreen();
    m_gui.getXHair()->showInCenter();
    m_gui.getXHair()->handleMagLoaded();
    m_gui.getMinimap()->show();

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
    m_fps = GAME_MAXFPS_DEF;

    return true;
}  // handleMapChangeFromServer()
