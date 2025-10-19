#pragma once

/*
    ###################################################################################
    RegTestMapChangeServerClient3Players.h
    Map Changing Regression test for PRooFPS-dd with 3 players: 1 server, 2 clients.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include <cstdio>
#include <filesystem>  // requires cpp17
#include <thread>

#ifndef WINPROOF88_ALLOW_VIRTUALKEYCODES
#define WINPROOF88_ALLOW_VIRTUALKEYCODES
#endif
#include "winproof88.h"

#include "UnitTest.h"

#include "Consts.h"
#include "GameMode.h"
#include "SharedWithTest.h"
#include "InputSim.h"
#include "Process.h"

class RegTestMapChangeServerClient3Players :
    public UnitTest
{
public:

    RegTestMapChangeServerClient3Players(
        const unsigned int& nTickrate,
        const unsigned int& nClUpdateRate,
        const unsigned int& nPhysicsRateMin,
        const proofps_dd::GameModeType& eGameModeType,
        const unsigned int& nTestIterations,
        const bool& bAreWeTestingReleaseBuild,
        const unsigned int& nClients) :
        UnitTest(std::string(__FILE__) +
            " TiR: " + std::to_string(nTickrate) +
            ", UpR: " + std::to_string(nClUpdateRate) +
            ", PhR: " + std::to_string(nPhysicsRateMin) +
            ", gamemodetype: " + std::string(proofps_dd::GameMode::getGameModeTypeName(eGameModeType)) +
            ", iterations: " + std::to_string(nTestIterations)),
        m_nTickRate(nTickrate),
        m_nClUpdateRate(nClUpdateRate),
        m_nPhysicsRateMin(nPhysicsRateMin),
        m_eGameModeType(eGameModeType),
        m_nTestIterations(nTestIterations),
        m_nSecondsWaitForInstancesToChangeMap(bAreWeTestingReleaseBuild ? 8 : 11),
        m_nClients(nClients),
        m_nPlayerCounter(0),
        hServerMainGameWindow(static_cast<HWND>(0))
    {
        memset(&procInfoServer, 0, sizeof(procInfoServer));

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
    RegTestMapChangeServerClient3Players& operator=(RegTestMapChangeServerClient3Players&&) = delete;

protected:

    virtual bool setUp() override
    {
        try
        {

            StartGame(0 /*server*/);
            for (unsigned int iClient = 0; iClient < m_nClients; iClient++)
            {
                StartGame(iClient+1);
            }

            // make sure the game windows are at the top, not the console windows
            input_sim_test::bringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            for (unsigned int iClient = 0; iClient < m_nClients; iClient++)
            {
                input_sim_test::bringWindowToFront(m_vecHClientMainGameWindow[iClient]);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
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

    virtual void tearDown() override
    {
        // We primarily try to do graceful shutdown here.
        // UnitTest::run() will invoke TearDown() no matter what setUp() or testMethod() returned, this means that
        // when we come here, even setUp() might had failed, so try graceful shutdown only if game windows are actually available!
        // Otherwise do ungraceful process stopping.

        for (unsigned int iClient = 0; iClient < m_nClients; iClient++)
        {
            if (m_vecHClientMainGameWindow[iClient] != NULL)
            {
                input_sim_test::bringWindowToFront(m_vecHClientMainGameWindow[iClient]);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                input_sim_test::keybdPress(VK_ESCAPE, 100);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            else
            {
                process_stackoverflow_42531::Process::stopProcess(m_vecProcInfoClient[iClient]);
            }
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

        for (unsigned int iClient = 0; iClient < m_nClients; iClient++)
        {
            do
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            } while (process_stackoverflow_42531::Process::checkIfProcessIsActive(m_vecProcInfoClient[iClient]));
        }

        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } while (process_stackoverflow_42531::Process::checkIfProcessIsActive(procInfoServer));
    } // TearDown()

    bool testMethod() override
    {
        // Game instances reposition the mouse cursor into the center of the window in every frame, keep that in mind when specifying relative coords!
        // By default, in testing mode, initially the server points xhair to the right, client points xhair to the left.

        bool bRes = true;
        
        for (unsigned int iTestIt = 0; (iTestIt < m_nTestIterations) && bRes; iTestIt++)
        {

            // server instance initiates map change
            {
                input_sim_test::bringWindowToFront(hServerMainGameWindow);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                if (proofps_dd::GameMode::isTeamBasedGame(m_eGameModeType))
                {
                    // hide team selection menu, otherwise we cannot open server admin menu
                    input_sim_test::keybdPress((unsigned char)VkKeyScan('m'), 100);
                }

                // need a bit sleep after team selection in-game menu closes, otherwise
                // any control inputs will be ignored
                std::this_thread::sleep_for(std::chrono::milliseconds(200));

                // open server admin menu
                input_sim_test::keybdPress((unsigned char)VkKeyScan('h'), 100);

                // press 'n' for Next map
                input_sim_test::keybdPress((unsigned char)VkKeyScan('n'), 100);
            }

            // wait for all instances change the map
            std::this_thread::sleep_for(std::chrono::seconds(m_nSecondsWaitForInstancesToChangeMap));

            {
                for (unsigned int iClient = 0; iClient < m_nClients; iClient++)
                {
                    // client to front
                    input_sim_test::bringWindowToFront(m_vecHClientMainGameWindow[iClient]);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));

                    // trigger client dump test data to file
                    input_sim_test::keybdPress(VK_RETURN, 100);
                }

                // server to front
                input_sim_test::bringWindowToFront(hServerMainGameWindow);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                // trigger dump test data to file
                input_sim_test::keybdPress(VK_RETURN, 100);
            }

            // wait for the test data files to be actually written
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));

            bRes &= assertTrue(evaluateTest(iTestIt == m_nTestIterations-1), (std::string("Failure in iteration ") + std::to_string(iTestIt+1)).c_str());
        }

        return bRes;
    } // testMethod()

