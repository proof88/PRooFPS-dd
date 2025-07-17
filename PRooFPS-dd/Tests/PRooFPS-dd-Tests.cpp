/*
    ###################################################################################
    PRooFPS-dd-Tests.cpp
    PRooFPS-dd - Unit Tests
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include "stdafx.h"  // PCH

// need to define this macro so we can use Test::runTests() with Console lib
#ifndef TEST_WITH_CCONSOLE
#define TEST_WITH_CCONSOLE
#endif
#include "Test.h"

#ifndef WINPROOF88_ALLOW_VIRTUALKEYCODES
#define WINPROOF88_ALLOW_VIRTUALKEYCODES
#endif
#include "winproof88.h"   // part of PFL lib: https://github.com/proof88/PFL

// unit tests
#include "EventListerTest.h"
#include "GameModeTest.h"
#include "MapItemTest.h"
#include "MapcycleTest.h"
#include "MapsTest.h"
#include "PlayerTest.h"

// performance tests (benchmarks)
#include "EventListerPerfTest.h"

// regression smoke tests
#include "RegTestBasicServerClient2Players.h"
#include "RegTestMapChangeServerClient3Players.h"

/**
    Entry point of our application.

    This doesn't use any API directly.

    @param hInstance     Application's current instance handle.
    @param hPrevInstance Always NULL.
    @param lpCmdLine     Command line.
    @param nCmdShow      How to display the window, ignored.
    @return 0 in success case, positive value in case of initialization or shutdown error.
*/
// use the DebugTest_PRooFPS-dd project configuration where this TESTING macro is defined!
#ifdef TESTING
static CConsole& getConsole()
{
    return CConsole::getConsoleInstance();
}

int WINAPI WinMain(const _In_ HINSTANCE /*hInstance*/, const _In_opt_ HINSTANCE /*hPrevInstance*/, const _In_ LPSTR /*lpCmdLine*/, const _In_ int /*nCmdShow*/)
{
    const std::string sConTitle = std::string("Tests for ") + proofps_dd::GAME_NAME + " " + proofps_dd::GAME_VERSION;
    getConsole().Initialize(sConTitle.c_str(), true);
    //getConsole().SetLoggingState("4LLM0DUL3S", true);
    getConsole().SetErrorsAlwaysOn(false);

    getConsole().OLn("");
    // Expecting NDEBUG to be reliable: https://man7.org/linux/man-pages/man3/assert.3.html
#ifdef NDEBUG
    const char* const szBuildType = "Release";
#else
    const char* const szBuildType = "Debug";
#endif 
    getConsole().OLn("%s.", sConTitle.c_str());
    getConsole().OLn("Tests Built in %s mode, on %s @ %s.", szBuildType, __DATE__, __TIME__);
    getConsole().OLn("");

    // this cfgProfiles instance is needed to be kept in memory during all unit tests below, because
    // some unit tests use the graphics engine, which is constructed only when the first relevant unit test
    // initializes it, and no construction happens in later relevant unit tests by calling createAndGet(), since
    // the engine instance is static ... so the engine instance will want to use the same cfgProfiles instance
    // in other tests as well, which is this:
    PGEcfgProfiles cfgProfiles; // TODO: even the engine should be constructed here and passed to test, but now this approach is enough ...
    cfgProfiles.reinitialize("");

    std::vector<std::unique_ptr<Test>> unitTests;
    std::vector<std::unique_ptr<Test>> perfTests;
    std::vector<std::unique_ptr<Test>> regTests;
    
    // unit tests
    unitTests.push_back(std::unique_ptr<Test>(new EventListerTest()));
    unitTests.push_back(std::unique_ptr<Test>(new GameModeTest(cfgProfiles)));
    unitTests.push_back(std::unique_ptr<Test>(new MapItemTest(cfgProfiles)));
    unitTests.push_back(std::unique_ptr<Test>(new MapsTest(cfgProfiles)));
    unitTests.push_back(std::unique_ptr<Test>(new MapcycleTest()));
    unitTests.push_back(std::unique_ptr<Test>(new PlayerTest(cfgProfiles)));
    
    // performance tests (benchmarks)
    perfTests.push_back(std::unique_ptr<Test>(new EventListerPerfTest()));
    
    // regression tests
    //const proofps_dd::GameModeType gamemode = proofps_dd::GameModeType::TeamDeathMatch;
    //for (auto gamemode = proofps_dd::GameModeType::DeathMatch; gamemode != proofps_dd::GameModeType::Max; ++gamemode)
    //{
    //    regTests.push_back(std::unique_ptr<Test>(new RegTestBasicServerClient2Players(60, 60, 60, gamemode)));
    //    regTests.push_back(std::unique_ptr<Test>(new RegTestBasicServerClient2Players(60, 20, 60, gamemode)));
    //    regTests.push_back(std::unique_ptr<Test>(new RegTestBasicServerClient2Players(20, 20, 60, gamemode)));
    //    
    //    constexpr bool bAreWeTestingReleaseBuild = true;
    //    regTests.push_back(std::unique_ptr<Test>(
    //        new RegTestMapChangeServerClient3Players(60, 60, 60, gamemode, 3 /*iterations*/, bAreWeTestingReleaseBuild, 2 /*clients*/)
    //    ));
    //}
    
    Test::runTests(unitTests, getConsole, "Unit Tests");
    Test::runTests(perfTests, getConsole, "Performance Tests");
    Test::runTests(regTests,  getConsole, "Regression E2E Tests");
    
    system("pause");

    getConsole().Deinitialize();

    return 0;
} // WinMain()

#endif
