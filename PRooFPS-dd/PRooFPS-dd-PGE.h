#pragma once

/*
    ###################################################################################
    PRooFPSddPGE.h
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include <chrono>  // requires cpp11

#include "PGE.h"
#include "Pure/include/external/Object3D/PureObject3DManager.h"

#include "CameraHandling.h"
#include "Config.h"
#include "Consts.h"
#include "Durations.h"
#include "GameMode.h"
#include "GUI.h"
#include "InputHandling.h"
#include "Maps.h"
#include "Networking.h"
#include "Physics.h"
#include "Player.h"
#include "PlayerHandling.h"
#include "PRooFPS-dd-packet.h"
#include "Sounds.h"
#include "WeaponHandling.h"

namespace proofps_dd
{

    /**
        The customized game engine class.
        This handles the game logic.
        Singleton.
    */
    class PRooFPSddPGE final :
        public PGE,
        protected proofps_dd::CameraHandling,
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

        proofps_dd::Config& m_config;
        proofps_dd::GUI& m_gui;
        proofps_dd::GameMode* m_gameMode;
        proofps_dd::DeathMatchMode* m_deathMatchMode;
        
        /** Whenever we initiate a network connection or disconnection, we save the timestamp here.
            This is needed because then in the main game loop we can check the amount of time elapsed
            and then try reconnect automatically after a specific time. */
        std::chrono::time_point<std::chrono::steady_clock> m_timeConnectionStateChangeInitiated;
        std::chrono::time_point<std::chrono::steady_clock> m_timeLastPrintWaitConnection;

        Maps m_maps;
        std::function<void(int)> m_cbDisplayMapLoadingProgressUpdate;

        std::chrono::time_point<std::chrono::steady_clock> m_timeSimulation;          /**< For stepping the time ahead in 1 single tick. */

        float m_fps;
        unsigned int m_fps_counter;
        unsigned long m_fps_lastmeasure;
        bool m_bFpsFirstMeasure;

        PureObject3D* m_pObjXHair;
        bool m_bWon;

        std::map<pge_network::PgeNetworkConnectionHandle, Player> m_mapPlayers;  /**< Connected players, used by both server and clients.
                                                                                      Key is server-side connection handle. */

        proofps_dd::Durations m_durations;
        proofps_dd::Sounds m_sounds;

        // ---------------------------------------------------------------------------

        void showLoadingScreen(int nProgress);
        void hideLoadingScreen();
        void showXHairInCenter();

        bool hasValidConnection() const;
        bool connect();
        void disconnect(bool bExitFromGameSession, const std::string& sExtraDebugText = "");

        void mainLoopConnectedServerOnlyOneTick(
            const long long& durElapsedMicrosecs);                      /**< Only server executes this. */
        void mainLoopConnectedClientOnlyOneTick(
            const long long& durElapsedMicrosecs);                      /**< Only client executes this. */
        void mainLoopConnectedShared(
            PureWindow& window);                                        /**< Both clients and listen-server executes this. */
        void mainLoopDisconnectedShared(
            PureWindow& window);                                        /**< Both clients and listen-server executes this. */

        void updateFramesPerSecond(PureWindow& window);
        void LoadSound(SoLoud::Wav& snd, const char* fname);
        void RestartGame();
        void UpdateGameMode();

        void serverPickupAndRespawnItems();

        bool handleUserSetupFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserSetupFromServer& msg);

        bool handleMapChangeFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgMapChangeFromServer& msg);
        bool handleMapItemUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgMapItemUpdateFromServer& msg);

    }; // class PRooFPSddPGE

} // namespace proofps_dd
