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



/*
   Explosion
   ###########################################################################
*/


// ############################### PUBLIC ################################


const char* proofps_dd::Explosion::getLoggerModuleName()
{
    return "Explosion";
}

proofps_dd::Explosion::ExplosionId proofps_dd::Explosion::getGlobalExplosionId()
{
    return m_globalExplosionId;
}

void proofps_dd::Explosion::ResetGlobalExplosionId()
{
    m_globalExplosionId = 0;
}

bool proofps_dd::Explosion::initExplosionsReference(PGE& pge)
{
    if (m_pReferenceObjExplosion)
    {
        return true;
    }

    m_pReferenceObjExplosion = pge.getPure().getObject3DManager().createFromFile((std::string(GAME_MODELS_DIR) + "rocketl_xpl.obj").c_str());
    if (!m_pReferenceObjExplosion)
    {
        return false;
    }

    m_pReferenceObjExplosion->SetDoubleSided(true);
    m_pReferenceObjExplosion->SetLit(false);
    // We want the animation to start with 0.1 unit size so we set scaling with this formula.
    // For example, diameter of rocketl_xpl.obj is 16 units, so to have a diameter of 1 unit, its scaling should be 1/16,
    // we use getSizeVec().getX() as diameter, considering the model object is always a sphere.
    m_pReferenceObjExplosion->SetScaling(1 / m_pReferenceObjExplosion->getSizeVec().getX() / 10.f);
    m_pReferenceObjExplosion->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE);
    m_pReferenceObjExplosion->Hide();

    return true;
}

void proofps_dd::Explosion::destroyExplosionsReference()
{
    if (m_pReferenceObjExplosion)
    {
        delete m_pReferenceObjExplosion; // will detach from manager
        m_pReferenceObjExplosion = nullptr;
    }
}

/**
    Ctor to be used by PGE server instance: bullet id will be assigned within the ctor.
*/
proofps_dd::Explosion::Explosion(
    PR00FsUltimateRenderingEngine& gfx,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const PureVector& pos,
    const TPureFloat& fDamageAreaSize) :
    m_id(m_globalExplosionId++),
    m_gfx(gfx),
    m_connHandle(connHandle),
    m_fDamageAreaSize(fDamageAreaSize),
    m_objPrimary(nullptr),
    m_objSecondary(nullptr),
    m_fScalingPrimary(m_pReferenceObjExplosion->getScaling().getX()),
    m_fScalingSecondary(m_pReferenceObjExplosion->getScaling().getX()),
    m_bCreateSentToClients(false)
{
    // TODO throw exception if cant create!
    m_objPrimary = gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
    m_objPrimary->Show();
    m_objPrimary->getPosVec() = pos;

    m_objSecondary = gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
    m_objSecondary->Hide();
    m_objSecondary->getPosVec() = pos;
}

/**
    Ctor to be used by PGE client instance: bullet id as received from server.
*/
proofps_dd::Explosion::Explosion(
    PR00FsUltimateRenderingEngine& gfx,
    const Explosion::ExplosionId& id,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const PureVector& pos,
    const TPureFloat& fDamageAreaSize) :
    m_id(id),
    m_gfx(gfx),
    m_connHandle(connHandle),
    m_fDamageAreaSize(fDamageAreaSize),
    m_objPrimary(nullptr),
    m_objSecondary(nullptr),
    m_fScalingPrimary(m_pReferenceObjExplosion->getScaling().getX()),
    m_fScalingSecondary(m_pReferenceObjExplosion->getScaling().getX()),
    m_bCreateSentToClients(true) /* irrelevant for this client-side ctor but we are client so yes it is sent :) */
{
    // TODO throw exception if cant create!
    m_objPrimary = gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
    m_objPrimary->Show();
    m_objPrimary->getPosVec() = pos;

    m_objSecondary = gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
    m_objSecondary->Hide();
    m_objSecondary->getPosVec() = pos;
}

