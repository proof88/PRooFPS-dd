#pragma once

/*
    ###################################################################################
    PRooFPSddPGE.h
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <chrono>  // requires cpp11

#include "PGE.h"
#include "Pure/include/external/Object3D/PureObject3DManager.h"

#include "Consts.h"
#include "Durations.h"
#include "GameMode.h"
#include "GUI.h"
#include "InputHandling.h"
#include "Maps.h"
#include "Networking.h"
#include "Physics.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"
#include "Sounds.h"
#include "WeaponHandling.h"

namespace proofps_dd
{

    /**
        The customized game engine class. This handles the game logic. Singleton.
    */
    class PRooFPSddPGE final :
        public PGE,
        protected proofps_dd::InputHandling,
        protected virtual proofps_dd::PlayerHandling,
        protected proofps_dd::WeaponHandling
    {

    public:

        static PRooFPSddPGE* createAndGetPRooFPSddPGEinstance();
        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

    protected:

        explicit PRooFPSddPGE(const char* gametitle);  /**< This is the only usable ctor, this is used by the static createAndGet(). */
        virtual ~PRooFPSddPGE();

        PRooFPSddPGE(const PRooFPSddPGE&) = delete;
        PRooFPSddPGE& operator=(const PRooFPSddPGE&) = delete;
        PRooFPSddPGE(PRooFPSddPGE&&) = delete;
        PRooFPSddPGE&& operator=(PRooFPSddPGE&&) = delete;

        virtual bool onGameInitializing() override;               /**< Must-have minimal stuff before loading anything. */
        virtual bool onGameInitialized() override;                /**< Loading game content here. */
        virtual void onGameFrameBegin() override;                 /**< Game logic right before the engine would do anything. */
        virtual void onGameRunning() override;                    /**< Game logic for each frame. */
        virtual bool onPacketReceived(
            const pge_network::PgePacket& pkt) override;          /**< Called when a new network packet is received. */
        virtual void onGameDestroying() override;                 /**< Freeing up game content here. */

    private:

        bool m_bInMenu;

        proofps_dd::GUI m_gui;
        proofps_dd::GameMode* m_gameMode;
        proofps_dd::DeathMatchMode* m_deathMatchMode;
        std::string m_sServerMapFilenameToLoad;                   /**< We set this as soon as we get to know which map we should load. */
        
        /** Whenever we initiate a network connection or disconnection, we save the timestamp here.
            This is needed because then in the main game loop we can check the amount of time elapsed
            and then try reconnect automatically after a specific time. */
        std::chrono::time_point<std::chrono::steady_clock> m_timeConnectionStateChangeInitiated;
        std::chrono::time_point<std::chrono::steady_clock> m_timeLastPrintWaitConnection;
        unsigned int m_nSecondsReconnectDelay;

        Maps m_maps;
        std::function<void(int)> m_cbDisplayMapLoadingProgressUpdate;

        std::chrono::time_point<std::chrono::steady_clock> m_timeSimulation;          /**< For stepping the time ahead in 1 single tick. */
        unsigned int m_nTickrate;
        unsigned int m_nPhysicsRateMin;
        unsigned int m_nClientUpdateRate;
        bool m_bCamFollowsXHair;
        bool m_bCamTilting;

        float m_fps;
        unsigned int m_fps_counter;
        unsigned long m_fps_lastmeasure;
        bool m_bFpsFirstMeasure;

        PureObject3D* m_pObjXHair;
        bool m_bWon;
        float m_fCameraMinY;

        std::map<pge_network::PgeNetworkConnectionHandle, Player> m_mapPlayers;  /**< Connected players, used by both server and clients.
                                                                                      Key is server-side connection handle. */

        proofps_dd::Durations m_durations;
        proofps_dd::Sounds m_sounds;

        // ---------------------------------------------------------------------------

        void showLoadingScreen(int nProgress);
        void hideLoadingScreen();

        bool hasValidConnection() const;
        bool connect();
        void disconnect(const std::string& sExtraDebugText = "");
        void mainLoopServerOnlyOneTick(
            const long long& durElapsedMicrosecs);                      /**< Only server executes this. */
        void mainLoopClientOnlyOneTick(
            const long long& durElapsedMicrosecs);                      /**< Only client executes this. */
        void mainLoopShared(
            PureWindow& window);                                 /**< Both clients and listen-server executes this. */
        void updateFramesPerSecond(PureWindow& window);
        void LoadSound(SoLoud::Wav& snd, const char* fname);
        void CameraMovement(
            const Player& player,
            bool bCamFollowsXHair,
            bool bCamTilting);
        void serverSendUserUpdates();
        void RestartGame();
        void serverUpdateRespawnTimers();
        void UpdateGameMode();
        void serverPickupAndRespawnItems();
        void WritePlayerList();
        bool handleUserSetupFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserSetupFromServer& msg);
        bool handleUserNameChange(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserNameChange& msg);
        bool handleMapChangeFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgMapChangeFromServer& msg);
        bool handleUserConnected(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const pge_network::MsgUserConnectedServerSelf& msg);
        bool handleUserDisconnected(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const pge_network::MsgUserDisconnectedFromServer& msg);
        bool handleUserUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserUpdateFromServer& msg);
        bool handleMapItemUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgMapItemUpdateFromServer& msg);

    }; // class PRooFPSddPGE

} // namespace proofps_dd
