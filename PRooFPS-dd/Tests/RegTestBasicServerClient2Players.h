#pragma once

/*
    ###################################################################################
    RegTestBasicServerClient2Players.h
    Basic Regression test for PRooFPS-dd with 2 players: 1 server, 1 client.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include <cstdio>
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
#include "Player.h"
#include "PlayerHandling.h"
#include "Process.h"
#include "WeaponHandling.h"

class RegTestBasicServerClient2Players :
    public UnitTest
{
public:

    enum class InstanceType
    {
        SERVER,
        CLIENT
    };

    RegTestBasicServerClient2Players(
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
        hServerMainGameWindow(NULL),
        hClientMainGameWindow(NULL),
        cfgWpnPistol(false, false),
        cfgWpnMachinegun(false, false)
    {
        memset(&procInfoServer, 0, sizeof(procInfoServer));
        memset(&procInfoClient, 0, sizeof(procInfoClient));

        // We have expected pkt count tables only for 60 and 20.
        // In the future we may dynamically calculate the current values for ExpectedPktStatsRanges
        // based on the predefined expectedPktStatsServerClUpdateRate60 and the given tick rate.
        if ((m_nTickRate != 60) && (m_nTickRate != 20))
        {
            throw std::runtime_error("Unsupported tick rate: " + std::to_string(m_nTickRate));
        }

        if ((m_nClUpdateRate != 60) && (m_nClUpdateRate != 20))
        {
            throw std::runtime_error("Unsupported client update rate: " + std::to_string(m_nClUpdateRate));
        }
    }

    ~RegTestBasicServerClient2Players()
    {
    }

    RegTestBasicServerClient2Players(const RegTestBasicServerClient2Players&) = delete;
    RegTestBasicServerClient2Players& operator=(const RegTestBasicServerClient2Players&) = delete;
    RegTestBasicServerClient2Players(RegTestBasicServerClient2Players&&) = delete;
    RegTestBasicServerClient2Players& operator=(RegTestBasicServerClient2Players&&) = delete;

protected:

    virtual bool setUp() override
    {
        try
        {
            if ( !(cfgWpnPistol.load((std::string(proofps_dd::GAME_WEAPONS_DIR) + "pistol.txt").c_str())) ||
                !(cfgWpnMachinegun.load((std::string(proofps_dd::GAME_WEAPONS_DIR) + "machinegun.txt").c_str())) )
            {
                throw std::exception("Weapon file load failed!");
            }

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

    virtual void tearDown() override
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

        // server player moves into position
        {
            input_sim_test::bringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            input_sim_test::keybdPressNoRelease(VK_RIGHT);
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                input_sim_test::keybdPress(VK_SPACE, 100); // jump over the hole
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                input_sim_test::keybdPress(VK_SPACE, 100); // 1st crate
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                input_sim_test::keybdPress(VK_SPACE, 100); // 2nd crate
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                input_sim_test::keybdPress(VK_SPACE, 100); // 3rd crate
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            input_sim_test::keybdRelease(VK_RIGHT);
        }

        // client player moves into position and shoots
        {
            input_sim_test::bringWindowToFront(hClientMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            input_sim_test::keybdPressNoRelease(VK_LEFT);
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                input_sim_test::keybdPress(VK_SPACE, 100); // jump over the hole
                std::this_thread::sleep_for(std::chrono::milliseconds(3500));
            }
            input_sim_test::keybdRelease(VK_LEFT);
            // wait a bit so keybdRelease and mouseScroll go out in separate pkts
            std::this_thread::sleep_for(std::chrono::milliseconds(300));

            // now server and client player should be just a few blocks from each other

            // client should be able to switch to machinegun using scrolling forward/up
            input_sim_test::mouseScroll(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            // shoot 2 bullets
            input_sim_test::mouseClick(cfgWpnMachinegun.getVars()["firing_cooldown"].getAsInt() * 2);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        // server player also shoots
        {
            input_sim_test::bringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // shoot 1 bullet
            // intentionally holding down mouse button for longer than cooldown, just for testing purpose, but pistol should shoot only 1 bullet!
            input_sim_test::mouseClick(cfgWpnPistol.getVars()["firing_cooldown"].getAsInt() * 2);

            // shoot 1 bullet again
            // intentionally holding down mouse button for longer than cooldown, just for testing purpose, but pistol should shoot only 1 bullet!
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            input_sim_test::mouseClick(cfgWpnPistol.getVars()["firing_cooldown"].getAsInt() * 2);
        }

        // client player shoots again and kills server player
        {
            input_sim_test::bringWindowToFront(hClientMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // shoot 4 bullets
            input_sim_test::mouseClick(cfgWpnMachinegun.getVars()["firing_cooldown"].getAsInt() * 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        // wait for the killed server player to respawn
        std::this_thread::sleep_for(std::chrono::milliseconds(m_nSvDmPlayerRespawnDelaySecs * 1000 + 200));

        {
            // it is recommended to be a bit more patient here, since after respawn the camera will reposition gradually, which
            // causes the weapon angle to change for 1-2 seconds, generating traffic. If traffic is not settling down before
            // dumping data, there will be mismatch between client and server sent/received pkt count which is misleading.
            // It is also worth noting that window switching will further trigger update to weapon angle, since weapon angle is
            // updated only when window is active. Thus after switching to window of another game instance, further patience
            // is needed to let weapon angle and traffic settle down.
            
            // client settle down
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));

            // allow server to update its angle
            input_sim_test::bringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));

            // switch back to client again who hopefully won't do any further alignment after switch
            input_sim_test::bringWindowToFront(hClientMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // switch back to server again who hopefully won't do any further alignment after switch
            input_sim_test::bringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // switch back to client again who hopefully won't do any further alignment after switch
            input_sim_test::bringWindowToFront(hClientMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        {
            // trigger dump test data to file

            // client
            input_sim_test::keybdPress(VK_RETURN, 100);

            // server
            input_sim_test::bringWindowToFront(hServerMainGameWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            input_sim_test::keybdPress(VK_RETURN, 100);
        }

        // wait for the test data files to be actually written
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        return evaluateTest();
    }

private:

    struct EvaluatePktStats
    {
        uint32_t nTxPktTotalCount;
        uint32_t nTxPktPerSecond;
        uint32_t nRxPktTotalCount;
        uint32_t nRxPktPerSecond;
        uint32_t nInjectPktTotalCount;
        uint32_t nInjectPktPerSecond;
    } evaluatePktStatsServer, evaluatePktStatsClient;

    struct PktStatRange
    {
        uint32_t nMin;
        uint32_t nMax;
    };

    struct ExpectedPktStatsRanges
    {
        PktStatRange nTxPktTotalCount;
        PktStatRange nTxPktPerSecond;
        PktStatRange nRxPktTotalCount;
        PktStatRange nRxPktPerSecond;
        PktStatRange nInjectPktTotalCount;
        PktStatRange nInjectPktPerSecond;
    };

    std::vector<proofps_dd::PlayersTableRow> evaluateFragTable;

    struct EvaluateWpn
    {
        std::string sName;
        TPureUInt nMagBulletCount;
        TPureUInt nUnmagBulletCount;
    };
    std::vector<EvaluateWpn> evaluateWpnData;

    const unsigned int m_nSvDmPlayerRespawnDelaySecs = 3;

    int nPlayerHealth;
    int nPlayerArmor;

    const unsigned int m_nTickRate;
    const unsigned int m_nClUpdateRate;
    const unsigned int m_nPhysicsRateMin;
    PROCESS_INFORMATION procInfoServer;
    PROCESS_INFORMATION procInfoClient;
    HWND hServerMainGameWindow;
    HWND hClientMainGameWindow;

    // These should be Weapon instances, however we would need proper gfx engine instance as well, or a stubbed gfx engine instance,
    // for now I dont want any of these, so I just load them as config files, so I can still use their config values as reference!
    PGEcfgFile cfgWpnPistol, cfgWpnMachinegun;

    bool evaluateInstance(const InstanceType& instType)
    {
        const bool bServer = instType == InstanceType::SERVER;
        const std::string sTestDumpFilename = proofps_dd::generateTestDumpFilename(
            bServer, bServer ? static_cast<unsigned long>(procInfoServer.dwProcessId) : static_cast<unsigned long>(procInfoClient.dwProcessId),
            m_nTickRate, m_nClUpdateRate, m_nPhysicsRateMin);
        std::ifstream f(sTestDumpFilename, std::ifstream::in);
        if (!f.good())
        {
            return assertFalse(true,
                (std::string("failed to open file: ") + sTestDumpFilename).c_str()
            );
        }

        EvaluatePktStats& currentPktStats = bServer ? evaluatePktStatsServer : evaluatePktStatsClient;
        memset(&currentPktStats, 0, sizeof(currentPktStats));
        evaluateFragTable.clear();
        evaluateWpnData.clear();
        nPlayerHealth = 0;
        nPlayerArmor = 0;

        const std::streamsize nBuffSize = 1024;
        char szLine[nBuffSize];

        // read pkt stats
        {
            f.getline(szLine, nBuffSize);  // Tx: Total Pkt Count, Pkt/Second
            f >> currentPktStats.nTxPktTotalCount;
            f >> currentPktStats.nTxPktPerSecond;
            f.getline(szLine, nBuffSize);  // consume remaining newline char in same line

            f.getline(szLine, nBuffSize);  // Rx: Total Pkt Count, Pkt/Second
            f >> currentPktStats.nRxPktTotalCount;
            f >> currentPktStats.nRxPktPerSecond;
            f.getline(szLine, nBuffSize);  // consume remaining newline char in same line

            f.getline(szLine, nBuffSize);  // Inject: Total Pkt Count, Pkt/Second
            f >> currentPktStats.nInjectPktTotalCount;
            f >> currentPktStats.nInjectPktPerSecond;
            f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
        }
        f.getline(szLine, nBuffSize);  // consume next blank line
        
        // read Byte Count stats
        {
            uint32_t uTmp;
            f.getline(szLine, nBuffSize);  // Tx: Total Byte Count
            f >> uTmp;
            f.getline(szLine, nBuffSize);  // consume remaining newline char in same line

            f.getline(szLine, nBuffSize);  // Rx: Total Byte Count
            f >> uTmp;
            f.getline(szLine, nBuffSize);  // consume remaining newline char in same line

            f.getline(szLine, nBuffSize);  // Inject: Total Byte Count
            f >> uTmp;
            f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
        }
        f.getline(szLine, nBuffSize);  // consume next blank line

        // read msg stats
        {
            f.getline(szLine, nBuffSize);  // Total Tx'd App Msg Count per AppMsgId:
            while (!f.eof())
            {
                // Id <msgAppId> <msgAppName> : <count>
                std::string sTmp;
                uint32_t uTmp;
                f >> sTmp; // "Id"
                if (sTmp != "Id")
                {
                    break;
                }
                f >> uTmp; // <msgAppId>
                f >> sTmp; // <msgAppName>
                f >> sTmp; // ":"
                f >> uTmp; // <count>
                f.getline(szLine, nBuffSize); // consume remaining newline char in same line
            }

            f.getline(szLine, nBuffSize);  // Total Rx'd App Msg Count per AppMsgId:
            while (!f.eof())
            {
                // Id <msgAppId> <msgAppName> : <count>
                std::string sTmp;
                uint32_t uTmp;
                f >> sTmp; // "Id"
                if (sTmp != "Id")
                {
                    break;
                }
                f >> uTmp; // <msgAppId>
                f >> sTmp; // <msgAppName>
                f >> sTmp; // ":"
                f >> uTmp; // <count>
                f.getline(szLine, nBuffSize); // consume remaining newline char in same line
            }

            f.getline(szLine, nBuffSize);  // Total Inj'd App Msg Count per AppMsgId:
            while (!f.eof())
            {
                // Id <msgAppId> <msgAppName> : <count>
                std::string sTmp;
                uint32_t uTmp;
                f >> sTmp; // "Id"
                if (sTmp != "Id")
                {
                    break;
                }
                f >> uTmp; // <msgAppId>
                f >> sTmp; // <msgAppName>
                f >> sTmp; // ":"
                f >> uTmp; // <count>
                f.getline(szLine, nBuffSize); // consume remaining newline char in same line
            }
        }

        bool bRet = (assertFalse(f.bad(), "f.bad()") & assertFalse(f.eof(), "f.eof()")) != 0;
        if (!bRet)
        {
            return bRet;
        }

        // read frag table
        {
            // note that at this point, most probably "Frag" word is already eaten by a previous while loop but we are still in good line
            f.getline(szLine, nBuffSize);  // [Frag ]Table: Player Name, Frags, Deaths
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

        // read weapon list
        {
            f.getline(szLine, nBuffSize);  // Weapons Available: Weapon Filename, Mag Bullet Count, Unmag Bullet Count
            while (!f.eof())
            {
                EvaluateWpn wpnData;
                f >> wpnData.sName;
                if (wpnData.sName.empty() || (wpnData.sName.find("Player") != std::string::npos))
                {
                    // actually if this is empty, it might be expected scenario because we have an extra empty line after
                    // the weapon list in the file, anyway just stop here, the evaluate functions will verify the read weapon data anyway!
                    break;
                }

                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                f >> wpnData.nMagBulletCount;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                f >> wpnData.nUnmagBulletCount;
                f.getline(szLine, nBuffSize);  // consume remaining newline char in same line
                evaluateWpnData.push_back(wpnData);
            }
        }

        // read player info
        {
            f.getline(szLine, nBuffSize);  // Player Info: Health, Armor
            f >> nPlayerHealth;
            f >> nPlayerArmor;
        }

        static constexpr ExpectedPktStatsRanges expectedPktStatsServerClUpdateRate60
        {
            /* I should enable Cpp20 for designated initializers so I don't need to use comments below */
            /*.nTxPktTotalCount =*/     {580u, 1100u},
            /*.nTxPktPerSecond =*/      { 20u,   39u},
            /*.nRxPktTotalCount =*/     { 21u,   40u},
            /*.nRxPktPerSecond =*/      {  0u,    3u},
            /*.nInjectPktTotalCount =*/ {450u, 1000u},
            /*.nInjectPktPerSecond =*/  { 15u,   33u}
        };

        static constexpr ExpectedPktStatsRanges expectedPktStatsServerClUpdateRate20
        {
            /* server tx rate should be only 1/3 in the 20 Hz case compared to 60 Hz case */
            {expectedPktStatsServerClUpdateRate60.nTxPktTotalCount.nMin / 3,  expectedPktStatsServerClUpdateRate60.nTxPktTotalCount.nMax / 3},
            {expectedPktStatsServerClUpdateRate60.nTxPktPerSecond.nMin / 3, expectedPktStatsServerClUpdateRate60.nTxPktPerSecond.nMax / 3},
            /* server rx rate should be same in 20 Hz case as in 60 Hz case because clients are still sending updates with refresh rate */
            {expectedPktStatsServerClUpdateRate60.nRxPktTotalCount.nMin,  expectedPktStatsServerClUpdateRate60.nRxPktTotalCount.nMax},
            {expectedPktStatsServerClUpdateRate60.nRxPktPerSecond.nMin, expectedPktStatsServerClUpdateRate60.nRxPktPerSecond.nMax},
            /* server inject rate should be somewhat less in 20 Hz case than in 60 Hz case because that input handling code is shared with client code,
               which means that server generates same amount of messages for its player as a client generates for their player, however injected pkt
               count in total is less because it also contains the player updates sent out to itself (inject) by tickrate */
            {expectedPktStatsServerClUpdateRate60.nInjectPktTotalCount.nMin / 2, expectedPktStatsServerClUpdateRate60.nInjectPktTotalCount.nMax / 2},
            {expectedPktStatsServerClUpdateRate60.nInjectPktPerSecond.nMin / 2, expectedPktStatsServerClUpdateRate60.nInjectPktPerSecond.nMax / 2}
        };

        static constexpr ExpectedPktStatsRanges expectedPktStatsClientClUpdateRate60
        {
            /* I should enable Cpp20 for designated initializers so I don't need to use comments below */
            /*.nTxPktTotalCount =*/     {  0u,    0u}, // 0 because we expect SAME as server RX
            /*.nTxPktPerSecond =*/      { expectedPktStatsServerClUpdateRate60.nRxPktPerSecond.nMin, expectedPktStatsServerClUpdateRate60.nRxPktPerSecond.nMax},
            /*.nRxPktTotalCount =*/     {  0u,    0u}, // 0 because we expect SAME as server TX
            /*.nRxPktPerSecond =*/      { expectedPktStatsServerClUpdateRate60.nTxPktPerSecond.nMin, expectedPktStatsServerClUpdateRate60.nTxPktPerSecond.nMax},
            /*.nInjectPktTotalCount =*/ {  0u,    0u}, // 0 because client never injects
            /*.nInjectPktPerSecond =*/  {  0u,    0u}  // 0 because client never injects
        };

        static constexpr ExpectedPktStatsRanges expectedPktStatsClientClUpdateRate20
        {
            /* I should enable Cpp20 for designated initializers so I don't need to use comments below */
            /*.nTxPktTotalCount =*/     { expectedPktStatsClientClUpdateRate60.nTxPktTotalCount.nMin,     expectedPktStatsClientClUpdateRate60.nTxPktTotalCount.nMax},
            /*.nTxPktPerSecond =*/      { expectedPktStatsClientClUpdateRate60.nTxPktPerSecond.nMin,      expectedPktStatsClientClUpdateRate60.nTxPktPerSecond.nMax},
            /*.nRxPktTotalCount =*/     { expectedPktStatsClientClUpdateRate60.nRxPktTotalCount.nMin,     expectedPktStatsClientClUpdateRate60.nRxPktTotalCount.nMax},
            /*.nRxPktPerSecond =*/      { expectedPktStatsClientClUpdateRate60.nRxPktPerSecond.nMin / 3,  expectedPktStatsClientClUpdateRate60.nRxPktPerSecond.nMax / 3},
            /*.nInjectPktTotalCount =*/ { expectedPktStatsClientClUpdateRate60.nInjectPktTotalCount.nMin, expectedPktStatsClientClUpdateRate60.nInjectPktTotalCount.nMax},
            /*.nInjectPktPerSecond =*/  { expectedPktStatsClientClUpdateRate60.nInjectPktPerSecond.nMin,  expectedPktStatsClientClUpdateRate60.nInjectPktPerSecond.nMax}
        };

        bRet = bServer ?
            evaluateServer(f, (m_nClUpdateRate == 60 ? expectedPktStatsServerClUpdateRate60 : expectedPktStatsServerClUpdateRate20)) :
            evaluateClient(f, (m_nClUpdateRate == 60 ? expectedPktStatsClientClUpdateRate60 : expectedPktStatsClientClUpdateRate20));
        
        f.close();
        return bRet;
    }

    bool evaluateFragTableCommon()
    {
        // frag table must contain same data for both server and client, that is why we have common function for both
        bool bRet = assertEquals(2u, evaluateFragTable.size(), "fragtable size");
        if (!bRet)
        {
            return bRet;
        }

        // client is Player 2
        bRet &= assertEquals("Player2", evaluateFragTable[0].m_sName, "fragtable row 1 name") &
            assertEquals(1, evaluateFragTable[0].m_nFrags, "fragtable row 1 frags") &
            assertEquals(0, evaluateFragTable[0].m_nDeaths, "fragtable row 1 deaths") &
            assertEquals(0u, evaluateFragTable[0].m_nSuicides, "fragtable row 1 suicides") &
            assertEquals(83.f, evaluateFragTable[0].m_fFiringAcc, "fragtable row 1 firing acc") &  /* last shot misses because Player 1 is already dead */
            assertEquals(6u, evaluateFragTable[0].m_nShotsFired, "fragtable row 1 shots fired");

        // server is Player 1
        bRet &= assertEquals("Player1", evaluateFragTable[1].m_sName, "fragtable row 2 name") &
            assertEquals(0, evaluateFragTable[1].m_nFrags, "fragtable row 2 frags") &
            assertEquals(1, evaluateFragTable[1].m_nDeaths, "fragtable row 2 deaths") &
            assertEquals(0u, evaluateFragTable[1].m_nSuicides, "fragtable row 2 suicides") &
            assertEquals(100.f, evaluateFragTable[1].m_fFiringAcc, "fragtable row 2 firing acc") &
            assertEquals(2u, evaluateFragTable[1].m_nShotsFired, "fragtable row 2 shots fired");

        return bRet;
    }

    bool evaluateServer(std::ifstream&, const ExpectedPktStatsRanges& expectedPktStatsRanges)
    {
        bool bRet = (
          assertBetween(
              expectedPktStatsRanges.nTxPktTotalCount.nMin,
              expectedPktStatsRanges.nTxPktTotalCount.nMax,
              evaluatePktStatsServer.nTxPktTotalCount, "server nTxPktTotalCount") &
          assertBetween(
              expectedPktStatsRanges.nTxPktPerSecond.nMin,
              expectedPktStatsRanges.nTxPktPerSecond.nMax,
              evaluatePktStatsServer.nTxPktPerSecond, "server nTxPktPerSecond") &
          assertBetween(
              expectedPktStatsRanges.nRxPktTotalCount.nMin,
              expectedPktStatsRanges.nRxPktTotalCount.nMax,
              evaluatePktStatsServer.nRxPktTotalCount, "server nRxPktTotalCount") &
          assertBetween(
              expectedPktStatsRanges.nRxPktPerSecond.nMin,
              expectedPktStatsRanges.nRxPktPerSecond.nMax,
              evaluatePktStatsServer.nRxPktPerSecond, "server nRxPktPerSecond") &
          assertBetween(
              expectedPktStatsRanges.nInjectPktTotalCount.nMin,
              expectedPktStatsRanges.nInjectPktTotalCount.nMax,
              evaluatePktStatsServer.nInjectPktTotalCount, "server nInjectPktTotalCount") &
          assertBetween(
              expectedPktStatsRanges.nInjectPktPerSecond.nMin,
              expectedPktStatsRanges.nInjectPktPerSecond.nMax,
              evaluatePktStatsServer.nInjectPktPerSecond, "server nInjectPktPerSecond") ) != 0;
        
        bRet &= evaluateFragTableCommon();

        bRet &= assertEquals(2u, evaluateWpnData.size(), "server wpn count");
        if (!bRet)
        {
            return bRet;
        }

        bRet &= assertEquals("knife.txt", evaluateWpnData[0].sName, "server wpn 0 name");

        bRet &= assertEquals(proofps_dd::GAME_WPN_DEFAULT, evaluateWpnData[1].sName, "server wpn 1 name") &
            assertEquals(static_cast<TPureUInt>(cfgWpnPistol.getVars()["bullets_default"].getAsInt()), evaluateWpnData[1].nMagBulletCount, "server wpn 1 mag bullet count") &
            assertEquals(0u, evaluateWpnData[1].nUnmagBulletCount, "server wpn 1 unmag bullet count");

        bRet &= assertEquals(100, nPlayerHealth, "server player health");
        bRet &= assertEquals(0, nPlayerArmor, "server player armor");  // no armor pickup happened

        return bRet;
    }

    bool evaluateClient(std::ifstream&, const ExpectedPktStatsRanges& expectedPktStatsRanges)
    {
        // when this function is called, server is already evaluated and evaluatePktStatsServer contains valid server data, so
        // we can also compare client data to server data!
        bool bRet = (
            assertEquals(evaluatePktStatsServer.nRxPktTotalCount, evaluatePktStatsClient.nTxPktTotalCount, "client nTxPktTotalCount") &
            assertBetween(
                expectedPktStatsRanges.nTxPktPerSecond.nMin,
                expectedPktStatsRanges.nTxPktPerSecond.nMax,
                evaluatePktStatsClient.nTxPktPerSecond, "client nTxPktPerSecond") &
            assertEquals(evaluatePktStatsServer.nTxPktTotalCount, evaluatePktStatsClient.nRxPktTotalCount, "client nRxPktTotalCount") &
            assertBetween(
                expectedPktStatsRanges.nRxPktPerSecond.nMin,
                expectedPktStatsRanges.nRxPktPerSecond.nMax,
                evaluatePktStatsClient.nRxPktPerSecond, "client nRxPktPerSecond") &
            /* client never injects packets to its pkt queue */
            assertEquals(evaluatePktStatsClient.nInjectPktTotalCount, 0u, "client nInjectPktTotalCount") &
            assertEquals(evaluatePktStatsClient.nInjectPktPerSecond, 0u, "client nInjectPktPerSecond") ) != 0;

        if (evaluatePktStatsServer.nRxPktTotalCount < evaluatePktStatsClient.nTxPktTotalCount)
        {   // this check is for logging more clear error, if we come here the above bRet check already failed
            assertFalse(true, "client -> server pkt loss!");
        }
        else if (evaluatePktStatsServer.nRxPktTotalCount > evaluatePktStatsClient.nTxPktTotalCount)
        {   // this check is for logging more clear error, if we come here the above bRet check already failed
            assertFalse(true, "cannot happen: server received more pkts than client sent!");
        }

        if (evaluatePktStatsServer.nTxPktTotalCount > evaluatePktStatsClient.nRxPktTotalCount)
        {   // this check is for logging more clear error, if we come here the above bRet check already failed
            assertFalse(true, "server -> client pkt loss!");
        }
        else if (evaluatePktStatsServer.nTxPktTotalCount < evaluatePktStatsClient.nRxPktTotalCount)
        {   // this check is for logging more clear error, if we come here the above bRet check already failed
            assertFalse(true, "cannot happen: client received more pkts than server sent!");
        }
        
        bRet &= evaluateFragTableCommon();

        bRet &= assertEquals(4u, evaluateWpnData.size(), "client wpn count");
        if (!bRet)
        {
            return bRet;
        }

        // TODO: the order of weapons in player's weapons vector looks to be alphabetic, and it is because
        // when the weapons are loaded, the files in weapons directory are being iterated over, and that order
        // looks to be alphabetic. However, the reference of std::filesystem::directory_iterator says that
        // the "iteration order is unspecified" which means that we are relying now on unspecified behavior if
        // we expect our vector's 1st element to be machinegun and 2nd element to be pistol.
        // On the long run I think we should fix this.

        bRet &= assertEquals("knife.txt", evaluateWpnData[0].sName, "server wpn 0 name");

        bRet &= /* client shot 2+4 = 6 bullets with machinegun */
            assertEquals(cfgWpnMachinegun.getFilename(), evaluateWpnData[1].sName, "client wpn 1 name") &
            assertEquals(static_cast<TPureUInt>(cfgWpnMachinegun.getVars()["bullets_default"].getAsInt() - 6), evaluateWpnData[1].nMagBulletCount, "client wpn 1 mag bullet count") &
            assertEquals(0u, evaluateWpnData[1].nUnmagBulletCount, "client wpn 1 unmag bullet count") &
            /* client picked up extra pistol ammo during walking towards server player */
            assertEquals(cfgWpnPistol.getFilename(), evaluateWpnData[2].sName, "client wpn 2 name") &
            assertEquals(static_cast<TPureUInt>(cfgWpnPistol.getVars()["bullets_default"].getAsInt()), evaluateWpnData[2].nMagBulletCount, "client wpn 2 mag bullet count") &
            assertEquals(static_cast<TPureUInt>(cfgWpnPistol.getVars()["reloadable"].getAsInt()), evaluateWpnData[2].nUnmagBulletCount, "client wpn 2 unmag bullet count");

        // after being shot twice by pistol
        bRet &= assertEquals(20, nPlayerHealth, "client player health");
        bRet &= assertEquals(0, nPlayerArmor, "client player armor");  // no armor pickup happened

        return bRet;
    }

    bool evaluateTest()
    {
        // order is important: evaluate server first, and then the client!
        bool bRet = assertTrue(evaluateInstance(InstanceType::SERVER), "evaluateServer");
        bRet &= assertTrue(evaluateInstance(InstanceType::CLIENT), "evaluateClient");

        return bRet;
    }

    void StartGame(const InstanceType& instType) noexcept(false)
    {
        const bool bServer = instType == InstanceType::SERVER;
        CConsole::getConsoleInstance().OLnOI("%s(%b) ...", __func__, bServer);

        // exe will be searched in PR00FPS-dd work dir, which means that the tested executable
        // needs to be manually put there: either the release or debug version can be put there.
        if (bServer)
        {
            procInfoServer = process_stackoverflow_42531::Process::launchProcess(
                "PRooFPS-dd.exe",
                "--gfx_windowed=true --gui_mainmenu=false --net_server=true --sv_map=map_test_good.txt --testing=true --tickrate=" +
                std::to_string(m_nTickRate) + " --cl_updaterate=" +
                std::to_string(m_nClUpdateRate) + " --physics_rate_min=" +
                std::to_string(m_nPhysicsRateMin) + " --cl_name=Player1" +
                " --" + proofps_dd::Player::szCVarSvDmRespawnDelaySecs + "=" + std::to_string(m_nSvDmPlayerRespawnDelaySecs) + 
                " --" + proofps_dd::szCvarClWpnAutoSwitchWhenPickedUpNewWeapon + "=" + proofps_dd::szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchIfEmpty + 
                " --" + proofps_dd::szCvarClWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag + "=false");
        }
        else
        {
            // we dont need to configure szCVarSvDmRespawnDelaySecs for clients, since they will receive it from server
            procInfoClient = process_stackoverflow_42531::Process::launchProcess(
                "PRooFPS-dd.exe",
                "--gfx_windowed=true --gui_mainmenu=false --net_server=false --cl_server_ip=127.0.0.1 --testing=true --tickrate=" +
                std::to_string(m_nTickRate) + " --cl_updaterate=" +
                std::to_string(m_nClUpdateRate) + " --physics_rate_min=" +
                std::to_string(m_nPhysicsRateMin) + " --cl_name=Player2" +
                " --" + proofps_dd::Player::szCVarSvDmRespawnDelaySecs + "=" + std::to_string(m_nSvDmPlayerRespawnDelaySecs) +
                " --" + proofps_dd::szCvarClWpnAutoSwitchWhenPickedUpNewWeapon + "=" + proofps_dd::szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchIfEmpty +
                " --" + proofps_dd::szCvarClWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag + "=false");
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