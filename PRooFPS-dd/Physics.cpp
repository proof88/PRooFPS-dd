/*
    ###################################################################################
    Physics.cpp
    Physics handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>
#include <chrono>

#include "Physics.h"

#include "Benchmarks.h"


// ############################### PUBLIC ################################


proofps_dd::Physics::Physics(
    PGE& pge,
    proofps_dd::Durations& durations,
    proofps_dd::GUI& gui,
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    proofps_dd::Maps& maps,
    proofps_dd::Sounds& sounds) :
    proofps_dd::PlayerHandling(pge, durations, gui, mapPlayers, maps, sounds),
    m_pge(pge),
    m_durations(durations),
    m_mapPlayers(mapPlayers),
    m_maps(maps),
    m_sounds(sounds),
    m_bAllowStrafeMidAir(true),
    m_bAllowStrafeMidAirFull(false),
    m_nFallDamageMultiplier(0),
    m_bCollisionModeBvh(true)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations, mapPlayers, maps, sounds
    // But they can be used in other functions.

    // Since this class is used to build up the PRooFPSddPGE class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. PRooFPSddPGE initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be existing at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
    assert(!pge.isGameRunning());
}

CConsole& proofps_dd::Physics::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* proofps_dd::Physics::getLoggerModuleName()
{
    return "Physics";
}


bool proofps_dd::Physics::colliding(const PureObject3D& a, const PureObject3D& b)
{
    return colliding2(
        a.getPosVec().getX(), a.getPosVec().getY(), a.getPosVec().getZ(),
        a.getScaledSizeVec().getX(), a.getScaledSizeVec().getY(), a.getScaledSizeVec().getZ(),
        b.getPosVec().getX(), b.getPosVec().getY(), b.getPosVec().getZ(),
        b.getScaledSizeVec().getX(), b.getScaledSizeVec().getY(), b.getScaledSizeVec().getZ()
    );
}

bool proofps_dd::Physics::colliding_NoZ(const PureObject3D& a, const PureObject3D& b)
{
    return colliding2_NoZ(
        a.getPosVec().getX(), a.getPosVec().getY(),
        a.getScaledSizeVec().getX(), a.getScaledSizeVec().getY(),
        b.getPosVec().getX(), b.getPosVec().getY(),
        b.getScaledSizeVec().getX(), b.getScaledSizeVec().getY()
    );
}

bool proofps_dd::Physics::colliding2(
    float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
    float o2px, float o2py, float o2pz, float o2sx, float o2sy, float o2sz)
{
    return (
        (
            (o1px - o1sx / 2 <= o2px + o2sx / 2)
            &&
            (o1px + o1sx / 2 >= o2px - o2sx / 2)
            )
        &&
        (
            (o1py - o1sy / 2 <= o2py + o2sy / 2)
            &&
            (o1py + o1sy / 2 >= o2py - o2sy / 2)
            )
        &&
        (
            (o1pz - o1sz / 2 <= o2pz + o2sz / 2)
            &&
            (o1pz + o1sz / 2 >= o2pz - o2sz / 2)
            )
        );
}

bool proofps_dd::Physics::colliding2_NoZ(
    float o1px, float o1py, float o1sx, float o1sy,
    float o2px, float o2py, float o2sx, float o2sy)
{
    return (
        (
            (o1px - o1sx / 2 <= o2px + o2sx / 2)
            &&
            (o1px + o1sx / 2 >= o2px - o2sx / 2)
            )
        &&
        (
            (o1py - o1sy / 2 <= o2py + o2sy / 2)
            &&
            (o1py + o1sy / 2 >= o2py - o2sy / 2)
            )
        );
}

float proofps_dd::Physics::distance_NoZ(
    float o1px, float o1py,
    float o2px, float o2py)
{
    const float fLengthX = o1px - o2px;
    const float fLengthY = o1py - o2py;
    return sqrt(fLengthX * fLengthX + fLengthY * fLengthY);
}

float proofps_dd::Physics::distance_NoZ(float o1px, float o1py, float o1sx, float o1sy, float o2px, float o2py)
{
    // first we check if O1 is colliding with O2, if yes then their distance is 0
    if (colliding2_NoZ(o1px, o1py, o1sx, o1sy, o2px, o2py, 0.1f, 0.1f))
    {
        return 0.f;
    }

    // if they are not colliding, we check the 4 points of O1 and return the distance to position of O2 center.
    // This could be further improved by using the 4 points of O2 too, however for now this is enough.
    float fRet = distance_NoZ(o1px + o1sx / 2.f, o1py + o1sy / 2.f, o2px, o2py);
    float fTmp = distance_NoZ(o1px + o1sx / 2.f, o1py - o1sy / 2.f, o2px, o2py);
    if (fTmp < fRet)
    {
        fRet = fTmp;
    }

    fTmp = distance_NoZ(o1px - o1sx / 2.f, o1py + o1sy / 2.f, o2px, o2py);
    if (fTmp < fRet)
    {
        fRet = fTmp;
    }

    fTmp = distance_NoZ(o1px - o1sx / 2.f, o1py - o1sy / 2.f, o2px, o2py);
    if (fTmp < fRet)
    {
        fRet = fTmp;
    }

    return fRet;
}

float proofps_dd::Physics::distance_NoZ_with_distancePerAxis(float o1px, float o1py, float o1sx, float o1sy, float o2px, float o2py, PureVector& vDirPerAxis, PureVector& vDistancePerAxis)
{
    vDirPerAxis.SetZ(0);
    // this could be cos()
    vDirPerAxis.SetX( o1px - o2px );
    vDirPerAxis.SetY( o1py - o2py );
    vDirPerAxis.Normalize();

    // first we check if O1 is colliding with O2, if yes then their distance is 0
    if (colliding2_NoZ(o1px, o1py, o1sx, o1sy, o2px, o2py, 0.1f, 0.1f))
    {
        vDistancePerAxis.SetZero();
        return 0.f;
    }

    // if they are not colliding, we check the 4 points of O1 and return their closest distance to position of O2 center.
    // This could be further improved by using the 4 points of O2 too, however for now this is enough.
    float fRetCenterDistance = distance_NoZ(o1px + o1sx / 2.f, o1py + o1sy / 2.f, o2px, o2py);
    vDistancePerAxis.Set(abs(o1px + o1sx / 2.f - o2px), abs(o1py + o1sy / 2.f - o2py), 0.f);
    float fTmpCenterDistance = distance_NoZ(o1px + o1sx / 2.f, o1py - o1sy / 2.f, o2px, o2py);
    if (fTmpCenterDistance < fRetCenterDistance)
    {
        fRetCenterDistance = fTmpCenterDistance;
        vDistancePerAxis.Set(abs(o1px + o1sx / 2.f - o2px), abs(o1py - o1sy / 2.f - o2py), 0.f);
    }

    fTmpCenterDistance = distance_NoZ(o1px - o1sx / 2.f, o1py + o1sy / 2.f, o2px, o2py);
    if (fTmpCenterDistance < fRetCenterDistance)
    {
        fRetCenterDistance = fTmpCenterDistance;
        vDistancePerAxis.Set(abs(o1px - o1sx / 2.f - o2px), abs(o1py + o1sy / 2.f - o2py), 0.f);
    }

    fTmpCenterDistance = distance_NoZ(o1px - o1sx / 2.f, o1py - o1sy / 2.f, o2px, o2py);
    if (fTmpCenterDistance < fRetCenterDistance)
    {
        fRetCenterDistance = fTmpCenterDistance;
        vDistancePerAxis.Set(abs(o1px - o1sx / 2.f - o2px), abs(o1py - o1sy / 2.f - o2py), 0.f);
    }

    return fRetCenterDistance;
}

bool proofps_dd::Physics::colliding3(
    const PureVector& vecPosMin, const PureVector& vecPosMax,
    const PureVector& vecObjPos, const PureVector& vecObjSize)
{
    const PureVector vecSize(
        vecPosMax.getX() - vecPosMin.getX(),
        vecPosMax.getY() - vecPosMin.getY(),
        vecPosMax.getZ() - vecPosMin.getZ()
    );
    const PureVector vecPos(
        vecPosMin.getX() + vecSize.getX() / 2.f,
        vecPosMin.getY() + vecSize.getY() / 2.f,
        vecPosMin.getZ() + vecSize.getZ() / 2.f
    );

    return colliding2(
        vecPos.getX(), vecPos.getY(), vecPos.getZ(),
        vecSize.getX(), vecSize.getY(), vecSize.getZ(),
        vecObjPos.getX(), vecObjPos.getY(), vecObjPos.getZ(),
        vecObjSize.getX(), vecObjSize.getY(), vecObjSize.getZ()
    );
}


// ############################## PROTECTED ##############################


void proofps_dd::Physics::serverSetAllowStrafeMidAir(bool bAllow)
{
    m_bAllowStrafeMidAir = bAllow;
}

void proofps_dd::Physics::serverSetAllowStrafeMidAirFull(bool bAllow)
{
    m_bAllowStrafeMidAirFull = bAllow;
}

void proofps_dd::Physics::serverSetFallDamageMultiplier(int n)
{
    m_nFallDamageMultiplier = n;
}

void proofps_dd::Physics::serverSetCollisionModeBvh(bool state)
{
    m_bCollisionModeBvh = state;
}

void proofps_dd::Physics::serverGravity(
    XHair& xhair,
    const unsigned int& nPhysicsRate,
    proofps_dd::GameMode& gameMode /* TODO: get rid of GameMode, Physics should not have it */ )
{   
    /* Although I tried to make calculations to have same result with different tickrate, the
       results are not the same, just SIMILAR when comparing 60 vs 20 Hz results.
       The real difference is during jumping:
        - for 60 Hz, GAME_GRAVITY_CONST should be 90.f,
        - for 20 Hz, GAME_GRAVITY_CONST should be 80.f to have same jumping.
       So I decided to define GAME_GRAVITY_CONST at runtime based on tickrate.
       Note that originally I wanted to lerp GAME_JUMP_GRAVITY_START as commented at its definition. */

    const float GAME_PHYSICS_RATE_LERP_FACTOR = (nPhysicsRate - GAME_TICKRATE_MIN) / static_cast<float>(GAME_TICKRATE_MAX - GAME_TICKRATE_MIN);
    const float GAME_GRAVITY_CONST = PFL::lerp(80.f /* 20 Hz */, 90.f /* 60 Hz */, GAME_PHYSICS_RATE_LERP_FACTOR);
    static constexpr float GAME_FALL_GRAVITY_MIN = -15.f;

    const float GAME_IMPACT_FORCE_Y_CHANGE = PFL::lerp(36.f, 50.f, GAME_PHYSICS_RATE_LERP_FACTOR);

    //const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        if (player.getRespawnFlag())
        {
            // do not do anything until server clears this flag!
            continue;
        }

        if (!gameMode.isPlayerAllowedForGameplay(player))
        {
            continue;
        }

        const float fPlayerImpactForceYChangePerTick = GAME_IMPACT_FORCE_Y_CHANGE / nPhysicsRate;
        if (player.getImpactForce().getY() > 0.f)
        {
            /* player.getImpactForce() is set in WeaponHandling::createExplosionServer() */
            player.getImpactForce().SetY(player.getImpactForce().getY() - fPlayerImpactForceYChangePerTick);
            if (player.getImpactForce().getY() < 0.f)
            {
                player.getImpactForce().SetY(0.f);
            }
        }
        else if (player.getImpactForce().getY() < 0.f)
        {
            player.getImpactForce().SetY(player.getImpactForce().getY() + fPlayerImpactForceYChangePerTick);
            if (player.getImpactForce().getY() > 0.f)
            {
                player.getImpactForce().SetY(0.f);
            }
        }

        player.setHasJustStartedFallingNaturallyInThisTick(false);
        player.setHasJustStartedFallingAfterJumpingStoppedInThisTick(false);
        const float fPlayerGravityChangePerTick = -GAME_GRAVITY_CONST / nPhysicsRate;

        player.setGravity(player.getGravity() + fPlayerGravityChangePerTick);
        if (player.isJumping())
        {
            if (player.getGravity() < 0.0f)
            {
                //getConsole().EOLn("stopped jumping up (natural): %f", player.getGravity());
                player.setGravity(0.f);
                player.stopJumping();
                player.getHasJustStoppedJumpingInThisTick() = true;
                player.setCanFall(true);
            }
        }
        else
        {
            if ((player.getGravity() < fPlayerGravityChangePerTick) && (player.getGravity() > 3 * fPlayerGravityChangePerTick))
            {
                // This means that we managed to decrease player gravity in 2 consecutive ticks, so we really just started to fall down.
                // We won't come here in next tick.
                if (player.getHasJustStoppedJumpingInThisTick())
                {
                    player.setHasJustStartedFallingAfterJumpingStoppedInThisTick(true);
                    //const auto nMillisecsJumpDuration =
                    //    std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - player.getTimeLastSetWillJump()).count();
                    //getConsole().EOLn("Started falling after jumping stopped: %f, jumping up duration: %d millisecs",
                    //    player.getGravity(),
                    //    nMillisecsJumpDuration);
                }
                else
                {
                    //getConsole().EOLn("Started falling naturally: %f", player.getGravity());
                    player.setHasJustStartedFallingNaturallyInThisTick(true);
                }
                player.getHasJustStoppedJumpingInThisTick() = false;
            }
            player.setCanFall(true);
            // player gravity cannot go below GAME_FALL_GRAVITY_MIN
            player.setGravity(std::max(player.getGravity(), GAME_FALL_GRAVITY_MIN));
        }

        if (!player.getCrouchInput().getOld() && player.getCrouchInput().getNew())
        {
            // player just initiated crouching by input
            player.doCrouchServer();
        } // end handle crouch

        // PPPKKKGGGGGG
        player.getPos().set(
            PureVector(
                player.getPos().getNew().getX(),
                player.getPos().getNew().getY() + player.getGravity() / nPhysicsRate + player.getImpactForce().getY() / nPhysicsRate,
                player.getPos().getNew().getZ()
            ));

        const auto& playerConst = player;
        if ((playerConst.getHealth() > 0) && (playerConst.getPos().getNew().getY() < m_maps.getBlockPosMin().getY() - 5.0f))
        {
            // need to die, out of map lower bound ... and this applies also when we player has invulnerability!

            // TODO: why we invoke handlePlayerDied() here?! Why dont we just set health to 0? handleUserUpdateFromServer() would invoke handlePlayerDied() anyway!
            // This is the only place where physics invokes handlePlayerDied() now.
            // In the future physics need to handle fall damage too, but that could also just set health to 0.
            // Note that we also need to consider who will know that upon dieing by falling down, splash die sound is needed to be played.
            // Because that would be also sent to clients so they can also play the sound. Maybe set death reason, I dont know.
            --player.getFrags(); // suicide
            ++player.getSuicides();
            handlePlayerDied(player, xhair, player.getServerSideConnectionHandle());

            //if (player.isFalling())
            //{
            //    const auto nFallDurationMillisecs = std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - player.getTimeStartedFalling()).count();
            //    getConsole().EOLn("Finished falling for %d millisecs, height: %f",
            //        static_cast<int>(nFallDurationMillisecs),
            //        player.getHeightStartedFalling() - player.getPos().getNew().getY());
            //}
        }
    }
}

