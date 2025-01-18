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
            proofps_dd::Sounds& sounds);

        PlayerHandling(const PlayerHandling&) = delete;
        PlayerHandling& operator=(const PlayerHandling&) = delete;
        PlayerHandling(PlayerHandling&&) = delete;
        PlayerHandling&& operator=(PlayerHandling&&) = delete;

    protected:

        void handlePlayerDied(
            Player& player,
            XHair& xhair,
            const pge_network::PgeNetworkConnectionHandle& nKillerConnHandleServerSide,
            proofps_dd::GameMode& gameMode);
        void handlePlayerRespawned(Player& player, XHair& xhair);
        void serverRespawnPlayer(Player& player, bool restartGame, const proofps_dd::Config& config);
        void serverUpdateRespawnTimers(
            const proofps_dd::Config& config,
            proofps_dd::GameMode& gameMode,
            proofps_dd::Durations& durations);
        void updatePlayersOldValues();
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
        void serverSendUserUpdates(
            PGEcfgProfiles& cfgProfiles,
            proofps_dd::Config& config,
            proofps_dd::Durations& durations);
        bool handleUserUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserUpdateFromServer& msg,
            XHair& xhair,
            const proofps_dd::Config& config,
            proofps_dd::GameMode& gameMode);
        bool handleDeathNotificationFromServer(
            pge_network::PgeNetworkConnectionHandle nDeadConnHandleServerSide, const proofps_dd::MsgDeathNotificationFromServer& msg);
        bool handlePlayerEventFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgPlayerEventFromServer& msg
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

        unsigned int m_nSendClientUpdatesInEveryNthTick = 1;
        unsigned int m_nSendClientUpdatesCntr = m_nSendClientUpdatesInEveryNthTick;

    }; // class PlayerHandling

} // namespace proofps_dd
