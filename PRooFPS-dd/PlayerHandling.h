#pragma once

/*
    ###################################################################################
    PlayerHandling.h
    PlayerHandling class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include <chrono>      // requires cpp11
#include <list>
#include <map>
#include <variant>     // requires cpp17
#include <vector>

#include "CConsole.h"

#include "PGE.h"
#include "Config/PgeOldNewValue.h"

#include "CameraHandling.h"
#include "Config.h"
#include "Consts.h"
#include "Durations.h"
#include "GameMode.h"
#include "GUI.h"
#include "Maps.h"
#include "Networking.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"
#include "Sounds.h"
#include "Strafe.h"

namespace proofps_dd
{
    class PlayerHandling :
        protected proofps_dd::Networking
    {
    public:

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        // TODO: pass "const proofps_dd::Config& config" also, too many functions need it anyway!
        PlayerHandling(
            PGE& pge,
            proofps_dd::Durations& durations,
            proofps_dd::GUI& gui,
            std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            proofps_dd::Maps& maps,
            proofps_dd::Sounds& sounds,
            proofps_dd::CameraHandling& camera);

        PlayerHandling(const PlayerHandling&) = delete;
        PlayerHandling& operator=(const PlayerHandling&) = delete;
        PlayerHandling(PlayerHandling&&) = delete;
        PlayerHandling&& operator=(PlayerHandling&&) = delete;

    protected:

        bool hasPlayerBootedUp(const pge_network::PgeNetworkConnectionHandle& connHandle) const;

        void handlePlayerDied(
            Player& player,
            XHair& xhair,
            const pge_network::PgeNetworkConnectionHandle& nKillerConnHandleServerSide);
        void handlePlayerRespawned(Player& player, XHair& xhair);

        /**
        * Respawn is required when either a game is started, restarted, or a dead player needs to be revived.
        * The player can be either dead or alive, doesn't matter.
        * Player's HP and AP will be reset to defaults, also items and weapons.
        * 
        * @param player      The player to be respawned.
        * @param restartGame Set it to true if reason for respawning is game restart, in such case even player's stats
        *                    and other stuff will be defaulted as well.
        * @param config      The usual Config instance.
        */
        void serverRespawnPlayer(Player& player, bool restartGame, const proofps_dd::Config& config);
        
        /**
        * Resettle is required when a round-based game starts a new round, and we need to reposition alive
        * players back to spawn points. Their HP will be defaulted as well to 100.
        * Player's AP, items and weapons are not touched, so these are carried over to next round.
        * 
        * Shall not be invoked for dead players: dead players shall be respawned, not resettled.
        * Shall not be invoked for players in spectator mode: they must toggle their spectator mode state manually
        * to trigger respawn.
        *
        * @param player      The player to be resettled.
        * @param config      The usual Config instance.
        */
        void serverResettlePlayer(Player& player, const proofps_dd::Config& config);

        void serverUpdateRespawnTimers(
            const proofps_dd::Config& config,
            proofps_dd::GameMode& gameMode,
            proofps_dd::Durations& durations);
        void handlePlayerTeamIdChangedOrToggledSpectatorMode(
            Player& player,
            const unsigned int& iTeamId,
            const bool& bToggledSpectatorMode,
            const proofps_dd::Config& config,
            PGEcfgProfiles& cfgProfiles);
        void handleExplosionMultiKill(
            int nPlayersDiedByExplosion);
        void writePlayerList();
        bool handleUserConnected(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const pge_network::MsgUserConnectedServerSelf& msg,
            PGEcfgProfiles& cfgProfiles,
            proofps_dd::Config& config,
            std::function<void(int)>& cbDisplayMapLoadingProgressUpdate);
        bool handleUserDisconnected(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const pge_network::MsgUserDisconnectedFromServer& msg,
            proofps_dd::GameMode& gameMode);
        bool handleUserNameChange(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserNameChangeAndBootupDone& msg,
            proofps_dd::Config& config,
            PGEcfgProfiles& cfgProfiles);
        void resetSendClientUpdatesCounter(proofps_dd::Config& config);
        void serverUpdatePlayersOldValues(
            proofps_dd::Config& config,
            PgeObjectPool<proofps_dd::Smoke>& smokes);
        void serverSendUserUpdates(
            PGEcfgProfiles& cfgProfiles,
            proofps_dd::Config& config,
            proofps_dd::Durations& durations,
            proofps_dd::GameMode& gameMode);
        bool handleUserUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserUpdateFromServer& msg,
            XHair& xhair,
            const proofps_dd::Config& config,
            proofps_dd::GameMode& gameMode,
            PgeObjectPool<proofps_dd::Smoke>& smokes);
        bool handleDeathNotificationFromServer(
            pge_network::PgeNetworkConnectionHandle nDeadConnHandleServerSide,
            const proofps_dd::MsgDeathNotificationFromServer& msg,
            proofps_dd::GameMode& gameMode);
        bool handlePlayerEventFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgPlayerEventFromServer& msg,
            PureVector& vecCamShakeForce,
            const proofps_dd::Config& config,
            PGEcfgProfiles& cfgProfiles
        );
        bool serverHandleUserInGameMenuCmd(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserInGameMenuCmd& msg,
            const proofps_dd::Config& config,
            PGEcfgProfiles& cfgProfiles
        );
        void updatePlayersVisuals(
            const proofps_dd::Config& config,
            proofps_dd::GameMode& gameMode);

    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;
        proofps_dd::GUI& m_gui;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;
        proofps_dd::CameraHandling& m_camera;

        unsigned int m_nSendClientUpdatesInEveryNthTick = 1;
        unsigned int m_nSendClientUpdatesCntr = m_nSendClientUpdatesInEveryNthTick;

    }; // class PlayerHandling

} // namespace proofps_dd