void proofps_dd::Physics::serverPlayerCollisionWithWalls(
    const unsigned int& nPhysicsRate,
    XHair& xhair,
    proofps_dd::GameMode& gameMode /* TODO: get rid of GameMode, Physics should not have it */,
    PureVector& vecCamShakeForce)
{
    if (m_bCollisionModeBvh)
    {
        serverPlayerCollisionWithWalls_bvh(nPhysicsRate, xhair, gameMode, vecCamShakeForce);
    }
    else
    {
        serverPlayerCollisionWithWalls_legacy(nPhysicsRate, xhair, gameMode, vecCamShakeForce);
    }
} // serverPlayerCollisionWithWalls()


// ############################### PRIVATE ###############################


static constexpr float fHeightPlayerCanStillStepUpOnto = 0.3f;


bool proofps_dd::Physics::serverPlayerCollisionWithWalls_legacy_LoopKernelVertical(
    proofps_dd::Player& player,
    const PureObject3D* obj,
    const int& iJumppad /* -1 means no jumppad */,
    const float& fPlayerHalfHeight,
    const float& fPlayerOPos1XMinusHalf,
    const float& fPlayerOPos1XPlusHalf,
    const float& fPlayerPos1YMinusHalf,
    const float& fPlayerPos1YPlusHalf,
    const float& fBlockSizeXhalf,
    const float& fBlockSizeYhalf,
    XHair& xhair,
    PureVector& vecCamShakeForce
)
{
    // TODO: RFR: this is 99% same as the bvh version, let's refactor!
    ScopeBenchmarker<std::chrono::microseconds> bm(__func__);

    assert(obj);

    if ((obj->getPosVec().getX() + fBlockSizeXhalf < fPlayerOPos1XMinusHalf) || (obj->getPosVec().getX() - fBlockSizeXhalf > fPlayerOPos1XPlusHalf))
    {
        return false;
    }

    if ((obj->getPosVec().getY() + fBlockSizeYhalf < fPlayerPos1YMinusHalf) || (obj->getPosVec().getY() - fBlockSizeYhalf > fPlayerPos1YPlusHalf))
    {
        return false;
    }

    const int nAlignUnderOrAboveWall = obj->getPosVec().getY() < player.getPos().getOld().getY() ? 1 : -1;
    const float fAlignCloseToWall = nAlignUnderOrAboveWall * (fBlockSizeYhalf + fPlayerHalfHeight + 0.01f);
    // TODO: we could write this simpler if PureVector::Set() would return the object itself!
    // e.g.: player.getPos().set( PureVector(player.getPos().getNew()).setY(obj->getPosVec().getY() + fAlignCloseToWall) )
    // do this everywhere where Ctrl+F finds this text (in Project): PPPKKKGGGGGG
    player.getPos().set(
        PureVector(
            player.getPos().getNew().getX(),
            obj->getPosVec().getY() + fAlignCloseToWall,
            player.getPos().getNew().getZ()
        ));

    if (nAlignUnderOrAboveWall == 1)
    {
        // we fell from above
        const bool bOriginalFalling = player.isFalling();
        float fFallHeight = 0.f;
        int nDamage = 0;

        // handle fall damage
        if ((std::as_const(player).getHealth() > 0) && (player.isFalling()) && (iJumppad == -1) /* no fall damage when falling on jumppad */)
        {
            //const auto nFallDurationMillisecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - player.getTimeStartedFalling()).count();
            fFallHeight = player.getHeightStartedFalling() - player.getPos().getNew().getY();
            static constexpr float fFallDamageFromHeight = 3.5f;  // should not have fall damage from doing mid-air salto with 1.5x jump-force multiplier
            if (fFallHeight > fFallDamageFromHeight)
            {
                nDamage = static_cast<int>(std::lroundf((fFallHeight - fFallDamageFromHeight) * m_nFallDamageMultiplier));
                // player invulnerability does NOT affect physics damage so we always do damage here!
                player.doDamage(nDamage, nDamage);
                if (std::as_const(player).getHealth() == 0)
                {
                    // server handles death here, clients will handle it when they receive MsgUserUpdateFromServer
                    handlePlayerDied(player, xhair, player.getServerSideConnectionHandle());
                }
            }

            //getConsole().EOLn("Finished falling for %d millisecs, height: %f, damage: %d",
            //    static_cast<int>(nFallDurationMillisecs),
            //    fFallHeight,
            //    nDamage);
        }

        if (bOriginalFalling)
        {
            // now handleLanded() has both the server- and client-side logic, this design should be the future design for most game logic
            player.handleLanded(fFallHeight, nDamage > 0, std::as_const(player).getHealth() == 0, vecCamShakeForce, isMyConnection(player.getServerSideConnectionHandle()));
        }
        player.setCanFall(false);

        // maybe not null everything out in the future but only decrement the components by
        // some value, since if there is an explosion-induced force, it shouldnt be nulled out
        // at this moment. Currently we want to null out the strafe-jump-induced force.
        // Update in v0.2.0.0: I decided to use separate vector for explosion-induced force, looks like
        // we can zero out jumpforce here completely!
        player.getJumpForce().Set(0.f, 0.f, 0.f);
        player.setGravity(0.f);

        if (iJumppad >= 0)
        {
            // this way jump() will be executed by caller main serverPlayerCollisionWithWalls() func, in same tick
            player.setWillJumpInNextTick(m_maps.getJumppadForceFactors(iJumppad).y, m_maps.getJumppadForceFactors(iJumppad).x);
            player.handleJumppadActivated();
        }
    }
    else
    {
        // we hit ceiling with our head during jumping
        //getConsole().EOLn("start falling (hit ceiling)");
        player.setCanFall(true);
        player.stopJumping();
        player.getHasJustStoppedJumpingInThisTick() = true;
        player.setGravity(0.f);
    }

    return true;
} // serverPlayerCollisionWithWalls_legacy_LoopKernelVertical()


