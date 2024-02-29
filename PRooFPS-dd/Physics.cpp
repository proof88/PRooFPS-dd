/*
    ###################################################################################
    Physics.cpp
    Physics handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>
#include <chrono>

#include "Physics.h"


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
    m_bAllowStrafeMidAirFull(false)
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


// ############################## PROTECTED ##############################


bool proofps_dd::Physics::Colliding(const PureObject3D& a, const PureObject3D& b)
{
    return Colliding2(
        a.getPosVec().getX(), a.getPosVec().getY(), a.getPosVec().getZ(),
        a.getScaledSizeVec().getX(), a.getScaledSizeVec().getY(), a.getScaledSizeVec().getZ(),
        b.getPosVec().getX(), b.getPosVec().getY(), b.getPosVec().getZ(),
        b.getScaledSizeVec().getX(), b.getScaledSizeVec().getY(), b.getScaledSizeVec().getZ()
    );
}

bool proofps_dd::Physics::Colliding_NoZ(const PureObject3D& a, const PureObject3D& b)
{
    return Colliding2_NoZ(
        a.getPosVec().getX(), a.getPosVec().getY(),
        a.getScaledSizeVec().getX(), a.getScaledSizeVec().getY(),
        b.getPosVec().getX(), b.getPosVec().getY(),
        b.getScaledSizeVec().getX(), b.getScaledSizeVec().getY()
    );
}

bool proofps_dd::Physics::Colliding2(
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

bool proofps_dd::Physics::Colliding2_NoZ(
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
    if (Colliding2_NoZ(o1px, o1py, o1sx, o1sy, o2px, o2py, 0.1f, 0.1f))
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
    vDirPerAxis.SetX( o1px - o2px );
    vDirPerAxis.SetY( o1py - o2py );
    vDirPerAxis.Normalize();

    // first we check if O1 is colliding with O2, if yes then their distance is 0
    if (Colliding2_NoZ(o1px, o1py, o1sx, o1sy, o2px, o2py, 0.1f, 0.1f))
    {
        vDistancePerAxis.SetZero();
        return 0.f;
    }

    // if they are not colliding, we check the 4 points of O1 and return the distance to position of O2 center.
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

bool proofps_dd::Physics::Colliding3(
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

    return Colliding2(
        vecPos.getX(), vecPos.getY(), vecPos.getZ(),
        vecSize.getX(), vecSize.getY(), vecSize.getZ(),
        vecObjPos.getX(), vecObjPos.getY(), vecObjPos.getZ(),
        vecObjSize.getX(), vecObjSize.getY(), vecObjSize.getZ()
    );
}

void proofps_dd::Physics::serverSetAllowStrafeMidAir(bool bAllow)
{
    m_bAllowStrafeMidAir = bAllow;
}

void proofps_dd::Physics::serverSetAllowStrafeMidAirFull(bool bAllow)
{
    m_bAllowStrafeMidAirFull = bAllow;
}

void proofps_dd::Physics::serverGravity(PureObject3D& objXHair, const unsigned int& nPhysicsRate)
{   
    /* Although I tried to make calculations to have same result with different tickrate, the
       results are not the same, just SIMILAR when comparing 60 vs 20 Hz results.
       The real difference is during jumping:
        - for 60 Hz, GAME_GRAVITY_CONST should be 90.f,
        - for 20 Hz, GAME_GRAVITY_CONST should be 80.f to have same jumping.
       So I decided to define GAME_GRAVITY_CONST at runtime based on tickrate.
       Note that originally I wanted to lerp GAME_JUMP_GRAVITY_START as commented at its definition. */

    const float GAME_GRAVITY_LERP_FACTOR = (nPhysicsRate - GAME_TICKRATE_MIN) / static_cast<float>(GAME_TICKRATE_MAX - GAME_TICKRATE_MIN);
    const float GAME_GRAVITY_CONST = PFL::lerp(80.f, 90.f, GAME_GRAVITY_LERP_FACTOR);
    static constexpr float GAME_FALL_GRAVITY_MIN = -15.f;

    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        const float fPlayerImpactForceYChangePerTick = 36.f / nPhysicsRate;
        if (player.getImpactForce().getY() > 0.f)
        {
            /* TODO: amount of decrease/increase of impact force here might be modified later with fForceMultiplier tweaking in createExplosionServer */
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

        player.getHasJustStartedFallingNaturallyInThisTick() = false;
        player.getHasJustStartedFallingAfterJumpingStoppedInThisTick() = false;
        const float fPlayerGravityChangePerTick = -GAME_GRAVITY_CONST / nPhysicsRate;

        if (!player.getCrouchInput().getOld() && player.getCrouchInput().getNew())
        {
            // player just initiated crouching by input
            player.DoCrouchServer(player.isJumping() || (player.getGravity() < fPlayerGravityChangePerTick));
        } // end handle crouch

        player.SetGravity(player.getGravity() + fPlayerGravityChangePerTick);
        if (player.isJumping())
        {
            if (player.getGravity() < 0.0f)
            {
                //getConsole().EOLn("stopped jumping up (natural): %f", player.getGravity());
                player.SetGravity(0.f);
                player.StopJumping();
                player.getHasJustStoppedJumpingInThisTick() = true;
                player.SetCanFall(true);
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
                    player.getHasJustStartedFallingAfterJumpingStoppedInThisTick() = true;
                }
                else
                {
                    player.getHasJustStartedFallingNaturallyInThisTick() = true;
                }
                player.getHasJustStoppedJumpingInThisTick() = false;
                //getConsole().EOLn("asd: %f", player.getGravity());
            }
            player.SetCanFall(true);
            // player gravity cannot go below GAME_FALL_GRAVITY_MIN
            player.SetGravity(std::max(player.getGravity(), GAME_FALL_GRAVITY_MIN));
        }
        // PPPKKKGGGGGG
        player.getPos().set(
            PureVector(
                player.getPos().getNew().getX(),
                player.getPos().getNew().getY() + player.getGravity() / nPhysicsRate + player.getImpactForce().getY() / nPhysicsRate,
                player.getPos().getNew().getZ()
            ));

        if ((player.getHealth() > 0) && (player.getPos().getNew().getY() < m_maps.getBlockPosMin().getY() - 5.0f))
        {
            // need to die, out of map lower bound
            HandlePlayerDied(player, objXHair);
        }
    }
}