proofps_dd::Explosion::Explosion(const proofps_dd::Explosion& other) :
    m_id(other.m_id),
    m_gfx(other.m_gfx),
    m_connHandle(other.m_connHandle),
    m_fDamageAreaSize(other.m_fDamageAreaSize),
    m_fScalingPrimary(other.m_fScalingPrimary),
    m_fScalingSecondary(other.m_fScalingSecondary),
    m_bCreateSentToClients(other.m_bCreateSentToClients)
{
    // TODO throw exception if cant create!
    if (other.m_objPrimary)
    {
        m_objPrimary = m_gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
        m_objPrimary->SetRenderingAllowed(other.m_objPrimary->isRenderingAllowed());
        m_objPrimary->getPosVec() = other.m_objPrimary->getPosVec();
        m_objPrimary->SetScaling(other.m_objPrimary->getScaling());
    }
    else
    {
        m_objPrimary = nullptr;
    }

    if (other.m_objSecondary)
    {
        m_objSecondary = m_gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
        m_objSecondary->SetRenderingAllowed(other.m_objSecondary->isRenderingAllowed());
        m_objSecondary->getPosVec() = other.m_objSecondary->getPosVec();
        m_objSecondary->SetScaling(other.m_objSecondary->getScaling());
    }
    else
    {
        m_objSecondary = nullptr;
    }
}

proofps_dd::Explosion& proofps_dd::Explosion::operator=(const proofps_dd::Explosion& other)
{
    m_id = other.m_id;
    m_gfx = other.m_gfx;
    m_connHandle = other.m_connHandle;
    m_fDamageAreaSize = other.m_fDamageAreaSize;
    m_fScalingPrimary = other.m_fScalingPrimary;
    m_fScalingSecondary = other.m_fScalingSecondary;
    m_bCreateSentToClients = other.m_bCreateSentToClients;

    // TODO throw exception if cant create!
    if (other.m_objPrimary)
    {
        m_objPrimary = m_gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
        m_objPrimary->SetRenderingAllowed(other.m_objPrimary->isRenderingAllowed());
        m_objPrimary->getPosVec() = other.m_objPrimary->getPosVec();
        m_objPrimary->SetScaling(other.m_objPrimary->getScaling());
    }
    else
    {
        m_objPrimary = nullptr;
    }

    if (other.m_objSecondary)
    {
        m_objSecondary = m_gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
        m_objSecondary->SetRenderingAllowed(other.m_objSecondary->isRenderingAllowed());
        m_objSecondary->getPosVec() = other.m_objSecondary->getPosVec();
        m_objSecondary->SetScaling(other.m_objSecondary->getScaling());
    }
    else
    {
        m_objSecondary = nullptr;
    }

    return *this;
}

proofps_dd::Explosion::~Explosion()
{
    if (m_objPrimary)
    {
        m_gfx.getObject3DManager().DeleteAttachedInstance(*m_objPrimary);
    }
    if (m_objSecondary)
    {
        m_gfx.getObject3DManager().DeleteAttachedInstance(*m_objSecondary);
    }
}

CConsole& proofps_dd::Explosion::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::Explosion::ExplosionId proofps_dd::Explosion::getId() const
{
    return m_id;
}

pge_network::PgeNetworkConnectionHandle proofps_dd::Explosion::getOwner() const
{
    return m_connHandle;
}

bool& proofps_dd::Explosion::isCreateSentToClients()
{
    return m_bCreateSentToClients;
}

void proofps_dd::Explosion::update(const unsigned int& nFactor)
{
    if (m_objPrimary)
    {
        m_fScalingPrimary += 1.5f / static_cast<TPureFloat>(nFactor);
        // either we are scaling in all dimensions or just in XY directions, not sure which looks better!
        //m_objPrimary->SetScaling(m_fScalingPrimary);
        m_objPrimary->SetScaling(PureVector(m_fScalingPrimary, m_fScalingPrimary, m_objPrimary->getScaling().getZ()));

        // the scaling animation should end when we reached the damage area size, which is the radius of damage,
        // thus we should stop at m_fDamageAreaSize * 2, however I use *3 because the transparency is also increasing:
        const float fTargetDiameter = m_fDamageAreaSize * 3;
        const float fCurrentDiameter = m_objPrimary->getSizeVec().getX() * m_fScalingPrimary;

        if (fCurrentDiameter > fTargetDiameter)
        {
            delete m_objPrimary;
            m_objPrimary = nullptr;
        }
        else
        {
            const float fAnimationProgress = fCurrentDiameter / fTargetDiameter;
            if (m_objSecondary && !m_objSecondary->isRenderingAllowed())
            {
                if (fAnimationProgress >= 0.4f)
                {
                    m_objSecondary->Show();
                }
            }
            const float fTargetTransparency = 1 - fAnimationProgress;
            m_objPrimary->getMaterial(false).getTextureEnvColor().SetAsFloats(fTargetTransparency, fTargetTransparency, fTargetTransparency, 1.f);
        }
    }

    // copy-paste code, not nice ...
    if (m_objSecondary && m_objSecondary->isRenderingAllowed())
    {
        m_fScalingSecondary += 1.5f / static_cast<TPureFloat>(nFactor);
        // either we are scaling in all dimensions or just in XY directions, not sure which looks better!
        //m_objSecondary->SetScaling(m_fScalingSecondary);
        m_objSecondary->SetScaling(PureVector(m_fScalingSecondary, m_fScalingSecondary, m_objSecondary->getScaling().getZ()));

        // the scaling animation should end when we reached the damage area size, which is the radius of damage,
        // thus we should stop at m_fDamageAreaSize * 2, however I use *3 because the transparency is also increasing:
        const float fTargetDiameter = m_fDamageAreaSize * 3;
        const float fCurrentDiameter = m_objSecondary->getSizeVec().getX() * m_fScalingSecondary;

        if (fCurrentDiameter > fTargetDiameter)
        {
            delete m_objSecondary;
            m_objSecondary = nullptr;
        }
        else
        {
            const float fTargetTransparency = 1 - (fCurrentDiameter / fTargetDiameter);
            m_objSecondary->getMaterial(false).getTextureEnvColor().SetAsFloats(fTargetTransparency, fTargetTransparency, fTargetTransparency, 1.f);
        }
    }
}

