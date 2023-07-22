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

#include "../../../PGE/PGE/UnitTests/UnitTest.h"
#include "../Player.h"
#include "../PRooFPS-dd-packet.h"

class PlayerTest :
    public UnitTest
{
public:

    PlayerTest(PGEcfgProfiles& cfgProfiles) :
        UnitTest(__FILE__),
        m_cfgProfiles(cfgProfiles),
        engine(nullptr)
    {}

    PlayerTest(const PlayerTest&) = delete;
    PlayerTest& operator=(const PlayerTest&) = delete;
    PlayerTest(PlayerTest&&) = delete;
    PlayerTest&& operator=(PlayerTest&&) = delete;

protected:

    virtual void Initialize() override
    {
        //CConsole::getConsoleInstance().SetLoggingState(proofps_dd::Player::getLoggerModuleName(), true);

        PGEInputHandler& inputHandler = PGEInputHandler::createAndGet(m_cfgProfiles);

        engine = &PR00FsUltimateRenderingEngine::createAndGet(m_cfgProfiles, inputHandler);
        engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        AddSubTest("test_initial_values", (PFNUNITSUBTEST)&PlayerTest::test_initial_values);
        AddSubTest("test_number_in_name_monotonically_increasing", (PFNUNITSUBTEST)&PlayerTest::test_number_in_name_monotonically_increasing);
        AddSubTest("test_set_name", (PFNUNITSUBTEST)&PlayerTest::test_set_name);
        AddSubTest("test_dirtiness_one_by_one", (PFNUNITSUBTEST)&PlayerTest::test_dirtiness_one_by_one);
        AddSubTest("test_update_old_frags_and_deaths", (PFNUNITSUBTEST)&PlayerTest::test_update_old_frags_and_deaths);
        AddSubTest("test_set_expecting_start_pos", (PFNUNITSUBTEST)&PlayerTest::test_set_expecting_start_pos);
        AddSubTest("test_update_old_pos", (PFNUNITSUBTEST)&PlayerTest::test_update_old_pos);
        AddSubTest("test_set_health_and_update_old_health", (PFNUNITSUBTEST)&PlayerTest::test_set_health_and_update_old_health);
        AddSubTest("test_do_damage", (PFNUNITSUBTEST)&PlayerTest::test_do_damage);
        AddSubTest("test_die_server", (PFNUNITSUBTEST)&PlayerTest::test_die_server);
        AddSubTest("test_die_client", (PFNUNITSUBTEST)&PlayerTest::test_die_client);
        AddSubTest("test_respawn", (PFNUNITSUBTEST)&PlayerTest::test_respawn);
        AddSubTest("test_jump", (PFNUNITSUBTEST)&PlayerTest::test_jump);
        AddSubTest("test_set_can_fall", (PFNUNITSUBTEST)&PlayerTest::test_set_can_fall);
        AddSubTest("test_gravity", (PFNUNITSUBTEST)&PlayerTest::test_gravity);
        AddSubTest("test_set_run", (PFNUNITSUBTEST)&PlayerTest::test_set_run);
        AddSubTest("test_set_strafe", (PFNUNITSUBTEST)&PlayerTest::test_set_strafe);
        AddSubTest("test_can_take_item_health", (PFNUNITSUBTEST)&PlayerTest::test_can_take_item_health);
        AddSubTest("test_can_take_item_weapon", (PFNUNITSUBTEST)&PlayerTest::test_can_take_item_weapon);
        AddSubTest("test_take_item_health", (PFNUNITSUBTEST)&PlayerTest::test_take_item_health);
        AddSubTest("test_take_item_weapon", (PFNUNITSUBTEST)&PlayerTest::test_take_item_weapon);
    }

    virtual bool setUp() override
    {
        return assertTrue(engine && engine->isInitialized(), "engine inited");
    }

    virtual void TearDown() override
    {

    }

    virtual void Finalize() override
    {
        if (engine)
        {
            engine->shutdown();
            engine = NULL;
        }

        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::Player::getLoggerModuleName(), false);
    }

