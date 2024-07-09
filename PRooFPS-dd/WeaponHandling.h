#pragma once

/*
    ###################################################################################
    WeaponHandling.h
    Weapon and explosion handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include "CConsole.h"

#include "PGE.h"

#include "Durations.h"
#include "Explosion.h"
#include "GameMode.h"
#include "GUI.h"
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
            proofps_dd::GUI& gui,
            std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            proofps_dd::Maps& maps,
            proofps_dd::Sounds& sounds);

        WeaponHandling(const WeaponHandling&) = delete;
        WeaponHandling& operator=(const WeaponHandling&) = delete;
        WeaponHandling(WeaponHandling&&) = delete;
        WeaponHandling&& operator=(WeaponHandling&&) = delete;

        bool initializeWeaponHandling();
        float getDamageAndImpactForceAtDistance(
            const Player& player,
            const Explosion& xpl,
            const TPureFloat& fDamageAreaPulse,
            const int& nDamageHp,
            PureVector& vecImpactForce);
        Explosion& createExplosionServer(
            const pge_network::PgeNetworkConnectionHandle& connHandle,
            const PureVector& pos,
            const TPureFloat& fDamageAreaSize,
            const TPureFloat& fDamageAreaPulse,
            const int& nDamageHp,
            XHair& xhair,
            PureVector& vecCamShakeForce,
            proofps_dd::GameMode& gameMode);
        Explosion& createExplosionClient(
            const proofps_dd::Explosion::ExplosionId& id,
            const pge_network::PgeNetworkConnectionHandle& connHandle,
            const PureVector& pos,
            const int& nDamageHp,
            const TPureFloat& fDamageAreaSize,
            const TPureFloat& fDamageAreaPulse,
            PureVector& vecCamShakeForce);

        void handleCurrentWeaponBulletCountsChangeShared(
            const TPureUInt& nOldMagCount,
            const TPureUInt& nNewMagCount,
            const TPureUInt& nOldUnmagCount,
            const TPureUInt& nNewUnmagCount);

    protected:

        void deleteWeaponHandlingAll();

        void serverUpdateWeapons(proofps_dd::GameMode& gameMode);
        
        bool isBulletOutOfMapBounds(const Bullet& bullet) const;
        void serverUpdateBullets(
            proofps_dd::GameMode& gameMode,
            XHair& xhair,
            const unsigned int& nPhysicsRate,
            PureVector& vecCamShakeForce);
        void clientUpdateBullets(const unsigned int& nPhysicsRate);
        void serverUpdateExplosions(
            proofps_dd::GameMode& gameMode,
            const unsigned int& nPhysicsRate);
        void clientUpdateExplosions(
            proofps_dd::GameMode& gameMode,
            const unsigned int& nPhysicsRate);
        
        bool handleBulletUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgBulletUpdateFromServer& msg,
            PureVector& vecCamShakeForce);
        bool handleWpnUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgWpnUpdateFromServer& msg);
        bool handleWpnUpdateCurrentFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgCurrentWpnUpdateFromServer& msg);
        void handleWeaponStateChangeShared(
            const Weapon::State& oldState,
            const Weapon::State& newState);

    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;
        proofps_dd::Durations& m_durations;
        proofps_dd::GUI& m_gui;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;

        std::list<Explosion> m_explosions;

    }; // class WeaponHandling

} // namespace proofps_dd