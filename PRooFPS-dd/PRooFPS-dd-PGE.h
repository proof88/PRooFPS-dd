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

#include "../../PGE/PGE/PGE.h"
#include "../../PGE/PGE/Pure/include/external/Object3D/PureObject3DManager.h"

#include "Consts.h"
#include "Durations.h"
#include "GameMode.h"
#include "InputHandling.h"
#include "Maps.h"
#include "Networking.h"
#include "Physics.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"
#include "Sounds.h"
#include "UserInterface.h"
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
        protected virtual proofps_dd::UserInterface,
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

        proofps_dd::GameMode* m_gameMode;
        proofps_dd::DeathMatchMode* m_deathMatchMode;
        std::string m_sServerMapFilenameToLoad;
        Maps m_maps;

        std::chrono::time_point<std::chrono::steady_clock> timeLastOnGameRunning;
        std::chrono::time_point<std::chrono::steady_clock> timeSimulation;
        unsigned int m_nTickrate;

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

        bool hasValidConnection() const;
        void mainLoopServerOnlyOneTick(
            std::chrono::steady_clock::time_point& timeStart,
            const long long& durElapsedMicrosecs);                      /**< Only server executes this. */
        void mainLoopShared(
            std::chrono::steady_clock::time_point& timeStart,
            PureWindow& window);                                 /**< Both clients and listen-server executes this. */
        void updateFramesPerSecond(PureWindow& window);
        void LoadSound(SoLoud::Wav& snd, const char* fname);
        void CameraMovement(Player& player);
        void serverSendUserUpdates();
        void RestartGame();
        void serverUpdateRespawnTimers();
        void UpdateGameMode();
        void serverPickupAndRespawnItems();
        void genUniqueUserName(char szNewUserName[proofps_dd::MsgUserSetupFromServer::nUserNameMaxLength]) const;
        void WritePlayerList();
        bool handleUserSetup(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserSetupFromServer& msg);
        bool handleUserConnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserConnected& msg);
        bool handleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnected& msg);
        bool handleUserUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserUpdateFromServer& msg);
        bool handleMapItemUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgMapItemUpdateFromServer& msg);

    }; // class PRooFPSddPGE

} // namespace proofps_dd
