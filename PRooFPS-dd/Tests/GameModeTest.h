#pragma once

/*
    ###################################################################################
    GameModeTest.h
    Unit test for PRooFPS-dd GameMode.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include <list>
#include <thread>

#include "UnitTests/UnitTest.h"

#include "PGE.h" // for Bullet and PgeCfgProfiles
#include "Network/Stubs/PgeNetworkStub.h"

#include "GameMode.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"


class GameModeTest :
    public UnitTest
{
public:

    GameModeTest(PGEcfgProfiles& cfgProfiles) :
        UnitTest(__FILE__),
        gm(nullptr),
        dm(nullptr),
        m_audio(cfgProfiles),
        m_cfgProfiles(cfgProfiles),
        m_itemPickupEvents(8 /* time limit secs */, 5 /* event count limit */),
        m_ammoChangeEvents(8 /* time limit secs */, 5 /* event count limit */, proofps_dd::EventLister::Orientation::Horizontal),
        m_engine(nullptr),
        m_network(cfgProfiles)
    {
    }

    GameModeTest(const GameModeTest&) = delete;
    GameModeTest& operator=(const GameModeTest&) = delete;
    GameModeTest(GameModeTest&&) = delete;
    GameModeTest&& operator=(GameModeTest&&) = delete;

protected:

    virtual void Initialize() override
    {
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::GameMode::getLoggerModuleName(), true);

        m_audio.initialize();

        PGEInputHandler& inputHandler = PGEInputHandler::createAndGet(m_cfgProfiles);

        m_engine = &PR00FsUltimateRenderingEngine::createAndGet(m_cfgProfiles, inputHandler);
        m_engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        AddSubTest("test_factory_creates_deathmatch_only", (PFNUNITSUBTEST)&GameModeTest::test_factory_creates_deathmatch_only);
        AddSubTest("test_restart_updates_times", (PFNUNITSUBTEST)&GameModeTest::test_restart_updates_times);
        AddSubTest("test_rename_player", (PFNUNITSUBTEST)&GameModeTest::test_rename_player);
        AddSubTest("test_deathmatch_time_limit_get_set_and_remaining_time_get", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_time_limit_get_set_and_remaining_time_get);
        AddSubTest("test_deathmatch_time_limit_client_update_time_remaining_secs", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_time_limit_client_update_time_remaining_secs);
        AddSubTest("test_deathmatch_frag_limit_get_set", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_frag_limit_get_set);
        AddSubTest("test_deathmatch_fetch_config", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_fetch_config);
        AddSubTest("test_deathmatch_add_player_zero_values_maintains_adding_order", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_add_player_zero_values_maintains_adding_order);
        AddSubTest("test_deathmatch_add_player_random_values", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_add_player_random_values);
        AddSubTest("test_deathmatch_add_player_already_existing_fails", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_add_player_already_existing_fails);
        AddSubTest("test_deathmatch_add_player_sends_winning_state_only_when_game_is_already_won", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_add_player_sends_winning_state_only_when_game_is_already_won);
        AddSubTest("test_deathmatch_update_player", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_update_player);
        AddSubTest("test_deathmatch_update_player_non_existing_fails", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_update_player_non_existing_fails);
        AddSubTest("test_deathmatch_remove_player", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_remove_player);
        AddSubTest("test_deathmatch_restart", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_restart);
        AddSubTest("test_deathmatch_restart_without_removing_players", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_restart_without_removing_players);
        AddSubTest("test_deathmatch_winning_cond_defaults_to_false", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_defaults_to_false);
        AddSubTest("test_deathmatch_winning_cond_time_limit", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_time_limit);
        AddSubTest("test_deathmatch_winning_cond_frag_limit", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_frag_limit);
        AddSubTest("test_deathmatch_winning_cond_time_and_frag_limit", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_time_and_frag_limit);
        AddSubTest("test_deathmatch_receive_and_update_winning_conditions_client", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_receive_and_update_winning_conditions_client);
        
    }

    virtual bool setUp() override
    {
        m_itemPickupEvents.clear();

        bool b = assertTrue(m_engine && m_engine->isInitialized(), "engine inited");

        if (b)
        {
            m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(false);
            b &= assertTrue(m_network.initialize(), "network inited");
        }

        return b;
    }

    virtual void TearDown() override
    {
        if (gm)
        {
            delete gm;
            gm = nullptr;
            dm = nullptr;
        }
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

        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::GameMode::getLoggerModuleName(), false);
    }

    bool testInitDeathmatch()
    {
        gm = proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType::DeathMatch);
        bool b = assertNotNull(gm, "gm null");
        if (b)
        {
            dm = dynamic_cast<proofps_dd::DeathMatchMode*>(gm);
            b &= assertNotNull(dm, "dm null");
        }
        return b;
    }

