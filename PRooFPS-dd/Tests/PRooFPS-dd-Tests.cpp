/*
    ###################################################################################
    PRooFPS-dd-Tests.cpp
    PRooFPS-dd - Unit Tests
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "UnitTest.h"
#include <memory>  // for std::unique_ptr; requires cpp11
#include <vector>

#ifndef WINPROOF88_ALLOW_VIRTUALKEYCODES
#define WINPROOF88_ALLOW_VIRTUALKEYCODES
#endif
#include "winproof88.h"

#include "CConsole.h"

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
    // Expecting NDEBUG to be reliable: https://man7.org/linux/man-pages/man3/assert.3.html
#ifdef NDEBUG
    const std::string sBuildType = "Release Build";
#else
    const std::string sBuildType = "Debug Build";
#endif 
    getConsole().OLn((std::string(CON_TITLE) + ", " + sBuildType).c_str());
    getConsole().L();
    getConsole().OLn("");

    // this cfgProfiles instance is needed to kept in memory during all unit tests below, because
    // some unit tests use the graphics engine, which is constructed only when the first relevant unit test
    // initializes it, and no construction happens in later relevant unit tests by calling createAndGet(), since
    // the engine instance is static ... so the engine instance will want to use the same cfgProfiles instance
    // in other tests as well, which is this:
    PGEcfgProfiles cfgProfiles; // TODO: even the engine should be constructed here and passed to test, but now this approach is enough ...
    cfgProfiles.reinitialize("");

    std::vector<std::unique_ptr<UnitTest>> unitTests;
    std::vector<std::unique_ptr<Benchmark>> perfTests;
    
    // unit tests
    unitTests.push_back(std::unique_ptr<UnitTest>(new EventListerTest()));
    //unitTests.push_back(std::unique_ptr<UnitTest>(new GameModeTest(cfgProfiles)));
    //unitTests.push_back(std::unique_ptr<UnitTest>(new MapItemTest(cfgProfiles)));
    //unitTests.push_back(std::unique_ptr<UnitTest>(new MapsTest(cfgProfiles)));
    //unitTests.push_back(std::unique_ptr<UnitTest>(new MapcycleTest()));
    //unitTests.push_back(std::unique_ptr<UnitTest>(new PlayerTest(cfgProfiles)));

    // performance tests (benchmarks)
    perfTests.push_back(std::unique_ptr<Benchmark>(new EventListerPerfTest()));
    
    // regression tests
    //unitTests.push_back(std::unique_ptr<UnitTest>(new RegTestBasicServerClient2Players(60, 60, 60)));
    //unitTests.push_back(std::unique_ptr<UnitTest>(new RegTestBasicServerClient2Players(60, 20, 60)));
    //unitTests.push_back(std::unique_ptr<UnitTest>(new RegTestBasicServerClient2Players(20, 20, 60)));
    //constexpr bool bAreWeTestingReleaseBuild = false;
    //unitTests.push_back(std::unique_ptr<UnitTest>(
    //    new RegTestMapChangeServerClient3Players(60, 60, 60, 3 /*iterations*/, bAreWeTestingReleaseBuild, 2 /*clients*/)
    //));
    
    size_t nSucceededTests = 0;
    size_t nTotalSubTests = 0;
    size_t nTotalPassedSubTests = 0;
    for (size_t i = 0; i < unitTests.size(); ++i)
    {
        getConsole().OLn("Running test %d / %d ... ", i+1, unitTests.size());
        unitTests[i]->run();
    }

    // summarizing
    getConsole().OLn("");
    for (size_t i = 0; i < unitTests.size(); ++i)
    {
        if ( unitTests[i]->isPassed() )
        {
            ++nSucceededTests;
            getConsole().SOn();
            if ( unitTests[i]->getName().empty() )
            {
                getConsole().OLn("Test passed: %s(%d)!", unitTests[i]->getFile().c_str(), unitTests[i]->getSubTestCount());
            }
            else if ( unitTests[i]->getFile().empty() )
            {
                getConsole().OLn("Test passed: %s(%d)!", unitTests[i]->getName().c_str(), unitTests[i]->getSubTestCount());
            }
            else
            {
                getConsole().OLn("Test passed: %s(%d) in %s!", unitTests[i]->getName().c_str(), unitTests[i]->getSubTestCount(), unitTests[i]->getFile().c_str());
            }
            getConsole().SOff();
        }
        else
        {
            getConsole().EOn();
            if ( unitTests[i]->getName().empty() )
            {
                getConsole().OLn("Test failed: %s", unitTests[i]->getFile().c_str());
            }
            else if ( unitTests[i]->getFile().empty() )
            {
                getConsole().OLn("Test failed: %s", unitTests[i]->getName().c_str());
            }
            else
            {
                getConsole().OLn("Test failed: %s in %s", unitTests[i]->getName().c_str(), unitTests[i]->getFile().c_str());
            }
            getConsole().Indent();
            for (size_t j = 0; j < unitTests[i]->getErrorMessages().size(); ++j)
            {
                getConsole().OLn("%s", unitTests[i]->getErrorMessages()[j].c_str());
            }
            getConsole().Outdent();
            getConsole().EOff();
        }
        nTotalSubTests += unitTests[i]->getSubTestCount();
        nTotalPassedSubTests += unitTests[i]->getPassedSubTestCount();
    }

    getConsole().OLn("");
    getConsole().OLn("========================================================");
    if ( nSucceededTests == unitTests.size() )
    {
        getConsole().SOn();
    }
    else
    {
        getConsole().EOn();
    }
    getConsole().OLn("Passed Unit Tests: %d / %d (SubTests: %d / %d)", nSucceededTests, unitTests.size(), nTotalPassedSubTests, nTotalSubTests);
    getConsole().NOn();
    getConsole().OLn("========================================================");

    nSucceededTests = 0;
    nTotalSubTests = 0;
    nTotalPassedSubTests = 0;
    for (size_t i = 0; i < perfTests.size(); ++i)
    {
        getConsole().OLn("Running performance test %d / %d ... ", i + 1, perfTests.size());
        perfTests[i]->run();
    }

    // summarizing
    getConsole().OLn("");
    for (size_t i = 0; i < perfTests.size(); ++i)
    {
        for (const auto& infoMsg : perfTests[i]->getInfoMessages())
        {
            getConsole().OLn("%s", infoMsg.c_str());
        }

        if (perfTests[i]->isPassed())
        {
            ++nSucceededTests;
            getConsole().SOn();
            if (perfTests[i]->getName().empty())
            {
                getConsole().OLn("Test passed: %s(%d)!", perfTests[i]->getFile().c_str(), perfTests[i]->getSubTestCount());
            }
            else if (perfTests[i]->getFile().empty())
            {
                getConsole().OLn("Test passed: %s(%d)!", perfTests[i]->getName().c_str(), perfTests[i]->getSubTestCount());
            }
            else
            {
                getConsole().OLn("Test passed: %s(%d) in %s!", perfTests[i]->getName().c_str(), perfTests[i]->getSubTestCount(), perfTests[i]->getFile().c_str());
            }
            getConsole().SOff();
        }
        else
        {
            getConsole().EOn();
            if (perfTests[i]->getName().empty())
            {
                getConsole().OLn("Test failed: %s", perfTests[i]->getFile().c_str());
            }
            else if (perfTests[i]->getFile().empty())
            {
                getConsole().OLn("Test failed: %s", perfTests[i]->getName().c_str());
            }
            else
            {
                getConsole().OLn("Test failed: %s in %s", perfTests[i]->getName().c_str(), perfTests[i]->getFile().c_str());
            }
            getConsole().Indent();
            for (size_t j = 0; j < perfTests[i]->getErrorMessages().size(); ++j)
            {
                getConsole().OLn("%s", perfTests[i]->getErrorMessages()[j].c_str());
            }
            getConsole().Outdent();
            getConsole().EOff();
        }
        nTotalSubTests += perfTests[i]->getSubTestCount();
        nTotalPassedSubTests += perfTests[i]->getPassedSubTestCount();
    }

    getConsole().OLn("");
    getConsole().OLn("========================================================");
    if (nSucceededTests == perfTests.size())
    {
        getConsole().SOn();
    }
    else
    {
        getConsole().EOn();
    }
    getConsole().OLn("Passed Performance Tests: %d / %d (SubTests: %d / %d)", nSucceededTests, perfTests.size(), nTotalPassedSubTests, nTotalSubTests);
    getConsole().NOn();
    getConsole().OLn("========================================================");
    
    system("pause");

    getConsole().Deinitialize();

    return 0;
} // WinMain()

#endif
