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

#include "../../../PGE/PGE/PGE.h"
#include "../../../PGE/PGE/Pure/include/external/Object3D/PureObject3DManager.h"

#include "Consts.h"
#include "Durations.h"
#include "GameMode.h"
#include "InputHandling.h"
#include "Maps.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"

namespace proofps_dd
{

    /**
        The customized game engine class. This handles the game logic. Singleton.
    */
    class PRooFPSddPGE final :
        public PGE,
        protected proofps_dd::InputHandling
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

        int m_fps, m_fps_counter;               /* fps méréséhez segédváltozók */
        unsigned int m_fps_lastmeasure;         /* - || - */
        unsigned int m_fps_ms;                  /* - || - */

        PureObject3D* m_pObjXHair;
        bool m_bWon;
        float m_fCameraMinY;

        pge_network::PgeNetworkConnectionHandle m_nServerSideConnectionHandle;   /**< Server-side connection handle received from server in PgePktUserConnected
                                                                                      (server instance also receives this from itself).
                                                                                      Server doesn't have a connection to itself, so it uses default 0 (invalid) handle. */
        std::map<pge_network::PgeNetworkConnectionHandle, Player> m_mapPlayers;  /**< Connected players, used by both server and clients.
                                                                                      Key is server-side connection handle. */

        SoLoud::Wav m_sndLetsgo;
        SoLoud::Wav m_sndReloadStart;
        SoLoud::Wav m_sndReloadFinish;
        SoLoud::Wav m_sndShootPistol;
        SoLoud::Wav m_sndShootMchgun;
        SoLoud::Wav m_sndShootDryPistol;
        SoLoud::Wav m_sndShootDryMchgun;
        SoLoud::Wav m_sndChangeWeapon;
        SoLoud::Wav m_sndPlayerDie;

        proofps_dd::Durations m_durations;

        // ---------------------------------------------------------------------------

        bool hasValidConnection() const;
        bool isMyConnection(const pge_network::PgeNetworkConnectionHandle& connHandleServerSide) const;

        void LoadSound(SoLoud::Wav& snd, const char* fname);
        void Text(const std::string& s, int x, int y) const;
        void AddText(const std::string& s, int x, int y) const;
        void CameraMovement(int fps, Player& player);
        void Gravity(int fps);
        bool Colliding(const PureObject3D& a, const PureObject3D& b);
        bool Colliding2(
            float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
            float o2px, float o2py, float o2pz, float o2sx, float o2sy, float o2sz);
        bool Colliding2_NoZ(
            float o1px, float o1py, float o1sx, float o1sy,
            float o2px, float o2py, float o2sx, float o2sy);
        bool Colliding3(
            const PureVector& vecPosMin, const PureVector& vecPosMax,
            const PureVector& vecObjPos, const PureVector& vecObjSize);
        void PlayerCollisionWithWalls(bool& won);
        void UpdateWeapons();
        void UpdateBullets();
        void SendUserUpdates();
        void HandlePlayerDied(Player& player);
        void HandlePlayerRespawned(Player& player);
        void ServerRespawnPlayer(Player& player, bool restartGame);
        void RestartGame();
        void UpdateRespawnTimers();
        void UpdateGameMode();
        void PickupAndRespawnItems();
        void genUniqueUserName(char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength]) const;
        void WritePlayerList();
        bool handleUserSetup(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserSetup& msg);
        bool handleUserConnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserConnected& msg);
        bool handleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnected& msg);
        bool handleUserCmdMove(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserCmdMove& msg);
        bool handleUserUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserUpdate& msg);
        bool handleBulletUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgBulletUpdate& msg);
        bool handleMapItemUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgMapItemUpdate& msg);
        bool handleWpnUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgWpnUpdate& msg);
        bool handleWpnUpdateCurrent(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgWpnUpdateCurrent& msg);

    }; // class PRooFPSddPGE

} // namespace proofps_dd
