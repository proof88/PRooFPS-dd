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

#include "GameModeTest.h"
#include "MapItemTest.h"
#include "MapsTest.h"

#define CON_TITLE "Unit tests for PRooFPS-dd"

using namespace std;

/**
    Entry point of our application.

    This doesn't use any API directly.

    @param hInstance     Application's current instance handle.
    @param hPrevInstance Always NULL.
    @param lpCmdLine     Command line.
    @param nCmdShow      How to display the window, ignored.
    @return 0 in success case, positive value in case of initialization or shutdown error.
*/

#pragma warning(disable:4100)  /* unreferenced formal parameter */

// use the DebugTest_PRooFPS-dd project configuration where this TESTING macro is defined!
#ifdef TESTING
static CConsole& getConsole()
{
    return CConsole::getConsoleInstance();
}

int WINAPI WinMain(const HINSTANCE hInstance, const HINSTANCE hPrevInstance, const LPSTR lpCmdLine, const int nCmdShow)
{
    getConsole().Initialize(CON_TITLE, true);
    //getConsole().SetLoggingState("4LLM0DUL3S", true);
    getConsole().SetErrorsAlwaysOn(false);
    getConsole().OLn(CON_TITLE);
    getConsole().L();
    getConsole().OLn("");

    GameModeTest gameModeTest;
    MapItemTest mapItemTest;
    MapsTest mapsTest;

    vector<UnitTest*> tests;
    tests.push_back(&gameModeTest);
    tests.push_back(&mapItemTest);
    tests.push_back(&mapsTest);

    vector<UnitTest*>::size_type nSucceededTests = 0;
    vector<UnitTest*>::size_type nTotalSubTests = 0;
    vector<UnitTest*>::size_type nTotalPassedSubTests = 0;
    for (vector<UnitTest*>::size_type i = 0; i < tests.size(); ++i)
    {
        getConsole().OLn("Running test %d / %d ... ", i+1, tests.size());
        tests[i]->run();
    }

    // summarizing
    getConsole().OLn("");
    for (vector<UnitTest*>::size_type i = 0; i < tests.size(); ++i)
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
            for (vector<string>::size_type j = 0; j < tests[i]->getMessages().size(); ++j)
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

