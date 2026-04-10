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

#include "GameMode.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"

/**
* This class is for tests where we test pure GameMode functionality: we don't need DeathMatchMode for that.
*/
class SpecialGameMode : public proofps_dd::GameMode
{
public:

    SpecialGameMode(const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers) :
        proofps_dd::GameMode(proofps_dd::GameModeType::DeathMatch, m_mapPlayers)
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
                player.getName(), player.getServerSideConnectionHandle(), player.getTeamId(), player.isInSpectatorMode(), player.getFrags(), player.getDeaths() });

        return true;
    }

    virtual bool updatePlayer(
        const proofps_dd::Player&,
        pge_network::PgeINetwork&) override
    {
        return true;
    }

    virtual bool removePlayer(
        const proofps_dd::Player&,
        pge_network::PgeINetwork&) override
    {
        return true;
    }

    virtual bool isTeamBasedGame() const override
    {
        return false;
    }

    virtual bool isRoundBased() const override
    {
        return false;
    }

    virtual bool isRespawnAllowedAfterDie() const override
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
        m_inventoryChangeEvents(8 /* time limit secs */, 5 /* event count limit */),
        m_ammoChangeEvents(8 /* time limit secs */, 5 /* event count limit */, proofps_dd::Orientation::Horizontal),
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
        addSubTest("test_factory_creates_teamroundgame", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_factory_creates_teamroundgame));
        addSubTest("test_restart_updates_times", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_restart_updates_times));
        addSubTest("test_rename_player", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_rename_player));
        addSubTest("test_get_rank", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_get_rank));
        addSubTest("test_time_limit_get_set_and_remaining_time_get", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_time_limit_get_set_and_remaining_time_get));
        addSubTest("test_time_limit_client_update_time_remaining_secs", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_time_limit_client_update_time_remaining_secs));
        addSubTest("test_winning_cond_time_limit", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_winning_cond_time_limit));
        addSubTest("test_receive_and_update_winning_conditions_client", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_receive_and_update_winning_conditions_client));
        addSubTest("test_frag_limit_get_set", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_frag_limit_get_set));
        addSubTest("test_round_win_limit_get_set", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_win_limit_get_set));
        addSubTest("test_deathmatch_fetch_config", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_fetch_config));
        addSubTest("test_teamroundgame_fetch_config", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_teamroundgame_fetch_config));
        addSubTest("test_add_player_zero_values_maintains_adding_order", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_add_player_zero_values_maintains_adding_order));
        addSubTest("test_add_player_random_values", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_add_player_random_values));
        addSubTest("test_add_player_already_existing_fails", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_add_player_already_existing_fails));
        addSubTest("test_add_player_sends_winning_state_only_when_game_is_already_won", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_add_player_sends_winning_state_only_when_game_is_already_won));
        addSubTest("test_update_player", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_update_player));
        addSubTest("test_update_player_non_existing_fails", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_update_player_non_existing_fails));
        addSubTest("test_remove_player", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_remove_player));
        addSubTest("test_restart_deathmatches", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_restart_deathmatches));
        addSubTest("test_restart_roundgames", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_restart_roundgames));
        addSubTest("test_restart_deathmatches_without_removing_players", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_restart_deathmatches_without_removing_players));
        addSubTest("test_restart_roundgames_without_removing_players", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_restart_roundgames_without_removing_players));
        addSubTest("test_winning_cond_defaults_to_false", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_winning_cond_defaults_to_false));
        addSubTest("test_winning_cond_frag_limit", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_winning_cond_frag_limit));
        addSubTest("test_deathmatch_does_count_frags_with_spectators", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_does_count_frags_with_spectators));
        addSubTest("test_winning_cond_time_and_frag_limit", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_winning_cond_time_and_frag_limit));
        addSubTest("test_deathmatch_is_player_allowed_for_gameplay", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_deathmatch_is_player_allowed_for_gameplay));
        addSubTest("test_team_deathmatch_get_team_color", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_team_deathmatch_get_team_color));
        addSubTest(
            "test_team_games_do_not_count_frags_with_zero_team_id_or_in_spectator_mode",
            static_cast<PFNUNITSUBTEST>(&GameModeTest::test_team_games_do_not_count_frags_with_zero_team_id_or_in_spectator_mode));
        addSubTest("test_team_games_do_not_allow_any_team_id", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_team_games_do_not_allow_any_team_id));
        addSubTest("test_team_games_is_player_allowed_for_gameplay", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_team_games_is_player_allowed_for_gameplay));
        addSubTest("test_team_games_get_alive_team_players_count", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_team_games_get_alive_team_players_count));
        addSubTest("test_round_games_winning_cond_time_and_round_win_limit", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_winning_cond_time_and_round_win_limit));
        addSubTest("test_round_games_get_time_limit_in_current_state", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_get_time_limit_in_current_state));
        addSubTest("test_round_games_get_time_remaining_in_current_state", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_get_time_remaining_in_current_state));
        addSubTest("test_round_games_client_update_time_remaining_in_current_state_millisecs", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_client_update_time_remaining_in_current_state_millisecs));
        addSubTest("test_round_games_transition_to_play_state_after_n_seconds", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_transition_to_play_state_after_n_seconds));
        addSubTest(
            "test_round_games_round_is_won_when_a_team_dies_and_other_team_is_not_empty", 
            static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_round_is_won_when_a_team_dies_and_other_team_is_not_empty));
        addSubTest(
            "test_round_games_round_ends_without_win_when_a_team_becomes_empty_and_other_team_is_not_empty",
            static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_round_ends_without_win_when_a_team_becomes_empty_and_other_team_is_not_empty));
        addSubTest(
            "test_round_games_round_ends_without_win_when_a_team_dies_and_other_team_is_empty",
            static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_round_ends_without_win_when_a_team_dies_and_other_team_is_empty));
        addSubTest(
            "test_round_games_round_cannot_be_won_by_killing_a_team_if_fsm_is_not_in_play_state",
            static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_round_cannot_be_won_by_killing_a_team_if_fsm_is_not_in_play_state));
        addSubTest("test_round_games_player_movement_allowed", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_player_movement_allowed));
        addSubTest("test_round_games_hasJustTransitionedTo_functions_server", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_hasJustTransitionedTo_functions_server));
        addSubTest("test_round_games_hasJustTransitionedTo_functions_client", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_hasJustTransitionedTo_functions_client));
        addSubTest("test_round_games_hasJustTransitionedTo_always_detects_restart", static_cast<PFNUNITSUBTEST>(&GameModeTest::test_round_games_hasJustTransitionedTo_always_detects_restart));
    }

    virtual bool setUp() override
    {
        m_itemPickupEvents.clear();
        m_inventoryChangeEvents.clear();
        m_ammoChangeEvents.clear();

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
        m_mapPlayers.clear();
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
        gm = proofps_dd::GameMode::createGameMode(gamemode, m_mapPlayers);
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
            case proofps_dd::GameModeType::TeamRoundGame:
                // since TRG is derived from TDM, we can easily cast TRG to TDM, and keep using tdm in unit tests even when gamemode is TRG!
                dm = dynamic_cast<proofps_dd::DeathMatchMode*>(gm);
                b &= assertNotNull(dm, (std::string("dm null, ") + std::string(proofps_dd::GameMode::getGameModeTypeName(gamemode))).c_str());
                tdm = dynamic_cast<proofps_dd::TeamDeathMatchMode*>(gm);
                b &= assertNotNull(tdm, (std::string("tdm null, ") + std::string(proofps_dd::GameMode::getGameModeTypeName(gamemode))).c_str());
                trg = dynamic_cast<proofps_dd::TeamRoundGameMode*>(gm);
                b &= assertNotNull(tdm, (std::string("rgm null, ") + std::string(proofps_dd::GameMode::getGameModeTypeName(gamemode))).c_str());
                break;
            default:
                b = assertFalse(true, "invalid gamemode!");
            }
        }
        return b;
    }

