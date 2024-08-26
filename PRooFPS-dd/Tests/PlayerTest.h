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
        m_audio(cfgProfiles),
        m_cfgProfiles(cfgProfiles),
        m_engine(nullptr),
        m_itemPickupEvents(8 /* time limit secs */, 5 /* event count limit */),
        m_ammoChangeEvents(8 /* time limit secs */, 5 /* event count limit */, proofps_dd::EventLister::Orientation::Horizontal),
        m_network(cfgProfiles)
    {}

    PlayerTest(const PlayerTest&) = delete;
    PlayerTest& operator=(const PlayerTest&) = delete;
    PlayerTest(PlayerTest&&) = delete;
    PlayerTest&& operator=(PlayerTest&&) = delete;

protected:

    virtual void Initialize() override
    {
        //CConsole::getConsoleInstance().SetLoggingState(proofps_dd::Player::getLoggerModuleName(), true);

        m_audio.initialize();

        PGEInputHandler& inputHandler = PGEInputHandler::createAndGet(m_cfgProfiles);

        m_engine = &PR00FsUltimateRenderingEngine::createAndGet(m_cfgProfiles, inputHandler);
        m_engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        AddSubTest("test_gen_unique_user_name", (PFNUNITSUBTEST)&PlayerTest::test_gen_unique_user_name);
        AddSubTest("test_initial_values", (PFNUNITSUBTEST)&PlayerTest::test_initial_values);
        AddSubTest("test_set_name", (PFNUNITSUBTEST)&PlayerTest::test_set_name);
        AddSubTest("test_set_booted_up", (PFNUNITSUBTEST)&PlayerTest::test_set_booted_up);
        AddSubTest("test_set_expecting_after_boot_up_delayed_update", (PFNUNITSUBTEST)&PlayerTest::test_set_expecting_after_boot_up_delayed_update);
        AddSubTest("test_show", (PFNUNITSUBTEST)&PlayerTest::test_show);
        AddSubTest("test_hide", (PFNUNITSUBTEST)&PlayerTest::test_hide);
        AddSubTest("test_set_visibility_state", (PFNUNITSUBTEST)&PlayerTest::test_set_visibility_state);
        AddSubTest("test_dirtiness_one_by_one", (PFNUNITSUBTEST)&PlayerTest::test_dirtiness_one_by_one);
        AddSubTest("test_update_old_frags_and_deaths", (PFNUNITSUBTEST)&PlayerTest::test_update_old_frags_and_deaths);
        AddSubTest("test_set_just_created_and_expecting_start_pos", (PFNUNITSUBTEST)&PlayerTest::test_set_just_created_and_expecting_start_pos);
        AddSubTest("test_update_old_pos", (PFNUNITSUBTEST)&PlayerTest::test_update_old_pos);
        AddSubTest("test_set_armor_and_update_old_armor", (PFNUNITSUBTEST)&PlayerTest::test_set_armor_and_update_old_armor);
        AddSubTest("test_set_health_and_update_old_health", (PFNUNITSUBTEST)&PlayerTest::test_set_health_and_update_old_health);
        AddSubTest("test_do_damage", (PFNUNITSUBTEST)&PlayerTest::test_do_damage);
        AddSubTest("test_die_server", (PFNUNITSUBTEST)&PlayerTest::test_die_server);
        AddSubTest("test_die_client", (PFNUNITSUBTEST)&PlayerTest::test_die_client);
        AddSubTest("test_respawn", (PFNUNITSUBTEST)&PlayerTest::test_respawn);
        AddSubTest("test_set_invulnerability", (PFNUNITSUBTEST)&PlayerTest::test_set_invulnerability);
        AddSubTest("test_jump", (PFNUNITSUBTEST)&PlayerTest::test_jump);
        AddSubTest("test_set_can_fall", (PFNUNITSUBTEST)&PlayerTest::test_set_can_fall);
        AddSubTest("test_gravity", (PFNUNITSUBTEST)&PlayerTest::test_gravity);
        AddSubTest("test_set_has_just_started_falling", (PFNUNITSUBTEST)&PlayerTest::test_set_has_just_started_falling);
        AddSubTest("test_is_in_air", (PFNUNITSUBTEST)&PlayerTest::test_is_in_air);
        AddSubTest("test_start_somersault_server_mid_air_when_auto_crouch_is_disabled", (PFNUNITSUBTEST)&PlayerTest::test_start_somersault_server_mid_air_when_auto_crouch_is_disabled);
        AddSubTest("test_start_somersault_server_mid_air_when_auto_crouch_is_enabled", (PFNUNITSUBTEST)&PlayerTest::test_start_somersault_server_mid_air_when_auto_crouch_is_enabled);
        AddSubTest("test_start_somersault_server_on_ground", (PFNUNITSUBTEST)&PlayerTest::test_start_somersault_server_on_ground);
        AddSubTest("test_step_somersault_angle_server", (PFNUNITSUBTEST)&PlayerTest::test_step_somersault_angle_server);
        AddSubTest("test_set_somersault_client", (PFNUNITSUBTEST)&PlayerTest::test_set_somersault_client);
        AddSubTest("test_set_run", (PFNUNITSUBTEST)&PlayerTest::test_set_run);
        AddSubTest("test_set_strafe", (PFNUNITSUBTEST)&PlayerTest::test_set_strafe);
        AddSubTest("test_server_do_crouch", (PFNUNITSUBTEST)&PlayerTest::test_server_do_crouch);
        AddSubTest("test_client_do_crouch", (PFNUNITSUBTEST)&PlayerTest::test_client_do_crouch);
        AddSubTest("test_get_proposed_new_pos_y_for_standup", (PFNUNITSUBTEST)&PlayerTest::test_get_proposed_new_pos_y_for_standup);
        AddSubTest("test_server_do_standup", (PFNUNITSUBTEST)&PlayerTest::test_server_do_standup);
        AddSubTest("test_client_do_standup", (PFNUNITSUBTEST)&PlayerTest::test_client_do_standup);
        AddSubTest("test_is_moving", (PFNUNITSUBTEST)&PlayerTest::test_is_moving);
        AddSubTest("test_attack", (PFNUNITSUBTEST)&PlayerTest::test_attack);
        AddSubTest("test_can_take_item_health", (PFNUNITSUBTEST)&PlayerTest::test_can_take_item_health);
        AddSubTest("test_can_take_item_weapon", (PFNUNITSUBTEST)&PlayerTest::test_can_take_item_weapon);
        AddSubTest("test_take_item_health", (PFNUNITSUBTEST)&PlayerTest::test_take_item_health);
        AddSubTest("test_take_item_weapon", (PFNUNITSUBTEST)&PlayerTest::test_take_item_weapon);
        AddSubTest("test_get_weapon_instance_by_map_item_type", (PFNUNITSUBTEST)&PlayerTest::test_get_weapon_instance_by_map_item_type);
        AddSubTest("test_handle_falling_from_high_server_instance", (PFNUNITSUBTEST)&PlayerTest::test_handle_falling_from_high_server_instance);
        AddSubTest("test_handle_falling_from_high_client_instance", (PFNUNITSUBTEST)&PlayerTest::test_handle_falling_from_high_client_instance);
        AddSubTest("test_handle_landed_server_instance", (PFNUNITSUBTEST)&PlayerTest::test_handle_landed_server_instance);
        AddSubTest("test_handle_landed_client_instance", (PFNUNITSUBTEST)&PlayerTest::test_handle_landed_client_instance);
        AddSubTest("test_handle_take_item_server_instance", (PFNUNITSUBTEST)&PlayerTest::test_handle_take_item_server_instance);
        AddSubTest("test_handle_take_item_client_instance", (PFNUNITSUBTEST)&PlayerTest::test_handle_take_item_client_instance);
        AddSubTest("test_handle_take_weapon_item", (PFNUNITSUBTEST)&PlayerTest::test_handle_take_weapon_item);
        AddSubTest("test_handle_jumppad_activated_server", (PFNUNITSUBTEST)&PlayerTest::test_handle_jumppad_activated_server);
        AddSubTest("test_handle_jumppad_activated_client", (PFNUNITSUBTEST)&PlayerTest::test_handle_jumppad_activated_client);
    }

    virtual bool setUp() override
    {
        m_itemPickupEvents.clear();

        bool b = assertTrue(m_engine && m_engine->isInitialized(), "engine inited");
        
        if (b)
        {
            // by default we are testing server behavior, however in some test cases we manually set it to client for specific testing
            m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
            b &= assertTrue(m_network.initialize(), "network inited");
        }

        return b;
    }

    virtual void TearDown() override
    {
        m_bullets.clear();
        m_network.shutdown();
    }

    virtual void Finalize() override
    {
        if (m_engine)
        {
            m_engine->shutdown();
            m_engine = NULL;
        }

        m_audio.shutdown();

        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::Player::getLoggerModuleName(), false);
    }

