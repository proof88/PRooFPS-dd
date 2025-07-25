/*
    ###################################################################################
    WeaponHandling.cpp
    Weapon and explosion handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>
#include <chrono>

#include "WeaponHandling.h"


static constexpr float SndWpnFireDistMin = 6.f;
static constexpr float SndWpnFireDistMax = 14.f;
static constexpr float SndWpnReloadDistMin = SndWpnFireDistMin;
static constexpr float SndWpnReloadDistMax = SndWpnFireDistMax;

static constexpr float SndMeleeWpnBulletHitDistMin = 6.f;
static constexpr float SndMeleeWpnBulletHitDistMax = 14.f;


// ############################### PUBLIC ################################


proofps_dd::WeaponHandling::WeaponHandling(
    PGE& pge,
    proofps_dd::Config& config,
    proofps_dd::Durations& durations,
    proofps_dd::GUI& gui,
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    proofps_dd::Maps& maps,
    proofps_dd::Sounds& sounds) :
    proofps_dd::Physics(
        pge,
        m_durations,
        gui,
        m_mapPlayers,
        m_maps,
        m_sounds),
    proofps_dd::PlayerHandling(pge, durations, gui, mapPlayers, maps, sounds),
    m_pge(pge),
    m_config(config),
    m_durations(durations),
    m_gui(gui),
    m_mapPlayers(mapPlayers),
    m_maps(maps),
    m_sounds(sounds)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, config, durations, mapPlayers, sounds
    // But they can be used in other functions.

    // Since this class is used to build up the WeaponHandling class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. WeaponHandling initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be existing at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
    assert(!pge.isGameRunning());
}

CConsole& proofps_dd::WeaponHandling::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* proofps_dd::WeaponHandling::getLoggerModuleName()
{
    return "WeaponHandling";
}

bool proofps_dd::WeaponHandling::initializeWeaponHandling(PGEcfgProfiles& cfgProfiles)
{
    // Which key should switch to which weapon
    WeaponManager::getKeypressToWeaponMap() = {
        {'1', "knife.txt"},
        {'2', "pistol.txt"},
        {'3', "machinegun.txt"},
        {'4', "bazooka.txt"},
        {'5', "pusha.txt"},
        {'6', "machinepistol.txt"}
    };

    Explosion::resetGlobalExplosionId();

    if (m_smokes.capacity() == 0)
    {
        /* 60 is the default min physics rate, dividing it by Extreme config's m_nEmitInEveryNPhysicsIteration should give 30 */
        static_assert(2 == Smoke::smokeEmitOperValues[static_cast<int>(Smoke::SmokeConfigAmount::Extreme)].m_nEmitInEveryNPhysicsIteration);
        
        // now this calculation is less "scientific" than the bullet pool capacity calculation.
        // We could use the bazooka firing rate and smoke amount config to determine pool size, however
        // on the long run smoke will be used also for other effects as well. Anyway, now I'm
        // calculating with 1 player firing 1 rocket / second, that results in 30 smokes max at any moment
        // with max smoke amount config. To be prepared for some other future effects using smoke, I use 40
        // instead of 30 per player.
        // Note that if ALL players are shooting rockets at the same time and ALL rockets travel far, then it can happen that
        // additionally launched rockets may not emit smoke, however I think this situation can happen rarely.
        // For now I keep pool reserve size calculation this way.
        
        //m_smokes.reserve("smokes", 10, m_pge.getPure());
        m_smokes.reserve(
            "smokes",
            cfgProfiles.getVars()[Player::szCVarPlayersMax].getAsUInt() *
                (10 + 60 / Smoke::smokeEmitOperValues[static_cast<int>(Smoke::SmokeConfigAmount::Extreme)].m_nEmitInEveryNPhysicsIteration),
            m_pge.getPure()
        );
    }

    return true;
}

float proofps_dd::WeaponHandling::getDamageAndImpactForceAtDistance(
    const Player& player,
    const Explosion& xpl,
    const Bullet::DamageAreaEffect& eDamageAreaEffect,
    const TPureFloat& fDamageAreaPulse,
    int& nDamageAp,
    const int& nDamageHp,
    PureVector& vecImpactForce)
{
    PureVector vDirPerAxis;
    PureVector vDistancePerAxis;
    const float fDistance = distance_NoZ_with_distancePerAxis(
        player.getPos().getNew().getX(), player.getPos().getNew().getY(),
        player.getObject3D()->getScaledSizeVec().getX(), player.getObject3D()->getScaledSizeVec().getY(),
        xpl.getPrimaryObject3D().getPosVec().getX(), xpl.getPrimaryObject3D().getPosVec().getY(),
        vDirPerAxis, vDistancePerAxis);

    nDamageAp = static_cast<int>(std::lroundf(xpl.getDamageAtDistance(fDistance, eDamageAreaEffect, nDamageAp)));
    const float fRadiusDamage = xpl.getDamageAtDistance(fDistance, eDamageAreaEffect, nDamageHp);

    // basically we calculate impact force from nDamageHp property of the bullet because this is explosive bullet, its damage_hp property
    // shall be bigger than its damage_ap, that is why we use damage_hp for this.
    if (fRadiusDamage > 0.f)
    {
        // to determine the direction of impact, we should use the center positions of player and explosion, however
        // to determine the magnitude of impact, we should use the edges/corners of player and explosion center per axis.
        // That is why fRadiusDamage itself is not good to be used for magnitude, as it is NOT per-axis.
        const float fPlayerWidthHeightRatio = player.getObject3D()->getScaledSizeVec().getX() / player.getObject3D()->getScaledSizeVec().getY();
        const float fDistanceXfactor =
            (eDamageAreaEffect == Bullet::DamageAreaEffect::Constant) ?
            (vDistancePerAxis.getX() <= xpl.getDamageAreaSize() ? 1.f : 0.f) :
            std::max(0.f, (1 - (vDistancePerAxis.getX() / xpl.getDamageAreaSize())));
        const float fDistanceYfactor =
            (eDamageAreaEffect == Bullet::DamageAreaEffect::Constant) ?
            (vDistancePerAxis.getY() <= xpl.getDamageAreaSize() ? 1.f : 0.f) :
            std::max(0.f, (1 - (vDistancePerAxis.getY() / xpl.getDamageAreaSize())));
        const float fImpactX = fDamageAreaPulse * fPlayerWidthHeightRatio * vDirPerAxis.getX() * fDistanceXfactor;
        const float fImpactY = fDamageAreaPulse * vDirPerAxis.getY() * fDistanceYfactor;
        //getConsole().EOLn("WeaponHandling::%s(): fX: %f, fY: %f!", __func__, fImpactX, fImpactY);
        vecImpactForce.Set(fImpactX, fImpactY, 0.f);
    }

    return fRadiusDamage;
}

proofps_dd::Explosion& proofps_dd::WeaponHandling::createExplosionServer(
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
    proofps_dd::GameMode& gameMode)
{
    const ExplosionObjRefId refId = PFL::calcHash(sExplosionGfxObjFilename);
    m_explosions.push_back(
        Explosion(
            m_pge,
            connHandle,
            refId,
            pos,
            fDamageAreaSize));

    const Explosion& xpl = m_explosions.back();

    bool bShotHitTargetStatUpdated = false; // make sure we increase it only once for the shooter, no matter how many players are hit!
    const auto itShooter = m_mapPlayers.find(xpl.getOwner());

    // apply area damage to players
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        const auto& playerConst = player;

        if (playerConst.getHealth() <= 0)
        {
            continue;
        }

        if (!gameMode.isPlayerAllowedForGameplay(player))
        {
            continue;
        }

        PureVector vecImpactForce;
        int nDamageApCalculated = nDamageAp;
        const float fRadiusDamage = getDamageAndImpactForceAtDistance(
            playerConst, xpl, eDamageAreaEffect, fDamageAreaPulse, nDamageApCalculated, nDamageHp, vecImpactForce
        );
        if (fRadiusDamage > 0.f)
        {
            /* player.getImpactForce() is decreased in Physics */
            player.getImpactForce() += vecImpactForce;

            if (playerConst.getServerSideConnectionHandle() == 0)
            {
                // this is server player so shake camera!
                vecCamShakeForce.SetX(abs(vecImpactForce.getX()) * 4);
                vecCamShakeForce.SetY(abs(vecImpactForce.getY()) * 2);
            }

            if (player.getInvulnerability())
            {
                // even for invulnerable players we let the impact for to be modified as usual above
                continue;
            }

            if (!canBulletHitPerFriendlyFireConfig(playerConst, itShooter))
            {
                continue;
            }

            player.doDamage(nDamageApCalculated, static_cast<int>(std::lroundf(fRadiusDamage)));
            //getConsole().EOLn("WeaponHandling::%s(): damage: %d!", __func__, static_cast<int>(std::lroundf(fRadiusDamage)));

            if (itShooter != m_mapPlayers.end())
            {
                if (!bShotHitTargetStatUpdated)
                {
                    bShotHitTargetStatUpdated = true;
                    // unlike in serverUpdateBullets(), here we dont check for weapon type because we believe any explosive weapon is non-melee type!
                    ++itShooter->second.getShotsHitTarget();
                    assert(itShooter->second.getShotsFiredCount()); // shall be non-zero if getShotsHitTarget() is non-zero; debug shall crash cause then it is logic error!
                    itShooter->second.getFiringAccuracy() =
                        (itShooter->second.getShotsFiredCount() == 0u) ? /* just in case of overflow which will most probably never happen */
                        0.f :
                        (itShooter->second.getShotsHitTarget() / static_cast<float>(itShooter->second.getShotsFiredCount()));
                }
            }

            if (playerConst.getHealth() == 0)
            {
                pge_network::PgeNetworkConnectionHandle nKillerConnHandleServerSide;
                if (itShooter == m_mapPlayers.end())
                {
                    // if killer got disconnected before the kill, we can say the killer is the player itself, since
                    // we still want to display the death notification without the killer's name, but we won't decrease
                    // frag count for the player because HandlePlayerDied() is not doing that.
                    nKillerConnHandleServerSide = playerConst.getServerSideConnectionHandle();
                    //getConsole().OLn("WeaponHandling::%s(): Player %s has been killed by a player already left!",
                    //    __func__, playerPair.first.c_str());
                }
                else
                {
                    nKillerConnHandleServerSide = itShooter->first;

                    // unlike in serverUpdateBullets(), here the owner of the explosion can kill even themself, so
                    // in that case frags should be decremented!
                    if (playerConst.getServerSideConnectionHandle() == xpl.getOwner())
                    {
                        --itShooter->second.getFrags();
                        ++itShooter->second.getSuicides();
                    }
                    else
                    {
                        if (shallShooterFragsDecreasedDueToFriendlyFireIfItIsFriendlyFire(playerConst, itShooter->second))
                        {
                            --itShooter->second.getFrags();
                        }
                        else
                        {
                            ++itShooter->second.getFrags();
                        }
                    }
                    //getConsole().OLn("WeaponHandling::%s(): Player %s has been killed by %s, who now has %d frags!",
                    //    __func__, playerPair.first.c_str(), itKiller->first.c_str(), itKiller->second.getFrags());
                }
                // server handles death here, clients will handle it when they receive MsgUserUpdateFromServer
                handlePlayerDied(player, xhair, nKillerConnHandleServerSide);
            }
        }
    }

    return m_explosions.back();
}

