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
#include <memory>
#include <thread>

#include "UnitTest.h"

#include "PGE.h" // for Bullet and PgeCfgProfiles
#include "Network/Stubs/PgeNetworkStub.h"

// PGE has, but here in application we dont have imconfig.h thus we should not try including it!
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include "imgui.h"

#include "GameMode.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"

/**
* This class is for tests where we test pure GameMode functionality: we don't need DeathMatchMode for that.
*/
class SpecialGameMode : public proofps_dd::GameMode
{
public:

    SpecialGameMode() :
        proofps_dd::GameMode(proofps_dd::GameModeType::DeathMatch)
    {

    }

    SpecialGameMode(const SpecialGameMode&) = delete;
    SpecialGameMode& operator=(const SpecialGameMode&) = delete;
    SpecialGameMode(SpecialGameMode&&) = delete;
    SpecialGameMode&& operator=(SpecialGameMode&&) = delete;

    virtual bool addPlayer(
        const proofps_dd::Player& player,
        pge_network::PgeINetwork&) override
    {
        // this is just a very dumb implementation to have minimal add behavior for test_rename_player()
        m_players.push_back(
            proofps_dd::PlayersTableRow{
                player.getName(), player.getServerSideConnectionHandle(), player.getTeamId(), player.getFrags(), player.getDeaths() });

        return true;
    }

    virtual bool updatePlayer(
        const proofps_dd::Player&,
        pge_network::PgeINetwork&) override
    {
        return true;
    }

    virtual bool removePlayer(const proofps_dd::Player&) override
    {
        return true;
    }

    virtual bool isTeamBasedGame() const override
    {
        return false;
    }

    virtual bool isPlayerAllowedForGameplay(const proofps_dd::Player&) const override
    {
        return true;
    }

}; // class SpecialGameMode


class GameModeTest :
    public UnitTest
{
public:

    GameModeTest(PGEcfgProfiles& cfgProfiles) :
        UnitTest(__FILE__),
        m_audio(cfgProfiles),
        gm(nullptr),
        dm(nullptr),
        tdm(nullptr),
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
    GameModeTest& operator=(GameModeTest&&) = delete;

protected:

    virtual void initialize() override
    {
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::GameMode::getLoggerModuleName(), true);

        m_audio.initialize();

        PGEInputHandler& inputHandler = PGEInputHandler::createAndGet(m_cfgProfiles);

        m_engine = &PR00FsUltimateRenderingEngine::createAndGet(m_cfgProfiles, inputHandler);
        m_engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        addSubTest("test_get_gamemodetype_name", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_get_gamemodetype_name));
        addSubTest("test_get_gamemodetype_from_config", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_get_gamemodetype_from_config));
        addSubTest("test_gamemodetype_prefix_increment_operator", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_gamemodetype_prefix_increment_operator));
        addSubTest("test_factory_creates_deathmatch", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_factory_creates_deathmatch));
        addSubTest("test_factory_creates_teamdeathmatch", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_factory_creates_teamdeathmatch));
        addSubTest("test_factory_does_not_create_non_deathmatch", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_factory_does_not_create_non_deathmatch));
        addSubTest("test_restart_updates_times", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_restart_updates_times));
        addSubTest("test_rename_player", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_rename_player));
        addSubTest("test_get_rank", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_get_rank));
        addSubTest("test_time_limit_get_set_and_remaining_time_get", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_time_limit_get_set_and_remaining_time_get));
        addSubTest("test_time_limit_client_update_time_remaining_secs", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_time_limit_client_update_time_remaining_secs));
        addSubTest("test_winning_cond_time_limit", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_winning_cond_time_limit));
        addSubTest("test_receive_and_update_winning_conditions_client", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_receive_and_update_winning_conditions_client));
        addSubTest("test_deathmatch_frag_limit_get_set", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_frag_limit_get_set));
        addSubTest("test_deathmatch_fetch_config", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_fetch_config));
        addSubTest("test_deathmatch_add_player_zero_values_maintains_adding_order", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_add_player_zero_values_maintains_adding_order));
        addSubTest("test_deathmatch_add_player_random_values", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_add_player_random_values));
        addSubTest("test_deathmatch_add_player_already_existing_fails", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_add_player_already_existing_fails));
        addSubTest("test_deathmatch_add_player_sends_winning_state_only_when_game_is_already_won", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_add_player_sends_winning_state_only_when_game_is_already_won));
        addSubTest("test_deathmatch_update_player", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_update_player));
        addSubTest("test_deathmatch_update_player_non_existing_fails", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_update_player_non_existing_fails));
        addSubTest("test_deathmatch_remove_player", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_remove_player));
        addSubTest("test_deathmatch_restart", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_restart));
        addSubTest("test_deathmatch_restart_without_removing_players", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_restart_without_removing_players));
        addSubTest("test_deathmatch_winning_cond_defaults_to_false", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_winning_cond_defaults_to_false));
        addSubTest("test_deathmatch_winning_cond_frag_limit", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_winning_cond_frag_limit));
        addSubTest("test_deathmatch_winning_cond_time_and_frag_limit", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_winning_cond_time_and_frag_limit));
        addSubTest("test_deathmatch_is_player_allowed_for_gameplay", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_is_player_allowed_for_gameplay));
        addSubTest("test_team_deathmatch_get_team_color", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_team_deathmatch_get_team_color));
        addSubTest("test_team_deathmatch_does_not_count_frags_with_zero_team_id", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_team_deathmatch_does_not_count_frags_with_zero_team_id));
        addSubTest("test_team_deathmatch_does_not_allow_any_team_id", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_team_deathmatch_does_not_allow_any_team_id));
        addSubTest("test_team_deathmatch_is_player_allowed_for_gameplay", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_team_deathmatch_is_player_allowed_for_gameplay));
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

    virtual void tearDown() override
    {
        m_network.shutdown();
    }

    virtual void finalize() override
    {
        if (m_engine)
        {
            m_engine->shutdown();
            m_engine = NULL;
        }

        m_audio.shutdown();

        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::GameMode::getLoggerModuleName(), false);
    }

    bool testInitGamemode(const proofps_dd::GameModeType& gamemode)
    {
        gm = proofps_dd::GameMode::createGameMode(gamemode);
        bool b = assertNotNull(gm, (std::string("gm null, ") + std::string(proofps_dd::GameMode::getGameModeTypeName(gamemode))).c_str());
        if (b)
        {
            switch (gamemode)
            {
            case proofps_dd::GameModeType::DeathMatch:
                dm = dynamic_cast<proofps_dd::DeathMatchMode*>(gm);
                b &= assertNotNull(dm, (std::string("dm null, ") + std::string(proofps_dd::GameMode::getGameModeTypeName(gamemode))).c_str());
                break;
            case proofps_dd::GameModeType::TeamDeathMatch:
                // since TDM is derived from DM, we can easily cast TDM to DM, and keep using dm in unit tests even when gamemode is TDM!
                dm = dynamic_cast<proofps_dd::DeathMatchMode*>(gm);
                b &= assertNotNull(dm, (std::string("dm null, ") + std::string(proofps_dd::GameMode::getGameModeTypeName(gamemode))).c_str());
                tdm = dynamic_cast<proofps_dd::TeamDeathMatchMode*>(gm);
                b &= assertNotNull(tdm, (std::string("tdm null, ") + std::string(proofps_dd::GameMode::getGameModeTypeName(gamemode))).c_str());
                break;
            default:
                b = assertFalse(true, "invalid gamemode!");
            }
        }
        return b;
    }