private:

    int nTeamTotalFrags1 = 0;
    int nTeamTotalFrags2 = 0;
    std::vector<proofps_dd::PlayersTableRow> evaluateFragTable;

    const unsigned int m_nTickRate;
    const unsigned int m_nClUpdateRate;
    const unsigned int m_nPhysicsRateMin;
    const proofps_dd::GameModeType m_eGameModeType;
    const unsigned int m_nTestIterations;
    const unsigned int m_nSecondsWaitForInstancesToChangeMap;
    const unsigned int m_nClients;
    unsigned int m_nPlayerCounter;
    PROCESS_INFORMATION procInfoServer;
    std::vector<PROCESS_INFORMATION> m_vecProcInfoClient;
    HWND hServerMainGameWindow;
    std::vector<HWND> m_vecHClientMainGameWindow;

    bool evaluateInstance(const unsigned int& iInstanceIndex, const bool& bLastIteration)
    {
        const bool bServer = (iInstanceIndex == 0);
        const std::string sTestDumpFilename = proofps_dd::generateTestDumpFilename(
            bServer, bServer ? static_cast<unsigned long>(procInfoServer.dwProcessId) : static_cast<unsigned long>(m_vecProcInfoClient[iInstanceIndex-1].dwProcessId),
            m_nTickRate, m_nClUpdateRate, m_nPhysicsRateMin);
        std::ifstream f(sTestDumpFilename, std::ifstream::in);
        if (!f.good())
        {
            return assertFalse(true,
                (std::string("failed to open file (simulated key down for DumpRegTest too early?): ") + sTestDumpFilename).c_str()
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
        bool bFoundAssignedPlayer = false; // team
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

            // If this is team-based game, then Team Total Frags first:
            if (proofps_dd::GameMode::isTeamBasedGame(m_eGameModeType))
            {
                f >> nTeamTotalFrags1;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                f >> nTeamTotalFrags2;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
            }
            
            while (!f.eof())
            {
                proofps_dd::PlayersTableRow ftRow;
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
                f >> ftRow.m_iTeamId;

                if (ftRow.m_iTeamId != 0u)
                {
                    bFoundAssignedPlayer = true;
                }

                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                f >> ftRow.m_nFrags;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                f >> ftRow.m_nDeaths;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                f >> ftRow.m_nSuicides;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                f >> ftRow.m_fFiringAcc;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                f >> ftRow.m_nShotsFired;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                evaluateFragTable.push_back(ftRow);
            }
        }

        f.close();

        bRet &= assertTrue(bRet, "an instance had an unexpected name in its frag table!") &
            assertEquals(m_nPlayerCounter, nErasedNameCounter, "Number of erased names vs number of generated names") &
            assertTrue(setOfExpectedPlayerNames.empty(), "set is still not empty!") &
            assertFalse(bFoundAssignedPlayer, "a player had assigned team!");

        // delete dump file only if test is successful, otherwise leave it in the filesystem for further debugging!
        // but last iteration dump files are never deleted because we might want to have a look at them anyway!
        if (bRet && !bLastIteration)
        {
            // delete the evaluated dump file so we definitely fail next iteration if an instance fails to generate new dump, otherwise
            // we may evaluate a same dump file in next iteration that was generated in a previous iteration and might think everything is ok!
            const std::filesystem::path fsPathToDumpFile(sTestDumpFilename);
            std::error_code nErrCode;
            bRet &= assertTrue(std::filesystem::remove(fsPathToDumpFile, nErrCode),
                (std::string("Could not remove dump file (") + fsPathToDumpFile.string().c_str() +
                    "), error code: " + std::to_string(nErrCode.value()) +
                    ", message: " + nErrCode.message()).c_str());
        }

        return bRet;
    } // evaluateInstance()

    bool evaluateTest(const bool& bLastIteration)
    {
        // unlike with RegTestBasicServerClient2Players, in this test evaluate order is irrelevant
        bool bRet = assertTrue(evaluateInstance(0, bLastIteration), "evaluateServer");
        
        for (unsigned int i = 1; i <= m_nClients; i++)
        {
            bRet &= assertTrue(evaluateInstance(i, bLastIteration), (std::string("evaluateClient ") + std::to_string(i)).c_str());
        }

        return bRet;
    }

    void StartGame(const unsigned int& iInstanceIndex) noexcept(false)
    {
        const bool bServer = (iInstanceIndex == 0);
        CConsole::getConsoleInstance().OLnOI("%s(%b) ...", __func__, bServer);

        m_nPlayerCounter++;
        const std::string sPlayerName = "Player" + std::to_string(m_nPlayerCounter);

        // exe will be searched in PRooFPS-dd work dir, which means that the tested executable
        // needs to be manually put there: either the release or debug version can be put there.
        if (bServer)
        {
            procInfoServer = process_stackoverflow_42531::Process::launchProcess(
                "PRooFPS-dd.exe",
                "--gfx_windowed=true --gui_mainmenu=false --net_server=true --sv_map=map_test_good.txt --testing=true --sv_map_team_spawn_groups=false --tickrate=" +
                std::to_string(m_nTickRate) + " --cl_updaterate=" +
                std::to_string(m_nClUpdateRate) + " --physics_rate_min=" +
                std::to_string(m_nPhysicsRateMin) + " --cl_name=" + sPlayerName +
                " --" + proofps_dd::GameMode::szCvarSvGamemode + "=" + std::to_string(static_cast<int>(m_eGameModeType))
            );
        }
        else
        {
            m_vecProcInfoClient.push_back( process_stackoverflow_42531::Process::launchProcess(
                "PRooFPS-dd.exe",
                "--gfx_windowed=true --gui_mainmenu=false --net_server=false --cl_server_ip=127.0.0.1 --testing=true --sv_map_team_spawn_groups=false --tickrate=" +
                std::to_string(m_nTickRate) + " --cl_updaterate=" +
                std::to_string(m_nClUpdateRate) + " --physics_rate_min=" +
                std::to_string(m_nPhysicsRateMin) + " --cl_name=" + sPlayerName +
                " --" + proofps_dd::GameMode::szCvarSvGamemode + "=" + std::to_string(static_cast<int>(m_eGameModeType)))
            );
            m_vecHClientMainGameWindow.push_back(static_cast<HWND>(0)); // we set this later below
        }

        // main game window
        CConsole::getConsoleInstance().OLn("Trying to find main game window ...");
        HWND& hMainGameWindow = bServer ? hServerMainGameWindow : m_vecHClientMainGameWindow[m_vecHClientMainGameWindow.size()-1];
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
        RECT rectGameWindow;  // screen coordinates
        memset(&rectGameWindow, 0, sizeof(rectGameWindow));
        if (TRUE == GetWindowRect(hMainGameWindow, &rectGameWindow))
        {
            if (FALSE == SetWindowPos(
                hMainGameWindow,
                NULL,
                /* traditionally, server goes to left edge, client 1 goes to right edge, subsequent clients go somewhere in between from left to right */
                bServer ? 0 : ((iInstanceIndex > 1) ? 200 + iInstanceIndex * 40 : 900),
                bServer ? rectGameWindow.top /*unchanged*/ : ((iInstanceIndex > 1) ? rectGameWindow.top + iInstanceIndex * 20 : rectGameWindow.top /*unchanged*/),
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
        while ((hTmpGameWindow != 0) && (iWaitCntr < 14))
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