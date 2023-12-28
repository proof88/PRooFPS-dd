#pragma once

/*
    ###################################################################################
    RegTestMapChangeServerClient3Players.h
    Map Changing Regression test for PRooFPS-dd with 3 players: 1 server, 2 clients.
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <cstdio>
#include <thread>

#ifndef WINPROOF88_ALLOW_VIRTUALKEYCODES
#define WINPROOF88_ALLOW_VIRTUALKEYCODES
#endif
#include "../../../PFL/PFL/winproof88.h"

#include "../../../PGE/PGE/UnitTests/UnitTest.h"
#include "../Consts.h"
#include "../GameMode.h"
#include "../SharedWithTest.h"
#include "InputSim.h"
#include "Process.h"

class RegTestMapChangeServerClient3Players :
    public UnitTest
{
public:

    enum class InstanceType
    {
        SERVER,
        CLIENT
    };

    RegTestMapChangeServerClient3Players(
        const unsigned int& nTickrate,
        const unsigned int& nClUpdateRate,
        const unsigned int& nPhysicsRateMin) :
        UnitTest(std::string(__FILE__) +
            " tickrate: " + std::to_string(nTickrate) +
            ", cl_updaterate: " + std::to_string(nClUpdateRate) +
            ", physics_rate_min: " + std::to_string(nPhysicsRateMin)),
        m_nTickRate(nTickrate),
        m_nClUpdateRate(nClUpdateRate),
        m_nPhysicsRateMin(nPhysicsRateMin),
        m_nPlayerCounter(0),
        hServerMainGameWindow(NULL),
        hClientMainGameWindow(NULL)
    {
        memset(&procInfoServer, 0, sizeof(procInfoServer));
        memset(&procInfoClient, 0, sizeof(procInfoClient));
        memset(&rectServerGameWindow, 0, sizeof(rectServerGameWindow));
        memset(&rectClientGameWindow, 0, sizeof(rectClientGameWindow));

        // unlike with RegTestBasicServerClient2Players, we don't need these restrictions in this test, but I still keep them
        // because officially we test with 60 and 20 anyway, since we want to support those 2 values only for now.
        if ((m_nTickRate != 60) && (m_nTickRate != 20))
        {
            throw std::runtime_error("Unsupported tick rate: " + std::to_string(m_nTickRate));
        }

        if ((m_nClUpdateRate != 60) && (m_nClUpdateRate != 20))
        {
            throw std::runtime_error("Unsupported client update rate: " + std::to_string(m_nClUpdateRate));
        }
    }

    ~RegTestMapChangeServerClient3Players()
    {
    }

    RegTestMapChangeServerClient3Players(const RegTestMapChangeServerClient3Players&) = delete;
    RegTestMapChangeServerClient3Players& operator=(const RegTestMapChangeServerClient3Players&) = delete;
    RegTestMapChangeServerClient3Players(RegTestMapChangeServerClient3Players&&) = delete;
    RegTestMapChangeServerClient3Players&& operator=(RegTestMapChangeServerClient3Players&&) = delete;

protected:

    virtual bool setUp() override
    {
        try
        {

            StartGame(InstanceType::SERVER);
            StartGame(InstanceType::CLIENT);

            // make sure the game windows are at the top, not the console windows
            input_sim_test::bringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            input_sim_test::bringWindowToFront(hClientMainGameWindow);
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
            input_sim_test::bringWindowToFront(hClientMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            input_sim_test::keybdPress(VK_ESCAPE, 100);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        else
        {
            process_stackoverflow_42531::Process::stopProcess(procInfoClient);
        }

        if (hServerMainGameWindow != NULL)
        {
            input_sim_test::bringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            input_sim_test::keybdPress(VK_ESCAPE, 100);
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

        // server instance initiates map change
        {
            input_sim_test::bringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            input_sim_test::keybdPress((unsigned char)VkKeyScan('m'), 100);
        }

        // wait for all instances change the map
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        {
            // client to front
            input_sim_test::bringWindowToFront(hClientMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // trigger dump test data to file
            input_sim_test::keybdPress(VK_RETURN, 100);

            // server to front
            input_sim_test::bringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // trigger dump test data to file
            input_sim_test::keybdPress(VK_RETURN, 100);
        }

        // wait for the test data files to be actually written
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        return evaluateTest();
    }

private:

    std::vector<proofps_dd::FragTableRow> evaluateFragTable;

    unsigned int m_nTickRate;
    unsigned int m_nClUpdateRate;
    unsigned int m_nPhysicsRateMin;
    unsigned int m_nPlayerCounter;
    PROCESS_INFORMATION procInfoServer;
    PROCESS_INFORMATION procInfoClient;
    HWND hServerMainGameWindow;
    HWND hClientMainGameWindow;
    RECT rectServerGameWindow;  // screen coordinates
    RECT rectClientGameWindow;  // screen coordinates

    bool evaluateInstance(const InstanceType& instType)
    {
        const bool bServer = instType == InstanceType::SERVER;
        const std::string sTestDumpFilename = proofps_dd::generateTestDumpFilename(bServer, m_nTickRate, m_nClUpdateRate, m_nPhysicsRateMin);
        std::ifstream f(sTestDumpFilename, std::ifstream::in);
        if (!f.good())
        {
            return assertFalse(true,
                (std::string("failed to open file: ") + sTestDumpFilename).c_str()
            );
        }

        std::set<std::string> setOfExpectedPlayerNames;
        for (unsigned int i = 1; i <= m_nPlayerCounter; i++)
        {
            setOfExpectedPlayerNames.insert("Player" + std::to_string(i));
        }

        const std::streamsize nBuffSize = 1024;
        char szLine[nBuffSize];
        unsigned int nErasedNameCounter = 0;
        bool bRet = true;
        // read frag table
        {
            // fast-forward to frag table
            while (!f.eof())
            {
                f.getline(szLine, nBuffSize);
                const std::string sLine = szLine;
                if (sLine.find("Frag Table:") != std::string::npos)
                {
                    break;
                }
            }
            
            while (!f.eof())
            {
                proofps_dd::FragTableRow ftRow;
                f >> ftRow.m_sName;
                if (ftRow.m_sName.empty() || (ftRow.m_sName.find("Weapons") != std::string::npos))
                {
                    // actually if this is empty, it might be expected scenario because we have an extra empty line after
                    // the frag table in the file, anyway just stop here, the evaluate functions will verify the read frag table data anyway!
                    break;
                }

                if (setOfExpectedPlayerNames.find(ftRow.m_sName) == setOfExpectedPlayerNames.end())
                {
                    // unexpected name found in frag table!
                    bRet = false;
                    break;
                }
                else
                {
                    setOfExpectedPlayerNames.erase(ftRow.m_sName);
                    nErasedNameCounter++;
                }

                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                f >> ftRow.m_nFrags;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                f >> ftRow.m_nDeaths;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                evaluateFragTable.push_back(ftRow);
            }
        }

        f.close();
        return assertTrue(bRet, "an instance had an unexpected name in its frag table!") &
            assertEquals(m_nPlayerCounter, nErasedNameCounter, "Number of erased names vs number of generated names") &
            assertTrue(setOfExpectedPlayerNames.empty(), "set is still not empty!");
    }

    bool evaluateTest()
    {
        // unlike with RegTestBasicServerClient2Players, in this test evaluate order is irrelevant
        bool bRet = assertTrue(evaluateInstance(InstanceType::SERVER), "evaluateServer") &
            assertTrue(evaluateInstance(InstanceType::CLIENT), "evaluateClient");

        return bRet;
    }

    void StartGame(const InstanceType& instType) noexcept(false)
    {
        const bool bServer = instType == InstanceType::SERVER;
        CConsole::getConsoleInstance().OLnOI("%s(%b) ...", __func__, bServer);

        m_nPlayerCounter++;
        const std::string sPlayerName = "Player" + std::to_string(m_nPlayerCounter);

        // exe will be searched in PRooFPS-dd work dir, which means that the tested executable
        // needs to be manually put there: either the release or debug version can be put there.
        if (bServer)
        {
            procInfoServer = process_stackoverflow_42531::Process::launchProcess(
                "PRooFPS-dd.exe",
                "--gfx_windowed=true --net_server=true --sv_map=map_test_good.txt --testing=true --tickrate=" +
                std::to_string(m_nTickRate) + " --cl_updaterate=" +
                std::to_string(m_nClUpdateRate) + " --physics_rate_min=" +
                std::to_string(m_nPhysicsRateMin) + " --cl_name=" + sPlayerName);
        }
        else
        {
            procInfoClient = process_stackoverflow_42531::Process::launchProcess(
                "PRooFPS-dd.exe",
                "--gfx_windowed=true --net_server=false --cl_server_ip=127.0.0.1 --testing=true --tickrate=" +
                std::to_string(m_nTickRate) + " --cl_updaterate=" +
                std::to_string(m_nClUpdateRate) + " --physics_rate_min=" +
                std::to_string(m_nPhysicsRateMin) + " --cl_name=" + sPlayerName);
        }

        // main game window
        CConsole::getConsoleInstance().OLn("Trying to find main game window ...");
        HWND& hMainGameWindow = bServer ? hServerMainGameWindow : hClientMainGameWindow;
        hMainGameWindow = 0;
        unsigned int iWaitCntr = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            // FindWindow() is case-insensitive and searches only top-level windows
            hMainGameWindow = FindWindow(NULL, std::string(proofps_dd::GAME_NAME + " " + proofps_dd::GAME_VERSION).c_str());
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
                /* traditionally, server goes to left edge, client 1 goes to right edge, subsequent clients go somewhere in between from left to right */
                bServer ? 0 : ((m_nPlayerCounter > 2) ? 200 + m_nPlayerCounter*20 : 900),
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
            hTmpGameWindow = FindWindow(NULL, std::string(proofps_dd::GAME_NAME + " " + proofps_dd::GAME_VERSION).c_str());
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

    // ---------------------------------------------------------------------------

};