PureObject3D& proofps_dd::Explosion::getPrimaryObject3D()
{
    return *m_objPrimary;
}

const PureObject3D& proofps_dd::Explosion::getPrimaryObject3D() const
{
    return *m_objPrimary;
}

PureObject3D& proofps_dd::Explosion::getSecondaryObject3D()
{
    return *m_objSecondary;
}

const PureObject3D& proofps_dd::Explosion::getSecondaryObject3D() const
{
    return *m_objSecondary;
}

const float& proofps_dd::Explosion::getDamageAreaSize() const
{
    return m_fDamageAreaSize;
}

float proofps_dd::Explosion::getDamageAtDistance(
    const float& fDistance,
    const int& nDamageHp) const
{
    // fDistance is distance between explosion center and other entity's center,
    // m_fDamageAreaSize is radius
    assert(fDistance >= 0.f);
    assert(m_fDamageAreaSize > 0.f);

    return nDamageHp * std::max(0.f, (1 - (fDistance / m_fDamageAreaSize)));
}

bool proofps_dd::Explosion::shouldBeDeleted() const
{
    return !m_objPrimary && !m_objSecondary;
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


proofps_dd::Explosion::ExplosionId proofps_dd::Explosion::m_globalExplosionId = 0;
PureObject3D* proofps_dd::Explosion::m_pReferenceObjExplosion = nullptr;


/*
   WeaponHandling
   ###########################################################################
*/


// ############################### PUBLIC ################################


proofps_dd::WeaponHandling::WeaponHandling(
    PGE& pge,
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
    m_durations(durations),
    m_mapPlayers(mapPlayers),
    m_maps(maps),
    m_sounds(sounds)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations, mapPlayers, sounds
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


// ############################## PROTECTED ##############################


bool proofps_dd::WeaponHandling::isBulletOutOfMapBounds(const Bullet& bullet) const
{
    // we relax map bounds a bit to let the bullets leave map area a bit more before destroying them ...
    const PureVector vRelaxedMapMinBounds(
        m_maps.getBlocksVertexPosMin().getX() - proofps_dd::Maps::fMapBlockSizeWidth * 4,
        m_maps.getBlocksVertexPosMin().getY() - proofps_dd::Maps::fMapBlockSizeHeight,
        m_maps.getBlocksVertexPosMin().getZ() - proofps_dd::Maps::fMapBlockSizeDepth); // ah why dont we have vector-scalar subtract operator defined ...
    const PureVector vRelaxedMapMaxBounds(
        m_maps.getBlocksVertexPosMax().getX() + proofps_dd::Maps::fMapBlockSizeWidth * 4,
        m_maps.getBlocksVertexPosMax().getY() + proofps_dd::Maps::fMapBlockSizeHeight,
        m_maps.getBlocksVertexPosMax().getZ() + proofps_dd::Maps::fMapBlockSizeDepth);
    
    return !colliding3(vRelaxedMapMinBounds, vRelaxedMapMaxBounds, bullet.getObject3D().getPosVec(), bullet.getObject3D().getSizeVec());
}

void proofps_dd::WeaponHandling::serverUpdateBullets(proofps_dd::GameMode& gameMode, PureObject3D& objXHair, const unsigned int& nPhysicsRate, PureVector& vecCamShakeForce)
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    // on the long run this function needs to be part of the game engine itself, however currently game engine doesn't handle collisions,
    // so once we introduce the collisions to the game engine, it will be an easy move of this function as well there
    pge_network::PgePacket newPktBulletUpdate;
    const float fBlockSizeXhalf = proofps_dd::Maps::fMapBlockSizeWidth / 2.f;
    const float fBlockSizeYhalf = proofps_dd::Maps::fMapBlockSizeHeight / 2.f;
    bool bEndGame = gameMode.checkWinningConditions();
    std::list<Bullet>& bullets = m_pge.getBullets();
    auto it = bullets.begin();
    while (it != bullets.end())
    {
        auto& bullet = *it;

        bool bDeleteBullet = false;
        if (bEndGame)
        {
            bDeleteBullet = true;
        }
        else
        {
            bullet.Update(nPhysicsRate);
        }

        const float fBulletPosX = bullet.getObject3D().getPosVec().getX();
        const float fBulletPosY = bullet.getObject3D().getPosVec().getY();

        if (!bDeleteBullet)
        {
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

                const auto& playerConst = player;
                if ((playerConst.getHealth() > 0) &&
                    colliding2_NoZ(
                        player.getPos().getNew().getX(), player.getPos().getNew().getY(),
                        player.getObject3D()->getScaledSizeVec().getX(), player.getObject3D()->getScaledSizeVec().getY(),
                        fBulletPosX, fBulletPosY,
                        bullet.getObject3D().getSizeVec().getX(), bullet.getObject3D().getSizeVec().getY()))
                {
                    bDeleteBullet = true;
                    if (bullet.getAreaDamageSize() == 0.f)
                    {
                        // non-explosive bullets do damage here, explosive bullets make explosions so then the explosion does damage in createExplosionServer()
                        player.doDamage(bullet.getDamageHp());
                        if (playerConst.getHealth() == 0)
                        {
                            const auto itKiller = m_mapPlayers.find(bullet.getOwner());
                            pge_network::PgeNetworkConnectionHandle nKillerConnHandleServerSide;
                            if (itKiller == m_mapPlayers.end())
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
                                nKillerConnHandleServerSide = itKiller->first;
                                itKiller->second.getFrags()++;
                                bEndGame = gameMode.checkWinningConditions();
                                //getConsole().OLn("WeaponHandling::%s(): Player %s has been killed by %s, who now has %d frags!",
                                //    __func__, playerPair.first.c_str(), itKiller->first.c_str(), itKiller->second.getFrags());
                            }
                            // server handles death here, clients will handle it when they receive MsgUserUpdateFromServer
                            handlePlayerDied(player, objXHair, nKillerConnHandleServerSide);
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
                // 
                // TODO: this part slows the game down in debug mode with a few bullets.
                // Originally with only 6 bullets, with VSync FPS went down from 60 to 45 on main dev machine, so
                // I recommend using spatial hierarchy to effectively reduce the number of collision checks ...
                // For now with the small bullet direction optimization in the loop I managed to keep FPS around 45-50
                // with 10-15 bullets.
                for (int i = 0; i < m_maps.getForegroundBlockCount(); i++)
                {
                    const PureObject3D* const obj = m_maps.getForegroundBlocks()[i];
                    const bool bGoingLeft = bullet.getObject3D().getAngleVec().getY() == 0.f; // otherwise it would be 180.f
                    const float fMapObjPosX = obj->getPosVec().getX();
                    const float fMapObjPosY = obj->getPosVec().getY();

                    if ((bGoingLeft && (fMapObjPosX - fBlockSizeXhalf > fBulletPosX)) ||
                        (!bGoingLeft && (fMapObjPosX + fBlockSizeXhalf < fBulletPosX)))
                    {
                        // optimization: rule out those blocks which are not in bullet's direction
                        continue;
                    }

                    const float fBulletPosXMinusHalf = fBulletPosX - bullet.getObject3D().getSizeVec().getX() / 2.f;
                    const float fBulletPosXPlusHalf = fBulletPosX - bullet.getObject3D().getSizeVec().getX() / 2.f;
                    const float fBulletPosYMinusHalf = fBulletPosY - bullet.getObject3D().getSizeVec().getY() / 2.f;
                    const float fBulletPosYPlusHalf = fBulletPosY - bullet.getObject3D().getSizeVec().getY() / 2.f;

                    if ((fMapObjPosX + fBlockSizeXhalf < fBulletPosXMinusHalf) || (fMapObjPosX - fBlockSizeXhalf > fBulletPosXPlusHalf))
                    {
                        continue;
                    }

                    if ((fMapObjPosY + fBlockSizeYhalf < fBulletPosYMinusHalf) || (fMapObjPosY - fBlockSizeYhalf > fBulletPosYPlusHalf))
                    {
                        continue;
                    }

                    bDeleteBullet = true;
                    break; // we can stop since 1 bullet can touch only 1 map element
                }
            }
        }

        if (bDeleteBullet)
        {
            // make explosion first if required;
            // server does not send specific message to client about creating explosion, it is client's responsibility to create explosion
            // when it receives MsgBulletUpdateFromServer with bDelete flag set, if bullet has area damage!
            if (!bEndGame)
            {
                if (bullet.getAreaDamageSize() > 0.f)
                {
                    createExplosionServer(
                        bullet.getOwner(),
                        bullet.getObject3D().getPosVec(),
                        bullet.getAreaDamageSize(),
                        bullet.getAreaDamagePulse(),
                        bullet.getDamageHp(),
                        objXHair,
                        vecCamShakeForce);
                }
            }

            // TODO: we should have a separate msg for deleting Bullet because its size would be much less than this msg!
            // But now we have to stick to this because explosion create on client-side requires all values, see details in:
            // handleBulletUpdateFromServer
            proofps_dd::MsgBulletUpdateFromServer::initPkt(
                newPktBulletUpdate,
                bullet.getOwner(),
                bullet.getId(),
                fBulletPosX,
                fBulletPosY,
                bullet.getObject3D().getPosVec().getZ(),
                bullet.getObject3D().getAngleVec().getX(),
                bullet.getObject3D().getAngleVec().getY(),
                bullet.getObject3D().getAngleVec().getZ(),
                bullet.getObject3D().getSizeVec().getX(),
                bullet.getObject3D().getSizeVec().getY(),
                bullet.getObject3D().getSizeVec().getZ(),
                bullet.getSpeed(),
                bullet.getGravity(),
                bullet.getDrag(),
                bullet.getDamageHp(),
                bullet.getAreaDamageSize(),
                bullet.getAreaDamagePulse()
                );
            // clients will also delete this bullet on their side because we set pkt's delete flag here
            proofps_dd::MsgBulletUpdateFromServer::getDelete(newPktBulletUpdate) = true;
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
                    fBulletPosX,
                    fBulletPosY,
                    bullet.getObject3D().getPosVec().getZ(),
                    bullet.getObject3D().getAngleVec().getX(),
                    bullet.getObject3D().getAngleVec().getY(),
                    bullet.getObject3D().getAngleVec().getZ(),
                    bullet.getObject3D().getSizeVec().getX(),
                    bullet.getObject3D().getSizeVec().getY(),
                    bullet.getObject3D().getSizeVec().getZ(),
                    bullet.getSpeed(),
                    bullet.getGravity(),
                    bullet.getDrag(),
                    bullet.getDamageHp(),
                    bullet.getAreaDamageSize(),
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
        Bullet::ResetGlobalBulletId();
    }

    m_durations.m_nUpdateBulletsDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void proofps_dd::WeaponHandling::clientUpdateBullets(const unsigned int& nPhysicsRate)
{
    // on the long run this function needs to be part of the game engine itself, however currently game engine doesn't handle collisions,
    // so once we introduce the collisions to the game engine, it will be an easy move of this function as well there
    std::list<Bullet>& bullets = m_pge.getBullets();
    auto it = bullets.begin();
    while (it != bullets.end())
    {
        auto& bullet = *it;

        // since v0.1.4, client simulates bullet movement, without any delete condition check, because delete happens only if server says so!
        // This can also mean that with higher latency, clients can render bullets moving over/into walls, players, etc. but it doesnt matter
        // because still the server is the authorative entity, and such visual anomalies might happen only for moments only.
        bullet.Update(nPhysicsRate);

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

    const bool bEndGame = gameMode.checkWinningConditions();
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
        Explosion::ResetGlobalExplosionId();
    }
}

void proofps_dd::WeaponHandling::clientUpdateExplosions(proofps_dd::GameMode& gameMode, const unsigned int& nPhysicsRate)
{
    // remember that on client-side, all explosions have id 0 because we simply do not set anything;
    // for now we can do exactly what server does with explosions

    serverUpdateExplosions(gameMode, nPhysicsRate);
}

bool proofps_dd::WeaponHandling::initializeWeaponHandling()
{
    // Which key should switch to which weapon
    WeaponManager::getKeypressToWeaponMap() = {
        {'2', "pistol.txt"},
        {'3', "machinegun.txt"},
        {'4', "bazooka.txt"}
    };

    Explosion::ResetGlobalExplosionId();
    return Explosion::initExplosionsReference(m_pge);
}


float proofps_dd::WeaponHandling::getDamageAndImpactForceAtDistance(
    const Player& player,
    const Explosion& xpl,
    const TPureFloat& fDamageAreaPulse,
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

    const float fRadiusDamage = xpl.getDamageAtDistance(fDistance, nDamageHp);

    if (fRadiusDamage > 0.f)
    {
        // to determine the direction of impact, we should use the center positions of player and explosion, however
        // to determine the magnitude of impact, we should use the edges/corners of player and explosion center per axis.
        // That is why fRadiusDamage itself is not good to be used for magnitude, as it is NOT per-axis.
        const float fPlayerWidthHeightRatio = player.getObject3D()->getScaledSizeVec().getX() / player.getObject3D()->getScaledSizeVec().getY();
        const float fImpactX = fDamageAreaPulse * fPlayerWidthHeightRatio * vDirPerAxis.getX() * std::max(0.f, (1 - (vDistancePerAxis.getX() / xpl.getDamageAreaSize())));
        const float fImpactY = fDamageAreaPulse * vDirPerAxis.getY() * std::max(0.f, (1 - (vDistancePerAxis.getY() / xpl.getDamageAreaSize())));
        //getConsole().EOLn("WeaponHandling::%s(): fX: %f, fY: %f!", __func__, fImpactX, fImpactY);
        vecImpactForce.Set(fImpactX, fImpactY, 0.f);
    }

    return fRadiusDamage;
}

proofps_dd::Explosion& proofps_dd::WeaponHandling::createExplosionServer(
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const PureVector& pos,
    const TPureFloat& fDamageAreaSize,
    const TPureFloat& fDamageAreaPulse,
    const int& nDamageHp,
    PureObject3D& objXHair,
    PureVector& vecCamShakeForce)
{
    m_explosions.push_back(
        Explosion(
            m_pge.getPure(),
            connHandle,
            pos,
            fDamageAreaSize));
    
    const Explosion& xpl = m_explosions.back();

    m_pge.getAudio().play(m_sounds.m_sndExplosion);

    // apply area damage to players
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        const auto& playerConst = player;

        if (playerConst.getHealth() <= 0)
        {
            continue;
        }

        PureVector vecImpactForce;
        const float fRadiusDamage = getDamageAndImpactForceAtDistance(
            playerConst, xpl, fDamageAreaPulse, nDamageHp, vecImpactForce
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
            
            player.doDamage(static_cast<int>(std::lroundf(fRadiusDamage)));
            //getConsole().EOLn("WeaponHandling::%s(): damage: %d!", __func__, static_cast<int>(std::lroundf(fRadiusDamage)));
            if (playerConst.getHealth() == 0)
            {
                const auto itKiller = m_mapPlayers.find(xpl.getOwner());
                pge_network::PgeNetworkConnectionHandle nKillerConnHandleServerSide;
                if (itKiller == m_mapPlayers.end())
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
                    nKillerConnHandleServerSide = itKiller->first;

                    // unlike in serverUpdateBullets(), here the owner of the explosion can kill even themself, so
                    // in that case frags should be decremented!
                    if (playerConst.getServerSideConnectionHandle() == xpl.getOwner())
                    {
                        itKiller->second.getFrags()--;
                    }
                    else
                    {
                        itKiller->second.getFrags()++;
                    }
                    //getConsole().OLn("WeaponHandling::%s(): Player %s has been killed by %s, who now has %d frags!",
                    //    __func__, playerPair.first.c_str(), itKiller->first.c_str(), itKiller->second.getFrags());
                }
                // server handles death here, clients will handle it when they receive MsgUserUpdateFromServer
                handlePlayerDied(player, objXHair, nKillerConnHandleServerSide);
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
    const TPureFloat& fDamageAreaPulse,
    PureVector& vecCamShakeForce)
{
    m_explosions.push_back(
        Explosion(
            m_pge.getPure(),
            id /* explosion id is not used on client-side */,
            connHandle,
            pos,
            fDamageAreaSize));

    Explosion& xpl = m_explosions.back();

    m_pge.getAudio().play(m_sounds.m_sndExplosion);

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
        PureVector vecImpactForce;
        const float fRadiusDamage = getDamageAndImpactForceAtDistance(
            playerConst, xpl, fDamageAreaPulse, nDamageHp, vecImpactForce
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

void proofps_dd::WeaponHandling::deleteWeaponHandlingAll()
{
    m_explosions.clear();
    Explosion::destroyExplosionsReference();
    Explosion::ResetGlobalExplosionId();

    m_pge.getBullets().clear();
    Bullet::ResetGlobalBulletId();
}

void proofps_dd::WeaponHandling::serverUpdateWeapons(proofps_dd::GameMode& gameMode)
{
    if (gameMode.checkWinningConditions())
    {
        return;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    for (auto& playerPair : m_mapPlayers)
    {
        const pge_network::PgeNetworkConnectionHandle& playerServerSideConnHandle = playerPair.first;
        Player& player = playerPair.second;
        Weapon* const wpn = player.getWeaponManager().getCurrentWeapon();
        if (!wpn)
        {
            continue;
        }

        bool bSendPkt = false;

        if (player.getAttack() && player.attack())
        {
            //getConsole().EOLn("WeaponHandling::%s(): player %u attack", __func__, playerServerSideConnHandle);
            // server will have the new bullet, clients will learn about the new bullet when server is sending out
            // the regular bullet updates;
            // but we send out the wpn update for bullet count change here for that single client
            if (playerServerSideConnHandle != pge_network::ServerConnHandle)
            {
                // server doesn't need to send this msg to itself, it already executed bullet count change by pullTrigger() in player.attack()
                bSendPkt = true;
            }
            else
            {
                // here server plays the firing sound, clients play for themselves when they receive newborn bullet update;
                // not nice, but this is just some temporal solution for private beta
                if (wpn->getFilename() == "pistol.txt")
                {
                    m_pge.getAudio().play(m_sounds.m_sndShootPistol);
                }
                else if (wpn->getFilename() == "machinegun.txt")
                {
                    m_pge.getAudio().play(m_sounds.m_sndShootMchgun);
                }
                else if (wpn->getFilename() == "bazooka.txt")
                {
                    m_pge.getAudio().play(m_sounds.m_sndShootBazooka);
                }
                else
                {
                    getConsole().EOLn("WeaponHandling::%s(): did not find correct weapon name for: %s!", __func__, wpn->getFilename().c_str());
                    assert(false);
                }
            }
        }  // end player.getAttack() && attack()

        if (wpn->update())
        {
            if (playerServerSideConnHandle != pge_network::ServerConnHandle)
            {
                // server doesn't need to send this msg to itself, it already executed bullet count change by wpn->update()
                bSendPkt = true;
            }
        }

        if (bSendPkt)
        {
            pge_network::PgePacket pktWpnUpdate;
            proofps_dd::MsgWpnUpdateFromServer::initPkt(
                pktWpnUpdate,
                pge_network::ServerConnHandle /* ignored by client anyway */,
                wpn->getFilename(),
                wpn->isAvailable(),
                wpn->getMagBulletCount(),
                wpn->getUnmagBulletCount());
            m_pge.getNetwork().getServer().send(pktWpnUpdate, playerServerSideConnHandle);
        }
    }  // end for playerPair

    m_durations.m_nUpdateWeaponsDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
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

    if (msg.m_bDelete)
    {
        // Make explosion first if required;
        // unlike server, client makes explosion when server says a bullet should be deleted, in such case client's job is to make explosion
        // with same parameters as server makes it, based on bullet properties such as position, area damage size, etc., this is why it is important
        // that server sends bullet position also when bDelete flag is set, so the explosion position will be the same as on server side!
        // Since client simulates bullet travel for visual purpose only, not doing collision check, it relies on server to know when a hit happened,
        // and for that obviously we need the server-side position of bullet at the moment of hit.
        // Also, we actually require other bullet parameters as well for making the explosion, because we might not have the bullet in getBullets() on
        // our side: when 2 players are almost at the same position (partially overlapping), one fires the weapon, then bullet will immediately hit on
        // the other player on server-side, we will receive a bullet delete request only from server in that case without any bullet create request prior to it!
        if (msg.m_fDamageAreaSize > 0.f)
        {
            createExplosionClient(
                0 /* explosion id is not used on client-side */,
                connHandleServerSide,
                /* server puts the last calculated bullet positions into message when it asks us to delete the bullet so we put explosion here */
                PureVector(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z),
                msg.m_nDamageHp,
                msg.m_fDamageAreaSize,
                msg.m_fDamageAreaPulse,
                vecCamShakeForce);
        }
    }

    Bullet* pBullet = nullptr;

    auto it = m_pge.getBullets().begin();
    while (it != m_pge.getBullets().end())
    {
        if (it->getId() == msg.m_bulletId)
        {
            break;
        }
        it++;
    }

    if (m_pge.getBullets().end() == it)
    {
        if (msg.m_bDelete)
        {
            // this is valid scenario: explained a few lines earlier
            return true;
        }
        // need to create this new bullet first on our side
        //getConsole().OLn("WeaponHandling::%s(): received MsgBulletUpdateFromServer: NEW bullet id %u", __func__, msg.m_bulletId);

        const auto playerIt = m_mapPlayers.find(m_nServerSideConnectionHandle);
        if (playerIt == m_mapPlayers.end())
        {
            // must always find self player
            assert(false);
            return false;
        }

        Player& player = playerIt->second;
        if (player.getServerSideConnectionHandle() == connHandleServerSide)
        {
            // this is my newborn bullet
            // I'm playing the sound associated to my current weapon, although it might happen that with BIG latency, when I receive this update from server,
            // I have already switched to another weapon ... but I think this cannot happen since my inputs are processed and responded by server in order.
            const Weapon* const wpn = player.getWeaponManager().getCurrentWeapon();
            if (!wpn)
            {
                getConsole().EOLn("WeaponHandling::%s(): getWeapon() failed!", __func__);
                assert(false);
                return false;
            }
            else
            {
                // not nice, but this is just some temporal solution for private beta
                if (wpn->getFilename() == "pistol.txt")
                {
                    m_pge.getAudio().play(m_sounds.m_sndShootPistol);
                }
                else if (wpn->getFilename() == "machinegun.txt")
                {
                    m_pge.getAudio().play(m_sounds.m_sndShootMchgun);
                }
                else if (wpn->getFilename() == "bazooka.txt")
                {
                    m_pge.getAudio().play(m_sounds.m_sndShootBazooka);
                }
                else
                {
                    getConsole().EOLn("WeaponHandling::%s(): did not find correct weapon name for: %s!", __func__, wpn->getFilename().c_str());
                    assert(false);
                    return false;
                }
            }
        }

        m_pge.getBullets().push_back(
            Bullet(
                m_pge.getPure(),
                msg.m_bulletId,
                msg.m_pos.x, msg.m_pos.y, msg.m_pos.z,
                msg.m_angle.x, msg.m_angle.y, msg.m_angle.z,
                msg.m_size.x, msg.m_size.y, msg.m_size.z,
                msg.m_fSpeed, msg.m_fGravity, msg.m_fDrag, msg.m_nDamageHp,
                msg.m_fDamageAreaSize, msg.m_fDamageAreaPulse));
        pBullet = &(m_pge.getBullets().back());
        it = m_pge.getBullets().end();
        it--; // iterator points to this newly inserted last bullet
    }
    else
    {
        //getConsole().OLn("WeaponHandling::%s(): received MsgBulletUpdateFromServer: old bullet id %u", __func__, msg.m_bulletId);
        //pBullet = &(*it);

        if (!msg.m_bDelete)
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

    //getConsole().OLn("WeaponHandling::%s(): received: %s, available: %s, mag: %u, unmag: %u!",
    //    __func__, msg.m_szWpnName, msg.m_bAvailable ? "yes" : "no", msg.m_nMagBulletCount, msg.m_nUnmagBulletCount);

    const auto playerIt = m_mapPlayers.find(m_nServerSideConnectionHandle);
    if (playerIt == m_mapPlayers.end())
    {
        // must always find self player
        return false;
    }

    Player& player = playerIt->second;
    Weapon* const wpn = player.getWeaponManager().getWeaponByFilename(msg.m_szWpnName);
    if (!wpn)
    {
        getConsole().EOLn("WeaponHandling::%s(): did not find wpn: %s!", __func__, msg.m_szWpnName);
        assert(false);
        return false;
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

    //getConsole().OLn("WeaponHandling::%s(): received: %s for player %u",  __func__, msg.m_szWpnCurrentName, connHandleServerSide);

    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("WeaponHandling::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        assert(false);
        return false;
    }

    auto& player = it->second;
    Weapon* const wpn = player.getWeaponManager().getWeaponByFilename(msg.m_szWpnCurrentName);
    if (!wpn)
    {
        getConsole().EOLn("WeaponHandling::%s(): did not find wpn: %s!", __func__, msg.m_szWpnCurrentName);
        assert(false);
        return false;
    }

    if (isMyConnection(it->first))
    {
        //getConsole().OLn("WeaponHandling::%s(): this current weapon update is changing my current weapon!", __func__);
        m_pge.getAudio().play(m_sounds.m_sndChangeWeapon);
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

    player.getWeaponManager().getCurrentWeapon()->UpdatePosition(player.getObject3D()->getPosVec(), player.isSomersaulting());

    return true;
}


// ############################### PRIVATE ###############################
