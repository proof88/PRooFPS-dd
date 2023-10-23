/*
    ###################################################################################
    WeaponHandling.cpp
    Weapon handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>
#include <chrono>

#include "WeaponHandling.h"


// ############################### PUBLIC ################################


proofps_dd::WeaponHandling::WeaponHandling(
    PGE& pge,
    proofps_dd::Durations& durations,
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    proofps_dd::Maps& maps,
    proofps_dd::Sounds& sounds) :
    proofps_dd::Physics(
        pge,
        m_durations,
        m_mapPlayers,
        m_maps,
        m_sounds),
    proofps_dd::PlayerHandling(pge, durations, maps, sounds),
    proofps_dd::UserInterface(pge),
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
    // be extisting at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
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
        m_maps.getBlocksVertexPosMin().getX() - proofps_dd::GAME_BLOCK_SIZE_X * 4,
        m_maps.getBlocksVertexPosMin().getY() - proofps_dd::GAME_BLOCK_SIZE_Y,
        m_maps.getBlocksVertexPosMin().getZ() - proofps_dd::GAME_BLOCK_SIZE_Z); // ah why dont we have vector-scalar subtract operator defined ...
    const PureVector vRelaxedMapMaxBounds(
        m_maps.getBlocksVertexPosMax().getX() + proofps_dd::GAME_BLOCK_SIZE_X * 4,
        m_maps.getBlocksVertexPosMax().getY() + proofps_dd::GAME_BLOCK_SIZE_Y,
        m_maps.getBlocksVertexPosMax().getZ() + proofps_dd::GAME_BLOCK_SIZE_Z);
    
    return !Colliding3(vRelaxedMapMinBounds, vRelaxedMapMaxBounds, bullet.getObject3D().getPosVec(), bullet.getObject3D().getSizeVec());
}

void proofps_dd::WeaponHandling::serverUpdateBullets(proofps_dd::GameMode& gameMode, PureObject3D& objXHair, const unsigned int& nTickRate)
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    // on the long run this function needs to be part of the game engine itself, however currently game engine doesn't handle collisions,
    // so once we introduce the collisions to the game engine, it will be an easy move of this function as well there
    pge_network::PgePacket newPktBulletUpdate;
    const float fBlockSizeXhalf = proofps_dd::GAME_BLOCK_SIZE_X / 2.f;
    const float fBlockSizeYhalf = proofps_dd::GAME_BLOCK_SIZE_Y / 2.f;
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
            bullet.Update(nTickRate);
        }

        const float fBulletPosX = bullet.getObject3D().getPosVec().getX();
        const float fBulletPosY = bullet.getObject3D().getPosVec().getY();

        if (!bDeleteBullet)
        {
            // check if bullet is hitting a player
            for (auto& playerPair : m_mapPlayers)
            {
                if (bullet.getOwner() == playerPair.second.getServerSideConnectionHandle())
                {
                    // bullet cannot hit the owner, at least for now ...
                    // in the future, when bullets start in proper position, we won't need this check ...
                    // this check will be bad anyway in future when we will have the guided rockets that actually can hit the owner if guided in suicide way!
                    continue;
                }

                if ((playerPair.second.getHealth() > 0) &&
                    Colliding(*(playerPair.second.getObject3D()), bullet.getObject3D()))
                {
                    bDeleteBullet = true;
                    playerPair.second.DoDamage(bullet.getDamageHp());
                    if (playerPair.second.getHealth() == 0)
                    {
                        const auto itKiller = m_mapPlayers.find(bullet.getOwner());
                        if (itKiller == m_mapPlayers.end())
                        {
                            //getConsole().OLn("WeaponHandling::%s(): Player %s has been killed by a player already left!",
                            //    __func__, playerPair.first.c_str());
                        }
                        else
                        {
                            itKiller->second.getFrags()++;
                            bEndGame = gameMode.checkWinningConditions();
                            //getConsole().OLn("WeaponHandling::%s(): Player %s has been killed by %s, who now has %d frags!",
                            //    __func__, playerPair.first.c_str(), itKiller->first.c_str(), itKiller->second.getFrags());
                        }
                        // server handles death here, clients will handle it when they receive MsgUserUpdateFromServer
                        HandlePlayerDied(playerPair.second, objXHair);
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
            // TODO: we should have a separate msg for deleting Bullet because its size would be much less than this msg!
            proofps_dd::MsgBulletUpdateFromServer::initPktForDeleting_WithGarbageValues(
                newPktBulletUpdate,
                pge_network::ServerConnHandle,
                bullet.getId()); // clients will also delete this bullet on their side because we set pkt's delete flag here
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
                    bullet.getDrag());
                m_pge.getNetwork().getServer().sendToAllClientsExcept(newPktBulletUpdate);
            }
            // bullet didn't touch anything, go to next
            it++;
            // since v.0.1.4, server doesn't send the bullet travel updates to clients since clients simulate the travel in clientUpdateBullets()
        }

        // 'it' is referring to next bullet, don't use it from here!
    }

    if (bEndGame && (Bullet::getGlobalBulletId() > 0))
    {
        Bullet::ResetGlobalBulletId();
    }

    m_durations.m_nUpdateBulletsDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
}

void proofps_dd::WeaponHandling::clientUpdateBullets(const unsigned int& nTickRate)
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
        bullet.Update(nTickRate);

        // There was a time when I was thinking that client should check against out of map bounds to cover corner case when we somehow miss
        // the server's signal about that, in that case client would continue simulate bullet travel forever.
        // However, I decided not to handle that because it could also introduce some unwanted effect: imagine that client detects out of map
        // bounds earlier, so it deletes the bullet, however in next moment a bullet update for the same bullet is coming from the server,
        // then client would have to create that bullet again which would bring performance penalty. Then next moment another msg from server
        // would come about deleting the bullet. So I think we should just wait anyway for server to tell us delete bullet for any reason.
        it++;
    }
}

void proofps_dd::WeaponHandling::deleteWeaponHandlingAll()
{
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
                // here server plays the firing sound, clients play for themselves when they receive newborn bullet update
                // not nice, but this is just some temporal solution for private beta
                if (wpn->getFilename() == "pistol.txt")
                {
                    m_pge.getAudio().play(m_sounds.m_sndShootPistol);
                }
                else if (wpn->getFilename() == "machinegun.txt")
                {
                    m_pge.getAudio().play(m_sounds.m_sndShootMchgun);
                }
                else
                {
                    getConsole().EOLn("InputHandling::%s(): did not find correct weapon name for: %s!", __func__, wpn->getFilename().c_str());
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

bool proofps_dd::WeaponHandling::handleBulletUpdateFromServer(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgBulletUpdateFromServer& msg)
{
    if (m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("WeaponHandling::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
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
            // this is valid scenario: when 2 players are at almost same position (partially overlapping), the bullet will immediately hit the other player
            // when being fired. In such case, we can just ignore doing anything here on client side.
            // TODO: btw why does sender send the message like this anyway to clients?!
            return true;
        }
        // need to create this new bullet first on our side
        //getConsole().OLn("WeaponHandling::%s(): received MsgBulletUpdateFromServer: NEW bullet id %u", __func__, msg.m_bulletId);

        const auto playerIt = m_mapPlayers.find(m_nServerSideConnectionHandle);
        if (playerIt == m_mapPlayers.end())
        {
            // must always find self player
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
                msg.m_fSpeed, msg.m_fGravity, msg.m_fDrag));
        pBullet = &(m_pge.getBullets().back());
        it = m_pge.getBullets().end();
        it--; // iterator points to this newly inserted last bullet
    }
    else
    {
        //getConsole().OLn("WeaponHandling::%s(): received MsgBulletUpdateFromServer: old bullet id %u", __func__, msg.m_bulletId);
        pBullet = &(*it);

        if (!msg.m_bDelete)
        {
            // for a known bullet, client should not receive position updates from server, so log error!
            getConsole().EOLn("WeaponHandling::%s(): received non-delete update for already known bullet, MUST NOT HAPPEN!", __func__);
        }
    }

    if (msg.m_bDelete)
    {
        m_pge.getBullets().erase(it);
        return true;
    }

    return true;
}

bool proofps_dd::WeaponHandling::handleWpnUpdateFromServer(
    pge_network::PgeNetworkConnectionHandle /* connHandleServerSide, not filled properly by server so we ignore it */,
    const proofps_dd::MsgWpnUpdateFromServer& msg,
    bool bHasValidConnection)
{
    if (m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("WeaponHandling::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    //getConsole().OLn("WeaponHandling::%s(): received: %s, available: %s, mag: %u, unmag: %u!",
    //    __func__, msg.m_szWpnName, msg.m_bAvailable ? "yes" : "no", msg.m_nMagBulletCount, msg.m_nUnmagBulletCount);

    if (!bHasValidConnection)
    {
        getConsole().EOLn("WeaponHandling::%s(): my connection is invalid, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

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

    Weapon* const wpn = it->second.getWeaponManager().getWeaponByFilename(msg.m_szWpnCurrentName);
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

    if (!it->second.getWeaponManager().setCurrentWeapon(wpn,
        true /* even client should record last switch time to be able to check it on client side too */,
        m_pge.getNetwork().isServer()))
    {
        getConsole().EOLn("WeaponHandling::%s(): player %s switching to %s failed due to setCurrentWeapon() failed!",
            __func__, it->second.getName().c_str(), wpn->getFilename().c_str());
        assert(false);
        return false;
    }

    it->second.getWeaponManager().getCurrentWeapon()->UpdatePosition(it->second.getObject3D()->getPosVec());

    return true;
}


// ############################### PRIVATE ###############################