void proofps_dd::Physics::serverPlayerCollisionWithWalls(bool& /*won*/, const unsigned int& nPhysicsRate)
{
    const float GAME_PLAYER_SPEED_WALK = 2.0f / nPhysicsRate;
    const float GAME_PLAYER_SPEED_RUN = 4.0f / nPhysicsRate;
    const float GAME_PLAYER_SPEED_CROUCH = 1.5f / nPhysicsRate;

    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        const PureObject3D* const plobj = player.getObject3D();
        PureVector vecOriginalJumpForce = player.getJumpForce();

        // how to make collision detection even faster:
        // if we dont want to use spatial hierarchy like BVH, just store the map elements in a matrix that we can address with i and j,
        // and based on player's position it is very easy to know which few map elements around matrix[i][j] should be checked ...
        // And I'm also thinking that not pointers but the objects themselves could be stored in matrix, that way the whole matrix
        // could be fetched into cache for even faster iteration on its elements ...

        // at this point, player.getPos().getY() is already updated by Gravity()
        const float fBlockSizeXhalf = proofps_dd::GAME_BLOCK_SIZE_X / 2.f;
        const float fBlockSizeYhalf = proofps_dd::GAME_BLOCK_SIZE_Y / 2.f;

        const float fPlayerHalfHeight = plobj->getScaledSizeVec().getY() / 2.f;

        // We use Player's Object3D scaling since that is used in physics calculations also in serverGravity(),
        // but we dont need to set Object3D position because Player object has its own position vector that is used in physics.
        // On the long run we should use colliders so physics does not depend on graphics.
        const float fPlayerOPos1XMinusHalf = player.getPos().getOld().getX() - plobj->getScaledSizeVec().getX() / 2.f;
        const float fPlayerOPos1XPlusHalf = player.getPos().getOld().getX() + plobj->getScaledSizeVec().getX() / 2.f;
        const float fPlayerPos1YMinusHalf = player.getPos().getNew().getY() - fPlayerHalfHeight;
        const float fPlayerPos1YPlusHalf = player.getPos().getNew().getY() + fPlayerHalfHeight;
        if (player.getPos().getOld().getY() != player.getPos().getNew().getY())
        {
            for (int i = 0; i < m_maps.getForegroundBlockCount(); i++)
            {
                const PureObject3D* const obj = m_maps.getForegroundBlocks()[i];
                assert(obj);  // we dont store nulls there

                if ((obj->getPosVec().getX() + fBlockSizeXhalf < fPlayerOPos1XMinusHalf) || (obj->getPosVec().getX() - fBlockSizeXhalf > fPlayerOPos1XPlusHalf))
                {
                    continue;
                }

                if ((obj->getPosVec().getY() + fBlockSizeYhalf < fPlayerPos1YMinusHalf) || (obj->getPosVec().getY() - fBlockSizeYhalf > fPlayerPos1YPlusHalf))
                {
                    continue;
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
                    player.SetCanFall(false);
                    
                    // maybe not null everything out in the future but only decrement the components by
                    // some value, since if there is an explosion-induced force, it shouldnt be nulled out
                    // at this moment. Currently we want to null out the strafe-jump-induced force.
                    player.getJumpForce().Set(0.f, 0.f, 0.f);
                    player.SetGravity(0.f);
                }
                else
                {
                    // we hit ceiling with our head during jumping
                    //getConsole().EOLn("start falling (hit ceiling)");
                    player.SetCanFall(true);
                    player.StopJumping();
                    player.getHasJustStoppedJumpingInThisTick() = true;
                    player.SetGravity(0.f);
                }

                break;
            } // end for i
        } // end if YPPos changed

        if (player.getWantToStandup())
        {
            if (player.getCrouchStateCurrent())
            {
                // we need to check if there is enough space to stand up
                const float fProposedNewPlayerHalfHeight = GAME_PLAYER_H_STAND / 2.f;
                const float fProposedNewPlayerPosY = player.getPos().getNew().getY() - (GAME_PLAYER_H_STAND * GAME_PLAYER_H_CROUCH_SCALING_Y) / 2.f + fProposedNewPlayerHalfHeight + 0.01f;
                const float fProposedNewPlayerPos1YMinusHalf = fProposedNewPlayerPosY - fProposedNewPlayerHalfHeight;
                const float fProposedNewPlayerPos1YPlusHalf = fProposedNewPlayerPosY + fProposedNewPlayerHalfHeight;
                bool bCanStandUp = true;
                for (int i = 0; i < m_maps.getForegroundBlockCount(); i++)
                {
                    const PureObject3D* const obj = m_maps.getForegroundBlocks()[i];
                    assert(obj);  // we dont store nulls there

                    if ((obj->getPosVec().getX() + fBlockSizeXhalf < fPlayerOPos1XMinusHalf) || (obj->getPosVec().getX() - fBlockSizeXhalf > fPlayerOPos1XPlusHalf))
                    {
                        continue;
                    }

                    if ((obj->getPosVec().getY() + fBlockSizeYhalf < fProposedNewPlayerPos1YMinusHalf) || (obj->getPosVec().getY() - fBlockSizeYhalf > fProposedNewPlayerPos1YPlusHalf))
                    {
                        continue;
                    }

                    // found a blocking object, cannot stand up
                    bCanStandUp = false;
                    break;
                } // end for i
                if (bCanStandUp)
                {
                    player.DoStandupServer(fProposedNewPlayerPosY);
                }
            }
        } // end if (player.getWantToStandup())

        static unsigned int nContinuousStrafeCountForDebugServerPlayerMovement = 0;
        if ((player.getHealth() > 0) && (player.getStrafe() != proofps_dd::Strafe::NONE))
        {
            float fStrafeSpeed = player.getCrouchStateCurrent() ? GAME_PLAYER_SPEED_CROUCH : (player.isRunning() ? GAME_PLAYER_SPEED_RUN : GAME_PLAYER_SPEED_WALK);
            if (player.getStrafe() == proofps_dd::Strafe::LEFT)
            {
                fStrafeSpeed = -fStrafeSpeed;
            }

            if (player.getHasJustStartedFallingNaturallyInThisTick())
            {
                player.getJumpForce().SetX(fStrafeSpeed);
                vecOriginalJumpForce = player.getJumpForce();
            }

            const bool bPlayerInAir = player.isJumping() || player.canFall();
            if ( !bPlayerInAir ||
                 (m_bAllowStrafeMidAir &&
                 (
                    /* if jump was initiated without horizontal force or we nulled it out due to hitting a wall */
                    (vecOriginalJumpForce.getX() == 0.f) ||
                    /* if we have horizontal jump force, we cannot add more to it in the same direction */
                    ((vecOriginalJumpForce.getX() > 0.f) && (fStrafeSpeed < 0.f)) || ((vecOriginalJumpForce.getX() < 0.f) && (fStrafeSpeed > 0.f))
                 ))
               )
            {
                ++nContinuousStrafeCountForDebugServerPlayerMovement;
                //getConsole().EOLn("Tick Strafe");

                if (m_bAllowStrafeMidAirFull)
                {
                    // cancel the saved horizontal force, let the player have full control over mid-air strafe
                    player.getJumpForce().SetX(0.f);
                    vecOriginalJumpForce = player.getJumpForce();
                }

                // PPPKKKGGGGGG
                player.getPos().set(
                    PureVector(
                        player.getPos().getNew().getX() + fStrafeSpeed,
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

        // Note that because we are handling jumping here, we are 1 frame late. We should handle it at the beginning of the Gravity() function,
        // to actually start jumping in the same frame as when we detected the player initiated the jumping.
        // However, we are handling it here because X-pos is updated by strafe here so here we will have actually different new and old X-pos,
        // that is essential for the Jump() function below to record the X-forces for the player.
        // For now this 1 frame latency is not critical so I'm not planning to change that. Might be addressed in the future though.
        if (player.getWillJumpInNextTick())
        {
            //getConsole().EOLn("start jumping");
            // now we can actually jump and have the correct forces be saved for the jump
            player.Jump(); // resets setWillJumpInNextTick()
        }

        // PPPKKKGGGGGG
        player.getPos().set(
            /* we use the vecOriginalJumpForce instead of current force because current force might had been
               zeroed out above in the moment of falling on the ground, however here we still have to add
               the original force to be perfect.
               If we dont do this, there will be a 1 frame outage in the continuity of strafe movement at
               the moment of finishing the jump because handleUserCmdMoveFromClient() in this frame did not allow strafe
               due to ongoing jumping, while this current function terminates jumping above before we could
               apply the force here for last time - hence we use the original force at the beginning of function. */
            PureVector(
                player.getPos().getNew().getX() + vecOriginalJumpForce.getX(),
                player.getPos().getNew().getY(),
                player.getPos().getNew().getZ()
            ));

        const float fPlayerImpactForceXChangePerTick = 25.f / nPhysicsRate;
        if (player.getImpactForce().getX() > 0.f)
        {
            /* TODO: amount of decrease/increase of impact force here might be modified later with fForceMultiplier tweaking in createExplosionServer */
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

        const float fPlayerPos1XMinusHalf = player.getPos().getNew().getX() - plobj->getSizeVec().getX() / 2.f;
        const float fPlayerPos1XPlusHalf = player.getPos().getNew().getX() + plobj->getSizeVec().getX() / 2.f;
        const float fPlayerPos1YMinusHalf_2 = player.getPos().getNew().getY() - fPlayerHalfHeight;
        const float fPlayerPos1YPlusHalf_2 = player.getPos().getNew().getY() + fPlayerHalfHeight;

        if (player.getPos().getOld().getX() != player.getPos().getNew().getX())
        {
            for (int i = 0; i < m_maps.getForegroundBlockCount(); i++)
            {
                const PureObject3D* const obj = m_maps.getForegroundBlocks()[i];
                assert(obj);  // we dont store nulls there

                if ((obj->getPosVec().getX() + fBlockSizeXhalf < fPlayerPos1XMinusHalf) || (obj->getPosVec().getX() - fBlockSizeXhalf > fPlayerPos1XPlusHalf))
                {
                    continue;
                }

                if ((obj->getPosVec().getY() + fBlockSizeYhalf < fPlayerPos1YMinusHalf_2) || (obj->getPosVec().getY() - fBlockSizeYhalf > fPlayerPos1YPlusHalf_2))
                {
                    continue;
                }

                if (m_bAllowStrafeMidAir || player.canFall())
                {
                    // Horizontal collision must stop horizontal jump-induced force (unlike explosion-induced force).
                    // However, in case of m_bAllowStrafeMidAir == false it would make it impossible to jump on a box
                    // when jump is initiated from right next to the box. So as a cheat we allow keeping the horizontal
                    // jump-induced force for the period of jumping and just zero it out at the moment of starting to fall.
                    player.getJumpForce().SetX(0.f);
                }

                // in case of horizontal collision, we should not reposition to previous position, but align next to the wall
                const int nAlignLeftOrRightToWall = obj->getPosVec().getX() < player.getPos().getOld().getX() ? 1 : -1;
                const float fAlignNextToWall = nAlignLeftOrRightToWall * (obj->getSizeVec().getX() / 2 + proofps_dd::GAME_PLAYER_W / 2.0f + 0.01f);
                //getConsole().EOLn("x align to wall");
                // PPPKKKGGGGGG
                player.getPos().set(
                    PureVector(
                        obj->getPosVec().getX() + fAlignNextToWall,
                        player.getPos().getNew().getY(),
                        player.getPos().getNew().getZ()
                    ));

                break;
            } // end for i
        } // end XPos changed
    } // end for player
}


// ############################### PRIVATE ###############################