bool proofps_dd::Physics::serverPlayerCollisionWithWalls_bvh_LoopKernelVertical(
    Player& player,
    const PureObject3D* obj,
    const int& iJumppad,
    const float& fPlayerHalfHeight,
    const float& fBlockSizeYhalf,
    XHair& xhair,
    PureVector& vecCamShakeForce)
{
    // TODO: RFR: this is 99% same as the legacy version, let's refactor!
    ScopeBenchmarker<std::chrono::microseconds> bm(__func__);

    assert(obj);

    const int nAlignUnderOrAboveWall = obj->getPosVec().getY() < player.getPos().getOld().getY() ? 1 : -1;
    const float fAlignCloseToWall = nAlignUnderOrAboveWall * (fBlockSizeYhalf + fPlayerHalfHeight + 0.01f);
    // TODO: we could write this simpler if PureVector::Set() would return the object itself!
    // e.g.: player.getPos().set( PureVector(player.getPos().getNew()).setY(obj->getPosVec().getY() + fAlignCloseToWall) )
    // do this everywhere where Ctrl+F finds this text (in Project): PPPKKKGGGGGG
    player.getPos().set(
        PureVector(
            player.getPos().getNew().getX(),
            obj->getPosVec().getY() + fAlignCloseToWall,
            player.getPos().getNew().getZ()
        ));

    if (nAlignUnderOrAboveWall == 1)
    {
        // we fell from above
        const bool bOriginalFalling = player.isFalling();
        float fFallHeight = 0.f;
        int nDamage = 0;

        // handle fall damage
        if ((std::as_const(player).getHealth() > 0) && (player.isFalling()) && (iJumppad == -1) /* no fall damage when falling on jumppad */)
        {
            //const auto nFallDurationMillisecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - player.getTimeStartedFalling()).count();
            fFallHeight = player.getHeightStartedFalling() - player.getPos().getNew().getY();
            static constexpr float fFallDamageFromHeight = 3.5f;  // should not have fall damage from doing mid-air salto with 1.5x jump-force multiplier
            if (fFallHeight > fFallDamageFromHeight)
            {
                nDamage = static_cast<int>(std::lroundf((fFallHeight - fFallDamageFromHeight) * m_nFallDamageMultiplier));
                // player invulnerability does NOT affect physics damage so we always do damage here!
                player.doDamage(nDamage, nDamage);
                if (std::as_const(player).getHealth() == 0)
                {
                    // server handles death here, clients will handle it when they receive MsgUserUpdateFromServer
                    handlePlayerDied(player, xhair, player.getServerSideConnectionHandle());
                }
            }

            //getConsole().EOLn("Finished falling for %d millisecs, height: %f, damage: %d",
            //    static_cast<int>(nFallDurationMillisecs),
            //    fFallHeight,
            //    nDamage);
        }

        if (bOriginalFalling)
        {
            // now handleLanded() has both the server- and client-side logic, this design should be the future design for most game logic
            player.handleLanded(fFallHeight, nDamage > 0, std::as_const(player).getHealth() == 0, vecCamShakeForce, isMyConnection(player.getServerSideConnectionHandle()));
        }
        player.setCanFall(false);

        // maybe not null everything out in the future but only decrement the components by
        // some value, since if there is an explosion-induced force, it shouldnt be nulled out
        // at this moment. Currently we want to null out the strafe-jump-induced force.
        // Update in v0.2.0.0: I decided to use separate vector for explosion-induced force, looks like
        // we can zero out jumpforce here completely!
        player.getJumpForce().Set(0.f, 0.f, 0.f);
        player.setGravity(0.f);

        if (iJumppad >= 0)
        {
            // this way jump() will be executed by caller main serverPlayerCollisionWithWalls() func, in same tick
            player.setWillJumpInNextTick(m_maps.getJumppadForceFactors(iJumppad).y, m_maps.getJumppadForceFactors(iJumppad).x);
            player.handleJumppadActivated();
        }
    }
    else
    {
        // we hit ceiling with our head during jumping
        //getConsole().EOLn("start falling (hit ceiling)");
        player.setCanFall(true);
        player.stopJumping();
        player.getHasJustStoppedJumpingInThisTick() = true;
        player.setGravity(0.f);
    }

    return true;
}

