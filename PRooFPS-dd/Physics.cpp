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

static const float GAME_FALLING_SPEED = 0.8f;
static const float GAME_JUMPING_SPEED = 2.0f;


// ############################### PUBLIC ################################


proofps_dd::Physics::Physics(
    proofps_dd::Durations& durations,
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    proofps_dd::Maps& maps,
    proofps_dd::Sounds& sounds) :
    /* due to virtual inheritance, we don't invoke ctor of PGE, PRooFPSddPGE invokes it only */
    /* due to virtual inheritance, we don't invoke ctor of UserInterface, PRooFPSddPGE invokes it only */
    Networking(durations),
    PlayerHandling(durations, maps, sounds),
    m_durations(durations),
    m_mapPlayers(mapPlayers),
    m_maps(maps),
    m_sounds(sounds)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // durations, mapPlayers, maps, sounds
    // But they can used in other functions.
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


void proofps_dd::Physics::Gravity(int /*fps*/, PureObject3D& objXHair)
{
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        if (player.isJumping())
        {
            player.SetGravity(player.getGravity() - GAME_JUMPING_SPEED / 60.f/*(float)fps*/);
            if (player.getGravity() < 0.0f)
            {
                player.StopJumping();
            }
        }
        else
        {
            if (player.getGravity() > proofps_dd::GAME_GRAVITY_MIN)
            {
                player.SetGravity(player.getGravity() - GAME_FALLING_SPEED / 60.f/*(float)fps*/);
                if (player.getGravity() < proofps_dd::GAME_GRAVITY_MIN)
                {
                    player.SetGravity(proofps_dd::GAME_GRAVITY_MIN);
                }
            }
        }
        // PPPKKKGGGGGG
        player.getPos().set(
            PureVector(
                player.getPos().getNew().getX(),
                player.getPos().getNew().getY() + player.getGravity(),
                player.getPos().getNew().getZ()
            ));

        if ((player.getHealth() > 0) && (player.getPos().getNew().getY() < m_maps.getBlockPosMin().getY() - 5.0f))
        {
            // need to die, out of map lower bound
            HandlePlayerDied(player, objXHair);
        }
    }
}

bool proofps_dd::Physics::Colliding(const PureObject3D& a, const PureObject3D& b)
{
    return Colliding2(
        a.getPosVec().getX(), a.getPosVec().getY(), a.getPosVec().getZ(),
        a.getSizeVec().getX(), a.getSizeVec().getY(), a.getSizeVec().getZ(),
        b.getPosVec().getX(), b.getPosVec().getY(), b.getPosVec().getZ(),
        b.getSizeVec().getX(), b.getSizeVec().getY(), b.getSizeVec().getZ()
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

void proofps_dd::Physics::PlayerCollisionWithWalls(bool& /*won*/)
{
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        const PureObject3D* const plobj = player.getObject3D();

        // how to make collision detection even faster:
        // if we dont want to use spatial hierarchy like BVH, just store the map elements in a matrix that we can address with i and j,
        // and based on player's position it is very easy to know which few map elements around matrix[i][j] should be checked ...
        // And I'm also thinking that not pointers but the objects themselves could be stored in matrix, that way the whole matrix
        // could be fetched into cache for even faster iteration on its elements ...

        // at this point, player.getPos().getY() is already updated by Gravity()
        const float fBlockSizeXhalf = proofps_dd::GAME_BLOCK_SIZE_X / 2.f;
        const float fBlockSizeYhalf = proofps_dd::GAME_BLOCK_SIZE_Y / 2.f;

        const float fPlayerOPos1XMinusHalf = player.getPos().getOld().getX() - plobj->getSizeVec().getX() / 2.f;
        const float fPlayerOPos1XPlusHalf = player.getPos().getOld().getX() + plobj->getSizeVec().getX() / 2.f;
        const float fPlayerPos1YMinusHalf = player.getPos().getNew().getY() - plobj->getSizeVec().getY() / 2.f;
        const float fPlayerPos1YPlusHalf = player.getPos().getNew().getY() + plobj->getSizeVec().getY() / 2.f;
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
                const float fAlignCloseToWall = nAlignUnderOrAboveWall * (fBlockSizeYhalf + proofps_dd::GAME_PLAYER_H / 2.0f + 0.01f);
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
                    player.getForce().Set(0.f, 0.f, 0.f);
                }
                else
                {
                    // we hit ceiling with our head during jumping
                    player.SetCanFall(true);
                    player.StopJumping();
                    player.SetGravity(0.f);
                }

                break;
            }
        }

        // PPPKKKGGGGGG
        player.getPos().set(
            PureVector(
                player.getPos().getNew().getX() + player.getForce().getX(),
                player.getPos().getNew().getY(),
                player.getPos().getNew().getZ()
            ));

        const float fPlayerPos1XMinusHalf = player.getPos().getNew().getX() - plobj->getSizeVec().getX() / 2.f;
        const float fPlayerPos1XPlusHalf = player.getPos().getNew().getX() + plobj->getSizeVec().getX() / 2.f;
        const float fPlayerPos1YMinusHalf_2 = player.getPos().getNew().getY() - plobj->getSizeVec().getY() / 2.f;
        const float fPlayerPos1YPlusHalf_2 = player.getPos().getNew().getY() + plobj->getSizeVec().getY() / 2.f;

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

                // in case of horizontal collision, we should not reposition to previous position, but align next to the wall
                const int nAlignLeftOrRightToWall = obj->getPosVec().getX() < player.getPos().getOld().getX() ? 1 : -1;
                const float fAlignNextToWall = nAlignLeftOrRightToWall * (obj->getSizeVec().getX() / 2 + proofps_dd::GAME_PLAYER_W / 2.0f + 0.01f);
                // PPPKKKGGGGGG
                player.getPos().set(
                    PureVector(
                        obj->getPosVec().getX() + fAlignNextToWall,
                        player.getPos().getNew().getY(),
                        player.getPos().getNew().getZ()
                    ));

                break;
            }
        }
    }
}


// ############################### PRIVATE ###############################