private:

    PGEcfgProfiles& m_cfgProfiles;
    PR00FsUltimateRenderingEngine* engine;
    std::list<Bullet> m_bullets;

    // ---------------------------------------------------------------------------

    bool loadWeaponsForPlayer(proofps_dd::Player& player)
    {
        bool b = assertTrue(player.getWeaponManager().load("gamedata/weapons/pistol.txt", 0), "wm wpn load pistol");
        b &= assertTrue(player.getWeaponManager().setDefaultAvailableWeaponByFilename("pistol.txt"), "wm set default wpn");
        b &= assertTrue(player.getWeaponManager().load("gamedata/weapons/machinegun.txt", 0), "wm wpn load mchgun");

        for (const auto pSrcWpn : player.getWeaponManager().getWeapons())
        {
            pSrcWpn->SetOwner(player.getServerSideConnectionHandle());
        }
        player.getWeaponManager().getWeapons()[0]->SetAvailable(true);
        b &= assertTrue(player.getWeaponManager().setCurrentWeapon(player.getWeaponManager().getWeapons()[0], false, true), "wm current wpn");

        return b;
    }

    bool test_initial_values()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        const std::string sIpAddr = "192.168.1.12";

        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, sIpAddr);

        return assertEquals(connHandleExpected, player.getServerSideConnectionHandle(), "connhandle") &
            assertEquals(sIpAddr, player.getIpAddress(), "ip address") &
            assertFalse(player.getName().empty(), "name") &
            assertNotNull(player.getObject3D(), "object3d") &
            assertFalse(player.getHealth().isDirty(), "old health") &
            assertEquals(100, player.getHealth(), "health") &
            assertFalse(player.getDeaths().isDirty(), "old deaths") &
            assertEquals(0, player.getDeaths(), "deaths") &
            assertFalse(player.getFrags().isDirty(), "old frags") &
            assertEquals(0, player.getFrags(), "frags") &
            assertEquals(0, player.getTimeDied().time_since_epoch().count(), "time died") &
            assertEquals(0, player.getWeaponManager().getTimeLastWeaponSwitch().time_since_epoch().count(), "time last wpn switch") &
            assertTrue(player.canFall(), "can fall") &
            assertTrue(player.isFalling(), "falling") &
            assertFalse(player.jumpAllowed(), "can jump") &
            assertFalse(player.isJumping(), "jumping") &
            assertFalse(player.getWillJump(), "will jump") &
            assertTrue(player.isExpectingStartPos(), "expecting start pos") &
            assertTrue(player.isRunning(), "running default") &
            assertEquals(proofps_dd::Strafe::NONE, player.getStrafe(), "strafe") &
            assertFalse(player.getRespawnFlag(), "respawn flag") &
            assertEquals(PureVector(), player.getForce(), "force") &
            assertFalse(player.getWeaponAngle().isDirty(), "old wpn angle") &
            assertEquals(PureVector(), player.getWeaponAngle(), "wpn angle") &
            assertFalse(player.getPos().isDirty(), "old pos") &
            assertEquals(PureVector(), player.getPos(), "pos") &
            assertFalse(player.getAngleY().isDirty(), "old angle y") &
            assertEquals(0.f, player.getAngleY(), "angle y") &
            assertEquals(0.f, player.getGravity(), "gravity") &
            assertNull(player.getWeaponManager().getCurrentWeapon(), "current weapon") &
            assertTrue(player.getWeaponManager().getWeapons().empty(), "weapons") /* weapons need to be manually loaded and added, maybe this change in future */;
    }

    bool test_number_in_name_monotonically_increasing()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        const std::string sIpAddr = "192.168.1.12";

        proofps_dd::Player player1(m_cfgProfiles, m_bullets, *engine, connHandleExpected, sIpAddr);
        {
            proofps_dd::Player player2(m_cfgProfiles, m_bullets, *engine, connHandleExpected, sIpAddr);
            // implicit call to dtor
        }
        proofps_dd::Player player3(m_cfgProfiles, m_bullets, *engine, connHandleExpected, sIpAddr);

        const std::size_t nSpacePos1 = player1.getName().find(' ');
        const uint32_t nPlayer1cntr = (nSpacePos1 < player1.getName().length()-1) ?
            std::stoul(player1.getName().substr(nSpacePos1 + 1)) : 0;

        const std::size_t nSpacePos3 = player3.getName().find(' ');
        const uint32_t nPlayer3cntr = (nSpacePos3 < player3.getName().length() - 1) ?
            std::stoul(player3.getName().substr(nSpacePos3 + 1)) : 0;

        return assertGreater(nPlayer1cntr, 0u, "nPlayer1cntr") &
            assertGreater(nPlayer3cntr, nPlayer1cntr + 1, "nPlayer3cntr");
    }

    bool test_set_name()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        const std::string sIpAddr = "192.168.1.12";

        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, sIpAddr);
        player.setName("apple");
        bool b = assertEquals("apple", player.getName(), "apple");

        return b;
    }

    bool test_dirtiness_one_by_one()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        bool b = assertFalse(player.isDirty(), "dirty 1");

        player.getHealth().set(5);
        b &= assertTrue(player.isDirty(), "dirty A 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty A 2");

        player.getFrags().set(5);
        b &= assertTrue(player.isDirty(), "dirty B 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty B 2");

        player.getDeaths().set(5);
        b &= assertTrue(player.isDirty(), "dirty C 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty C 2");

        player.getAngleY().set(5.f);
        b &= assertTrue(player.isDirty(), "dirty D 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty D 2");

        player.getWeaponAngle().set(PureVector(5.f, 6.f, 7.f));
        b &= assertTrue(player.isDirty(), "dirty E 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty E 2");

        player.getPos().set(PureVector(5.f, 6.f, 7.f));
        b &= assertTrue(player.isDirty(), "dirty F 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty F 2");

        return b;
    }

    bool test_update_old_frags_and_deaths()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.getFrags() = 2;
        player.getDeaths() = 1;

        bool b = assertEquals(0, player.getDeaths().getOld(), "old deaths 1") &
            assertEquals(1, player.getDeaths(), "deaths 1") &
            assertEquals(0, player.getFrags().getOld(), "old frags 1") &
            assertEquals(2, player.getFrags(), "frags 1") &
            assertTrue(player.isDirty(), "dirty 1");

        player.updateOldValues();

        b &= assertEquals(player.getDeaths(), player.getDeaths().getOld(), "old deaths 2") &
            assertEquals(1, player.getDeaths(), "deaths 2") &
            assertEquals(player.getFrags(), player.getFrags().getOld(), "old frags 2") &
            assertEquals(2, player.getFrags(), "frags 2") &
            assertFalse(player.isDirty(), "dirty 2");
        
        return b;
    }

    bool test_set_expecting_start_pos()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetExpectingStartPos(false);
        bool b = assertFalse(player.isExpectingStartPos(), "exp 1");
        
        player.SetExpectingStartPos(true);
        b &= assertTrue(player.isExpectingStartPos(), "exp 2");

        return b;
    }

    bool test_update_old_pos()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        const PureVector vecPosOriginal(1.f, 2.f, 3.f);
        const TPureFloat fAngleYOriginal = 90.f;
        const PureVector vecAngleWpnOriginal(10.f, 20.f, 30.f);

        player.getPos() = vecPosOriginal;
        player.getAngleY() = fAngleYOriginal;
        player.getWeaponAngle() = vecAngleWpnOriginal;

        bool b = assertEquals(PureVector(), player.getWeaponAngle().getOld(), "old wpn angle 1") &
            assertEquals(vecAngleWpnOriginal, player.getWeaponAngle(), "wpn angle 1")&
            assertEquals(PureVector(), player.getPos().getOld(), "old pos 1") &
            assertEquals(vecPosOriginal, player.getPos(), "pos 1") &
            assertEquals(0.f, player.getAngleY().getOld(), "old angle y 1") &
            assertEquals(fAngleYOriginal, player.getAngleY(), "angle y 1") &
            assertTrue(player.isDirty(), "dirty 1");

        player.updateOldValues();

        b &= assertEquals(vecAngleWpnOriginal, player.getWeaponAngle().getOld(), "old wpn angle 2") &
            assertEquals(vecAngleWpnOriginal, player.getWeaponAngle(), "wpn angle 2") &
            assertEquals(vecPosOriginal, player.getPos().getOld(), "old pos 2") &
            assertEquals(vecPosOriginal, player.getPos(), "pos 2") &
            assertEquals(fAngleYOriginal, player.getAngleY().getOld(), "old angle y 2") &
            assertEquals(fAngleYOriginal, player.getAngleY(), "angle y 2") &
            assertFalse(player.isDirty(), "dirty 2");

        return b;
    }

    bool test_set_health_and_update_old_health()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetHealth(200);
        bool b = assertEquals(100, player.getHealth(), "health 1") &
            assertFalse(player.getHealth().isDirty(), "dirty 1") &
            assertFalse(player.isDirty(), "dirty 2");

        player.SetHealth(-1);
        b &= assertEquals(0, player.getHealth(), "health 2") &
            assertTrue(player.getHealth().isDirty(), "dirty 3") &
            assertTrue(player.isDirty(), "dirty 4");

        player.updateOldValues();
        b &= assertEquals(0, player.getHealth().getOld(), "health 3") &
            assertFalse(player.getHealth().isDirty(), "dirty 5") &
            assertFalse(player.isDirty(), "dirty 6");

        return b;
    }

    bool test_do_damage()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.DoDamage(25);
        bool b = assertEquals(75, player.getHealth(), "health 1") &
            assertEquals(100, player.getHealth().getOld(), "old health 1");

        player.DoDamage(25);
        b &= assertEquals(50, player.getHealth(), "health 2") &
            assertEquals(100, player.getHealth().getOld(), "old health 2");

        player.DoDamage(100);
        b &= assertEquals(0, player.getHealth(), "health 3") &
            assertEquals(100, player.getHealth().getOld(), "old health 3");

        return b;
    }

    bool test_die_server()
    {
        const bool bServer = true;
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };

        player.Die(true, bServer);
        const auto nFirstTimeDiedSinceEpoch = player.getTimeDied().time_since_epoch().count();
        bool b = assertEquals(0, player.getHealth(), "health 1") &
            assertEquals(100, player.getHealth().getOld(), "old health 1") &
            assertNotEquals(0, nFirstTimeDiedSinceEpoch, "time died a 1") &
            assertFalse(player.getObject3D()->isRenderingAllowed(), "player object visible 1") &
            assertFalse(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 1") &
            assertEquals(1, player.getDeaths(), "deaths 1") /* server increases it */ &
            assertEquals(0, player.getDeaths().getOld(), "old deaths 1");
        
        player.SetHealth(100);
        player.Respawn(true, *(player.getWeaponManager().getWeapons()[0]), bServer);
        
        player.Die(true, bServer);
        const auto nSecondTimeDiedSinceEpoch = player.getTimeDied().time_since_epoch().count();
        b &= assertEquals(0, player.getHealth(), "health 2") &
            assertEquals(100, player.getHealth().getOld(), "old health 2") &
            assertNotEquals(0, nSecondTimeDiedSinceEpoch, "time died a 2") &
            assertNotEquals(nFirstTimeDiedSinceEpoch, nSecondTimeDiedSinceEpoch, "time died b 2") &
            assertFalse(player.getObject3D()->isRenderingAllowed(), "player object visible 2") &
            assertFalse(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 2") &
            assertEquals(2, player.getDeaths(), "deaths 2") /* server increases it */ &
            assertEquals(0, player.getDeaths().getOld(), "old deaths 2");

        return b;
    }

    bool test_die_client()
    {
        const bool bServer = false;
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };

        player.Die(true, bServer);
        const auto nFirstTimeDiedSinceEpoch = player.getTimeDied().time_since_epoch().count();
        bool b = assertEquals(0, player.getHealth(), "health 1") &
            assertEquals(100, player.getHealth().getOld(), "old health 1") &
            assertNotEquals(0, nFirstTimeDiedSinceEpoch, "time died 1") &
            assertFalse(player.getObject3D()->isRenderingAllowed(), "player object visible 1") &
            assertFalse(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 1") &
            assertEquals(0, player.getDeaths(), "deaths 1") /* client doesn't change it, will receive update from server */ &
            assertEquals(0, player.getDeaths().getOld(), "old deaths 1");

        player.SetHealth(100);
        player.Respawn(true, *(player.getWeaponManager().getWeapons()[0]), bServer);

        player.Die(true, bServer);
        const auto nSecondTimeDiedSinceEpoch = player.getTimeDied().time_since_epoch().count();
        b &= assertEquals(0, player.getHealth(), "health 2") &
            assertEquals(100, player.getHealth().getOld(), "old health 2") &
            assertNotEquals(0, nSecondTimeDiedSinceEpoch, "time died a 2") &
            assertNotEquals(nFirstTimeDiedSinceEpoch, nSecondTimeDiedSinceEpoch, "time died b 2") &
            assertFalse(player.getObject3D()->isRenderingAllowed(), "player object visible 2") &
            assertFalse(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 2") &
            assertEquals(0, player.getDeaths(), "deaths 2") /* client doesn't change it, will receive update from server */ &
            assertEquals(0, player.getDeaths().getOld(), "old deaths 2");

        return b;
    }

    bool test_respawn()
    {
        const bool bServer = true;
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };
        player.getWeaponManager().getWeapons()[1]->SetAvailable(true);
        bool b = assertTrue(player.getWeaponManager().setCurrentWeapon(player.getWeaponManager().getWeapons()[1], false, bServer), "set current wpn");

        player.Die(true, bServer);
        player.Respawn(true, *(player.getWeaponManager().getWeapons()[0]), bServer);

        return b & assertTrue(player.getObject3D()->isRenderingAllowed(), "player object visible") &
            assertTrue(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible") &
            assertFalse(player.getWeaponManager().getWeapons()[1]->isAvailable(), "wpn 2 not available") &
            assertEquals(player.getWeaponManager().getWeapons()[0], player.getWeaponManager().getCurrentWeapon(), "current wpn");
    }

    bool test_jump()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.getPos().set(PureVector(1.f, 2.f, 3.f));
        player.getPos().commit();
        player.getPos().set(PureVector(2.f, 4.f, 8.f));

        const PureVector vecExpectedForce = player.getPos().getNew() - player.getPos().getOld();

        player.SetJumpAllowed(true);
        bool b = assertTrue(player.jumpAllowed(), "allowed 1") &
            assertFalse(player.isJumping(), "jumping 1") &
            assertEquals(0.f, player.getGravity(), "gravity 1") &
            assertEquals(PureVector(), player.getForce(), "force 1") &
            assertTrue(player.isFalling(), "falling 1");

        player.setWillJump(true);
        b &= assertTrue(player.getWillJump(), "will jump 1");
        player.Jump();
        b &= assertFalse(player.jumpAllowed(), "allowed 2") &
            assertTrue(player.isJumping(), "jumping 2") &
            assertEquals(proofps_dd::GAME_JUMP_GRAVITY_START, player.getGravity(), "gravity 2") &
            assertEquals(vecExpectedForce, player.getForce(), "force 2") &
            assertFalse(player.isFalling(), "falling 2") &
            assertFalse(player.getWillJump(), "will jump 2");

        player.StopJumping();
        b &= assertFalse(player.jumpAllowed(), "allowed 3") &
            assertFalse(player.isJumping(), "jumping 3") &
            assertEquals(proofps_dd::GAME_JUMP_GRAVITY_START, player.getGravity(), "gravity 3") &
            assertEquals(vecExpectedForce, player.getForce(), "force 3") &
            assertFalse(player.isFalling(), "falling 3");

        player.SetJumpAllowed(false);
        player.setWillJump(true);
        b &= assertFalse(player.getWillJump(), "will jump 3");

        player.Jump();
        b &= assertFalse(player.jumpAllowed(), "allowed 4") &
            assertFalse(player.isJumping(), "jumping 4") &
            assertEquals(proofps_dd::GAME_JUMP_GRAVITY_START, player.getGravity(), "gravity 4") &
            assertEquals(vecExpectedForce, player.getForce(), "force 4") &
            assertFalse(player.isFalling(), "falling 4");

        return b;
    }

    bool test_set_can_fall()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetCanFall(false);
        bool b = assertFalse(player.canFall(), "can fall 1");

        player.SetCanFall(true);
        b &= assertTrue(player.canFall(), "can fall 2");

        return b;
    }

    bool test_gravity()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetGravity(5.f);

        return assertEquals(5.f, player.getGravity());
    }

    bool test_set_run()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.SetRun(false);
        
        return assertFalse(player.isRunning());
    }

    bool test_set_strafe()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.setStrafe(proofps_dd::Strafe::LEFT);
        bool b = assertEquals(proofps_dd::Strafe::LEFT, player.getStrafe(), "1");

        player.setStrafe(proofps_dd::Strafe::RIGHT);
        b &= assertEquals(proofps_dd::Strafe::RIGHT, player.getStrafe(), "2");

        player.setStrafe(proofps_dd::Strafe::NONE);
        b &= assertEquals(proofps_dd::Strafe::NONE, player.getStrafe(), "3");

        return b;
    }

    bool test_can_take_item_health()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        const proofps_dd::MapItem miHealth(*engine, proofps_dd::MapItemType::ITEM_HEALTH, PureVector(1, 2, 3));

        bool b = assertFalse(player.canTakeItem(miHealth), "1");

        player.SetHealth(50);
        b &= assertTrue(player.canTakeItem(miHealth), "2");

        return b;
    }

    bool test_can_take_item_weapon()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        const proofps_dd::MapItem miPistol(*engine, proofps_dd::MapItemType::ITEM_WPN_PISTOL, PureVector(1, 2, 3));
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };

        bool b = assertTrue(player.canTakeItem(miPistol), "1");

        player.getWeaponManager().getWeapons()[0]->SetUnmagBulletCount(player.getWeaponManager().getWeapons()[0]->getVars()["cap_max"].getAsInt());
        b &= assertFalse(player.canTakeItem(miPistol), "2");

        return b;
    }

    bool test_take_item_health()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        proofps_dd::MapItem miHealth(*engine, proofps_dd::MapItemType::ITEM_HEALTH, PureVector(1, 2, 3));
        pge_network::PgePacket newPktWpnUpdate;

        player.SetHealth(90);
        player.TakeItem(miHealth, newPktWpnUpdate);

        return assertEquals(100, player.getHealth(), "player health") &
            assertTrue(miHealth.isTaken(), "item taken");
    }

    bool test_take_item_weapon()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        proofps_dd::MapItem miPistol(*engine, proofps_dd::MapItemType::ITEM_WPN_PISTOL, PureVector(1, 2, 3));
        proofps_dd::MapItem miMchGun(*engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        pge_network::PgePacket pktWpnUpdate;
        const proofps_dd::MsgWpnUpdate& msgWpnUpdate = reinterpret_cast<const proofps_dd::MsgWpnUpdate&>(pktWpnUpdate.msg.app.cData);
        if (!assertTrue(loadWeaponsForPlayer(player)))
        {
            return false;
        };

        player.TakeItem(miPistol, pktWpnUpdate);
        bool bStrSafeChecked = strncmp(
            player.getWeaponManager().getWeapons()[0]->getFilename().c_str(),
            msgWpnUpdate.m_szWpnName,
            proofps_dd::MsgWpnUpdate::nWpnNameNameMaxLength) == 0;

        bool b = assertEquals(player.getWeaponManager().getWeapons()[0]->getVars()["reloadable"].getAsInt(),
            static_cast<int>(player.getWeaponManager().getWeapons()[0]->getUnmagBulletCount()), "wpn 1 unmag") &
            assertTrue(miPistol.isTaken(), "item 1 taken") &
            assertEquals(static_cast<uint32_t>(pge_network::MsgApp::id),
                static_cast<uint32_t>(pktWpnUpdate.pktId),
                "pkt 1 id") &
            assertEquals(static_cast<uint32_t>(proofps_dd::MsgWpnUpdate::id),
                static_cast<uint32_t>(static_cast<proofps_dd::ElteFailMsgId>(pktWpnUpdate.msg.app.msgId)),
                "msg 1 id") &
            assertTrue(bStrSafeChecked, "msg 1 sWeaponBecomingAvailable") &
            assertTrue(msgWpnUpdate.m_bAvailable, "msg wpn 1 becoming available") &
            assertEquals(player.getWeaponManager().getWeapons()[0]->getVars()["reloadable"].getAsInt(),
                static_cast<int>(msgWpnUpdate.m_nMagBulletCount), "msg wpn 1 mag") &
            assertEquals(player.getWeaponManager().getWeapons()[0]->getVars()["reloadable"].getAsInt() /* we already had pistol, so we expect unmag count to be non-zero in msg */,
                static_cast<int>(msgWpnUpdate.m_nUnmagBulletCount), "msg wpn 1 unmag");

        player.TakeItem(miMchGun, pktWpnUpdate);
        bStrSafeChecked = strncmp(
            player.getWeaponManager().getWeapons()[1]->getFilename().c_str(),
            msgWpnUpdate.m_szWpnName,
            proofps_dd::MsgWpnUpdate::nWpnNameNameMaxLength) == 0;

        b &= assertEquals(player.getWeaponManager().getWeapons()[1]->getVars()["reloadable"].getAsInt(),
            static_cast<int>(player.getWeaponManager().getWeapons()[1]->getMagBulletCount()), "wpn 2 mag") &
            assertTrue(player.getWeaponManager().getWeapons()[1]->isAvailable(), "wpn 2 available") &
            assertTrue(miMchGun.isTaken(), "item 2 taken") &
            assertEquals(static_cast<uint32_t>(pge_network::MsgApp::id),
                static_cast<uint32_t>(pktWpnUpdate.pktId),
                "pkt 2 id") &
            assertEquals(static_cast<uint32_t>(proofps_dd::MsgWpnUpdate::id),
                static_cast<uint32_t>(static_cast<proofps_dd::ElteFailMsgId>(pktWpnUpdate.msg.app.msgId)),
                "msg 2 id") &
            assertTrue(bStrSafeChecked, "msg 2 sWeaponBecomingAvailable") &
            assertTrue(msgWpnUpdate.m_bAvailable, "msg wpn 2 becoming available") &
            assertEquals(player.getWeaponManager().getWeapons()[1]->getVars()["reloadable"].getAsInt(),
                static_cast<int>(msgWpnUpdate.m_nMagBulletCount), "msg wpn 2 mag") &
            assertEquals(0 /* we didnt have machinegun yet, so we expect unmag count to be 0 in msg */,
                static_cast<int>(msgWpnUpdate.m_nUnmagBulletCount), "msg wpn 2 unmag");

        return b;
    }

};