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

    virtual bool setUp() override
    {
        try
        {
            StartGame(true /* server */);
            StartGame(false /* client */);

            // make sure the game windows are at the top, not the console windows
            BringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            BringWindowToFront(hClientMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        catch (const std::exception& e)
        {
            CConsole::getConsoleInstance().EOLn("Exception: %s", e.what());
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return true;
    }

    virtual void TearDown() override
    {
        //process_stackoverflow_42531::Process::stopProcess(procInfoClient);
        BringWindowToFront(hClientMainGameWindow);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        keybd_event(VK_ESCAPE, 0, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        //process_stackoverflow_42531::Process::stopProcess(procInfoServer);
        BringWindowToFront(hServerMainGameWindow);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        keybd_event(VK_ESCAPE, 0, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);

        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } while (process_stackoverflow_42531::Process::checkIfProcessIsActive(procInfoClient));

        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } while (process_stackoverflow_42531::Process::checkIfProcessIsActive(procInfoServer));
    }

    bool testMethod() override
    {
        return true;
    }

private:

    void BringWindowToFront(HWND hTargetWindow)
    {
        // technique copied from: https://stackoverflow.com/questions/916259/win32-bring-a-window-to-top
        HWND hCurWnd = GetForegroundWindow();
        DWORD dwMyID = GetCurrentThreadId();
        DWORD dwCurID = GetWindowThreadProcessId(hCurWnd, NULL);
        AttachThreadInput(dwCurID, dwMyID, TRUE);
        SetWindowPos(hTargetWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        SetWindowPos(hTargetWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
        SetForegroundWindow(hTargetWindow);
        SetFocus(hTargetWindow);
        SetActiveWindow(hTargetWindow);
        AttachThreadInput(dwCurID, dwMyID, FALSE);
    }

    void StartGame(bool bServer) noexcept(false)
    {
        // exe will be searched in PR00FPS-dd work dir, which means that the tested executable
        // is manually put there: either the release or debug version can be put there.
        if (bServer)
        {
            procInfoServer = process_stackoverflow_42531::Process::launchProcess("PRooFPS-dd.exe", "--gfx_windowed=true --net_server=true");
        }
        else
        {
            procInfoClient = process_stackoverflow_42531::Process::launchProcess("PRooFPS-dd.exe", "--gfx_windowed=true --net_server=false");
        }

        // Following commented code is only for the old case when app showed dialoge box about server and fullscreen.
        //
        //// server or client dialog
        //HWND hDialogWndServerOrClient = 0;
        //while (hDialogWndServerOrClient == 0)
        //{
        //    hDialogWndServerOrClient = FindWindow(NULL, ":)");
        //    std::this_thread::sleep_for(std::chrono::milliseconds(200));
        //}
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        //SetActiveWindow(hDialogWndServerOrClient);
        //
        //// select server or client button
        //if (!bServer)
        //{
        //    keybd_event(VK_RIGHT, 0, 0, 0);
        //    keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
        //}
        //keybd_event(VK_RETURN, 0, 0, 0);
        //keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
        //
        //// fullscreen dialog
        //HWND hDialogWndFullscreen = 0;
        //do
        //{
        //    std::this_thread::sleep_for(std::chrono::milliseconds(200));
        //    hDialogWndFullscreen = FindWindow(NULL, ":)");
        //} while (hDialogWndFullscreen == 0);
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        //SetActiveWindow(hDialogWndFullscreen);
        //
        //// select no fullscreen
        //keybd_event(VK_RIGHT, 0, 0, 0);
        //keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
        //keybd_event(VK_RETURN, 0, 0, 0);
        //keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);

        // main game window
        HWND& hMainGameWindow = bServer ? hServerMainGameWindow : hClientMainGameWindow;
        hMainGameWindow = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            // FindWindow() is case-insensitive and searches only top-level windows
            hMainGameWindow = FindWindow(NULL, "PRooFPS-dd 0.1.0.0 Private Beta");
        } while (hMainGameWindow == 0);

        RECT rectGameWindow;
        if (TRUE == GetWindowRect(hMainGameWindow, &rectGameWindow))
        {
            SetWindowPos(
                hMainGameWindow,
                NULL,
                bServer ? 0 : 900,
                rectGameWindow.top,
                0, 0,
                SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
        }
        else
        {
            CConsole::getConsoleInstance().EOLn("GetWindowRect() failed (%d).", GetLastError());
        }

        // now wait until we CANNOT find this window anymore - it is when the title text changes because
        // the game loaded and its main loop is refreshing the title bar with additional FPS data.
        while (FindWindow(NULL, "PRooFPS-dd 0.1.0.0 Private Beta") != 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
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