private:

    // GameMode ctors require this external container, note that it is used in rare occasions only when a player attribute
    // needs to be fetched that is not stored in the GameMode's internal container, therefore in the unit tests we
    // fill this internal container rarely.
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player> m_mapPlayers;
    proofps_dd::GameMode* gm;
    proofps_dd::DeathMatchMode* dm;
    proofps_dd::TeamDeathMatchMode* tdm;
    proofps_dd::TeamRoundGameMode* trg;
    pge_audio::PgeAudio m_audio;
    PGEcfgProfiles& m_cfgProfiles;
    PgeObjectPool<PooledBullet> m_bullets;
    proofps_dd::EventLister<> m_itemPickupEvents;
    proofps_dd::EventLister<> m_inventoryChangeEvents;
    proofps_dd::EventLister<> m_ammoChangeEvents;
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
                b &= assertEquals(itExpectedPlayers->m_bSpectatorMode, itFragTable->m_bSpectatorMode, ("Spectator in row " + std::to_string(i) + " " + sLogText).c_str());
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

    /**
    * Creates 3 new players in m_mapPlayers, assigns the first 2 to Team 1, and the last player to Team 2.
    * @return True on success, false otherwise.
    */
    bool add2plus1players()
    {
        auto insertRes = m_mapPlayers.insert(
            {
                1,
                proofps_dd::Player(
                    m_audio, m_cfgProfiles, m_bullets,
                    m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                    *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1")
            }); // TODO: emplace_back()
        bool b = assertTrue(insertRes.second, "player1 insert into m_mapPlayers");
        proofps_dd::Player& player1 = insertRes.first->second;
        if (b)
        {
            player1.setName("Adam");
            player1.getFrags() = 0;
            player1.getDeaths() = 0;
            player1.getTeamId() = 1; // required non-zero teamId for TDM test, otherwise TDM does not count frags
            player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
        }

        insertRes = m_mapPlayers.insert(
            {
                2,
                proofps_dd::Player(
                    m_audio, m_cfgProfiles, m_bullets,
                    m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                    *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2")
            }); // TODO: emplace_back()
        b &= assertTrue(insertRes.second, "player2 insert into m_mapPlayers");
        proofps_dd::Player& player2 = insertRes.first->second;
        if (b)
        {
            player2.setName("Apple");
            player2.getFrags() = 2;
            player2.getDeaths() = 0;
            player2.getTeamId() = player1.getTeamId(); // yes, intentionally in same team
            player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
        }

        // 1 more player into the other team, because round game requires both teams to have non-spectator mode players for game win condition
        insertRes = m_mapPlayers.insert(
            {
                3,
                proofps_dd::Player(
                    m_audio, m_cfgProfiles, m_bullets,
                    m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                    *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.12")
            }); // TODO: emplace_back()
        b &= assertTrue(insertRes.second, "player5 insert into m_mapPlayers");
        proofps_dd::Player& player5 = insertRes.first->second;
        if (b)
        {
            player5.setName("Bela");
            player5.getFrags() = 0;
            player5.getDeaths() = 0;
            player5.getTeamId() = 2;
            player5.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
        }

        return b;
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
        b &= assertTrue(proofps_dd::GameModeType::TeamRoundGame == ++gamemode, "2");
        b &= assertTrue(proofps_dd::GameModeType::Max == ++gamemode, "3");
        b &= assertTrue(proofps_dd::GameModeType::DeathMatch == ++gamemode, "4");

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
        b &= assertFalse(gm->wasGameWonAlreadyInPreviousTick(), "game not won previous tick");
        b &= assertTrue(gm->getPlayersTable().empty(), "playerdata");
        b &= assertEquals(0u, gm->getSpectatorModePlayersCount(), "spectator mode players count");
        b &= assertFalse(gm->isTeamBasedGame(), "team based");
        b &= assertFalse(gm->isRoundBased(), "round based");
        b &= assertTrue(gm->isRespawnAllowedAfterDie(), "respawn allowed after die");
        b &= assertTrue(gm->isPlayerMovementAllowed(), "movement allowed");

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
        b &= assertFalse(gm->wasGameWonAlreadyInPreviousTick(), "game not won previous tick");
        b &= assertTrue(gm->getPlayersTable().empty(), "playerdata");
        b &= assertEquals(0u, gm->getSpectatorModePlayersCount(), "spectator mode players count");
        b &= assertTrue(gm->isTeamBasedGame(), "team based");
        b &= assertFalse(gm->isRoundBased(), "round based");
        b &= assertTrue(gm->isRespawnAllowedAfterDie(), "respawn allowed after die");
        b &= assertTrue(gm->isPlayerMovementAllowed(), "movement allowed");
    
        return b;
    }

    bool test_factory_creates_teamroundgame()
    {
        if (!testInitGamemode(proofps_dd::GameModeType::TeamRoundGame))
        {
            return assertFalse(true, "testInitGamemode fail");
        }

        bool b = assertTrue(proofps_dd::GameModeType::TeamRoundGame == gm->getGameModeType(), "gmtype");
        b &= assertEquals(std::string("Team Round Game"), gm->getGameModeTypeName(), "gmtype name");
        b &= assertEquals(0, gm->getResetTime().time_since_epoch().count(), "reset time is epoch");
        b &= assertEquals(0, gm->getWinTime().time_since_epoch().count(), "win time is epoch");
        b &= assertFalse(gm->isGameWon(), "game not won");
        b &= assertFalse(gm->wasGameWonAlreadyInPreviousTick(), "game not won previous tick");
        b &= assertTrue(gm->getPlayersTable().empty(), "playerdata");
        b &= assertEquals(0u, gm->getSpectatorModePlayersCount(), "spectator mode players count");
        b &= assertTrue(gm->isTeamBasedGame(), "team based");
        b &= assertTrue(gm->isRoundBased(), "round based");
        b &= assertFalse(gm->isRespawnAllowedAfterDie(), "respawn allowed after die");
        b &= assertFalse(gm->isPlayerMovementAllowed(), "movement allowed");
        b &= assertEquals(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(),
            "fsm state");

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

            SpecialGameMode sgm(m_mapPlayers);

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

            SpecialGameMode sgm(m_mapPlayers);

            b &= assertFalse(sgm.renamePlayer("alma", "gg"), (std::string("rename 1 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());
            b &= assertFalse(sgm.renamePlayer("", ""), (std::string("rename 2 fail, testing as ") + (bTestingAsServer ? "server" : "client")).c_str());

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 2;
            player1.getDeaths() = 0;
            //player1.isInSpectatorMode() = false; // we keep Adam in spectator mode, as it shall not influence rename (and grouping is not task of GameMode)!

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 1;
            player2.getDeaths() = 0;
            player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 0;
            player3.getDeaths() = 0;
            player3.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 0;
            player4.getDeaths() = 0;
            player4.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

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
                { /*"Adam"*/ "Peter", player1.getServerSideConnectionHandle(), 0 /* iTeamId*/, player1.isInSpectatorMode(), 2, 0},
                { "Apple", player2.getServerSideConnectionHandle(), 0 /* iTeamId*/, player2.isInSpectatorMode(), 1, 0 },
                { "Joe", player3.getServerSideConnectionHandle(), 0 /* iTeamId*/, player3.isInSpectatorMode(), 0, 0 },
                { "Banana", player4.getServerSideConnectionHandle(), 0 /* iTeamId*/, player4.isInSpectatorMode(), 0, 0 }
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

            SpecialGameMode sgm(m_mapPlayers);

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
        SpecialGameMode sgm(m_mapPlayers);

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

        SpecialGameMode sgm(m_mapPlayers);

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
        bool bPrevWonState = false;
        while ((iSleep++ < 5) && !sgm.serverCheckAndUpdateWinningConditions(m_network))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            setRemainingSecs.erase(static_cast<unsigned int>(std::floor(sgm.getTimeRemainingMillisecs() / 1000.f)));

            if (!bPrevWonState && sgm.isGameWon())
            {
                b &= assertFalse(gm->wasGameWonAlreadyInPreviousTick(), "game not won previous tick");
            }
            bPrevWonState = sgm.isGameWon();
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
        SpecialGameMode sgm(m_mapPlayers);

        bool b = assertEquals(0, sgm.getWinTime().time_since_epoch().count(), "win time 1");
        b &= assertFalse(sgm.isGameWon(), "winning state 1");
        b &= assertFalse(sgm.wasGameWonAlreadyInPreviousTick(), "game not won previous tick 1");

        sgm.clientReceiveAndUpdateWinningConditions(m_network, true);

        b &= assertLess(0, sgm.getWinTime().time_since_epoch().count(), "win time 2");
        b &= assertTrue(sgm.isGameWon(), "winning state 2");
        b &= assertFalse(sgm.wasGameWonAlreadyInPreviousTick(), "game not won previous tick 2");

        sgm.clientReceiveAndUpdateWinningConditions(m_network, false);

        b &= assertEquals(0, sgm.getWinTime().time_since_epoch().count(), "win time 3");
        b &= assertFalse(sgm.isGameWon(), "winning state 3");
        b &= assertFalse(sgm.wasGameWonAlreadyInPreviousTick(), "game not won previous tick 3");

        return b;
    }

    bool test_frag_limit_get_set(const proofps_dd::GameModeType& gamemode)
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

    bool test_frag_limit_get_set()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_frag_limit_get_set(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_round_win_limit_get_set(const proofps_dd::GameModeType& gamemode)
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

            b &= assertEqualsEz(static_cast<unsigned int>(proofps_dd::GameMode::nSvRgmRoundWinLimitDef), trg->getRoundWinLimit(), gamemode, bTestingAsServer, "default fail");
            trg->setRoundWinLimit(25u);
            b &= assertEqualsEz(25u, trg->getRoundWinLimit(), gamemode, bTestingAsServer, "new fail 1");

            // cannot set 0
            trg->setRoundWinLimit(0);
            b &= assertEqualsEz(25u, trg->getRoundWinLimit(), gamemode, bTestingAsServer, "new fail 2");
        }

        return b;
    }

    bool test_round_win_limit_get_set()
    {
        bool b = true;

        const proofps_dd::GameModeType gamemodetype = proofps_dd::GameModeType::TeamRoundGame;

        b &= assertTrue( test_round_win_limit_get_set(gamemodetype), proofps_dd::GameMode::getGameModeTypeName(gamemodetype) );

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
            b &= assertTrue( test_deathmatch_fetch_config(gamemode), proofps_dd::GameMode::getGameModeTypeName(gamemode) );
        }
        return b;
    }

    bool test_teamroundgame_fetch_config(const proofps_dd::GameModeType& gamemode)
    {
        // extension of test_deathmatch_fetch_config()

        m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvRgmRoundWinLimit].Set(10);

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
            assert(gm->isRoundBased());

            b &= assertEqualsEz(
                static_cast<unsigned int>(proofps_dd::GameMode::nSvRgmRoundWinLimitDef),
                trg->getRoundWinLimit(), gamemode, bTestingAsServer, "default round win limit fail");

            gm->fetchConfig(m_cfgProfiles, m_network);

            b &= assertEqualsEz(10u, trg->getRoundWinLimit(), gamemode, bTestingAsServer, "new round win limit fail");
        }

        return b;
    }

    bool test_teamroundgame_fetch_config()
    {
        // extension of test_deathmatch_fetch_config()
        const proofps_dd::GameModeType gamemodetype = proofps_dd::GameModeType::TeamRoundGame;
        bool b = true;

        b &= assertTrue( test_deathmatch_fetch_config(gamemodetype), proofps_dd::GameMode::getGameModeTypeName(gamemodetype) );
        b &= test_teamroundgame_fetch_config(gamemodetype);

        return b;
    }

    bool test_add_player_zero_values_maintains_adding_order(const proofps_dd::GameModeType& gamemode)
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
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 0;
            player1.getDeaths() = 0;
            player1.getTeamId() = 1;
            player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 0;
            player2.getDeaths() = 0;
            player2.getTeamId() = 2;
            //player2.isInSpectatorMode() = false; // we keep Apple in spectator mode, as it shall not influence order (and grouping is not task of GameMode)!

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 0;
            player3.getDeaths() = 0;
            player3.getTeamId() = 1;
            player3.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 0;
            player4.getDeaths() = 0;
            player4.getTeamId() = 2;
            player4.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail");
            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail");
            b &= assertTrueEz(gm->addPlayer(player3, m_network), gamemode, bTestingAsServer, "add player 3 fail");
            b &= assertTrueEz(gm->addPlayer(player4, m_network), gamemode, bTestingAsServer, "add player 4 fail");

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers = {
                { "Adam", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, player1.isInSpectatorMode(), 0, 0},
                { "Apple", player2.getServerSideConnectionHandle(), 2 /* iTeamId*/, player2.isInSpectatorMode(), 0, 0 },
                { "Joe", player3.getServerSideConnectionHandle(), 1 /* iTeamId*/, player3.isInSpectatorMode(), 0, 0 },
                { "Banana", player4.getServerSideConnectionHandle(), 2 /* iTeamId*/, player4.isInSpectatorMode(), 0, 0 }
            };

            b &= assertFragTableEqualsEz(expectedPlayers, gm->getPlayersTable(), gamemode, bTestingAsServer, "table fail");
        }

        return b;
    }

    bool test_add_player_zero_values_maintains_adding_order()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_add_player_zero_values_maintains_adding_order(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_add_player_random_values(const proofps_dd::GameModeType& gamemode)
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
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 10;
            player1.getDeaths() = 0;
            player1.getTeamId() = 1;
            player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 5;
            player2.getDeaths() = 2;
            player2.getTeamId() = 2;
            player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 8;
            player3.getDeaths() = 2;
            player3.getTeamId() = 1;
            //player3.isInSpectatorMode() = false; // we keep Joe in spectator mode, as it shall not influence order (and grouping is not task of GameMode)!

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 8;
            player4.getDeaths() = 0;
            player4.getTeamId() = 2;
            player4.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail");
            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail");
            b &= assertTrueEz(gm->addPlayer(player3, m_network), gamemode, bTestingAsServer, "add player 3 fail");
            b &= assertTrueEz(gm->addPlayer(player4, m_network), gamemode, bTestingAsServer, "add player 4 fail");

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers = {
                { "Adam", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, player1.isInSpectatorMode(), 10, 0 },
                { "Banana", player4.getServerSideConnectionHandle(), 2 /* iTeamId*/, player2.isInSpectatorMode(), 8, 0 },
                { "Joe", player3.getServerSideConnectionHandle(), 1 /* iTeamId*/, player3.isInSpectatorMode(), 8, 2 },
                { "Apple", player2.getServerSideConnectionHandle(), 2 /* iTeamId*/, player4.isInSpectatorMode(), 5, 2 }
            };

            b &= assertFragTableEqualsEz(expectedPlayers, gm->getPlayersTable(), gamemode, bTestingAsServer, "table fail");
        }

        return b;
    }

    bool test_add_player_random_values()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_add_player_random_values(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_add_player_already_existing_fails(const proofps_dd::GameModeType& gamemode)
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
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 10;
            player1.getDeaths() = 0;
            player1.getTeamId() = 1;
            player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 5;
            player2.getDeaths() = 2;
            player2.getTeamId() = 2;
            player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player3(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            player3.setName("Joe");
            player3.getFrags() = 8;
            player3.getDeaths() = 2;
            player3.getTeamId() = 1;
            player3.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player4(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(4), "192.168.1.4");
            player4.setName("Banana");
            player4.getFrags() = 8;
            player4.getDeaths() = 0;
            player4.getTeamId() = 2;
            player4.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1");
            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2");
            b &= assertTrueEz(gm->addPlayer(player3, m_network), gamemode, bTestingAsServer, "add player 3");
            b &= assertTrueEz(gm->addPlayer(player4, m_network), gamemode, bTestingAsServer, "add player 4");

            player3.setName("Joe");
            player3.getFrags() = 12;
            player3.getDeaths() = 0;
            b &= assertFalseEz(gm->addPlayer(player3, m_network), gamemode, bTestingAsServer, "add player 3 again");

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers = {
                { "Adam", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, player1.isInSpectatorMode(), 10, 0 },
                { "Banana", player4.getServerSideConnectionHandle(), 2 /* iTeamId*/, player2.isInSpectatorMode(), 8, 0 },
                { "Joe", player3.getServerSideConnectionHandle(), 1 /* iTeamId*/, player3.isInSpectatorMode(), 8, 2 },
                { "Apple", player2.getServerSideConnectionHandle(), 2 /* iTeamId*/, player4.isInSpectatorMode(), 5, 2 }
            };

            b &= assertFragTableEqualsEz(expectedPlayers, gm->getPlayersTable(), gamemode, bTestingAsServer, "table fail");

            // spectator mode shall not have influence on this either
            player3.isInSpectatorMode() = !player3.isInSpectatorMode();
            b &= assertFalseEz(gm->addPlayer(player3, m_network), gamemode, bTestingAsServer, "add player 3 again 2");

            b &= assertFragTableEqualsEz(expectedPlayers, gm->getPlayersTable(), gamemode, bTestingAsServer, "table fail 2");
        }

        return b;
    }

    bool test_add_player_already_existing_fails()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_add_player_already_existing_fails(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_add_player_sends_winning_state_only_when_game_is_already_won(const proofps_dd::GameModeType& gamemode)
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
        if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
        {
            trg->setRoundWinLimit(1);
        }

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 10;
        player1.getDeaths() = 0;
        player1.getTeamId() = 1; // required non-zero teamId for TDM test, otherwise TDM does not count frags
        player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 5;
        player2.getDeaths() = 2;
        player2.getTeamId() = 2; // required non-zero teamId for TDM test, otherwise TDM does not count frags
        player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        // player 3 only for TRG
        proofps_dd::Player player3(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
        player3.setName("Ggg");
        player3.getFrags() = 0;
        player3.getDeaths() = 0;
        player3.getTeamId() = 2; // required non-zero teamId for TDM test, otherwise TDM does not count frags
        player3.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true /*server*/, "add player 1 fail");

        // TRG sends out FSM state for any added player
        const uint32_t nExpectedTxPktCount1 = ((gamemode == proofps_dd::GameModeType::TeamRoundGame) ? 1u : 0u);
        b &= assertEqualsEz(nExpectedTxPktCount1, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count 1");
        if (b && (nExpectedTxPktCount1 != 0))
        {
            try
            {
                b &= assertEqualsEz(nExpectedTxPktCount1, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 1"
                );
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true /*server*/, "tx msg count 1");
            }
        }

        // adding same player fails so still no sent out packets
        b &= assertFalseEz(gm->addPlayer(player1, m_network), gamemode, true /*server*/, "add player 1 again fail 1");
        b &= assertEqualsEz(nExpectedTxPktCount1, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count 1");

        // game is now won, expecting 1 sent pkts to the virtually connected client (ServerStub has 1 virtual always-connected client)
        player1.getFrags()++;
        b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true /*server*/, "update player 1 fail 1");

        const uint32_t nExpectedTxPktCount2 =
            (gamemode == proofps_dd::GameModeType::TeamRoundGame) ?
            nExpectedTxPktCount1 /* team round game: does not care about frag limit */ :
            1u;

        b &= assertEqualsEz(nExpectedTxPktCount2, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count 2");
        if (b && (gamemode != proofps_dd::GameModeType::TeamRoundGame))
        {
            try
            {
                b &= assertEqualsEz(nExpectedTxPktCount2, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 2"
                );
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true /*server*/, "tx msg count 2");
            }
        }

        const uint32_t nExpectedTxPktCountTRG = 2u;
        if (b && (gamemode == proofps_dd::GameModeType::TeamRoundGame))
        {
            // need to add player3 because if any team is empty than game cannot be win
            b &= assertTrueEz(gm->addPlayer(player3, m_network), gamemode, true /*server*/, "add player 3 fail");
            b &= assertEqualsEz(nExpectedTxPktCountTRG, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count 1");
            if (b)
            {
                try
                {
                    b &= assertEqualsEz(nExpectedTxPktCountTRG, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                        gamemode, true /*server*/, "tx msg count 3"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, true /*server*/, "tx msg count 3");
                }
            }

            /* win the game by reaching round win limit: we are allowed to do this because we are not testing the mechanism of auto-incrementing won rounds now */
            trg->setTeamRoundWins(1, 1);
            trg->getFSM().transitionToPlayState(); /* forcing FSM to Play state, otherwise team round wins are not checked against round win limit */
            b &= assertEqualsEz(
                proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
                trg->getFSM().getState(),
                gamemode, true /*server*/, "FSM state 2");
            b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true /*server*/, "update player 1 fail 2");

            // we only have 1 virtual client connected in network stub, regardless of how many players are now in GameMode
            b &= assertEqualsEz(nExpectedTxPktCountTRG + 2, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count 3");
            try
            {
                b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 4"
                );
                b &= assertEqualsEz(nExpectedTxPktCountTRG + 1, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 5"
                );
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true /*server*/, "tx msg count 4 or 5");
            }
        }

        const uint32_t nExpectedTxPktCount3 = m_network.getServer().getTxPacketCount();
        // adding same player fails so number of sent pkts should not change
        b &= assertFalseEz(gm->addPlayer(player1, m_network), gamemode, true /*server*/, "add player 1 again fail 2");
        b &= assertEqualsEz(nExpectedTxPktCount3, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count 5");

        // now adding new player should trigger sending out MsgGameSessionStateFromServer to the player since game state is already won,
        // for TRG MsgGameRoundStateFromServer is sent out 2 times:
        // - 1 for addPlayer() override in TRG class,
        // - 1 for addPlayer() default behavior: game is won, serverSendGameSessionStateToClient() is invoked and it has override in TRG class.
        const uint32_t nExpectedTxPktCount4 =
            (gamemode == proofps_dd::GameModeType::TeamRoundGame) ?
            nExpectedTxPktCount3 + 3 :
            nExpectedTxPktCount3 + 1;
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true /*server*/, "add player 2 fail");
        b &= assertEqualsEz(nExpectedTxPktCount4, m_network.getServer().getTxPacketCount(), gamemode, true /*server*/, "tx pkt count 6");
        if (b)
        {
            try
            {
                b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 6"
                );
                if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
                {
                    b &= assertEqualsEz(nExpectedTxPktCountTRG + 3, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                        gamemode, true /*server*/, "tx msg count 7"
                    );
                }
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true /*server*/, "tx msg count 6 or 7");
            }
        }

        return b;
    }

    bool test_add_player_sends_winning_state_only_when_game_is_already_won()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_add_player_sends_winning_state_only_when_game_is_already_won(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
            
            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;
    }

    bool test_update_player(const proofps_dd::GameModeType& gamemode)
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
            else if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
            {
                // TRG does not care about frag limit but we are also testing that actually it does not care
                dm->setFragLimit(16);
                trg->setRoundWinLimit(1);
            }
            else
            {
                return assertTrue(false, "unhandled gamemode");
            }

            proofps_dd::Player playerAdam(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            playerAdam.setName("Adam");
            playerAdam.getFrags() = 10;
            playerAdam.getDeaths() = 0;
            playerAdam.getTeamId() = 1;
            playerAdam.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player playerApple(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            playerApple.setName("Apple");
            playerApple.getFrags() = 5;
            playerApple.getDeaths() = 2;
            playerApple.getTeamId() = 2;
            playerApple.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player playerJoe(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            playerJoe.setName("Joe");
            playerJoe.getFrags() = 4;
            playerJoe.getDeaths() = 2;
            playerJoe.getTeamId() = 1;
            playerJoe.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            b &= assertTrueEz(gm->addPlayer(playerAdam, m_network), gamemode, bTestingAsServer, "add player Adam fail");
            b &= assertTrueEz(gm->addPlayer(playerApple, m_network), gamemode, bTestingAsServer, "add player Apple fail");
            b &= assertTrueEz(gm->addPlayer(playerJoe, m_network), gamemode, bTestingAsServer, "add player Joe fail");

            playerJoe.getFrags()++;
            b &= assertTrueEz(gm->updatePlayer(playerJoe, m_network), gamemode, bTestingAsServer, "update player Joe 1 fail");
            // since Joe got same number of frags _later_ than Apple, Joe must stay behind Apple
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers1 = {
                { "Adam", playerAdam.getServerSideConnectionHandle(), 1 /* iTeamId*/, playerAdam.isInSpectatorMode(), 10, 0 },
                { "Apple", playerApple.getServerSideConnectionHandle(), 2 /* iTeamId*/, playerApple.isInSpectatorMode(), 5, 2 },
                { "Joe", playerJoe.getServerSideConnectionHandle(), 1 /* iTeamId*/, playerJoe.isInSpectatorMode(), 5, 2 }
            };
            assertFragTableEqualsEz(expectedPlayers1, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 1 fail");

            playerApple.getDeaths()++;
            b &= assertTrueEz(gm->updatePlayer(playerApple, m_network), gamemode, bTestingAsServer, "update player Apple 1 fail");
            // since Apple now has more deaths than Joe, it must goe behind Joe
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers2 = {
                { "Adam", playerAdam.getServerSideConnectionHandle(), 1 /* iTeamId*/, playerAdam.isInSpectatorMode(), 10, 0 },
                { "Joe", playerJoe.getServerSideConnectionHandle(), 1 /* iTeamId*/, playerJoe.isInSpectatorMode(), 5, 2 },
                { "Apple", playerApple.getServerSideConnectionHandle(), 2 /* iTeamId*/, playerApple.isInSpectatorMode(), 5, 3 }
            };
            assertFragTableEqualsEz(expectedPlayers2, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 2 fail");

            playerJoe.getDeaths()++;
            b &= assertTrueEz(gm->updatePlayer(playerJoe, m_network), gamemode, bTestingAsServer, "update player Joe 2 fail");
            // since Joe got same number of frags _earlier_ than Apple, and got same number for deaths _later_ than Apple, it must stay in front of Apple
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers3 = {
                { "Adam", playerAdam.getServerSideConnectionHandle(), 1 /* iTeamId*/, playerAdam.isInSpectatorMode(), 10, 0 },
                { "Joe", playerJoe.getServerSideConnectionHandle(), 1 /* iTeamId*/, playerJoe.isInSpectatorMode(), 5, 3 },
                { "Apple", playerApple.getServerSideConnectionHandle(), 2 /* iTeamId*/, playerApple.isInSpectatorMode(), 5, 3 }
            };
            assertFragTableEqualsEz(expectedPlayers3, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 3 fail");

            // reaching frag limit (frag limit is set differently for DM and TDM)
            playerAdam.getFrags()++;
            b &= assertTrueEz(gm->updatePlayer(playerAdam, m_network), gamemode, bTestingAsServer, "update player Adam 1 fail");

            if (bTestingAsServer && (gamemode == proofps_dd::GameModeType::TeamRoundGame))
            {
                // team round game does not care about frag limit
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won TRG");

                /* win the game by reaching round win limit: we are allowed to do this because we are testing updatePlayer() itself */
                trg->setTeamRoundWins(1, 1);
                trg->getFSM().transitionToPlayState(); /* forcing FSM to Play state, otherwise team round wins are not checked against round win limit */
                b &= assertEqualsEz(
                    proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
                    trg->getFSM().getState(),
                    gamemode, true /*server*/, "FSM state 1");

                // try winning the game again
                b &= assertTrueEz(gm->updatePlayer(playerAdam, m_network), gamemode, bTestingAsServer, "update player Adam 2 fail");
            }

            if (bTestingAsServer)
            {
                // game won, win time is already updated by updatePlayer() even before explicit call to serverCheckAndUpdateWinningConditions();
                // this is known only by server, client needs to be informed by server
                b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won");
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick");
                b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time fail");
                b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning server");
                
                /* TRG: 3 addPlayer()'s send out 3 MsgGameRoundStateFromServer, and we have 1 more MsgGameRoundStateFromServer send when winning the game */
                const uint32_t nExpectedTxPktCount = ((gamemode == proofps_dd::GameModeType::TeamRoundGame) ? 5u : 1u);
                b &= assertEqualsEz(nExpectedTxPktCount, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count");
                if (b)
                {
                    try
                    {
                        b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                            static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                            gamemode, bTestingAsServer, "tx msg count 1"
                        );
                        if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
                        {
                            b &= assertEqualsEz(4u, m_network.getServer().getTxMsgCount().at(
                                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                                gamemode, true /*server*/, "tx msg count 2"
                            );
                        }
                    }
                    catch (...)
                    {
                        b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count 1 or 2");
                    }
                }
            }

        }

        return b;
    }

    bool test_update_player()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue( test_update_player(gamemode), proofps_dd::GameMode::getGameModeTypeName(gamemode) );
        }
        return b;
    }

    bool test_update_player_non_existing_fails(const proofps_dd::GameModeType& gamemode)
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
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            playerAdam.setName("Adam");
            playerAdam.getFrags() = 10;
            playerAdam.getDeaths() = 0;
            playerAdam.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            b &= assertFalse(dm->updatePlayer(playerAdam, m_network), "update player Adam 1 fail");

            if (bTestingAsServer)
            {
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game not won");
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick");
                b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning");
                b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
            }

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers1;
            b &= assertFragTableEqualsEz(expectedPlayers1, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 1 fail");

            b &= assertTrueEz(gm->addPlayer(playerAdam, m_network), gamemode, bTestingAsServer, "add player Adam fail");

            proofps_dd::Player playerJoe(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            playerJoe.setName("Joe");
            playerJoe.getFrags() = 4;
            playerJoe.getDeaths() = 2;
            playerJoe.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
            b &= assertFalseEz(gm->updatePlayer(playerJoe, m_network), gamemode, bTestingAsServer, "update player Joe 1 fail");
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers2 = {
                { "Adam", playerAdam.getServerSideConnectionHandle(), 0 /* iTeamId*/, playerAdam.isInSpectatorMode(), 10, 0 }
            };
            b &= assertFragTableEqualsEz(expectedPlayers2, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 2 fail");

        }

        return b;
    }

    bool test_update_player_non_existing_fails()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue( test_update_player_non_existing_fails(gamemode), proofps_dd::GameMode::getGameModeTypeName(gamemode) );
        }
        return b;
    }

    bool test_remove_player(const proofps_dd::GameModeType& gamemode)
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
            if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
            {
                trg->setRoundWinLimit(1);
            }

            proofps_dd::Player playerAdam(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            playerAdam.setName("Adam");
            playerAdam.getFrags() = dm->getFragLimit();
            playerAdam.getDeaths() = 0;
            playerAdam.getTeamId() = 1; // required non-zero teamId for TDM test, otherwise TDM does not count frags
            playerAdam.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            b &= assertFalseEz(gm->removePlayer(playerAdam, m_network), gamemode, bTestingAsServer, "removep player Adam 1 fail");

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers1;
            b &= assertFragTableEqualsEz(expectedPlayers1, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 1 fail");

            // here game state becomes won for non-round-games
            b &= assertTrueEz(gm->addPlayer(playerAdam, m_network), gamemode, bTestingAsServer, "add player Adam fail");
            if (bTestingAsServer)
            {
                if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
                {
                    b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 1 TRG");
                }
                else
                {
                    b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 1");
                }
            }

            proofps_dd::Player playerJoe(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
            playerJoe.setName("Joe");
            playerJoe.getFrags() = 4;
            playerJoe.getDeaths() = 2;
            playerJoe.getTeamId() = 2; // required non-zero teamId for TDM test, otherwise TDM does not count frags
            playerJoe.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
            b &= assertFalseEz(gm->removePlayer(playerJoe, m_network), gamemode, bTestingAsServer, "remove player Joe 1 fail");
            const std::vector<proofps_dd::PlayersTableRow> expectedPlayers2 = {
                { "Adam", playerAdam.getServerSideConnectionHandle(), 1 /* iTeamId*/, playerAdam.isInSpectatorMode(), 10, 0 }
            };
            b &= assertFragTableEqualsEz(expectedPlayers2, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 2 fail");

            if (bTestingAsServer && (gamemode == proofps_dd::GameModeType::TeamRoundGame))
            {
                // team round game does not care about frag limit so we have to set this to see if removePlayer() works fine
                trg->setTeamRoundWins(2, 1);
                trg->getFSM().transitionToPlayState(); /* forcing FSM to Play state, otherwise team round wins are not checked against round win limit */
                b &= assertEqualsEz(
                    proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
                    trg->getFSM().getState(),
                    gamemode, true /*server*/, "FSM state 1");

                // no round win can be achieved if any team is empty at the moment of winning!
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 2 TRG");
            }
            
            b &= assertTrueEz(gm->removePlayer(playerAdam, m_network), gamemode, bTestingAsServer, "remove player Adam 1 fail");
            b &= assertFragTableEqualsEz(expectedPlayers1, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 3 fail");

            if (bTestingAsServer && (gamemode != proofps_dd::GameModeType::TeamRoundGame))
            {
                // even though winner player is removed, winning condition stays true, win time is still valid, an explicit reset() would be needed to clear them!
                b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 3");
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick");
                b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning");
            }

            if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
            {
                // for TRG removePlayer() we actually have to add enough players to both teams because game cannot be won
                // by hitting round win limit if any team is empty!
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 3 TRG");

                // temporarily decrease win count otherwise addPlayer() would trigger the win
                trg->setTeamRoundWins(2, 0);

                b &= assertTrueEz(gm->addPlayer(playerAdam, m_network), gamemode, bTestingAsServer, "add player Adam fail 2");
                b &= assertTrueEz(gm->addPlayer(playerJoe, m_network), gamemode, bTestingAsServer, "add player Joe fail 2");

                proofps_dd::Player playerBanana(
                    m_audio, m_cfgProfiles, m_bullets,
                    m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                    *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.12");
                playerBanana.setName("Banana");
                playerBanana.getFrags() = 4;
                playerBanana.getDeaths() = 1;
                playerBanana.getTeamId() = 2;
                playerBanana.isInSpectatorMode() = false;
                b &= assertTrueEz(gm->addPlayer(playerBanana, m_network), gamemode, bTestingAsServer, "add player Banana fail 2");

                const std::vector<proofps_dd::PlayersTableRow> expectedPlayers3 = {
                    { "Adam", playerAdam.getServerSideConnectionHandle(), 1 /* iTeamId*/, playerAdam.isInSpectatorMode(), 10, 0 },
                    { "Banana", playerBanana.getServerSideConnectionHandle(), 2 /* iTeamId*/, playerBanana.isInSpectatorMode(), 4, 1 },
                    { "Joe", playerJoe.getServerSideConnectionHandle(), 2 /* iTeamId*/, playerJoe.isInSpectatorMode(), 4, 2 }
                };
                b &= assertFragTableEqualsEz(expectedPlayers3, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 3 fail");

                // now set win condition again and test removePlayer() can finally detect it
                trg->setTeamRoundWins(2, 1);
                b &= assertTrueEz(gm->removePlayer(playerBanana, m_network), gamemode, bTestingAsServer, "remove player Banana 1 fail");
                const std::vector<proofps_dd::PlayersTableRow> expectedPlayers4 = {
                    { "Adam", playerAdam.getServerSideConnectionHandle(), 1 /* iTeamId*/, playerAdam.isInSpectatorMode(), 10, 0 },
                    { "Joe", playerJoe.getServerSideConnectionHandle(), 2 /* iTeamId*/, playerJoe.isInSpectatorMode(), 4, 2 }
                };
                b &= assertFragTableEqualsEz(expectedPlayers4, gm->getPlayersTable(), gamemode, bTestingAsServer, "table 3 fail");
                
                if (bTestingAsServer)
                {
                    b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 4 TRG");
                    b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick TRG");
                    b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time TRG");
                    b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning TRG");
                }

                // even though 1 more player is removed and 1 team becomes empty, winning condition stays true, win time is still valid,
                // an explicit reset() would be needed to clear them!
                b &= assertTrueEz(gm->removePlayer(playerAdam, m_network), gamemode, bTestingAsServer, "remove player Adam 2 fail");

                if (bTestingAsServer)
                {
                    b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 5 TRG");
                    b &= assertTrueEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick TRG 2");
                    b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time TRG 2");
                    b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning TRG 2");
                }
            }
        }

        return b;
    }

    bool test_remove_player()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue( test_remove_player(gamemode), proofps_dd::GameMode::getGameModeTypeName(gamemode) );
        }
        return b;
    }

    bool test_restart_deathmatches(const proofps_dd::GameModeType& gamemode)
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

            assert(!gm->isRoundBased());

            gm->setTimeLimitSecs(25u);
            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(15u);

            // need to restart to correctly initialize time-specific values if we have time limit!
            gm->restart(m_network);
           
            if (bTestingAsServer)
            {
                // restart triggers MsgGameSessionStateFromServer out no matter current state
                b &= assertEqualsEz(1u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 1");
                try
                {
                    b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count 1"
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count 1");
                }
            }

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 15;
            player1.getDeaths() = 0;
            player1.getTeamId() = 1;
            player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 5;
            player2.getDeaths() = 2;
            player2.getTeamId() = 2;
            player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail 1");

            if (bTestingAsServer)
            {
                b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 1");
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 1");
                b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 1");
                b &= assertEqualsEz(2u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 2");
                try
                {
                    b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count 2"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count 2");
                }
            }

            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail");
            if (bTestingAsServer)
            {
                // in case of server instance, addPlayer() sends MsgGameSessionStateFromServer to newly added player when isGameWon() is true 
                b &= assertEqualsEz(3u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 2");
                if (b)
                {
                    try
                    {
                        b &= assertEqualsEz(3u, m_network.getServer().getTxMsgCount().at(
                            static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                            gamemode, bTestingAsServer, "tx msg count 2"
                        );
                    }
                    catch (...)
                    {
                        b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count 2");
                    }
                }
            }

            gm->restart(m_network);

            b &= assertEqualsEz(25u, gm->getTimeLimitSecs(), gamemode, bTestingAsServer, "time limit fail");
            b &= assertEqualsEz(15u, dm->getFragLimit(), gamemode, bTestingAsServer, "frag limit fail");

            if (bTestingAsServer)
            {
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 2");
                b &= assertTrueEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 2");
                b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertLessEz(0, gm->getResetTime().time_since_epoch().count(), gamemode, bTestingAsServer, "reset time");
                b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 2");

                // outgoing packet for winning state true -> false transition too
                b &= assertEqualsEz(4u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count");
                if (b)
                {
                    try
                    {
                        b &= assertEqualsEz(4u, m_network.getServer().getTxMsgCount().at(
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
            b &= assertTrueEz(gm->getPlayersTable().empty(), gamemode, bTestingAsServer, "players empty fail");

        }

        return b;
    }

    bool test_restart_deathmatches()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::TeamRoundGame; ++gamemode)
        {
            b &= assertTrue(test_restart_deathmatches(gamemode), proofps_dd::GameMode::getGameModeTypeName(gamemode) );
        }
        return b;
    }

    bool test_restart_roundgames(const proofps_dd::GameModeType& gamemode)
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

            assert(gm->isRoundBased());

            gm->setTimeLimitSecs(25u);
            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(15u);
            trg->setRoundWinLimit(1);

            // need to restart to correctly initialize time-specific values if we have time limit!
            gm->restart(m_network);

            if (bTestingAsServer)
            {
                // server stub always has 1 client connected
                b &= assertEqualsEz(2u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 1");
                try
                {
                    b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count 1"
                    );
                    b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                        gamemode, true /*server*/, "tx msg count 2"
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count 1 or 2");
                }
            }

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getFrags() = 15;
            player1.getDeaths() = 0;
            player1.getTeamId() = 1;
            player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getFrags() = 5;
            player2.getDeaths() = 2;
            player2.getTeamId() = 2;
            player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

            /* win the game by reaching round win limit: we are allowed to do this because we are not testing the mechanism of auto-incrementing won rounds now */
            trg->setTeamRoundWins(1, 1);
            trg->getFSM().transitionToPlayState(); /* forcing FSM to Play state, otherwise team round wins are not checked against round win limit */
            b &= assertEqualsEz(
                proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
                trg->getFSM().getState(),
                gamemode, true /*server*/, "FSM state 1");
            
            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail 1");
            // cannot win yet, other team has 0 players
            b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 0");

            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail 1");

            if (bTestingAsServer)
            {
                b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 1");
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 1");
                b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 1");
                b &= assertEqualsEz(6u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 2");
                try
                {
                    b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count 3"
                    );
                    // addPlayer() also sends out MsgGameRoundState
                    b &= assertEqualsEz(4u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                        gamemode, true /*server*/, "tx msg count 4"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count 3 or 4");
                }
            }

            gm->restart(m_network);

            b &= assertEqualsEz(25u, gm->getTimeLimitSecs(), gamemode, bTestingAsServer, "time limit fail");
            b &= assertEqualsEz(15u, dm->getFragLimit(), gamemode, bTestingAsServer, "frag limit fail");
            b &= assertEqualsEz(1u, trg->getRoundWinLimit(), gamemode, bTestingAsServer, "round win limit fail");

            if (bTestingAsServer)
            {
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 2");
                b &= assertTrueEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 2");
                b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertLessEz(0, gm->getResetTime().time_since_epoch().count(), gamemode, bTestingAsServer, "reset time");
                b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, bTestingAsServer, "team round wins 1");
                b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, bTestingAsServer, "team round wins 2");
 
                b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 2");

                // outgoing packet for winning state true -> false transition too
                b &= assertEqualsEz(8u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 3");
                if (b)
                {
                    try
                    {
                        b &= assertEqualsEz(3u, m_network.getServer().getTxMsgCount().at(
                            static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                            gamemode, bTestingAsServer, "tx msg count 5"
                        );
                        b &= assertEqualsEz(5u, m_network.getServer().getTxMsgCount().at(
                            static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                            gamemode, true /*server*/, "tx msg count 6"
                        );
                    }
                    catch (...)
                    {
                        b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count 5 or 6");
                    }
                }
                b &= assertEqualsEz(
                    proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
                    trg->getFSM().getState(),
                    gamemode, bTestingAsServer, "FSM state 2");
            }
            b &= assertTrueEz(gm->getPlayersTable().empty(), gamemode, bTestingAsServer, "players empty fail");

        }
        
        return b;
    }

    bool test_restart_roundgames()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::TeamRoundGame; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(test_restart_roundgames(gamemode), proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_restart_deathmatches_without_removing_players(const proofps_dd::GameModeType& gamemode)
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

            assert(!gm->isRoundBased());

            gm->setTimeLimitSecs(25u);
            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(15u);

            // need to restart to correctly initialize time-specific values if we have time limit!
            gm->restartWithoutRemovingPlayers(m_network);

            if (bTestingAsServer)
            {
                // restart triggers MsgGameSessionStateFromServer out no matter current state
                b &= assertEqualsEz(1u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 1");
                try
                {
                    b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count 1"
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count 1");
                }
            }

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getTeamId() = 1;
            player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
            player1.getFrags() = 15;
            player1.getDeaths() = 0;
            player1.getSuicides() = 0;
            player1.getFiringAccuracy() = 1.f;
            player1.getShotsFiredCount() = 20;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getTeamId() = 2;
            player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
            player2.getFrags() = 5;
            player2.getDeaths() = 2;
            player2.getSuicides() = 1;
            player2.getFiringAccuracy() = 0.5f;
            player2.getShotsFiredCount() = 10;

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail 1");

            if (bTestingAsServer)
            {
                b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 1");
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 1");
                b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 1");
                b &= assertEqualsEz(2u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 2");
                try
                {
                    b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count 2"
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count 2");
                }
            }

            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail");
            if (bTestingAsServer)
            {
                // in case of server instance, addPlayer() sends MsgGameSessionStateFromServer to newly added player when isGameWon() is true 
                b &= assertEqualsEz(3u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 3");
                if (b)
                {
                    try
                    {
                        b &= assertEqualsEz(3u, m_network.getServer().getTxMsgCount().at(
                            static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                            gamemode, bTestingAsServer, "tx msg count 3"
                        );
                    }
                    catch (...)
                    {
                        b &= assertFalse(true, "tx msg count 3");
                    }
                }
            }

            gm->restartWithoutRemovingPlayers(m_network);

            b &= assertEqualsEz(25u, gm->getTimeLimitSecs(), gamemode, bTestingAsServer, "time limit fail");
            b &= assertEqualsEz(15u, dm->getFragLimit(), gamemode, bTestingAsServer, "frag limit fail");

            if (bTestingAsServer)
            {
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 2");
                b &= assertTrueEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 2");
                b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertLessEz(0, gm->getResetTime().time_since_epoch().count(), gamemode, bTestingAsServer, "reset time");
                b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 2");

                // outgoing packet for winning state true -> false transition too
                b &= assertEqualsEz(4u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 4");
                if (b)
                {
                    try
                    {
                        b &= assertEqualsEz(4u, m_network.getServer().getTxMsgCount().at(
                            static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                            gamemode, bTestingAsServer, "tx msg count 4"
                        );
                    }
                    catch (...)
                    {
                        b &= assertFalseEz(true, gamemode, bTestingAsServer, "tx msg count 4");
                    }
                }
            }

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayersStillThereWithResetStats = {
                { "Adam", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, true /* must spectate after restart */, 0, 0 /* and rest are zeroed too */},
                { "Apple", player2.getServerSideConnectionHandle(), 2 /* iTeamId*/, true /* must spectate after restart */, 0, 0 /* and rest are zeroed too */}
            };

            b &= assertFragTableEqualsEz(expectedPlayersStillThereWithResetStats, gm->getPlayersTable(), gamemode, bTestingAsServer, "table fail");

        }

        return b;
    }

    bool test_restart_deathmatches_without_removing_players()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::TeamRoundGame; ++gamemode)
        {
            b &= assertTrue(
                test_restart_deathmatches_without_removing_players(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_restart_roundgames_without_removing_players(const proofps_dd::GameModeType& gamemode)
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

            assert(gm->isRoundBased());

            gm->setTimeLimitSecs(25u);
            // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
            // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.
            dm->setFragLimit(15u);
            trg->setRoundWinLimit(1);

            // need to restart to correctly initialize time-specific values if we have time limit!
            gm->restartWithoutRemovingPlayers(m_network);

            if (bTestingAsServer)
            {
                // restart triggers MsgGameSessionStateFromServer out no matter current state
                b &= assertEqualsEz(2u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 1");
                try
                {
                    b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count 1"
                    );
                    b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                        gamemode, true /*server*/, "tx msg count 2"
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count 1 or 2");
                }
            }

            proofps_dd::Player player1(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
            player1.setName("Adam");
            player1.getTeamId() = 1;
            player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
            player1.getFrags() = 15;
            player1.getDeaths() = 0;
            player1.getSuicides() = 0;
            player1.getFiringAccuracy() = 1.f;
            player1.getShotsFiredCount() = 20;

            proofps_dd::Player player2(
                m_audio, m_cfgProfiles, m_bullets,
                m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
            player2.setName("Apple");
            player2.getTeamId() = 2;
            player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
            player2.getFrags() = 5;
            player2.getDeaths() = 2;
            player2.getSuicides() = 1;
            player2.getFiringAccuracy() = 0.5f;
            player2.getShotsFiredCount() = 10;

            /* win the game by reaching round win limit: we are allowed to do this because we are not testing the mechanism of auto-incrementing won rounds now */
            trg->setTeamRoundWins(1, 1);
            trg->getFSM().transitionToPlayState(); /* forcing FSM to Play state, otherwise team round wins are not checked against round win limit */
            b &= assertEqualsEz(
                proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
                trg->getFSM().getState(),
                gamemode, true /*server*/, "FSM state 1");

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail 1");
            // cannot win yet, other team has 0 players
            b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 0");

            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail 1");

            if (bTestingAsServer)
            {
                b &= assertTrueEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 1");
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 1");
                b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 1");
                b &= assertEqualsEz(6u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 2");
                try
                {
                    b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, bTestingAsServer, "tx msg count 3"
                    );
                    // addPlayer()'s also send out MsgGameRoundState
                    b &= assertEqualsEz(4u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                        gamemode, true /*server*/, "tx msg count 4"
                    );
                }
                catch (...)
                {
                    b &= assertFalse(true, "tx msg count 3 or 4");
                }
            }

            gm->restartWithoutRemovingPlayers(m_network);

            b &= assertEqualsEz(25u, gm->getTimeLimitSecs(), gamemode, bTestingAsServer, "time limit fail");
            b &= assertEqualsEz(15u, dm->getFragLimit(), gamemode, bTestingAsServer, "frag limit fail");
            b &= assertEqualsEz(1u, trg->getRoundWinLimit(), gamemode, bTestingAsServer, "round win limit fail");

            if (bTestingAsServer)
            {
                b &= assertFalseEz(gm->isGameWon(), gamemode, bTestingAsServer, "game won 2");
                b &= assertTrueEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 2");
                b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, bTestingAsServer, "win time");
                b &= assertLessEz(0, gm->getResetTime().time_since_epoch().count(), gamemode, bTestingAsServer, "reset time");
                b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, bTestingAsServer, "team round wins 1");
                b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, bTestingAsServer, "team round wins 2");
                b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, bTestingAsServer, "winning 2");

                // outgoing packet for winning state true -> false transition too
                b &= assertEqualsEz(8u, m_network.getServer().getTxPacketCount(), gamemode, bTestingAsServer, "tx pkt count 3");
                if (b)
                {
                    try
                    {
                        b &= assertEqualsEz(3u, m_network.getServer().getTxMsgCount().at(
                            static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                            gamemode, bTestingAsServer, "tx msg count 5"
                        );
                        b &= assertEqualsEz(5u, m_network.getServer().getTxMsgCount().at(
                            static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                            gamemode, true /*server*/, "tx msg count 6"
                        );
                    }
                    catch (...)
                    {
                        b &= assertFalse(true, "tx msg count 5 or 6");
                    }
                }
                b &= assertEqualsEz(
                    proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
                    trg->getFSM().getState(),
                    gamemode, bTestingAsServer, "FSM state 2");
            }

            const std::vector<proofps_dd::PlayersTableRow> expectedPlayersStillThereWithResetStats = {
                { "Adam", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, true /* must spectate after restart */, 0, 0 /* and rest are zeroed too */},
                { "Apple", player2.getServerSideConnectionHandle(), 2 /* iTeamId*/, true /* must spectate after restart */, 0, 0 /* and rest are zeroed too */}
            };

            b &= assertFragTableEqualsEz(expectedPlayersStillThereWithResetStats, gm->getPlayersTable(), gamemode, bTestingAsServer, "table fail");

        }

        return b;
    }

    bool test_restart_roundgames_without_removing_players()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::TeamRoundGame; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_restart_roundgames_without_removing_players(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));
        }
        return b;
    }

    bool test_winning_cond_defaults_to_false(const proofps_dd::GameModeType& gamemode)
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

    bool test_winning_cond_defaults_to_false()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_winning_cond_defaults_to_false(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));

            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;
    }

    bool test_winning_cond_frag_limit(const proofps_dd::GameModeType& gamemode, bool playersSpectatorMode)
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
        else if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
        {
            // TRG does not care about frag limit but we are also testing that actually it does not care
            dm->setFragLimit(7);
            trg->setRoundWinLimit(1);
        }
        else
        {
            return assertTrue(false, "unhandled gamemode");
        }

        gm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        const uint32_t nExpectedTxPktCount1 = ((gamemode == proofps_dd::GameModeType::TeamRoundGame) ? 2u : 1u);
        bool b = assertEqualsEz(nExpectedTxPktCount1, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true/*server*/, "tx msg count 1"
            );
            if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
            {
                b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 2"
                );
            }
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count 1 or 2");
        }

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 1;
        player1.isInSpectatorMode() = playersSpectatorMode;

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Adam");
        player2.getFrags() = 2;
        player2.getDeaths() = 0;
        player2.getTeamId() = player1.getTeamId(); // yes, intentionally in same team
        player2.isInSpectatorMode() = playersSpectatorMode;

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");

        bool bPrevWonState = false;
        unsigned int i = 0;
        while (!gm->isGameWon() && (i < 5))
        {
            i++;
            player1.getFrags()++;
            b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player");

            if (!bPrevWonState && gm->isGameWon())
            {
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 2");
            }
            bPrevWonState = gm->isGameWon();
        }

        // in both DM and TDM same amount of frags needed to be collected by player 1, since frag limit is different!
        b &= assertEqualsEz(5u, i, gamemode, true/*server*/, "frags collected");

        if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
        {
            b &= assertFalseEz(gm->isGameWon(), gamemode, true/*server*/, "game won 2 trg");
            b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning trg");
        }
        else
        {
            b &= assertTrueEz(gm->isGameWon(), gamemode, true/*server*/, "game won 2 dm");
            b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time");
            b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning dm");
        }

        /* TRG already sent out 2 pkts at the beginning, DM and TDM sent only 1 but they won the game now so now 2 pkts for them,
           however addPlayer()'s also send out MsgGameRoundState in case of TRG */
        const uint32_t nExpectedTxPktCount2 = ((gamemode == proofps_dd::GameModeType::TeamRoundGame) ? 4u : 2u);
        b &= assertEqualsEz(nExpectedTxPktCount2, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count 2");
        if (b)
        {
            const uint32_t nExpectedTxMsgSessionStateCount = ((gamemode == proofps_dd::GameModeType::TeamRoundGame) ? 1u : 2u);
            try
            {
                b &= assertEqualsEz(nExpectedTxMsgSessionStateCount, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true/*server*/, "tx msg count 3"
                );
                if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
                {
                    b &= assertEqualsEz(3u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                        gamemode, true /*server*/, "tx msg count 4"
                    );
                }
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count 3 or 4");
            }
        }

        return b;
    }

    bool test_winning_cond_frag_limit()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_winning_cond_frag_limit(gamemode, false /* players spectator mode */),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));

            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;
    }

    bool test_deathmatch_does_count_frags_with_spectators()
    {
        // serverCheckAndUpdateWinningConditions() will detect win even when all players are spectators, because GameMode does not care if
        // the player's frag count is increased in spectator mode!
        // It is the game's responsibility not to let increase frags of a spectator player!
        // This test case basically demonstrates this behavior.

        // Note that unlike in test_winning_cond_frag_limit(), here we are not testing different game modes in loop.
        // Reason: DM will behave as expected. However, in TDM the game state will stay not yet won. Because in TDM, the team summed frags
        // are checked, but team summed frags automatically exclude spectators. In DM though, blindly the topmost player's frags is checked,
        // which can be increased anytime in GameMode even for a spectator player.

        bool b = true;
        b &= assertTrue(
            test_winning_cond_frag_limit(proofps_dd::GameModeType::DeathMatch, true /* players spectator state */),
            proofps_dd::GameMode::getGameModeTypeName(proofps_dd::GameModeType::DeathMatch));

        return b;
    }

    bool test_winning_cond_time_and_frag_limit(const proofps_dd::GameModeType& gamemode)
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
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Adam");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 1; // required non-zero teamId for TDM test, otherwise TDM does not count frags
        player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Apple");
        player2.getFrags() = 2;
        player2.getDeaths() = 0;
        player2.getTeamId() = player1.getTeamId(); // yes, intentionally in same team
        player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

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
        else if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
        {
            // TRG does not care about frag limit but we are also testing that actually it does not care
            dm->setFragLimit(7);
            trg->setRoundWinLimit(1);
        }
        else
        {
            return assertTrue(false, "unhandled gamemode");
        }

        gm->setTimeLimitSecs(2);

        // need restart to properly initialize some values when we have time limit set!
        gm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        const uint32_t nExpectedTxPktCount1 = ((gamemode == proofps_dd::GameModeType::TeamRoundGame) ? 2u : 1u);
        bool b = assertEqualsEz(nExpectedTxPktCount1, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count");
        try
        {
            b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                gamemode, true/*server*/, "tx msg count 1"
            );
            if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
            {
                b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 2"
                );
            }
        }
        catch (...)
        {
            b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count 1 or 2");
        }

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");

        // time limit elapse also means winning even if frag limit not reached
        std::set<unsigned int> setRemainingSecs = { 0, 1 };
        int iSleep = 0;
        bool bPrevWonState = false;
        while ((iSleep++ < 5) && !gm->serverCheckAndUpdateWinningConditions(m_network))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            setRemainingSecs.erase(static_cast<unsigned int>(std::floor(gm->getTimeRemainingMillisecs() / 1000.f)));

            if (!bPrevWonState && gm->isGameWon())
            {
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 1");
            }
            bPrevWonState = gm->isGameWon();
        }

        b &= assertTrueEz(gm->isGameWon(), gamemode, true/*server*/, "game won 1");
        const auto durationSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - gm->getResetTime());
        b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time 1");
        b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning due to time");
        b &= assertLequalsEz(static_cast<std::chrono::seconds::rep>(gm->getTimeLimitSecs()), durationSecs.count(), gamemode, true/*server*/, "time limit elapsed");
        b &= assertTrueEz(setRemainingSecs.empty(), gamemode, true/*server*/, "no remaining");

        const uint32_t nExpectedTxPktCount2 = ((gamemode == proofps_dd::GameModeType::TeamRoundGame) ? 6u : 2u);
        b &= assertEqualsEz(nExpectedTxPktCount2, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count 2");
        if (b)
        {
            try
            {
                b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true/*server*/, "tx msg count 3"
                );
                if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
                {
                    // addPlayer()'s also send out MsgGameRoundState
                    b &= assertEqualsEz(4u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                        gamemode, true /*server*/, "tx msg count 4"
                    );
                }
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count 3 or 4");
            }
        }

        // frag limit reach also means winning even if time limit not reached
        gm->setTimeLimitSecs(100);
        gm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        const uint32_t nExpectedTxPktCount3 = ((gamemode == proofps_dd::GameModeType::TeamRoundGame) ? nExpectedTxPktCount2 + 2 : nExpectedTxPktCount2 + 1);
        b &= assertEqualsEz(nExpectedTxPktCount3, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count 3");
        if (b)
        {
            try
            {
                b &= assertEqualsEz(3u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true/*server*/, "tx msg count 5"
                );
                if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
                {
                    b &= assertEqualsEz(5u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                        gamemode, true /*server*/, "tx msg count 6"
                    );
                }
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count 5 or 6");
            }
        }

        b &= assertFalseEz(gm->isGameWon(), gamemode, true/*server*/, "game won 2");
        b &= assertTrueEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 2");
        b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time 2");

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");

        unsigned int i = 0;
        bPrevWonState = false;
        while (!gm->isGameWon() && (i < 5))
        {
            i++;
            player1.getFrags()++;
            b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player");

            if (!bPrevWonState && gm->isGameWon())
            {
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 3");
            }
            bPrevWonState = gm->isGameWon();
        }

        // in both DM and TDM same amount of frags needed to be collected by player 1, since frag limit is different!
        b &= assertEqualsEz(5u, i, gamemode, true/*server*/, "frags collected");
        
        if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
        {
            b &= assertFalseEz(gm->isGameWon(), gamemode, true/*server*/, "game won 3 trg");
            b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning due to frags trg");
        }
        else
        {
            b &= assertTrueEz(gm->isGameWon(), gamemode, true/*server*/, "game won 3 dm");
            b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time");
            b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning due to frags dm");
        }

        const uint32_t nExpectedTxPktCount4 = ((gamemode == proofps_dd::GameModeType::TeamRoundGame) ? nExpectedTxPktCount3 + 2 : nExpectedTxPktCount3 + 1);
        b &= assertEqualsEz(nExpectedTxPktCount4, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count 4");
        if (b)
        {
            const uint32_t nExpectedTxMsgGameSessionStateCount =
                ((gamemode == proofps_dd::GameModeType::TeamRoundGame) ? 3u /* no change */ : 4u);
            try
            {
                b &= assertEqualsEz(nExpectedTxMsgGameSessionStateCount, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true/*server*/, "tx msg count 7"
                );
                if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
                {
                    // addPlayer()'s also send out MsgGameRoundState
                    b &= assertEqualsEz(7u, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                        gamemode, true /*server*/, "tx msg count 8"
                    );
                }
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count 7 or 8");
            }
        }

        return b;
    }

    bool test_winning_cond_time_and_frag_limit()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_winning_cond_time_and_frag_limit(gamemode),
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
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 0;

        bool b = true;

        // spectator mode player not allowed to play
        b &= assertFalseEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 0");
        player1.isInSpectatorMode() = false;
        
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

    bool test_team_games_do_not_count_frags_with_zero_team_id_or_in_spectator_mode(const proofps_dd::GameModeType& gamemode)
    {
        // in this test we also test getTeamFrags() and getTeamPlayersCount()

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
        assert(gm->isTeamBasedGame());

        // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
        // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.

        // Basically TDM and DM classes' behavior is very similar, even though players can be in different teams, it does not matter. We just dont care.
        // We just care about frag limit being reached or not, that is why serverCheckAndUpdateWinningConditions() can be overridden.
        // GUI will take care of displaying the players, and it can still simply iterate over the players in the order as DM or TDM is containing the players,
        // no further logic is needed in GUI, just separate them by team ID!
        dm->setFragLimit(7);

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 1;
        player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        proofps_dd::Player player2(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(2), "192.168.1.2");
        player2.setName("Adam");
        player2.getFrags() = 2;
        player2.getDeaths() = 0;
        player2.getTeamId() = player1.getTeamId(); // yes, intentionally in same team
        player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        // we do not test with player who is not in spectator mode but has no team because it cannot happen since spectator mode is default mode, exiting from it
        // in TDM and TRG can be done only be selecting team!

        proofps_dd::Player player3_spectate_and_team_unassigned(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.3");
        player3_spectate_and_team_unassigned.setName("Joe");
        player3_spectate_and_team_unassigned.getFrags() = dm->getFragLimit();  // game would be already won if this player had a valid teamId!
        player3_spectate_and_team_unassigned.getDeaths() = 0;
        player3_spectate_and_team_unassigned.getTeamId() = 0; // intentionally zero, meaning no team selected!

        proofps_dd::Player player4_team_assigned_but_in_spectator_mode(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.4");
        player4_team_assigned_but_in_spectator_mode.setName("Banana");
        player4_team_assigned_but_in_spectator_mode.getFrags() = dm->getFragLimit();  // game would be already won if this player was not in spectator mode!
        player4_team_assigned_but_in_spectator_mode.getDeaths() = 0;
        player4_team_assigned_but_in_spectator_mode.getTeamId() = 2;

        // 1 more player into the other team, because round game requires both teams to have non-spectator mode players for game win condition
        proofps_dd::Player player5(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(3), "192.168.1.12");
        player5.setName("Bela");
        player5.getFrags() = 0;
        player5.getDeaths() = 0;
        player5.getTeamId() = 2;
        player5.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        bool b = true;
        b &= assertEquals(0, tdm->getTeamFrags(0), "team 0 frags 1");  // team 0 always 0 summed frags
        b &= assertEquals(0, tdm->getTeamFrags(1), "team 1 frags 1");
        b &= assertEquals(0, tdm->getTeamFrags(2), "team 2 frags 1");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(0), "team 0 players count 1"); // team 0 always has 0 players
        b &= assertEquals(0u, tdm->getTeamPlayersCount(1), "team 1 players count 1");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(2), "team 2 players count 1");
        b &= assertEquals(0u, tdm->getSpectatorModePlayersCount(), "spectator mode players count 1");
        
        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");
        b &= assertTrueEz(gm->addPlayer(player3_spectate_and_team_unassigned, m_network), gamemode, true/*server*/, "add player 3");
        b &= assertTrueEz(gm->addPlayer(player4_team_assigned_but_in_spectator_mode, m_network), gamemode, true/*server*/, "add player 4");
        b &= assertTrueEz(gm->addPlayer(player5, m_network), gamemode, true/*server*/, "add player 5");
        b &= assertEquals(0, tdm->getTeamFrags(0), "team 0 frags 2");  // team 0 always 0 summed frags
        b &= assertEquals(2, tdm->getTeamFrags(1), "team 1 frags 2");
        b &= assertEquals(0, tdm->getTeamFrags(2), "team 2 frags 2");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(0), "team 0 players count 2"); // team 0 always has 0 players
        b &= assertEquals(2u, tdm->getTeamPlayersCount(1), "team 1 players count 2");
        b &= assertEquals(1u, tdm->getTeamPlayersCount(2), "team 2 players count 2");
        b &= assertEquals(2u, tdm->getSpectatorModePlayersCount(), "spectator mode players count 2");

        unsigned int i = 0;
        bool bPrevWonState = false;
        while (!gm->isGameWon() && (i < 5))
        {
            i++;
            player1.getFrags()++;
            b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player");

            if (!bPrevWonState && gm->isGameWon())
            {
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick");
            }
            bPrevWonState = gm->isGameWon();
        }

        b &= assertEquals(0, tdm->getTeamFrags(0), "team 0 frags 3");  // team 0 always 0 summed frags
        b &= assertEquals(7, tdm->getTeamFrags(1), "team 1 frags 3");
        b &= assertEquals(0, tdm->getTeamFrags(2), "team 2 frags 3");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(0), "team 0 players count 3"); // team 0 always has 0 players
        b &= assertEquals(2u, tdm->getTeamPlayersCount(1), "team 1 players count 3");
        b &= assertEquals(1u, tdm->getTeamPlayersCount(2), "team 2 players count 3");
        b &= assertEquals(2u, tdm->getSpectatorModePlayersCount(), "spectator mode players count 3");

        b &= assertEqualsEz(5u, i, gamemode, true/*server*/, "frags collected");

        const uint32_t nExpectedPktCount = (gamemode == proofps_dd::GameModeType::TeamRoundGame) ? 0 : 1;
        if (gamemode == proofps_dd::GameModeType::TeamRoundGame)
        {
            b &= assertFalseEz(gm->isGameWon(), gamemode, true/*server*/, "game won 2 trg");
            b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning trg");
        }
        else
        {
            b &= assertTrueEz(gm->isGameWon(), gamemode, true/*server*/, "game won 2 tdm");
            b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time tdm");
            b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning tdm");
        }

        if (gamemode != proofps_dd::GameModeType::TeamRoundGame)
        {
            b &= assertEqualsEz(nExpectedPktCount, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count");
            if (b)
            {
                try
                {
                    b &= assertEqualsEz(nExpectedPktCount, m_network.getServer().getTxMsgCount().at(
                        static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                        gamemode, true/*server*/, "tx msg count"
                    );
                }
                catch (...)
                {
                    b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count");
                }
            }
        }

        // now assign the previously unassigned player to team 2 and toggle spectator mode!
        player3_spectate_and_team_unassigned.getTeamId() = 2;
        player3_spectate_and_team_unassigned.isInSpectatorMode() = false;
        player3_spectate_and_team_unassigned.getFrags() = 4;
        b &= assertTrueEz(gm->updatePlayer(player3_spectate_and_team_unassigned, m_network), gamemode, true/*server*/, "update player 2");
        b &= assertEquals(0, tdm->getTeamFrags(0), "team 0 frags 4");  // team 0 always 0 summed frags
        b &= assertEquals(7, tdm->getTeamFrags(1), "team 1 frags 4");
        b &= assertEquals(4, tdm->getTeamFrags(2), "team 2 frags 4");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(0), "team 0 players count 4"); // team 0 always has 0 players
        b &= assertEquals(2u, tdm->getTeamPlayersCount(1), "team 1 players count 4");
        b &= assertEquals(2u, tdm->getTeamPlayersCount(2), "team 2 players count 4");
        b &= assertEquals(1u, tdm->getSpectatorModePlayersCount(), "spectator mode players count 4");

        // now toggle the spectator mode of the last player who was already assigned to team 2 but were in spectator mode for the whole time!
        player4_team_assigned_but_in_spectator_mode.isInSpectatorMode() = false;
        b &= assertTrueEz(gm->updatePlayer(player4_team_assigned_but_in_spectator_mode, m_network), gamemode, true/*server*/, "update player 3");
        b &= assertEquals(0, tdm->getTeamFrags(0), "team 0 frags 5");  // team 0 always 0 summed frags
        b &= assertEquals(7, tdm->getTeamFrags(1), "team 1 frags 5");
        b &= assertEquals(11, tdm->getTeamFrags(2), "team 2 frags 5");
        b &= assertEquals(0u, tdm->getTeamPlayersCount(0), "team 0 players count 5"); // team 0 always has 0 players
        b &= assertEquals(2u, tdm->getTeamPlayersCount(1), "team 1 players count 5");
        b &= assertEquals(3u, tdm->getTeamPlayersCount(2), "team 2 players count 5");
        b &= assertEquals(0u, tdm->getSpectatorModePlayersCount(), "spectator mode players count 5");

        return b;
    }

    bool test_team_games_do_not_count_frags_with_zero_team_id_or_in_spectator_mode()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::TeamDeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_team_games_do_not_count_frags_with_zero_team_id_or_in_spectator_mode(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));

            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;

    }

    bool test_team_games_do_not_allow_any_team_id(const proofps_dd::GameModeType& gamemode)
    {
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
        assert(gm->isTeamBasedGame());

        // dm is DeathMatchMode class instance when gamemode is DeathMatch, and
        // both DeathMatchMode and TeamDeathMatchMode class instance when gamemode is TeamDeathMatch.

        // Basically TDM and DM classes' behavior is very similar, even though players can be in different teams, it does not matter. We just dont care.
        // We just care about frag limit being reached or not, that is why serverCheckAndUpdateWinningConditions() can be overridden.
        // GUI will take care of displaying the players, and it can still simply iterate over the players in the order as DM or TDM is containing the players,
        // no further logic is needed in GUI, just separate them by team ID!
        dm->setFragLimit(7);

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 3;
        player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        bool b = true;
        b &= assertFalseEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        const std::vector<proofps_dd::PlayersTableRow> expectedPlayers1;
        assertFragTableEqualsEz(expectedPlayers1, gm->getPlayersTable(), gamemode, true /*server*/, "table 1 fail");

        player1.getTeamId() = 2;
        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 2");
        const std::vector<proofps_dd::PlayersTableRow> expectedPlayers2 = {
                { "Apple", player1.getServerSideConnectionHandle(), 2 /* iTeamId*/, player1.isInSpectatorMode(), 0, 0 }
        };
        assertFragTableEqualsEz(expectedPlayers2, gm->getPlayersTable(), gamemode, true /*server*/, "table 2 fail");

        player1.getTeamId() = 3;
        b &= assertFalseEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player 1");
        assertFragTableEqualsEz(expectedPlayers2, gm->getPlayersTable(), gamemode, true /*server*/, "table 3 fail");

        player1.getTeamId() = 1;
        b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player 2");
        const std::vector<proofps_dd::PlayersTableRow> expectedPlayers3 = {
                { "Apple", player1.getServerSideConnectionHandle(), 1 /* iTeamId*/, player1.isInSpectatorMode(), 0, 0 }
        };
        assertFragTableEqualsEz(expectedPlayers3, gm->getPlayersTable(), gamemode, true /*server*/, "table 4 fail");

        return b;
    }

    bool test_team_games_do_not_allow_any_team_id()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::TeamDeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_team_games_do_not_allow_any_team_id(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));

            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;
    }

    bool test_team_games_is_player_allowed_for_gameplay(const proofps_dd::GameModeType& gamemode)
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
        assert(gm->isTeamBasedGame());

        proofps_dd::Player player1(
            m_audio, m_cfgProfiles, m_bullets,
            m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
            *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1");
        player1.setName("Apple");
        player1.getFrags() = 0;
        player1.getDeaths() = 0;
        player1.getTeamId() = 0;
        player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        bool b = true;

        b &= assertFalseEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 0");
        player1.isInSpectatorMode() = false;

        b &= assertFalseEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 1");

        player1.getTeamId() = 1;
        b &= assertTrueEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 2");

        player1.getTeamId() = 2;
        b &= assertTrueEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 3");

        player1.isInSpectatorMode() = true;
        b &= assertFalseEz(gm->isPlayerAllowedForGameplay(player1), gamemode, true/*server*/, "allowed 4");

        return b;
    }

    bool test_team_games_is_player_allowed_for_gameplay()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::TeamDeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_team_games_is_player_allowed_for_gameplay(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));

            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;
    }

    bool test_team_games_get_alive_team_players_count(const proofps_dd::GameModeType& gamemode)
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

            assert(gm->isTeamBasedGame());

            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(0), gamemode, bTestingAsServer, "team 0 alive count 1");
            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(1), gamemode, bTestingAsServer, "team 1 alive count 1");
            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(2), gamemode, bTestingAsServer, "team 2 alive count 1");

            if (!add2plus1players())
            {
                return false;
            }
            assert(m_mapPlayers.size() == 3u);
            proofps_dd::Player& player1 = m_mapPlayers.begin()->second;
            proofps_dd::Player& player2 = (++m_mapPlayers.begin())->second;
            proofps_dd::Player& player5 = (++(++m_mapPlayers.begin()))->second;

            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(0), gamemode, bTestingAsServer, "team 0 alive count 2");
            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(1), gamemode, bTestingAsServer, "team 1 alive count 2");
            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(2), gamemode, bTestingAsServer, "team 2 alive count 2");

            b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, bTestingAsServer, "add player 1 fail 1");
            b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, bTestingAsServer, "add player 2 fail 1");
            b &= assertTrueEz(gm->addPlayer(player5, m_network), gamemode, bTestingAsServer, "add player 5 fail 1");
            
            if (!b)
            {
                return false;
            }

            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(0), gamemode, bTestingAsServer, "team 0 alive count 3");
            b &= assertEqualsEz(2u, tdm->getAliveTeamPlayersCount(1), gamemode, bTestingAsServer, "team 1 alive count 3");
            b &= assertEqualsEz(1u, tdm->getAliveTeamPlayersCount(2), gamemode, bTestingAsServer, "team 2 alive count 3");
            
            player5.isInSpectatorMode() = true;
            b &= assertTrueEz(gm->updatePlayer(player5, m_network), gamemode, bTestingAsServer, "update player 5 fail 1");
            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(2), gamemode, bTestingAsServer, "team 2 alive count 4");

            player1.isInSpectatorMode() = true;
            b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, bTestingAsServer, "update player 1 fail 1");
            b &= assertEqualsEz(1u, tdm->getAliveTeamPlayersCount(1), gamemode, bTestingAsServer, "team 1 alive count 4");

            player2.setHealth(0); /* no need to update this player in gm since HP is fetched from Player object */
            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(1), gamemode, bTestingAsServer, "team 1 alive count 5");

            m_mapPlayers.clear();
            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(0), gamemode, bTestingAsServer, "team 0 alive count 6");
            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(1), gamemode, bTestingAsServer, "team 1 alive count 6");
            b &= assertEqualsEz(0u, tdm->getAliveTeamPlayersCount(2), gamemode, bTestingAsServer, "team 2 alive count 6");
        }

        return b;
    }

    bool test_team_games_get_alive_team_players_count()
    {
        bool b = true;
        for (auto gamemode = proofps_dd::GameModeType::TeamDeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
        {
            b &= assertTrue(
                test_team_games_get_alive_team_players_count(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));

            // just in case test does not invoke tearDown() before reinitializing something like network, call it here before next iteration
            tearDown();
        }
        return b;
    }

    bool test_round_games_winning_cond_time_and_round_win_limit(const proofps_dd::GameModeType& gamemode)
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

        assert(gm->isRoundBased());

        if (!add2plus1players())
        {
            return false;
        }
        assert(m_mapPlayers.size() == 3u);
        proofps_dd::Player& player1 = m_mapPlayers.begin()->second;
        proofps_dd::Player& player2 = (++m_mapPlayers.begin())->second;
        proofps_dd::Player& player5 = (++(++m_mapPlayers.begin()))->second;
        bool b = true;

        trg->setRoundWinLimit(5);
        gm->setTimeLimitSecs(2);

        gm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        b &= assertEqualsEz(2u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count 1");
        if (b)
        {
            try
            {
                b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true/*server*/, "tx msg count 1"
                );
                b &= assertEqualsEz(1u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 2"
                );
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count 1 or 2");
            }
        }

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");
        b &= assertTrueEz(gm->addPlayer(player5, m_network), gamemode, true/*server*/, "add player 5");

        // time limit elapse also means winning even if round win limit not reached
        std::set<unsigned int> setRemainingSecs = { 0, 1 };
        int iSleep = 0;
        bool bPrevWonState = false;
        while ((iSleep++ < 5) && !gm->serverCheckAndUpdateWinningConditions(m_network))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            setRemainingSecs.erase(static_cast<unsigned int>(std::floor(gm->getTimeRemainingMillisecs() / 1000.f)));

            if (!bPrevWonState && gm->isGameWon())
            {
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 1");
            }
            bPrevWonState = gm->isGameWon();
        }

        b &= assertTrueEz(gm->isGameWon(), gamemode, true/*server*/, "game won 1");
        const auto durationSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - gm->getResetTime());
        b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time 1");
        b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning due to time");
        b &= assertLequalsEz(static_cast<std::chrono::seconds::rep>(gm->getTimeLimitSecs()), durationSecs.count(), gamemode, true/*server*/, "time limit elapsed");
        b &= assertTrueEz(setRemainingSecs.empty(), gamemode, true/*server*/, "no remaining");

        b &= assertEqualsEz(7u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count 2");
        if (b)
        {
            try
            {
                b &= assertEqualsEz(2u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true/*server*/, "tx msg count 3"
                );
                // addPlayer()'s also send out MsgGameRoundState
                b &= assertEqualsEz(5u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 4"
                );
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count 3 or 4");
            }
        }

        // round win limit reach also means winning even if time limit not reached
        gm->setTimeLimitSecs(100);
        gm->restart(m_network);
        // restart triggers MsgGameSessionStateFromServer out no matter current state
        b &= assertEqualsEz(9u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count 3");
        if (b)
        {
            try
            {
                b &= assertEqualsEz(3u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true/*server*/, "tx msg count 5"
                );
                b &= assertEqualsEz(6u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 6"
                );
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count 5 or 6");
            }
        }

        b &= assertFalseEz(gm->isGameWon(), gamemode, true/*server*/, "game won 2");
        b &= assertTrueEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 2");
        b &= assertEqualsEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time 2");

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");
        b &= assertTrueEz(gm->addPlayer(player5, m_network), gamemode, true/*server*/, "add player 5");

        unsigned int i = 0;
        bPrevWonState = false;
        trg->setTeamRoundWins(1, 3); // just because why not
        trg->getFSM().transitionToPlayState(); /* forcing FSM to Play state, otherwise team round wins are not checked against round win limit */
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /*server*/, "FSM state 1");
        while (!gm->isGameWon() && (i < 5))
        {
            i++;
            trg->setTeamRoundWins(2, trg->getTeamRoundWins(2) + 1);

            gm->serverCheckAndUpdateWinningConditions(m_network);
            if (!bPrevWonState && gm->isGameWon())
            {
                b &= assertFalseEz(gm->wasGameWonAlreadyInPreviousTick(), gamemode, true/*server*/, "game not won previous tick 3");
            }
            bPrevWonState = gm->isGameWon();
        }

        b &= assertEqualsEz(trg->getTeamRoundWins(2), i, gamemode, true/*server*/, "round wins collected");

        b &= assertTrueEz(gm->isGameWon(), gamemode, true/*server*/, "game won 3");
        b &= assertLessEz(0, gm->getWinTime().time_since_epoch().count(), gamemode, true/*server*/, "win time");
        b &= assertTrueEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true/*server*/, "winning due to frags");

        b &= assertEqualsEz(14u, m_network.getServer().getTxPacketCount(), gamemode, true/*server*/, "tx pkt count 4");
        if (b)
        {
            try
            {
                b &= assertEqualsEz(4u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameSessionStateFromServer::id)),
                    gamemode, true/*server*/, "tx msg count 7"
                );
                // addPlayer()'s also send out MsgGameRoundState
                b &= assertEqualsEz(10u, m_network.getServer().getTxMsgCount().at(
                    static_cast<pge_network::MsgApp::TMsgId>(proofps_dd::MsgGameRoundStateFromServer::id)),
                    gamemode, true /*server*/, "tx msg count 8"
                );
            }
            catch (...)
            {
                b &= assertFalseEz(true, gamemode, true/*server*/, "tx msg count 7 or 8");
            }
        }

        return b;
    }

    bool test_round_games_winning_cond_time_and_round_win_limit()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;
        
        bool b = true;
        b &= assertTrue(
                test_round_games_winning_cond_time_and_round_win_limit(gamemode),
                proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }

    bool test_round_games_get_time_limit_in_current_state(const proofps_dd::GameModeType& gamemode)
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

        assert(gm->isRoundBased());

        bool b = true;

        /* -------------- Test 1: Prepare --------------------------------------- */

        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(), gamemode, true /*server*/, "FSM state 1");
        b &= assertEqualsEz(3ll, trg->getFSM().getTimeLimitInCurrentStateSeconds(), gamemode, true /*server*/, "time limit 1");

        trg->getFSM().transitionToPlayState();

        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(), gamemode, true /*server*/, "FSM state 2");
        b &= assertEqualsEz(999ll, trg->getFSM().getTimeLimitInCurrentStateSeconds(), gamemode, true /*server*/, "time limit 2");

        trg->getFSM().roundWon();

        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
            trg->getFSM().getState(), gamemode, true /*server*/, "FSM state 3");
        b &= assertEqualsEz(5ll, trg->getFSM().getTimeLimitInCurrentStateSeconds(), gamemode, true /*server*/, "time limit 3");

        return b;
    }

    bool test_round_games_get_time_limit_in_current_state()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_get_time_limit_in_current_state(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }

    bool test_round_games_get_time_remaining_in_current_state(const proofps_dd::GameModeType& gamemode)
    {
        // server-only test, clients do not transition FSM on their own but they receive state changes
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true/*server*/, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        assert(gm->isRoundBased());

        // need to restart to correctly initialize time-specific values if we have time limit!
        gm->restartWithoutRemovingPlayers(m_network);

        bool b = true;

        /* -------------- Test 1: Prepare -> Play --------------------------------------- */

        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 1");

        std::set<std::chrono::seconds::rep> setRemainingSecs = { 0, 1, 2 };
        int iSleep = 0;
        while ((iSleep++ < 10) &&
            !trg->serverCheckAndUpdateWinningConditions(m_network) /* this drives RoundStateFSM */ &&
            (trg->getFSM().getState() != proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            const auto nSecondsRemaining = trg->getFSM().getTimeRemainingInCurrentStateSeconds();
            setRemainingSecs.erase(nSecondsRemaining);

            b &= assertLequals(
                trg->getFSM().getTimeRemainingInCurrentStateMilliseconds(),
                static_cast<std::chrono::milliseconds::rep>((nSecondsRemaining + 1) * 1000),
                "millisecs 1");
        }

        /* check: we exited the loop because time elapsed the FSM has transitioned to next state */
        b &= assertTrueEz(setRemainingSecs.empty(), gamemode, true /* testing as server */, "no remaining 1");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 2");

        /* game won state did not change */
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won");
        b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "winning");

        b &= assertEquals(999ll, trg->getFSM().getTimeRemainingInCurrentStateSeconds(), "remaining secs in Play state");
        b &= assertEquals(999000ll, trg->getFSM().getTimeRemainingInCurrentStateMilliseconds(), "remaining millisecs in Play state");

        /* -------------- Test 2: WaitForReset -> Prepare --------------------------------------- */

        trg->getFSM().roundWon();
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 3");

        setRemainingSecs = { 0, 1, 2, 3, 4 };
        iSleep = 0;
        while ((iSleep++ < 15) &&
            !trg->serverCheckAndUpdateWinningConditions(m_network) /* this drives RoundStateFSM */ &&
            (trg->getFSM().getState() != proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            const auto nSecondsRemaining = trg->getFSM().getTimeRemainingInCurrentStateSeconds();
            setRemainingSecs.erase(nSecondsRemaining);

            b &= assertLequals(
                trg->getFSM().getTimeRemainingInCurrentStateMilliseconds(),
                static_cast<std::chrono::milliseconds::rep>((nSecondsRemaining + 1) * 1000),
                "millisecs 2");
        }

        /* check: we exited the loop because time elapsed the FSM has transitioned to next state */
        b &= assertTrueEz(setRemainingSecs.empty(), gamemode, true /* testing as server */, "no remaining 2");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 4");

        return b;
    }

    bool test_round_games_get_time_remaining_in_current_state()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_get_time_remaining_in_current_state(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }

    bool test_round_games_client_update_time_remaining_in_current_state_millisecs(const proofps_dd::GameModeType& gamemode)
    {
        // client-only test
        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        assert(gm->isRoundBased());

        // need to restart to correctly initialize time-specific values if we have time limit!
        gm->restartWithoutRemovingPlayers(m_network);

        bool b = true;
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 1");

        //constexpr unsigned int nTimeLimitSecs = 13u;
        //m_cfgProfiles.getVars()[proofps_dd::GameMode::szCvarSvGmTimeLimit].Set(nTimeLimitSecs);
        //gm->fetchConfig(m_cfgProfiles, m_network);
        trg->getFSM().clientUpdateTimeRemainingInCurrentStateMillisecs(1300u, m_network);

        // positive case
        b &= //assertEquals(nTimeLimitSecs, sgm.getTimeLimitSecs(), "time limit") &
            assertLequals(trg->getFSM().getTimeRemainingInCurrentStateSeconds(), 2u, "remaining prepare 1") &
            assertNotEquals(0u, trg->getFSM().getTimeRemainingInCurrentStateSeconds(), "remaining prepare 2");

        // negative case: remaining time as seconds is bigger than time limit as seconds
        trg->getFSM().clientUpdateTimeRemainingInCurrentStateMillisecs(6000u, m_network);
        b &= assertLequals(1300u, trg->getFSM().getTimeRemainingInCurrentStateSeconds()*1000, "remaining prepare 3");

        return b;
    }

    bool test_round_games_client_update_time_remaining_in_current_state_millisecs()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_client_update_time_remaining_in_current_state_millisecs(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }

    bool test_round_games_transition_to_play_state_after_n_seconds(const proofps_dd::GameModeType& gamemode)
    {
        // server-only test, clients do not transition FSM on their own but they receive state changes
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true/*server*/, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        assert(gm->isRoundBased());

        // need to restart to correctly initialize time-specific values if we have time limit!
        gm->restartWithoutRemovingPlayers(m_network);

        bool b = true;
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 1");

        std::set<std::chrono::seconds::rep> setRemainingSecs = { 0, 1, 2 };
        int iSleep = 0;
        while ((iSleep++ < 10) &&
            !trg->serverCheckAndUpdateWinningConditions(m_network) /* this drives RoundStateFSM */ &&
            (trg->getFSM().getState() != proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            const auto nSecondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - trg->getFSM().getTimeEnteredCurrentState()).count();
            setRemainingSecs.erase(nSecondsRemaining);
        }
       
        /* check: we exited the loop because time elapsed the FSM has transitioned to next state */
        b &= assertTrueEz(setRemainingSecs.empty(), gamemode, true /* testing as server */, "no remaining");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 2");
        
        /* game won state did not change */
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won");
        b &= assertFalseEz(gm->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "winning");

        return b;
    }

    bool test_round_games_transition_to_play_state_after_n_seconds()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_transition_to_play_state_after_n_seconds(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }

    bool test_round_games_round_is_won_when_a_team_dies_and_other_team_is_not_empty(const proofps_dd::GameModeType& gamemode)
    {
        // server-only test, clients do not transition FSM on their own but they receive state changes
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true/*server*/, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        assert(gm->isRoundBased());

        if (!add2plus1players())
        {
            return false;
        }
        assert(m_mapPlayers.size() == 3u);
        proofps_dd::Player& player1 = m_mapPlayers.begin()->second;
        proofps_dd::Player& player2 = (++m_mapPlayers.begin())->second;
        proofps_dd::Player& player5 = (++(++m_mapPlayers.begin()))->second;
        bool b = true;

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");
        b &= assertTrueEz(gm->addPlayer(player5, m_network), gamemode, true/*server*/, "add player 5");

        if (!b)
        {
            return false;
        }

        trg->setRoundWinLimit(2);

        // need to restart to correctly initialize time-specific values if we have time limit!
        gm->restartWithoutRemovingPlayers(m_network);

        player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
        player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
        player5.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player 1 fail 1");
        b &= assertTrueEz(gm->updatePlayer(player2, m_network), gamemode, true/*server*/, "update player 2 fail 1");
        b &= assertTrueEz(gm->updatePlayer(player5, m_network), gamemode, true/*server*/, "update player 5 fail 1");

        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 1");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 1");
        
        trg->getFSM().transitionToPlayState();  /* do not wait for countdown in Prepare state */
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 1");

        /* test-drive FSM but we don't expect any change at this moment */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 1");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 2");

        /* ----- test 1: Team 2 dies ----------------------------- */

        player5.setHealth(0);
        /* drive FSM */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 2");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 3");
        b &= assertEqualsEz(1u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 2");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 2");
        /* round winning not necessarily means game win */
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won 1");

        /* reset stuff */
        player5.setHealth(100);
        trg->getFSM().reset();
        trg->getFSM().transitionToPlayState();  /* do not wait for countdown in Prepare state */
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 3");

        /* ----- test 2: Team 1 dies ----------------------------- */

        player1.setHealth(0);
        player2.setHealth(0);
        /* drive FSM */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 4");
        b &= assertEqualsEz(1u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 3");
        b &= assertEqualsEz(1u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 3");
        
        /* round winning not necessarily means game win */
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won 2");

        /* reset stuff */
        player1.setHealth(100);
        player2.setHealth(100);
        trg->getFSM().reset();
        trg->getFSM().transitionToPlayState();  /* do not wait for countdown in Prepare state */
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 5");

        /* ----- test 3: Team 2 dies again ----------------------- */

        player5.setHealth(0);
        /* drive FSM */
        b &= assertTrueEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 4");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 6");
        b &= assertEqualsEz(2u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 4");
        b &= assertEqualsEz(1u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 4");
        /* finally hit game round win limit */
        b &= assertTrueEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won 3");

        return b;
    }

    bool test_round_games_round_is_won_when_a_team_dies_and_other_team_is_not_empty()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_round_is_won_when_a_team_dies_and_other_team_is_not_empty(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }

    bool test_round_games_round_ends_without_win_when_a_team_becomes_empty_and_other_team_is_not_empty(const proofps_dd::GameModeType& gamemode)
    {
        // server-only test, clients do not transition FSM on their own but they receive state changes
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true/*server*/, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        assert(gm->isRoundBased());

        if (!add2plus1players())
        {
            return false;
        }
        assert(m_mapPlayers.size() == 3u);
        proofps_dd::Player& player1 = m_mapPlayers.begin()->second;
        proofps_dd::Player& player2 = (++m_mapPlayers.begin())->second;
        proofps_dd::Player& player5 = (++(++m_mapPlayers.begin()))->second;
        bool b = true;

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");
        b &= assertTrueEz(gm->addPlayer(player5, m_network), gamemode, true/*server*/, "add player 5");

        if (!b)
        {
            return false;
        }

        trg->setRoundWinLimit(3);

        // need to restart to correctly initialize time-specific values if we have time limit!
        gm->restartWithoutRemovingPlayers(m_network);

        player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
        player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
        player5.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player 1 fail 1");
        b &= assertTrueEz(gm->updatePlayer(player2, m_network), gamemode, true/*server*/, "update player 2 fail 1");
        b &= assertTrueEz(gm->updatePlayer(player5, m_network), gamemode, true/*server*/, "update player 5 fail 1");

        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 1");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 1");

        trg->getFSM().transitionToPlayState();  /* do not wait for countdown in Prepare state */
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 1");

        /* test-drive FSM but we don't expect any change at this moment */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 1");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 2");

        /* ----- test 1: Team 2 becomes empty due to spectator mode ----------------------------- */

        player5.isInSpectatorMode() = true;
        b &= assertTrueEz(gm->updatePlayer(player5, m_network), gamemode, true/*server*/, "update player 5 fail 3");
        /* drive FSM */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 4");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 5");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 2");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 2");
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won 2");

        /* reset stuff */
        player5.isInSpectatorMode() = false;
        b &= assertTrueEz(gm->updatePlayer(player5, m_network), gamemode, true/*server*/, "update player 5 fail 4");
        trg->getFSM().reset();
        trg->getFSM().transitionToPlayState();  /* do not wait for countdown in Prepare state */
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 6");
      
        /* ----- test 3: Team 2 becomes empty due to disconnecting player ----------------------------- */

        b &= assertTrueEz(gm->removePlayer(player5, m_network), gamemode, true/*server*/, "remove player 5 fail");
        b &= assertEqualsEz(1u, m_mapPlayers.erase(player5.getServerSideConnectionHandle()), gamemode, true/*server*/, "erase player 5 fail");
        
        /* drive FSM */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 5");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 9");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 3");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 3");
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won 3");

        return b;
    }

    bool test_round_games_round_ends_without_win_when_a_team_becomes_empty_and_other_team_is_not_empty()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_round_ends_without_win_when_a_team_becomes_empty_and_other_team_is_not_empty(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;

    }

    bool test_round_games_round_ends_without_win_when_a_team_dies_and_other_team_is_empty(const proofps_dd::GameModeType& gamemode)
    {
        // server-only test, clients do not transition FSM on their own but they receive state changes
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true/*server*/, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        assert(gm->isRoundBased());

        auto insertRes = m_mapPlayers.insert(
            {
                1,
                proofps_dd::Player(
                    m_audio, m_cfgProfiles, m_bullets,
                    m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents,
                    *m_engine, m_network, static_cast<pge_network::PgeNetworkConnectionHandle>(1), "192.168.1.1")
            }); // TODO: emplace_back()
        bool b = assertTrue(insertRes.second, "player1 insert into m_mapPlayers");
        proofps_dd::Player& player1 = insertRes.first->second;
        if (b)
        {
            player1.setName("Adam");
            player1.getFrags() = 0;
            player1.getDeaths() = 0;
            player1.getTeamId() = 1; // required non-zero teamId for TDM test, otherwise TDM does not count frags
            player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
        }

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");

        if (!b)
        {
            return false;
        }

        trg->setRoundWinLimit(3);

        // need to restart to correctly initialize time-specific values if we have time limit!
        gm->restartWithoutRemovingPlayers(m_network);

        player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player 1 fail 1");

        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 1");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 1");

        trg->getFSM().transitionToPlayState();  /* do not wait for countdown in Prepare state */
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 1");

        /* test-drive FSM but we don't expect any change at this moment */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 1");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 2");

        /* ----- test 1: Team 1 becomes empty due to spectator mode (no change in empty Team 2) ----------------------------- */

        // no round end in this case, since although a team has become empty, no teams have any players left

        player1.isInSpectatorMode() = true;
        b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player 1 fail 3");
        /* drive FSM */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 4");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 5");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 2");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 2");
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won 2");

        /* reset stuff */
        player1.isInSpectatorMode() = false;
        b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player 1 fail 4");
        trg->getFSM().reset();
        trg->getFSM().transitionToPlayState();  /* do not wait for countdown in Prepare state */
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 6");

        /* ----- test 2: Team 1 is killed due to player dies (no change in empty Team 2) ----------------------------- */

        // round end is expected in this case since 1 team has assigned player(s), but without win since nobody is in the other team

        player1.setHealth(0);  /* no need to update this player in gm since HP is fetched from Player object */
        /* drive FSM */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 5");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 7");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 3");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 3");
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won 3");

        /* reset stuff */
        player1.setHealth(100);  /* no need to update this player in gm since HP is fetched from Player object */
        trg->getFSM().reset();
        trg->getFSM().transitionToPlayState();  /* do not wait for countdown in Prepare state */
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 8");

        /* ----- test 3: Team 1 becomes empty due to disconnecting player (no change in empty Team 2) ----------------------------- */

        // no round end in this case, since although a team has become empty, no teams have any players left

        b &= assertTrueEz(gm->removePlayer(player1, m_network), gamemode, true/*server*/, "remove player 1 fail");
        b &= assertEqualsEz(1u, m_mapPlayers.erase(player1.getServerSideConnectionHandle()), gamemode, true/*server*/, "erase player 1 fail");

        /* drive FSM */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 6");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 9");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 4");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 4");
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won 4");

        return b;
    }

    bool test_round_games_round_ends_without_win_when_a_team_dies_and_other_team_is_empty()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_round_ends_without_win_when_a_team_dies_and_other_team_is_empty(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }

    bool test_round_games_round_cannot_be_won_by_killing_a_team_if_fsm_is_not_in_play_state(
        const proofps_dd::GameModeType& gamemode,
        const proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState& fsmStateBeingTested)
    {
        // server-only test, clients do not transition FSM on their own but they receive state changes
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true/*server*/, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        assert(gm->isRoundBased());

        if (!add2plus1players())
        {
            return false;
        }
        assert(m_mapPlayers.size() == 3u);
        proofps_dd::Player& player1 = m_mapPlayers.begin()->second;
        proofps_dd::Player& player2 = (++m_mapPlayers.begin())->second;
        proofps_dd::Player& player5 = (++(++m_mapPlayers.begin()))->second;
        bool b = true;

        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true/*server*/, "add player 1");
        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true/*server*/, "add player 2");
        b &= assertTrueEz(gm->addPlayer(player5, m_network), gamemode, true/*server*/, "add player 5");

        if (!b)
        {
            return false;
        }

        trg->setRoundWinLimit(2);

        // need to restart to correctly initialize time-specific values if we have time limit!
        gm->restartWithoutRemovingPlayers(m_network);

        player1.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
        player2.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.
        player5.isInSpectatorMode() = false; // otherwise player won't be taken into account for their assigned team, gamemode win state, etc.

        b &= assertTrueEz(gm->updatePlayer(player1, m_network), gamemode, true/*server*/, "update player 1 fail 1");
        b &= assertTrueEz(gm->updatePlayer(player2, m_network), gamemode, true/*server*/, "update player 2 fail 1");
        b &= assertTrueEz(gm->updatePlayer(player5, m_network), gamemode, true/*server*/, "update player 5 fail 1");

        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 1");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 1");

        if (fsmStateBeingTested == proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset)
        {
            trg->getFSM().transitionToPlayState();
            trg->getFSM().roundWon();
        }
        b &= assertEqualsEz(
            fsmStateBeingTested,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 1");

        /* test-drive FSM but we don't expect any change at this moment */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 1");
        b &= assertEqualsEz(
            fsmStateBeingTested,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 2");

        /* ----- test 1: Team 2 dies ----------------------------- */

        player5.setHealth(0);
        /* drive FSM */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 2");
        b &= assertEqualsEz(
            fsmStateBeingTested,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 3");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 2");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 2");
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won 1");

        /* reset stuff */
        player5.setHealth(100);
        b &= assertEqualsEz(
            fsmStateBeingTested,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 3");

        /* ----- test 2: Team 1 dies ----------------------------- */

        player1.setHealth(0);
        player2.setHealth(0);
        /* drive FSM */
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 3");
        b &= assertEqualsEz(
            fsmStateBeingTested,
            trg->getFSM().getState(),
            gamemode, true /* testing as server */, "FSM state 4");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(1), gamemode, true /* testing as server */, "team 1 round wins 3");
        b &= assertEqualsEz(0u, trg->getTeamRoundWins(2), gamemode, true /* testing as server */, "team 2 round wins 3");
        b &= assertFalseEz(gm->isGameWon(), gamemode, true /* testing as server */, "game won 2");

        return b;
    }

    bool test_round_games_round_cannot_be_won_by_killing_a_team_if_fsm_is_not_in_play_state()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_round_cannot_be_won_by_killing_a_team_if_fsm_is_not_in_play_state(
                gamemode, proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare),
            (std::string(proofps_dd::GameMode::getGameModeTypeName(gamemode)) + " Prepare State").c_str());
        
        tearDown(); /* for network reinit to pass in next test below */

        b &= assertTrue(
            test_round_games_round_cannot_be_won_by_killing_a_team_if_fsm_is_not_in_play_state(
                gamemode, proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset),
            (std::string(proofps_dd::GameMode::getGameModeTypeName(gamemode)) + " WaitForReset State").c_str());

        return b;
    }

    bool test_round_games_player_movement_allowed(const proofps_dd::GameModeType& gamemode)
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

            assert(gm->isRoundBased());

            b &= assertEquals(
                proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
                trg->getFSM().getState(),
                "fsm state 1");
            b &= assertFalse(gm->isPlayerMovementAllowed(), "movement allowed 1");
           
            trg->getFSM().transitionToPlayState();
            b &= assertEquals(
                proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
                trg->getFSM().getState(),
                "fsm state 2");
            b &= assertTrue(gm->isPlayerMovementAllowed(), "movement allowed 2");

            trg->getFSM().roundWon();
            b &= assertEquals(
                proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
                trg->getFSM().getState(),
                "fsm state 3");
            b &= assertFalse(gm->isPlayerMovementAllowed(), "movement allowed 3");
        }

        return b;
    }

    bool test_round_games_player_movement_allowed()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_player_movement_allowed(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }

    bool test_round_games_hasJustTransitionedTo_functions_server(const proofps_dd::GameModeType& gamemode)
    {
        m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
        if (!m_network.initialize())
        {
            return assertFalseEz(true, gamemode, true/*server*/, "network reinit as server");
        }

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, true/*server*/, "testInitGamemode fail");
        }

        assert(gm->isRoundBased());

        if (!add2plus1players())
        {
            return false;
        }
        assert(m_mapPlayers.size() == 3u);
        proofps_dd::Player& player1 = m_mapPlayers.begin()->second;
        proofps_dd::Player& player2 = (++m_mapPlayers.begin())->second;
        proofps_dd::Player& player5 = (++(++m_mapPlayers.begin()))->second;

        bool b = true;

        gm->restartWithoutRemovingPlayers(m_network);  // for time-limit-sensitive stuff

        /* server tick 1 */

        // server transitions to Prepare (from Prepare) this tick, but also a player joins:
        // this is important detail, because addPlayer() also invokes serverCheckAndUpdateWinningConditions(), so
        // what we are testing here is that hasJustTransitionedTo_RoundPrepareState_InThisTick() does not revert to
        // false until explicitly cleared by serverTickUpdateWinningConditions().
        b &= assertTrueEz(gm->addPlayer(player1, m_network), gamemode, true /* server */, "add player 1 fail 1");

        b &= assertFalse(trg->serverCheckAndUpdateWinningConditions(m_network), "serverCheck 1");
        
        b &= assertTrue(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 1 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 1 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 1 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(), gamemode, true /* server */, "FSM state 1");

        trg->serverTickUpdateWinningConditions(m_network);

        /* server tick 2 */

        b &= assertTrueEz(gm->addPlayer(player2, m_network), gamemode, true /* server */, "add player 2 fail 1");
        b &= assertTrueEz(gm->addPlayer(player5, m_network), gamemode, true /* server */, "add player 5 fail 1");

        if (!b)
        {
            return false;
        }

        b &= assertFalse(trg->serverCheckAndUpdateWinningConditions(m_network), "serverCheck 2");

        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 2 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 2 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 2 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(), gamemode, true /* server */, "FSM state 2");

        trg->serverTickUpdateWinningConditions(m_network);

        /* server tick 3..n */

        std::set<std::chrono::seconds::rep> setRemainingSecs = { 0, 1, 2 };
        int iSleep = 0;
        while ((iSleep++ < 10) &&
            !trg->serverCheckAndUpdateWinningConditions(m_network) /* this drives RoundStateFSM */ &&
            (trg->getFSM().getState() != proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            const auto nSecondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - trg->getFSM().getTimeEnteredCurrentState()).count();
            setRemainingSecs.erase(nSecondsRemaining);
        }

        b &= assertTrueEz(setRemainingSecs.empty(), gamemode, true /* testing as server */, "no remaining 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 3 1");
        b &= assertTrue(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 3 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 3 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(), gamemode, true /* server */, "FSM state 3");

        trg->serverTickUpdateWinningConditions(m_network);

        /* server tick n+1 */

        b &= assertFalse(trg->serverCheckAndUpdateWinningConditions(m_network), "serverCheck 4");

        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 4 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 4 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 4 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(), gamemode, true /* server */, "FSM state 4");

        trg->serverTickUpdateWinningConditions(m_network);

        /* server tick (n+2)..m */

        player5.setHealth(0);
        b &= assertFalseEz(
            trg->serverCheckAndUpdateWinningConditions(m_network), gamemode, true /* testing as server */, "serverCheck 5");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 5 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 5 2");
        b &= assertTrue(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 5 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
            trg->getFSM().getState(), gamemode, true /* server */, "FSM state 5");

        trg->serverTickUpdateWinningConditions(m_network);

        /* server tick m+1 */

        b &= assertFalse(trg->serverCheckAndUpdateWinningConditions(m_network), "serverCheck 6");

        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 6 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 6 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 6 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
            trg->getFSM().getState(), gamemode, true /* server */, "FSM state 6");

        trg->serverTickUpdateWinningConditions(m_network);

        /* server tick (m+2)..p  */

        setRemainingSecs = { 0, 1, 2, 3, 4 };
        iSleep = 0;
        while ((iSleep++ < 15) &&
            !trg->serverCheckAndUpdateWinningConditions(m_network) /* this drives RoundStateFSM */ &&
            (trg->getFSM().getState() != proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            const auto nSecondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - trg->getFSM().getTimeEnteredCurrentState()).count();
            setRemainingSecs.erase(nSecondsRemaining);
        }

        b &= assertTrueEz(setRemainingSecs.empty(), gamemode, true /* testing as server */, "no remaining 2");
        b &= assertTrue(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 7 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 7 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 7 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(), gamemode, true /* server */, "FSM state 7");

        trg->serverTickUpdateWinningConditions(m_network);

        /* server tick p+1 */

        b &= assertFalse(trg->serverCheckAndUpdateWinningConditions(m_network), "serverCheck 8");

        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 8 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 8 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 8 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(), gamemode, true /* server */, "FSM state 8");

        trg->serverTickUpdateWinningConditions(m_network);

        return b;
    }

    bool test_round_games_hasJustTransitionedTo_functions_server()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_hasJustTransitionedTo_functions_server(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }

    bool test_round_games_hasJustTransitionedTo_functions_client(const proofps_dd::GameModeType& gamemode)
    {
        bool b = true;
        b &= assertFalse(
            m_cfgProfiles.getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].getAsBool(), "client");

        if (!testInitGamemode(gamemode))
        {
            return assertFalseEz(true, gamemode, false/*server*/, "testInitGamemode fail");
        }

        assert(gm->isRoundBased());

        /* client tick 1 */

        b &= assertTrue(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 1 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 1 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 1 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(), gamemode, false /* server */, "FSM state 1");

        trg->clientTickUpdateWinningConditions(m_network);

        /* client tick 2 */

        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 2 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 2 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 2 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(), gamemode, false /* server */, "FSM state 2");

        trg->clientTickUpdateWinningConditions(m_network);

        /* client tick 3 */

        proofps_dd::MsgGameRoundStateFromServer msgGameRoundState{};
        msgGameRoundState.m_fsmState = proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play;
        trg->clientHandleGameRoundStateFromServer(m_network, msgGameRoundState);

        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 3 1");
        b &= assertTrue(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 3 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 3 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(), gamemode, false /* server */, "FSM state 3");

        trg->clientTickUpdateWinningConditions(m_network);

        /* client tick 4 */

        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 4 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 4 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 4 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play,
            trg->getFSM().getState(), gamemode, false /* server */, "FSM state 4");

        trg->clientTickUpdateWinningConditions(m_network);

        /* client tick 5 */

        msgGameRoundState.m_fsmState = proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset;
        msgGameRoundState.m_nTeam1RoundWins++;
        trg->clientHandleGameRoundStateFromServer(m_network, msgGameRoundState);

        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 5 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 5 2");
        b &= assertTrue(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 5 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
            trg->getFSM().getState(), gamemode, false /* server */, "FSM state 5");

        trg->clientTickUpdateWinningConditions(m_network);

        /* client tick 6 */

        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 6 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 6 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 6 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset,
            trg->getFSM().getState(), gamemode, false /* server */, "FSM state 6");

        trg->clientTickUpdateWinningConditions(m_network);

        /* client tick 7 */

        msgGameRoundState.m_fsmState = proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare;
        trg->clientHandleGameRoundStateFromServer(m_network, msgGameRoundState);

        b &= assertTrue(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 7 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 7 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 7 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(), gamemode, false /* server */, "FSM state 7");

        trg->clientTickUpdateWinningConditions(m_network);

        /* client tick 8 */

        b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 8 1");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 8 2");
        b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 8 3");
        b &= assertEqualsEz(
            proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
            trg->getFSM().getState(), gamemode, false /* server */, "FSM state 8");

        trg->clientTickUpdateWinningConditions(m_network);

        return b;
    }

    bool test_round_games_hasJustTransitionedTo_functions_client()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_hasJustTransitionedTo_functions_client(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }

    bool test_round_games_hasJustTransitionedTo_always_detects_restart(const proofps_dd::GameModeType& gamemode)
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

            assert(gm->isRoundBased());

            /* ending current tick */
            if (bTestingAsServer)
            {
                trg->serverTickUpdateWinningConditions(m_network);
            }
            else
            {
                trg->clientTickUpdateWinningConditions(m_network);
            }

            /* sanity check */
            b &= assertFalse(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 1 1");
            b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 1 2");
            b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 1 3");
            b &= assertEqualsEz(
                proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
                trg->getFSM().getState(), gamemode, false /* server */, "FSM state 1");

            /* hasJustTransitionedTo_RoundPrepareState_InThisTick() must detect transition upon game restart
               even though it was not a real state transition */
            gm->restartWithoutRemovingPlayers(m_network);

            b &= assertTrue(trg->hasJustTransitionedTo_RoundPrepareState_InThisTick(), "hasTransitioned 2 1");
            b &= assertFalse(trg->hasJustTransitionedTo_RoundPlayState_InThisTick(), "hasTransitioned 2 2");
            b &= assertFalse(trg->hasJustTransitionedTo_RoundWaitForResetState_InThisTick(), "hasTransitioned 2 3");
            b &= assertEqualsEz(
                proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare,
                trg->getFSM().getState(), gamemode, false /* server */, "FSM state 2");
        }

        return b;
    }

    bool test_round_games_hasJustTransitionedTo_always_detects_restart()
    {
        const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamRoundGame;

        bool b = true;
        b &= assertTrue(
            test_round_games_hasJustTransitionedTo_always_detects_restart(gamemode),
            proofps_dd::GameMode::getGameModeTypeName(gamemode));

        return b;
    }
};