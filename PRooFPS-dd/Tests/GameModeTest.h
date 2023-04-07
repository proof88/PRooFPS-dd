#pragma once

/*
    ###################################################################################
    GameModeTest.h
    Unit test for PRooFPS-dd GameMode.
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <thread>

#include "../../../../PGE/PGE/UnitTests/UnitTest.h"
#include "../GameMode.h"

class GameModeTest :
    public UnitTest
{
public:

    GameModeTest(PGEcfgProfiles& cfgProfiles) :
        UnitTest(__FILE__),
        gm(nullptr),
        dm(nullptr),
        m_cfgProfiles(cfgProfiles),
        m_engine(nullptr)
    {
    }

protected:

    virtual void Initialize() override
    {
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::GameMode::getLoggerModuleName(), true);

        PGEInputHandler& inputHandler = PGEInputHandler::createAndGet(m_cfgProfiles);

        m_engine = &PR00FsUltimateRenderingEngine::createAndGet(m_cfgProfiles, inputHandler);
        m_engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        AddSubTest("test_factory_creates_deathmatch_only", (PFNUNITSUBTEST)&GameModeTest::test_factory_creates_deathmatch_only);
        AddSubTest("test_reset_updates_times", (PFNUNITSUBTEST)&GameModeTest::test_reset_updates_times);
        AddSubTest("test_deathmatch_time_limit_get_set_and_remaining_time_get", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_time_limit_get_set_and_remaining_time_get);
        AddSubTest("test_deathmatch_frag_limit_get_set", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_frag_limit_get_set);
        AddSubTest("test_deathmatch_add_player_zero_values_maintains_adding_order", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_add_player_zero_values_maintains_adding_order);
        AddSubTest("test_deathmatch_add_player_random_values", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_add_player_random_values);
        AddSubTest("test_deathmatch_add_player_already_existing_fails", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_add_player_already_existing_fails);
        AddSubTest("test_deathmatch_update_player", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_update_player);
        AddSubTest("test_deathmatch_update_player_non_existing_fails", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_update_player_non_existing_fails);
        AddSubTest("test_deathmatch_remove_player", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_remove_player);
        AddSubTest("test_deathmatch_reset", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_reset);
        AddSubTest("test_deathmatch_winning_cond_defaults_to_false", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_defaults_to_false);
        AddSubTest("test_deathmatch_winning_cond_time_limit", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_time_limit);
        AddSubTest("test_deathmatch_winning_cond_frag_limit", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_frag_limit);
        AddSubTest("test_deathmatch_winning_cond_time_and_frag_limit", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_time_and_frag_limit);
    }

    virtual void TearDown() override
    {
        if (gm)
        {
            delete gm;
            gm = nullptr;
            dm = nullptr;
        }
    }

    virtual void Finalize() override
    {
        if (m_engine)
        {
            m_engine->shutdown();
            m_engine = NULL;
        }

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
    PGEcfgProfiles& m_cfgProfiles;
    PR00FsUltimateRenderingEngine* m_engine;

    // ---------------------------------------------------------------------------

    GameModeTest(const GameModeTest&) :
        gm(nullptr),
        dm(nullptr),
        m_cfgProfiles(m_cfgProfiles),
        m_engine(nullptr)
    {};

    GameModeTest& operator=(const GameModeTest&)
    {
        return *this;
    };

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
            return false;
        }

        bool b = assertTrue(proofps_dd::GameModeType::DeathMatch == gm->getGameModeType(), "gmtype");
        b &= assertNull(proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType::TeamDeathMatch), "tdm null");
        b &= assertNull(proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType::TeamRoundGame), "trg null");
        b &= assertEquals(0, gm->getResetTime().time_since_epoch().count(), "reset time is epoch");
        b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time is epoch");
        b &= assertTrue(dm->getFragTable().empty(), "playerdata");

        return b;
    }

    bool test_reset_updates_times()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        gm->restart();
        return assertLess(0, gm->getResetTime().time_since_epoch().count(), "reset time") &
            assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time");
    }

    bool test_deathmatch_time_limit_get_set_and_remaining_time_get()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        bool b = assertEquals(0u, dm->getTimeLimitSecs(), "default time limit") &
            assertEquals(0u, dm->getTimeRemainingSecs(), "remaining default");

        dm->setTimeLimitSecs(25u);
        dm->restart();  // restart() is needed to have correct value for remaining time
        b &= assertEquals(25u, dm->getTimeLimitSecs(), "new time limit") &
            assertEquals(25u, dm->getTimeRemainingSecs(), "new remaining");

        return b;
    }

    bool test_deathmatch_frag_limit_get_set()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        bool b = assertEquals(0u, dm->getFragLimit(), "default");
        dm->setFragLimit(25u);
        b &= assertEquals(25u, dm->getFragLimit(), "new");

        return b;
    }

    bool test_deathmatch_add_player_zero_values_maintains_adding_order()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        dm->setFragLimit(11);

        proofps_dd::Player player1(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;

        proofps_dd::Player player2(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 0;
        player2.getDeaths() = 0;

        proofps_dd::Player player3(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
        player3.setName("Joe");
        player3.getFrags() = 0;
        player3.getDeaths() = 0;

        proofps_dd::Player player4(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
        player4.setName("Banana");
        player4.getFrags() = 0;
        player4.getDeaths() = 0;

        bool b = assertTrue(dm->addPlayer(player1), "add player 1");
        b &= assertTrue(dm->addPlayer(player2), "add player 2");
        b &= assertTrue(dm->addPlayer(player3), "add player 3");
        b &= assertTrue(dm->addPlayer(player4), "add player 4");

        const std::vector<proofps_dd::FragTableRow> expectedPlayers = {
            { "Adam", 0, 0 },
            { "Apple", 0, 0 },
            { "Joe", 0, 0 },
            { "Banana", 0, 0 }
        };

        b &= assertFragTableEquals(expectedPlayers, dm->getFragTable());

        return b;
    }

    bool test_deathmatch_add_player_random_values()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        dm->setFragLimit(11);

        proofps_dd::Player player1(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 10;
        player1.getDeaths() = 0;

        proofps_dd::Player player2(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 5;
        player2.getDeaths() = 2;

        proofps_dd::Player player3(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
        player3.setName("Joe");
        player3.getFrags() = 8;
        player3.getDeaths() = 2;

        proofps_dd::Player player4(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
        player4.setName("Banana");
        player4.getFrags() = 8;
        player4.getDeaths() = 0;

        bool b = assertTrue(dm->addPlayer(player1), "add player 1");
        b &= assertTrue(dm->addPlayer(player2), "add player 2");
        b &= assertTrue(dm->addPlayer(player3), "add player 3");
        b &= assertTrue(dm->addPlayer(player4), "add player 4");

        const std::vector<proofps_dd::FragTableRow> expectedPlayers = {
            { "Adam", 10, 0 },
            { "Banana", 8, 0 },
            { "Joe", 8, 2 },
            { "Apple", 5, 2 }
        };

        b &= assertFragTableEquals(expectedPlayers, dm->getFragTable());

        return b;
    }

    bool test_deathmatch_add_player_already_existing_fails()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        dm->setFragLimit(11);

        proofps_dd::Player player1(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 10;
        player1.getDeaths() = 0;

        proofps_dd::Player player2(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 5;
        player2.getDeaths() = 2;

        proofps_dd::Player player3(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
        player3.setName("Joe");
        player3.getFrags() = 8;
        player3.getDeaths() = 2;

        proofps_dd::Player player4(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
        player4.setName("Banana");
        player4.getFrags() = 8;
        player4.getDeaths() = 0;

        bool b = assertTrue(dm->addPlayer(player1), "add player 1");
        b &= assertTrue(dm->addPlayer(player2), "add player 2");
        b &= assertTrue(dm->addPlayer(player3), "add player 3");
        b &= assertTrue(dm->addPlayer(player4), "add player 4");

        player3.setName("Joe");
        player3.getFrags() = 12;
        player3.getDeaths() = 0;
        b &= assertFalse(dm->addPlayer(player3), "add player 3 again");

        const std::vector<proofps_dd::FragTableRow> expectedPlayers = {
            { "Adam", 10, 0 },
            { "Banana", 8, 0 },
            { "Joe", 8, 2 },
            { "Apple", 5, 2 }
        };

        b &= assertFragTableEquals(expectedPlayers, dm->getFragTable());

        return b;
    }

    bool test_deathmatch_update_player()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        dm->setFragLimit(11);

        proofps_dd::Player playerAdam(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        playerAdam.setName("Adam");
        playerAdam.getFrags() = 10;
        playerAdam.getDeaths() = 0;

        proofps_dd::Player playerApple(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        playerApple.setName("Apple");
        playerApple.getFrags() = 5;
        playerApple.getDeaths() = 2;

        proofps_dd::Player playerJoe(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
        playerJoe.setName("Joe");
        playerJoe.getFrags() = 4;
        playerJoe.getDeaths() = 2;

        bool b = assertTrue(dm->addPlayer(playerAdam), "add player Adam");
        b &= assertTrue(dm->addPlayer(playerApple), "add player Apple");
        b &= assertTrue(dm->addPlayer(playerJoe), "add player Joe");

        playerJoe.getFrags()++;
        b &= assertTrue(dm->updatePlayer(playerJoe), "update player Joe 1");
        // since Joe got same number of frags _later_ than Apple, Joe must stay behind Apple
        const std::vector<proofps_dd::FragTableRow> expectedPlayers1 = {
            { "Adam", 10, 0 },
            { "Apple", 5, 2 },
            { "Joe", 5, 2 }
        };      
        b &= assertFragTableEquals(expectedPlayers1, dm->getFragTable(), "table 1");

        playerApple.getDeaths()++;
        b &= assertTrue(dm->updatePlayer(playerApple), "update player Apple 1");
        // since Apple now has more deaths than Joe, it must goe behind Joe
        const std::vector<proofps_dd::FragTableRow> expectedPlayers2 = {
            { "Adam", 10, 0 },
            { "Joe", 5, 2 },
            { "Apple", 5, 3 }
        };
        b &= assertFragTableEquals(expectedPlayers2, dm->getFragTable(), "table 2");

        playerJoe.getDeaths()++;
        b &= assertTrue(dm->updatePlayer(playerJoe), "update player Joe 2");
        // since Joe got same number of frags _earlier_ than Apple, and got same number for deaths _later_ than Apple, it must stay in front of Apple
        const std::vector<proofps_dd::FragTableRow> expectedPlayers3 = {
            { "Adam", 10, 0 },
            { "Joe", 5, 3 },
            { "Apple", 5, 3 }
        };
        b &= assertFragTableEquals(expectedPlayers3, dm->getFragTable(), "table 3");

        playerAdam.getFrags()++;
        b &= assertTrue(dm->updatePlayer(playerAdam), "update player Adam 1");
        // game won, win time is already updated by updatePlayer() even before explicit call to checkWinningConditions()
        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time");
        b &= assertTrue(dm->checkWinningConditions(), "winning");

        return b;
    }

    bool test_deathmatch_update_player_non_existing_fails()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        dm->setFragLimit(10);

        proofps_dd::Player playerAdam(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        playerAdam.setName("Adam");
        playerAdam.getFrags() = 10;
        playerAdam.getDeaths() = 0;

        bool b = assertFalse(dm->updatePlayer(playerAdam), "update player Adam 1");
        b &= assertFalse(dm->checkWinningConditions(), "winning");
        b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time");
        
        const std::vector<proofps_dd::FragTableRow> expectedPlayers1;
        b &= assertFragTableEquals(expectedPlayers1, dm->getFragTable(), "table 1");

        b &= assertTrue(dm->addPlayer(playerAdam), "add player Adam");

        proofps_dd::Player playerJoe(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
        playerJoe.setName("Joe");
        playerJoe.getFrags() = 4;
        playerJoe.getDeaths() = 2;
        b &= assertFalse(dm->updatePlayer(playerJoe), "update player Joe 1");
        const std::vector<proofps_dd::FragTableRow> expectedPlayers2 = {
            { "Adam", 10, 0 }
        };
        b &= assertFragTableEquals(expectedPlayers2, dm->getFragTable(), "table 2");

        return b;
    }

    bool test_deathmatch_remove_player()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        dm->setFragLimit(10);

        proofps_dd::Player playerAdam(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        playerAdam.setName("Adam");
        playerAdam.getFrags() = 10;
        playerAdam.getDeaths() = 0;

        bool b = assertFalse(dm->removePlayer(playerAdam), "remove player Adam 1");

        const std::vector<proofps_dd::FragTableRow> expectedPlayers1;
        b &= assertFragTableEquals(expectedPlayers1, dm->getFragTable(), "table 1");

        b &= assertTrue(dm->addPlayer(playerAdam), "add player Adam");

        proofps_dd::Player playerJoe(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
        playerJoe.setName("Joe");
        playerJoe.getFrags() = 4;
        playerJoe.getDeaths() = 2;
        b &= assertFalse(dm->removePlayer(playerJoe), "remove player Joe 1");
        const std::vector<proofps_dd::FragTableRow> expectedPlayers2 = {
            { "Adam", 10, 0 }
        };
        b &= assertFragTableEquals(expectedPlayers2, dm->getFragTable(), "table 2");

        b &= assertTrue(dm->removePlayer(playerAdam), "remove player Adam 1");
        b &= assertFragTableEquals(expectedPlayers1, dm->getFragTable(), "table 2");

        // even though winner player is removed, winning condition stays true, win time is still valid, an explicit reset() would be needed to clear them!
        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time");
        b &= assertTrue(dm->checkWinningConditions(), "winning");

        return b;
    }

    bool test_deathmatch_reset()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        dm->setTimeLimitSecs(25u);
        dm->setFragLimit(15u);
        
        proofps_dd::Player player1(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 15;
        player1.getDeaths() = 0;

        proofps_dd::Player player2(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 5;
        player2.getDeaths() = 2;

        bool b = assertTrue(dm->addPlayer(player1), "add player 1");

        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time");
        b &= assertTrue(dm->checkWinningConditions(), "winning 1");

        b &= assertTrue(dm->addPlayer(player2), "add player 2");

        dm->restart();

        b &= assertEquals(25u, dm->getTimeLimitSecs(), "time limit");
        b &= assertEquals(15u, dm->getFragLimit(), "frag limit");
        b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time");
        b &= assertLess(0, gm->getResetTime().time_since_epoch().count(), "reset time");
        b &= assertFalse(dm->checkWinningConditions(), "winning 2");
        b &= assertTrue(dm->getFragTable().empty(), "players empty");

        return b;
    }

    bool test_deathmatch_winning_cond_defaults_to_false()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }
        
        return assertFalse(dm->checkWinningConditions());
    }

    bool test_deathmatch_winning_cond_time_limit()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        dm->setTimeLimitSecs(2);
        dm->restart();
        std::set<unsigned int> setRemainingSecs = {0, 1};
        int iSleep = 0;
        while ((iSleep++ < 5) && !dm->checkWinningConditions())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            setRemainingSecs.erase( dm->getTimeRemainingSecs() );
        }
        const auto durationSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - dm->getResetTime());
        bool b = assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time");
        b &= assertTrue(dm->checkWinningConditions(), "winning");
        b &= assertLequals(dm->getTimeLimitSecs(), durationSecs.count(), "time limit elapsed");
        b &= assertTrue(setRemainingSecs.empty(), "no remaining");

        return b;
    }

    bool test_deathmatch_winning_cond_frag_limit()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        dm->setFragLimit(5);
        dm->restart();

        proofps_dd::Player player1(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;

        proofps_dd::Player player2(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Adam");
        player2.getFrags() = 2;
        player2.getDeaths() = 0;

        bool b = assertTrue(dm->addPlayer(player1), "add player 1");
        b &= assertTrue(dm->addPlayer(player2), "add player 2");

        unsigned int i = 0;
        while (!dm->checkWinningConditions() && (i++ < 5))
        {
            player1.getFrags()++;
            b &= assertTrue(dm->updatePlayer(player1), "update player");
        }

        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time");
        b &= assertTrue(dm->checkWinningConditions(), "winning");
        b &= assertEquals(dm->getFragLimit(), i, "frags collected");

        return b;
    }

    bool test_deathmatch_winning_cond_time_and_frag_limit()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        proofps_dd::Player player1(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;

        proofps_dd::Player player2(*m_engine, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 2;
        player2.getDeaths() = 0;

        dm->setFragLimit(5);
        dm->setTimeLimitSecs(2);
        dm->restart();

        bool b = assertTrue(dm->addPlayer(player1), "add player 1");
        b &= assertTrue(dm->addPlayer(player2), "add player 2");

        // time limit elapse also means winning even if frag limit not reached
        std::set<unsigned int> setRemainingSecs = { 0, 1 };
        int iSleep = 0;
        while ((iSleep++ < 5) && !dm->checkWinningConditions())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            setRemainingSecs.erase(dm->getTimeRemainingSecs());
        }
        const auto durationSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - dm->getResetTime());
        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time 1");
        b &= assertTrue(dm->checkWinningConditions(), "winning due to time");
        b &= assertLequals(dm->getTimeLimitSecs(), durationSecs.count(), "time limit elapsed");
        b &= assertTrue(setRemainingSecs.empty(), "no remaining");

        // frag limit reach also means winning even if time limit not reached
        dm->setTimeLimitSecs(100);
        dm->restart();
        b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time 2");
        b &= assertTrue(dm->addPlayer(player1), "add player 1");
        b &= assertTrue(dm->addPlayer(player2), "add player 2");

        unsigned int i = 0;
        while (!dm->checkWinningConditions() && (i++ < 5))
        {
            player1.getFrags()++;
            b &= assertTrue(dm->updatePlayer(player1), "update player");
        }
        b &= assertLess(0, gm->getWinTime().time_since_epoch().count(), "win time 3");
        b &= assertTrue(dm->checkWinningConditions(), "winning due to frags");
        b &= assertEquals(dm->getFragLimit(), i, "frags collected");

        return b;
    }

};