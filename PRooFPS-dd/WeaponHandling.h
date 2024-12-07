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

#include "Config.h"
#include "Durations.h"
#include "Explosion.h"
#include "GameMode.h"
#include "GUI.h"
#include "Maps.h"
#include "Physics.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"
#include "Smoke.h"
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
            proofps_dd::Config& config,
            proofps_dd::Durations& durations,
            proofps_dd::GUI& gui,
            std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            proofps_dd::Maps& maps,
            proofps_dd::Sounds& sounds);

        WeaponHandling(const WeaponHandling&) = delete;
        WeaponHandling& operator=(const WeaponHandling&) = delete;
        WeaponHandling(WeaponHandling&&) = delete;
        WeaponHandling&& operator=(WeaponHandling&&) = delete;

        bool initializeWeaponHandling(PGEcfgProfiles& cfgProfiles);
        float getDamageAndImpactForceAtDistance(
            const Player& player,
            const Explosion& xpl,
            const Bullet::DamageAreaEffect& eDamageAreaEffect,
            const TPureFloat& fDamageAreaPulse,
            int& nDamageAp,
            const int& nDamageHp,
            PureVector& vecImpactForce);
        Explosion& createExplosionServer(
            const pge_network::PgeNetworkConnectionHandle& connHandle,
            const PureVector& pos,
            const TPureFloat& fDamageAreaSize,
            const Bullet::DamageAreaEffect& eDamageAreaEffect,
            const TPureFloat& fDamageAreaPulse,
            const std::string& sExplosionGfxObjFilename,
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
            const Bullet::DamageAreaEffect& eDamageAreaEffect,
            const TPureFloat& fDamageAreaPulse,
            const std::string& sExplosionGfxObjFilename,
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

        Weapon* getWeaponPickupInducedAutoSwitchRequest() const;
        void clearWeaponPickupInducedAutoSwitchRequest();
        void scheduleWeaponPickupInducedAutoSwitchRequest(Weapon* wpn);

        const PgeObjectPool<Smoke>& getSmokePool() const;

    protected:

        void deleteWeaponHandlingAll(const bool& bDeallocBullets);

        void serverUpdateWeapons(proofps_dd::GameMode& gameMode);
        
        bool isBulletOutOfMapBounds(const Bullet& bullet) const;
        Weapon* getWeaponByIdFromAnyPlayersWeaponManager(const WeaponId& wpnId);
        void play3dMeleeWeaponHitSound(
            const WeaponId& wpnId,
            const float& posX,
            const float& posY,
            const float& posZ,
            const proofps_dd::MsgBulletUpdateFromServer::BulletDelete& hitType);
        void play3dMeleeWeaponHitSound(const WeaponId& wpnId, const PureVector& posVec, const proofps_dd::MsgBulletUpdateFromServer::BulletDelete& hitType);
        void play3dMeleeWeaponHitSound(const Bullet& bullet, const proofps_dd::MsgBulletUpdateFromServer::BulletDelete& hitType);
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
        void updateSmokes(proofps_dd::GameMode& gameMode, const unsigned int& nPhysicsRate);
        
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
        proofps_dd::Config& m_config;
        proofps_dd::Durations& m_durations;
        proofps_dd::GUI& m_gui;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;
        SoLoud::handle m_sndWpnReloadStartHandle;
        SoLoud::handle m_sndWpnReloadEndHandle;

        std::list<Explosion> m_explosions;
        PgeObjectPool<Smoke> m_smokes;
        bool m_bWpnAutoReloadRequest = false;
        bool m_bWpnAutoSwitchToBestLoadedRequest = false;
        bool m_bWpnAutoSwitchToBestWithAnyKindOfAmmoRequest = false;
        
        // TODO: should be weak ptr, as we dont want ownership of Weapon instance but want to detect when it is deleted.
        // It can be deleted only when the Player instance owning the parent WeaponManager instance is deleted.
        // So we could think, a regular ptr is enough.
        // But this thinking caused a bug: since auto switch handling is being done in keyboard handling, which is running only if window is active,
        // auto weapon switch does not happen with an inactive window. If the window is inactive, but Player picks up a weapon unintentionally e.g.
        // respawning after being killed, this ptr will be set, and later when map change happens, all Player and Weapon instances are deleted and recreated,
        // this pointer will be invalid! When window becomes active again, this ptr will be tried to be used to switch to this Weapon instance which
        // has been already deleted! Result: undefined behavior, sooner or later crash.
        Weapon* m_pWpnAutoSwitchWhenPickedUp = nullptr;

        void emitParticles(PooledBullet& bullet);

    }; // class WeaponHandling

} // namespace proofps_dd