private:

    proofps_dd::GameMode* gm;
    proofps_dd::DeathMatchMode* dm;
    pge_audio::PgeAudio m_audio;
    PGEcfgProfiles& m_cfgProfiles;
    std::list<Bullet> m_bullets;
    proofps_dd::EventLister m_itemPickupEvents;
    proofps_dd::EventLister m_ammoChangeEvents;
    PR00FsUltimateRenderingEngine* m_engine;
    pge_network::PgeNetworkStub m_network;

    // ---------------------------------------------------------------------------

    bool assertFragTableEquals(
        const std::vector<proofps_dd::FragTableRow>& expectedPlayers,
        const std::list<proofps_dd::FragTableRow>& fragTable,
        const std::string& sLogText = "")
    {
        bool b = assertEquals(expectedPlayers.size(), fragTable.size(), ("size " + sLogText).c_str());
        if (b)
        {
            auto itFragTable = fragTable.begin();
            auto itExpectedPlayers = expectedPlayers.begin();
            int i = 0;
            while ((itFragTable != fragTable.end()) && (itExpectedPlayers != expectedPlayers.end()))
            {
                i++;
                b &= assertEquals(itExpectedPlayers->m_sName, itFragTable->m_sName, ("Names in row " + std::to_string(i) + " " + sLogText).c_str());
                b &= assertEquals(itExpectedPlayers->m_nFrags, itFragTable->m_nFrags, ("Frags in row " + std::to_string(i) + " " + sLogText).c_str());
                b &= assertEquals(itExpectedPlayers->m_nDeaths, itFragTable->m_nDeaths, ("Death in row " + std::to_string(i) + " " + sLogText).c_str());
                itFragTable++;
                itExpectedPlayers++;
            }
        }
        return b;
    }

    bool test_factory_creates_deathmatch_only()
    {
        if (!testInitDeathmatch())
        {
            return assertFalse(true, "testInitDeathmatch fail");
        }

        bool b = assertTrue(proofps_dd::GameModeType::DeathMatch == gm->getGameModeType(), "gmtype");
        b &= assertNull(proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType::TeamDeathMatch), "tdm null");
        b &= assertNull(proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType::TeamRoundGame), "trg null");
        b &= assertEquals(0, gm->getResetTime().time_since_epoch().count(), "reset time is epoch");
        b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time is epoch");
        b &= assertFalse(gm->isGameWon(), "game not won");
        b &= assertTrue(dm->getFragTable().empty(), "playerdata");

        return b;
    }

    bool test_restart_updates_times()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            gm->restart(m_network);
            b &= (assertLess(0, gm->getResetTime().time_since_epoch().count(), (std::string("reset time fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str()) &
                assertEquals(0, gm->getWinTime().time_since_epoch().count(), (std::string("win time fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str())) != 0;
        }

        return b;
    }

    bool test_rename_player()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            dm->setFragLimit(11);

            b &= assertFalse(gm->renamePlayer("alma", "gg"), (std::string("rename 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(gm->renamePlayer("", ""), (std::string("rename 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 2;
            player1.getDeaths() = 0;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 1;
            player2.getDeaths() = 0;

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 0;
            player3.getDeaths() = 0;

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 0;
            player4.getDeaths() = 0;

            b &= assertTrue(dm->addPlayer(player1, m_network), (std::string("add player 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player2, m_network), (std::string("add player 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player3, m_network), (std::string("add player 3 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player4, m_network), (std::string("add player 4 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            b &= assertFalse(gm->renamePlayer("", ""), (std::string("rename 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(gm->renamePlayer("Adam", ""), (std::string("rename 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(gm->renamePlayer("", "Adam"), (std::string("rename 3 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(gm->renamePlayer("gg", "kkk"), (std::string("rename 4 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(gm->renamePlayer("Adam", "Joe"), (std::string("rename 5 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(gm->renamePlayer("Joe", "Adam"), (std::string("rename 6 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(gm->renamePlayer("adam", "Peter"), (std::string("rename 7 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(gm->renamePlayer("Adam", "Peter"), (std::string("rename 8 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            const std::vector<proofps_dd::FragTableRow> expectedPlayers = {
                { /*"Adam"*/ "Peter", 2, 0 },
                { "Apple", 1, 0 },
                { "Joe", 0, 0 },
                { "Banana", 0, 0 }
            };

            b &= assertFragTableEquals(expectedPlayers, dm->getFragTable());

        }

        return b;
    }

    bool test_deathmatch_time_limit_get_set_and_remaining_time_get()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            b &= (assertEquals(0u, dm->getTimeLimitSecs(), (std::string("default time limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str()) &
                assertEquals(0u, dm->getTimeRemainingMillisecs(), (std::string("remaining default fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str())) != 0;

            dm->setTimeLimitSecs(25u);
            dm->restart(m_network);  // restart() is needed to have correct value for remaining time
            b &= assertEquals(25u, dm->getTimeLimitSecs(), (std::string("new time limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str()) &
                /* 23000 millisecs so there is a 2 seconds time window for evaluating the condition (in case scheduler or anything would cause too much delay here) */
                assertLequals(23000u, dm->getTimeRemainingMillisecs(), (std::string("new remaining fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
        }

        return b;
    }

    bool test_deathmatch_time_limit_client_update_time_remaining_secs()
    {
        // client-only test
        if (!testInitDeathmatch())
        {
            return assertFalse(true, "testInitDeathmatch fail");
        }

        constexpr unsigned int nTimeLimitSecs = 13u;
        constexpr unsigned int nTimeRemainingServerSideMillisecs = 4000u;

        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvDmTimeLimit].Set(nTimeLimitSecs);
        dm->fetchConfig(m_cfgProfiles, m_network);
        dm->clientUpdateTimeRemainingMillisecs(nTimeRemainingServerSideMillisecs, m_network);

        bool b = true;

        // positive case
        b &= assertEquals(nTimeLimitSecs, dm->getTimeLimitSecs(), "time limit") &
            assertLequals(dm->getTimeRemainingMillisecs(), nTimeRemainingServerSideMillisecs, "remaining 1 a") &
            assertNotEquals(0u, dm->getTimeRemainingMillisecs(), "remaining 1 b");

        // negative case: remaining time as seconds is bigger than time limit as seconds
        dm->clientUpdateTimeRemainingMillisecs((nTimeLimitSecs + 1u) * 1000, m_network);
        // (nTimeLimitSecs-2) so that there is a 2 seconds time window for evaluating the condition (in case scheduler or anything would cause too much delay here)
        b &= assertLequals((nTimeLimitSecs-2) * 1000, dm->getTimeRemainingMillisecs(), "remaining 2");

        return b;
    }

    bool test_deathmatch_frag_limit_get_set()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            b &= assertEquals(0u, dm->getFragLimit(), (std::string("default fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            dm->setFragLimit(25u);
            b &= assertEquals(25u, dm->getFragLimit(), (std::string("new fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
        }

        return b;
    }

    bool test_deathmatch_fetch_config()
    {
        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvDmFragLimit].Set(25);
        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvDmTimeLimit].Set(13);

        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            b &= (assertEquals(0u, dm->getFragLimit(), (std::string("default frag limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str()) &
                assertEquals(0u, dm->getTimeLimitSecs(), (std::string("default time limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str()));

            dm->fetchConfig(m_cfgProfiles, m_network);
            dm->restart(m_network);  // restart() is needed to have correct value for remaining time

            b &= assertEquals(25u, dm->getFragLimit(), (std::string("new frag limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str()) &
                assertEquals(13u, dm->getTimeLimitSecs(), (std::string("new time limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str()) &
                /* 11000 millisecs so there is a 2 seconds time window for evaluating the condition (in case scheduler or anything would cause too much delay here) */
                assertLequals(11000u, dm->getTimeRemainingMillisecs(), (std::string("new remaining fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
        }

        return b;
    }

    bool test_deathmatch_add_player_zero_values_maintains_adding_order()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            dm->setFragLimit(11);

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 0;
            player1.getDeaths() = 0;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 0;
            player2.getDeaths() = 0;

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 0;
            player3.getDeaths() = 0;

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 0;
            player4.getDeaths() = 0;

            b &= assertTrue(dm->addPlayer(player1, m_network), (std::string("add player 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player2, m_network), (std::string("add player 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player3, m_network), (std::string("add player 3 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player4, m_network), (std::string("add player 4 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            const std::vector<proofps_dd::FragTableRow> expectedPlayers = {
                { "Adam", 0, 0 },
                { "Apple", 0, 0 },
                { "Joe", 0, 0 },
                { "Banana", 0, 0 }
            };

            b &= assertFragTableEquals(expectedPlayers, dm->getFragTable(), std::string("table fail, testing as ") + (bTestingAsServer ? "server" : "client"));
        }

        return b;
    }

    bool test_deathmatch_add_player_random_values()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            dm->setFragLimit(11);

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 10;
            player1.getDeaths() = 0;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 5;
            player2.getDeaths() = 2;

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 8;
            player3.getDeaths() = 2;

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 8;
            player4.getDeaths() = 0;

            b &= assertTrue(dm->addPlayer(player1, m_network), (std::string("add player 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player2, m_network), (std::string("add player 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player3, m_network), (std::string("add player 3 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player4, m_network), (std::string("add player 4 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            const std::vector<proofps_dd::FragTableRow> expectedPlayers = {
                { "Adam", 10, 0 },
                { "Banana", 8, 0 },
                { "Joe", 8, 2 },
                { "Apple", 5, 2 }
            };

            b &= assertFragTableEquals(expectedPlayers, dm->getFragTable(), std::string("table fail, testing as ") + (bTestingAsServer ? "server" : "client"));
        }

        return b;
    }

    bool test_deathmatch_add_player_already_existing_fails()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            dm->setFragLimit(11);

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 10;
            player1.getDeaths() = 0;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 5;
            player2.getDeaths() = 2;

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 8;
            player3.getDeaths() = 2;

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 8;
            player4.getDeaths() = 0;

            b &= assertTrue(dm->addPlayer(player1, m_network), (std::string("add player 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player2, m_network), (std::string("add player 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player3, m_network), (std::string("add player 3 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(player4, m_network), (std::string("add player 4 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            player3.setName("Joe");
            player3.getFrags() = 12;
            player3.getDeaths() = 0;
            b &= assertFalse(dm->addPlayer(player3, m_network), (std::string("add player 3 again, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            const std::vector<proofps_dd::FragTableRow> expectedPlayers = {
                { "Adam", 10, 0 },
                { "Banana", 8, 0 },
                { "Joe", 8, 2 },
                { "Apple", 5, 2 }
            };

            b &= assertFragTableEquals(expectedPlayers, dm->getFragTable(), std::string("table fail, testing as ") + (bTestingAsServer ? "server" : "client"));
        }

        return b;
    }

    bool test_deathmatch_add_player_sends_winning_state_only_when_game_is_already_won()
    {
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalse(true, "network reinit as server");
        }

        if (!testInitDeathmatch())
        {
            return assertFalse(true, "testInitDeathmatch fail");
        }

        bool b = true;

        dm->setFragLimit(11);

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 10;
        player1.getDeaths() = 0;

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 5;
        player2.getDeaths() = 2;

        b &= assertTrue(dm->addPlayer(player1, m_network), "add player 1 fail");

        // initially there is no sent out packets because game is NOT yet won
        b &= assertEquals(0u, m_network.getServer().getTxPacketCount(), "tx pkt count");

        // adding same player fails so still no sent out packets
        b &= assertFalse(dm->addPlayer(player1, m_network), "add player 1 again fail 1");
        b &= assertEquals(0u, m_network.getServer().getTxPacketCount(), "tx pkt count");

        // game is now won, expecting 1 sent pkts to the virtually connected client (ServerStub has 1 virtual always-connected client)
        player1.getFrags()++;
        b &= assertTrue(dm->updatePlayer(player1, m_network), "update player 1 fail");
        b &= assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count");
        try
        {
            b &= assertEquals(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "tx msg count");
        }

        // adding same player fails so number of sent pkts should not change
        b &= assertFalse(dm->addPlayer(player1, m_network), "add player 1 again fail 2");
        b &= assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count");

        // now adding new player should trigger sending out MsgGameSessionStateFromServer to the player since game state is already won
        b &= assertTrue(dm->addPlayer(player2, m_network), "add player 2 fail");
        b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count");
        try
        {
            b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "tx msg count");
        }

        return b;
    }

    bool test_deathmatch_update_player()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            dm->setFragLimit(11);

            proofps_dd::Player playerAdam(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            playerAdam.setName("Adam");
            playerAdam.getFrags() = 10;
            playerAdam.getDeaths() = 0;

            proofps_dd::Player playerApple(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            playerApple.setName("Apple");
            playerApple.getFrags() = 5;
            playerApple.getDeaths() = 2;

            proofps_dd::Player playerJoe(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            playerJoe.setName("Joe");
            playerJoe.getFrags() = 4;
            playerJoe.getDeaths() = 2;

            b &= assertTrue(dm->addPlayer(playerAdam, m_network), (std::string("add player Adam fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(playerApple, m_network), (std::string("add player Apple fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(dm->addPlayer(playerJoe, m_network), (std::string("add player Joe fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            playerJoe.getFrags()++;
            b &= assertTrue(dm->updatePlayer(playerJoe, m_network), (std::string("update player Joe 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            // since Joe got same number of frags _later_ than Apple, Joe must stay behind Apple
            const std::vector<proofps_dd::FragTableRow> expectedPlayers1 = {
                { "Adam", 10, 0 },
                { "Apple", 5, 2 },
                { "Joe", 5, 2 }
            };
            b &= assertFragTableEquals(expectedPlayers1, dm->getFragTable(), std::string("table 1 fail, testing as ") + (bTestingAsServer ? "server" : "client"));

            playerApple.getDeaths()++;
            b &= assertTrue(dm->updatePlayer(playerApple, m_network), (std::string("update player Apple 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            // since Apple now has more deaths than Joe, it must goe behind Joe
            const std::vector<proofps_dd::FragTableRow> expectedPlayers2 = {
                { "Adam", 10, 0 },
                { "Joe", 5, 2 },
                { "Apple", 5, 3 }
            };
            b &= assertFragTableEquals(expectedPlayers2, dm->getFragTable(), std::string("table 2 fail, testing as ") + (bTestingAsServer ? "server" : "client"));

            playerJoe.getDeaths()++;
            b &= assertTrue(dm->updatePlayer(playerJoe, m_network), (std::string("update player Joe 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            // since Joe got same number of frags _earlier_ than Apple, and got same number for deaths _later_ than Apple, it must stay in front of Apple
            const std::vector<proofps_dd::FragTableRow> expectedPlayers3 = {
                { "Adam", 10, 0 },
                { "Joe", 5, 3 },
                { "Apple", 5, 3 }
            };
            b &= assertFragTableEquals(expectedPlayers3, dm->getFragTable(), std::string("table 3 fail, testing as ") + (bTestingAsServer ? "server" : "client"));

            playerAdam.getFrags()++;
            b &= assertTrue(dm->updatePlayer(playerAdam, m_network), (std::string("update player Adam 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            if (bTestingAsServer)
            {
                // game won, win time is already updated by updatePlayer() even before explicit call to serverCheckAndUpdateWinningConditions();
                // this is known only by server, client needs to be informed by server
                b &= assertTrue(gm->isGameWon(), "game won");
                b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), (std::string("win time fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
                b &= assertTrue(dm->serverCheckAndUpdateWinningConditions(m_network), "winning server");
                b &= assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count");
                try
                {
                    b &= assertEquals(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count");
                }
            }

        }

        return b;
    }

    bool test_deathmatch_update_player_non_existing_fails()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            dm->setFragLimit(10);

            proofps_dd::Player playerAdam(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            playerAdam.setName("Adam");
            playerAdam.getFrags() = 10;
            playerAdam.getDeaths() = 0;

            b &= assertFalse(dm->updatePlayer(playerAdam, m_network), (std::string("update player Adam 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            if (bTestingAsServer)
            {
                b &= assertFalse(gm->isGameWon(), "game not won");
                b &= assertFalse(dm->serverCheckAndUpdateWinningConditions(m_network), "winning");
                b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time");
            }

            const std::vector<proofps_dd::FragTableRow> expectedPlayers1;
            b &= assertFragTableEquals(expectedPlayers1, dm->getFragTable(), std::string("table 1 fail, testing as ") + (bTestingAsServer ? "server" : "client"));

            b &= assertTrue(dm->addPlayer(playerAdam, m_network), (std::string("add player Adam fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            proofps_dd::Player playerJoe(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            playerJoe.setName("Joe");
            playerJoe.getFrags() = 4;
            playerJoe.getDeaths() = 2;
            b &= assertFalse(dm->updatePlayer(playerJoe, m_network), (std::string("update player Joe 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            const std::vector<proofps_dd::FragTableRow> expectedPlayers2 = {
                { "Adam", 10, 0 }
            };
            b &= assertFragTableEquals(expectedPlayers2, dm->getFragTable(), std::string("table 2 fail, testing as ") + (bTestingAsServer ? "server" : "client"));

        }

        return b;
    }

    bool test_deathmatch_remove_player()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            dm->setFragLimit(10);

            proofps_dd::Player playerAdam(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            playerAdam.setName("Adam");
            playerAdam.getFrags() = 10;
            playerAdam.getDeaths() = 0;

            b &= assertFalse(dm->removePlayer(playerAdam), (std::string("removep player Adam 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            const std::vector<proofps_dd::FragTableRow> expectedPlayers1;
            b &= assertFragTableEquals(expectedPlayers1, dm->getFragTable(), std::string("table 1 fail, testing as ") + (bTestingAsServer ? "server" : "client"));

            b &= assertTrue(dm->addPlayer(playerAdam, m_network), (std::string("add player Adam fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            proofps_dd::Player playerJoe(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            playerJoe.setName("Joe");
            playerJoe.getFrags() = 4;
            playerJoe.getDeaths() = 2;
            b &= assertFalse(dm->removePlayer(playerJoe), (std::string("remove player Joe 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            const std::vector<proofps_dd::FragTableRow> expectedPlayers2 = {
                { "Adam", 10, 0 }
            };
            b &= assertFragTableEquals(expectedPlayers2, dm->getFragTable(), std::string("table 2 fail, testing as ") + (bTestingAsServer ? "server" : "client"));

            b &= assertTrue(dm->removePlayer(playerAdam), (std::string("remove player Adam 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFragTableEquals(expectedPlayers1, dm->getFragTable(), std::string("table 3 fail, testing as ") + (bTestingAsServer ? "server" : "client"));

            if (bTestingAsServer)
            {
                // even though winner player is removed, winning condition stays true, win time is still valid, an explicit reset() would be needed to clear them!
                b &= assertTrue(gm->isGameWon(), "game won");
                b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time");
                b &= assertTrue(dm->serverCheckAndUpdateWinningConditions(m_network), "winning");
            }

        }

        return b;
    }

    bool test_deathmatch_restart()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            dm->setTimeLimitSecs(25u);
            dm->setFragLimit(15u);

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 15;
            player1.getDeaths() = 0;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 5;
            player2.getDeaths() = 2;

            b &= assertTrue(dm->addPlayer(player1, m_network), (std::string("add player 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            if (bTestingAsServer)
            {
                b &= assertTrue(gm->isGameWon(), "game won 1");
                b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time");
                b &= assertTrue(dm->serverCheckAndUpdateWinningConditions(m_network), "winning 1");
                b &= assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count");
                try
                {
                    b &= assertEquals(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count");
                }
            }

            b &= assertTrue(dm->addPlayer(player2, m_network), (std::string("add player 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            if (bTestingAsServer)
            {
                // in case of server instance, addPlayer() sends MsgGameSessionStateFromServer to newly added player when isGameWon() is true 
                b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count");
                try
                {
                    b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count");
                }
            }

            dm->restart(m_network);

            b &= assertEquals(25u, dm->getTimeLimitSecs(), (std::string("time limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertEquals(15u, dm->getFragLimit(), (std::string("frag limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            if (bTestingAsServer)
            {
                b &= assertFalse(gm->isGameWon(), "game won 2");
                b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time");
                b &= assertLess(0, gm->getResetTime().time_since_epoch().count(), "reset time");
                b &= assertFalse(dm->serverCheckAndUpdateWinningConditions(m_network), "winning 2");

                // outgoing packet for winning state true -> false transition too
                b &= assertEquals(3u, m_network.getServer().getTxPacketCount(), "tx pkt count");
                try
                {
                    b &= assertEquals(3u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count");
                }
            }
            b &= assertTrue(dm->getFragTable().empty(), (std::string("players empty fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

        }

        return b;
    }

    bool test_deathmatch_restart_without_removing_players()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                TearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            if (!testInitDeathmatch())
            {
                return assertFalse(true, (std::string("testInitDeathmatch fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

            dm->setTimeLimitSecs(25u);
            dm->setFragLimit(15u);

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 15;
            player1.getDeaths() = 0;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 5;
            player2.getDeaths() = 2;

            b &= assertTrue(dm->addPlayer(player1, m_network), (std::string("add player 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            if (bTestingAsServer)
            {
                b &= assertTrue(gm->isGameWon(), "game won 1");
                b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time");
                b &= assertTrue(dm->serverCheckAndUpdateWinningConditions(m_network), "winning 1");
                b &= assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count");
                try
                {
                    b &= assertEquals(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count");
                }
            }

            b &= assertTrue(dm->addPlayer(player2, m_network), (std::string("add player 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            if (bTestingAsServer)
            {
                // in case of server instance, addPlayer() sends MsgGameSessionStateFromServer to newly added player when isGameWon() is true 
                b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count");
                try
                {
                    b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count");
                }
            }

            dm->restartWithoutRemovingPlayers(m_network);

            b &= assertEquals(25u, dm->getTimeLimitSecs(), (std::string("time limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertEquals(15u, dm->getFragLimit(), (std::string("frag limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            if (bTestingAsServer)
            {
                b &= assertFalse(gm->isGameWon(), "game won 2");
                b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time");
                b &= assertLess(0, gm->getResetTime().time_since_epoch().count(), "reset time");
                b &= assertFalse(dm->serverCheckAndUpdateWinningConditions(m_network), "winning 2");

                // outgoing packet for winning state true -> false transition too
                b &= assertEquals(3u, m_network.getServer().getTxPacketCount(), "tx pkt count");
                try
                {
                    b &= assertEquals(3u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count");
                }
            }
            b &= assertEquals(2u, dm->getFragTable().size(), (std::string("players still there fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            for (const auto& player : dm->getFragTable())
            {
                b &= assertEquals(0, player.m_nFrags, (std::string("players frags 0 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
                b &= assertEquals(0, player.m_nDeaths, (std::string("players deaths 0 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            }

        }

        return b;
    }

    bool test_deathmatch_winning_cond_defaults_to_false()
    {
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalse(true, "network reinit as server");
        }

        if (!testInitDeathmatch())
        {
            return assertFalse(true, "testInitDeathmatch fail");
        }
        
        return assertFalse(dm->serverCheckAndUpdateWinningConditions(m_network));
    }

    bool test_deathmatch_winning_cond_time_limit()
    {
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalse(true, "network reinit as server");
        }

        if (!testInitDeathmatch())
        {
            return assertFalse(true, "testInitDeathmatch fail");
        }

        dm->setTimeLimitSecs(2);
        dm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        bool b = assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count");
        try
        {
            b &= assertEquals(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "tx msg count");
        }

        std::set<unsigned int> setRemainingSecs = {0, 1};
        int iSleep = 0;
        while ((iSleep++ < 5) && !dm->serverCheckAndUpdateWinningConditions(m_network))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            setRemainingSecs.erase( static_cast<unsigned int>(std::floor(dm->getTimeRemainingMillisecs() / 1000.f)) );
        }
        const auto durationSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - dm->getResetTime());
        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time");
        b &= assertTrue(gm->isGameWon(), "game won");
        b &= assertTrue(dm->serverCheckAndUpdateWinningConditions(m_network), "winning");
        b &= assertLequals(dm->getTimeLimitSecs(), durationSecs.count(), "time limit elapsed");
        b &= assertTrue(setRemainingSecs.empty(), "no remaining");

        b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count");
        try
        {
            b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "tx msg count");
        }

        return b;
    }

    bool test_deathmatch_winning_cond_frag_limit()
    {
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalse(true, "network reinit as server");
        }

        if (!testInitDeathmatch())
        {
            return assertFalse(true, "testInitDeathmatch fail");
        }

        dm->setFragLimit(5);
        dm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        bool b = assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count");
        try
        {
            b &= assertEquals(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "tx msg count");
        }

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Adam");
        player2.getFrags() = 2;
        player2.getDeaths() = 0;

        b &= assertTrue(dm->addPlayer(player1, m_network), "add player 1");
        b &= assertTrue(dm->addPlayer(player2, m_network), "add player 2");

        unsigned int i = 0;
        while (!dm->serverCheckAndUpdateWinningConditions(m_network) && (i++ < 5))
        {
            player1.getFrags()++;
            b &= assertTrue(dm->updatePlayer(player1, m_network), "update player");
        }

        b &= assertTrue(gm->isGameWon(), "game won 2");
        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time");
        b &= assertTrue(dm->serverCheckAndUpdateWinningConditions(m_network), "winning");
        b &= assertEquals(dm->getFragLimit(), i, "frags collected");

        b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count");
        try
        {
            b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "tx msg count");
        }

        return b;
    }

    bool test_deathmatch_winning_cond_time_and_frag_limit()
    {
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalse(true, "network reinit as server");
        }

        if (!testInitDeathmatch())
        {
            return assertFalse(true, "testInitDeathmatch fail");
        }

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 2;
        player2.getDeaths() = 0;

        dm->setFragLimit(5);
        dm->setTimeLimitSecs(2);
        dm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        bool b = assertEquals(1u, m_network.getServer().getTxPacketCount(), "tx pkt count");
        try
        {
            b &= assertEquals(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "tx msg count");
        }

        b &= assertTrue(dm->addPlayer(player1, m_network), "add player 1");
        b &= assertTrue(dm->addPlayer(player2, m_network), "add player 2");

        // time limit elapse also means winning even if frag limit not reached
        std::set<unsigned int> setRemainingSecs = { 0, 1 };
        int iSleep = 0;
        while ((iSleep++ < 5) && !dm->serverCheckAndUpdateWinningConditions(m_network))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            setRemainingSecs.erase(static_cast<unsigned int>(std::floor(dm->getTimeRemainingMillisecs() / 1000.f)));
        }
        b &= assertTrue(gm->isGameWon(), "game won 1");
        const auto durationSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - dm->getResetTime());
        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time 1");
        b &= assertTrue(dm->serverCheckAndUpdateWinningConditions(m_network), "winning due to time");
        b &= assertLequals(dm->getTimeLimitSecs(), durationSecs.count(), "time limit elapsed");
        b &= assertTrue(setRemainingSecs.empty(), "no remaining");

        b &= assertEquals(2u, m_network.getServer().getTxPacketCount(), "tx pkt count");
        try
        {
            b &= assertEquals(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "tx msg count");
        }

        // frag limit reach also means winning even if time limit not reached
        dm->setTimeLimitSecs(100);
        dm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        b &= assertEquals(3u, m_network.getServer().getTxPacketCount(), "tx pkt count");
        try
        {
            b &= assertEquals(3u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "tx msg count");
        }

        b &= assertFalse(gm->isGameWon(), "game won 2");
        b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time 2");

        b &= assertTrue(dm->addPlayer(player1, m_network), "add player 1");
        b &= assertTrue(dm->addPlayer(player2, m_network), "add player 2");

        unsigned int i = 0;
        while (!dm->serverCheckAndUpdateWinningConditions(m_network) && (i++ < 5))
        {
            player1.getFrags()++;
            b &= assertTrue(dm->updatePlayer(player1, m_network), "update player");
        }
        b &= assertTrue(gm->isGameWon(), "game won 3");
        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time 3");
        b &= assertTrue(dm->serverCheckAndUpdateWinningConditions(m_network), "winning due to frags");
        b &= assertEquals(dm->getFragLimit(), i, "frags collected");

        b &= assertEquals(4u, m_network.getServer().getTxPacketCount(), "tx pkt count");
        try
        {
            b &= assertEquals(4u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id))
            );
        }
        catch (...)
        {
            b &= assertFalse(true, "tx msg count");
        }

        return b;
    }

    bool test_deathmatch_receive_and_update_winning_conditions_client()
    {
        // client-only test
        if (!testInitDeathmatch())
        {
            return assertFalse(true, "testInitDeathmatch fail");
        }

        bool b = assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time 1");
        b &= assertFalse(dm->isGameWon(), "winning state 1");

        dm->clientReceiveAndUpdateWinningConditions(m_network, true);

        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time 2");
        b &= assertTrue(dm->isGameWon(), "winning state 2");

        dm->clientReceiveAndUpdateWinningConditions(m_network, false);

        b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time 3");
        b &= assertFalse(dm->isGameWon(), "winning state 3");

        return b;
    }

};