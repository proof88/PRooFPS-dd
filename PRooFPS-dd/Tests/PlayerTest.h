#pragma once

/*
    ###################################################################################
    PlayerTest.h
    Unit test for PRooFPS-dd Player class.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include <string.h>

#include <cassert>
#include <set>
#include <string>

#include "UnitTests/UnitTest.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"

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

        AddSubTest("test_gen_unique_user_name", (PFNUNITSUBTEST)&PlayerTest::test_gen_unique_user_name);
        AddSubTest("test_initial_values", (PFNUNITSUBTEST)&PlayerTest::test_initial_values);
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
        AddSubTest("test_set_has_just_started_falling", (PFNUNITSUBTEST)&PlayerTest::test_set_has_just_started_falling);
        AddSubTest("test_start_somersault_when_auto_crouch_is_disabled", (PFNUNITSUBTEST)&PlayerTest::test_start_somersault_when_auto_crouch_is_disabled);
        AddSubTest("test_start_somersault_when_auto_crouch_is_enabled", (PFNUNITSUBTEST)&PlayerTest::test_start_somersault_when_auto_crouch_is_enabled);
        AddSubTest("test_step_somersault_angle", (PFNUNITSUBTEST)&PlayerTest::test_step_somersault_angle);
        AddSubTest("test_set_run", (PFNUNITSUBTEST)&PlayerTest::test_set_run);
        AddSubTest("test_set_strafe", (PFNUNITSUBTEST)&PlayerTest::test_set_strafe);
        AddSubTest("test_server_do_crouch", (PFNUNITSUBTEST)&PlayerTest::test_server_do_crouch);
        AddSubTest("test_client_do_crouch", (PFNUNITSUBTEST)&PlayerTest::test_client_do_crouch);
        AddSubTest("test_server_do_standup", (PFNUNITSUBTEST)&PlayerTest::test_server_do_standup);
        AddSubTest("test_client_do_standup", (PFNUNITSUBTEST)&PlayerTest::test_client_do_standup);
        AddSubTest("test_attack", (PFNUNITSUBTEST)&PlayerTest::test_attack);
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
        m_bullets.clear();
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

    enum class SetDfltWpn
    {
        Yes,
        No
    };

    PGEcfgProfiles& m_cfgProfiles;
    PR00FsUltimateRenderingEngine* engine;
    std::list<Bullet> m_bullets;

    // ---------------------------------------------------------------------------

    bool loadWeaponsForPlayer(proofps_dd::Player& player, const SetDfltWpn& setDefWpn)
    {
        bool b = assertTrue(player.getWeaponManager().load("gamedata/weapons/pistol.txt", 0), "wm wpn load pistol");
        b &= assertTrue(player.getWeaponManager().setDefaultAvailableWeaponByFilename("pistol.txt"), "wm set default wpn");
        b &= assertTrue(player.getWeaponManager().load("gamedata/weapons/machinegun.txt", 0), "wm wpn load mchgun");
        b &= assertTrue(player.getWeaponManager().load("gamedata/weapons/bazooka.txt", 0), "wm wpn load bazooka");

        for (const auto pSrcWpn : player.getWeaponManager().getWeapons())
        {
            pSrcWpn->SetOwner(player.getServerSideConnectionHandle());
        }
        player.getWeaponManager().getWeapons()[0]->SetAvailable(true);

        if (setDefWpn == SetDfltWpn::Yes)
        {
            b &= assertTrue(player.getWeaponManager().setCurrentWeapon(player.getWeaponManager().getWeapons()[0], false, true), "wm current wpn");
        }

        return b;
    }

    bool test_gen_unique_user_name()
    {
        char szNewUserName[proofps_dd::MsgUserNameChange::nUserNameBufferLength];
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player> mapPlayers;
        bool b = true;

        // insert 20 players with same name, no name truncating should happen
        for (int i = 0; i < 20; i++)
        {
            const pge_network::PgeNetworkConnectionHandle connHandle = static_cast<pge_network::PgeNetworkConnectionHandle>(i);
            const auto insertRes = mapPlayers.insert(
                {
                    connHandle,
                    proofps_dd::Player(m_cfgProfiles, m_bullets, *engine, connHandle, std::string("192.168.1.") + std::to_string(i))
                });
            b &= insertRes.second;
            if (!b)
            {
                return assertFalse(true, (std::string("player ") + std::to_string(i) + " insert").c_str());
            }
            proofps_dd::Player::genUniqueUserName(szNewUserName, "Player", mapPlayers);
            proofps_dd::Player& insertedPlayer = insertRes.first->second;
            insertedPlayer.setName(szNewUserName);
        }

        // checkpoint 1
        std::set<std::string> namesSet;
        for (const auto& connHandlePlayerPair : mapPlayers)
        {
            b &= assertEquals("Player", connHandlePlayerPair.second.getName().substr(0, 6), "player name cp 1");
            const auto insertRes = namesSet.insert(connHandlePlayerPair.second.getName());
            b &= assertTrue(insertRes.second, "insert into set 1");
        }
        b &= assertEquals(mapPlayers.size(), namesSet.size(), "container sizes 1");

        mapPlayers.clear();
        namesSet.clear();

        // insert 20 players with same name, but now name truncating should happen
        for (int i = 0; i < 20; i++)
        {
            const pge_network::PgeNetworkConnectionHandle connHandle = static_cast<pge_network::PgeNetworkConnectionHandle>(i);
            const auto insertRes = mapPlayers.insert(
                {
                    connHandle,
                    proofps_dd::Player(m_cfgProfiles, m_bullets, *engine, connHandle, std::string("192.168.1.") + std::to_string(i))
                });
            b &= insertRes.second;
            if (!b)
            {
                return assertFalse(true, (std::string("player ") + std::to_string(i) + " insert").c_str());
            }
            proofps_dd::Player::genUniqueUserName(szNewUserName, "ASDASDADSASDASDASD", mapPlayers);
            proofps_dd::Player& insertedPlayer = insertRes.first->second;
            insertedPlayer.setName(szNewUserName);
        }

        // checkpoint 2
        for (const auto& connHandlePlayerPair : mapPlayers)
        {
            b &= assertEquals("ASDASDA", connHandlePlayerPair.second.getName().substr(0, 7), "player name cp 2") &
                assertEquals(connHandlePlayerPair.second.getName().length(), proofps_dd::MsgUserNameChange::nUserNameBufferLength-1u, "player name length cp 2");
            const auto insertRes = namesSet.insert(connHandlePlayerPair.second.getName());
            b &= assertTrue(insertRes.second, "insert into set 2");
        }
        b &= assertEquals(mapPlayers.size(), namesSet.size(), "container sizes 2");

        mapPlayers.clear();
        namesSet.clear();
        
        // try inserting with empty name
        for (int i = 0; i < 20; i++)
        {
            const pge_network::PgeNetworkConnectionHandle connHandle = static_cast<pge_network::PgeNetworkConnectionHandle>(i);
            const auto insertRes = mapPlayers.insert(
                {
                    connHandle,
                    proofps_dd::Player(m_cfgProfiles, m_bullets, *engine, connHandle, std::string("192.168.1.") + std::to_string(i))
                });
            b &= insertRes.second;
            if (!b)
            {
                return assertFalse(true, (std::string("player ") + std::to_string(i) + " insert").c_str());
            }
            proofps_dd::Player::genUniqueUserName(szNewUserName, "", mapPlayers);
            proofps_dd::Player& insertedPlayer = insertRes.first->second;
            insertedPlayer.setName(szNewUserName);
            b &= assertFalse(insertedPlayer.getName().empty(), ("player " + std::to_string(i) + " name empty").c_str());
        }

        // checkpoint 3
        for (const auto& connHandlePlayerPair : mapPlayers)
        {
            const auto insertRes = namesSet.insert(connHandlePlayerPair.second.getName());
            b &= assertTrue(insertRes.second, "insert into set 3");
        }
        b &= assertEquals(mapPlayers.size(), namesSet.size(), "container sizes 3");

        mapPlayers.clear();
        namesSet.clear();
        
        return b;
    }

    bool test_initial_values()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        const std::string sIpAddr = "192.168.1.12";

        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, sIpAddr);
        const auto& playerConst = player;

        return (assertEquals(connHandleExpected, player.getServerSideConnectionHandle(), "connhandle") &
            assertEquals(sIpAddr, player.getIpAddress(), "ip address") &
            assertTrue(player.getName().empty(), "name") &
            assertNotNull(player.getObject3D(), "object3d") &
            assertFalse(player.isDirty(), "isDirty") &
            assertFalse(player.isNetDirty(), "isNetDirty") &
            assertFalse(playerConst.getHealth().isDirty(), "old health") &
            assertEquals(100, playerConst.getHealth(), "health") &
            assertFalse(player.getDeaths().isDirty(), "old deaths") &
            assertEquals(0, player.getDeaths(), "deaths") &
            assertFalse(player.getFrags().isDirty(), "old frags") &
            assertEquals(0, player.getFrags(), "frags") &
            assertEquals(0, player.getTimeDied().time_since_epoch().count(), "time died") &
            assertEquals(0, player.getWeaponManager().getTimeLastWeaponSwitch().time_since_epoch().count(), "time last wpn switch") &
            assertTrue(player.canFall(), "can fall") &
            assertTrue(player.isFalling(), "is falling") &
            assertTrue(player.getHasJustStartedFallingNaturallyInThisTick(), "getHasJustStartedFallingNaturallyInThisTick") &
            assertFalse(player.getHasJustStartedFallingAfterJumpingStoppedInThisTick(), "getHasJustStartedFallingAfterJumpingStoppedInThisTick") &
            assertEquals(0, player.getTimeStartedFalling().time_since_epoch().count(), "time started falling") &
            assertEquals(0.f, player.getHeightStartedFalling(), "height started falling") &
            assertFalse(player.getHasJustStoppedJumpingInThisTick(), "getHasJustStoppedJumpingInThisTick") &
            // TODO: maybe we should also check for object height
            assertFalse(player.getCrouchStateCurrent(), "getCrouchStateCurrent") &
            assertTrue(player.getWantToStandup(), "getWantToStandup") &
            assertFalse(player.jumpAllowed(), "can jump") &
            assertFalse(player.isJumping(), "jumping") &
            assertFalse(player.getWillJumpInNextTick(), "will jump") &
            assertEquals(0, player.getTimeLastSetWillJump().time_since_epoch().count(), "time last setwilljump") &
            assertFalse(player.isSomersaulting(), "isSomersaulting") &
            assertEquals(0.f, player.getSomersaultAngle(), "getSomersaultAngle") &
            assertTrue(player.isExpectingStartPos(), "expecting start pos") &
            assertTrue(player.isRunning(), "running default") &
            assertEquals(0, player.getTimeLastToggleRun().time_since_epoch().count(), "time last run toggle") &
            assertEquals(proofps_dd::Strafe::NONE, player.getStrafe(), "strafe") &
            assertFalse(player.getAttack(), "attack") &
            assertFalse(player.getRespawnFlag(), "respawn flag") &
            assertEquals(PureVector(), player.getJumpForce(), "jump force") &
            assertEquals(PureVector(), player.getImpactForce(), "impact force") &
            assertFalse(player.getWeaponAngle().isDirty(), "old wpn angle") &
            assertEquals(PureVector(), player.getWeaponAngle(), "wpn angle") &
            assertFalse(player.getPos().isDirty(), "old pos") &
            assertEquals(PureVector(), player.getPos(), "pos") &
            assertFalse(player.getAngleY().isDirty(), "old angle y") &
            assertEquals(0.f, player.getAngleY(), "angle y") &
            assertEquals(0.f, player.getGravity(), "gravity") &
            assertFalse(player.getCrouchInput().isDirty(), "old crouch") &
            assertFalse(player.getCrouchInput(), "crouch") &
            assertNull(player.getWeaponManager().getCurrentWeapon(), "current weapon") &
            assertTrue(player.getWeaponManager().getWeapons().empty(), "weapons") /* weapons need to be manually loaded and added, maybe this change in future */) != 0;
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

        bool b = (assertFalse(player.isDirty(), "dirty 1") &
            assertFalse(player.isNetDirty(), "net dirty 1")) != 0;

        player.setHealth(5);
        b &= assertTrue(player.isDirty(), "dirty A 1");
        b &= assertFalse(player.isNetDirty(), "net dirty A 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty A 2");
        b &= assertTrue(player.isNetDirty(), "net dirty A 2");
        player.clearNetDirty();
        b &= assertFalse(player.isNetDirty(), "net dirty A 3");

        player.getFrags().set(5);
        b &= assertTrue(player.isDirty(), "dirty B 1");
        b &= assertFalse(player.isNetDirty(), "net dirty B 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty B 2");
        b &= assertTrue(player.isNetDirty(), "net dirty B 2");
        player.clearNetDirty();
        b &= assertFalse(player.isNetDirty(), "net dirty B 3");

        player.getDeaths().set(5);
        b &= assertTrue(player.isDirty(), "dirty C 1");
        b &= assertFalse(player.isNetDirty(), "net dirty C 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty C 2");
        b &= assertTrue(player.isNetDirty(), "net dirty C 2");
        player.clearNetDirty();
        b &= assertFalse(player.isNetDirty(), "net dirty C 3");

        player.getAngleY().set(5.f);
        b &= assertTrue(player.isDirty(), "dirty D 1");
        b &= assertFalse(player.isNetDirty(), "net dirty D 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty D 2");
        b &= assertTrue(player.isNetDirty(), "net dirty D 2");
        player.clearNetDirty();
        b &= assertFalse(player.isNetDirty(), "net dirty D 3");

        player.getWeaponAngle().set(PureVector(5.f, 6.f, 7.f));
        b &= assertTrue(player.isDirty(), "dirty E 1");
        b &= assertFalse(player.isNetDirty(), "net dirty E 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty E 2");
        b &= assertTrue(player.isNetDirty(), "net dirty E 2");
        player.clearNetDirty();
        b &= assertFalse(player.isNetDirty(), "net dirty E 3");

        player.getPos().set(PureVector(5.f, 6.f, 7.f));
        b &= assertTrue(player.isDirty(), "dirty F 1");
        b &= assertFalse(player.isNetDirty(), "net dirty F 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty F 2");
        b &= assertTrue(player.isNetDirty(), "net dirty F 2");
        player.clearNetDirty();
        b &= assertFalse(player.isNetDirty(), "net dirty F 3");

        player.getCrouchInput().set(true);
        b &= assertTrue(player.isDirty(), "dirty G 1");
        b &= assertFalse(player.isNetDirty(), "net dirty G 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty G 2");
        b &= assertTrue(player.isNetDirty(), "net dirty G 2");
        player.clearNetDirty();
        b &= assertFalse(player.isNetDirty(), "net dirty G 3");

        return b;
    }

    bool test_update_old_frags_and_deaths()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.getFrags() = 2;
        player.getDeaths() = 1;

        bool b = (assertEquals(0, player.getDeaths().getOld(), "old deaths 1") &
            assertEquals(1, player.getDeaths(), "deaths 1") &
            assertEquals(0, player.getFrags().getOld(), "old frags 1") &
            assertEquals(2, player.getFrags(), "frags 1") &
            assertTrue(player.isDirty(), "dirty 1")) != 0;

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

        player.setExpectingStartPos(false);
        bool b = assertFalse(player.isExpectingStartPos(), "exp 1");
        
        player.setExpectingStartPos(true);
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

        bool b = (assertEquals(PureVector(), player.getWeaponAngle().getOld(), "old wpn angle 1") &
            assertEquals(vecAngleWpnOriginal, player.getWeaponAngle(), "wpn angle 1")&
            assertEquals(PureVector(), player.getPos().getOld(), "old pos 1") &
            assertEquals(vecPosOriginal, player.getPos(), "pos 1") &
            assertEquals(0.f, player.getAngleY().getOld(), "old angle y 1") &
            assertEquals(fAngleYOriginal, player.getAngleY(), "angle y 1") &
            assertTrue(player.isDirty(), "dirty 1")) != 0;

        player.updateOldValues();
        b &= assertTrue(player.isNetDirty(), "is net dirty");

        b &= (assertEquals(vecAngleWpnOriginal, player.getWeaponAngle().getOld(), "old wpn angle 2") &
            assertEquals(vecAngleWpnOriginal, player.getWeaponAngle(), "wpn angle 2") &
            assertEquals(vecPosOriginal, player.getPos().getOld(), "old pos 2") &
            assertEquals(vecPosOriginal, player.getPos(), "pos 2") &
            assertEquals(fAngleYOriginal, player.getAngleY().getOld(), "old angle y 2") &
            assertEquals(fAngleYOriginal, player.getAngleY(), "angle y 2") &
            assertFalse(player.isDirty(), "dirty 2")) != 0;

        return b;
    }

    bool test_set_health_and_update_old_health()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        const auto& playerConst = player;

        player.setHealth(200);
        bool b = (assertEquals(100, playerConst.getHealth(), "health 1") &
            assertFalse(playerConst.getHealth().isDirty(), "dirty 1") &
            assertFalse(playerConst.isDirty(), "dirty 2")) != 0;

        player.setHealth(-1);
        b &= (assertEquals(0, playerConst.getHealth(), "health 2") &
            assertTrue(playerConst.getHealth().isDirty(), "dirty 3") &
            assertTrue(playerConst.isDirty(), "dirty 4")) != 0;

        player.updateOldValues();
        b &= assertTrue(playerConst.isNetDirty(), "is net dirty");
        b &= (assertEquals(0, playerConst.getHealth().getOld(), "health 3") &
            assertFalse(playerConst.getHealth().isDirty(), "dirty 5") &
            assertFalse(playerConst.isDirty(), "dirty 6")) != 0;

        return b;
    }

    bool test_do_damage()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        const auto& playerConst = player;

        player.doDamage(25);
        bool b = (assertEquals(75, playerConst.getHealth(), "health 1") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 1")) != 0;

        player.doDamage(25);
        b &= (assertEquals(50, playerConst.getHealth(), "health 2") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 2")) != 0;

        player.doDamage(100);
        b &= (assertEquals(0, playerConst.getHealth(), "health 3") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 3")) != 0;

        return b;
    }

    bool test_die_server()
    {
        const bool bServer = true;
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        const auto& playerConst = player;
        m_cfgProfiles.getVars()[proofps_dd::Player::CVAR_SV_SOMERSAULT_MID_AIR_AUTO_CROUCH].Set(true);
        player.setJumpAllowed(true);
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };

        player.getAttack() = true;
        player.getJumpForce().Set(1,2,3);
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        player.jump();
        player.startSomersault();
        bool b = assertTrue(player.isSomersaulting(), "somersaulting 0");
        player.die(true, bServer);
        const auto nFirstTimeDiedSinceEpoch = playerConst.getTimeDied().time_since_epoch().count();
        b &= (assertEquals(0, playerConst.getHealth(), "health 1") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 1") &
            assertFalse(player.getAttack(), "attack 1") &
            assertEquals(PureVector(), player.getJumpForce(), "jumpforce 1") &
            assertEquals(proofps_dd::Strafe::NONE, playerConst.getStrafe(), "strafe 1") &
            assertFalse(playerConst.isSomersaulting(), "somersaulting 1") &
            assertNotEquals(0, nFirstTimeDiedSinceEpoch, "time died a 1") &
            assertFalse(playerConst.getObject3D()->isRenderingAllowed(), "player object visible 1") &
            assertFalse(playerConst.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 1") &
            assertEquals(1, playerConst.getDeaths(), "deaths 1") /* server increases it */ &
            assertEquals(0, playerConst.getDeaths().getOld(), "old deaths 1")) != 0;
        
        player.setHealth(100);
        player.respawn(true, *(player.getWeaponManager().getWeapons()[0]), bServer);
        
        player.die(true, bServer);
        const auto nSecondTimeDiedSinceEpoch = playerConst.getTimeDied().time_since_epoch().count();
        b &= assertEquals(0, playerConst.getHealth(), "health 2") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 2") &
            assertNotEquals(0, nSecondTimeDiedSinceEpoch, "time died a 2") &
            assertNotEquals(nFirstTimeDiedSinceEpoch, nSecondTimeDiedSinceEpoch, "time died b 2") &
            assertEquals(2, playerConst.getDeaths(), "deaths 2") /* server increases it */ &
            assertEquals(0, playerConst.getDeaths().getOld(), "old deaths 2");

        return b;
    }

    bool test_die_client()
    {
        const bool bServer = false;
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        const auto& playerConst = player;
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };

        player.die(true, bServer);
        const auto nFirstTimeDiedSinceEpoch = player.getTimeDied().time_since_epoch().count();
        bool b = (assertEquals(0, playerConst.getHealth(), "health 1") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 1") &
            assertNotEquals(0, nFirstTimeDiedSinceEpoch, "time died 1") &
            assertFalse(playerConst.getObject3D()->isRenderingAllowed(), "player object visible 1") &
            assertFalse(playerConst.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 1") &
            assertEquals(0, playerConst.getDeaths(), "deaths 1") /* client doesn't change it, will receive update from server */ &
            assertEquals(0, playerConst.getDeaths().getOld(), "old deaths 1")) != 0;

        player.setHealth(100);
        player.respawn(true, *(player.getWeaponManager().getWeapons()[0]), bServer);

        player.die(true, bServer);
        const auto nSecondTimeDiedSinceEpoch = playerConst.getTimeDied().time_since_epoch().count();
        b &= assertEquals(0, playerConst.getHealth(), "health 2") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 2") &
            assertNotEquals(0, nSecondTimeDiedSinceEpoch, "time died a 2") &
            assertNotEquals(nFirstTimeDiedSinceEpoch, nSecondTimeDiedSinceEpoch, "time died b 2") &
            assertFalse(playerConst.getObject3D()->isRenderingAllowed(), "player object visible 2") &
            assertFalse(playerConst.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 2") &
            assertEquals(0, playerConst.getDeaths(), "deaths 2") /* client doesn't change it, will receive update from server */ &
            assertEquals(0, playerConst.getDeaths().getOld(), "old deaths 2");

        return b;
    }

    bool test_respawn()
    {
        const bool bServer = true;
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };
        player.getWeaponManager().getWeapons()[1]->SetAvailable(true);
        bool b = assertTrue(player.getWeaponManager().setCurrentWeapon(player.getWeaponManager().getWeapons()[1], false, bServer), "set current wpn");

        // TODO: maybe we should also check for object height
        player.getWantToStandup() = false;
        // "# Since a player can spawn only in standing position, placing a spawnpoint to a low-height
        //  # tunnel where only crouching would be available is considered as map design error."
        player.getCrouchStateCurrent() = true;  // respawn event is allowed to set it to false
        player.getImpactForce().Set(1,2,3);

        player.die(true, bServer);
        player.respawn(true, *(player.getWeaponManager().getWeapons()[0]), bServer);

        return (b & assertTrue(player.getObject3D()->isRenderingAllowed(), "player object visible") &
            assertTrue(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible") &
            assertFalse(player.getWeaponManager().getWeapons()[1]->isAvailable(), "wpn 2 not available") &
            assertEquals(player.getWeaponManager().getWeapons()[0], player.getWeaponManager().getCurrentWeapon(), "current wpn") &
            assertFalse(player.getCrouchStateCurrent(), "getCrouchStateCurrent") &
            assertTrue(player.getWantToStandup(), "wantstandup") &
            assertEquals(PureVector(), player.getImpactForce(), "impact force")) != 0;
    }

    bool test_jump()
    {
        bool b = true;
        // use loop with 2 cycles for 2 different starting state: standing and crouching.
        for (int i = 0; i < 2; i++)
        {
            proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

            player.getPos().set(PureVector(1.f, 2.f, 3.f));
            player.getPos().commit();
            player.getPos().set(PureVector(2.f, 4.f, 8.f));

            // we modify only the X-component of jumpForce in jump(), since other components are not used at all!
            //const PureVector vecExpectedForce = player.getPos().getNew() - player.getPos().getOld();
            const PureVector vecExpectedForce(player.getPos().getNew().getX() - player.getPos().getOld().getX(), 0.f, 0.f);

            player.getCrouchInput().set(i == 1);
            const float fExpectedInitialGravity = player.getCrouchInput() ? proofps_dd::GAME_JUMP_GRAVITY_START_FROM_CROUCHING : proofps_dd::GAME_JUMP_GRAVITY_START_FROM_STANDING;
            player.getCrouchStateCurrent() = true;  // jumping is not changing crouching state since crouching is also valid mid-air

            // TODO: maybe we should also check for object height
            player.setJumpAllowed(true);
            b &= assertTrue(player.jumpAllowed(), "allowed 1") &
                assertFalse(player.isJumping(), "jumping 1") &
                assertEquals(0.f, player.getGravity(), "gravity 1") &
                assertEquals(PureVector(), player.getJumpForce(), "jump force 1") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent 1") &
                assertTrue(player.getWantToStandup(), "wantstandup 1");

            const std::chrono::time_point<std::chrono::steady_clock> timeBeforeSetWillJump = std::chrono::steady_clock::now();
            player.setWillJumpInNextTick(true);
            const std::chrono::time_point<std::chrono::steady_clock> timeLastSetWillJump = player.getTimeLastSetWillJump();
            b &= assertTrue(timeLastSetWillJump > timeBeforeSetWillJump, "cmp timeBefore") &
                assertTrue(timeLastSetWillJump < std::chrono::steady_clock::now(), "cmp timeAfter");
            b &= assertTrue(player.getWillJumpInNextTick(), "will jump 1");
            player.jump();
            b &= assertFalse(player.jumpAllowed(), "allowed 2") &
                assertTrue(player.isJumping(), "jumping 2") &
                assertEquals(fExpectedInitialGravity, player.getGravity(), (std::string("gravity 2 crouch: ") + std::to_string(player.getCrouchInput())).c_str()) &
                assertEquals(vecExpectedForce, player.getJumpForce(), "jump force 2") &
                assertFalse(player.getWillJumpInNextTick(), "will jump 2") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent 2") &
                assertTrue(player.getWantToStandup(), "wantstandup 2");

            player.stopJumping();
            b &= assertFalse(player.jumpAllowed(), "allowed 3") &
                assertFalse(player.isJumping(), "jumping 3") &
                assertEquals(fExpectedInitialGravity, player.getGravity(), (std::string("gravity 3 crouch: ") + std::to_string(player.getCrouchInput())).c_str()) &
                assertEquals(vecExpectedForce, player.getJumpForce(), "jump force 3") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent 3") &
                assertTrue(player.getWantToStandup(), "wantstandup 3");

            player.setJumpAllowed(false);
            player.setWillJumpInNextTick(true);
            b &= assertFalse(player.getWillJumpInNextTick(), "will jump 3") &
                assertTrue(player.getTimeLastSetWillJump() == timeLastSetWillJump, "time last setwilljump unchanged") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent 4") &
                assertTrue(player.getWantToStandup(), "wantstandup 4");

            player.jump();
            b &= assertFalse(player.jumpAllowed(), "allowed 4") &
                assertFalse(player.isJumping(), "jumping 4") &
                assertEquals(fExpectedInitialGravity, player.getGravity(), (std::string("gravity 4 crouch: ") + std::to_string(player.getCrouchInput())).c_str()) &
                assertEquals(vecExpectedForce, player.getJumpForce(), "jump force 4") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent 5") &
                assertTrue(player.getWantToStandup(), "wantstandup 5");

        } // end for i

        return b;
    }

    bool test_set_can_fall()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.setCanFall(false);
        bool b = assertFalse(player.canFall(), "can fall 1");

        player.setCanFall(true);
        b &= assertTrue(player.canFall(), "can fall 2");

        return b;
    }

    bool test_gravity()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        bool b = assertTrue(player.isFalling(), "is falling 1");
        
        player.setGravity(-1.f);
        b &= assertTrue(player.isFalling(), "is falling 2");
        
        player.setGravity(0.f);
        b &= assertFalse(player.isFalling(), "is falling 3");

        player.setGravity(5.f);

        return (b & assertEquals(5.f, player.getGravity())) != 0;
    }

    bool test_set_has_just_started_falling()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        
        player.getPos().set(
            PureVector(0.f, 5.f, 0.f)
        );
        player.setGravity(0.f);  // way to set isFalling to false
        bool b = assertFalse(player.isFalling(), "is falling 1");
        const auto timeBeforeJustStartedFallingAfterJumpingStopped = std::chrono::steady_clock::now();
        player.setHasJustStartedFallingAfterJumpingStoppedInThisTick(true);
        b &= assertTrue(player.isFalling(), "is falling 2");
        b &= assertTrue(timeBeforeJustStartedFallingAfterJumpingStopped <= player.getTimeStartedFalling(), "time started falling after jumping stopped 1");
        b &= assertTrue(player.getTimeStartedFalling() <= std::chrono::steady_clock::now(), "time started falling after jumping stopped 1");

        player.getPos().set(
            PureVector(0.f, 1.f, 0.f)
        );
        b &= assertEquals(5.f, player.getHeightStartedFalling(), 0.001f, "height falling 1");

        player.setGravity(0.f);  // way to set isFalling to false
        b &= assertFalse(player.isFalling(), "is falling 3");
        const auto timeBeforeJustStartedFallingNaturally = std::chrono::steady_clock::now();
        player.setHasJustStartedFallingNaturallyInThisTick(true);
        b &= assertTrue(player.isFalling(), "is falling 4");
        b &= assertTrue(timeBeforeJustStartedFallingNaturally <= player.getTimeStartedFalling(), "time started falling naturally 1");
        b &= assertTrue(player.getTimeStartedFalling() <= std::chrono::steady_clock::now(), "time started falling naturally 2");

        player.getPos().set(
            PureVector(0.f, 0.f, 0.f)
        );
        b &= assertEquals(1.f, player.getHeightStartedFalling(), 0.001f, "height falling 2");

        return b;
    }

    bool test_start_somersault_when_auto_crouch_is_disabled()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        m_cfgProfiles.getVars()[proofps_dd::Player::CVAR_SV_SOMERSAULT_MID_AIR_AUTO_CROUCH].Set(false);
        player.setJumpAllowed(true);
        player.getPos().set(PureVector()); // old
        player.getPos().commit();
        player.getPos().set(PureVector(1.0, 1.f, 1.f)); // new
        player.setGravity(5.f);

        player.startSomersault();
        bool b = assertFalse(player.isSomersaulting(), "cannot somersault on ground");
        b &= assertEquals(PureVector(), player.getJumpForce(), "jumpforce 1");
        b &= assertEquals(5.f, player.getGravity(), "gravity 1");

        player.getCrouchInput() = true; // at the moment of triggering jump (still being on the ground), we are pressing crouch
        player.setGravity(5.f);
        player.jump();
        auto vecOriginalJumpForce = player.getJumpForce();
        auto fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = true; // and we are still crouching
        player.startSomersault();
        b &= assertFalse(player.isSomersaulting(), "cannot somersault from crouch-jumping from ground");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 2");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 2");

        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = false; // and we are still NOT crouching
        player.startSomersault();
        b &= assertFalse(player.isSomersaulting(), "cannot somersault without manual crouch after jump if auto crouch is disabled");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 3");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 3");

        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = true; // in the meantime we have started crouching AFTER triggering jump
        player.startSomersault();
        b &= assertTrue(player.isSomersaulting(), "finally true 1");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(0.1f, player.getSomersaultAngle(), "angle 1 when strafe is NONE");
        b &= assertEquals(vecOriginalJumpForce * 2, player.getJumpForce(), "jumpforce 4");
        b &= assertEquals(fOriginalGravity * 2, player.getGravity(), "gravity 4");

        // same as previous but we change Strafe
        player.resetSomersault();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = true; // in the meantime we have started crouching AFTER triggering jump
        player.startSomersault();
        b &= assertTrue(player.isSomersaulting(), "finally true 2");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(-0.1f, player.getSomersaultAngle(), "angle 2 when strafe is RIGHT");
        b &= assertEquals(vecOriginalJumpForce * 2, player.getJumpForce(), "jumpforce 5");
        b &= assertEquals(fOriginalGravity * 2, player.getGravity(), "gravity 5");

        // same as previous but we change Strafe
        player.resetSomersault();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setStrafe(proofps_dd::Strafe::LEFT);
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = true; // in the meantime we have started crouching AFTER triggering jump
        player.startSomersault();
        b &= assertTrue(player.isSomersaulting(), "finally true 3");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(0.1f, player.getSomersaultAngle(), "angle 3 when strafe is LEFT");
        b &= assertEquals(vecOriginalJumpForce * 2, player.getJumpForce(), "jumpforce 6");
        b &= assertEquals(fOriginalGravity * 2, player.getGravity(), "gravity 6");

        return b;
    }

    bool test_start_somersault_when_auto_crouch_is_enabled()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        m_cfgProfiles.getVars()[proofps_dd::Player::CVAR_SV_SOMERSAULT_MID_AIR_AUTO_CROUCH].Set(true);
        player.setJumpAllowed(true);
        player.getPos().set(PureVector()); // old
        player.getPos().commit();
        player.getPos().set(PureVector(1.0, 1.f, 1.f)); // new
        player.setGravity(5.f);

        player.startSomersault();
        bool b = assertFalse(player.isSomersaulting(), "cannot somersault on ground");
        b &= assertEquals(PureVector(), player.getJumpForce(), "jumpforce 1");
        b &= assertEquals(5.f, player.getGravity(), "gravity 1");

        player.getCrouchInput() = true; // at the moment of triggering jump (still being on the ground), we are pressing crouch
        player.setGravity(5.f);
        player.jump();
        auto vecOriginalJumpForce = player.getJumpForce();
        auto fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = true; // and we are still crouching (although not needed since auto-crouch is enabled)
        player.startSomersault();
        b &= assertFalse(player.isSomersaulting(), "cannot somersault from crouch-jumping from ground 1");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 2");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 2");

        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = true; // at the moment of triggering jump (still being on the ground), we are pressing crouch
        player.setGravity(5.f);
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = false; // but we are NOT crouching anymore (although not needed since auto-crouch is enabled)
        player.startSomersault();
        b &= assertFalse(player.isSomersaulting(), "cannot somersault from crouch-jumping from ground 2");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 3");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 3");

        player.resetSomersault();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = true; // in the meantime we have started crouching AFTER triggering jump (although not needed since auto-crouch is enabled)
        player.startSomersault();
        b &= assertTrue(player.isSomersaulting(), "finally true 1");
        b &= assertEquals(vecOriginalJumpForce * 2, player.getJumpForce(), "jumpforce 4");
        b &= assertEquals(fOriginalGravity * 2, player.getGravity(), "gravity 4");

        player.resetSomersault();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = false; // and we are still NOT crouching (not needed since auto-crouch is enabled)
        player.startSomersault();
        b &= assertTrue(player.isSomersaulting(), "finally true 2");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(0.1f, player.getSomersaultAngle(), "angle 1 when strafe is NONE");
        b &= assertEquals(vecOriginalJumpForce * 2, player.getJumpForce(), "jumpforce 5");
        b &= assertEquals(fOriginalGravity * 2, player.getGravity(), "gravity 5");

        // same as previous but we change Strafe
        player.resetSomersault();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = false; // and we are still NOT crouching (not needed since auto-crouch is enabled)
        player.startSomersault();
        b &= assertTrue(player.isSomersaulting(), "finally true 3");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(-0.1f, player.getSomersaultAngle(), "angle 2 when strafe is RIGHT");
        b &= assertEquals(vecOriginalJumpForce * 2, player.getJumpForce(), "jumpforce 6");
        b &= assertEquals(fOriginalGravity * 2, player.getGravity(), "gravity 6");

        // same as previous but we change Strafe
        player.resetSomersault();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setStrafe(proofps_dd::Strafe::LEFT);
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        player.getCrouchStateCurrent() = false; // and we are still NOT crouching (not needed since auto-crouch is enabled)
        player.startSomersault();
        b &= assertTrue(player.isSomersaulting(), "finally true 4");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(0.1f, player.getSomersaultAngle(), "angle 3 when strafe is LEFT");
        b &= assertEquals(vecOriginalJumpForce * 2, player.getJumpForce(), "jumpforce 7");
        b &= assertEquals(fOriginalGravity * 2, player.getGravity(), "gravity 7");

        return b;
    }

    bool test_step_somersault_angle()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        m_cfgProfiles.getVars()[proofps_dd::Player::CVAR_SV_SOMERSAULT_MID_AIR_AUTO_CROUCH].Set(true);
        player.setJumpAllowed(true);

        // Strafe is NONE
        player.jump();
        player.startSomersault();
        bool b = assertTrue(player.isSomersaulting(), "somersaulting 1");

        player.stepSomersaultAngle(30.f);  // step amount is input from physics based on physics rate, here we just pass something
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(30.1f, player.getSomersaultAngle(), "angle 1 when strafe is NONE");
        b &= assertTrue(player.isSomersaulting(), "somersaulting 2");
        player.stepSomersaultAngle(400.f);
        b &= assertEquals(0.f, player.getSomersaultAngle(), "angle 2 when strafe is NONE");
        b &= assertFalse(player.isSomersaulting(), "somersaulting 3");

        // Strafe is RIGHT
        player.resetSomersault();
        player.setJumpAllowed(true); // allow jump again
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        player.jump();
        player.startSomersault();
        b &= assertTrue(player.isSomersaulting(), "somersaulting 4");

        player.stepSomersaultAngle(30.f);  // step amount is input from physics based on physics rate, here we just pass something
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(-30.1f, player.getSomersaultAngle(), "angle 1 when strafe is RIGHT");
        b &= assertTrue(player.isSomersaulting(), "somersaulting 5");
        player.stepSomersaultAngle(400.f);
        b &= assertEquals(0.f, player.getSomersaultAngle(), "angle 2 when strafe is RIGHT");
        b &= assertFalse(player.isSomersaulting(), "somersaulting 6");

        // Strafe is LEFT
        player.resetSomersault();
        player.setJumpAllowed(true); // allow jump again
        player.setStrafe(proofps_dd::Strafe::LEFT);
        player.jump();
        player.startSomersault();
        b &= assertTrue(player.isSomersaulting(), "somersaulting 7");

        player.stepSomersaultAngle(30.f);  // step amount is input from physics based on physics rate, here we just pass something
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(30.1f, player.getSomersaultAngle(), "angle 1 when strafe is LEFT");
        b &= assertTrue(player.isSomersaulting(), "somersaulting 8");
        player.stepSomersaultAngle(400.f);
        b &= assertEquals(0.f, player.getSomersaultAngle(), "angle 2 when strafe is LEFT");
        b &= assertFalse(player.isSomersaulting(), "somersaulting 9");

        return b;
    }

    bool test_set_run()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        const std::chrono::time_point<std::chrono::steady_clock> timeBeforeToggleRun = std::chrono::steady_clock::now();
        player.setRun(false);

        bool b = (assertTrue(player.getTimeLastToggleRun() >= timeBeforeToggleRun, "cmp timeBefore") &
            assertTrue(player.getTimeLastToggleRun() <= std::chrono::steady_clock::now(), "cmp timeAfter")) != 0;
        
        return (b & assertFalse(player.isRunning(), "running")) != 0;
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

    bool test_server_do_crouch()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        const auto vecOriginalPos = player.getPos();
        player.doCrouchServer(false);
        bool b = (assertEquals(vecOriginalPos, player.getPos(), "pos vec 1") &
            assertNotEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 1") &
            assertTrue(player.getCrouchStateCurrent(), "crouch current state 1")) != 0;

        player.doStandupServer(player.getPos().getNew().getY());

        player.doCrouchServer(true);
        b &= assertNotEquals(vecOriginalPos, player.getPos(), "pos vec 2") &
            assertNotEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 2") &
            assertTrue(player.getCrouchStateCurrent(), "crouch current state 2");

        return b;
    }

    bool test_client_do_crouch()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        const auto pOrigTex = player.getObject3D()->getMaterial().getTexture();
        player.doCrouchShared();
        bool b = (assertNotEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 1") &
            assertTrue(player.getCrouchStateCurrent(), "crouch current state 1") &
            assertNotEquals(pOrigTex, player.getObject3D()->getMaterial().getTexture(), "texture 1") &
            assertTrue(player.getWantToStandup(), "want standup intact 1")) != 0;

        return b;
    }

    bool test_server_do_standup()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.doCrouchServer(false);

        player.doStandupServer(12.f);
        bool b = (assertEquals(12.f, player.getPos().getNew().getY(), "pos vec 1") &
            assertEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 1") &
            assertFalse(player.getCrouchStateCurrent(), "crouch current state 1")) != 0;

        return b;
    }

    bool test_client_do_standup()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.doCrouchShared();
        const auto pOrigTex = player.getObject3D()->getMaterial().getTexture();

        player.doStandupShared();
        bool b = (assertEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 1") &
            assertFalse(player.getCrouchStateCurrent(), "crouch current state 1") &
            assertNotEquals(pOrigTex, player.getObject3D()->getMaterial().getTexture(), "texture 1") &
            assertTrue(player.getWantToStandup(), "want standup intact 1")) != 0;

        return b;
    }

    bool test_attack()
    {
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::No)))
        {
            return false;
        };

        bool b = assertFalse(player.attack(), "player cannot attack by default due to not having attack state");

        player.getAttack() = true;
        b &= assertFalse(player.attack(), "player cannot attack without wpn");
        
        if (!player.getWeaponManager().setCurrentWeapon(player.getWeaponManager().getWeapons()[0], false, true))
        {
            return false;
        }
        const TPureUInt nOriginalBulletCount = player.getWeaponManager().getCurrentWeapon()->getMagBulletCount();

        player.getAttack() = false; /* dbl check if first false was really because of state and not because of missing wpn */
        b &= assertFalse(player.attack(), "player cannot attack due to not having attack state");
        b &= assertEquals(nOriginalBulletCount, player.getWeaponManager().getCurrentWeapon()->getMagBulletCount(), "no change in bullet count 1");

        player.getAttack() = true;
        b &= assertTrue(player.attack(), "player should fire wpn");
        b &= assertGreater(nOriginalBulletCount, player.getWeaponManager().getCurrentWeapon()->getMagBulletCount(), "bullet count changed 1");

        // wait for wpn to go back to ready state
        player.getWeaponManager().getCurrentWeapon()->releaseTrigger();
        Sleep(player.getWeaponManager().getCurrentWeapon()->getVars()["firing_cooldown"].getAsInt());
        player.getWeaponManager().getCurrentWeapon()->update();
        if (!assertEquals(Weapon::State::WPN_READY, player.getWeaponManager().getCurrentWeapon()->getState()))
        {   // make sure wpn is ready again
            return false;
        }

        const TPureUInt nNewBulletCountAfterAttack = player.getWeaponManager().getCurrentWeapon()->getMagBulletCount();
        if (!assertLess(0u, nNewBulletCountAfterAttack))
        {   // make sure we still have loaded bullet to shoot
            return false;
        }

        player.setHealth(0);
        b &= assertFalse(player.attack(), "dead player cannot attack");
        b &= assertEquals(nNewBulletCountAfterAttack, player.getWeaponManager().getCurrentWeapon()->getMagBulletCount(), "no change in bullet count 2");

        player.setHealth(100);
        player.getWeaponManager().getCurrentWeapon()->SetMagBulletCount(0);
        b &= assertFalse(player.attack(), "should return wpn->pullTrigger() which is false in this case");

        return b;
    }

    bool test_can_take_item_health()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        const proofps_dd::MapItem miHealth(*engine, proofps_dd::MapItemType::ITEM_HEALTH, PureVector(1, 2, 3));

        bool b = assertFalse(player.canTakeItem(miHealth), "1");

        player.setHealth(50);
        b &= assertTrue(player.canTakeItem(miHealth), "2");

        return b;
    }

    bool test_can_take_item_weapon()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        const proofps_dd::MapItem miPistol(*engine, proofps_dd::MapItemType::ITEM_WPN_PISTOL, PureVector(1, 2, 3));
        const proofps_dd::MapItem miMchgun(*engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        const proofps_dd::MapItem miBazooka(*engine, proofps_dd::MapItemType::ITEM_WPN_BAZOOKA, PureVector(1, 2, 3));
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };

        bool b = assertTrue(player.canTakeItem(miPistol), "1");

        player.getWeaponManager().getWeapons()[0]->SetUnmagBulletCount(player.getWeaponManager().getWeapons()[0]->getVars()["cap_max"].getAsInt());
        b &= assertFalse(player.canTakeItem(miPistol), "2");

        b &= assertTrue(player.canTakeItem(miMchgun), "3");
        b &= assertTrue(player.canTakeItem(miBazooka), "4");

        return b;
    }

    bool test_take_item_health()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        const auto& playerConst = player;
        proofps_dd::MapItem miHealth(*engine, proofps_dd::MapItemType::ITEM_HEALTH, PureVector(1, 2, 3));
        pge_network::PgePacket newPktWpnUpdate;

        player.setHealth(90);
        player.takeItem(miHealth, newPktWpnUpdate);

        return (assertEquals(100, playerConst.getHealth(), "player health") &
            assertTrue(miHealth.isTaken(), "item taken")) != 0;
    }

    bool test_take_item_weapon()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_cfgProfiles, m_bullets, *engine, connHandleExpected, "192.168.1.12");
        proofps_dd::MapItem miPistol(*engine, proofps_dd::MapItemType::ITEM_WPN_PISTOL, PureVector(1, 2, 3));
        proofps_dd::MapItem miMchGun(*engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        proofps_dd::MapItem miBazooka(*engine, proofps_dd::MapItemType::ITEM_WPN_BAZOOKA, PureVector(1, 2, 3));
        pge_network::PgePacket pktWpnUpdate;
        
        // Warning: this way of pointing to message is valid only if there is only 1 message (the first) in the packet and we want that!
        const proofps_dd::MsgWpnUpdateFromServer& msgWpnUpdate = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgWpnUpdateFromServer>(pktWpnUpdate);
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };

        player.takeItem(miPistol, pktWpnUpdate);
        bool bStrSafeChecked = strncmp(
            player.getWeaponManager().getWeapons()[0]->getFilename().c_str(),
            msgWpnUpdate.m_szWpnName,
            proofps_dd::MsgWpnUpdateFromServer::nWpnNameNameMaxLength) == 0;

        bool b = (assertEquals(player.getWeaponManager().getWeapons()[0]->getVars()["reloadable"].getAsInt(),
            static_cast<int>(player.getWeaponManager().getWeapons()[0]->getUnmagBulletCount()), "wpn 1 unmag") &
            assertTrue(miPistol.isTaken(), "item 1 taken") &
            assertEquals(static_cast<uint32_t>(pge_network::MsgApp::id),
                static_cast<uint32_t>(pge_network::PgePacket::getPacketId(pktWpnUpdate)),
                "pkt 1 id") &
            assertEquals(static_cast<uint32_t>(proofps_dd::MsgWpnUpdateFromServer::id),
                static_cast<uint32_t>(pge_network::PgePacket::getMsgAppIdFromPkt(pktWpnUpdate)),
                "msg 1 id") &
            assertTrue(bStrSafeChecked, "msg 1 sWeaponBecomingAvailable") &
            assertTrue(msgWpnUpdate.m_bAvailable, "msg wpn 1 becoming available") &
            assertEquals(player.getWeaponManager().getWeapons()[0]->getVars()["reloadable"].getAsInt(),
                static_cast<int>(msgWpnUpdate.m_nMagBulletCount), "msg wpn 1 mag") &
            assertEquals(player.getWeaponManager().getWeapons()[0]->getVars()["reloadable"].getAsInt() /* we already had pistol, so we expect unmag count to be non-zero in msg */,
                static_cast<int>(msgWpnUpdate.m_nUnmagBulletCount), "msg wpn 1 unmag")) != 0;

        player.takeItem(miMchGun, pktWpnUpdate);
        bStrSafeChecked = strncmp(
            player.getWeaponManager().getWeapons()[1]->getFilename().c_str(),
            msgWpnUpdate.m_szWpnName,
            proofps_dd::MsgWpnUpdateFromServer::nWpnNameNameMaxLength) == 0;

        b &= assertEquals(player.getWeaponManager().getWeapons()[1]->getVars()["reloadable"].getAsInt(),
            static_cast<int>(player.getWeaponManager().getWeapons()[1]->getMagBulletCount()), "wpn 2 mag") &
            assertTrue(player.getWeaponManager().getWeapons()[1]->isAvailable(), "wpn 2 available") &
            assertTrue(miMchGun.isTaken(), "item 2 taken") &
            assertEquals(static_cast<uint32_t>(pge_network::MsgApp::id),
                static_cast<uint32_t>(pge_network::PgePacket::getPacketId(pktWpnUpdate)),
                "pkt 2 id") &
            assertEquals(static_cast<uint32_t>(proofps_dd::MsgWpnUpdateFromServer::id),
                static_cast<uint32_t>(pge_network::PgePacket::getMsgAppIdFromPkt(pktWpnUpdate)),
                "msg 2 id") &
            assertTrue(bStrSafeChecked, "msg 2 sWeaponBecomingAvailable") &
            assertTrue(msgWpnUpdate.m_bAvailable, "msg wpn 2 becoming available") &
            assertEquals(player.getWeaponManager().getWeapons()[1]->getVars()["reloadable"].getAsInt(),
                static_cast<int>(msgWpnUpdate.m_nMagBulletCount), "msg wpn 2 mag") &
            assertEquals(0 /* we didnt have machinegun yet, so we expect unmag count to be 0 in msg */,
                static_cast<int>(msgWpnUpdate.m_nUnmagBulletCount), "msg wpn 2 unmag");

        player.takeItem(miBazooka, pktWpnUpdate);
        bStrSafeChecked = strncmp(
            player.getWeaponManager().getWeapons()[2]->getFilename().c_str(),
            msgWpnUpdate.m_szWpnName,
            proofps_dd::MsgWpnUpdateFromServer::nWpnNameNameMaxLength) == 0;

        b &= assertEquals(player.getWeaponManager().getWeapons()[2]->getVars()["reloadable"].getAsInt(),
            static_cast<int>(player.getWeaponManager().getWeapons()[2]->getMagBulletCount()), "wpn 3 mag") &
            assertTrue(player.getWeaponManager().getWeapons()[2]->isAvailable(), "wpn 3 available") &
            assertTrue(miBazooka.isTaken(), "item 3 taken") &
            assertEquals(static_cast<uint32_t>(pge_network::MsgApp::id),
                static_cast<uint32_t>(pge_network::PgePacket::getPacketId(pktWpnUpdate)),
                "pkt 3 id") &
            assertEquals(static_cast<uint32_t>(proofps_dd::MsgWpnUpdateFromServer::id),
                static_cast<uint32_t>(pge_network::PgePacket::getMsgAppIdFromPkt(pktWpnUpdate)),
                "msg 3 id") &
            assertTrue(bStrSafeChecked, "msg 3 sWeaponBecomingAvailable") &
            assertTrue(msgWpnUpdate.m_bAvailable, "msg wpn 3 becoming available") &
            assertEquals(player.getWeaponManager().getWeapons()[2]->getVars()["reloadable"].getAsInt(),
                static_cast<int>(msgWpnUpdate.m_nMagBulletCount), "msg wpn 3 mag") &
            assertEquals(0 /* we didnt have bazooka yet, so we expect unmag count to be 0 in msg */,
                static_cast<int>(msgWpnUpdate.m_nUnmagBulletCount), "msg wpn 3 unmag");

        return b;
    }

};