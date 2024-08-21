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
    static constexpr char* szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchIfEmpty = "auto-switch-if-empty";
    static constexpr char* szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchIfBetter = "auto-switch-if-better";
    static constexpr char* szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchAlways = "auto-switch";

    static constexpr char* szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueNoop = "no-op";
    static constexpr char* szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoReload = "auto-reload";
    static constexpr char* szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoSwitchToBestLoaded = "auto-switch-to-best-non-empty";
    static constexpr char* szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoSwitchToBestReloadable = "auto-switch-to-best-empty-but-reloadable";

    static constexpr char* szCvarClWpnEmptyMagEmptyUnmagBehaviorValueNoop = "no-op";
    static constexpr char* szCvarClWpnEmptyMagEmptyUnmagBehaviorValueAutoSwitchToBestLoaded = "auto-switch-to-best-non-empty";
    static constexpr char* szCvarClWpnEmptyMagEmptyUnmagBehaviorValueAutoSwitchToBestReloadable = "auto-switch-to-best-empty-but-reloadable";

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
            int& nDamageAp,
            const int& nDamageHp,
            PureVector& vecImpactForce);
        Explosion& createExplosionServer(
            const pge_network::PgeNetworkConnectionHandle& connHandle,
            const PureVector& pos,
            const TPureFloat& fDamageAreaSize,
            const TPureFloat& fDamageAreaPulse,
            const int& nDamageAp,
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

        void handleCurrentPlayersCurrentWeaponBulletCountsChangeShared(
            const Player& player,
            Weapon& wpnCurrent,
            const TPureUInt& nOldMagCount,
            const TPureUInt& nNewMagCount,
            const TPureUInt& nOldUnmagCount,
            const TPureUInt& nNewUnmagCount,
            const Weapon::State& oldState,
            const Weapon::State& newState);

        const bool& getWeaponAutoReloadRequest() const;
        void clearWeaponAutoReloadRequest();
        void scheduleWeaponAutoReloadRequest();

        const bool& getWeaponAutoSwitchToBestLoadedRequest() const;
        void clearWeaponAutoSwitchToBestLoadedRequest();
        void scheduleWeaponAutoSwitchToBestLoadedRequest();

        const bool& getWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest() const;
        void clearWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest();
        void scheduleWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest();

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
        void handleCurrentPlayersCurrentWeaponStateChangeShared(
            const Player& player,
            Weapon& wpnCurrent,
            const Weapon::State& oldState,
            const Weapon::State& newState,
            const TPureUInt& nMagCount,
            const TPureUInt& nUnmagCount);

        void handleAutoSwitchUponWeaponPickupShared(
            const Player& player,
            Weapon& wpnCurrent,
            Weapon& wpnPicked,
            const bool& bHasJustBecomeAvailable);

    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;
        proofps_dd::Durations& m_durations;
        proofps_dd::GUI& m_gui;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;
        SoLoud::handle m_sndWpnReloadStartHandle;
        SoLoud::handle m_sndWpnReloadEndHandle;

        std::list<Explosion> m_explosions;
        bool m_bWpnAutoReloadRequest = false;
        bool m_bWpnAutoSwitchToBestLoadedRequest = false;
        bool m_bWpnAutoSwitchToBestWithAnyKindOfAmmoRequest = false;

    }; // class WeaponHandling

} // namespace proofps_dd