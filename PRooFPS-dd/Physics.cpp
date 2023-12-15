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
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    proofps_dd::Maps& maps,
    proofps_dd::Sounds& sounds) :
    proofps_dd::PlayerHandling(pge, durations, mapPlayers, maps, sounds),
    proofps_dd::UserInterface(pge),
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
    // But they can used in other functions.

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

void proofps_dd::Physics::serverGravity(PureObject3D& objXHair, const unsigned int& nPhysicsRate)
{   
    /* Although I tried to make calculations to have same result with different tickrate, the
       results are not the same, just SIMILAR when comparing 60 vs 20 Hz results.
       The real difference is during jumping:
        - for 60 Hz, GAME_GRAVITY_CONST should be 90.f,
        - for 20 Hz, GAME_GRAVITY_CONST should be 80.f to have same jumping.
       So I decided to define GAME_GRAVITY_CONST at runtime based on tickrate.
       Note that originally I wanted to lerp GAME_JUMP_GRAVITY_START as commented at its definition. */

    static const float GAME_GRAVITY_LERP_FACTOR = (nPhysicsRate - GAME_TICKRATE_MIN) / static_cast<float>(GAME_TICKRATE_MAX - GAME_TICKRATE_MIN);
    static const float GAME_GRAVITY_CONST = PFL::lerp(80.f, 90.f, GAME_GRAVITY_LERP_FACTOR);
    static constexpr float GAME_FALL_GRAVITY_MIN = -15.f;

    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        player.getHasJustStartedFallingNaturallyInThisTick() = false;
        player.getHasJustStartedFallingAfterJumpingStoppedInThisTick() = false;
        const float fPlayerGravityChangePerTick = -GAME_GRAVITY_CONST / nPhysicsRate;

        if (!player.getCrouchInput().getOld() && player.getCrouchInput().getNew())
        {
            // TODO: move this into Player function with a flag bPullUpLegs;
            // player just initiated crouching;
            // scaling change is legal since player object will be smaller thus no unexpected collision can happen;
            // position change is also legal since player area stays within previous standing area;
            // We need to set Object3D scaling since that is used in physics calculations also in serverPlayerCollisionWithWalls(),
            // but we dont need to set Object3D position because Player object has its own position vector that is used in physics.
            // On the long run we should use colliders so physics does not depend on graphics.
            player.getObject3D()->SetScaling(PureVector(1.f, GAME_PLAYER_H_CROUCH_SCALING_Y, 1.f));
            if (player.isJumping() || (player.getGravity() < fPlayerGravityChangePerTick))
            {
                // reposition is allowed only if being in the air: pulling up the legs, so the head supposed to stay in same position as before,
                // however we don't reposition if being on ground because it looks bad
                PureVector playerPos = player.getPos().getNew();
                playerPos.SetY(playerPos.getY() + GAME_PLAYER_H_STAND / 2.f - (GAME_PLAYER_H_STAND * GAME_PLAYER_H_CROUCH_SCALING_Y) / 2.f);
                player.getPos().set(playerPos);
                // since we are at the beginning of a tick, it is legal to commit the position now, as old and new positions supposed to be the same at this point
                player.getPos().commit();
            }
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
                player.getPos().getNew().getY() + player.getGravity() / nPhysicsRate,
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
    static const float GAME_PLAYER_SPEED_WALK = 2.0f / nPhysicsRate;
    static const float GAME_PLAYER_SPEED_RUN = 4.0f / nPhysicsRate;
    static const float GAME_PLAYER_SPEED_CROUCH = 1.5f / nPhysicsRate;

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
                    player.getCrouchStateCurrent() = false;
                    player.getObject3D()->SetScaling(PureVector(1.f, 1.f, 1.f));
                    // reposition so the legs will stay at the same position as we stand up, so we are essentially growing up from the ground
                    player.getPos().set(
                        PureVector(
                            player.getPos().getNew().getX(),
                            fProposedNewPlayerPosY,
                            player.getPos().getNew().getZ()
                        ));
                    
                    // WA: This line is only needed to get rid of the phenomenon on server side: for 1 frame, player object will overlap the ground object below it.
                    // This is because after physics iteration there will be 1 frame rendered, and only at the beginning of next frame the server will
                    // process the handleUserUpdateFromServer() where it repositions the player object.
                    // So this line here is just a workaround to get rid of it.
                    // Clients don't see this phenomenon since they dont run this physics, just get position updates from server.
                    // The only way to fix this is to introduce colliders so we dont mess with graphical entities in physics.
                    // A bug ticket has been opened for this: https://github.com/proof88/PRooFPS-dd/issues/265.
                    player.getObject3D()->getPosVec().SetY(player.getPos().getNew().getY());
                }
            }
        } // end if (player.getWantToStandup())

        static unsigned int nContinuousStrafeCount = 0;
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
                ++nContinuousStrafeCount;
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
            if (nContinuousStrafeCount > 0)
            {
                //getConsole().EOLn("Strafe stopped after: %u consecutive strafes", nContinuousStrafeCount);
                nContinuousStrafeCount = 0;
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

void proofps_dd::Physics::serverSetAllowStrafeMidAir(bool bAllow)
{
    m_bAllowStrafeMidAir = bAllow;
}

void proofps_dd::Physics::serverSetAllowStrafeMidAirFull(bool bAllow)
{
    m_bAllowStrafeMidAirFull = bAllow;
}


// ############################### PRIVATE ###############################