void proofps_dd::Physics::serverPlayerCollisionWithWalls_strafe(
    const unsigned int& nPhysicsRate,
    Player& player,
    PureVector vecOriginalJumpForceBeforeVerticalCollisionHandled /* yes, copy it in */)
{
    ScopeBenchmarker<std::chrono::microseconds> bm(__func__);

    const float GAME_PLAYER_SPEED_WALK = Player::fBaseSpeedWalk / nPhysicsRate;
    const float GAME_PLAYER_SPEED_RUN = Player::fBaseSpeedRun / nPhysicsRate;
    const float GAME_PLAYER_SPEED_CROUCH = Player::fBaseSpeedCrouch / nPhysicsRate;

    static unsigned int nContinuousStrafeCountForDebugServerPlayerMovement = 0;

    const auto& playerConst = player;
    if ((playerConst.getHealth() > 0) && (player.getStrafe() != proofps_dd::Strafe::NONE))
    {
        float fTargetStrafeSpeed =
            player.getCrouchStateCurrent() ?
            (player.isSomersaulting() ? GAME_PLAYER_SPEED_RUN : GAME_PLAYER_SPEED_CROUCH) :
            (player.isRunning() ? GAME_PLAYER_SPEED_RUN : GAME_PLAYER_SPEED_WALK);
        if (player.getStrafe() == proofps_dd::Strafe::LEFT)
        {
            fTargetStrafeSpeed = -fTargetStrafeSpeed;
        }

        // in case of strafe input direction change, always start changing strafe speed from 0, otherwise it will feel like
        // we have thruster that we need to work against :D (jetpack-like)
        if (((fTargetStrafeSpeed > 0.f) && (player.getStrafeSpeed() < 0.f))
            ||
            ((fTargetStrafeSpeed < 0.f) && (player.getStrafeSpeed() > 0.f)))
        {
            player.getStrafeSpeed() = 0.f;
        }

        // using same way of calculation here as in serverGravity()
        const float GAME_PHYSICS_RATE_LERP_FACTOR = (nPhysicsRate - GAME_TICKRATE_MIN) / static_cast<float>(GAME_TICKRATE_MAX - GAME_TICKRATE_MIN);
        const float GAME_STRAFE_PHYSICS_RATE_DIVIDER = PFL::lerp(5.f /* 20 Hz */, 10.f /* 60 Hz */, GAME_PHYSICS_RATE_LERP_FACTOR);
        // unlike as in serverGravity() here I divide by the lerped value instead of multiply, I dont know why I multiply in serverGravity() anyway, but
        // obviously I need to divide here as lower physics rate results in higher strafe speeds, need to have the per-tick change higher also!
        const float fPlayerStrafeChangePerTick =
            fTargetStrafeSpeed / GAME_STRAFE_PHYSICS_RATE_DIVIDER
            /* no need to divide by nPhysicsRate as those const values assigned to fTargetStrafeSpeed are already divided by it */;
        player.getStrafeSpeed() += fPlayerStrafeChangePerTick;

        // always limit strafe speed to target strafe speed
        if (((fTargetStrafeSpeed > 0.f) && (player.getStrafeSpeed() > fTargetStrafeSpeed))
            ||
            ((fTargetStrafeSpeed < 0.f) && (player.getStrafeSpeed() < fTargetStrafeSpeed)))
        {
            player.getStrafeSpeed() = fTargetStrafeSpeed;
        }

        if (player.getHasJustStartedFallingNaturallyInThisTick())
        {
            player.getJumpForce().SetX(player.getStrafeSpeed());
            vecOriginalJumpForceBeforeVerticalCollisionHandled = player.getJumpForce();
        }

        if (!player.isInAir() ||
            (m_bAllowStrafeMidAir &&
                (
                    /* if jump was initiated without horizontal force or we nulled it out due to hitting a wall */
                    (vecOriginalJumpForceBeforeVerticalCollisionHandled.getX() == 0.f) ||
                    /* if we have horizontal jump force, we cannot add more to it in the same direction */
                    ((vecOriginalJumpForceBeforeVerticalCollisionHandled.getX() > 0.f) && (player.getStrafeSpeed() < 0.f)) ||
                    ((vecOriginalJumpForceBeforeVerticalCollisionHandled.getX() < 0.f) && (player.getStrafeSpeed() > 0.f))
                    ))
            )
        {
            ++nContinuousStrafeCountForDebugServerPlayerMovement;
            //getConsole().EOLn("Tick Strafe");

            if (m_bAllowStrafeMidAirFull)
            {
                // cancel the saved horizontal force, let the player have full control over mid-air strafe
                player.getJumpForce().SetX(0.f);
                vecOriginalJumpForceBeforeVerticalCollisionHandled = player.getJumpForce();
            }

            // PPPKKKGGGGGG
            player.getPos().set(
                PureVector(
                    player.getPos().getNew().getX() + player.getStrafeSpeed(),
                    player.getPos().getNew().getY(),
                    player.getPos().getNew().getZ()
                ));
        }
    }
    else
    {
        if (nContinuousStrafeCountForDebugServerPlayerMovement > 0)
        {
            //getConsole().EOLn("Strafe stopped after: %u consecutive strafes", nContinuousStrafeCountForDebugServerPlayerMovement);
            nContinuousStrafeCountForDebugServerPlayerMovement = 0;
        }
    }

    // serverPlayerCollisionWithWalls_LoopKernelVertical() could had also set WillJumpInNextTick() to true (i.e. non-0) above, in that case, we are
    // jumping in the same tick as that condition was detected.
    // However, we also handle player-input-induced jumping here, in that case we are 1 frame late.
    // We should handle it at the beginning of the Gravity() function, to actually start jumping in the same frame as when we detected the
    // player initiated the jumping.
    // However, we are handling it here because X-pos is updated by strafe here so here we will have actually different new and old X-pos,
    // that is essential for the jump() function below to record the X-forces for the player.
    // For now this 1 frame latency is not critical so I'm not planning to change that. Might be addressed in the future though.
    if (player.getWillJumpYInNextTick() > 0.f)
    {
        //getConsole().EOLn("start jumping");
        // now we can actually jump and have the correct forces be saved for the jump
        player.jump(GAME_PLAYER_SPEED_RUN); // resets setWillJumpInNextTick()
    }

    // PPPKKKGGGGGG
    player.getPos().set(
        /* we use the vecOriginalJumpForceBeforeVerticalCollisionHandled instead of current force because
           current force might had been zeroed out above in the moment of falling on the ground, however here
           we still have to add the original force to be perfect.
           If we dont do this, there will be a 1 frame outage in the continuity of strafe movement at
           the moment of finishing the jump because serverHandleUserCmdMoveFromClient() in this frame did not allow strafe
           due to ongoing jumping, while this current function terminates jumping above before we could
           apply the force here for last time - hence we use the original force at the beginning of function. */
        PureVector(
            player.getPos().getNew().getX() + vecOriginalJumpForceBeforeVerticalCollisionHandled.getX(),
            player.getPos().getNew().getY(),
            player.getPos().getNew().getZ()
        ));

    const float GAME_PHYSICS_RATE_LERP_FACTOR = (nPhysicsRate - GAME_TICKRATE_MIN) / static_cast<float>(GAME_TICKRATE_MAX - GAME_TICKRATE_MIN);
    const float GAME_IMPACT_FORCE_X_CHANGE = PFL::lerp(25.f, 26.f, GAME_PHYSICS_RATE_LERP_FACTOR);
    const float fPlayerImpactForceXChangePerTick = GAME_IMPACT_FORCE_X_CHANGE / nPhysicsRate;
    if (player.getImpactForce().getX() > 0.f)
    {
        /* player.getImpactForce() is set in WeaponHandling::createExplosionServer() */
        player.getImpactForce().SetX(player.getImpactForce().getX() - fPlayerImpactForceXChangePerTick);
        if (player.getImpactForce().getX() < 0.f)
        {
            player.getImpactForce().SetX(0.f);
        }
    }
    else if (player.getImpactForce().getX() < 0.f)
    {
        player.getImpactForce().SetX(player.getImpactForce().getX() + fPlayerImpactForceXChangePerTick);
        if (player.getImpactForce().getX() > 0.f)
        {
            player.getImpactForce().SetX(0.f);
        }
    }

    // PPPKKKGGGGGG
    player.getPos().set(
        PureVector(
            player.getPos().getNew().getX() + player.getImpactForce().getX() / nPhysicsRate,
            player.getPos().getNew().getY(),
            player.getPos().getNew().getZ()
        ));
}

