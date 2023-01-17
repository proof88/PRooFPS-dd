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
        UnitTest(__FILE__),
        hServerMainGameWindow(NULL),
        hClientMainGameWindow(NULL)
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
            StartGame(true /* server */);
            StartGame(false /* client */);
        }
        catch (const std::exception& e)
        {
            CConsole::getConsoleInstance().EOLn("Exception: %s", e.what());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    virtual void Finalize() override
    {
        // will change these ungraceful stops to sending ESCAPE key
        process_stackoverflow_42531::Process::stopProcess(procInfoClient);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        process_stackoverflow_42531::Process::stopProcess(procInfoServer);
    }

    bool testMethod() override
    {
        return false;
    }

private:

    void StartGame(bool bServer) noexcept(false)
    {
        // exe will be searched in PR00FPS-dd work dir, which means that the tested executable
        // is manually put there: either the release or debug version can be put there.
        if (bServer)
        {
            procInfoServer = process_stackoverflow_42531::Process::launchProcess("PRooFPS-dd.exe", "");
        }
        else
        {
            procInfoClient = process_stackoverflow_42531::Process::launchProcess("PRooFPS-dd.exe", "");
        }

        // server or client dialog
        HWND hDialogWndServerOrClient = 0;
        while (hDialogWndServerOrClient == 0)
        {
            hDialogWndServerOrClient = FindWindow(NULL, ":)");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        SetActiveWindow(hDialogWndServerOrClient);

        // select server or client button
        if (!bServer)
        {
            keybd_event(VK_RIGHT, 0, 0, 0);
            keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
        }
        keybd_event(VK_RETURN, 0, 0, 0);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);

        // fullscreen dialog
        HWND hDialogWndFullscreen = 0;
        while (hDialogWndFullscreen == 0)
        {
            hDialogWndFullscreen = FindWindow(NULL, ":)");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        SetActiveWindow(hDialogWndFullscreen);

        // select no fullscreen
        keybd_event(VK_RIGHT, 0, 0, 0);
        keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_RETURN, 0, 0, 0);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);

        // main game window
        HWND& hMainGameWindow = bServer ? hServerMainGameWindow : hClientMainGameWindow;
        hMainGameWindow = 0;
        while (hMainGameWindow == 0)
        {
            // FindWindow() is case-insensitive and searches only top-level windows
            hMainGameWindow = FindWindow(NULL, "PRooFPS-dd 0.1.0.0 Private Beta");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        // now wait until we CANNOT find this window anymore - it is when the title text changes because
        // the game loaded and its main loop is refreshing the title bar with additional FPS data.
        while (FindWindow(NULL, "PRooFPS-dd 0.1.0.0 Private Beta") != 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    PROCESS_INFORMATION procInfoServer;
    PROCESS_INFORMATION procInfoClient;
    HWND hServerMainGameWindow;
    HWND hClientMainGameWindow;

    // ---------------------------------------------------------------------------

    RegTestBasicServerClient2Players(const RegTestBasicServerClient2Players&)
    {};

    RegTestBasicServerClient2Players& operator=(const RegTestBasicServerClient2Players&)
    {
        return *this;
    };

};