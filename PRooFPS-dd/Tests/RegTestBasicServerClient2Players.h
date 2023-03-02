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
#include "../Consts.h"
#include "Input.h"
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
        memset(&rectServerGameWindow, 0, sizeof(rectServerGameWindow));
        memset(&rectClientGameWindow, 0, sizeof(rectClientGameWindow));
        
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
            // TearDown() is always invoked, so we don't have to do any cleanup here.
            CConsole::getConsoleInstance().EOLn("Exception: %s", e.what());
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return true;
    } // setUp()

    virtual void TearDown() override
    {
        // We primarily try to do graceful shutdown here.
        // UnitTest::run() will invoke TearDown() no matter what setUp() or testMethod() returned, this means that
        // when we come here, even setUp() might had failed, so try graceful shutdown only if game windows are actually available!
        // Otherwise do ungraceful process stopping.

        if (hClientMainGameWindow != NULL)
        {
            BringWindowToFront(hClientMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            input::keybdPress(VK_ESCAPE, 100);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        else
        {
            process_stackoverflow_42531::Process::stopProcess(procInfoClient);
        }

        if (hServerMainGameWindow != NULL)
        {
            BringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            input::keybdPress(VK_ESCAPE, 100);
        }
        else
        {
            process_stackoverflow_42531::Process::stopProcess(procInfoServer);
        }

        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } while (process_stackoverflow_42531::Process::checkIfProcessIsActive(procInfoClient));

        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } while (process_stackoverflow_42531::Process::checkIfProcessIsActive(procInfoServer));
    } // TearDown()

    bool testMethod() override
    {
        // Game instances reposition the mouse cursor into the center of the window in every frame, keep that in mind when specifying relative coords!
        // By default, in testing mode, initially the server points xhair to the right, client points xhair to the left.

        // server player moves into position
        {
            BringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            input::keybdPressNoRelease(VK_RIGHT);
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                input::keybdPress(VK_SPACE, 100); // jump over the hole
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                input::keybdPress(VK_SPACE, 100); // 1st crate
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                input::keybdPress(VK_SPACE, 100); // 2nd crate
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                input::keybdPress(VK_SPACE, 100); // 3rd crate
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            input::keybdRelease(VK_RIGHT);
        }

        // client player moves into position and shoots
        {
            BringWindowToFront(hClientMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            input::keybdPressNoRelease(VK_LEFT);
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(850));
                input::keybdPress(VK_SPACE, 100); // jump over the hole
                std::this_thread::sleep_for(std::chrono::milliseconds(3500));
            }
            input::keybdRelease(VK_LEFT);

            // now server and client player should be just a few blocks from each other

            // client should be able to switch to machinegun using scrolling forward/up
            input::mouseScroll(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            // shoot 2 bullets
            input::mouseClick(300);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        // server player also shoots
        {
            BringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // shoot 1 bullet
            // intentionally holding down mouse button for 2000 msecs, just for testing purpose, pistol should shoot only 1 bullet!
            input::mouseClick(2000);

            // shoot 1 bullet again
            // intentionally holding down mouse button for 2000 msecs, just for testing purpose, pistol should shoot only 1 bullet!
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            input::mouseClick(2000);
        }

        // client player shoots again and kills server player
        {
            BringWindowToFront(hClientMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // shoot 4 bullets
            input::mouseClick(600);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        // wait for the killed server player to respawn
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        // TODO: check players' frag and death count
        // TODO: check weapon bullet counts for players
        // TODO: check health of players
        // TODO: check sent and received pkts for both players

        return true;
    }

private:

    void BringWindowToFront(HWND hTargetWindow) noexcept(false)
    {
        // technique copied from: https://stackoverflow.com/questions/916259/win32-bring-a-window-to-top
        const HWND hCurWnd = GetForegroundWindow();
        if (hCurWnd == NULL)
        {
            // Win32 SDK: "The foreground window can be NULL in certain circumstances, such as when a window is losing activation."
            // I still want to report this as an error since we use hCurWnd as input to other functions.
            throw std::exception(
                std::string("ERROR: GetForegroundWindow() returned NULL!").c_str());
        }

        const DWORD dwMyID = GetCurrentThreadId();
        if (dwMyID == 0)
        {
            throw std::exception(
                std::string("ERROR: GetCurrentThreadId() returned 0!").c_str());
        }

        const DWORD dwCurID = GetWindowThreadProcessId(hCurWnd, NULL);
        if (dwCurID == 0)
        {
            throw std::exception(
                std::string("ERROR: GetWindowThreadProcessId() returned 0!").c_str());
        }

        if (0 == AttachThreadInput(dwCurID, dwMyID, TRUE))
        {
            throw std::exception(
                std::string("ERROR: AttachThreadInput(..., TRUE) failed: " + std::to_string(GetLastError()) + "!").c_str());
        }

        if (0 == SetWindowPos(hTargetWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE))
        {
            throw std::exception(
                std::string("ERROR: SetWindowPos(TOPMOST) failed: " + std::to_string(GetLastError()) + "!").c_str());
        }

        if (0 == SetWindowPos(hTargetWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE))
        {
            throw std::exception(
                std::string("ERROR: SetWindowPos(NOTOPMOST) failed: " + std::to_string(GetLastError()) + "!").c_str());
        }

        if (0 == SetForegroundWindow(hTargetWindow))
        {
            throw std::exception(
                std::string("ERROR: SetForegroundWindow() failed!").c_str());
        }

        // This actually returns NULL for some reason, on for the client window, with error code 5: access denied.
        //if (NULL == SetFocus(hTargetWindow))
        //{
        //    throw std::exception(
        //        std::string("ERROR: SetFocus() failed: " + std::to_string(GetLastError()) + "!").c_str());
        //}

        // This also returns NULL for some reason, only for the client window, with error code 0: error_success.
        //if (NULL == SetActiveWindow(hTargetWindow))
        //{
        //    throw std::exception(
        //        std::string("ERROR: SetActiveWindow() failed: " + std::to_string(GetLastError()) + "!").c_str());
        //}

        if (0 == AttachThreadInput(dwCurID, dwMyID, FALSE))
        {
            throw std::exception(
                std::string("ERROR: AttachThreadInput(..., FALSE) failed: " + std::to_string(GetLastError()) + "!").c_str());
        }
    } // BringWindowToFront()

    void StartGame(bool bServer) noexcept(false)
    {
        CConsole::getConsoleInstance().OLnOI("%s(%b) ...", __func__, bServer);

        // exe will be searched in PR00FPS-dd work dir, which means that the tested executable
        // is manually put there: either the release or debug version can be put there.
        if (bServer)
        {
            procInfoServer = process_stackoverflow_42531::Process::launchProcess(
                "PRooFPS-dd.exe",
                "--gfx_windowed=true --net_server=true --sv_map=map_test_good.txt --testing=true");
        }
        else
        {
            procInfoClient = process_stackoverflow_42531::Process::launchProcess(
                "PRooFPS-dd.exe",
                "--gfx_windowed=true --net_server=false --cl_server_ip=127.0.0.1 --testing=true");
        }

        // Following commented code is only for the old case when app showed dialog box about server and fullscreen.
        // But this is not needed anymore, since we pass the required config in command line now.
        // However, in the future we may use this code for other code, testing if dialog boxes correctly appear without passing config!
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
        CConsole::getConsoleInstance().OLn("Trying to find main game window ...");
        HWND& hMainGameWindow = bServer ? hServerMainGameWindow : hClientMainGameWindow;
        hMainGameWindow = 0;
        unsigned int iWaitCntr = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            // FindWindow() is case-insensitive and searches only top-level windows
            hMainGameWindow = FindWindow(NULL, std::string(GAME_NAME + " " + GAME_VERSION).c_str());
            iWaitCntr++;
        } while ((hMainGameWindow == 0) && (iWaitCntr < 10));
        if (hMainGameWindow == 0)
        {
            CConsole::getConsoleInstance().OO();
            throw std::exception(
                (std::string("ERROR: Failed to find window of ") +
                    (bServer ? "server" : "client") +
                    " instance!").c_str());
        }

        CConsole::getConsoleInstance().SOLn("Found game window, fetching RECT ...");
        RECT& rectGameWindow = bServer ? rectServerGameWindow : rectClientGameWindow;
        if (TRUE == GetWindowRect(hMainGameWindow, &rectGameWindow))
        {
            if (FALSE == SetWindowPos(
                hMainGameWindow,
                NULL,
                bServer ? 0 : 900,
                rectGameWindow.top,
                0, 0,
                SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER))
            {
                CConsole::getConsoleInstance().OO();
                throw std::exception(
                    ((std::string("ERROR: SetWindowPos() failed ( ") + std::to_string(GetLastError()) + ")!").c_str()));
            }
        }
        else
        {
            CConsole::getConsoleInstance().OO();
            throw std::exception(
                ((std::string("ERROR: GetWindowRect() failed ( ") + std::to_string(GetLastError()) + ")!").c_str()));
        }

        // fetch again the new rect, we will use this data later
        CConsole::getConsoleInstance().SOLn("Repositioned window, fetching RECT again ...");
        if (FALSE == GetWindowRect(hMainGameWindow, &rectGameWindow))
        {
            CConsole::getConsoleInstance().OO();
            throw std::exception(
                ((std::string("ERROR: 2nd GetWindowRect() failed ( ") + std::to_string(GetLastError()) + ")!").c_str()));
        }

        // now wait until we CANNOT find this window anymore - it is when the title text changes because
        // the game loaded and its main loop is refreshing the title bar with additional FPS data.
        CConsole::getConsoleInstance().SOLn("Fetched RECT again, waiting for main game window to change its title bar text ...");
        HWND hTmpGameWindow = hMainGameWindow;
        iWaitCntr = 0;
        while ((hTmpGameWindow != 0) && (iWaitCntr < 10))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            hTmpGameWindow = FindWindow(NULL, std::string(GAME_NAME + " " + GAME_VERSION).c_str());
            iWaitCntr++;
        }
        if (hTmpGameWindow != 0)
        {
            CConsole::getConsoleInstance().OO();
            throw std::exception(
                (std::string("ERROR: Still able to find window of ") +
                    (bServer ? "server" : "client") +
                    " instance with initial window title which should have changed already!").c_str());
        }

        CConsole::getConsoleInstance().SOLnOO("> %s(%b) Successful!", __func__, bServer);
    } // StartGame()

    PROCESS_INFORMATION procInfoServer;
    PROCESS_INFORMATION procInfoClient;
    HWND hServerMainGameWindow;
    HWND hClientMainGameWindow;
    RECT rectServerGameWindow;  // screen coordinates
    RECT rectClientGameWindow;  // screen coordinates

    // ---------------------------------------------------------------------------

    RegTestBasicServerClient2Players(const RegTestBasicServerClient2Players&)
    {};

    RegTestBasicServerClient2Players& operator=(const RegTestBasicServerClient2Players&)
    {
        return *this;
    };

};