private:

    enum class SetDfltWpn
    {
        Yes,
        No
    };

    pge_audio::PgeAudio m_audio;
    PGEcfgProfiles& m_cfgProfiles;
    PR00FsUltimateRenderingEngine* m_engine;
    std::list<Bullet> m_bullets;
    proofps_dd::EventLister m_itemPickupEvents;
    proofps_dd::EventLister m_ammoChangeEvents;
    pge_network::PgeNetworkStub m_network;

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
        char szNewUserName[proofps_dd::MsgUserNameChangeAndBootupDone::nUserNameBufferLength];
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player> mapPlayers;
        bool b = true;

        // insert 20 players with same name, no name truncating should happen
        for (int i = 0; i < 20; i++)
        {
            const pge_network::PgeNetworkConnectionHandle connHandle = static_cast<pge_network::PgeNetworkConnectionHandle>(i);
            const auto insertRes = mapPlayers.insert(
                {
                    connHandle,
                    proofps_dd::Player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandle, std::string("192.168.1.") + std::to_string(i))
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
                    proofps_dd::Player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandle, std::string("192.168.1.") + std::to_string(i))
                });
            b &= insertRes.second;
            if (!b)
            {
                return assertFalse(true, (std::string("player ") + std::to_string(i) + " insert").c_str());
            }
            proofps_dd::Player::genUniqueUserName(szNewUserName, "ASDASDASD0ASDASDASD0ASDASDASD0ASDASDASD0ASDASDASD0", mapPlayers);
            proofps_dd::Player& insertedPlayer = insertRes.first->second;
            insertedPlayer.setName(szNewUserName);
        }

        // checkpoint 2
        for (const auto& connHandlePlayerPair : mapPlayers)
        {
            b &= assertEquals(
                    "ASDASDASD0ASDASDASD0ASDASDASD0ASDASDASD0A",
                    connHandlePlayerPair.second.getName().substr(0, proofps_dd::MsgUserNameChangeAndBootupDone::nUserNameBufferLength - 4u),
                    "player name cp 2") &
                assertEquals(connHandlePlayerPair.second.getName().length(), proofps_dd::MsgUserNameChangeAndBootupDone::nUserNameBufferLength-1u, "player name length cp 2");
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
                    proofps_dd::Player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandle, std::string("192.168.1.") + std::to_string(i))
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

        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, sIpAddr);
        const auto& playerConst = player;

        return (assertEquals(connHandleExpected, player.getServerSideConnectionHandle(), "connhandle") &
            assertEquals(sIpAddr, player.getIpAddress(), "ip address") &
            assertTrue(player.getName().empty(), "name") &
            assertLess(0, player.getTimeConstructed().time_since_epoch().count(), "time constructed") &
            assertEquals(0, player.getTimeBootedUp().time_since_epoch().count(), "time booted up") &
            assertTrue(player.isExpectingAfterBootUpDelayedUpdate(), "expecting after bootup delayed update") &
            assertNotNull(player.getObject3D(), "object3d") &
            assertTrue(player.getObject3D() && player.getObject3D()->isRenderingAllowed(), "object3d visible") &
            assertFalse(player.isDirty(), "isDirty") &
            assertFalse(player.isNetDirty(), "isNetDirty") &
            assertFalse(playerConst.getArmor().isDirty(), "old armor") &
            assertEquals(0, playerConst.getArmor(), "armor") &
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
            assertFalse(player.isJumpingInitiatedFromCrouching(), "isJumpingInitiatedFromCrouching") &
            assertFalse(player.isJumpInitiatedByJumppad(), "isJumpInitiatedByJumppad") &
            assertEquals(0.f, player.getWillJumpXInNextTick(), "will jump X") &
            assertEquals(0.f, player.getWillJumpYInNextTick(), "will jump Y") &
            assertEquals(0, player.getTimeLastSetWillJump().time_since_epoch().count(), "time last setwilljump") &
            assertFalse(player.getWillSomersaultInNextTick(), "will somersault") &
            assertFalse(player.isSomersaulting(), "isSomersaulting") &
            assertEquals(0.f, player.getSomersaultAngle(), "getSomersaultAngle") &
            assertTrue(player.isJustCreatedAndExpectingStartPos(), "expecting start pos") &
            assertTrue(player.isRunning(), "running default") &
            assertEquals(0, player.getTimeLastToggleRun().time_since_epoch().count(), "time last run toggle") &
            assertEquals(proofps_dd::Strafe::NONE, player.getStrafe(), "strafe") &
            assertEquals(proofps_dd::Strafe::NONE, player.getPreviousActualStrafe(), "prev actual strafe") &
            assertEquals(0, player.getTimeLastActualStrafe().time_since_epoch().count(), "time last strafe") &
            assertEquals(0.f, player.getStrafeSpeed(), "strafe speed") &
            assertTrue(player.isMoving(), "moving") /* moving because isInAir() is true, due to canFall() is by default true */ &
            assertFalse(player.getAttack(), "attack") &
            assertEquals(0.f, playerConst.getWeaponMomentaryAccuracy(), "wpn aim") &
            assertFalse(player.getRespawnFlag(), "respawn flag") &
            assertFalse(player.getInvulnerability().isDirty(), "old invulnerability") &
            assertTrue(player.getInvulnerability(), "invulnerability") &
            assertEquals(0, player.getTimeInvulnerabilityStarted().time_since_epoch().count(), "time invulnerability started") &
            assertEquals(0u, player.getInvulnerabilityDurationSeconds(), "invulnerability duration") &
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

        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, sIpAddr);
        player.setName("apple");
        bool b = assertEquals("apple", player.getName(), "apple");

        return b;
    }

    bool test_set_booted_up()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        const std::string sIpAddr = "192.168.1.12";

        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, sIpAddr);
        player.setTimeBootedUp();

        bool b = true;
        b &= (assertLess(0, player.getTimeBootedUp().time_since_epoch().count(), "time booted up 1") &
            assertLess(player.getTimeConstructed().time_since_epoch().count(), player.getTimeBootedUp().time_since_epoch().count(), "time booted up 2"));

        return b;
    }

    bool test_set_expecting_after_boot_up_delayed_update()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.setExpectingAfterBootUpDelayedUpdate(false);
        bool b = assertFalse(player.isExpectingAfterBootUpDelayedUpdate(), "exp 1");

        player.setExpectingAfterBootUpDelayedUpdate(true);
        b &= assertTrue(player.isExpectingAfterBootUpDelayedUpdate(), "exp 2");

        return b;
    }

    bool test_show()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirAutoCrouch].Set(true);
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };

        player.hide();
        player.show();

        bool b = true;
        b &= assertNotNull(player.getObject3D(), "object3d") &
            assertTrue(player.getObject3D() && player.getObject3D()->isRenderingAllowed(), "object3d visible") &
            assertNotNull(player.getWeaponManager().getCurrentWeapon(), "wpn not null") &
            assertTrue(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn visible");

        return b;
    }

    bool test_hide()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirAutoCrouch].Set(true);
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };

        player.hide();

        bool b = true;
        b &= assertNotNull(player.getObject3D(), "object3d") &
            assertFalse(player.getObject3D() && player.getObject3D()->isRenderingAllowed(), "object3d not visible") &
            assertNotNull(player.getWeaponManager().getCurrentWeapon(), "wpn not null") &
            assertFalse(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn not visible");

        return b;
    }

    bool test_set_visibility_state()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirAutoCrouch].Set(true);
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };

        player.setVisibilityState(false);

        bool b = true;
        b &= assertNotNull(player.getObject3D(), "object3d 1") &
            assertFalse(player.getObject3D() && player.getObject3D()->isRenderingAllowed(), "object3d not visible 1") &
            assertNotNull(player.getWeaponManager().getCurrentWeapon(), "wpn not null 1") &
            assertFalse(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn not visible 1");

        player.setVisibilityState(true);

        b &= assertNotNull(player.getObject3D(), "object3d 2") &
            assertTrue(player.getObject3D() && player.getObject3D()->isRenderingAllowed(), "object3d visible 2") &
            assertNotNull(player.getWeaponManager().getCurrentWeapon(), "wpn not null 2") &
            assertTrue(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn visible 2");

        return b;
    }

    bool test_dirtiness_one_by_one()
    {
        proofps_dd::Player player(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        bool b = (assertFalse(player.isDirty(), "dirty 1") &
            assertFalse(player.isNetDirty(), "net dirty 1")) != 0;

        player.setArmor(5);
        b &= assertTrue(player.isDirty(), "dirty X 1");
        b &= assertFalse(player.isNetDirty(), "net dirty X 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty X 2");
        b &= assertTrue(player.isNetDirty(), "net dirty X 2");
        player.clearNetDirty();
        b &= assertFalse(player.isNetDirty(), "net dirty X 3");

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

        player.setWeaponMomentaryAccuracy(2.f);
        b &= assertTrue(player.isDirty(), "dirty W 1");
        b &= assertFalse(player.isNetDirty(), "net dirty W 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty W 2");
        b &= assertTrue(player.isNetDirty(), "net dirty W 2");
        player.clearNetDirty();
        b &= assertFalse(player.isNetDirty(), "net dirty W 3");

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

        player.setInvulnerability(false);
        b &= assertTrue(player.isDirty(), "dirty H 1");
        b &= assertFalse(player.isNetDirty(), "net dirty H 1");
        player.updateOldValues();
        b &= assertFalse(player.isDirty(), "dirty H 2");
        b &= assertTrue(player.isNetDirty(), "net dirty H 2");
        player.clearNetDirty();
        b &= assertFalse(player.isNetDirty(), "net dirty H 3");

        return b;
    }

    bool test_update_old_frags_and_deaths()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

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

    bool test_set_just_created_and_expecting_start_pos()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.setJustCreatedAndExpectingStartPos(false);
        bool b = assertFalse(player.isJustCreatedAndExpectingStartPos(), "exp 1");
        
        player.setJustCreatedAndExpectingStartPos(true);
        b &= assertTrue(player.isJustCreatedAndExpectingStartPos(), "exp 2");

        return b;
    }

    bool test_update_old_pos()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

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

    bool test_set_armor_and_update_old_armor()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        const auto& playerConst = player;

        player.setArmor(200);
        bool b = (assertEquals(100, playerConst.getArmor(), "armor 1") &
            assertTrue(playerConst.getArmor().isDirty(), "dirty 1") &
            assertTrue(playerConst.isDirty(), "dirty 2")) != 0;

        player.updateOldValues();
        player.setArmor(-1);
        b &= (assertEquals(0, playerConst.getArmor(), "armor 2") &
            assertTrue(playerConst.getArmor().isDirty(), "dirty 3") &
            assertTrue(playerConst.isDirty(), "dirty 4")) != 0;

        player.updateOldValues();
        b &= assertTrue(playerConst.isNetDirty(), "is net dirty");
        b &= (assertEquals(0, playerConst.getArmor().getOld(), "armor 3") &
            assertFalse(playerConst.getArmor().isDirty(), "dirty 5") &
            assertFalse(playerConst.isDirty(), "dirty 6")) != 0;

        return b;
    }

    bool test_set_health_and_update_old_health()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
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
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        const auto& playerConst = player;

        /* these are with default 0 AP, so we expect 100% of HP damage to be subtracted from player HP */

        player.doDamage(0 /* AP */, 25);
        bool b = (assertEquals(75, playerConst.getHealth(), "health 1") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 1") &
            assertEquals(0, playerConst.getArmor(), "armor 1") &
            assertEquals(0, playerConst.getArmor().getOld(), "old armor 1")) != 0;

        player.doDamage(0 /* AP */, 25);
        b &= (assertEquals(50, playerConst.getHealth(), "health 2") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 2") &
            assertEquals(0, playerConst.getArmor(), "armor 2") &
            assertEquals(0, playerConst.getArmor().getOld(), "old armor 2")) != 0;

        player.doDamage(0 /* AP */, 100); /* should not go below 0 */
        b &= (assertEquals(0, playerConst.getHealth(), "health 3") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 3") &
            assertEquals(0, playerConst.getArmor(), "armor 3") &
            assertEquals(0, playerConst.getArmor().getOld(), "old armor 3")) != 0;

        /* try doing damage to AP now */

        player.setHealth(100);
        player.setArmor(100);

        player.doDamage(25 /* AP */, 0);
        b &= (assertEquals(75, playerConst.getArmor(), "armor 4") &
            assertEquals(0, playerConst.getArmor().getOld(), "old armor 4") &
            assertEquals(100, playerConst.getHealth(), "health 4") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 4")) != 0;

        player.doDamage(25 /* AP */, 0);
        b &= (assertEquals(50, playerConst.getArmor(), "armor 5") &
            assertEquals(0, playerConst.getArmor().getOld(), "old armor 5") & 
            assertEquals(100, playerConst.getHealth(), "health 5") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 5")) != 0;

        player.doDamage(100 /* AP */, 0); /* should not go below 0 */
        b &= (assertEquals(0, playerConst.getArmor(), "armor 6") &
            assertEquals(0, playerConst.getArmor().getOld(), "old armor 6") &
            assertEquals(100, playerConst.getHealth(), "health 6") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 6")) != 0;

        /* now see what happens when we have no AP, but do AP damage and HP damage too */

        player.setArmor(0);
        player.setHealth(100);

        player.doDamage(25 /* AP */, 50);
        b &= (assertEquals(0, playerConst.getArmor(), "armor 7") &
            assertEquals(0, playerConst.getArmor().getOld(), "old armor 7") &
            assertEquals(50, playerConst.getHealth(), "health 7") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 7")) != 0;

        /* now see what happens when we have AP, and do AP damage and HP damage too */

        player.setArmor(100);
        player.setHealth(100);

        player.doDamage(50 /* AP */, 40);
        b &= (assertEquals(50, playerConst.getArmor(), "armor 8") &
            assertEquals(0, playerConst.getArmor().getOld(), "old armor 8") &
            assertEquals(80, playerConst.getHealth(), "health 8") & /* AP 50 decreases damage 40 by 50% to 20 */
            assertEquals(100, playerConst.getHealth().getOld(), "old health 8")) != 0;

        // do same damage again

        player.doDamage(50 /* AP */, 40);
        b &= (assertEquals(0, playerConst.getArmor(), "armor 9") &
            assertEquals(0, playerConst.getArmor().getOld(), "old armor 9") &
            assertEquals(40, playerConst.getHealth(), "health 9") & /* AP 0 decreases damage 40 by 0% */
            assertEquals(100, playerConst.getHealth().getOld(), "old health 9")) != 0;

        return b;
    }

    bool test_die_server()
    {
        // armor is not tested since it has nothing to do with dieing

        constexpr bool bServer = true;
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
        const auto& playerConst = player;
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirAutoCrouch].Set(true);
        player.setJumpAllowed(true);
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };

        player.getAttack() = true; // this should be reset by die()
        player.getJumpForce().Set(1,2,3); // this should be also reset by die()
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        player.setStrafe(proofps_dd::Strafe::LEFT); // we are doing this so we can test if prevActualStrafe is also reset by die()!
        player.setWillJumpInNextTick(1.f, 1.f); // this should be also reset by die()
        player.jump();
        player.startSomersaultServer(true);
        player.setJumpAllowed(true); // jump() has set it to false, but we forcing it now back to true and expect it to be reset by die()
        player.setWillSomersaultInNextTick(true); // this should be also reset by die()
        bool b = assertTrue(player.isSomersaulting(), "somersaulting 0");

        player.die(true, bServer);

        const auto nFirstTimeDiedSinceEpoch = playerConst.getTimeDied().time_since_epoch().count();
        b &= (assertEquals(0, playerConst.getHealth(), "health 1") &
            assertEquals(100, playerConst.getHealth().getOld(), "old health 1") &
            assertFalse(player.getAttack(), "attack 1") &
            assertEquals(PureVector(), player.getJumpForce(), "jumpforce 1") &
            assertEquals(0.f, player.getWillJumpXInNextTick(), "will jump X 1") &
            assertEquals(0.f, player.getWillJumpYInNextTick(), "will jump Y 1") &
            assertEquals(proofps_dd::Strafe::NONE, playerConst.getStrafe(), "strafe 1") &
            assertEquals(proofps_dd::Strafe::NONE, player.getPreviousActualStrafe(), "prev actual strafe 1") &
            assertEquals(0.f, player.getStrafeSpeed(), "strafe speed") &
            assertFalse(playerConst.isSomersaulting(), "somersaulting 1") &
            assertFalse(player.getWillSomersaultInNextTick(), "will somersault 1") &
            assertNotEquals(0, nFirstTimeDiedSinceEpoch, "time died a 1") &
            assertFalse(playerConst.getObject3D()->isRenderingAllowed(), "player object visible 1") &
            assertFalse(playerConst.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible 1") &
            assertEquals(1, playerConst.getDeaths(), "deaths 1") /* server increases it */ &
            assertEquals(0, playerConst.getDeaths().getOld(), "old deaths 1")) != 0;

        // now we just testing for getTimeDied()
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
        // armor is not tested since it has nothing to do with dieing

        constexpr bool bServer = false;
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
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
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
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
        player.getJumpForce().Set(1,2,3);
        player.setGravity(-15);
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        player.setStrafe(proofps_dd::Strafe::LEFT); // we are doing this so we can test if prevActualStrafe is also reset!
        player.setArmor(50);

        player.die(true, bServer);
        player.respawn(true, *(player.getWeaponManager().getWeapons()[0]), bServer);

        const auto& playerConst = player;
        return (b & assertTrue(player.getObject3D()->isRenderingAllowed(), "player object visible") &
            assertTrue(player.getWeaponManager().getCurrentWeapon()->getObject3D().isRenderingAllowed(), "wpn object visible") &
            assertFalse(player.getWeaponManager().getWeapons()[1]->isAvailable(), "wpn 2 not available") &
            assertEquals(player.getWeaponManager().getWeapons()[0], player.getWeaponManager().getCurrentWeapon(), "current wpn") &
            assertFalse(player.getCrouchStateCurrent(), "getCrouchStateCurrent") &
            assertTrue(player.getWantToStandup(), "wantstandup") &
            assertEquals(PureVector(), player.getImpactForce(), "impact force") &
            assertEquals(PureVector(), player.getJumpForce(), "jump force") &
            assertEquals(0.f, player.getGravity(), "gravity") &
            assertEquals(proofps_dd::Strafe::NONE, player.getPreviousActualStrafe(), "prev actual strafe") &
            assertEquals(0, playerConst.getArmor(), "armor")) != 0;
    }

    bool test_set_invulnerability()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");

        player.setInvulnerability(false);

        bool b = true;
        b &= assertTrue(player.getInvulnerability().isDirty(), "old invulnerability") &
            assertFalse(player.getInvulnerability(), "invulnerability");

        player.setInvulnerability(true, 25);
        b &= assertTrue(player.getInvulnerability(), "invulnerability") &
            assertNotEquals(0, player.getTimeInvulnerabilityStarted().time_since_epoch().count(), "time invulnerability started") &
            assertEquals(25u, player.getInvulnerabilityDurationSeconds(), "invulnerability duration");

        return b;
    }

    bool test_jump()
    {
        bool b = true;
        // use loop with 2 cycles for 2 different starting state: standing and crouching.
        for (int i = 0; i < 2; i++)
        {
            proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

            player.getPos().set(PureVector(1.f, 2.f, 3.f));
            player.getPos().commit();
            player.getPos().set(PureVector(2.f, 4.f, 8.f));

            // we modify only the X-component of jumpForce in jump(), since other components are not used at all!
            //const PureVector vecExpectedForce = player.getPos().getNew() - player.getPos().getOld();
            const PureVector vecExpectedForceForRegularJump(player.getPos().getNew().getX() - player.getPos().getOld().getX(), 0.f, 0.f);
            const PureVector vecExpectedForceForJumppadJump(0.f, 0.f, 0.f);

            player.getCrouchInput().set(i == 1);
            const float fExpectedInitialGravity_a1 = player.getCrouchInput() ? proofps_dd::Player::fJumpGravityStartFromCrouching : proofps_dd::Player::fJumpGravityStartFromStanding;
            player.getCrouchStateCurrent() = true;  // jumping is not changing crouching state since crouching is also valid mid-air

            // positive test, with player input-induced jump (indicated by 1.f as jump force multiplier in setWillJumpInNextTick())
            // TODO: maybe we should also check for object height
            player.setJumpAllowed(true);
            b &= assertTrue(player.jumpAllowed(), "allowed a 1") &
                assertFalse(player.isJumping(), "jumping a 1") &
                assertEquals(0.f, player.getGravity(), "gravity a 1") &
                assertEquals(PureVector(), player.getJumpForce(), "jump force a 1") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent a 1") &
                assertTrue(player.getWantToStandup(), "wantstandup a 1");

            const std::chrono::time_point<std::chrono::steady_clock> timeBeforeSetWillJump_a1 = std::chrono::steady_clock::now();
            player.setWillJumpInNextTick(1.f, 0.f); // any positive Y value means jumping
            const std::chrono::time_point<std::chrono::steady_clock> timeLastSetWillJump_a1 = player.getTimeLastSetWillJump();
            b &= assertTrue(timeLastSetWillJump_a1 > timeBeforeSetWillJump_a1, "cmp timeBefore a 1") &
                assertTrue(timeLastSetWillJump_a1 < std::chrono::steady_clock::now(), "cmp timeAfter a 1");
            b &= assertEquals(1.f, player.getWillJumpYInNextTick(), "will jump Y a 1");
            player.jump();
            b &= assertFalse(player.jumpAllowed(), "allowed a 2") &
                assertTrue(player.isJumping(), "jumping a 2") &
                assertEquals(fExpectedInitialGravity_a1, player.getGravity(), (std::string("gravity a 2 crouch: ") + std::to_string(player.getCrouchInput())).c_str()) &
                assertEquals(vecExpectedForceForRegularJump, player.getJumpForce(), "jump force a 2") &
                assertEquals(0.f, player.getWillJumpYInNextTick(), "will jump Y a 2") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent a 2") &
                assertTrue(player.getWantToStandup(), "wantstandup a 2");

            player.stopJumping();
            b &= assertFalse(player.jumpAllowed(), "allowed a 3") &
                assertFalse(player.isJumping(), "jumping a 3") &
                assertEquals(fExpectedInitialGravity_a1, player.getGravity(), (std::string("gravity a 3 crouch: ") + std::to_string(player.getCrouchInput())).c_str()) &
                assertEquals(vecExpectedForceForRegularJump, player.getJumpForce(), "jump force a 3") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent a 3") &
                assertTrue(player.getWantToStandup(), "wantstandup a 3");

            constexpr float fJumppadJumpForceMultiplier = 2.f;
            const float fExpectedInitialGravity_b1 = fJumppadJumpForceMultiplier * proofps_dd::Player::fJumpGravityStartFromStanding /* does not depend on crouching state */;
            
            // positive test, with jumppad-induced jump (indicated by positive, non-1.f as Y jump force multiplier in setWillJumpInNextTick())
            // TODO: maybe we should also check for object height
            player.setJumpAllowed(true);
            b &= assertTrue(player.jumpAllowed(), "allowed b 1") &
                assertFalse(player.isJumping(), "jumping b 1") &
                assertEquals(fExpectedInitialGravity_a1, player.getGravity(), "gravity b 1") &
                assertEquals(vecExpectedForceForRegularJump, player.getJumpForce(), "jump force b 1") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent b 1") &
                assertTrue(player.getWantToStandup(), "wantstandup b 1");

            const std::chrono::time_point<std::chrono::steady_clock> timeBeforeSetWillJump_b1 = std::chrono::steady_clock::now();
            player.setWillJumpInNextTick(fJumppadJumpForceMultiplier, 0.f); // any positive Y value means jumping
            const std::chrono::time_point<std::chrono::steady_clock> timeLastSetWillJump_b1 = player.getTimeLastSetWillJump();
            b &= assertTrue(timeLastSetWillJump_b1 > timeBeforeSetWillJump_b1, "cmp timeBefore b 1") &
                assertTrue(timeLastSetWillJump_b1 < std::chrono::steady_clock::now(), "cmp timeAfter b 1");
            b &= assertEquals(fJumppadJumpForceMultiplier, player.getWillJumpYInNextTick(), "will jump Y b 1");
            player.jump();
            b &= assertFalse(player.jumpAllowed(), "allowed b 2") &
                assertTrue(player.isJumping(), "jumping b 2") &
                assertEquals(fExpectedInitialGravity_b1, player.getGravity(), (std::string("gravity b 2 crouch: ") + std::to_string(player.getCrouchInput())).c_str()) &
                assertEquals(vecExpectedForceForJumppadJump, player.getJumpForce(), "jump force b 2") &
                assertEquals(0.f, player.getWillJumpYInNextTick(), "will jump Y b 2") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent b 2") &
                assertTrue(player.getWantToStandup(), "wantstandup b 2");

            player.stopJumping();
            b &= assertFalse(player.jumpAllowed(), "allowed b 3") &
                assertFalse(player.isJumping(), "jumping b 3") &
                assertEquals(fExpectedInitialGravity_b1, player.getGravity(), (std::string("gravity b 3 crouch: ") + std::to_string(player.getCrouchInput())).c_str()) &
                assertEquals(vecExpectedForceForJumppadJump, player.getJumpForce(), "jump force b 3") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent b 3") &
                assertTrue(player.getWantToStandup(), "wantstandup b 3");

            // negative test
            player.setJumpAllowed(false);
            player.setWillJumpInNextTick(1.f, 0.f);
            b &= assertEquals(0.f, player.getWillJumpYInNextTick(), "will jump Y 3") &
                assertTrue(player.getTimeLastSetWillJump() == timeLastSetWillJump_b1, "time last setwilljump unchanged") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent 4") &
                assertTrue(player.getWantToStandup(), "wantstandup 4");

            player.jump();
            b &= assertFalse(player.jumpAllowed(), "allowed 4") &
                assertFalse(player.isJumping(), "jumping 4") &
                assertEquals(fExpectedInitialGravity_b1, player.getGravity(), (std::string("gravity 4 crouch: ") + std::to_string(player.getCrouchInput())).c_str()) &
                assertEquals(vecExpectedForceForJumppadJump, player.getJumpForce(), "jump force 4") &
                assertTrue(player.getCrouchStateCurrent(), "getCrouchStateCurrent 5") &
                assertTrue(player.getWantToStandup(), "wantstandup 5");

            // negative test: we are already somersaulting on-ground
            player.stopJumping();
            player.setJumpAllowed(true);
            player.setCanFall(false);
            player.setStrafe(proofps_dd::Strafe::RIGHT);
            player.startSomersaultServer(false);
            player.setWillJumpInNextTick(1.f, 0.f);
            player.jump();
            b &= assertTrue(player.isSomersaulting(), "somersaulting 1") &
                assertFalse(player.isJumping(), "jumping 5") &
                assertEquals(vecExpectedForceForJumppadJump, player.getJumpForce(), "jump force 5") &
                assertEquals(0.f, player.getWillJumpYInNextTick(), "will jump Y 5");

        } // end for i

        return b;
    }

    bool test_set_can_fall()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.setCanFall(false);
        bool b = assertFalse(player.canFall(), "can fall 1");

        player.setCanFall(true);
        b &= assertTrue(player.canFall(), "can fall 2");

        return b;
    }

    bool test_gravity()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

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
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        
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

    bool test_is_in_air()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        // we are on the ground now
        player.setCanFall(false);
        player.setJumpAllowed(true);

        bool b = assertFalse(player.isInAir(), "1");

        // jumping up
        player.jump();
        b &= assertTrue(player.isInAir(), "2");

        // reached max Y pos
        player.stopJumping();
        b &= assertFalse(player.isInAir(), "3");

        // canFall needs to be set manually
        player.setCanFall(true);
        b &= assertTrue(player.isInAir(), "4");

        // TODO: canFall() and setCanFall() are a bit fishy. They have been around since the very beginning, but using isFalling() seems to be better.

        return b;
    }

    bool test_start_somersault_server_mid_air_when_auto_crouch_is_disabled()
    {
        constexpr bool bJumpInducedSomersault = true;
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirAutoCrouch].Set(false);
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirJumpForceMultiplier].Set(2.f);
        
        player.setJumpAllowed(true);
        player.getPos().set(PureVector()); // old
        player.getPos().commit();
        player.getPos().set(PureVector(1.f, 1.f, 1.f)); // new
        player.setGravity(5.f);

        // negative case: not even jumping
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        bool b = assertFalse(player.isSomersaulting(), "cannot somersault on ground");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault neg 1");
        b &= assertEquals(PureVector(), player.getJumpForce(), "jumpforce neg 1");
        b &= assertEquals(5.f, player.getGravity(), "gravity neg 1");
        b &= assertEquals(0.f, player.getImpactForce().getX(), "impact force x neg 1");

        // negative case: initiated jump from crouching
        player.getCrouchInput() = true; // at the moment of triggering jump (still being on the ground), we are pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        auto vecOriginalJumpForce = player.getJumpForce();
        auto fOriginalGravity = player.getGravity();
        auto fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = true; // and we are still crouching
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertFalse(player.isSomersaulting(), "cannot somersault from crouch-jumping from ground");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault neg 2");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce neg 2");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity neg 2");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x neg 2");

        // negative case: not initiated crouching after jumping but startSomersaultServer() is triggered
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = false; // and we are still NOT crouching
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertFalse(player.isSomersaulting(), "cannot somersault without manual crouch after jump if auto crouch is disabled");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault neg 3");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce neg 3");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity neg 3");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x neg 3");

        // negative case: all input is properly executed but based on jump factor, it is a jumppad-induced jump!
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(2.f, 0.f); // here we indicate jumppad-induced jump with non-1.f Y mult factor
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = true; // in the meantime we have started crouching AFTER triggering jump
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertFalse(player.isSomersaulting(), "cannot somersault if jumppad triggered the jump");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault neg 4");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce neg 4");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity neg 4");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x neg 4");

        // positive case
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = true; // in the meantime we have started crouching AFTER triggering jump
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 1");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault pos 4");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(0.1f, player.getSomersaultAngle(), "angle 1 when strafe is NONE");
        b &= assertEquals(vecOriginalJumpForce * 2.f, player.getJumpForce(), "jumpforce pos 4");
        b &= assertEquals(fOriginalGravity * 2.f, player.getGravity(), "gravity pos 4");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x pos 4");

        // positive case
        // same as previous but we change Strafe and Multiplier for Jump Force
        player.resetSomersaultServer();
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirJumpForceMultiplier].Set(1.5f);
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = true; // in the meantime we have started crouching AFTER triggering jump
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 2");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault pos 5");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(-0.1f, player.getSomersaultAngle(), "angle 2 when strafe is RIGHT");
        b &= assertEquals(vecOriginalJumpForce * 1.5f, player.getJumpForce(), "jumpforce pos 5");
        b &= assertEquals(fOriginalGravity * 1.5f, player.getGravity(), "gravity pos 5");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x pos 5");

        // positive case
        // same as previous but we change Strafe and Multiplier for Jump Force
        player.resetSomersaultServer();
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirJumpForceMultiplier].Set(1.f);
        player.setStrafe(proofps_dd::Strafe::LEFT);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = true; // in the meantime we have started crouching AFTER triggering jump
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 3");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault pos 6");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(0.1f, player.getSomersaultAngle(), "angle 3 when strafe is LEFT");
        b &= assertEquals(vecOriginalJumpForce * 1.f, player.getJumpForce(), "jumpforce pos 6");
        b &= assertEquals(fOriginalGravity * 1.f, player.getGravity(), "gravity pos 6");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x pos 6");

        return b;
    }

    bool test_start_somersault_server_mid_air_when_auto_crouch_is_enabled()
    {
        constexpr bool bJumpInducedSomersault = true;
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirAutoCrouch].Set(true);
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirJumpForceMultiplier].Set(2.f);
        player.setJumpAllowed(true);
        player.getPos().set(PureVector()); // old
        player.getPos().commit();
        player.getPos().set(PureVector(1.f, 1.f, 1.f)); // new
        player.setGravity(5.f);

        // negative case: not even jumping
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        bool b = assertFalse(player.isSomersaulting(), "cannot somersault on ground");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault neg 1");
        b &= assertEquals(PureVector(), player.getJumpForce(), "jumpforce neg 1");
        b &= assertEquals(5.f, player.getGravity(), "gravity neg 1");
        b &= assertEquals(0.f, player.getImpactForce().getX(), "impact force x neg 1");

        // negative case: initiated jump from crouching
        player.getCrouchInput() = true; // at the moment of triggering jump (still being on the ground), we are pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        auto vecOriginalJumpForce = player.getJumpForce();
        auto fOriginalGravity = player.getGravity();
        auto fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = true; // and we are still crouching (although not needed since auto-crouch is enabled)
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertFalse(player.isSomersaulting(), "cannot somersault from crouch-jumping from ground 1");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault neg 2");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce neg 2");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity neg 2");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x neg 2");

        // negative case: initiated jump from crouching, but not crouching anymore in-air, basically almost same as previous case
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = true; // at the moment of triggering jump (still being on the ground), we are pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = false; // but we are NOT crouching anymore (although not needed since auto-crouch is enabled)
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertFalse(player.isSomersaulting(), "cannot somersault from crouch-jumping from ground 2");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault neg 3");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce neg 3");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity neg 3");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x neg 3");

        // negative case: all input is properly executed but based on jump factor, it is a jumppad-induced jump!
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(2.f, 0.f); // here we indicate jumppad-induced jump with non-1.f Y mult factor
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = false; // but we are NOT crouching anymore (although not needed since auto-crouch is enabled)
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertFalse(player.isSomersaulting(), "cannot somersault if jumppad triggered the jump");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault neg 4");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce neg 4");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity neg 4");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x neg 4");

        // positive case
        player.resetSomersaultServer();
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        // in the meantime we have started crouching AFTER triggering jump (although not needed since auto-crouch is enabled, but in this case we test this anyway)
        player.getCrouchStateCurrent() = true;
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 1");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault pos 4");
        b &= assertEquals(vecOriginalJumpForce * 2, player.getJumpForce(), "jumpforce pos 4");
        b &= assertEquals(fOriginalGravity * 2, player.getGravity(), "gravity pos 4");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x pos 4");

        // positive case
        player.resetSomersaultServer();
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = false; // and we are still NOT crouching (not needed since auto-crouch is enabled)
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 2");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault pos 5");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(0.1f, player.getSomersaultAngle(), "angle 1 when strafe is NONE");
        b &= assertEquals(vecOriginalJumpForce * 2.f, player.getJumpForce(), "jumpforce pos 5");
        b &= assertEquals(fOriginalGravity * 2.f, player.getGravity(), "gravity pos 5");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x pos 5");

        // positive case
        // same as previous but we change Strafe and Multiplier for Jump Force
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirJumpForceMultiplier].Set(1.5f);
        player.resetSomersaultServer();
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = false; // and we are still NOT crouching (not needed since auto-crouch is enabled)
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 3");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault pos 6");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(-0.1f, player.getSomersaultAngle(), "angle 2 when strafe is RIGHT");
        b &= assertEquals(vecOriginalJumpForce * 1.5f, player.getJumpForce(), "jumpforce pos 6");
        b &= assertEquals(fOriginalGravity * 1.5f, player.getGravity(), "gravity pos 6");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x pos 6");

        // positive case
        // same as previous but we change Strafe and Multiplier for Jump Force
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirJumpForceMultiplier].Set(1.f);
        player.resetSomersaultServer();
        player.stopJumping();
        player.setJumpAllowed(true); // allow jump again
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setStrafe(proofps_dd::Strafe::LEFT);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = false; // and we are still NOT crouching (not needed since auto-crouch is enabled)
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 4");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault pos 7");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(0.1f, player.getSomersaultAngle(), "angle 3 when strafe is LEFT");
        b &= assertEquals(vecOriginalJumpForce * 1.f, player.getJumpForce(), "jumpforce pos 7");
        b &= assertEquals(fOriginalGravity * 1.f, player.getGravity(), "gravity pos 7");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x pos 7");

        return b;
    }

    bool test_start_somersault_server_on_ground()
    {
        constexpr bool bJumpInducedSomersault = false;
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirAutoCrouch].Set(true); // should affect mid-air somersault only
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirJumpForceMultiplier].Set(2.f); // jump forces must NOT change in this test
        player.setJumpAllowed(true);
        player.getPos().set(PureVector()); // old
        player.getPos().commit();
        player.getPos().set(PureVector(1.f, 1.f, 1.f)); // new
        player.setGravity(5.f);

        // negative case
        player.getCrouchInput() = true; // at the moment of triggering jump (still being on the ground), we are pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        auto vecOriginalJumpForce = player.getJumpForce();
        auto fOriginalGravity = player.getGravity();
        auto fOriginalImpactForceX = player.getImpactForce().getX();
        player.getCrouchStateCurrent() = true; // and we are still crouching
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        bool b = assertFalse(player.isSomersaulting(), "cannot somersault on-ground when jumping up");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault 1");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 1");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 1");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x 1");

        // negative case
        player.stopJumping();
        player.setCanFall(true); // we are falling now
        player.getCrouchStateCurrent() = true; // and we are still crouching
        player.setGravity(-5.f);
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertFalse(player.isSomersaulting(), "cannot somersault on-ground when falling down");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault 2");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 2");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 2");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x 2");

        // negative case
        player.setJumpAllowed(true); // allow jump again
        player.setCanFall(false); // we are not falling anymore
        player.getCrouchInput() = false;
        player.getCrouchStateCurrent() = false; // we are not pressing crouch
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertFalse(player.isSomersaulting(), "cannot somersault on-ground when not crouching");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault 3");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 3");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 3");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x 3");

        // negative case
        // we are on-ground and crouching, but not strafing
        player.getCrouchInput() = false;
        player.getCrouchStateCurrent() = true; // we are not pressing crouch but we are crouching (e.g. in a low tunnel)
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertFalse(player.isSomersaulting(), "cannot somersault on-ground when not strafing");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault 4");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 4");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 4");
        b &= assertEquals(fOriginalImpactForceX, player.getImpactForce().getX(), "impact force x 4");

        // positive case
        // finally we are strafing
        player.resetSomersaultServer();
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 1");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault 5");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(-0.1f, player.getSomersaultAngle(), "angle 1 when strafe is RIGHT");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 5");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 5");
        b &= assertEquals(fOriginalImpactForceX + proofps_dd::Player::fSomersaultGroundImpactForceX, player.getImpactForce().getX(), "impact force x 5");

        // positive case
        // same as previous but we change initial impact force
        player.getImpactForce().SetX(5.f);
        player.resetSomersaultServer();
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 2");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault 6");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(-0.1f, player.getSomersaultAngle(), "angle 2 when strafe is RIGHT");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 6");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 6");
        // we cannot go above proofps_dd::Player::fSomersaultGroundImpactForceX
        b &= assertEquals(proofps_dd::Player::fSomersaultGroundImpactForceX, player.getImpactForce().getX(), "impact force x 6");

        // positive case
        // now we are changing Strafe
        player.getImpactForce().SetX(0.f);
        player.resetSomersaultServer();
        player.setStrafe(proofps_dd::Strafe::LEFT);
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 3");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault 7");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(0.1f, player.getSomersaultAngle(), "angle 3 when strafe is LEFT");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 7");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 7");
        b &= assertEquals(fOriginalImpactForceX - proofps_dd::Player::fSomersaultGroundImpactForceX, player.getImpactForce().getX(), "impact force x 7");

        // positive case
        // same as previous but we change initial impact force
        player.getImpactForce().SetX(-5.f);
        player.resetSomersaultServer();
        player.setStrafe(proofps_dd::Strafe::LEFT);
        vecOriginalJumpForce = player.getJumpForce();
        fOriginalGravity = player.getGravity();
        fOriginalImpactForceX = player.getImpactForce().getX();
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(bJumpInducedSomersault);
        b &= assertTrue(player.isSomersaulting(), "finally true 4");
        b &= assertFalse(player.getWillSomersaultInNextTick(), "will somersault 8");
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(0.1f, player.getSomersaultAngle(), "angle 4 when strafe is LEFT");
        b &= assertEquals(vecOriginalJumpForce, player.getJumpForce(), "jumpforce 8");
        b &= assertEquals(fOriginalGravity, player.getGravity(), "gravity 8");
        // we cannot go below -proofps_dd::Player::fSomersaultGroundImpactForceX
        b &= assertEquals(-proofps_dd::Player::fSomersaultGroundImpactForceX, player.getImpactForce().getX(), "impact force x 8");

        return b;
    }

    bool test_step_somersault_angle_server()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        m_cfgProfiles.getVars()[proofps_dd::Player::szCVarSvSomersaultMidAirAutoCrouch].Set(true);
        player.setJumpAllowed(true);

        // Strafe is NONE
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        player.startSomersaultServer(true);
        bool b = assertTrue(player.isSomersaulting(), "somersaulting 1");

        player.stepSomersaultAngleServer(30.f);  // step amount is input from physics based on physics rate, here we just pass something
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(30.1f, player.getSomersaultAngle(), "angle 1 when strafe is NONE");
        b &= assertTrue(player.isSomersaulting(), "somersaulting 2");
        player.stepSomersaultAngleServer(400.f);
        b &= assertEquals(0.f, player.getSomersaultAngle(), "angle 2 when strafe is NONE");
        b &= assertFalse(player.isSomersaulting(), "somersaulting 3");

        // Strafe is RIGHT
        player.resetSomersaultServer();
        player.setJumpAllowed(true); // allow jump again
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        player.startSomersaultServer(true);
        b &= assertTrue(player.isSomersaulting(), "somersaulting 4");

        player.stepSomersaultAngleServer(30.f);  // step amount is input from physics based on physics rate, here we just pass something
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(-30.1f, player.getSomersaultAngle(), "angle 1 when strafe is RIGHT");
        b &= assertTrue(player.isSomersaulting(), "somersaulting 5");
        player.stepSomersaultAngleServer(400.f);
        b &= assertEquals(0.f, player.getSomersaultAngle(), "angle 2 when strafe is RIGHT");
        b &= assertFalse(player.isSomersaulting(), "somersaulting 6");

        // Strafe is LEFT
        player.resetSomersaultServer();
        player.setJumpAllowed(true); // allow jump again
        player.setStrafe(proofps_dd::Strafe::LEFT);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        player.startSomersaultServer(true);
        b &= assertTrue(player.isSomersaulting(), "somersaulting 7");

        player.stepSomersaultAngleServer(30.f);  // step amount is input from physics based on physics rate, here we just pass something
        // somersault angle also depends on player angle Y but for know this amount of testing is enough
        b &= assertEquals(30.1f, player.getSomersaultAngle(), "angle 1 when strafe is LEFT");
        b &= assertTrue(player.isSomersaulting(), "somersaulting 8");
        player.stepSomersaultAngleServer(400.f);
        b &= assertEquals(0.f, player.getSomersaultAngle(), "angle 2 when strafe is LEFT");
        b &= assertFalse(player.isSomersaulting(), "somersaulting 9");

        return b;
    }

    bool test_set_somersault_client()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        bool b = assertFalse(player.isSomersaulting(), "somersaulting 1");

        player.setSomersaultClient(30.f);
        b &= assertTrue(player.isSomersaulting(), "somersaulting 2");

        player.setSomersaultClient(0.f);
        b &= assertFalse(player.isSomersaulting(), "somersaulting 3");

        return b;
    }

    bool test_set_run()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        const std::chrono::time_point<std::chrono::steady_clock> timeBeforeToggleRun = std::chrono::steady_clock::now();
        player.setRun(false);

        bool b = (assertTrue(player.getTimeLastToggleRun() >= timeBeforeToggleRun, "cmp timeBefore") &
            assertTrue(player.getTimeLastToggleRun() <= std::chrono::steady_clock::now(), "cmp timeAfter")) != 0;
        
        return (b & assertFalse(player.isRunning(), "running")) != 0;
    }

    bool test_set_strafe()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        std::chrono::time_point<std::chrono::steady_clock> timeBeforeStrafe = std::chrono::steady_clock::now();
        player.setStrafe(proofps_dd::Strafe::LEFT);
        bool b = assertEquals(proofps_dd::Strafe::LEFT, player.getStrafe(), "strafe 1");
        b &= assertEquals(proofps_dd::Strafe::NONE, player.getPreviousActualStrafe(), "prev actual strafe 1") &
            assertTrue(player.getTimeLastActualStrafe() >= timeBeforeStrafe, "cmp timeBefore 1") &
            assertTrue(player.getTimeLastActualStrafe() <= std::chrono::steady_clock::now(), "cmp timeAfter 1");

        timeBeforeStrafe = std::chrono::steady_clock::now();
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        b &= assertEquals(proofps_dd::Strafe::RIGHT, player.getStrafe(), "strafe 2");
        b &= assertEquals(proofps_dd::Strafe::LEFT, player.getPreviousActualStrafe(), "prev actual strafe 2") &
            assertTrue(player.getTimeLastActualStrafe() >= timeBeforeStrafe, "cmp timeBefore 2") &
            assertTrue(player.getTimeLastActualStrafe() <= std::chrono::steady_clock::now(), "cmp timeAfter 2");

        player.getStrafeSpeed() = 3.f;
        const std::chrono::time_point<std::chrono::steady_clock> timeBeforeStopStrafe = std::chrono::steady_clock::now();
        player.setStrafe(proofps_dd::Strafe::NONE);
        b &= assertEquals(proofps_dd::Strafe::NONE, player.getStrafe(), "strafe 3");
        b &= assertEquals(proofps_dd::Strafe::RIGHT, player.getPreviousActualStrafe(), "prev actual strafe 3") &
            assertEquals(0.f, player.getStrafeSpeed(), "strafe speed") &
            assertTrue(player.getTimeLastActualStrafe() >= timeBeforeStrafe, "cmp timeBefore 3") &
            assertTrue(player.getTimeLastActualStrafe() < timeBeforeStopStrafe, "cmp timeAfter 3");

        player.setStrafe(proofps_dd::Strafe::LEFT);
        b &= assertEquals(proofps_dd::Strafe::RIGHT, player.getPreviousActualStrafe(), "prev actual strafe 4");

        player.setStrafe(proofps_dd::Strafe::LEFT);
        b &= assertEquals(proofps_dd::Strafe::LEFT, player.getPreviousActualStrafe(), "prev actual strafe 5");

        player.setStrafe(proofps_dd::Strafe::NONE);
        b &= assertEquals(proofps_dd::Strafe::LEFT, player.getPreviousActualStrafe(), "prev actual strafe 6");

        player.setStrafe(proofps_dd::Strafe::NONE);
        b &= assertEquals(proofps_dd::Strafe::LEFT, player.getPreviousActualStrafe(), "prev actual strafe 7");

        return b;
    }

    bool test_server_do_crouch()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        
        // we are on the ground now
        player.setCanFall(false);
        player.setJumpAllowed(true);

        // first we test crouching when being on ground
        const auto vecOriginalPos = player.getPos().getNew();
        player.doCrouchServer();
        // pos should be unchanged since object is shrinked vertically to the center
        bool b = (assertEquals(vecOriginalPos, player.getPos().getNew(), "pos vec 1") &
            assertNotEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 1") &
            assertTrue(player.getCrouchStateCurrent(), "crouch current state 1")) != 0;

        player.doStandupServer();

        // now we test crouching when being in air
        player.jump();
        player.doCrouchServer();
        // pos should be changed since the top of the object stays unchanged (keep the head at the same position) while the legs are pulled upwards!
        b &= assertNotEquals(vecOriginalPos, player.getPos().getNew(), "pos vec 2") &
            assertGreater(player.getPos().getNew().getY(), vecOriginalPos.getY(), "pos Y greater") &
            assertNotEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 2") &
            assertTrue(player.getCrouchStateCurrent(), "crouch current state 2");

        return b;
    }

    bool test_client_do_crouch()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        const auto pOrigTex = player.getObject3D()->getMaterial().getTexture();
        player.doCrouchShared();
        bool b = (assertNotEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 1") &
            assertTrue(player.getCrouchStateCurrent(), "crouch current state 1") &
            assertNotEquals(pOrigTex, player.getObject3D()->getMaterial().getTexture(), "texture 1") &
            assertTrue(player.getWantToStandup(), "want standup intact 1")) != 0;

        return b;
    }

    bool test_get_proposed_new_pos_y_for_standup()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        // we are crouching on the ground now
        player.setCanFall(false);
        player.setJumpAllowed(true);
        player.doCrouchServer();

        // first we test new proposed Y pos when being on ground
        const auto vecOriginalPos = player.getPos().getNew();
        const auto fProposedNewY = player.getProposedNewPosYforStandup();
        // new Y pos should be more up since we are crouching on the ground
        bool b = assertGreater(fProposedNewY, vecOriginalPos.getY(), "pos Y greater 1");

        // now we test standing up when being in air i.e. reset standing height with head position not changing but legs are pushed down
        player.jump();
        const auto vecOriginalPos_2 = player.getPos().getNew();
        const auto fProposedNewY_2 = player.getProposedNewPosYforStandup();
        b &= assertLess(fProposedNewY_2, vecOriginalPos_2.getY(), "pos Y greater 1");

        return b;
    }

    bool test_server_do_standup()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        // we are crouching on the ground now
        player.setCanFall(false);
        player.setJumpAllowed(true);
        player.doCrouchServer();

        // first we test standing up when being on ground
        const auto vecOriginalPos = player.getPos().getNew();
        const auto fProposedNewY = player.getProposedNewPosYforStandup();
        player.doStandupServer();
        // new Y pos should be more up since we were crouching on the ground
        bool b = (assertNotEquals(vecOriginalPos, player.getPos().getNew(), "pos vec 1") &
            assertGreater(player.getPos().getNew().getY(), vecOriginalPos.getY(), "pos Y greater 1") &
            assertEquals(fProposedNewY, player.getPos().getNew().getY(), "pos vec 1") &
            assertEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 1") &
            assertFalse(player.getCrouchStateCurrent(), "crouch current state 1")) != 0;

        // now we test standing up when being in air
        player.jump();
        player.doCrouchServer();

        const auto vecOriginalPos_2 = player.getPos().getNew();
        const auto fProposedNewY_2 = player.getProposedNewPosYforStandup();
        player.doStandupServer();
        // new Y pos should be more down since we are simulating pushing down our legs by keeping the head position at the same position.
        b &= (assertNotEquals(vecOriginalPos, player.getPos().getNew(), "pos vec 2") &
            assertLess(player.getPos().getNew().getY(), vecOriginalPos_2.getY(), "pos Y greater 2") &
            assertEquals(fProposedNewY_2, player.getPos().getNew().getY(), "pos vec 2") &
            assertEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 2") &
            assertFalse(player.getCrouchStateCurrent(), "crouch current state 2")) != 0;

        return b;
    }

    bool test_client_do_standup()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");

        player.doCrouchShared();
        const auto pOrigTex = player.getObject3D()->getMaterial().getTexture();

        player.doStandupShared();
        bool b = (assertEquals(1.f, player.getObject3D()->getScaling().getY(), "scaling Y 1") &
            assertFalse(player.getCrouchStateCurrent(), "crouch current state 1") &
            assertNotEquals(pOrigTex, player.getObject3D()->getMaterial().getTexture(), "texture 1") &
            assertTrue(player.getWantToStandup(), "want standup intact 1")) != 0;

        return b;
    }

    bool test_is_moving()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
        bool b = true;

        // we are on the ground now
        player.setCanFall(false);
        player.setJumpAllowed(true);
        b &= assertFalse(player.isMoving(), "still on the ground 1");

        // simple jump
        player.jump();
        b &= assertTrue(player.isMoving(), "jump");

        // copied from test_is_in_air: reached max Y pos, this is the moment when we are stationary!
        player.stopJumping();
        b &= assertFalse(player.isMoving(), "stationary in-air");

        // ccopied from test_is_in_air: canFall needs to be set manually
        player.setCanFall(true);
        b &= assertTrue(player.isMoving(), "falling");

        player.setCanFall(false);
        player.setJumpAllowed(true); // allow jump again

        b &= assertFalse(player.isMoving(), "still on the ground 2");

        // jump-induced somersault
        player.getCrouchInput() = false; // at the moment of triggering jump (still being on the ground), we are not pressing crouch
        player.setGravity(5.f);
        player.setWillJumpInNextTick(1.f, 0.f); // Y need to be 1.f, otherwise startSomersaultServer() thinks it is jumppad-induced jump and rejects
        player.jump();
        player.getCrouchStateCurrent() = true; // in the meantime we have started crouching AFTER triggering jump
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(true);
        b &= assertTrue(player.isMoving(), "somersault from jump");
        player.resetSomersaultServer();
        player.getCrouchStateCurrent() = false;
        player.stopJumping();
        player.setCanFall(false);
        player.setJumpAllowed(true); // allow jump again

        b &= assertFalse(player.isMoving(), "still on the ground 3");

        // on-ground somersaulting
        player.getCrouchInput() = true;
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        player.setWillSomersaultInNextTick(true);
        player.startSomersaultServer(false);
        b &= assertTrue(player.isMoving(), "somersault from crouch");
        player.resetSomersaultServer();
        player.getCrouchInput() = false;
        player.setStrafe(proofps_dd::Strafe::NONE);

        b &= assertFalse(player.isMoving(), "still on the ground 4");

        // strafe to left
        player.setStrafe(proofps_dd::Strafe::LEFT);
        b &= assertTrue(player.isMoving(), "strafe to left");
        player.setStrafe(proofps_dd::Strafe::NONE);

        b &= assertFalse(player.isMoving(), "still on the ground 5");

        // strafe to right
        player.setStrafe(proofps_dd::Strafe::RIGHT);
        b &= assertTrue(player.isMoving(), "strafe to right");
        player.setStrafe(proofps_dd::Strafe::NONE);

        b &= assertFalse(player.isMoving(), "still on the ground 6");

        // something else is moving us
        player.getPos().set({ 1.f, 0.f, 0.f });
        b &= assertTrue(player.isMoving(), "old pos != new pos");
        player.getPos().commit();

        b &= assertFalse(player.isMoving(), "still on the ground 7");

        return b;
    }

    bool test_attack()
    {
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(12345), "192.168.1.12");
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
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
        const proofps_dd::MapItem miHealth(*m_engine, proofps_dd::MapItemType::ITEM_HEALTH, PureVector(1, 2, 3));

        bool b = assertFalse(player.canTakeItem(miHealth), "1");

        player.setHealth(50);
        b &= assertTrue(player.canTakeItem(miHealth), "2");

        return b;
    }

    bool test_can_take_item_weapon()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
        const proofps_dd::MapItem miPistol(*m_engine, proofps_dd::MapItemType::ITEM_WPN_PISTOL, PureVector(1, 2, 3));
        const proofps_dd::MapItem miMchgun(*m_engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        const proofps_dd::MapItem miBazooka(*m_engine, proofps_dd::MapItemType::ITEM_WPN_BAZOOKA, PureVector(1, 2, 3));
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
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
        const auto& playerConst = player;
        proofps_dd::MapItem miHealth(*m_engine, proofps_dd::MapItemType::ITEM_HEALTH, PureVector(1, 2, 3));
        pge_network::PgePacket newPktWpnUpdate;
        bool bHasJustBecomeAvailable = false;

        player.setHealth(90);
        player.takeItem(miHealth, newPktWpnUpdate, bHasJustBecomeAvailable);

        // ignoring bHasJustBecomeAvailable because that is not set for health item
        return (assertEquals(100, playerConst.getHealth(), "player health") &
            assertTrue(miHealth.isTaken(), "item taken")) != 0;
    }

    bool test_take_item_weapon()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
        proofps_dd::MapItem miPistol(*m_engine, proofps_dd::MapItemType::ITEM_WPN_PISTOL, PureVector(1, 2, 3));
        proofps_dd::MapItem miMchGun(*m_engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        proofps_dd::MapItem miBazooka(*m_engine, proofps_dd::MapItemType::ITEM_WPN_BAZOOKA, PureVector(1, 2, 3));
        pge_network::PgePacket pktWpnUpdate;
        bool bHasJustBecomeAvailable = false;
        
        // Warning: this way of pointing to message is valid only if there is only 1 message (the first) in the packet and we want that!
        const proofps_dd::MsgWpnUpdateFromServer& msgWpnUpdate = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgWpnUpdateFromServer>(pktWpnUpdate);
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };

        player.takeItem(miPistol, pktWpnUpdate, bHasJustBecomeAvailable);
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
                static_cast<int>(msgWpnUpdate.m_nUnmagBulletCount), "msg wpn 1 unmag") &
            assertFalse(bHasJustBecomeAvailable, "bHasJustBecomeAvailable 1")) != 0;

        bHasJustBecomeAvailable = false;
        player.takeItem(miMchGun, pktWpnUpdate, bHasJustBecomeAvailable);
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
                static_cast<int>(msgWpnUpdate.m_nUnmagBulletCount), "msg wpn 2 unmag") &
            assertTrue(bHasJustBecomeAvailable, "bHasJustBecomeAvailable 2");

        bHasJustBecomeAvailable = false;
        player.takeItem(miBazooka, pktWpnUpdate, bHasJustBecomeAvailable);
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
                static_cast<int>(msgWpnUpdate.m_nUnmagBulletCount), "msg wpn 3 unmag") &
            assertTrue(bHasJustBecomeAvailable, "bHasJustBecomeAvailable 3");

        return b;
    }

    bool test_get_weapon_instance_by_map_item_type()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleExpected = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player player(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleExpected, "192.168.1.12");
        
        if (!assertTrue(loadWeaponsForPlayer(player, SetDfltWpn::Yes)))
        {
            return false;
        };

        bool b = true;

        // negative tests
        b &= assertEquals(nullptr, player.getWeaponInstanceByMapItemType(proofps_dd::MapItemType::ITEM_HEALTH), "health") &
            assertEquals(nullptr, player.getWeaponInstanceByMapItemType(proofps_dd::MapItemType::ITEM_ARMOR), "armor");

        // positive tests
        b &= assertEquals(player.getWeaponManager().getWeaponByFilename("pistol.txt"), player.getWeaponInstanceByMapItemType(proofps_dd::MapItemType::ITEM_WPN_PISTOL), "pistol") &
            assertEquals(player.getWeaponManager().getWeaponByFilename("machinegun.txt"), player.getWeaponInstanceByMapItemType(proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN), "mchgun") &
            assertEquals(player.getWeaponManager().getWeaponByFilename("bazooka.txt"), player.getWeaponInstanceByMapItemType(proofps_dd::MapItemType::ITEM_WPN_BAZOOKA), "bazooka");

        return b;
    }

    bool test_handle_falling_from_high_server_instance()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleServer = pge_network::ServerConnHandle;
        const pge_network::PgeNetworkConnectionHandle connHandleClient = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player playerServer(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleServer, "192.168.1.11");
        proofps_dd::Player playerClient(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleClient, "192.168.1.12");

        bool b = true;
        // (I cannot check sound now, need stubs for that)
        playerServer.handleFallingFromHigh(0);
        b &= assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count 1");
        try
        {
            b &= assertEquals(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgPlayerEventFromServer::id)),
                "tx msg count 1"
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "no such tx msg found 2");
        }

        // (I cannot check sound now, need stubs for that)
        playerClient.handleFallingFromHigh(0);
        b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count 2");
        try
        {
            b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgPlayerEventFromServer::id)),
                "tx msg count 2"
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "no such tx msg found 1");
        }

        // calling again must not do anything, values should stay unchanged
        playerServer.handleFallingFromHigh(0);
        b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count 3");
        try
        {
            b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgPlayerEventFromServer::id)),
                "tx msg count 3"
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "no such tx msg found 2");
        }

        // calling again must not do anything, values should stay unchanged
        playerClient.handleFallingFromHigh(0);
        b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count 4");
        try
        {
            b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgPlayerEventFromServer::id)),
                "tx msg count 4"
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "no such tx msg found 2");
        }

        return b;
    }

    bool test_handle_falling_from_high_client_instance()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleClient = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);

        // client-only test
        m_network.shutdown();
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(false);
        if (!m_network.initialize())
        {
            return assertFalse(true, "network reinit as client");
        }

        proofps_dd::Player playerClient(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleClient, "192.168.1.12");
        bool b = true;

        // client never sends pkt from this function, and always plays sound (I cannot check sound now, need stubs for that)
        playerClient.handleFallingFromHigh(0);
        b &= assertEquals(0u, m_network.getServer().getTxPacketCount(), "tx pkt count 1");

        return b;
    }

    bool test_handle_landed_server_instance()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleServer = pge_network::ServerConnHandle;
        const pge_network::PgeNetworkConnectionHandle connHandleClient = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player playerServer(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleServer, "192.168.1.11");
        proofps_dd::Player playerClient(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleClient, "192.168.1.12");

        bool b = true;
        // (I cannot check sound now, need stubs for that)
        playerServer.handleLanded(1.f, false, false);
        b &= assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count 1");

        // (I cannot check sound now, need stubs for that)
        playerClient.handleLanded(1.f, false, false);
        b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count 2");
        try
        {
            b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgPlayerEventFromServer::id)),
                "tx msg count 1"
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "no such tx msg found 1");
        }

        // same actions as above, no protective flag is in place that needs to be cleared first
        playerClient.handleLanded(1.f, false, false);
        b &= assertEquals(3u, m_network.getServer().getTxPacketCount(), "tx pkt count 3");
        try
        {
            b &= assertEquals(3u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgPlayerEventFromServer::id)),
                "tx msg count 2"
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "no such tx msg found 2");
        }

        return b;
    }

    bool test_handle_landed_client_instance()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleClient = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);

        // client-only test
        m_network.shutdown();
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(false);
        if (!m_network.initialize())
        {
            return assertFalse(true, "network reinit as client");
        }

        proofps_dd::Player playerClient(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleClient, "192.168.1.12");
        bool b = true;

        // client never sends pkt from this function, and always plays sound (I cannot check sound now, need stubs for that)
        playerClient.handleLanded(1.f, false, false);
        b &= assertEquals(0u, m_network.getServer().getTxPacketCount(), "tx pkt count 1");

        return b;
    }

    bool test_handle_take_item_server_instance()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleServer = pge_network::ServerConnHandle;
        const pge_network::PgeNetworkConnectionHandle connHandleClient = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player playerServer(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleServer, "192.168.1.11");
        proofps_dd::Player playerClient(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleClient, "192.168.1.12");

        bool b = true;
        // sound play only (I cannot check sound now, need stubs for that), no pkt for server player
        playerServer.handleTakeNonWeaponItem(proofps_dd::MapItemType::ITEM_HEALTH);
        b &= assertEquals(0u, m_network.getServer().getTxPacketCount(), "tx pkt count 1");

        // no sound play (I cannot check sound now, need stubs for that), tx to client player
        playerClient.handleTakeNonWeaponItem(proofps_dd::MapItemType::ITEM_HEALTH);
        b &= assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count 2");
        try
        {
            b &= assertEquals(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgPlayerEventFromServer::id)),
                "tx msg count 1"
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "no such tx msg found 1");
        }

        // same actions as above, no protective flag is in place that needs to be cleared first
        playerClient.handleTakeNonWeaponItem(proofps_dd::MapItemType::ITEM_HEALTH);
        b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count 3");
        try
        {
            b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgPlayerEventFromServer::id)),
                "tx msg count 2"
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "no such tx msg found 2");
        }

        return b;
    }

    bool test_handle_take_item_client_instance()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleClient = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);

        // client-only test
        m_network.shutdown();
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(false);
        if (!m_network.initialize())
        {
            return assertFalse(true, "network reinit as client");
        }

        proofps_dd::Player playerClient(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleClient, "192.168.1.12");
        bool b = true;

        // client never sends pkt from this function, and always plays sound (I cannot check sound now, need stubs for that)
        playerClient.handleTakeNonWeaponItem(proofps_dd::MapItemType::ITEM_HEALTH);
        b &= assertEquals(0u, m_network.getServer().getTxPacketCount(), "tx pkt count 1");

        return b;
    }

    bool test_handle_take_weapon_item()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleServer = pge_network::ServerConnHandle;
        proofps_dd::Player playerServer(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleServer, "192.168.1.11");

        if (!assertTrue(loadWeaponsForPlayer(playerServer, SetDfltWpn::Yes)))
        {
            return false;
        };

        const Weapon* const wpnPistol = playerServer.getWeaponManager().getWeapons()[0];
        if (!assertNotNull(wpnPistol))
        {
            return false;
        }

        // for now we dont test anything, just invoke and expect not to crash
        playerServer.handleTakeWeaponItem(
            proofps_dd::MapItemType::ITEM_WPN_PISTOL, *wpnPistol, true, 3);

        return true;
    }

    bool test_handle_jumppad_activated_server()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleServer = pge_network::ServerConnHandle;
        const pge_network::PgeNetworkConnectionHandle connHandleClient = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);
        proofps_dd::Player playerServer(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleServer, "192.168.1.11");
        proofps_dd::Player playerClient(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleClient, "192.168.1.12");

        bool b = true;
        // sound play only (I cannot check sound now, need stubs for that), no pkt for server player
        playerServer.handleJumppadActivated();
        b &= assertEquals(0u, m_network.getServer().getTxPacketCount(), "tx pkt count 1");

        // no sound play (I cannot check sound now, need stubs for that), tx to client player
        playerClient.handleJumppadActivated();
        b &= assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count 2");
        try
        {
            b &= assertEquals(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgPlayerEventFromServer::id)),
                "tx msg count 1"
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "no such tx msg found 1");
        }

        // same actions as above, no protective flag is in place that needs to be cleared first
        playerClient.handleJumppadActivated();
        b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count 3");
        try
        {
            b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgPlayerEventFromServer::id)),
                "tx msg count 2"
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "no such tx msg found 2");
        }

        return b;
    }

    bool test_handle_jumppad_activated_client()
    {
        const pge_network::PgeNetworkConnectionHandle connHandleClient = static_cast<pge_network::PgeNetworkConnectionHandle>(12345);

        // client-only test
        m_network.shutdown();
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(false);
        if (!m_network.initialize())
        {
            return assertFalse(true, "network reinit as client");
        }

        proofps_dd::Player playerClient(m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_ammoChangeEvents, *m_engine, m_network, connHandleClient, "192.168.1.12");
        bool b = true;

        // client never sends pkt from this function, and always plays sound (I cannot check sound now, need stubs for that)
        playerClient.handleJumppadActivated();
        b &= assertEquals(0u, m_network.getServer().getTxPacketCount(), "tx pkt count 1");

        return b;
    }

};