void proofps_dd::Physics::serverPlayerCollisionWithWalls_legacy(const unsigned int& nPhysicsRate, XHair& xhair, proofps_dd::GameMode& gameMode, PureVector& vecCamShakeForce)
{
    ScopeBenchmarker<std::chrono::microseconds> bm_main(__func__);
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        if (player.getRespawnFlag())
        {
            // do not do anything until server clears this flag!
            continue;
        }

        if (!gameMode.isPlayerAllowedForGameplay(player))
        {
            continue;
        }

        const PureObject3D* const plobj = player.getObject3D();
        // we need the CURRENT jump force LATER below for strafe movement, save it because vertical collision handling might change it!
        const PureVector vecOriginalJumpForceBeforeVerticalCollisionHandled = player.getJumpForce();

        // how to make collision detection even faster:
        // if we dont want to use spatial hierarchy like BVH, just store the map elements in a matrix that we can address with i and j,
        // and based on player's position it is very easy to know which few map elements around matrix[i][j] should be checked ...
        // And I'm also thinking that not pointers but the objects themselves could be stored in matrix, that way the whole matrix
        // could be fetched into cache for even faster iteration on its elements ...

        constexpr float fUsualBlockSizeXhalf = proofps_dd::Maps::fMapBlockSizeWidth / 2.f;
        constexpr float fUsualBlockSizeYhalf = proofps_dd::Maps::fMapBlockSizeHeight / 2.f;
        const PureVector vecPlayerScaledSize = plobj->getScaledSizeVec();
        const float fPlayerHalfHeight = vecPlayerScaledSize.getY() / 2.f;

        // At this point, player.getPos().getY() is already updated by Gravity().
        // We use Player's Object3D scaling since that is used in physics calculations also in serverGravity(),
        // but we dont need to set Object3D position because Player object has its own position vector that is used in physics.
        // Object3D is then repositioned to Player's own position vector.
        // On the long run we should use colliders so physics does not depend on graphics.
        const float fPlayerOPos1XMinusHalf = player.getPos().getOld().getX() - vecPlayerScaledSize.getX() / 2.f;
        const float fPlayerOPos1XPlusHalf = player.getPos().getOld().getX() + vecPlayerScaledSize.getX() / 2.f;
        const float fPlayerPos1YMinusHalf = player.getPos().getNew().getY() - fPlayerHalfHeight;
        const float fPlayerPos1YPlusHalf = player.getPos().getNew().getY() + fPlayerHalfHeight;
        if (player.getPos().getOld().getY() != player.getPos().getNew().getY())
        {
            ScopeBenchmarker<std::chrono::microseconds> bm("legacy vertical collision");
            // first we check collision with jump pads, because it is faster to check, and if we collide, we can skip further
            // check for vertical collision with regular foreground blocks.
            // Also, actually we need to check with jump pads first, because otherwise if we have vertical collision with a
            // regular block and with jump pad at the same time, it won't make us jump if we handle the collision with the
            // regular one first and break from the loop immediately then.
            bool bCollided = false;
            for (size_t iJumppad = 0; iJumppad < m_maps.getJumppads().size(); iJumppad++)
            {
                assert(m_maps.getJumppads()[iJumppad]);  // we dont store nulls there
                bCollided = serverPlayerCollisionWithWalls_legacy_LoopKernelVertical(
                    player,
                    m_maps.getJumppads()[iJumppad],
                    static_cast<int>(iJumppad),
                    fPlayerHalfHeight,
                    fPlayerOPos1XMinusHalf,
                    fPlayerOPos1XPlusHalf,
                    fPlayerPos1YMinusHalf,
                    fPlayerPos1YPlusHalf,
                    fUsualBlockSizeXhalf,
                    fUsualBlockSizeYhalf,
                    xhair,
                    vecCamShakeForce);

                if (bCollided)
                {
                    // there is no need to check further, since we handle collision with only 1 jumppad at a time
                    break;
                }
            } // end for jumppads

            for (int i = 0; !bCollided && (i < m_maps.getForegroundBlockCount()); i++)
            {
                const PureObject3D* const pObj = m_maps.getForegroundBlocks()[i];
                assert(pObj);  // we dont store nulls there

                const auto itJumppad = std::find(m_maps.getJumppads().begin(), m_maps.getJumppads().end(), pObj);
                if (itJumppad != m_maps.getJumppads().end())
                {
                    // we already checked them in above loop
                    continue;
                }

                bCollided = serverPlayerCollisionWithWalls_legacy_LoopKernelVertical(
                    player,
                    pObj,
                    -1 /* invalid jumppad index */,
                    fPlayerHalfHeight,
                    fPlayerOPos1XMinusHalf,
                    fPlayerOPos1XPlusHalf,
                    fPlayerPos1YMinusHalf,
                    fPlayerPos1YPlusHalf,
                    pObj->getSizeVec().getX() / 2.f,
                    pObj->getSizeVec().getY() / 2.f,
                    xhair,
                    vecCamShakeForce);
            } // end for i

            if (!bCollided && player.isFalling() && (std::as_const(player).getHealth() > 0))
            {
                // we have been in the air for a while now

                const float fFallHeight = player.getHeightStartedFalling() - player.getPos().getNew().getY();
                if (fFallHeight >= 6.f)
                {
                    player.handleFallingFromHigh(0 /* relevant for clients, but for server it is unused as input, server will update this parameter within function */);
                }
            }
        } // end if YPos changed

        if (player.isSomersaulting())
        {
            const float GAME_PLAYER_SOMERSAULT_ROTATE_STEP = 360.f / nPhysicsRate * (1000.f / Player::nSomersaultTargetDurationMillisecs);
            player.stepSomersaultAngleServer(GAME_PLAYER_SOMERSAULT_ROTATE_STEP);
        }

        // TODO: RFR: when this standup part is extracted into a function, it should return the final selected Y size of the player so
        // we dont have to query it again later.
        if (player.getWantToStandup() && !player.isSomersaulting())
        {
            if (player.getCrouchStateCurrent())
            {
                // we need to check if there is enough space to stand up
                constexpr float fProposedNewPlayerHalfHeight = Player::fObjHeightStanding / 2.f;
                const float fProposedNewPlayerPosY = player.getProposedNewPosYforStandup();
                const float fProposedNewPlayerPos1YMinusHalf = fProposedNewPlayerPosY - fProposedNewPlayerHalfHeight;
                const float fProposedNewPlayerPos1YPlusHalf = fProposedNewPlayerPosY + fProposedNewPlayerHalfHeight;
                bool bCanStandUp = true;
                for (int i = 0; i < m_maps.getForegroundBlockCount(); i++)
                {
                    const PureObject3D* const obj = m_maps.getForegroundBlocks()[i];
                    assert(obj);  // we dont store nulls there

                    const float fRealBlockSizeXhalf = obj->getSizeVec().getX() / 2.f;
                    const float fRealBlockSizeYhalf = obj->getSizeVec().getY() / 2.f;
                    const PureVector& vecFgBlockPos = obj->getPosVec();

                    if ((vecFgBlockPos.getX() + fRealBlockSizeXhalf < fPlayerOPos1XMinusHalf) || (vecFgBlockPos.getX() - fRealBlockSizeXhalf > fPlayerOPos1XPlusHalf))
                    {
                        continue;
                    }

                    if ((vecFgBlockPos.getY() + fRealBlockSizeYhalf < fProposedNewPlayerPos1YMinusHalf) || (vecFgBlockPos.getY() - fRealBlockSizeYhalf > fProposedNewPlayerPos1YPlusHalf))
                    {
                        continue;
                    }

                    // found a blocking object, cannot stand up
                    bCanStandUp = false;
                    break;
                } // end for i
                if (bCanStandUp)
                {
                    player.doStandupServer();
                }
            }
        } // end if (player.getWantToStandup())

        serverPlayerCollisionWithWalls_strafe(nPhysicsRate, player, vecOriginalJumpForceBeforeVerticalCollisionHandled);

        const float fPlayerPos1XMinusHalf = player.getPos().getNew().getX() - plobj->getSizeVec().getX() / 2.f;
        const float fPlayerPos1XPlusHalf = player.getPos().getNew().getX() + plobj->getSizeVec().getX() / 2.f;
        // TODO: I think here we shall introduce a fPlayerHalfHeight2 because if above we stood up then we need to fetch updated height!
        // But for now this is considered a corner case I'm not handling.
        const float fPlayerPos1YMinusHalf_2 = player.getPos().getNew().getY() - fPlayerHalfHeight;
        const float fPlayerPos1YPlusHalf_2 = player.getPos().getNew().getY() + fPlayerHalfHeight;

        bool bHorizontalCollisionOccured = false;
        if (player.getPos().getOld().getX() != player.getPos().getNew().getX())
        {
            ScopeBenchmarker<std::chrono::microseconds> bm("legacy horizontal collision");

            for (int i = 0; i < m_maps.getForegroundBlockCount(); i++)
            {
                // TODO: RFR: we can introduce a HorizontalKernel function similar to the VerticalKernel stuff above
                const PureObject3D* const obj = m_maps.getForegroundBlocks()[i];
                assert(obj);  // we dont store nulls there

                const float fRealBlockSizeXhalf = obj->getSizeVec().getX() / 2.f;
                const float fRealBlockSizeYhalf = obj->getSizeVec().getY() / 2.f;
                const PureVector& vecFgBlockPos = obj->getPosVec();

                if ((vecFgBlockPos.getX() + fRealBlockSizeXhalf < fPlayerPos1XMinusHalf) || (vecFgBlockPos.getX() - fRealBlockSizeXhalf > fPlayerPos1XPlusHalf))
                {
                    continue;
                }

                if ((vecFgBlockPos.getY() + fRealBlockSizeYhalf < fPlayerPos1YMinusHalf_2) || (vecFgBlockPos.getY() - fRealBlockSizeYhalf > fPlayerPos1YPlusHalf_2))
                {
                    continue;
                }

                // horizontal collision occurred BUT its effect might be cancelled if we can step up on the object

                // TODO: RFR: this part down below is again same as in serverPlayerCollisionWithWalls_bvh()

                // maybe this is a stairstep we can step onto
                if (!player.isFalling() && !player.canFall() && ((vecFgBlockPos.getY() + fRealBlockSizeYhalf) - fPlayerPos1YMinusHalf_2) <= fHeightPlayerCanStillStepUpOnto)
                {
                    // TODO: check if there is enough space to step onto the stairstep!
                    // For now I'm not doing it, it would introduce additional slowdown when stepping up on each stairstep and currently this is
                    // considered as a corner case.
                
                    // PPPKKKGGGGGG
                    player.getPos().set(
                        PureVector(
                            player.getPos().getNew().getX(),
                            vecFgBlockPos.getY() + fRealBlockSizeYhalf + fPlayerHalfHeight + 0.01f,
                            player.getPos().getNew().getZ()
                        ));
                }
                else
                {
                    // could not step up onto the object so we set the flag here, important for proper run sound kept playing on stairs!
                    bHorizontalCollisionOccured = true;

                    if (m_bAllowStrafeMidAir || player.canFall())
                    {
                        // Horizontal collision must stop horizontal jump-induced force (unlike explosion-induced force).
                        // However, in case of m_bAllowStrafeMidAir == false it would make it impossible to jump on a box
                        // when jump is initiated from right next to the box. So as a cheat we allow keeping the horizontal
                        // jump-induced force for the period of jumping and just zero it out at the moment of starting to fall.
                        player.getJumpForce().SetX(0.f);
                    }

                    // in case of horizontal collision, we should not reposition to previous position, but align next to the wall
                    const int nAlignLeftOrRightToWall = vecFgBlockPos.getX() < player.getPos().getOld().getX() ? 1 : -1;
                    const float fAlignNextToWall = nAlignLeftOrRightToWall * (obj->getSizeVec().getX() / 2 + proofps_dd::Player::fObjWidth / 2.0f + 0.01f);
                    //getConsole().EOLn(
                    //    "x align to wall: old pos x: %f, new pos x: %f, fAlignNextToWall: %f",
                    //    player.getPos().getOld().getX(),
                    //    player.getPos().getNew().getX(),
                    //    fAlignNextToWall);
                    // PPPKKKGGGGGG
                    player.getPos().set(
                        PureVector(
                            vecFgBlockPos.getX() + fAlignNextToWall,
                            player.getPos().getNew().getY(),
                            player.getPos().getNew().getZ()
                        ));
                } // endif can step onto it or not

                break;
            } // end for i
        } // end XPos changed

        // TODO: RFR: this part is also came as in serverPlayerCollisionWithWalls_bvh()
        if (!bHorizontalCollisionOccured)
        {
            if (player.getWillSomersaultInNextTick())
            {
                // only here can we really trigger on-ground somersaulting
                player.startSomersaultServer(false);
            }

            if (!player.isInAir() && (player.getStrafeSpeed() != 0.f) && player.isRunning() && !player.getCrouchStateCurrent())
            {
                player.getActuallyRunningOnGround().set(true);
            }
            else
            {
                player.getActuallyRunningOnGround().set(false);
            }
        }
        else
        {
            player.getActuallyRunningOnGround().set(false);
        }
    } // end for player
} // serverPlayerCollisionWithWalls_legacy()


