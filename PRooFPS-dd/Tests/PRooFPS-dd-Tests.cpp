/*
    ###################################################################################
    PRooFPS-dd-Tests.cpp
    PRooFPS-dd - Unit Tests
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "../../../../PGE/PGE/UnitTests/UnitTest.h"
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "../../../../CConsole/CConsole/src/CConsole.h"

// unit tests
#include "GameModeTest.h"
#include "MapItemTest.h"
#include "MapsTest.h"
#include "PlayerTest.h"

// regression smoke tests
#include "RegTestBasicServerClient2Players.h"

static constexpr const char* CON_TITLE = "Tests for PRooFPS-dd";

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
    getConsole().Initialize(CON_TITLE, true);
    //getConsole().SetLoggingState("4LLM0DUL3S", true);
    getConsole().SetErrorsAlwaysOn(false);
    getConsole().OLn(CON_TITLE);
    getConsole().L();
    getConsole().OLn("");

    // this cfgProfiles instance is needed to kept in memory during all unit tests below, because
    // some unit tests use the graphics engine, which is constructed only when the first relevant unit test
    // initializes it, and no construction happens in later relevant unit tests by calling createAndGet(), since
    // the engine instance is static ... so the engine instance will want to use the same cfgProfiles instance
    // in other tests as well, which is this:
    PGEcfgProfiles cfgProfiles(""); // TODO: even the engine should be constructed here and passed to test, but now this approach is enough ...

    std::vector<std::unique_ptr<UnitTest>> tests;
    
    // unit tests
    tests.push_back(std::unique_ptr<UnitTest>(new GameModeTest(cfgProfiles)));
    tests.push_back(std::unique_ptr<UnitTest>(new MapItemTest(cfgProfiles)));
    tests.push_back(std::unique_ptr<UnitTest>(new MapsTest(cfgProfiles)));
    tests.push_back(std::unique_ptr<UnitTest>(new PlayerTest(cfgProfiles)));

    // regression tests
    tests.push_back(std::unique_ptr<UnitTest>(new RegTestBasicServerClient2Players()));

    std::vector<UnitTest*>::size_type nSucceededTests = 0;
    std::vector<UnitTest*>::size_type nTotalSubTests = 0;
    std::vector<UnitTest*>::size_type nTotalPassedSubTests = 0;
    for (std::vector<UnitTest*>::size_type i = 0; i < tests.size(); ++i)
    {
        getConsole().OLn("Running test %d / %d ... ", i+1, tests.size());
        tests[i]->run();
    }

    // summarizing
    getConsole().OLn("");
    for (std::vector<UnitTest*>::size_type i = 0; i < tests.size(); ++i)
    {
        if ( tests[i]->isPassed() )
        {
            ++nSucceededTests;
            getConsole().SOn();
            if ( tests[i]->getName().empty() )
            {
                getConsole().OLn("Test passed: %s(%d)!", tests[i]->getFile().c_str(), tests[i]->getSubTestCount());
            }
            else if ( tests[i]->getFile().empty() )
            {
                getConsole().OLn("Test passed: %s(%d)!", tests[i]->getName().c_str(), tests[i]->getSubTestCount());
            }
            else
            {
                getConsole().OLn("Test passed: %s(%d) in %s!", tests[i]->getName().c_str(), tests[i]->getSubTestCount(), tests[i]->getFile().c_str());
            }
            getConsole().SOff();
        }
        else
        {
            getConsole().EOn();
            if ( tests[i]->getName().empty() )
            {
                getConsole().OLn("Test failed: %s", tests[i]->getFile().c_str());
            }
            else if ( tests[i]->getFile().empty() )
            {
                getConsole().OLn("Test failed: %s", tests[i]->getName().c_str());
            }
            else
            {
                getConsole().OLn("Test failed: %s in %s", tests[i]->getName().c_str(), tests[i]->getFile().c_str());
            }
            getConsole().Indent();
            for (std::vector<std::string>::size_type j = 0; j < tests[i]->getMessages().size(); ++j)
            {
                getConsole().OLn("%s", tests[i]->getMessages()[j].c_str());
            }
            getConsole().Outdent();
            getConsole().EOff();
        }
        nTotalSubTests += tests[i]->getSubTestCount();
        nTotalPassedSubTests += tests[i]->getPassedSubTestCount();
    }

    getConsole().OLn("");
    getConsole().OLn("========================================================");
    if ( nSucceededTests == tests.size() )
    {
        getConsole().SOn();
    }
    else
    {
        getConsole().EOn();
    }
    getConsole().OLn("Passed tests: %d / %d (SubTests: %d / %d)", nSucceededTests, tests.size(), nTotalPassedSubTests, nTotalSubTests);
    getConsole().NOn();
    getConsole().OLn("========================================================");
    
    system("pause");

    getConsole().Deinitialize();

    return 0;
} // WinMain()

#endif
