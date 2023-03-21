#pragma once

/*
    ###################################################################################
    PlayerTest.h
    Unit test for PRooFPS-dd Player class.
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <string.h>

#include <cassert>

#include "../../../../PGE/PGE/UnitTests/UnitTest.h"
#include "../Player.h"
#include "../PRooFPS-dd-packet.h"

class PlayerTest :
    public UnitTest
{
public:

    PlayerTest(PGEcfgProfiles& cfgProfiles) :
        UnitTest(__FILE__),
        m_cfgProfiles(cfgProfiles),
        engine(nullptr),
        wm(nullptr)
    {}

protected:

    virtual void Initialize()
    {
        //CConsole::getConsoleInstance().SetLoggingState(Player::getLoggerModuleName(), true);

        PGEInputHandler& inputHandler = PGEInputHandler::createAndGet(m_cfgProfiles);

        engine = &PR00FsUltimateRenderingEngine::createAndGet(m_cfgProfiles, inputHandler);
        engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        wm = new WeaponManager(m_cfgProfiles, *engine);

        if (assertNotNull(wm, "wm null") &&
            assertTrue(wm->load("gamedata/weapons/pistol.txt", 0), "wm wpn load pistol") &&
            assertTrue(wm->setDefaultAvailableWeaponByFilename("pistol.txt"), "wm set default wpn") &&
            assertTrue(wm->load("gamedata/weapons/machinegun.txt", 0), "wm wpn load mchgun"))
        {
            AddSubTest("test_initial_values", (PFNUNITSUBTEST)&PlayerTest::test_initial_values);
            AddSubTest("test_update_frags_deaths", (PFNUNITSUBTEST)&PlayerTest::test_update_frags_deaths);
            AddSubTest("test_set_expecting_start_pos", (PFNUNITSUBTEST)&PlayerTest::test_set_expecting_start_pos);
            AddSubTest("test_update_old_pos", (PFNUNITSUBTEST)&PlayerTest::test_update_old_pos);
            AddSubTest("test_set_health", (PFNUNITSUBTEST)&PlayerTest::test_set_health);
            AddSubTest("test_update_old_health", (PFNUNITSUBTEST)&PlayerTest::test_update_old_health);
            AddSubTest("test_do_damage", (PFNUNITSUBTEST)&PlayerTest::test_do_damage);
            AddSubTest("test_die_server", (PFNUNITSUBTEST)&PlayerTest::test_die_server);
            AddSubTest("test_die_client", (PFNUNITSUBTEST)&PlayerTest::test_die_client);
            AddSubTest("test_respawn", (PFNUNITSUBTEST)&PlayerTest::test_respawn);
            AddSubTest("test_jump", (PFNUNITSUBTEST)&PlayerTest::test_jump);
            AddSubTest("test_set_can_fall", (PFNUNITSUBTEST)&PlayerTest::test_set_can_fall);
            AddSubTest("test_gravity", (PFNUNITSUBTEST)&PlayerTest::test_gravity);
            AddSubTest("test_set_run", (PFNUNITSUBTEST)&PlayerTest::test_set_run);
            AddSubTest("test_set_weapon", (PFNUNITSUBTEST)&PlayerTest::test_set_weapon);
            AddSubTest("test_can_take_item_health", (PFNUNITSUBTEST)&PlayerTest::test_can_take_item_health);
            AddSubTest("test_can_take_item_weapon", (PFNUNITSUBTEST)&PlayerTest::test_can_take_item_weapon);
            AddSubTest("test_take_item_health", (PFNUNITSUBTEST)&PlayerTest::test_take_item_health);
            AddSubTest("test_take_item_weapon", (PFNUNITSUBTEST)&PlayerTest::test_take_item_weapon);
        }
    }

    virtual bool setUp()
    {
        if (assertNotNull(wm, "wm null"))
        {
            return assertFalse(wm->getWeapons().empty(), "wm empty") &
                assertTrue(engine && engine->isInitialized(), "engine inited");
        }
        return false;
    }

    virtual void TearDown()
    {

    }

    virtual void Finalize()
    {
        if (wm)
        {
            wm->Clear();
            delete wm;
            wm = nullptr;
        }

        if (engine)
        {
            engine->shutdown();
            engine = NULL;
        }

        CConsole::getConsoleInstance().SetLoggingState(Player::getLoggerModuleName(), false);
    }

private:

    PGEcfgProfiles& m_cfgProfiles;
    PR00FsUltimateRenderingEngine* engine;
    WeaponManager* wm;

    // ---------------------------------------------------------------------------

    PlayerTest(const PlayerTest&) :
        m_cfgProfiles(m_cfgProfiles),
        engine(nullptr),
        wm(nullptr)
    {};

    PlayerTest& operator=(const PlayerTest&)
    {
        return *this;
    };

    bool loadWeaponsForPlayer(Player& player)
    {
        for (const auto pSrcWpn : wm->getWeapons())
        {
            assert(pSrcWpn);
            Weapon* pNewWpn = new Weapon(*pSrcWpn);
            if (!pNewWpn)
            {
                return false;
            }

            player.getWeapons().push_back(pNewWpn);
            pNewWpn->SetOwner(player.getServerSideConnectionHandle());
        }
        player.getWeapons()[0]->SetAvailable(true);
        player.SetWeapon(player.getWeapons()[0], false);

        return true;
    }

    bool test_initial_values()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        const std::string sIpAddr = "192.168.1.12";

        Player player(*engine, connHandleExpected, sIpAddr);

        return assertEquals(connHandleExpected, player.getServerSideConnectionHandle(), "connhandle") &
            assertEquals(sIpAddr, player.getIpAddress(), "ip address") &
            assertNotNull(player.getObject3D(), "object3d") &
            assertEquals(100, player.getOldHealth(), "old health") &
            assertEquals(100, player.getHealth(), "health") &
            assertEquals(0, player.getOldDeaths(), "old deaths") &
            assertEquals(0, player.getDeaths(), "deaths") &
            assertEquals(0, player.getOldFrags(), "old frags") &
            assertEquals(0, player.getFrags(), "frags") &
            assertEquals(0, player.getTimeDied().time_since_epoch().count(), "time died") &
            assertEquals(0, player.getTimeLastWeaponSwitch().time_since_epoch().count(), "time last wpn switch") &
            assertTrue(player.canFall(), "can fall") &
            assertTrue(player.isFalling(), "falling") &
            assertFalse(player.jumpAllowed(), "can jump") &
            assertFalse(player.isJumping(), "jumping") &
            assertTrue(player.isExpectingStartPos(), "expecting start pos") &
            assertTrue(player.isRunning(), "running default") &
            assertFalse(player.getRespawnFlag(), "respawn flag") &
            assertEquals(PureVector(), player.getForce(), "force") &
            assertEquals(PureVector(), player.getOldWeaponAngle(), "old wpn angle") &
            assertEquals(PureVector(), player.getWeaponAngle(), "wpn angle") &
            assertEquals(PureVector(), player.getOPos(), "old pos") &
            assertEquals(PureVector(), player.getPos(), "pos") &
            assertEquals(0.f, player.getOldAngleY(), "old angle y") &
            assertEquals(0.f, player.getAngleY(), "angle y") &
            assertEquals(0.f, player.getGravity(), "gravity") &
            assertNull(player.getWeapon(), "current weapon") &
            assertTrue(player.getWeapons().empty(), "weapons") /* weapons need to be manually loaded and added, maybe this change in future */;
    }

    bool test_update_frags_deaths()
    {
        Player player(*engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.getFrags() = 2;
        player.getDeaths() = 1;

        bool b = assertEquals(0, player.getOldDeaths(), "old deaths 1") &
            assertEquals(1, player.getDeaths(), "deaths 1") &
            assertEquals(0, player.getOldFrags(), "old frags 1") &
            assertEquals(2, player.getFrags(), "frags 1");

        player.UpdateFragsDeaths();

        b &= assertEquals(player.getDeaths(), player.getOldDeaths(), "old deaths 2") &
            assertEquals(1, player.getDeaths(), "deaths 2") &
            assertEquals(player.getFrags(), player.getOldFrags(), "old frags 2") &
            assertEquals(2, player.getFrags(), "frags 2");
        
        return b;
    }

    bool test_set_expecting_start_pos()
    {
        Player player(*engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetExpectingStartPos(false);
        bool b = assertFalse(player.isExpectingStartPos(), "exp 1");
        
        player.SetExpectingStartPos(true);
        b &= assertTrue(player.isExpectingStartPos(), "exp 2");

        return b;
    }

    bool test_update_old_pos()
    {
        Player player(*engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        const PureVector vecPosOriginal(1.f, 2.f, 3.f);
        const TPureFloat fAngleYOriginal = 90.f;
        const PureVector vecAngleWpnOriginal(10.f, 20.f, 30.f);

        player.getPos() = vecPosOriginal;
        player.getAngleY() = fAngleYOriginal;
        player.getWeaponAngle() = vecAngleWpnOriginal;

        bool b = assertEquals(PureVector(), player.getOldWeaponAngle(), "old wpn angle 1")&
            assertEquals(vecAngleWpnOriginal, player.getWeaponAngle(), "wpn angle 1")&
            assertEquals(PureVector(), player.getOPos(), "old pos 1")&
            assertEquals(vecPosOriginal, player.getPos(), "pos 1")&
            assertEquals(0.f, player.getOldAngleY(), "old angle y 1")&
            assertEquals(fAngleYOriginal, player.getAngleY(), "angle y 1");

        player.UpdateOldPos();

        b &= assertEquals(vecAngleWpnOriginal, player.getOldWeaponAngle(), "old wpn angle 2") &
            assertEquals(vecAngleWpnOriginal, player.getWeaponAngle(), "wpn angle 2") &
            assertEquals(vecPosOriginal, player.getOPos(), "old pos 2") &
            assertEquals(vecPosOriginal, player.getPos(), "pos 2") &
            assertEquals(fAngleYOriginal, player.getOldAngleY(), "old angle y 2") &
            assertEquals(fAngleYOriginal, player.getAngleY(), "angle y 2");

        return b;
    }

    bool test_set_health()
    {
        Player player(*engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetHealth(200);
        bool b = assertEquals(100, player.getHealth(), "health 1");

        player.SetHealth(-1);
        b &= assertEquals(0, player.getHealth(), "health 2");

        return b;
    }

    bool test_update_old_health()
    {
        Player player(*engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetHealth(50);
        bool b = assertEquals(50, player.getHealth(), "health 1") &
            assertEquals(100, player.getOldHealth(), "old health 1");

        player.UpdateOldHealth();

        b &= assertEquals(50, player.getHealth(), "health 2") &
            assertEquals(player.getHealth(), player.getOldHealth(), "old health 2");

        return b;
    }

    bool test_do_damage()
    {
        Player player(*engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.DoDamage(25);
        bool b = assertEquals(75, player.getHealth(), "health 1") &
            assertEquals(100, player.getOldHealth(), "old health 1");

        player.DoDamage(25);
        b &= assertEquals(50, player.getHealth(), "health 2") &
            assertEquals(100, player.getOldHealth(), "old health 2");

        player.DoDamage(100);
        b &= assertEquals(0, player.getHealth(), "health 3") &
            assertEquals(100, player.getOldHealth(), "old health 3");

        return b;
    }

    bool test_die_server()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        Player player(*engine, connHandleExpected, "192.168.1.12");
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };

        player.Die(true, true);
        const auto nFirstTimeDiedSinceEpoch = player.getTimeDied().time_since_epoch().count();
        bool b = assertEquals(0, player.getHealth(), "health 1") &
            assertEquals(100, player.getOldHealth(), "old health 1") &
            assertNotEquals(0, nFirstTimeDiedSinceEpoch, "time died a 1") &
            assertFalse(player.getObject3D()->isRenderingAllowed(), "player object visible 1") &
            assertFalse(player.getWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 1") &
            assertEquals(1, player.getDeaths(), "deaths 1") /* server increases it */ &
            assertEquals(0, player.getOldDeaths(), "old deaths 1");
        
        player.SetHealth(100);
        player.Respawn(true, *(player.getWeapons()[0]));
        
        player.Die(true, true);
        const auto nSecondTimeDiedSinceEpoch = player.getTimeDied().time_since_epoch().count();
        b &= assertEquals(0, player.getHealth(), "health 2") &
            assertEquals(100, player.getOldHealth(), "old health 2") &
            assertNotEquals(0, nSecondTimeDiedSinceEpoch, "time died a 2") &
            assertNotEquals(nFirstTimeDiedSinceEpoch, nSecondTimeDiedSinceEpoch, "time died b 2") &
            assertFalse(player.getObject3D()->isRenderingAllowed(), "player object visible 2") &
            assertFalse(player.getWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 2") &
            assertEquals(2, player.getDeaths(), "deaths 2") /* server increases it */ &
            assertEquals(0, player.getOldDeaths(), "old deaths 2");

        return b;
    }

    bool test_die_client()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        Player player(*engine, connHandleExpected, "192.168.1.12");
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };

        player.Die(true, false);
        const auto nFirstTimeDiedSinceEpoch = player.getTimeDied().time_since_epoch().count();
        bool b = assertEquals(0, player.getHealth(), "health 1") &
            assertEquals(100, player.getOldHealth(), "old health 1") &
            assertNotEquals(0, nFirstTimeDiedSinceEpoch, "time died 1") &
            assertFalse(player.getObject3D()->isRenderingAllowed(), "player object visible 1") &
            assertFalse(player.getWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 1") &
            assertEquals(0, player.getDeaths(), "deaths 1") /* client doesn't change it, will receive update from server */ &
            assertEquals(0, player.getOldDeaths(), "old deaths 1");

        player.SetHealth(100);
        player.Respawn(true, *(player.getWeapons()[0]));

        player.Die(true, false);
        const auto nSecondTimeDiedSinceEpoch = player.getTimeDied().time_since_epoch().count();
        b &= assertEquals(0, player.getHealth(), "health 2") &
            assertEquals(100, player.getOldHealth(), "old health 2") &
            assertNotEquals(0, nSecondTimeDiedSinceEpoch, "time died a 2") &
            assertNotEquals(nFirstTimeDiedSinceEpoch, nSecondTimeDiedSinceEpoch, "time died b 2") &
            assertFalse(player.getObject3D()->isRenderingAllowed(), "player object visible 2") &
            assertFalse(player.getWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 2") &
            assertEquals(0, player.getDeaths(), "deaths 2") /* client doesn't change it, will receive update from server */ &
            assertEquals(0, player.getOldDeaths(), "old deaths 2");

        return b;
    }

    bool test_respawn()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        Player player(*engine, connHandleExpected, "192.168.1.12");
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };
        player.getWeapons()[1]->SetAvailable(true);
        player.SetWeapon(player.getWeapons()[1], false);

        player.Die(true, true);
        player.Respawn(true, *(player.getWeapons()[0]));

        return assertTrue(player.getObject3D()->isRenderingAllowed(), "player object visible") &
            assertTrue(player.getWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible") &
            assertFalse(player.getWeapons()[1]->isAvailable(), "wpn 2 not available") &
            assertEquals(player.getWeapons()[0], player.getWeapon(), "current wpn");
    }

    bool test_jump()
    {
        Player player(*engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        player.getPos().Set(2.f, 4.f, 8.f);
        player.getOPos().Set(1.f, 2.f, 3.f);
        const PureVector vecExpectedForce = player.getPos() - player.getOPos();

        player.SetJumpAllowed(true);
        bool b = assertTrue(player.jumpAllowed(), "allowed 1") &
            assertFalse(player.isJumping(), "jumping 1") &
            assertEquals(0.f, player.getGravity(), "gravity 1") &
            assertEquals(PureVector(), player.getForce(), "force 1") &
            assertTrue(player.isFalling(), "falling 1");

        player.Jump();
        b &= assertFalse(player.jumpAllowed(), "allowed 2") &
            assertTrue(player.isJumping(), "jumping 2") &
            assertEquals(GAME_GRAVITY_MAX, player.getGravity(), "gravity 2") &
            assertEquals(vecExpectedForce, player.getForce(), "force 2") &
            assertFalse(player.isFalling(), "falling 2");

        player.StopJumping();
        b &= assertFalse(player.jumpAllowed(), "allowed 3") &
            assertFalse(player.isJumping(), "jumping 3") &
            assertEquals(GAME_GRAVITY_MAX, player.getGravity(), "gravity 3") &
            assertEquals(vecExpectedForce, player.getForce(), "force 3") &
            assertFalse(player.isFalling(), "falling 3");

        player.SetJumpAllowed(false);
        player.Jump();
        b &= assertFalse(player.jumpAllowed(), "allowed 4") &
            assertFalse(player.isJumping(), "jumping 4") &
            assertEquals(GAME_GRAVITY_MAX, player.getGravity(), "gravity 4") &
            assertEquals(vecExpectedForce, player.getForce(), "force 4") &
            assertFalse(player.isFalling(), "falling 4");

        return b;
    }

    bool test_set_can_fall()
    {
        Player player(*engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetCanFall(false);
        bool b = assertFalse(player.canFall(), "can fall 1");

        player.SetCanFall(true);
        b &= assertTrue(player.canFall(), "can fall 2");

        return b;
    }

    bool test_gravity()
    {
        Player player(*engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetGravity(5.f);

        return assertEquals(5.f, player.getGravity());
    }

    bool test_set_run()
    {
        Player player(*engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetRun(false);
        
        return assertFalse(player.isRunning());
    }

    bool test_set_weapon()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        Player player(*engine, connHandleExpected, "192.168.1.12");
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };
        
        player.SetWeapon(player.getWeapons()[1], false);
        bool b = assertEquals(player.getWeapons()[0], player.getWeapon(), "current wpn 1");

        player.getWeapons()[1]->SetAvailable(true);
        player.SetWeapon(player.getWeapons()[1], false);
        b &= assertEquals(player.getWeapons()[1], player.getWeapon(), "current wpn 2");

        return b;
    }

    bool test_can_take_item_health()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        Player player(*engine, connHandleExpected, "192.168.1.12");
        const MapItem miHealth(*engine, MapItemType::ITEM_HEALTH, PureVector(1, 2, 3));

        bool b = assertFalse(player.canTakeItem(miHealth), "1");

        player.SetHealth(50);
        b &= assertTrue(player.canTakeItem(miHealth), "2");

        return b;
    }

    bool test_can_take_item_weapon()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        Player player(*engine, connHandleExpected, "192.168.1.12");
        const MapItem miPistol(*engine, MapItemType::ITEM_WPN_PISTOL, PureVector(1, 2, 3));
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };

        bool b = assertTrue(player.canTakeItem(miPistol), "1");

        player.getWeapons()[0]->SetUnmagBulletCount(player.getWeapons()[0]->getVars()["cap_max"].getAsInt());
        b &= assertFalse(player.canTakeItem(miPistol), "2");

        return b;
    }

    bool test_take_item_health()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        Player player(*engine, connHandleExpected, "192.168.1.12");
        MapItem miHealth(*engine, MapItemType::ITEM_HEALTH, PureVector(1, 2, 3));
        pge_network::PgePacket newPktWpnUpdate;

        player.SetHealth(90);
        player.TakeItem(miHealth, newPktWpnUpdate);

        return assertEquals(100, player.getHealth(), "player health") &
            assertTrue(miHealth.isTaken(), "item taken");
    }

    bool test_take_item_weapon()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        Player player(*engine, connHandleExpected, "192.168.1.12");
        MapItem miPistol(*engine, MapItemType::ITEM_WPN_PISTOL, PureVector(1, 2, 3));
        MapItem miMchGun(*engine, MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        pge_network::PgePacket pktWpnUpdate;
        const proofps_dd::MsgWpnUpdate& msgWpnUpdate = reinterpret_cast<const proofps_dd::MsgWpnUpdate&>(pktWpnUpdate.msg.app.cData);
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };

        player.TakeItem(miPistol, pktWpnUpdate);
        bool bStrSafeChecked = strncmp(
            player.getWeapons()[0]->getFilename().c_str(),
            msgWpnUpdate.m_szWpnName,
            proofps_dd::MsgWpnUpdate::nWpnNameNameMaxLength) == 0;

        bool b = assertEquals(player.getWeapons()[0]->getVars()["reloadable"].getAsInt(),
            static_cast<int>(player.getWeapons()[0]->getUnmagBulletCount()), "wpn 1 unmag") &
            assertTrue(miPistol.isTaken(), "item 1 taken") &
            assertEquals(static_cast<uint32_t>(pge_network::MsgApp::id),
                static_cast<uint32_t>(pktWpnUpdate.pktId),
                "pkt 1 id") &
            assertEquals(static_cast<uint32_t>(proofps_dd::MsgWpnUpdate::id),
                static_cast<uint32_t>(static_cast<proofps_dd::ElteFailMsgId>(pktWpnUpdate.msg.app.msgId)),
                "msg 1 id") &
            assertTrue(bStrSafeChecked, "msg 1 sWeaponBecomingAvailable") &
            assertTrue(msgWpnUpdate.m_bAvailable, "msg wpn 1 becoming available") &
            assertEquals(player.getWeapons()[0]->getVars()["reloadable"].getAsInt(),
                static_cast<int>(msgWpnUpdate.m_nMagBulletCount), "msg wpn 1 mag") &
            assertEquals(player.getWeapons()[0]->getVars()["reloadable"].getAsInt() /* we already had pistol, so we expect unmag count to be non-zero in msg */,
                static_cast<int>(msgWpnUpdate.m_nUnmagBulletCount), "msg wpn 1 unmag");

        player.TakeItem(miMchGun, pktWpnUpdate);
        bStrSafeChecked = strncmp(
            player.getWeapons()[1]->getFilename().c_str(),
            msgWpnUpdate.m_szWpnName,
            proofps_dd::MsgWpnUpdate::nWpnNameNameMaxLength) == 0;

        b &= assertEquals(player.getWeapons()[1]->getVars()["reloadable"].getAsInt(),
            static_cast<int>(player.getWeapons()[1]->getMagBulletCount()), "wpn 2 mag") &
            assertTrue(player.getWeapons()[1]->isAvailable(), "wpn 2 available") &
            assertTrue(miMchGun.isTaken(), "item 2 taken") &
            assertEquals(static_cast<uint32_t>(pge_network::MsgApp::id),
                static_cast<uint32_t>(pktWpnUpdate.pktId),
                "pkt 2 id") &
            assertEquals(static_cast<uint32_t>(proofps_dd::MsgWpnUpdate::id),
                static_cast<uint32_t>(static_cast<proofps_dd::ElteFailMsgId>(pktWpnUpdate.msg.app.msgId)),
                "msg 2 id") &
            assertTrue(bStrSafeChecked, "msg 2 sWeaponBecomingAvailable") &
            assertTrue(msgWpnUpdate.m_bAvailable, "msg wpn 2 becoming available") &
            assertEquals(player.getWeapons()[1]->getVars()["reloadable"].getAsInt(),
                static_cast<int>(msgWpnUpdate.m_nMagBulletCount), "msg wpn 2 mag") &
            assertEquals(0 /* we didnt have machinegun yet, so we expect unmag count to be 0 in msg */,
                static_cast<int>(msgWpnUpdate.m_nUnmagBulletCount), "msg wpn 2 unmag");

        return b;
    }

};