void proofps_dd::Physics::serverPlayerCollisionWithWalls_bvh(const unsigned int& nPhysicsRate, XHair& xhair, proofps_dd::GameMode& gameMode, PureVector& vecCamShakeForce)
{
    static std::vector<const PureObject3D*> colliders;
    ScopeBenchmarker<std::chrono::microseconds> bm_main(__func__);
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        if (player.getRespawnFlag())
        {
            // do not do anything until server clears this flag!
            continue;
        }

        if (!gameMode.isPlayerAllowedForGameplay(player))
        {
            continue;
        }

        const PureObject3D* const plobj = player.getObject3D();
        // we need the CURRENT jump force LATER below for strafe movement, save it because vertical collision handling might change it!
        const PureVector vecOriginalJumpForceBeforeVerticalCollisionHandled = player.getJumpForce();

        const PureVector vecPlayerScaledSize = plobj->getScaledSizeVec();
        const float fPlayerHalfHeight = vecPlayerScaledSize.getY() / 2.f;

        // At this point, player.getPos().getY() is already updated by Gravity().
        // We use Player's Object3D scaling since that is used in physics calculations also in serverGravity(),
        // but we dont need to set Object3D position because Player object has its own position vector that is used in physics.
        // Object3D is then repositioned to Player's own position vector.
        // On the long run we should use colliders so physics does not depend on graphics.

        if (player.getPos().getOld().getY() != player.getPos().getNew().getY())
        {
            ScopeBenchmarker<std::chrono::microseconds> bm("bvh vertical collision");

            const PureAxisAlignedBoundingBox aabbPlayer(
                PureVector(player.getPos().getOld().getX(), player.getPos().getNew().getY(), player.getPos().getNew().getZ()),
                PureVector(vecPlayerScaledSize.getX(), vecPlayerScaledSize.getY(), vecPlayerScaledSize.getZ()));
            const bool bVerticalCollisionOccured = m_maps.getBVH().findAllColliderObjects(aabbPlayer, nullptr, colliders);
            if (bVerticalCollisionOccured)
            {
                assert(!colliders.empty());

                // first we check collision with jump pads, because it is faster to check, and if we collide, we can skip further
                // check for vertical collision with regular foreground blocks.
                // We need to check vertical collision _with jump pads first_, because otherwise if we have vertical collision with a
                // regular block and with jump pad at the same time, it won't make us jump if we handle the collision with a
                // single regular block.
                // So we find all colliders and check if there is jump pad there:
                // - if yes, handle it and stop the vertical collision checking;
                // - if no, continue with handling vertical collision with the 1st found object(any other object will be at same Y-pos / -size anyway).

                int iCollidedWithJumppad = -1;
                const PureObject3D* pObj = *colliders.begin(); // if no jumppad collision is detected in the loop below, then we handle collision with this
                for (size_t iCollider = 0; (iCollider < colliders.size()) && (iCollidedWithJumppad == -1); iCollider++)
                {
                    for (size_t iJumppad = 0; (iJumppad < m_maps.getJumppads().size()) && (iCollidedWithJumppad == -1); iJumppad++)
                    {
                        if (m_maps.getJumppads()[iJumppad] == colliders[iCollider])
                        {
                            // vertical collision with a jump pad occurred
                            iCollidedWithJumppad = iJumppad;
                            pObj = colliders[iCollider];
                        }
                    }

                }

                serverPlayerCollisionWithWalls_bvh_LoopKernelVertical(
                    player,
                    pObj,
                    iCollidedWithJumppad,
                    fPlayerHalfHeight,
                    pObj->getSizeVec().getY() / 2.f,
                    xhair,
                    vecCamShakeForce);
            } // endif bVerticalCollisionOccured

            if (!bVerticalCollisionOccured && player.isFalling() && (std::as_const(player).getHealth() > 0))
            {
                // we have been in the air for a while now

                const float fFallHeight = player.getHeightStartedFalling() - player.getPos().getNew().getY();
                if (fFallHeight >= 6.f)
                {
                    player.handleFallingFromHigh(0 /* relevant for clients, but for server it is unused as input, server will update this parameter within function */);
                }
            }
        } // end if YPos changed

        if (player.isSomersaulting())
        {
            const float GAME_PLAYER_SOMERSAULT_ROTATE_STEP = 360.f / nPhysicsRate * (1000.f / Player::nSomersaultTargetDurationMillisecs);
            player.stepSomersaultAngleServer(GAME_PLAYER_SOMERSAULT_ROTATE_STEP);
        }

        // TODO: RFR: when this standup part is extracted into a function, it should return the final selected Y size of the player so
        // we dont have to query it again later.
        if (player.getWantToStandup() && !player.isSomersaulting())
        {
            if (player.getCrouchStateCurrent())
            {
                // we need to check if there is enough space to stand up
                constexpr float fProposedNewPlayerHalfHeight = Player::fObjHeightStanding / 2.f;
                const PureAxisAlignedBoundingBox aabbPlayer(
                    PureVector(player.getPos().getOld().getX(), player.getProposedNewPosYforStandup(), player.getPos().getNew().getZ()),
                    PureVector(plobj->getSizeVec().getX(), fProposedNewPlayerHalfHeight, plobj->getSizeVec().getZ()));
                const bool bCanStandUp = (m_maps.getBVH().findOneColliderObject(aabbPlayer, nullptr) == nullptr);
                if (bCanStandUp)
                {
                    player.doStandupServer();
                }
            }
        } // end if (player.getWantToStandup())

        serverPlayerCollisionWithWalls_strafe(nPhysicsRate, player, vecOriginalJumpForceBeforeVerticalCollisionHandled);

        bool bHorizontalCollisionOccured = false;
        if (player.getPos().getOld().getX() != player.getPos().getNew().getX())                                     
        {
            ScopeBenchmarker<std::chrono::microseconds> bm("bvh horizontal collision");

            const PureAxisAlignedBoundingBox aabbPlayer(
                PureVector(player.getPos().getNew().getX(), player.getPos().getNew().getY(), player.getPos().getNew().getZ()),
                PureVector(plobj->getSizeVec().getX(), plobj->getScaledSizeVec().getY(), plobj->getSizeVec().getZ()));
            const PureObject3D* const pWallObj = m_maps.getBVH().findOneColliderObject(aabbPlayer, nullptr);

            if (pWallObj)
            {
                // horizontal collision occurred BUT its effect might be cancelled if we can step up on the object
                const float fRealBlockSizeYhalf = pWallObj->getSizeVec().getY() / 2.f;
                // TODO: I think here we shall introduce a fPlayerHalfHeight2 because if above we stood up then we need to fetch updated height!
                // But for now this is considered a corner case I'm not handling so fPlayerHalfHeight is good enough.
                const float fPlayerPos1YMinusHalf_2 = player.getPos().getNew().getY() - fPlayerHalfHeight;

                //getConsole().EOLn("lolol");

                // TODO: RFR: this part down below is again same as in serverPlayerCollisionWithWalls_legacy()

                const PureVector& vecWallObjPos = pWallObj->getPosVec();
                // maybe this is a stairstep we can step onto
                if (!player.isFalling() && !player.canFall() && ((vecWallObjPos.getY() + fRealBlockSizeYhalf) - fPlayerPos1YMinusHalf_2) <= fHeightPlayerCanStillStepUpOnto)
                {
                    // TODO: check if there is enough space to step onto the stairstep!
                    // For now I'm not doing it, it would introduce additional slowdown when stepping up on each stairstep and currently this is
                    // considered as a corner case.

                    // PPPKKKGGGGGG
                    player.getPos().set(
                        PureVector(
                            player.getPos().getNew().getX(),
                            vecWallObjPos.getY() + fRealBlockSizeYhalf + fPlayerHalfHeight + 0.01f,
                            player.getPos().getNew().getZ()
                        ));
                }
                else
                {
                    // could not step up onto the object so we set the flag here, important for proper run sound kept playing on stairs!
                    bHorizontalCollisionOccured = true;

                    if (m_bAllowStrafeMidAir || player.canFall())
                    {
                        // Horizontal collision must stop horizontal jump-induced force (unlike explosion-induced force).
                        // However, in case of m_bAllowStrafeMidAir == false it would make it impossible to jump on a box
                        // when jump is initiated from right next to the box. So as a cheat we allow keeping the horizontal
                        // jump-induced force for the period of jumping and just zero it out at the moment of starting to fall.
                        player.getJumpForce().SetX(0.f);
                    }

                    // in case of horizontal collision, we should not reposition to previous position, but align next to the wall
                    const int nAlignLeftOrRightToWall = vecWallObjPos.getX() < player.getPos().getOld().getX() ? 1 : -1;
                    const float fAlignNextToWall = nAlignLeftOrRightToWall * (pWallObj->getSizeVec().getX() / 2 + proofps_dd::Player::fObjWidth / 2.0f + 0.01f);
                    //getConsole().EOLn(
                    //    "x align to wall: old pos x: %f, new pos x: %f, fAlignNextToWall: %f",
                    //    player.getPos().getOld().getX(),
                    //    player.getPos().getNew().getX(),
                    //    fAlignNextToWall);
                    // PPPKKKGGGGGG
                    player.getPos().set(
                        PureVector(
                            vecWallObjPos.getX() + fAlignNextToWall,
                            player.getPos().getNew().getY(),
                            player.getPos().getNew().getZ()
                        ));
                } // endif can step onto it or not
            } // endif object touched horizontally
        } // endif XPos changed

        // TODO: RFR: this part is also came as in serverPlayerCollisionWithWalls_legacy()
        if (!bHorizontalCollisionOccured)
        {
            if (player.getWillSomersaultInNextTick())
            {
                // only here can we really trigger on-ground somersaulting
                player.startSomersaultServer(false);
            }

            if (!player.isInAir() && (player.getStrafeSpeed() != 0.f) && player.isRunning() && !player.getCrouchStateCurrent())
            {
                player.getActuallyRunningOnGround().set(true);
            }
            else
            {
                player.getActuallyRunningOnGround().set(false);
            }
        }
        else
        {
            player.getActuallyRunningOnGround().set(false);
        }
    } // end for player
} // serverPlayerCollisionWithWalls_bvh()
