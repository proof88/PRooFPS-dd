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

    GameModeTest() :
        UnitTest(__FILE__),
        gm(nullptr),
        dm(nullptr)
    {
    }

protected:

    virtual void Initialize() override
    {
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::GameMode::getLoggerModuleName(), true);

        AddSubTest("test_factory_creates_deathmatch_only", (PFNUNITSUBTEST)&GameModeTest::test_factory_creates_deathmatch_only);
        AddSubTest("test_reset_updates_reset_time", (PFNUNITSUBTEST)&GameModeTest::test_reset_updates_reset_time);
        AddSubTest("test_deathmatch_time_limit_get_set", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_time_limit_get_set);
        AddSubTest("test_deathmatch_frag_limit_get_set", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_frag_limit_get_set);
        AddSubTest("test_deathmatch_winning_cond_defaults_to_false", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_defaults_to_false);
        AddSubTest("test_deathmatch_winning_cond_time_limit", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_time_limit);
        AddSubTest("test_deathmatch_winning_cond_frag_limit", (PFNUNITSUBTEST)&GameModeTest::test_deathmatch_winning_cond_frag_limit);
    }

    virtual void Finalize() override
    {
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::GameMode::getLoggerModuleName(), false);
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

    // ---------------------------------------------------------------------------

    GameModeTest(const GameModeTest&)
    {};

    GameModeTest& operator=(const GameModeTest&)
    {
        return *this;
    };

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

        return b;
    }

    bool test_reset_updates_reset_time()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        gm->Reset();
        return assertLess(0, gm->getResetTime().time_since_epoch().count());
    }

    bool test_deathmatch_time_limit_get_set()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        bool b = assertEquals(0u, dm->getTimeLimitSecs(), "default");
        dm->SetTimeLimitSecs(25u);
        b &= assertEquals(25u, dm->getTimeLimitSecs(), "new");

        return b;
    }

    bool test_deathmatch_frag_limit_get_set()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        bool b = assertEquals(0u, dm->getFragLimit(), "default");
        dm->SetFragLimit(25u);
        b &= assertEquals(25u, dm->getFragLimit(), "new");

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

        dm->SetTimeLimitSecs(2);
        dm->Reset();
        int iSleep = 0;
        while ((iSleep++ < 5) && !dm->checkWinningConditions())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        const auto duration = std::chrono::steady_clock::now() - dm->getResetTime();
        bool b = assertTrue(dm->checkWinningConditions(), "winning");
        b &= assertLequals(dm->getTimeLimitSecs(), duration.count(), "time limit elapsed");

        return b;
    }

    bool test_deathmatch_winning_cond_frag_limit()
    {
        if (!testInitDeathmatch())
        {
            return false;
        }

        // TODO
        return false;
    }

};