#pragma once

/*
    ###################################################################################
    WeaponHandling.h
    Network handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../../Console/CConsole/src/CConsole.h"

#include "../../PGE/PGE/PGE.h"

#include "Durations.h"
#include "GameMode.h"
#include "Maps.h"
#include "Physics.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"
#include "Sounds.h"

namespace proofps_dd
{

    class WeaponHandling :
        protected virtual proofps_dd::Physics
    {
    public:

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        WeaponHandling(
            PGE& pge,
            proofps_dd::Durations& durations,
            std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            proofps_dd::Maps& maps,
            proofps_dd::Sounds& sounds);

        WeaponHandling(const WeaponHandling&) = delete;
        WeaponHandling& operator=(const WeaponHandling&) = delete;
        WeaponHandling(WeaponHandling&&) = delete;
        WeaponHandling&& operator=(WeaponHandling&&) = delete;

    protected:

        void deleteWeaponHandlingAll();

        void serverUpdateWeapons(proofps_dd::GameMode& gameMode);
        
        bool isBulletOutOfMapBounds(const Bullet& bullet) const;
        void serverUpdateBullets(
            proofps_dd::GameMode& gameMode,
            PureObject3D& objXHair,
            const unsigned int& nTickRate);
        void clientUpdateBullets(const unsigned int& nTickRate);
        
        bool handleBulletUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgBulletUpdateFromServer& msg);
        bool handleWpnUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgWpnUpdateFromServer& msg,
            bool bHasValidConnection);
        bool handleWpnUpdateCurrentFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgCurrentWpnUpdateFromServer& msg);

    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;
        proofps_dd::Durations& m_durations;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;


    }; // class WeaponHandling

} // namespace proofps_dd