proofps_dd::Explosion& proofps_dd::WeaponHandling::createExplosionClient(
    const proofps_dd::Explosion::ExplosionId& id /* explosion id is not used on client-side */,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const PureVector& pos,
    const int& nDamageHp,
    const TPureFloat& fDamageAreaSize,
    const Bullet::DamageAreaEffect& eDamageAreaEffect,
    const TPureFloat& fDamageAreaPulse,
    const std::string& sExplosionGfxObjFilename,
    PureVector& vecCamShakeForce)
{
    const ExplosionObjRefId refId = PFL::calcHash(sExplosionGfxObjFilename);
    m_explosions.push_back(
        Explosion(
            m_pge,
            id /* explosion id is not used on client-side */,
            connHandle,
            refId,
            pos,
            fDamageAreaSize));

    Explosion& xpl = m_explosions.back();

    const auto playerIt = m_mapPlayers.find(m_nServerSideConnectionHandle);
    if (playerIt == m_mapPlayers.end())
    {
        // must always find self player
        assert(false);
        return xpl;
    }

    const auto& playerConst = playerIt->second;
    if (playerConst.getHealth() > 0)
    {
        // on server-side we calculate damage to do damage, but here on client-side we do it just to shake camera
        int nDamageApDummyVar;
        PureVector vecImpactForce;
        const float fRadiusDamage = getDamageAndImpactForceAtDistance(
            playerConst, xpl, eDamageAreaEffect, fDamageAreaPulse, nDamageApDummyVar, nDamageHp, vecImpactForce
        );

        if (fRadiusDamage > 0.f)
        {
            // close enough, shake camera!
            vecCamShakeForce.SetX(abs(vecImpactForce.getX()) * 4);
            vecCamShakeForce.SetY(abs(vecImpactForce.getY()) * 2);
        }
    }

    return xpl;
}

