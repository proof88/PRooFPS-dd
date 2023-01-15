#pragma once

/*
    ###################################################################################
    RegTestBasicServerClient2Players.h
    Regression test for PRooFPS-dd.
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <thread>

#include <Windows.h>

#include "../../../../PGE/PGE/UnitTests/UnitTest.h"
#include "Process.h"

class RegTestBasicServerClient2Players :
    public UnitTest
{
public:

    RegTestBasicServerClient2Players() :
        UnitTest(__FILE__)
    {
        memset(&procInfoServer, 0, sizeof(procInfoServer));
        memset(&procInfoClient, 0, sizeof(procInfoClient));
    }

    ~RegTestBasicServerClient2Players()
    {
    }

protected:

    virtual void Initialize() override
    {
        try
        {
            // exe will be searched in PR00FPS-dd work dir, which means that the tested executable
            // is manually put there: either the release or debug version can be put there.
            procInfoServer = process_stackoverflow_42531::Process::launchProcess("PRooFPS-dd.exe", "");
        }
        catch (const std::exception& e)
        {
            CConsole::getConsoleInstance().EOLn("Exception: %s", e.what());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    virtual void Finalize() override
    {
        process_stackoverflow_42531::Process::stopProcess(procInfoServer);
    }

    bool testMethod() override
    {
        return false;
    }

private:

    PROCESS_INFORMATION procInfoServer;
    PROCESS_INFORMATION procInfoClient;

    // ---------------------------------------------------------------------------

    RegTestBasicServerClient2Players(const RegTestBasicServerClient2Players&)
    {};

    RegTestBasicServerClient2Players& operator=(const RegTestBasicServerClient2Players&)
    {
        return *this;
    };

};