private:

    proofps_dd::GameMode* gm;
    proofps_dd::DeathMatchMode* dm;
    proofps_dd::TeamDeathMatchMode* tdm;
    pge_audio::PgeAudio m_audio;
    PGEcfgProfiles& m_cfgProfiles;
    PgeObjectPool<PooledBullet> m_bullets;
    proofps_dd::EventLister m_itemPickupEvents;
    proofps_dd::EventLister m_ammoChangeEvents;
    PR00FsUltimateRenderingEngine* m_engine;
    pge_network::PgeNetworkStub m_network;

    // ---------------------------------------------------------------------------

    bool assertFragTableEquals(
        const std::vector<proofps_dd::PlayersTableRow>& expectedPlayers,
        const std::list<proofps_dd::PlayersTableRow>& fragTable,
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
                b &= assertEquals(itExpectedPlayers->m_sName, itFragTable->m_sName, ("Name in row " + std::to_string(i) + " " + sLogText).c_str());
                b &= assertEquals(itExpectedPlayers->m_connHandle, itFragTable->m_connHandle, ("ConnHandle in row " + std::to_string(i) + " " + sLogText).c_str());
                b &= assertEquals(itExpectedPlayers->m_iTeamId, itFragTable->m_iTeamId, ("Team in row " + std::to_string(i) + " " + sLogText).c_str());
                b &= assertEquals(itExpectedPlayers->m_nFrags, itFragTable->m_nFrags, ("Frags in row " + std::to_string(i) + " " + sLogText).c_str());
                b &= assertEquals(itExpectedPlayers->m_nDeaths, itFragTable->m_nDeaths, ("Deaths in row " + std::to_string(i) + " " + sLogText).c_str());
                b &= assertEquals(itExpectedPlayers->m_nSuicides, itFragTable->m_nSuicides, ("Suicides in row " + std::to_string(i) + " " + sLogText).c_str());
                b &= assertEquals(itExpectedPlayers->m_nShotsFired, itFragTable->m_nShotsFired, ("Shots fired in row " + std::to_string(i) + " " + sLogText).c_str());
                b &= assertEquals(itExpectedPlayers->m_fFiringAcc, itFragTable->m_fFiringAcc, ("Firing acc in row " + std::to_string(i) + " " + sLogText).c_str());
                
                itFragTable++;
                itExpectedPlayers++;
            }
        }
        return b;
    }

    // convenience function for easily forming failure strings in variadic-gamemode tests
    bool assertFragTableEqualsEz(
        const std::vector<proofps_dd::PlayersTableRow>& expectedPlayers,
        const std::list<proofps_dd::PlayersTableRow>& fragTable,
        const proofps_dd::GameModeType& gamemode,
        const bool& bTestingAsServer,
        const std::string& sLogText)
    {
        return assertFragTableEquals(
            expectedPlayers,
            fragTable,
            sLogText + ", testing as " + (bTestingAsServer ? "server, " : "client, ") + proofps_dd::GameMode::getGameModeTypeName(gamemode));
    }

    // convenience function for easily forming failure strings in variadic-gamemode tests
    bool assertTrueEz(bool statement, const proofps_dd::GameModeType& gamemode, const bool& bTestingAsServer, const std::string& strText)
    {
        return assertTrue(
            statement,
            (strText + ", testing as " + (bTestingAsServer ? "server, " : "client, ") + proofps_dd::GameMode::getGameModeTypeName(gamemode)).c_str());
    }

    // convenience function for easily forming failure strings in variadic-gamemode tests
    bool assertFalseEz(bool statement, const proofps_dd::GameModeType& gamemode, const bool& bTestingAsServer, const std::string& strText)
    {
        return assertFalse(
            statement,
            (strText + ", testing as " + (bTestingAsServer ? "server, " : "client, ") + proofps_dd::GameMode::getGameModeTypeName(gamemode)).c_str());
    }

    // convenience function for easily forming failure strings in variadic-gamemode tests
    template <class T, class S>
    bool assertEqualsEz(const T& expected, const S& checked, const proofps_dd::GameModeType& gamemode, const bool& bTestingAsServer, const std::string& strText)
    {
        return assertEquals(
            expected,
            checked,
            (strText + ", testing as " + (bTestingAsServer ? "server, " : "client, ") + proofps_dd::GameMode::getGameModeTypeName(gamemode)).c_str());
    }

    // convenience function for easily forming failure strings in variadic-gamemode tests
    template <class T, class S>
    bool assertLequalsEz(const T& checked, const S& comparedTo, const proofps_dd::GameModeType& gamemode, const bool& bTestingAsServer, const std::string& strText)
    {
        return assertLequals(
            checked,
            comparedTo,
            (strText + ", testing as " + (bTestingAsServer ? "server, " : "client, ") + proofps_dd::GameMode::getGameModeTypeName(gamemode)).c_str());
    }

    // convenience function for easily forming failure strings in variadic-gamemode tests
    template <class T, class S>
    bool assertLessEz(const T& checked, const S& comparedTo, const proofps_dd::GameModeType& gamemode, const bool& bTestingAsServer, const std::string& strText)
    {
        return assertLess(
            checked,
            comparedTo,
            (strText + ", testing as " + (bTestingAsServer ? "server, " : "client, ") + proofps_dd::GameMode::getGameModeTypeName(gamemode)).c_str());
    }

    bool test_get_gamemodetype_name()
    {
        return (assertEquals("Deathmatch / Free for All", proofps_dd::GameMode::getGameModeTypeName(proofps_dd::GameModeType::DeathMatch), "dm") &
            assertEquals("Team Deathmatch", proofps_dd::GameMode::getGameModeTypeName(proofps_dd::GameModeType::TeamDeathMatch), "tdm") &
            assertEquals("Team Round Game", proofps_dd::GameMode::getGameModeTypeName(proofps_dd::GameModeType::TeamRoundGame), "trg") &
            assertEquals("", proofps_dd::GameMode::getGameModeTypeName(proofps_dd::GameModeType::Max), "max")) != 0;
    }

    bool test_get_gamemodetype_from_config()
    {
        bool b = true;

        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvGamemode].Set(
            static_cast<int>(proofps_dd::GameModeType::DeathMatch));
        b &= assertTrue(proofps_dd::GameModeType::DeathMatch == proofps_dd::GameMode::getGameModeTypeFromConfig(m_cfgProfiles), "dm");
        
        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvGamemode].Set(
            static_cast<int>(proofps_dd::GameModeType::TeamDeathMatch));
        b &= assertTrue(proofps_dd::GameModeType::TeamDeathMatch == proofps_dd::GameMode::getGameModeTypeFromConfig(m_cfgProfiles), "tdm");

        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvGamemode].Set(
            static_cast<int>(proofps_dd::GameModeType::TeamRoundGame));
        b &= assertTrue(proofps_dd::GameModeType::TeamRoundGame == proofps_dd::GameMode::getGameModeTypeFromConfig(m_cfgProfiles), "trg");

        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvGamemode].Set(
            static_cast<int>(proofps_dd::GameModeType::Max));
        b &= assertTrue(proofps_dd::GameModeType::DeathMatch == proofps_dd::GameMode::getGameModeTypeFromConfig(m_cfgProfiles), "max");

        return b;
    }

    bool test_gamemodetype_prefix_increment_operator()
    {
        proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::DeathMatch;
        bool b = true;
        b &= assertTrue(proofps_dd::GameModeType::TeamDeathMatch == ++gamemode, "1");
        b &= assertTrue(proofps_dd::GameModeType::Max == ++gamemode, "2");
        b &= assertTrue(proofps_dd::GameModeType::DeathMatch == ++gamemode, "3");

        return b;
    }

    bool test_factory_creates_deathmatch()
    {
        if (!testInitGamemode(proofps_dd::GameModeType::DeathMatch))
        {
            return assertFalse(true, "testInitGamemode fail");
        }

        bool b = assertTrue(proofps_dd::GameModeType::DeathMatch == gm->getGameModeType(), "gmtype");
        b &= assertEquals(std::string("Deathmatch / Free for All"), gm->getGameModeTypeName(), "gmtype name");
        b &= assertEquals(0, gm->getResetTime().time_since_epoch().count(), "reset time is epoch");
        b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time is epoch");
        b &= assertFalse(gm->isGameWon(), "game not won");
        b &= assertTrue(gm->getPlayersTable().empty(), "playerdata");
        b &= assertFalse(gm->isTeamBasedGame(), "team based");

        return b;
    }

    bool test_factory_creates_teamdeathmatch()
    {
        if (!testInitGamemode(proofps_dd::GameModeType::TeamDeathMatch))
        {
            return assertFalse(true, "testInitGamemode fail");
        }
    
        bool b = assertTrue(proofps_dd::GameModeType::TeamDeathMatch == gm->getGameModeType(), "gmtype");
        b &= assertEquals(std::string("Team Deathmatch"), gm->getGameModeTypeName(), "gmtype name");
        b &= assertEquals(0, gm->getResetTime().time_since_epoch().count(), "reset time is epoch");
        b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time is epoch");
        b &= assertFalse(gm->isGameWon(), "game not won");
        b &= assertTrue(gm->getPlayersTable().empty(), "playerdata");
        b &= assertTrue(gm->isTeamBasedGame(), "team based");
    
        return b;
    }

    bool test_factory_does_not_create_non_deathmatch()
    {
        bool b = assertNull(proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType::Max), "max null");
        b &= assertNull(proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType::TeamRoundGame), "trg null");

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
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            SpecialGameMode sgm;

            sgm.restart(m_network);
            b &= (assertLess(0, sgm.getResetTime().time_since_epoch().count(), (std::string("reset time fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str()) &
                assertEquals(0, sgm.getWinTime().time_since_epoch().count(), (std::string("win time fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str())) != 0;
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
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            SpecialGameMode sgm;

            b &= assertFalse(sgm.renamePlayer("alma", "gg"), (std::string("rename 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(sgm.renamePlayer("", ""), (std::string("rename 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

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

            // because SpecialGameMode::addPlayer() is DUMB, do not modify the order of adding players in this test!
            b &= assertTrue(sgm.addPlayer(player1, m_network), (std::string("add player 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(sgm.addPlayer(player2, m_network), (std::string("add player 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(sgm.addPlayer(player3, m_network), (std::string("add player 3 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(sgm.addPlayer(player4, m_network), (std::string("add player 4 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            b &= assertFalse(sgm.renamePlayer("", ""), (std::string("rename 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(sgm.renamePlayer("Adam", ""), (std::string("rename 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(sgm.renamePlayer("", "Adam"), (std::string("rename 3 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(sgm.renamePlayer("gg", "kkk"), (std::string("rename 4 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(sgm.renamePlayer("Adam", "Joe"), (std::string("rename 5 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(sgm.renamePlayer("Joe", "Adam"), (std::string("rename 6 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(sgm.renamePlayer("adam", "Peter"), (std::string("rename 7 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertTrue(sgm.renamePlayer("Adam", "Peter"), (std::string("rename 8 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            // because SpecialGameMode::addPlayer() is DUMB, do not modify the order of adding players in this test!
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers = {
                { /*"Adam"*/ "Peter", player1.getServerSideConnectionHandle(), 0 /* iTeamId*/, 2, 0},
                { "Apple", player2.getServerSideConnectionHandle(), 0 /* iTeamId*/, 1, 0 },
                { "Joe", player3.getServerSideConnectionHandle(), 0 /* iTeamId*/, 0, 0 },
                { "Banana", player4.getServerSideConnectionHandle(), 0 /* iTeamId*/, 0, 0 }
            };

            b &= assertFragTableEquals(expectedPlayers, sgm.getPlayersTable(), std::string("table fail, testing as ") + (bTestingAsServer ? "server" : "client"));

        }

        return b;
    }

    bool test_get_rank()
    {
        proofps_dd::PlayersTableRow row{};

        bool b = assertEquals(std::string("G0aT"), proofps_dd::GameMode::getRank(row), "1");

        row.m_nFrags = 30;
        b &= assertEquals(std::string("G0aT"), proofps_dd::GameMode::getRank(row), "2");

        row.m_nFrags = -1;
        b &= assertEquals(std::string("Cl0wN"), proofps_dd::GameMode::getRank(row), "3");

        row.m_nDeaths = 15;
        b &= assertEquals(std::string("Cl0wN"), proofps_dd::GameMode::getRank(row), "4");

        row.m_nFrags = 30;
        b &= assertEquals(std::string("G0aT"), proofps_dd::GameMode::getRank(row), "5");

        row.m_nDeaths = 19;
        b &= assertEquals(std::string("G0sU"), proofps_dd::GameMode::getRank(row), "6");

        row.m_nDeaths = 20;
        b &= assertEquals(std::string("G0sU"), proofps_dd::GameMode::getRank(row), "7");

        row.m_nDeaths = 25;
        b &= assertEquals(std::string("Pr0"), proofps_dd::GameMode::getRank(row), "8");

        row.m_nDeaths = 26;
        b &= assertEquals(std::string("N00b"), proofps_dd::GameMode::getRank(row), "9");

        row.m_nFrags = 25;
        row.m_nDeaths = 30;
        b &= assertEquals(std::string("N00b"), proofps_dd::GameMode::getRank(row), "10");

        row.m_nFrags = 15;
        b &= assertEquals(std::string("L0w"), proofps_dd::GameMode::getRank(row), "11");

        row.m_nDeaths = 31;
        b &= assertEquals(std::string("Cl0wN"), proofps_dd::GameMode::getRank(row), "12");

        return b;
    }

    bool test_time_limit_get_set_and_remaining_time_get()
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalse(true, "network reinit as server");
                }
            }

            SpecialGameMode sgm;

            b &= (assertEquals(0u, sgm.getTimeLimitSecs(), (std::string("default time limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str()) &
                assertEquals(0u, sgm.getTimeRemainingMillisecs(), (std::string("remaining default fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str())) != 0;

            sgm.setTimeLimitSecs(25u);
            sgm.restart(m_network);  // restart() is needed to have correct value for remaining time
            b &= assertEquals(25u, sgm.getTimeLimitSecs(), (std::string("new time limit fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str()) &
                /* 23000 millisecs so there is a 2 seconds time window for evaluating the condition (in case scheduler or anything would cause too much delay here) */
                assertLequals(23000u, sgm.getTimeRemainingMillisecs(), (std::string("new remaining fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
        }

        return b;
    }

    bool test_time_limit_client_update_time_remaining_secs()
    {
        // client-only test
        SpecialGameMode sgm;

        constexpr unsigned int nTimeLimitSecs = 13u;
        constexpr unsigned int nTimeRemainingServerSideMillisecs = 4000u;

        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvGmTimeLimit].Set(nTimeLimitSecs);
        sgm.fetchConfig(m_cfgProfiles, m_network);
        sgm.clientUpdateTimeRemainingMillisecs(nTimeRemainingServerSideMillisecs, m_network);

        bool b = true;

        // positive case
        b &= assertEquals(nTimeLimitSecs, sgm.getTimeLimitSecs(), "time limit") &
            assertLequals(sgm.getTimeRemainingMillisecs(), nTimeRemainingServerSideMillisecs, "remaining 1 a") &
            assertNotEquals(0u, sgm.getTimeRemainingMillisecs(), "remaining 1 b");

        // negative case: remaining time as seconds is bigger than time limit as seconds
        sgm.clientUpdateTimeRemainingMillisecs((nTimeLimitSecs + 1u) * 1000, m_network);
        // (nTimeLimitSecs-2) so that there is a 2 seconds time window for evaluating the condition (in case scheduler or anything would cause too much delay here)
        b &= assertLequals((nTimeLimitSecs-2) * 1000, sgm.getTimeRemainingMillisecs(), "remaining 2");

        return b;
    }

    bool test_winning_cond_time_limit()
    {
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalse(true, "network reinit as server");
        }

        SpecialGameMode sgm;

        sgm.setTimeLimitSecs(2);
        sgm.restart(m_network);
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

        std::set<unsigned int> setRemainingSecs = { 0, 1 };
        int iSleep = 0;
        while ((iSleep++ < 5) && !sgm.serverCheckAndUpdateWinningConditions(m_network))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            setRemainingSecs.erase(static_cast<unsigned int>(std::floor(sgm.getTimeRemainingMillisecs() / 1000.f)));
        }
        const auto durationSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - sgm.getResetTime());
        b &= assertLess(0, sgm.getWinTime().time_since_epoch().count(), "win time");
        b &= assertTrue(sgm.isGameWon(), "game won");
        b &= assertTrue(sgm.serverCheckAndUpdateWinningConditions(m_network), "winning");
        b &= assertLequals(static_cast<std::chrono::seconds::rep>(sgm.getTimeLimitSecs()), durationSecs.count(), "time limit elapsed");
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

    bool test_receive_and_update_winning_conditions_client()
    {
        SpecialGameMode sgm;

        bool b = assertEquals(0, sgm.getWinTime().time_since_epoch().count(), "win time 1");
        b &= assertFalse(sgm.isGameWon(), "winning state 1");

        sgm.clientReceiveAndUpdateWinningConditions(m_network, true);

        b &= assertLess(0, sgm.getWinTime().time_since_epoch().count(), "win time 2");
        b &= assertTrue(sgm.isGameWon(), "winning state 2");

        sgm.clientReceiveAndUpdateWinningConditions(m_network, false);

        b &= assertEquals(0, sgm.getWinTime().time_since_epoch().count(), "win time 3");
        b &= assertFalse(sgm.isGameWon(), "winning state 3");

        return b;
    }

    bool test_deathmatch_frag_limit_get_set(const proofps_dd::GameModeType& gamemode)
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalseEz(true, gamemode, bTestingAsServer, "network reinit as server");
                }
            }

            if (!testInitGamemode(gamemode))
            {
                return assertFalseEz(true, gamemode, bTestingAsServer, "testInitGamemode fail");
            }

            b &= assertEqualsEz(0u, dm->getFragLimit(), gamemode, bTestingAsServer, "default fail");
            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(25u);
            b &= assertEqualsEz(25u, dm->getFragLimit(), gamemode, bTestingAsServer, "new fail");
        }

        return b;
    }

    bool test_deathmatch_frag_limit_get_set()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_frag_limit_get_set(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_deathmatch_fetch_config(const proofps_dd::GameModeType& gamemode)
    {
        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvDmFragLimit].Set(25);
        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvGmTimeLimit].Set(13);

        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalseEz(true, gamemode, bTestingAsServer, "network reinit as server");
                }
            }

            if (!testInitGamemode(gamemode))
            {
                return assertFalseEz(true, gamemode, bTestingAsServer, "testInitGamemode fail");
            }

            b &= assertEqualsEz(0u, dm->getFragLimit(), gamemode, bTestingAsServer, "default frag limit fail") &
                assertEqualsEz(0u, gm->getTimeLimitSecs(), gamemode, bTestingAsServer, "default time limit fail");

            gm->fetchConfig(m_cfgProfiles, m_network);
            gm->restart(m_network);  // restart() is needed to have correct value for remaining time

            b &= assertEqualsEz(25u, dm->getFragLimit(), gamemode, bTestingAsServer, "new frag limit fail") &
                assertEqualsEz(13u, gm->getTimeLimitSecs(), gamemode, bTestingAsServer, "new time limit fail") &
                /* 11000 millisecs so there is a 2 seconds time window for evaluating the condition (in case scheduler or anything would cause too much delay here) */
                assertLequalsEz(11000u, gm->getTimeRemainingMillisecs(), gamemode, bTestingAsServer, "new remaining fail");
        }

        return b;
    }

    bool test_deathmatch_fetch_config()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_fetch_config(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_deathmatch_add_player_zero_values_maintains_adding_order(const proofps_dd::GameModeType& gamemode)
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalseEz(true, gamemode, bTestingAsServer, "network reinit as server");
                }
            }

            if (!testInitGamemode(gamemode))
            {
                return assertFalseEz(true, gamemode, bTestingAsServer, "testInitGamemode fail");
            }

            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(11);

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 0;
            player1.getDeaths() = 0;
            player1.getTeamId() = 1;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 0;
            player2.getDeaths() = 0;
            player2.getTeamId() = 2;

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 0;
            player3.getDeaths() = 0;
            player3.getTeamId() = 1;

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 0;
            player4.getDeaths() = 0;
            player4.getTeamId() = 2;

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail");
            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail");
            b &= assertTrueEz(gm->addPlayer(player3, m_network), gamemode, bTestingAsServer, "add player 3 fail");
            b &= assertTrueEz(gm->addPlayer(player4, m_network), gamemode, bTestingAsServer, "add player 4 fail");

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers = {
                { "Adam", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, 0, 0 },
                { "Apple", player2.getServerSideConnectionHandle(), 2 /* iTeamId*/, 0, 0 },
                { "Joe", player3.getServerSideConnectionHandle(), 1 /* iTeamId*/, 0, 0 },
                { "Banana", player4.getServerSideConnectionHandle(), 2 /* iTeamId*/, 0, 0 }
            };

            b &= assertFragTableEqualsEz(expectedPlayers, gm->getPlayersTable(), gamemode, bTestingAsServer, "table fail");
        }

        return b;
    }

    bool test_deathmatch_add_player_zero_values_maintains_adding_order()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_add_player_zero_values_maintains_adding_order(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_deathmatch_add_player_random_values(const proofps_dd::GameModeType& gamemode)
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalseEz(true, gamemode, bTestingAsServer, "network reinit as server");
                }
            }

            if (!testInitGamemode(gamemode))
            {
                return assertFalseEz(true, gamemode, bTestingAsServer, "testInitGamemode fail");
            }

            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(11);

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 10;
            player1.getDeaths() = 0;
            player1.getTeamId() = 1;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 5;
            player2.getDeaths() = 2;
            player2.getTeamId() = 2;

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 8;
            player3.getDeaths() = 2;
            player3.getTeamId() = 1;

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 8;
            player4.getDeaths() = 0;
            player4.getTeamId() = 2;

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail");
            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail");
            b &= assertTrueEz(gm->addPlayer(player3, m_network), gamemode, bTestingAsServer, "add player 3 fail");
            b &= assertTrueEz(gm->addPlayer(player4, m_network), gamemode, bTestingAsServer, "add player 4 fail");

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers = {
                { "Adam", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, 10, 0 },
                { "Banana", player4.getServerSideConnectionHandle(), 2 /* iTeamId*/, 8, 0 },
                { "Joe", player3.getServerSideConnectionHandle(), 1 /* iTeamId*/, 8, 2 },
                { "Apple", player2.getServerSideConnectionHandle(), 2 /* iTeamId*/, 5, 2 }
            };

            b &= assertFragTableEqualsEz(expectedPlayers, gm->getPlayersTable(), gamemode, bTestingAsServer, "table fail");
        }

        return b;
    }

    bool test_deathmatch_add_player_random_values()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_add_player_random_values(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_deathmatch_add_player_already_existing_fails(const proofps_dd::GameModeType& gamemode)
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalseEz(true, gamemode, bTestingAsServer, "network reinit as server");
                }
            }

            if (!testInitGamemode(gamemode))
            {
                return assertFalseEz(true, gamemode, bTestingAsServer, "testInitGamemode fail");
            }

            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(11);

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 10;
            player1.getDeaths() = 0;
            player1.getTeamId() = 1;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 5;
            player2.getDeaths() = 2;
            player2.getTeamId() = 2;

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 8;
            player3.getDeaths() = 2;
            player3.getTeamId() = 1;

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 8;
            player4.getDeaths() = 0;
            player4.getTeamId() = 2;

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1");
            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2");
            b &= assertTrueEz(gm->addPlayer(player3, m_network), gamemode, bTestingAsServer, "add player 3");
            b &= assertTrueEz(gm->addPlayer(player4, m_network), gamemode, bTestingAsServer, "add player 4");

            player3.setName("Joe");
            player3.getFrags() = 12;
            player3.getDeaths() = 0;
            b &= assertFalseEz(gm->addPlayer(player3, m_network), gamemode, bTestingAsServer, "add player 3 again");

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers = {
                { "Adam", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, 10, 0 },
                { "Banana", player4.getServerSideConnectionHandle(), 2 /* iTeamId*/, 8, 0 },
                { "Joe", player3.getServerSideConnectionHandle(), 1 /* iTeamId*/, 8, 2 },
                { "Apple", player2.getServerSideConnectionHandle(), 2 /* iTeamId*/, 5, 2 }
            };

            b &= assertFragTableEqualsEz(expectedPlayers, gm->getPlayersTable(), gamemode, bTestingAsServer, "table fail");
        }

        return b;
    }

    bool test_deathmatch_add_player_already_existing_fails()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_add_player_already_existing_fails(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_deathmatch_add_player_sends_winning_state_only_when_game_is_already_won(const proofps_dd::GameModeType& gamemode)
    {
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true, "testInitGamemode fail");
        }

        bool b = true;

        // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
        // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
        dm->setFragLimit(11);

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 10;
        player1.getDeaths() = 0;
        player1.getTeamId() = 1; // required non-zero teamId for TDM test, otherwise TDM does not count frags

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 5;
        player2.getDeaths() = 2;
        player1.getTeamId() = 2; // required non-zero teamId for TDM test, otherwise TDM does not count frags

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true /*server*/, "add player 1 fail");

        // initially there is no sent out packets because game is NOT yet won
        b &= assertEqualsEz(0u, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count");

        // adding same player fails so still no sent out packets
        b &= assertFalseEz(gm->addPlayer(player1, m_network), gamemode, true /*server*/, "add player 1 again fail 1");
        b &= assertEqualsEz(0u, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count");

        // game is now won, expecting 1 sent pkts to the virtually connected client (ServerStub has 1 virtual always-connected client)
        player1.getFrags()++;
        b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true /*server*/, "update player 1 fail");
        b &= assertEqualsEz(1u, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true /*server*/, "tx msg count"
            );
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true /*server*/, "tx msg count");
        }

        // adding same player fails so number of sent pkts should not change
        b &= assertFalseEz(gm->addPlayer(player1, m_network), gamemode, true /*server*/, "add player 1 again fail 2");
        b &= assertEqualsEz(1u, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count");

        // now adding new player should trigger sending out MsgGameSessionStateFromServer to the player since game state is already won
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true /*server*/, "add player 2 fail");
        b &= assertEqualsEz(2u, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true /*server*/, "tx msg count"
            );
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true /*server*/, "tx msg count");
        }

        return b;
    }

    bool test_deathmatch_add_player_sends_winning_state_only_when_game_is_already_won()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_add_player_sends_winning_state_only_when_game_is_already_won(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
            
            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;
    }

    bool test_deathmatch_update_player(const proofps_dd::GameModeType& gamemode)
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalseEz(true, gamemode, bTestingAsServer, "network reinit as server");
                }
            }

            if (!testInitGamemode(gamemode))
            {
                return assertFalseEz(true, gamemode, bTestingAsServer, "testInitGamemode fail");
            }

            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            
            // Basically TDM::updatePlayer() does not alter DM::updatePlayer() behavior wrt the ordering when frag count decreases or death count increases,
            // even though players can be in different teams, it does not matter. We just dont care. We just care about frag limit being reached or not.
            // GUI will take care of displaying the players, and it can still simply iterate over the players in the order as DM or TDM is containing the players,
            // no further logic is needed in GUI, just separate them by team ID!
            if (gamemode == proofps_dd::GameModeType::DeathMatch)
            {
                dm->setFragLimit(11);
            }
            else if (gamemode == proofps_dd::GameModeType::TeamDeathMatch)
            {
                dm->setFragLimit(16);
            }
            else
            {
                return assertTrue(false, "unhandled gamemode");
            }

            proofps_dd::Player playerAdam(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            playerAdam.setName("Adam");
            playerAdam.getFrags() = 10;
            playerAdam.getDeaths() = 0;
            playerAdam.getTeamId() = 1;

            proofps_dd::Player playerApple(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            playerApple.setName("Apple");
            playerApple.getFrags() = 5;
            playerApple.getDeaths() = 2;
            playerApple.getTeamId() = 2;

            proofps_dd::Player playerJoe(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            playerJoe.setName("Joe");
            playerJoe.getFrags() = 4;
            playerJoe.getDeaths() = 2;
            playerJoe.getTeamId() = 1;

            b &= assertTrueEz(gm->addPlayer(playerAdam, m_network), gamemode, bTestingAsServer, "add player Adam fail");
            b &= assertTrueEz(gm->addPlayer(playerApple, m_network), gamemode, bTestingAsServer, "add player Apple fail");
            b &= assertTrueEz(gm->addPlayer(playerJoe, m_network), gamemode, bTestingAsServer, "add player Joe fail");

            playerJoe.getFrags()++;
            b &= assertTrueEz(gm->updatePlayer(playerJoe, m_network), gamemode, bTestingAsServer, "update player Joe 1 fail");
            // since Joe got same number of frags _later_ than Apple, Joe must stay behind Apple
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers1 = {
                { "Adam", playerAdam.getServerSideConnectionHandle(), 1 /* iTeamId*/, 10, 0 },
                { "Apple", playerApple.getServerSideConnectionHandle(), 2 /* iTeamId*/, 5, 2 },
                { "Joe", playerJoe.getServerSideConnectionHandle(), 1 /* iTeamId*/, 5, 2 }
            };
            assertFragTableEqualsEz(expectedPlayers1, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 1 fail");

            playerApple.getDeaths()++;
            b &= assertTrueEz(gm->updatePlayer(playerApple, m_network), gamemode, bTestingAsServer, "update player Apple 1 fail");
            // since Apple now has more deaths than Joe, it must goe behind Joe
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers2 = {
                { "Adam", playerAdam.getServerSideConnectionHandle(), 1 /* iTeamId*/, 10, 0 },
                { "Joe", playerJoe.getServerSideConnectionHandle(), 1 /* iTeamId*/, 5, 2 },
                { "Apple", playerApple.getServerSideConnectionHandle(), 2 /* iTeamId*/, 5, 3 }
            };
            assertFragTableEqualsEz(expectedPlayers2, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 2 fail");

            playerJoe.getDeaths()++;
            b &= assertTrueEz(gm->updatePlayer(playerJoe, m_network), gamemode, bTestingAsServer, "update player Joe 2 fail");
            // since Joe got same number of frags _earlier_ than Apple, and got same number for deaths _later_ than Apple, it must stay in front of Apple
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers3 = {
                { "Adam", playerAdam.getServerSideConnectionHandle(), 1 /* iTeamId*/, 10, 0 },
                { "Joe", playerJoe.getServerSideConnectionHandle(), 1 /* iTeamId*/, 5, 3 },
                { "Apple", playerApple.getServerSideConnectionHandle(), 2 /* iTeamId*/, 5, 3 }
            };
            assertFragTableEqualsEz(expectedPlayers3, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 3 fail");

            // reaching frag limit (frag limit is set differently for DM and TDM)
            playerAdam.getFrags()++;
            b &= assertTrueEz(gm->updatePlayer(playerAdam, m_network), gamemode, bTestingAsServer, "update player Adam 1 fail");

            if (bTestingAsServer)
            {
                // game won, win time is already updated by updatePlayer() even before explicit call to serverCheckAndUpdateWinningConditions();
                // this is known only by server, client needs to be informed by server
                b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won");
                b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time fail");
                b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning server");
                b &= assertEqualsEz(1u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count");
                try
                {
                    b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count");
                }
            }

        }

        return b;
    }

    bool test_deathmatch_update_player()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_update_player(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_deathmatch_update_player_non_existing_fails(const proofps_dd::GameModeType& gamemode)
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalseEz(true, gamemode, bTestingAsServer, "network reinit as server");
                }
            }

            if (!testInitGamemode(gamemode))
            {
                return assertFalseEz(true, gamemode, bTestingAsServer, "testInitGamemode fail");
            }

            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(10);

            proofps_dd::Player playerAdam(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            playerAdam.setName("Adam");
            playerAdam.getFrags() = 10;
            playerAdam.getDeaths() = 0;

            b &= assertFalse(dm->updatePlayer(playerAdam, m_network), "update player Adam 1 fail");

            if (bTestingAsServer)
            {
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game not won");
                b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning");
                b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
            }

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers1;
            b &= assertFragTableEqualsEz(expectedPlayers1, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 1 fail");

            b &= assertTrueEz(gm->addPlayer(playerAdam, m_network), gamemode, bTestingAsServer, "add player Adam fail");

            proofps_dd::Player playerJoe(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            playerJoe.setName("Joe");
            playerJoe.getFrags() = 4;
            playerJoe.getDeaths() = 2;
            b &= assertFalseEz(gm->updatePlayer(playerJoe, m_network), gamemode, bTestingAsServer, "update player Joe 1 fail");
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers2 = {
                { "Adam", playerAdam.getServerSideConnectionHandle(), 0 /* iTeamId*/, 10, 0 }
            };
            b &= assertFragTableEqualsEz(expectedPlayers2, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 2 fail");

        }

        return b;
    }

    bool test_deathmatch_update_player_non_existing_fails()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_update_player_non_existing_fails(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_deathmatch_remove_player(const proofps_dd::GameModeType& gamemode)
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalseEz(true, gamemode, bTestingAsServer, "network reinit as server");
                }
            }

            if (!testInitGamemode(gamemode))
            {
                return assertFalseEz(true, gamemode, bTestingAsServer, "testInitGamemode fail");
            }

            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(10);

            proofps_dd::Player playerAdam(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            playerAdam.setName("Adam");
            playerAdam.getFrags() = dm->getFragLimit();
            playerAdam.getDeaths() = 0;
            playerAdam.getTeamId() = 1; // required non-zero teamId for TDM test, otherwise TDM does not count frags

            b &= assertFalseEz(gm->removePlayer(playerAdam), gamemode, bTestingAsServer, "removep player Adam 1 fail");

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers1;
            b &= assertFragTableEqualsEz(expectedPlayers1, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 1 fail");

            // here game state becomes won
            b &= assertTrueEz(gm->addPlayer(playerAdam, m_network), gamemode, bTestingAsServer, "add player Adam fail");

            proofps_dd::Player playerJoe(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            playerJoe.setName("Joe");
            playerJoe.getFrags() = 4;
            playerJoe.getDeaths() = 2;
            playerJoe.getTeamId() = 2; // required non-zero teamId for TDM test, otherwise TDM does not count frags
            b &= assertFalseEz(gm->removePlayer(playerJoe), gamemode, bTestingAsServer, "remove player Joe 1 fail");
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers2 = {
                { "Adam", playerAdam.getServerSideConnectionHandle(), 1 /* iTeamId*/, 10, 0 }
            };
            b &= assertFragTableEqualsEz(expectedPlayers2, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 2 fail");

            b &= assertTrueEz(gm->removePlayer(playerAdam), gamemode, bTestingAsServer, "remove player Adam 1 fail");
            b &= assertFragTableEqualsEz(expectedPlayers1, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 3 fail");

            if (bTestingAsServer)
            {
                // even though winner player is removed, winning condition stays true, win time is still valid, an explicit reset() would be needed to clear them!
                b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won");
                b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning");
            }

        }

        return b;
    }

    bool test_deathmatch_remove_player()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_remove_player(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_deathmatch_restart(const proofps_dd::GameModeType& gamemode)
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalseEz(true, gamemode, bTestingAsServer, "network reinit as server");
                }
            }

            if (!testInitGamemode(gamemode))
            {
                return assertFalseEz(true, gamemode, bTestingAsServer, "testInitGamemode fail");
            }

            gm->setTimeLimitSecs(25u);
            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
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

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail");

            if (bTestingAsServer)
            {
                b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 1");
                b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 1");
                b &= assertEqualsEz(1u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count");
                try
                {
                    b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count");
                }
            }

            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail");
            if (bTestingAsServer)
            {
                // in case of server instance, addPlayer() sends MsgGameSessionStateFromServer to newly added player when isGameWon() is true 
                b &= assertEqualsEz(2u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count");
                try
                {
                    b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count");
                }
            }

            gm->restart(m_network);

            b &= assertEqualsEz(25u, gm->getTimeLimitSecs(), gamemode, bTestingAsServer, "time limit fail");
            b &= assertEqualsEz(15u, dm->getFragLimit(), gamemode, bTestingAsServer, "frag limit fail");
            if (bTestingAsServer)
            {
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 2");
                b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertLessEz(0, gm->getResetTime().time_since_epoch().count(), gamemode, bTestingAsServer, "reset time");
                b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 2");

                // outgoing packet for winning state true -> false transition too
                b &= assertEqualsEz(3u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count");
                try
                {
                    b &= assertEqualsEz(3u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count");
                }
            }
            b &= assertTrueEz(gm->getPlayersTable().empty(), gamemode, bTestingAsServer, "players empty fail");

        }

        return b;
    }

    bool test_deathmatch_restart()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_restart(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_deathmatch_restart_without_removing_players(const proofps_dd::GameModeType& gamemode)
    {
        bool bTestingAsServer = false;
        bool b = true;

        for (auto i = 1; i <= 2; i++)
        {
            if (i == 2)
            {
                tearDown();
                bTestingAsServer = true;
                m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(bTestingAsServer);
                if (!m_network.initialize())
                {
                    return assertFalseEz(true, gamemode, bTestingAsServer, "network reinit as server");
                }
            }

            if (!testInitGamemode(gamemode))
            {
                return assertFalseEz(true, gamemode, bTestingAsServer, "testInitGamemode fail");
            }

            gm->setTimeLimitSecs(25u);
            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(15u);

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getTeamId() = 1;
            player1.getFrags() = 15;
            player1.getDeaths() = 0;
            player1.getSuicides() = 0;
            player1.getFiringAccuracy() = 1.f;
            player1.getShotsFiredCount() = 20;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getTeamId() = 2;
            player2.getFrags() = 5;
            player2.getDeaths() = 2;
            player2.getSuicides() = 1;
            player2.getFiringAccuracy() = 0.5f;
            player2.getShotsFiredCount() = 10;

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail");

            if (bTestingAsServer)
            {
                b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 1");
                b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 1");
                b &= assertEqualsEz(1u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count");
                try
                {
                    b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count");
                }
            }

            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail");
            if (bTestingAsServer)
            {
                // in case of server instance, addPlayer() sends MsgGameSessionStateFromServer to newly added player when isGameWon() is true 
                b &= assertEqualsEz(2u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count");
                try
                {
                    b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count");
                }
            }

            gm->restartWithoutRemovingPlayers(m_network);

            b &= assertEqualsEz(25u, gm->getTimeLimitSecs(), gamemode, bTestingAsServer, "time limit fail");
            b &= assertEqualsEz(15u, dm->getFragLimit(), gamemode, bTestingAsServer, "frag limit fail");
            if (bTestingAsServer)
            {
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 2");
                b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertLessEz(0, gm->getResetTime().time_since_epoch().count(), gamemode, bTestingAsServer, "reset time");
                b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 2");

                // outgoing packet for winning state true -> false transition too
                b &= assertEqualsEz(3u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count");
                try
                {
                    b &= assertEqualsEz(3u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count");
                }
            }

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayersStillThereWithResetStats = {
                { "Adam", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, 0, 0 /* and rest are zeroed too */},
                { "Apple", player2.getServerSideConnectionHandle(), 2 /* iTeamId*/, 0, 0 /* and rest are zeroed too */}
            };

            b &= assertFragTableEqualsEz(expectedPlayersStillThereWithResetStats, gm->getPlayersTable(), gamemode, bTestingAsServer, "table fail");

        }

        return b;
    }

    bool test_deathmatch_restart_without_removing_players()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_restart_without_removing_players(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_deathmatch_winning_cond_defaults_to_false(const proofps_dd::GameModeType& gamemode)
    {
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true, "testInitGamemode fail");
        }
        
        return assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "");
    }

    bool test_deathmatch_winning_cond_defaults_to_false()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_winning_cond_defaults_to_false(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));

            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;
    }

    bool test_deathmatch_winning_cond_frag_limit(const proofps_dd::GameModeType& gamemode)
    {
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true, "testInitGamemode fail");
        }

        // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
        // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.

        // Basically TDM and DM classes' behavior is very similar, even though players can be in different teams, it does not matter. We just dont care.
        // We just care about frag limit being reached or not, that is why serverCheckAndUpdateWinningConditions() can be overridden.
        // GUI will take care of displaying the players, and it can still simply iterate over the players in the order as DM or TDM is containing the players,
        // no further logic is needed in GUI, just separate them by team ID!
        if (gamemode == proofps_dd::GameModeType::DeathMatch)
        {
            dm->setFragLimit(5);
        }
        else if (gamemode == proofps_dd::GameModeType::TeamDeathMatch)
        {
            dm->setFragLimit(7);
        }
        else
        {
            return assertTrue(false, "unhandled gamemode");
        }

        gm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        bool b = assertEqualsEz(1u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true/*server*/, "tx msg count"
            );
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count");
        }

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 1;

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Adam");
        player2.getFrags() = 2;
        player2.getDeaths() = 0;
        player2.getTeamId() = player1.getTeamId(); // yes, intentionally in same team

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");

        unsigned int i = 0;
        while (!gm->serverCheckAndUpdateWinningConditions(m_network) && (i++ < 5))
        {
            player1.getFrags()++;
            b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player");
        }

        b &= assertTrueEz(gm->isGameWon(), gamemode, true/*server*/, "game won 2");
        b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time");
        b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning");
        // in both DM and TDM same amount of frags needed to be collected by player 1, since frag limit is different!
        b &= assertEqualsEz(5u, i, gamemode, true/*server*/, "frags collected");

        b &= assertEqualsEz(2u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true/*server*/, "tx msg count"
            );
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count");
        }

        return b;
    }

    bool test_deathmatch_winning_cond_frag_limit()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_winning_cond_frag_limit(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));

            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;
    }

    bool test_deathmatch_winning_cond_time_and_frag_limit(const proofps_dd::GameModeType& gamemode)
    {
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true/*server*/, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 1; // required non-zero teamId for TDM test, otherwise TDM does not count frags

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 2;
        player2.getDeaths() = 0;
        player2.getTeamId() = player1.getTeamId(); // yes, intentionally in same team

        // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
        // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.

        // Basically TDM and DM classes' behavior is very similar, even though players can be in different teams, it does not matter. We just dont care.
        // We just care about frag limit being reached or not, that is why serverCheckAndUpdateWinningConditions() can be overridden.
        // GUI will take care of displaying the players, and it can still simply iterate over the players in the order as DM or TDM is containing the players,
        // no further logic is needed in GUI, just separate them by team ID!

        if (gamemode == proofps_dd::GameModeType::DeathMatch)
        {
            dm->setFragLimit(5);
        }
        else if (gamemode == proofps_dd::GameModeType::TeamDeathMatch)
        {
            dm->setFragLimit(7);
        }
        else
        {
            return assertTrue(false, "unhandled gamemode");
        }

        gm->setTimeLimitSecs(2);

        gm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        bool b = assertEqualsEz(1u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true/*server*/, "tx msg count"
            );
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count");
        }

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");

        // time limit elapse also means winning even if frag limit not reached
        std::set<unsigned int> setRemainingSecs = { 0, 1 };
        int iSleep = 0;
        while ((iSleep++ < 5) && !gm->serverCheckAndUpdateWinningConditions(m_network))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            setRemainingSecs.erase(static_cast<unsigned int>(std::floor(gm->getTimeRemainingMillisecs() / 1000.f)));
        }
        b &= assertTrueEz(gm->isGameWon(), gamemode, true/*server*/, "game won 1");
        const auto durationSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - gm->getResetTime());
        b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time 1");
        b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning due to time");
        b &= assertLequalsEz(static_cast<std::chrono::seconds::rep>(gm->getTimeLimitSecs()), durationSecs.count(), gamemode, true/*server*/, "time limit elapsed");
        b &= assertTrueEz(setRemainingSecs.empty(), gamemode, true/*server*/, "no remaining");

        b &= assertEqualsEz(2u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true/*server*/, "tx msg count"
            );
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count");
        }

        // frag limit reach also means winning even if time limit not reached
        gm->setTimeLimitSecs(100);
        gm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        b &= assertEqualsEz(3u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(3u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true/*server*/, "tx msg count"
            );
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count");
        }

        b &= assertFalseEz(gm->isGameWon(), gamemode, true/*server*/, "game won 2");
        b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time 2");

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");

        unsigned int i = 0;
        while (!gm->serverCheckAndUpdateWinningConditions(m_network) && (i++ < 5))
        {
            player1.getFrags()++;
            b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player");
        }
        b &= assertTrueEz(gm->isGameWon(), gamemode, true/*server*/, "game won 3");
        b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time 3");
        b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning due to frags");
        // in both DM and TDM same amount of frags needed to be collected by player 1, since frag limit is different!
        b &= assertEqualsEz(5u, i, gamemode, true/*server*/, "frags collected");

        b &= assertEqualsEz(4u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(4u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true/*server*/, "tx msg count"
            );
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count");
        }

        return b;
    }

    bool test_deathmatch_winning_cond_time_and_frag_limit()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_deathmatch_winning_cond_time_and_frag_limit(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));

            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;
    }

    bool test_deathmatch_is_player_allowed_for_gameplay()
    {
        constexpr proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::DeathMatch;
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true/*server*/, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 0;

        bool b = true;

        // DM mode just dont care about team id
        b &= assertTrueEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 1");

        player1.getTeamId() = 1;
        b &= assertTrueEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 2");

        return b;
    }

    bool test_team_deathmatch_get_team_color()
    {
        const PureColor expectedPureColor0(255, 255, 255, 255);
        const PureColor expectedPureColor1(127, 255, 255, 255);
        const PureColor expectedPureColor2(255, 127, 127, 255);
        const PureColor resultPureColor0 = proofps_dd::TeamDeathMatchMode::getTeamColor(0);
        const PureColor resultPureColor1 = proofps_dd::TeamDeathMatchMode::getTeamColor(1);
        const PureColor resultPureColor2 = proofps_dd::TeamDeathMatchMode::getTeamColor(2);

        return (assertEquals(expectedPureColor0, resultPureColor0, "0") &
            assertEquals(expectedPureColor1, resultPureColor1, "1") &
            assertEquals(expectedPureColor2, resultPureColor2, "2")) != 0;
    }

    bool test_team_deathmatch_does_not_count_frags_with_zero_team_id()
    {
        // in this TDM test we also test getTeamFrags() and getTeamPlayersCount()

        constexpr proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamDeathMatch;
        // since earlier tests like test_deathmatch_winning_cond_frag_limit() test TDM class also, here we are keeping this test very small.

        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true, "testInitGamemode fail");
        }

        // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
        // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.

        // Basically TDM and DM classes' behavior is very similar, even though players can be in different teams, it does not matter. We just dont care.
        // We just care about frag limit being reached or not, that is why serverCheckAndUpdateWinningConditions() can be overridden.
        // GUI will take care of displaying the players, and it can still simply iterate over the players in the order as DM or TDM is containing the players,
        // no further logic is needed in GUI, just separate them by team ID!
        dm->setFragLimit(7);

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 1;

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Adam");
        player2.getFrags() = 2;
        player2.getDeaths() = 0;
        player2.getTeamId() = player1.getTeamId(); // yes, intentionally in same team

        proofps_dd::Player player3(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
        player3.setName("Joe");
        player3.getFrags() = dm->getFragLimit();  // game would be already won if this player had a valid teamId!
        player3.getDeaths() = 0;
        player3.getTeamId() = 0; // intentionally zero, meaning no team selected!

        bool b = true;
        b &= assertEquals(0, tdm->getTeamFrags(0), "team 0 frags 1");  // team 0 always 0 summed frags
        b &= assertEquals(0, tdm->getTeamFrags(1), "team 1 frags 1");
        b &= assertEquals(0, tdm->getTeamFrags(2), "team 2 frags 1");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(0), "team 0 players count 1");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(1), "team 1 players count 1");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(2), "team 2 players count 1");
        
        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");
        b &= assertTrueEz(gm->addPlayer(player3, m_network), gamemode, true/*server*/, "add player 3");
        b &= assertEquals(0, tdm->getTeamFrags(0), "team 0 frags 2");  // team 0 always 0 summed frags
        b &= assertEquals(2, tdm->getTeamFrags(1), "team 1 frags 2");
        b &= assertEquals(0, tdm->getTeamFrags(2), "team 2 frags 2");
        b &= assertEquals(1u, tdm->getTeamPlayersCount(0), "team 0 players count 2");
        b &= assertEquals(2u, tdm->getTeamPlayersCount(1), "team 1 players count 2");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(2), "team 2 players count 2");

        unsigned int i = 0;
        while (!gm->serverCheckAndUpdateWinningConditions(m_network) && (i++ < 5))
        {
            player1.getFrags()++;
            b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player");
        }

        b &= assertEquals(0, tdm->getTeamFrags(0), "team 0 frags 3");  // team 0 always 0 summed frags
        b &= assertEquals(7, tdm->getTeamFrags(1), "team 1 frags 3");
        b &= assertEquals(0, tdm->getTeamFrags(2), "team 2 frags 3");
        b &= assertEquals(1u, tdm->getTeamPlayersCount(0), "team 0 players count 3");
        b &= assertEquals(2u, tdm->getTeamPlayersCount(1), "team 1 players count 3");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(2), "team 2 players count 3");

        b &= assertTrueEz(gm->isGameWon(), gamemode, true/*server*/, "game won 2");
        b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time");
        b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning");
        b &= assertEqualsEz(5u, i, gamemode, true/*server*/, "frags collected");

        b &= assertEqualsEz(1u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true/*server*/, "tx msg count"
            );
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count");
        }

        player3.getTeamId() = 2;
        player3.getFrags() = 4;
        b &= assertTrueEz(gm->updatePlayer(player3, m_network), gamemode, true/*server*/, "update player 2");
        b &= assertEquals(0, tdm->getTeamFrags(0), "team 0 frags 4");  // team 0 always 0 summed frags
        b &= assertEquals(7, tdm->getTeamFrags(1), "team 1 frags 4");
        b &= assertEquals(4, tdm->getTeamFrags(2), "team 2 frags 4");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(0), "team 0 players count 4");
        b &= assertEquals(2u, tdm->getTeamPlayersCount(1), "team 1 players count 4");
        b &= assertEquals(1u, tdm->getTeamPlayersCount(2), "team 2 players count 4");

        return b;
    }

    bool test_team_deathmatch_does_not_allow_any_team_id()
    {
        constexpr proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamDeathMatch;
        // since earlier tests like test_deathmatch_winning_cond_frag_limit() test TDM class also, here we are keeping this test very small.

        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true, "testInitGamemode fail");
        }

        // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
        // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.

        // Basically TDM and DM classes' behavior is very similar, even though players can be in different teams, it does not matter. We just dont care.
        // We just care about frag limit being reached or not, that is why serverCheckAndUpdateWinningConditions() can be overridden.
        // GUI will take care of displaying the players, and it can still simply iterate over the players in the order as DM or TDM is containing the players,
        // no further logic is needed in GUI, just separate them by team ID!
        dm->setFragLimit(7);

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 3;

        bool b = true;
        b &= assertFalseEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        const std::vector<proofps_dd::PlayersTableRow> expectedPlayers1;
        assertFragTableEqualsEz(expectedPlayers1, gm->getPlayersTable(), gamemode, true /*server*/, "table 1 fail");

        player1.getTeamId() = 2;
        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 2");
        const std::vector<proofps_dd::PlayersTableRow> expectedPlayers2 = {
                { "Apple", player1.getServerSideConnectionHandle(), 2 /* iTeamId*/, 0, 0 }
        };
        assertFragTableEqualsEz(expectedPlayers2, gm->getPlayersTable(), gamemode, true /*server*/, "table 2 fail");

        player1.getTeamId() = 3;
        b &= assertFalseEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player 1");
        assertFragTableEqualsEz(expectedPlayers2, gm->getPlayersTable(), gamemode, true /*server*/, "table 3 fail");

        player1.getTeamId() = 1;
        b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player 2");
        const std::vector<proofps_dd::PlayersTableRow> expectedPlayers3 = {
                { "Apple", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, 0, 0 }
        };
        assertFragTableEqualsEz(expectedPlayers3, gm->getPlayersTable(), gamemode, true /*server*/, "table 4 fail");

        return b;
    }

    bool test_team_deathmatch_is_player_allowed_for_gameplay()
    {
        constexpr proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamDeathMatch;
        // server-only test
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true/*server*/, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 0;

        bool b = true;

        b &= assertFalseEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 1");

        player1.getTeamId() = 1;
        b &= assertTrueEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 2");

        player1.getTeamId() = 2;
        b &= assertTrueEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 3");

        return b;
    }

};