void proofps_dd::WeaponHandling::handleCurrentPlayersCurrentWeaponBulletCountsChangeShared(
    const Player& player,
    Weapon& wpnCurrent,
    const TPureUInt& nOldMagCount,  /* passing arguments separately because sometimes we use different values */
    const TPureUInt& nNewMagCount,
    const TPureUInt& /*nOldUnmagCount*/,
    const TPureUInt& nNewUnmagCount,
    const Weapon::State& oldState,
    const Weapon::State& newState)
{
    // processing weapon bullets count change for the CURRENT player on THIS machine, no matter if we are server or client

    // since auto-reload and auto-switch settings are client-only, they can be also used in this function.

    // It MIGHT happen we receive this when player is dead, we should check and NOT do auto requests in that case!
    // However, InputHandling clears all auto-behaviors before returning in case of health 0, so this is not an issue now.
    // But we also might play sounds here so better return early when player is dead.
    if (player.getHealth() == 0)
    {
        return;
    }

    //if (!m_pge.getNetwork().isServer())
    //{
    //    // because on server this is continuously invoked by serverUpdateWeapons() and flooding
    //    getConsole().EOLn(
    //        "WeaponHandling::%s() nOldMagCount: %u, nNewMagCount: %u, nOldUnmagCount: %u, nNewUnmagCount: %u, oldState: %s, newState: %s!",
    //        __func__,
    //        nOldMagCount,
    //        nNewMagCount,
    //        nOldUnmagCount,
    //        nNewUnmagCount,
    //        Weapon::stateToString(oldState).c_str(),
    //        Weapon::stateToString(newState).c_str());
    //}

    if ((nOldMagCount > 0) && (nNewMagCount == 0))
    {
        m_gui.getXHair()->handleMagEmpty();

        // we fired a bullet and magazine became empty, but cannot yet set wpn auto reload request flag here because this is too early:
        // we don't yet know the updated state of the weapon, or it still not went back to idle/ready after becoming empty ...
        // so we set the flag in handleCurrentPlayersCurrentWeaponStateChangeShared() upon the proper state is set!
    }
    else if (nNewMagCount > 0)
    {
        if (newState == Weapon::WPN_SHOOTING)
        {
            m_gui.getXHair()->handleCooldownStart();
        }
        else
        {
            m_gui.getXHair()->handleMagLoaded();

            if (((newState == Weapon::WPN_RELOADING) || (oldState == Weapon::WPN_RELOADING)) && (nNewMagCount == (nOldMagCount + 1)))
            {
                if (!wpnCurrent.getVars()["reload_per_mag"].getAsBool())
                {
                    // Reload sounds are only played for the specific player, and players cannot hear each other's reload.
                    m_sndWpnReloadStartHandle = m_pge.getAudio().play3dSound(wpnCurrent.getReloadStartSound(), player.getPos().getNew());
                    m_pge.getAudio().getAudioEngineCore().set3dSourceMinMaxDistance(m_sndWpnReloadStartHandle, SndWpnReloadDistMin, SndWpnReloadDistMax);
                    m_pge.getAudio().getAudioEngineCore().set3dSourceAttenuation(m_sndWpnReloadStartHandle, SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
                }
            }
        }
    }
    else if ((newState == Weapon::WPN_READY) && (oldState == Weapon::WPN_READY) && (nOldMagCount == 0) && (nNewMagCount == 0) && (nNewUnmagCount != 0))
    {
        // somehow, now we have just got spare ammo for the current empty weapon, we might initiate auto-reload for this ammo pickup case.
        // Unlike with the weapon change- or weapon state change-induced auto-reload, the ammo pickup-induced auto-reload is handled here.
        // Since this function might be called earlier than handleCurrentPlayersCurrentWeaponStateChangeShared(), I'm making sure prev state and current state are both READY,
        // so this function does not hijack other "auto" settings that might be initiated in handleCurrentPlayersCurrentWeaponStateChangeShared().
        // Would be logical to handle this pickup-induced auto-reload where we handle item pickups for player, but did not find proper place:
        // neither player.takeItem() nor player.handleTakeWeaponItem() looks suitable.
        // Anyway, since both client and server invokes this function upon change in unmag ammo, this looks to be the perfect place.
        if (m_pge.getConfigProfiles().getVars()[szCvarClWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag].getAsBool())
        {
            scheduleWeaponAutoReloadRequest();
            //getConsole().EOLn("WeaponHandling::%s(): empty, has unmag, auto requesting wpn reload!", __func__);
        }
        else
        {
            //getConsole().EOLn("WeaponHandling::%s(): empty, has unmag, but auto reload is NOT configured!", __func__);
        }
    }
}

const bool& proofps_dd::WeaponHandling::getWeaponAutoReloadRequest() const
{
    return m_bWpnAutoReloadRequest;
}

void proofps_dd::WeaponHandling::clearWeaponAutoReloadRequest()
{
    m_bWpnAutoReloadRequest = false;
}

void proofps_dd::WeaponHandling::scheduleWeaponAutoReloadRequest()
{
    if (getWeaponAutoSwitchToBestLoadedRequest() || getWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest() || getWeaponPickupInducedAutoSwitchRequest())
    {
        return;
    }

    m_bWpnAutoReloadRequest = true;
}

const bool& proofps_dd::WeaponHandling::getWeaponAutoSwitchToBestLoadedRequest() const
{
    return m_bWpnAutoSwitchToBestLoadedRequest;
}

void proofps_dd::WeaponHandling::clearWeaponAutoSwitchToBestLoadedRequest()
{
    m_bWpnAutoSwitchToBestLoadedRequest = false;
}

void proofps_dd::WeaponHandling::scheduleWeaponAutoSwitchToBestLoadedRequest()
{
    if (getWeaponAutoReloadRequest() || getWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest() || getWeaponPickupInducedAutoSwitchRequest())
    {
        return;
    }

    m_bWpnAutoSwitchToBestLoadedRequest = true;
}

const bool& proofps_dd::WeaponHandling::getWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest() const
{
    return m_bWpnAutoSwitchToBestWithAnyKindOfAmmoRequest;
}

void proofps_dd::WeaponHandling::clearWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest()
{
    m_bWpnAutoSwitchToBestWithAnyKindOfAmmoRequest = false;
}

void proofps_dd::WeaponHandling::scheduleWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest()
{
    if (getWeaponAutoReloadRequest() || getWeaponAutoSwitchToBestLoadedRequest() || getWeaponPickupInducedAutoSwitchRequest())
    {
        return;
    }

    m_bWpnAutoSwitchToBestWithAnyKindOfAmmoRequest = true;
}

Weapon* proofps_dd::WeaponHandling::getWeaponPickupInducedAutoSwitchRequest() const
{
    return m_pWpnAutoSwitchWhenPickedUp;
}

void proofps_dd::WeaponHandling::clearWeaponPickupInducedAutoSwitchRequest()
{
    m_pWpnAutoSwitchWhenPickedUp = nullptr;
}

void proofps_dd::WeaponHandling::scheduleWeaponPickupInducedAutoSwitchRequest(Weapon* wpn)
{
    if (getWeaponAutoReloadRequest() || getWeaponAutoSwitchToBestLoadedRequest() || getWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest())
    {
        return;
    }

    m_pWpnAutoSwitchWhenPickedUp = wpn;
}

const PgeObjectPool<proofps_dd::Smoke>& proofps_dd::WeaponHandling::getSmokePool() const
{
    return m_smokes;
}


// ############################## PROTECTED ##############################


void proofps_dd::WeaponHandling::deleteWeaponHandlingAll(const bool& bDeallocBullets)
{
    // as explained at m_pWpnAutoSwitchWhenPickedUp, we need to clear these stuff!
    clearWeaponAutoReloadRequest();
    clearWeaponAutoSwitchToBestLoadedRequest();
    clearWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest();
    clearWeaponPickupInducedAutoSwitchRequest();

    m_explosions.clear();
    Explosion::destroyReferenceExplosions();
    Explosion::resetGlobalExplosionId();

    if (bDeallocBullets)
    {
        m_pge.getBullets().deallocate();
        Bullet::destroyReferenceObject();   // we would not need explicit call if Bullet implemented reference counting
        Bullet::resetGlobalBulletId();
        
        m_smokes.deallocate();
        Smoke::destroyReferenceObject();    // we would not need explicit call if Smoke implemented reference counting
    }
    else
    {
        m_pge.getBullets().clear();
        m_smokes.clear();
    }
}

void proofps_dd::WeaponHandling::serverUpdateWeapons(proofps_dd::GameMode& gameMode)
{
    if (gameMode.isGameWon())
    {
        return;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    /* Do not set the momentary weapon accuracy every frame, as triggering MsgUserUpdate just for this would be overkill.
       Remember, now we are in a per - frame server function.
       Enough to set it every Nth frame, this is only for xhair scaling anyway, not used in physics. */
    constexpr int nEveryNthWeaponMomentaryAccuracyIsActuallySet = 4;
    static int nWeaponMomentaryAccuracySetCounter = 0;
    nWeaponMomentaryAccuracySetCounter++;
    const bool bSetWeaponMomentaryAccuracyInThisFrame = (nWeaponMomentaryAccuracySetCounter == nEveryNthWeaponMomentaryAccuracyIsActuallySet);
    if (bSetWeaponMomentaryAccuracyInThisFrame)
    {
        nWeaponMomentaryAccuracySetCounter = 0;
    }

    for (auto& playerPair : m_mapPlayers)
    {
        const pge_network::PgeNetworkConnectionHandle& playerServerSideConnHandle = playerPair.first;
        Player& player = playerPair.second;
        Weapon* const wpn = player.getWeaponManager().getCurrentWeapon();
        if (!wpn)
        {
            continue;
        }

        const auto nOldMagCount = wpn->getMagBulletCount();
        const auto nOldUnmagCount = wpn->getUnmagBulletCount();
        bool bSendPrivateWpnUpdatePktToTheClientOnly = false;
        if (player.getAttack() && player.attack())
        {
            //getConsole().EOLn("WeaponHandling::%s(): player %u attack", __func__, playerServerSideConnHandle);
            // server will have the new bullet, clients will learn about the new bullet when server is sending out
            // the regular bullet updates;
            // but we send out the wpn update for bullet count change here for that single client
            if (playerServerSideConnHandle != pge_network::ServerConnHandle)
            {
                // server doesn't need to send this msg to itself, it already executed bullet count change by pullTrigger() in player.attack()
                bSendPrivateWpnUpdatePktToTheClientOnly = true;
            }

            // here server plays the firing sound, clients play for themselves when they receive newborn bullet update;
            // this is lame, as I think the weapon object itself should play when it fires a bullet, however currently
            // firing i.e. pullTrigger() is not actually happening on client-side. On the long run we should send a shoot action flag to client
            // so it will execute its weapon object's pullTrigger(). Probably this will be needed for other purpose as well
            // such as handling weapon statuses better on client-side, for animation, more sounds, etc.
            // Reload sounds are only played for the specific player, and players cannot hear each other's reload.
            if (playerServerSideConnHandle == pge_network::ServerConnHandle)
            {
                // my newborn bullet, so reload sounds must be killed asap
                m_pge.getAudio().stopSoundInstance(m_sndWpnReloadStartHandle);
                m_pge.getAudio().stopSoundInstance(m_sndWpnReloadEndHandle);
            }
            const auto sndWpnFireHandle = m_pge.getAudio().play3dSound(wpn->getFiringSound(), player.getPos().getNew());
            m_pge.getAudio().getAudioEngineCore().set3dSourceMinMaxDistance(sndWpnFireHandle, SndWpnFireDistMin, SndWpnFireDistMax);
            m_pge.getAudio().getAudioEngineCore().set3dSourceAttenuation(sndWpnFireHandle, SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
        }  // end player.getAttack() && attack()

        if (wpn->update())
        {
            if (playerServerSideConnHandle != pge_network::ServerConnHandle)
            {
                // server doesn't need to send this msg to itself, it already executed bullet count change by wpn->update()
                bSendPrivateWpnUpdatePktToTheClientOnly = true;
            }
        }

        /* Since clients do not have enough data to calculate momentary accuracy, and does not worth implementing replicating all of those data,
           we are sending their momentary weapon accuracy in regular user updates. */
        if (bSetWeaponMomentaryAccuracyInThisFrame)
        {
            player.setWeaponMomentaryAccuracy(
                wpn->getMomentaryAccuracy(
                    player.isMoving() && m_pge.getConfigProfiles().getVars()[Player::szCVarSvMovingAffectsAim].getAsBool(),
                    player.isRunning(),
                    player.getCrouchStateCurrent())
            );
        }

        if (playerServerSideConnHandle == pge_network::ServerConnHandle)
        {
            handleCurrentPlayersCurrentWeaponBulletCountsChangeShared(
                player,
                *wpn,
                nOldMagCount,
                wpn->getMagBulletCount(),
                nOldUnmagCount,
                wpn->getUnmagBulletCount(),
                wpn->getState().getOld(),
                wpn->getState().getNew());
            // TODO: would be nice to find a way to AVOID calling this every frame on server, but we have code inside even for READY->READY state change,
            // as being recognized as possible weapon change case when we might also need to initiate the auto-reload.
            // Client is not invoking this every frame, only when receiving a message.
            handleCurrentPlayersCurrentWeaponStateChangeShared(
                player, *wpn, wpn->getState().getOld(), wpn->getState().getNew(), wpn->getMagBulletCount(), wpn->getUnmagBulletCount());
        }

        // to make the auto weapon reload work properly for clients, MsgWpnUpdateFromServer should be always sent out earlier than MsgCurrentWpnUpdateFromServer, because
        // they will initiate the auto reload for MsgCurrentWpnUpdateFromServer if relevant bullet mag and unmag conditions meet, and we need them updated!
        // But they also need the latest status too, so from v0.2.7 I include weapon state in this msg too. Previously it was only in MsgCurrentWpnUpdateFromServer.
        if (bSendPrivateWpnUpdatePktToTheClientOnly)
        {
            pge_network::PgePacket pktWpnUpdatePrivate;
            if (!proofps_dd::MsgWpnUpdateFromServer::initPkt(
                pktWpnUpdatePrivate,
                pge_network::ServerConnHandle /* ignored by client anyway */,
                wpn->getFilename(),
                MapItemType::ITEM_HEALTH /* intentionally setting nonsense type, because itemType should be valid only in case of item pickup for now */,
                /* IMPORTANT: later if we remove wpn filename from message, even here we will need to use correct itemType, so we will be unable to distinguish
                   from wpn reload induced ammo increase or item pickup induced ammo increase in handleWpnUpdateFromServer(), thus we will need to add a flag
                   that clearly indicates if scenario is item pickup or not! */
                wpn->isAvailable(),
                wpn->getState().getNew(),
                wpn->getMagBulletCount(),
                wpn->getUnmagBulletCount(),
                0 /* unused when itemType is MapItemType::ITEM_HEALTH */))
            {
                getConsole().EOLn("WeaponHandling::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
                continue;
            }
            m_pge.getNetwork().getServer().send(pktWpnUpdatePrivate, playerServerSideConnHandle);
        }

        bool bSendPublicWpnUpdatePktToAllClients = false;
        if (wpn->getState().isDirty())
        {
            bSendPublicWpnUpdatePktToAllClients = true;
            wpn->updateOldValues();
        }

        if (bSendPublicWpnUpdatePktToAllClients)
        {
            // we use the same msg as being used for handling weapon change in InputHandling, however it cannot happen that same type of message
            // is sent out twice in same tick to clients since in case of weapon change the weapon state is not changing for sure!
            pge_network::PgePacket pktWpnUpdateCurrentPublic;
            if (!proofps_dd::MsgCurrentWpnUpdateFromServer::initPkt(
                pktWpnUpdateCurrentPublic,
                playerServerSideConnHandle,
                wpn->getFilename(),
                wpn->getState().getNew()))
            {
                getConsole().EOLn("WeaponHandling::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
                continue;
            }
            //getConsole().EOLn("WeaponHandling::%s(): sending weapon state old: %d, new: %d", __func__, wpn->getState().getOld(), wpn->getState().getNew());
            m_pge.getNetwork().getServer().sendToAllClientsExcept(pktWpnUpdateCurrentPublic);
        }
    }  // end for playerPair

    m_durations.m_nUpdateWeaponsDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

bool proofps_dd::WeaponHandling::isBulletOutOfMapBounds(const Bullet& bullet) const
{
    // we relax map bounds a bit to let the bullets leave map area a bit more before destroying them ...
    const PureVector vRelaxedMapMinBounds(
        m_maps.getBlocksVertexPosMin().getX() - proofps_dd::Maps::fMapBlockSizeWidth * 4,
        m_maps.getBlocksVertexPosMin().getY() - proofps_dd::Maps::fMapBlockSizeHeight * 4,
        m_maps.getBlocksVertexPosMin().getZ() - proofps_dd::Maps::fMapBlockSizeDepth); // ah why dont we have vector-scalar subtract operator defined ...
    const PureVector vRelaxedMapMaxBounds(
        m_maps.getBlocksVertexPosMax().getX() + proofps_dd::Maps::fMapBlockSizeWidth * 4,
        m_maps.getBlocksVertexPosMax().getY() + proofps_dd::Maps::fMapBlockSizeHeight * 4,
        m_maps.getBlocksVertexPosMax().getZ() + proofps_dd::Maps::fMapBlockSizeDepth);
    
    return !colliding3(vRelaxedMapMinBounds, vRelaxedMapMaxBounds, bullet.getObject3D().getPosVec(), bullet.getObject3D().getScaledSizeVec());
}

Weapon* proofps_dd::WeaponHandling::getWeaponByIdFromAnyPlayersWeaponManager(const WeaponId& wpnId)
{
    if (m_mapPlayers.empty())
    {
        return nullptr;
    }

    return m_mapPlayers.begin()->second.getWeaponManager().getWeaponById(wpnId);
}

void proofps_dd::WeaponHandling::play3dMeleeWeaponHitSound(
    const WeaponId& wpnId,
    const float& posX,
    const float& posY,
    const float& posZ,
    const proofps_dd::MsgBulletUpdateFromServer::BulletDelete& hitType)
{
    assert(hitType != proofps_dd::MsgBulletUpdateFromServer::BulletDelete::No);
    assert(hitType != proofps_dd::MsgBulletUpdateFromServer::BulletDelete::Yes);

    // just for playing sound, let's use any WeaponManager to retrieve weapon, even tho it is not their bullet, it doesnt matter now!
    Weapon* const wpnForSound = getWeaponByIdFromAnyPlayersWeaponManager(wpnId);
    if (!wpnForSound)
    {
        return;
    }

    if (wpnForSound && (wpnForSound->getType() == Weapon::Type::Melee))
    {
        const auto handleSndHit = m_pge.getAudio().play3dSound(
            ((hitType == proofps_dd::MsgBulletUpdateFromServer::BulletDelete::YesHitPlayer) ? wpnForSound->getPlayerHitSound() : wpnForSound->getWallHitSound()),
            posX, posY, posZ);
        m_pge.getAudio().getAudioEngineCore().set3dSourceMinMaxDistance(handleSndHit, SndMeleeWpnBulletHitDistMin, SndMeleeWpnBulletHitDistMax);
        m_pge.getAudio().getAudioEngineCore().set3dSourceAttenuation(handleSndHit, SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
    }
}

void proofps_dd::WeaponHandling::play3dMeleeWeaponHitSound(const WeaponId& wpnId, const PureVector& posVec, const proofps_dd::MsgBulletUpdateFromServer::BulletDelete& hitType)
{
    play3dMeleeWeaponHitSound(wpnId, posVec.getX(), posVec.getY(), posVec.getZ(), hitType);
}

void proofps_dd::WeaponHandling::play3dMeleeWeaponHitSound(const Bullet& bullet, const proofps_dd::MsgBulletUpdateFromServer::BulletDelete& hitType)
{
    play3dMeleeWeaponHitSound(bullet.getWeaponId(), bullet.getObject3D().getPosVec(), hitType);
}

bool proofps_dd::WeaponHandling::canBulletHitPerFriendlyFireConfig(
    const Player& playerHit,
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>::iterator& itShooter) const
{
    if (itShooter == m_mapPlayers.end())
    {
        // shooter got disconnected in the meantime, in such case we allow depending on game mode and config
        return !GameMode::getGameMode()->isTeamBasedGame() ||
            (GameMode::getGameMode()->isTeamBasedGame() && m_config.getFriendlyFire());
    }

    const Player& playerShooter = itShooter->second;

    return !GameMode::getGameMode()->isTeamBasedGame() ||
        ((playerHit.getTeamId() != playerShooter.getTeamId()) || m_config.getFriendlyFire());
}

bool proofps_dd::WeaponHandling::shallShooterFragsDecreasedDueToFriendlyFireIfItIsFriendlyFire(const Player& playerHit, const Player& playerShooter) const
{
    return GameMode::getGameMode()->isTeamBasedGame() &&
        (playerHit.getTeamId() == playerShooter.getTeamId()) && m_config.getFriendlyFire();
}

void proofps_dd::WeaponHandling::serverUpdateBullets(proofps_dd::GameMode& gameMode, XHair& xhair, const unsigned int& nPhysicsRate, PureVector& vecCamShakeForce)
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    const bool bCollisionModeBvh = (m_pge.getConfigProfiles().getVars()[Maps::szCVarSvMapCollisionMode].getAsInt() == 1);

    // on the long run this function needs to be part of the game engine itself, however currently game engine doesn't handle collisions,
    // so once we introduce the collisions to the game engine, it will be an easy move of this function as well there
    pge_network::PgePacket newPktBulletUpdate;
    bool bEndGame = gameMode.isGameWon();
    PgeObjectPool<PooledBullet>& bullets = m_pge.getBullets();
    size_t iti = 0; // to track how many used bullets we processed in the loop, to exit early if we already processed all used
    // we need iti because there is no way to explicitly iterate over the used elems on the object pool
    auto it = bullets.begin();
    while ((iti != bullets.size()) && (it != bullets.end()))
    {
        if (!it->used())
        {
            it++;
            continue;
        }
        iti++;

        auto& bullet = *it;

        bool bDeleteBullet = false;
        bool bWallHit = false;
        bool bPlayerHit = false;
        if (bEndGame)
        {
            bDeleteBullet = true;
        }
        else
        {
            bullet.Update(nPhysicsRate);

            if ((bullet.getTravelDistanceMax() > 0.f) && (bullet.getTravelledDistance() >= bullet.getTravelDistanceMax()))
            {
                bDeleteBullet = true;
            }
        }

        const float fBulletPosX = bullet.getObject3D().getPosVec().getX();
        const float fBulletPosY = bullet.getObject3D().getPosVec().getY();
        const float fBulletScaledSizeX = bullet.getObject3D().getScaledSizeVec().getX();
        const float fBulletScaledSizeY = bullet.getObject3D().getScaledSizeVec().getY();

        if (!bDeleteBullet)
        {
            emitParticles(bullet);

            // check if bullet is hitting a player
            for (auto& playerPair : m_mapPlayers)
            {
                auto& player = playerPair.second;
                if (bullet.getOwner() == player.getServerSideConnectionHandle())
                {
                    // bullet cannot hit the owner, at least for now ...
                    // in the future, when bullets start in proper position, we won't need this check ...
                    // this check will be bad anyway in future when we will have the guided rockets that actually can hit the owner if guided in suicide way!
                    continue;
                }

                if (player.getInvulnerability())
                {
                    continue;
                }

                if (!gameMode.isPlayerAllowedForGameplay(player))
                {
                    continue;
                }

                const auto& playerConst = player;
                const auto playerScaledSizeVec = player.getObject3D()->getScaledSizeVec();
                const auto itShooter = m_mapPlayers.find(bullet.getOwner());
                if ((playerConst.getHealth() > 0) &&
                    canBulletHitPerFriendlyFireConfig(playerConst, itShooter) &&
                    colliding2_NoZ(
                        player.getPos().getNew().getX(), player.getPos().getNew().getY(),
                        playerScaledSizeVec.getX(), playerScaledSizeVec.getY(),
                        fBulletPosX, fBulletPosY,
                        fBulletScaledSizeX, fBulletScaledSizeY))
                {
                    bDeleteBullet = true;
                    bPlayerHit = true;
                    if (bullet.getAreaDamageSize() == 0.f)
                    {
                        // non-explosive bullets do damage here, explosive bullets make explosions so then the explosion does damage in createExplosionServer()
                        player.doDamage(bullet.getDamageAp(), bullet.getDamageHp());

                        if (itShooter != m_mapPlayers.end())
                        {
                            // let's use any WeaponManager to retrieve weapon, even tho it is not their bullet, it doesnt matter now, we just need the weapon type!
                            const Weapon* const wpnForType = getWeaponByIdFromAnyPlayersWeaponManager(bullet.getWeaponId());
                            // intentionally not counting with melee weapons for aim accuracy stat, let them swing the knife in the air and against walls without affecting their aim accuracy stat!
                            if (wpnForType && (wpnForType->getType() != Weapon::Type::Melee))
                            {
                                ++itShooter->second.getShotsHitTarget();
                                assert(itShooter->second.getShotsFiredCount()); // shall be non-zero if getShotsHitTarget() is non-zero; debug shall crash cause then it is logic error!
                                itShooter->second.getFiringAccuracy() =
                                    (itShooter->second.getShotsFiredCount() == 0u) ? /* just in case of overflow which will most probably never happen */
                                    0.f :
                                    (itShooter->second.getShotsHitTarget() / static_cast<float>(itShooter->second.getShotsFiredCount()));
                            }
                        } // otherwise shooter got disconnected before hitting the player

                        if (playerConst.getHealth() == 0)
                        {
                            pge_network::PgeNetworkConnectionHandle nKillerConnHandleServerSide;
                            if (itShooter == m_mapPlayers.end())
                            {
                                // if killer got disconnected before the kill, we can say the killer is the player itself, since
                                // we still want to display the death notification without the killer's name, but we won't decrease
                                // frag count for the player because HandlePlayerDied() is not doing that.
                                nKillerConnHandleServerSide = player.getServerSideConnectionHandle();
                                //getConsole().OLn("WeaponHandling::%s(): Player %s has been killed by a player already left!",
                                //    __func__, playerPair.first.c_str());
                            }
                            else
                            {
                                nKillerConnHandleServerSide = itShooter->first;
                                if (shallShooterFragsDecreasedDueToFriendlyFireIfItIsFriendlyFire(playerConst, itShooter->second))
                                {
                                    --itShooter->second.getFrags();
                                }
                                else
                                {
                                    ++itShooter->second.getFrags();
                                }
                                bEndGame = gameMode.serverCheckAndUpdateWinningConditions(m_pge.getNetwork());
                                //getConsole().OLn("WeaponHandling::%s(): Player %s has been killed by %s, who now has %d frags!",
                                //    __func__, playerPair.first.c_str(), itShooter->first.c_str(), itShooter->second.getFrags());
                            }
                            // server handles death here, clients will handle it when they receive MsgUserUpdateFromServer
                            handlePlayerDied(player, xhair, nKillerConnHandleServerSide);
                        }
                    }
                    break; // we can stop since a bullet can touch 1 playerPair only at a time
                }
            }

            if (!bDeleteBullet)
            {
                if (isBulletOutOfMapBounds(bullet))
                {
                    bDeleteBullet = true;
                }
            }

            if (!bDeleteBullet)
            {
                // check if bullet is hitting a map element
                
                if (bCollisionModeBvh)
                {
                    const float fBulletPosZ = bullet.getObject3D().getPosVec().getZ();
                    const float fBulletScaledSizeZ = bullet.getObject3D().getScaledSizeVec().getZ();
                    bWallHit = serverUpdateBullets_collisionWithWalls_bvh(
                        fBulletPosX, fBulletPosY, fBulletPosZ,
                        fBulletScaledSizeX, fBulletScaledSizeY, fBulletScaledSizeZ);
                }
                else
                {
                    bWallHit = serverUpdateBullets_collisionWithWalls_legacy(
                        bullet,
                        fBulletPosX, fBulletPosY,
                        fBulletScaledSizeX, fBulletScaledSizeY);
                }

                if (bWallHit)
                {
                    bDeleteBullet = true;
                }
            }  // endif !bDeleteBullet
        }  // endif !bDeleteBullet

        if (bDeleteBullet)
        {
            // make explosion first if required;
            // server does not send specific message to client about creating explosion, it is client's responsibility to create explosion
            // when it receives MsgBulletUpdateFromServer with bDelete flag set, if bullet has area damage!
            if (!bEndGame)
            {
                if (bullet.getAreaDamageSize() > 0.f)
                {
                    // let's use any WeaponManager to retrieve weapon, even tho it is not their bullet, it doesnt matter now, we need explosion data!
                    Weapon* const wpnForExplosionData = getWeaponByIdFromAnyPlayersWeaponManager(bullet.getWeaponId());
                    if (wpnForExplosionData)
                    {
                        createExplosionServer(
                            bullet.getOwner(),
                            bullet.getObject3D().getPosVec(),
                            bullet.getAreaDamageSize(),
                            bullet.getAreaDamageEffect(),
                            bullet.getAreaDamagePulse(),
                            wpnForExplosionData->getVars()["damage_area_gfx_obj"].getAsString(),
                            bullet.getDamageAp(),
                            bullet.getDamageHp(),
                            xhair,
                            vecCamShakeForce,
                            gameMode);
                    }
                }
            }

            // TODO: we should have a separate msg for deleting Bullet because its size would be much less than this msg!
            // But now we have to stick to this because explosion create on client-side requires all values, see details in:
            // handleBulletUpdateFromServer
            proofps_dd::MsgBulletUpdateFromServer::initPkt(
                newPktBulletUpdate,
                bullet.getOwner(),
                bullet.getId(),
                bullet.getWeaponId(),
                fBulletPosX,
                fBulletPosY,
                bullet.getObject3D().getPosVec().getZ(),
                bullet.getObject3D().getAngleVec().getX(),
                bullet.getObject3D().getAngleVec().getY(),
                bullet.getObject3D().getAngleVec().getZ(),
                bullet.getDamageHp(),
                bullet.getAreaDamageSize(),
                bullet.getAreaDamageEffect(),
                bullet.getAreaDamagePulse()
                );
            // clients will also delete this bullet on their side because we set pkt's delete flag here
            if (bPlayerHit)
            {
                proofps_dd::MsgBulletUpdateFromServer::getDelete(newPktBulletUpdate) = proofps_dd::MsgBulletUpdateFromServer::BulletDelete::YesHitPlayer;
                play3dMeleeWeaponHitSound(bullet, proofps_dd::MsgBulletUpdateFromServer::BulletDelete::YesHitPlayer);
            }
            else if (bWallHit)
            {
                proofps_dd::MsgBulletUpdateFromServer::getDelete(newPktBulletUpdate) = proofps_dd::MsgBulletUpdateFromServer::BulletDelete::YesHitWall;
                play3dMeleeWeaponHitSound(bullet, proofps_dd::MsgBulletUpdateFromServer::BulletDelete::YesHitWall);
            }
            else
            {
                proofps_dd::MsgBulletUpdateFromServer::getDelete(newPktBulletUpdate) = proofps_dd::MsgBulletUpdateFromServer::BulletDelete::Yes;
            }
            it = bullets.erase(it); // delete it right now, otherwise later we would send further updates to clients about this bullet
            m_pge.getNetwork().getServer().sendToAllClientsExcept(newPktBulletUpdate);
        }
        else
        {
            if (!bullet.isCreateSentToClients())
            {
                // new bullet, inform clients
                bullet.isCreateSentToClients() = true;
                proofps_dd::MsgBulletUpdateFromServer::initPkt(
                    newPktBulletUpdate,
                    bullet.getOwner(),
                    bullet.getId(),
                    bullet.getWeaponId(),
                    fBulletPosX,
                    fBulletPosY,
                    bullet.getObject3D().getPosVec().getZ(),
                    bullet.getObject3D().getAngleVec().getX(),
                    bullet.getObject3D().getAngleVec().getY(),
                    bullet.getObject3D().getAngleVec().getZ(),
                    bullet.getDamageHp(),
                    bullet.getAreaDamageSize(),
                    bullet.getAreaDamageEffect(),
                    bullet.getAreaDamagePulse());
                m_pge.getNetwork().getServer().sendToAllClientsExcept(newPktBulletUpdate);
            }
            // bullet didn't touch anything, go to next
            it++;
            // since v0.1.4, server doesn't send the bullet travel updates to clients since clients simulate the travel in clientUpdateBullets()
        }

        // 'it' is referring to next bullet, don't use it from here!
    }

    if (bEndGame && (Bullet::getGlobalBulletId() > 0))
    {
        Bullet::resetGlobalBulletId();
    }

    m_durations.m_nUpdateBulletsDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void proofps_dd::WeaponHandling::clientUpdateBullets(const unsigned int& nPhysicsRate)
{
    // on the long run this function needs to be part of the game engine itself, however currently game engine doesn't handle collisions,
    // so once we introduce the collisions to the game engine, it will be an easy move of this function as well there
    PgeObjectPool<PooledBullet>& bullets = m_pge.getBullets();
    size_t iti = 0; // to track how many used bullets we processed in the loop, to exit early if we already processed all used
    // we need iti because there is no way to explicitly iterate over the used elems on the object pool
    auto it = bullets.begin();
    while ((iti != bullets.size()) && (it != bullets.end()))
    {
        if (!it->used())
        {
            it++;
            continue;
        }
        iti++;

        auto& bullet = *it;

        // since v0.1.4, client simulates bullet movement, without any delete condition check, because delete happens only if server says so!
        // This can also mean that with higher latency, clients can render bullets moving over/into walls, players, etc. but it doesnt matter
        // because still the server is the authorative entity, and such visual anomalies might happen only for moments only.
        bullet.Update(nPhysicsRate);
        emitParticles(bullet);

        // There was a time when I was thinking that client should check against out of map bounds to cover corner case when we somehow miss
        // the server's signal about that, in that case client would continue simulate bullet travel forever.
        // However, I decided not to handle that because it could also introduce some unwanted effect: imagine that client detects out of map
        // bounds earlier, so it deletes the bullet, however in next moment a bullet update for the same bullet is coming from the server,
        // then client would have to create that bullet again which would bring performance penalty. Then next moment another msg from server
        // would come about deleting the bullet. So I think we should just wait anyway for server to tell us delete bullet for any reason.
        it++;
    }
}

void proofps_dd::WeaponHandling::serverUpdateExplosions(proofps_dd::GameMode& gameMode, const unsigned int& nPhysicsRate)
{
    // on the long run this function needs to be part of the game engine itself, however first serverUpdateBullets() should be moved there!

    const bool bEndGame = gameMode.isGameWon();
    auto it = m_explosions.begin();
    while (it != m_explosions.end())
    {
        auto& xpl = *it;
        xpl.update(nPhysicsRate);

        if (xpl.shouldBeDeleted())
        {
            it = m_explosions.erase(it);
        }
        else
        {
            it++;
        }
    }

    if (bEndGame && (Explosion::getGlobalExplosionId() > 0))
    {
        Explosion::resetGlobalExplosionId();
    }
}

void proofps_dd::WeaponHandling::clientUpdateExplosions(proofps_dd::GameMode& gameMode, const unsigned int& nPhysicsRate)
{
    // remember that on client-side, all explosions have id 0 because we simply do not set anything;
    // for now we can do exactly what server does with explosions

    serverUpdateExplosions(gameMode, nPhysicsRate);
}

void proofps_dd::WeaponHandling::updateSmokes(proofps_dd::GameMode&, const unsigned int& nPhysicsRate)
{
    // on the long run this function needs to be part of the game engine itself

    auto it = m_smokes.begin();
    size_t iti = 0; // to track how many used bullets we processed in the loop, to exit early if we already processed all used
    // we need iti because there is no way to explicitly iterate over the used elems on the object pool
    while ((iti != m_smokes.size()) && (it != m_smokes.end()))
    {
        auto& smoke = *it;
        if (!smoke.used())
        {
            it++;
            continue;
        }
        iti++;

        smoke.update(nPhysicsRate);
        it++;
    }
}

bool proofps_dd::WeaponHandling::handleBulletUpdateFromServer(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const proofps_dd::MsgBulletUpdateFromServer& msg,
    PureVector& vecCamShakeForce)
{
    if (m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("WeaponHandling::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    if (!m_maps.loaded())
    {
        // do not deal with these if client has not yet loaded the map
        return true;
    }

    if (msg.m_delete != proofps_dd::MsgBulletUpdateFromServer::BulletDelete::No)
    {
        if (msg.m_delete != proofps_dd::MsgBulletUpdateFromServer::BulletDelete::Yes)
        {
            play3dMeleeWeaponHitSound(msg.m_weaponId, msg.m_pos.x, msg.m_pos.y, msg.m_pos.z, msg.m_delete);
        }

        // Make explosion first if required;
        // unlike server, client makes explosion when server says a bullet should be deleted, in such case client's job is to make explosion
        // with same parameters as server makes it, based on bullet properties such as position, area damage size, etc., this is why it is important
        // that server sends bullet position also when bDelete flag is set, so the explosion position will be the same as on server side!
        // Since client simulates bullet travel for visual purpose only, not doing collision check, it relies on server to know when a hit happened,
        // and for that obviously we need the server-side position of bullet at the moment of hit.
        // Also, we actually require other bullet parameters as well for making the explosion, because we might not have the bullet in getBullets() on
        // our side: when 2 players are almost at the same position (partially overlapping), one fires the weapon, then bullet will immediately hit
        // the other player on server-side, we will receive a bullet delete request only from server in that case without any bullet create request prior to it!
        if (msg.m_fDamageAreaSize > 0.f)
        {
            // let's use any WeaponManager to retrieve weapon, even tho it is not their bullet, it doesnt matter now, we need explosion data!
            Weapon* const wpnForExplosionData = getWeaponByIdFromAnyPlayersWeaponManager(msg.m_weaponId);
            if (wpnForExplosionData)
            {
                createExplosionClient(
                    0 /* explosion id is not used on client-side */,
                    connHandleServerSide,
                    /* server puts the last calculated bullet positions into message when it asks us to delete the bullet so we put explosion here */
                    PureVector(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z),
                    msg.m_nDamageHp,
                    msg.m_fDamageAreaSize,
                    msg.m_eDamageAreaEffect,
                    msg.m_fDamageAreaPulse,
                    wpnForExplosionData->getVars()["damage_area_gfx_obj"].getAsString(),
                    vecCamShakeForce);
            }
            else
            {
                getConsole().EOLn("WeaponHandling::%s(): getWeaponByIdFromAnyPlayersWeaponManager() failed with weapon id: %u!", 
                   __func__, static_cast<uint32_t>(msg.m_weaponId));
                assert(false);
                return false;
            }
        }
    }

    auto it = m_pge.getBullets().begin();
    while (it != m_pge.getBullets().end())
    {
        if ((it->used()) && (it->getId() == msg.m_bulletId))
        {
            break;
        }
        it++;
    }

    if (m_pge.getBullets().end() == it)
    {
        // newborn bullet 

        if (msg.m_delete != proofps_dd::MsgBulletUpdateFromServer::BulletDelete::No)
        {
            // this is valid scenario: explained a few lines earlier
            return true;
        }
        // need to create this new bullet first on our side
        //getConsole().OLn("WeaponHandling::%s(): received MsgBulletUpdateFromServer: NEW bullet id %u", __func__, msg.m_bulletId);

        // Find the owner of this new bullet.
        // Due to https://github.com/proof88/PRooFPS-dd/issues/268, we need to apply WA here.
        // Explained in details in handlePlayerEventFromServer().
        // Due to this, now this WA is applied: return true from non-serveronly msg handling functions if connHandle is not found in m_mapPlayers.
        const auto playerIt = m_mapPlayers.find(connHandleServerSide);
        if (playerIt == m_mapPlayers.end())
        {
            // in theory, must always find bullet owner player since even player disconnect should come later than their created bullets
            assert(false);  // keep crashing in debug mode, nobody will connect to server during ongoing shooting with debug builds!
            return true; // WA
        }

        Player& player = playerIt->second;

        Weapon* const wpn = player.getWeaponManager().getWeaponById(msg.m_weaponId);
        if (!wpn)
        {
            getConsole().EOLn("WeaponHandling::%s(): getWeapon() failed!", __func__);
            assert(false);
            return false;
        }

        if (isMyConnection(connHandleServerSide))
        {
            // my bullet just fired so reload sounds must be killed asap
            m_pge.getAudio().stopSoundInstance(m_sndWpnReloadStartHandle);
            m_pge.getAudio().stopSoundInstance(m_sndWpnReloadEndHandle);
        }
        const auto sndWpnFireHandle = m_pge.getAudio().play3dSound(wpn->getFiringSound(), msg.m_pos);
        m_pge.getAudio().getAudioEngineCore().set3dSourceMinMaxDistance(sndWpnFireHandle, SndWpnFireDistMin, SndWpnFireDistMax);
        m_pge.getAudio().getAudioEngineCore().set3dSourceAttenuation(sndWpnFireHandle, SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);

        // here create() invokes PooledBullet::init(), should invoke the client version!
        if (!m_pge.getBullets().create(
            msg.m_bulletId,
            msg.m_weaponId,
            m_pge.getPure(),
            msg.m_pos.x, msg.m_pos.y, msg.m_pos.z,
            msg.m_angle.x, msg.m_angle.y, msg.m_angle.z,
            wpn->getVars()["bullet_visible"].getAsBool(),
            wpn->getVars()["bullet_size_x"].getAsFloat(),
            wpn->getVars()["bullet_size_y"].getAsFloat(),
            wpn->getVars()["bullet_size_z"].getAsFloat(),
            wpn->getVars()["bullet_speed"].getAsFloat(),
            wpn->getVars()["bullet_gravity"].getAsFloat(),
            wpn->getVars()["bullet_drag"].getAsFloat(),
            /* fragile is not used by client-side ctor */
            /* distanceMax is not used by client-side ctor */
            (wpn->getVars()["bullet_particle"].getAsString() == "smoke" ?
                Bullet::ParticleType::Smoke :
                Bullet::ParticleType::None),
            /* damageAP is not used by client-side ctor */
            msg.m_nDamageHp,
            msg.m_fDamageAreaSize, msg.m_eDamageAreaEffect, msg.m_fDamageAreaPulse))
        {
            getConsole().EOLn("WeaponHandling::%s():  pool did not create bullet!", __func__);
            assert(false); // crash in debug
            return true; // dont crash release
        }
    }
    else
    {
        //getConsole().OLn("WeaponHandling::%s(): received MsgBulletUpdateFromServer: old bullet id %u", __func__, msg.m_bulletId);
        //pBullet = &(*it);

        if (msg.m_delete == proofps_dd::MsgBulletUpdateFromServer::BulletDelete::No)
        {
            // for a known bullet, client should not receive position updates from server, so log error!
            getConsole().EOLn("WeaponHandling::%s(): received non-delete update for already known bullet, MUST NOT HAPPEN!", __func__);
        }

        m_pge.getBullets().erase(it);
    }

    return true;
}

bool proofps_dd::WeaponHandling::handleWpnUpdateFromServer(
    pge_network::PgeNetworkConnectionHandle /* connHandleServerSide, not filled properly by server so we ignore it */,
    const proofps_dd::MsgWpnUpdateFromServer& msg)
{
    if (m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("WeaponHandling::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    getConsole().OLn("WeaponHandling::%s(): received: %s, available: %s, state: %s, mag: %u, unmag: %u!",
        __func__, msg.m_szWpnName, msg.m_bAvailable ? "yes" : "no", Weapon::stateToString(msg.m_state).c_str(), msg.m_nMagBulletCount, msg.m_nUnmagBulletCount);

    // this is private message, it always refers to me and one of my weapons (it is always my current weapon on server side, should be current here too).

    // Because of issue 268 (https://github.com/proof88/PRooFPS-dd/issues/268) is about receiving msg about other players, but this msg is for me, WA should not be applied here!
    const auto playerIt = m_mapPlayers.find(m_nServerSideConnectionHandle);
    if (playerIt == m_mapPlayers.end())
    {
        // must always find self player
        return false;
    }

    Player& player = playerIt->second;
    // TODO: since from v0.2.6 we also have msg.m_eMapItemType, we should get rid of msg.m_szWpnName,
    // and use the getWeaponInstanceByMapItemType() as already used later in this function from v0.2.8!
    Weapon* const wpn = player.getWeaponManager().getWeaponByFilename(msg.m_szWpnName);
    if (!wpn)
    {
        getConsole().EOLn("WeaponHandling::%s(): did not find wpn: %s!", __func__, msg.m_szWpnName);
        assert(false);
        return false;
    }

    assert(player.getWeaponManager().getCurrentWeapon());

    wpn->clientReceiveStateFromServer(msg.m_state);
    if (player.getWeaponManager().getCurrentWeapon()->getFilename() == msg.m_szWpnName)
    {
        handleCurrentPlayersCurrentWeaponStateChangeShared(player, *wpn, wpn->getState().getOld(), msg.m_state, wpn->getMagBulletCount(), wpn->getUnmagBulletCount());

        handleCurrentPlayersCurrentWeaponBulletCountsChangeShared(
            player,
            *wpn,
            wpn->getMagBulletCount(),
            msg.m_nMagBulletCount,
            wpn->getUnmagBulletCount(),
            msg.m_nUnmagBulletCount,
            wpn->getState().getOld(),
            wpn->getState().getNew());
    }
    else
    {
        // as explained few lines below, we need to check for ITEM_HEALTH cases even in weapon handling function to distinguish between different cases!
        if (msg.m_eMapItemType != MapItemType::ITEM_HEALTH)
        {
            auto* pWpnPicked = player.getWeaponInstanceByMapItemType(msg.m_eMapItemType);
            if (pWpnPicked)
            {
                handleAutoSwitchUponWeaponPickupShared(player, *player.getWeaponManager().getCurrentWeapon(), *pWpnPicked, !pWpnPicked->isAvailable() && msg.m_bAvailable);
            }
            else
            {
                getConsole().EOLn("WeaponHandling::%s(): did not find wpn by map item type %d at line %d!", __func__, msg.m_eMapItemType, __LINE__);
                assert(false);
                return false;
            }
        }
    }

    // since we receive this message also for any other kind of weapon update, we need to understand if this is item pickup scenario or not, and as a hack,
    // currently MapItemType::ITEM_HEALTH is sent (from serverUpdateWeapons()) when it is NOT an item pickup scenario!
    // For example, ITEM_HEALTH is set in this msg when the bullet count change is due to firing or reload!
    if (msg.m_eMapItemType != MapItemType::ITEM_HEALTH)
    {
        // server invokes handleTakeWeaponItem() in Player::takeItem();
        // we can be sure that this update is for the CURRENT client and not about other players, so we can play sound too
        player.handleTakeWeaponItem(msg.m_eMapItemType, *wpn, !wpn->isAvailable() && msg.m_bAvailable, msg.m_nAmmoIncrease);
    }

    wpn->SetAvailable(msg.m_bAvailable);
    wpn->SetMagBulletCount(msg.m_nMagBulletCount);
    wpn->SetUnmagBulletCount(msg.m_nUnmagBulletCount);

    return true;
}

bool proofps_dd::WeaponHandling::handleWpnUpdateCurrentFromServer(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgCurrentWpnUpdateFromServer& msg)
{
    if (m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("WeaponHandling::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    //getConsole().EOLn("WeaponHandling::%s(): received: %s for player %u, state: %d",  __func__, msg.m_szWpnCurrentName, connHandleServerSide, static_cast<int>(msg.m_state));

    // Due to https://github.com/proof88/PRooFPS-dd/issues/268, we need to apply WA here.
    // Explained in details in handlePlayerEventFromServer().
    // Due to this, now this WA is applied: return true from non-serveronly msg handling functions if connHandle is not found in m_mapPlayers.
    // For this message, it can happen right after changing map to warena where players most probably pick up a new weapon right after spawning, and due to
    // pickup-induced auto-switch to new is configured, they will switch to it automatically, triggering this kind of message being sent to all clients.
    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("WeaponHandling::%s(): failed to find user with connHandleServerSide: %u, WA in place, ignoring error!", __func__, connHandleServerSide);
        //assert(false);
        return true; // WA
    }

    auto& player = it->second;
    Weapon* const wpn = player.getWeaponManager().getWeaponByFilename(msg.m_szWpnCurrentName);
    if (!wpn)
    {
        getConsole().EOLn("WeaponHandling::%s(): did not find wpn: %s!", __func__, msg.m_szWpnCurrentName);
        assert(false);
        return false;
    }

    // this is public message, it may refer to any other play, thus I should always check with isMyConnection() to know if it is related to me or not!

    // state must be set always, no matter if we are alive or not!
    wpn->clientReceiveStateFromServer(msg.m_state);
    if (isMyConnection(it->first))
    {
        handleCurrentPlayersCurrentWeaponStateChangeShared(player, *wpn, wpn->getState().getOld(), msg.m_state, wpn->getMagBulletCount(), wpn->getUnmagBulletCount());
    }
    
    if (std::as_const(player).getHealth() > 0)
    {
        if (isMyConnection(it->first) && (player.getWeaponManager().getCurrentWeapon()->getFilename() != msg.m_szWpnCurrentName))
        {
            //getConsole().OLn("WeaponHandling::%s(): this current weapon update is changing my current weapon!", __func__);
            m_pge.getAudio().playSound(m_sounds.m_sndChangeWeapon);

            assert(m_gui.getPlayerAmmoChangeEvents());
            m_gui.getPlayerAmmoChangeEvents()->clear();

            handleCurrentPlayersCurrentWeaponBulletCountsChangeShared(
                player,
                *wpn,
                player.getWeaponManager().getCurrentWeapon()->getMagBulletCount(),
                wpn->getMagBulletCount(),
                player.getWeaponManager().getCurrentWeapon()->getUnmagBulletCount(),
                wpn->getUnmagBulletCount(),
                wpn->getState().getOld(),
                wpn->getState().getNew());
        }
        
        if (!player.getWeaponManager().setCurrentWeapon(wpn,
            true /* even client should record last switch time to be able to check it on client side too */,
            m_pge.getNetwork().isServer()))
        {
            getConsole().EOLn("WeaponHandling::%s(): player %s switching to %s failed due to setCurrentWeapon() failed!",
                __func__, player.getName().c_str(), wpn->getFilename().c_str());
            assert(false);
            return false;
        }
    }
    player.getWeaponManager().getCurrentWeapon()->UpdatePosition(player.getObject3D()->getPosVec(), player.isSomersaulting());

    return true;
}

void proofps_dd::WeaponHandling::handleCurrentPlayersCurrentWeaponStateChangeShared(
    const Player& player,
    Weapon& wpnCurrent,
    const Weapon::State& oldState /* passing arguments separately because sometimes we use different values */,
    const Weapon::State& newState,
    const TPureUInt& nMagCount,
    const TPureUInt& nUnmagCount)
{
    // processing weapon state change for the CURRENT player on THIS machine, no matter if we are server or client

    // since auto-reload and auto-switch settings are client-only, they can be also used in this function.

    // It MIGHT happen we receive this when player is dead, we should check and NOT do auto requests in that case!
    // However, InputHandling clears all auto-behaviors before returning in case of health 0, so this is not an issue now.
    // But we also might play sounds here so better return early when player is dead.
    if (player.getHealth() == 0)
    {
        return;
    }

    switch (oldState)
    {
    case Weapon::State::WPN_RELOADING:
    {
        switch (newState)
        {
        case Weapon::State::WPN_READY:
            if (wpnCurrent.getVars()["reload_per_mag"].getAsBool())
            {
                // Reload sounds are only played for the specific player, and players cannot hear each other's reload.
                // Reload end sound valid only for per-magazine reload weapons but we just dont check here, getReloadEndSound() is just not loaded.
                m_sndWpnReloadEndHandle = m_pge.getAudio().play3dSound(wpnCurrent.getReloadEndSound(), player.getPos().getNew());
                m_pge.getAudio().getAudioEngineCore().set3dSourceMinMaxDistance(m_sndWpnReloadEndHandle, SndWpnReloadDistMin, SndWpnReloadDistMax);
                m_pge.getAudio().getAudioEngineCore().set3dSourceAttenuation(m_sndWpnReloadEndHandle, SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
            }
            [[fallthrough]];
        case Weapon::State::WPN_SHOOTING:
            m_gui.getXHair()->stopBlinking();
            break;
        default:
            break;
        }
        break;
    }
    case Weapon::State::WPN_SHOOTING:
        switch (newState)
        {
        case Weapon::State::WPN_READY:
            //getConsole().EOLn(
            //    "WeaponHandling::%s() state change: %s -> %s", __func__, Weapon::stateToString(oldState).c_str(), Weapon::stateToString(newState).c_str());

            // this is when depending on user setting, we might reload current or switch to another (or do nothing)
            if (nMagCount == 0)
            {
                if (nUnmagCount != 0)
                {
                    /* "when current goes empty BUT HAS spare ammo" setting */

                    if (m_pge.getConfigProfiles().getVars()[szCvarClWpnEmptyMagNonemptyUnmagBehavior].getAsString() == szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoReload)
                    {
                        // Since server should do the reload, we need to ask it do try do that for us.
                        // The easiest way would be from InputHandling as a real user keypress for reload, so I set this flag that supposed to be checked by InputHandling
                        // at its next run! Basically this is how we "signal" InputHandling to do this for "us"!
                        scheduleWeaponAutoReloadRequest();
                        //getConsole().EOLn("WeaponHandling::%s(): has unmag, auto requesting wpn reload!", __func__);
                    }
                    else if (m_pge.getConfigProfiles().getVars()[szCvarClWpnEmptyMagNonemptyUnmagBehavior].getAsString() == szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoSwitchToBestLoaded)
                    {
                        // This is also handled in InputHandling like a real user weapon change keypress
                        scheduleWeaponAutoSwitchToBestLoadedRequest();
                        //getConsole().EOLn("WeaponHandling::%s(): has unmag, auto switch to best loaded wpn!", __func__);
                    }
                    else if (m_pge.getConfigProfiles().getVars()[szCvarClWpnEmptyMagNonemptyUnmagBehavior].getAsString() == szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoSwitchToBestReloadable)
                    {
                        // This is also handled in InputHandling like a real user weapon change keypress
                        scheduleWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest();
                        //getConsole().EOLn("WeaponHandling::%s(): has unmag, auto switch to best wpn with any kind of ammo!", __func__);
                    }
                    else
                    {
                        //getConsole().EOLn("WeaponHandling::%s(): has unmag, configured to do nothing!", __func__);
                    }
                }
                else
                {
                    /* "when current goes empty AND has NO spare ammo" setting */

                    // this look redundant coding now, I might change later, for now I need this only for clarity of what I'm doing
                    if (m_pge.getConfigProfiles().getVars()[szCvarClWpnEmptyMagEmptyUnmagBehavior].getAsString() == szCvarClWpnEmptyMagEmptyUnmagBehaviorValueAutoSwitchToBestLoaded)
                    {
                        // This is also handled in InputHandling like a real user weapon change keypress
                        scheduleWeaponAutoSwitchToBestLoadedRequest();
                        //getConsole().EOLn("WeaponHandling::%s(): no unmag, auto switch to best loaded wpn!", __func__);
                    }
                    else if (m_pge.getConfigProfiles().getVars()[szCvarClWpnEmptyMagEmptyUnmagBehavior].getAsString() == szCvarClWpnEmptyMagEmptyUnmagBehaviorValueAutoSwitchToBestReloadable)
                    {
                        // This is also handled in InputHandling like a real user weapon change keypress
                        scheduleWeaponAutoSwitchToBestWithAnyKindOfAmmoRequest();
                        //getConsole().EOLn("WeaponHandling::%s(): no unmag, auto switch to best wpn with any kind of ammo!", __func__);
                    }
                    else
                    {
                        //getConsole().EOLn("WeaponHandling::%s(): no unmag, configured to do nothing!", __func__);
                    }
                }
            }
            else
            {
                // required by client, server handles this in handleCurrentPlayersCurrentWeaponBulletCountsChangeShared()
                m_gui.getXHair()->handleCooldownEnd();
            }
            break; // end case SHOOTING -> READY
        default:
            break;
        }
        break;
    case Weapon::State::WPN_READY:
        switch (newState)
        {
        case Weapon::State::WPN_RELOADING:
            m_gui.getXHair()->startBlinking();
            {
                if (wpnCurrent.getVars()["reload_per_mag"].getAsBool())
                {
                    // Reload sounds are only played for the specific player, and players cannot hear each other's reload.
                    m_sndWpnReloadStartHandle = m_pge.getAudio().play3dSound(wpnCurrent.getReloadStartSound(), player.getPos().getNew());
                    m_pge.getAudio().getAudioEngineCore().set3dSourceMinMaxDistance(m_sndWpnReloadStartHandle, SndWpnReloadDistMin, SndWpnReloadDistMax);
                    m_pge.getAudio().getAudioEngineCore().set3dSourceAttenuation(m_sndWpnReloadStartHandle, SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
                }
            }
            break;
        case Weapon::State::WPN_READY:
            // READY -> READY, so most probably this is the result of weapon change, thus here we dont initiate any switch, but can initiate reload

            if ((nMagCount == 0) && (nUnmagCount != 0))
            {
                if (m_pge.getConfigProfiles().getVars()[szCvarClWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag].getAsBool())
                {
                    scheduleWeaponAutoReloadRequest();
                    //getConsole().EOLn("WeaponHandling::%s(): READY -> READY, has unmag, auto requesting wpn reload!", __func__);
                }
                else
                {
                    //getConsole().EOLn("WeaponHandling::%s(): READY -> READY, switched to, has unmag, but auto reload is NOT configured!", __func__);
                }
            }

            break;
        default:
            break;
        }
        break;
    default:
        getConsole().EOLn("WeaponHandling::%s(): unhandled old state: %s!", __func__, Weapon::stateToString(oldState).c_str());
    }
}

/**
* This function was made specifically for deciding if we initiate auto-switch upon picking up a weapon.
* The pickup-induced auto-reload and firing-induced actions are handled in separate functions.
*/
void proofps_dd::WeaponHandling::handleAutoSwitchUponWeaponPickupShared(const Player& /*player*/, Weapon& wpnCurrent, Weapon& wpnPicked, const bool& bHasJustBecomeAvailable)
{
    if (&wpnCurrent == &wpnPicked)
    {
        // since both weapons are residing in the same player's same container, comparing their address is enough to know if they are the same weapon
        return;
    }

    //getConsole().EOLn(
    //    "WeaponHandling::%s(): wpn %s different than current (%s) has just got picked up, bHasJustBecomeAvailable: %b!",
    //    __func__,
    //    wpnPicked.getFilename().c_str(),
    //    wpnCurrent.getFilename().c_str(),
    //    bHasJustBecomeAvailable);

    if (bHasJustBecomeAvailable)
    {
        if (m_pge.getConfigProfiles().getVars()[szCvarClWpnAutoSwitchWhenPickedUpNewWeapon].getAsString() == szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchAlways)
        {
            // since these weapons are pre-created for the player, we can simply pass the pointer itself which stays valid until deleting the Player itself!
            scheduleWeaponPickupInducedAutoSwitchRequest(&wpnPicked);
            //getConsole().EOLn("WeaponHandling::%s(): auto-switch: always!", __func__);
        }
        else if (m_pge.getConfigProfiles().getVars()[szCvarClWpnAutoSwitchWhenPickedUpNewWeapon].getAsString() == szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchIfBetter)
        {
            if (wpnCurrent.getDamagePerSecondRating() < wpnPicked.getDamagePerSecondRating())
            {
                scheduleWeaponPickupInducedAutoSwitchRequest(&wpnPicked);
                //getConsole().EOLn("WeaponHandling::%s(): auto-switch: if better!", __func__);
            }
        }
    }
    
    if ((wpnCurrent.getMagBulletCount() == 0) &&
        (wpnCurrent.getState() != Weapon::WPN_RELOADING) /* obviously dont schedule change empty to non-empty if we are already reloading the current one! */ &&
        m_pge.getConfigProfiles().getVars()[szCvarClWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag].getAsBool())
    {
        // since these weapons are pre-created for the player, we can simply pass the pointer itself which stays valid until deleting the Player itself!
        scheduleWeaponPickupInducedAutoSwitchRequest(&wpnPicked);
        //getConsole().EOLn("WeaponHandling::%s(): auto-switch: if empty!", __func__);
    }
}


// ############################### PRIVATE ###############################


void proofps_dd::WeaponHandling::emitParticles(PooledBullet& bullet)
{
    if ((bullet.getParticleType() == Bullet::ParticleType::Smoke) && (m_config.getSmokeConfigAmount() != Smoke::SmokeConfigAmount::None))
    {
        // generate smoke, note this should be in bullet.update() on the long run
        bullet.getParticleEmitPerNthPhysicsIterationCntr()++;
        assert(static_cast<int>(m_config.getSmokeConfigAmount()) < static_cast<int>(Smoke::smokeEmitOperValues.size()));
        if (bullet.getParticleEmitPerNthPhysicsIterationCntr() >= Smoke::smokeEmitOperValues[static_cast<int>(m_config.getSmokeConfigAmount())].m_nEmitInEveryNPhysicsIteration)
        {
            bullet.getParticleEmitPerNthPhysicsIterationCntr() = 0;
            m_smokes.create(
                bullet.getPut(),
                (bullet.getObject3D().getAngleVec().getY() == 0.f) /* goingLeft, otherwise it would be 180.f */);
        }
    }
}


/**
* Used before v0.5.
* 
* @return True if bullet hit a wall (foreground block), false otherwise.
*/
bool proofps_dd::WeaponHandling::serverUpdateBullets_collisionWithWalls_legacy(
    PooledBullet& bullet,
    const float& fBulletPosX,
    const float& fBulletPosY,
    const float& fBulletScaledSizeX,
    const float& fBulletScaledSizeY)
{
    // Originally with only 6 bullets, with VSync FPS went down from 60 to 45 on main dev machine.
    // For now with the small bullet direction optimization in the loop I managed to keep FPS around 45-50
    // with 10-15 bullets.

    for (int i = 0; i < m_maps.getForegroundBlockCount(); i++)
    {
        const PureObject3D* const obj = m_maps.getForegroundBlocks()[i];
        const bool bGoingLeft = (bullet.getObject3D().getAngleVec().getY() == 0.f); // otherwise it would be 180.f
        const float fMapObjPosX = obj->getPosVec().getX();
        const float fMapObjPosY = obj->getPosVec().getY();
        const float fRealBlockSizeXhalf = obj->getSizeVec().getX() / 2.f;
        const float fRealBlockSizeYhalf = obj->getSizeVec().getY() / 2.f;

        if ((bGoingLeft && (fMapObjPosX - fRealBlockSizeXhalf > fBulletPosX)) ||
            (!bGoingLeft && (fMapObjPosX + fRealBlockSizeXhalf < fBulletPosX)))
        {
            // optimization: rule out those blocks which are not in bullet's direction
            continue;
        }

        const float fBulletPosXMinusHalf = fBulletPosX - fBulletScaledSizeX / 2.f;
        const float fBulletPosXPlusHalf = fBulletPosX - fBulletScaledSizeX / 2.f;
        const float fBulletPosYMinusHalf = fBulletPosY - fBulletScaledSizeY / 2.f;
        const float fBulletPosYPlusHalf = fBulletPosY - fBulletScaledSizeY / 2.f;

        if ((fMapObjPosX + fRealBlockSizeXhalf < fBulletPosXMinusHalf) || (fMapObjPosX - fRealBlockSizeXhalf > fBulletPosXPlusHalf))
        {
            continue;
        }

        if ((fMapObjPosY + fRealBlockSizeYhalf < fBulletPosYMinusHalf) || (fMapObjPosY - fRealBlockSizeYhalf > fBulletPosYPlusHalf))
        {
            continue;
        }

        return true;
    }

    return false;
} // serverUpdateBullets_collisionWithWalls_legacy()

/**
* Used from v0.5.
*
* @return True if bullet hit a wall (foreground block), false otherwise.
*/
bool proofps_dd::WeaponHandling::serverUpdateBullets_collisionWithWalls_bvh(
    const float& fBulletPosX,
    const float& fBulletPosY,
    const float& fBulletPosZ,
    const float& fBulletScaledSizeX,
    const float& fBulletScaledSizeY,
    const float& fBulletScaledSizeZ)
{
    const PureAxisAlignedBoundingBox aabbBullet(
        PureVector(fBulletPosX, fBulletPosY, fBulletPosZ),
        PureVector(fBulletScaledSizeX, fBulletScaledSizeY, fBulletScaledSizeZ));
    
    return (m_maps.getBVH().findOneColliderObject_startFromFirstNode(aabbBullet, nullptr) != nullptr);
} // serverUpdateBullets_collisionWithWalls_bvh()
