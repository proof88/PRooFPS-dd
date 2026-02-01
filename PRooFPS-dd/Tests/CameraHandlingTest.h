#pragma once

/*
    ###################################################################################
    CameraHandlingTest.h
    Unit test for PRooFPS-dd CameraHandling.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2026
    ###################################################################################
*/

#include <list>
#include <memory>

#include "UnitTest.h"

#include "PGE.h" // for Audio, PURE and PgeCfgProfiles
#include "Network/Stubs/PgeNetworkStub.h"

#include "CameraHandling.h"
#include "Durations.h"
#include "GameMode.h"
#include "Maps.h"
#include "Player.h"

class CameraHandlingTest :
    public UnitTest
{
public:

    CameraHandlingTest(PGEcfgProfiles& cfgProfiles) :
        UnitTest(__FILE__),
        m_audio(cfgProfiles),
        m_gm(nullptr),
        m_cfgProfiles(cfgProfiles),
        m_engine(nullptr),
        m_network(cfgProfiles),
        m_maps(nullptr),
        m_camera(nullptr),
        m_itemPickupEvents(8 /* time limit secs */, 5 /* event count limit */),
        m_inventoryChangeEvents(8 /* time limit secs */, 5 /* event count limit */),
        m_ammoChangeEvents(8 /* time limit secs */, 5 /* event count limit */, proofps_dd::Orientation::Horizontal)
    {
    }

    CameraHandlingTest(const CameraHandlingTest&) = delete;
    CameraHandlingTest& operator=(const CameraHandlingTest&) = delete;
    CameraHandlingTest(CameraHandlingTest&&) = delete;
    CameraHandlingTest& operator=(CameraHandlingTest&&) = delete;

protected:

    static constexpr unsigned int nPlayersExpected = 4;

    virtual void initialize() override
    {
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::CameraHandling::getLoggerModuleName(), true);

        m_audio.initialize();

        PGEInputHandler& inputHandler = PGEInputHandler::createAndGet(m_cfgProfiles);

        m_engine = &PR00FsUltimateRenderingEngine::createAndGet(m_cfgProfiles, inputHandler);
        assert(m_engine);

        m_gm = proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType::DeathMatch);

        m_maps = new proofps_dd::Maps(m_audio, m_cfgProfiles, *m_engine);
        // no need to init Maps for this test

        addSubTest("test_ctor", static_cast<PFNUNITSUBTEST>(&CameraHandlingTest::test_ctor));
        addSubTest("test_cameraToggleSpectatingView", static_cast<PFNUNITSUBTEST>(&CameraHandlingTest::test_cameraToggleSpectatingView));
        addSubTest("test_findNextValidPlayerToFollowInPlayerSpectatingView",
            static_cast<PFNUNITSUBTEST>(&CameraHandlingTest::test_findNextValidPlayerToFollowInPlayerSpectatingView));
        addSubTest("test_findPrevValidPlayerToFollowInPlayerSpectatingView",
            static_cast<PFNUNITSUBTEST>(&CameraHandlingTest::test_findPrevValidPlayerToFollowInPlayerSpectatingView));
        addSubTest("test_findAnyValidPlayerToFollowInPlayerSpectatingView",
            static_cast<PFNUNITSUBTEST>(&CameraHandlingTest::test_findAnyValidPlayerToFollowInPlayerSpectatingView));
    }

    virtual bool setUp() override
    {
        // check initialize()
        bool b = assertNotNull(m_engine, "engine created");
        b &= assertTrue(m_audio.isInitialized(), "audio inited");
        b &= assertNotNull(m_gm, "gamemode created");
        b &= assertNotNull(m_maps, "maps created");

        b &= assertTrue(m_network.initialize(), "network inited");

        // check previous tearDown()
        if (b)
        {
            b &= assertNull(m_camera, "CameraHandling under test created 1");
            m_camera = new proofps_dd::CameraHandling(*m_engine, m_durations, *m_maps);
            b &= assertNotNull(m_camera, "CameraHandling under test created 2");

            // due to an assertion in CameraHandling's ctor, engine must be initialized AFTER CameraHandling's ctor is invoked,
            // that assertion is due to game main class design and shall stay there
            b &= assertFalse(m_engine->isInitialized(), "engine inited 1");
            b &= assertEquals(0u, m_engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0), "engine inited 2");
        }

        b &= assertTrue(m_mapPlayers.empty(), "no players 1");
        if (b)
        {
            b &= assertTrue(m_gm->getPlayersTable().empty(), "no players 2");
        }

        return b;
    }

    virtual void tearDown() override
    {
        if (m_gm)
        {
            m_gm->restart(m_network); // removes players
        }
        m_mapPlayers.clear();

        if (m_camera)
        {
            delete m_camera;
            m_camera = nullptr;
        }

        if (m_engine)
        {
            m_engine->shutdown();
        }

        m_network.shutdown();
    }

    virtual void finalize() override
    {
        if (m_maps)
        {
            delete m_maps;
            m_maps = nullptr;
        }
        if (m_gm)
        {
            m_gm = nullptr; // unique ptr will deallocate within GameMode dtor
        }
        if (m_engine)
        {
            m_engine->shutdown();
            m_engine = nullptr;
        }

        m_audio.shutdown();

        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::CameraHandling::getLoggerModuleName(), false);
    }

private:

    pge_audio::PgeAudio m_audio;
    proofps_dd::GameMode* m_gm;
    PGEcfgProfiles& m_cfgProfiles;
    PR00FsUltimateRenderingEngine* m_engine;
    pge_network::PgeNetworkStub m_network;
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player> m_mapPlayers;
    proofps_dd::Durations m_durations;
    proofps_dd::Maps* m_maps;
    proofps_dd::CameraHandling* m_camera;
    PgeObjectPool<PooledBullet> m_bullets;
    proofps_dd::EventLister<> m_itemPickupEvents;
    proofps_dd::EventLister<> m_inventoryChangeEvents;
    proofps_dd::EventLister<> m_ammoChangeEvents;

    // ---------------------------------------------------------------------------

    /**
    * Preconditions: both m_mapPlayers and m_gm must be empty.
    * Postconditions: upon success, current test case can index players from 0 to nPlayersExpected-1 using m_mapPlayers.at(), and
    *                 order of players in m_mapPlayers is same as in m_gm.
    * 
    * @return True if both m_mapPlayers and m_gm now contain the same nPlayersExpected recently added players, false otherwise.
    */
    bool test_addPlayers()
    {
        bool b = true;
        b &= assertTrue(m_mapPlayers.empty(), "m_mapPlayers init empty");
        b &= assertNotNull(m_gm, "gamemode not null");
        if (b)
        {
            b &= assertTrue(m_gm->getPlayersTable().empty(), "gamemode init empty");
        }

        for (unsigned int i = 0; b && (i < nPlayersExpected); i++)
        {
            const pge_network::PgeNetworkConnectionHandle connHandleServerSide = i;
            const auto pairInsertRes = m_mapPlayers.insert(
                {
                connHandleServerSide,
                proofps_dd::Player(
                    m_audio, m_cfgProfiles, m_bullets, m_itemPickupEvents, m_inventoryChangeEvents, m_ammoChangeEvents, *m_engine, m_network,
                    connHandleServerSide, "192.168.1.1" /* dont care about IP in this test */)
                }
            );  // TODO: emplace_back()

            b &= assertTrue(pairInsertRes.second, "m_mapPlayers insert ok");
            if (!b)
            {
                return false;
            }
            
            pairInsertRes.first->second.setName(
                std::string("Player ") + std::to_string(i+1));  // otherwise GameMode will complain
            b &= assertTrue(m_gm->addPlayer(pairInsertRes.first->second, m_network), "gamemode insert ok");
        }

        b &= assertEquals(nPlayersExpected, m_mapPlayers.size(), "m_mapPlayers size");
        b &= assertEquals(nPlayersExpected, m_gm->getPlayersTable().size(), "gamemode size");

        if (b)
        {
            auto itGm = m_gm->getPlayersTable().begin();
            for (unsigned int i = 0; b && (i < nPlayersExpected); i++)
            {
                b &= assertEquals(i, m_mapPlayers.at(i).getServerSideConnectionHandle(), "player order connhandle sanity check in m_mapPlayers");
                b &= assertEquals(i, itGm->m_connHandle, "player order connhandle sanity check in gamemode");
                ++itGm;
            }
        }

        return b;
    }

    bool test_ctor()
    {
        assert(m_camera);
        proofps_dd::CameraHandling& camera = *m_camera;
        bool b = true;

        b &= assertEquals(
            static_cast<int>(proofps_dd::CameraHandling::SpectatingView::Free),
            static_cast<int>(camera.cameraGetSpectatingView()),
            "spectating view");
        // b &= assertEquals(PureVector(), camera.cameraGetPosToFollowInSpectatorMode(), "pos to follow");  // not public function
        b &= assertEquals(pge_network::ServerConnHandle, camera.cameraGetPlayerConnectionHandleToFollowInSpectatingView(), "conn handle");

        return b;
    }

    bool test_cameraToggleSpectatingView()
    {
        assert(m_camera);
        proofps_dd::CameraHandling& camera = *m_camera;
        bool b = true;

        camera.cameraToggleSpectatingView();
        b &= assertEquals(
            static_cast<int>(proofps_dd::CameraHandling::SpectatingView::PlayerFollow),
            static_cast<int>(camera.cameraGetSpectatingView()),
            "spectating view 1");
        
        camera.cameraToggleSpectatingView();
        b &= assertEquals(
            static_cast<int>(proofps_dd::CameraHandling::SpectatingView::Free),
            static_cast<int>(camera.cameraGetSpectatingView()),
            "spectating view 2");

        return b;
    }

    bool test_findNextValidPlayerToFollowInPlayerSpectatingView()
    {
        assert(m_camera);
        proofps_dd::CameraHandling& camera = *m_camera;
        bool b = true;

        b &= assertTrue(test_addPlayers(), "add players");
        if (!b)
        {
            // test_addPlayers() ensures map.at(key) does not fail when key is in [0..nPlayersExpected-1] range, and
            // order of players is same in both containers: map and gamemode.
            return false;
        }

        proofps_dd::Player& player0 = m_mapPlayers.at(0);
        proofps_dd::Player& player1 = m_mapPlayers.at(1);
        proofps_dd::Player& player2 = m_mapPlayers.at(2);
        proofps_dd::Player& player3 = m_mapPlayers.at(3);
        
        // initially everyone is in spectator mode
        b &= assertFalse(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 1");

        // first players joins the game
        player0.isInSpectatorMode() = false;
        b &= assertTrue(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 2");
        b &= assertEquals(player0.getServerSideConnectionHandle(), camera.cameraGetPlayerConnectionHandleToFollowInSpectatingView(), "connhandle 2");

        // first player goes spectating again
        player0.isInSpectatorMode() = true;
        b &= assertFalse(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 3");
        
        // all players join the game
        player0.isInSpectatorMode() = false;
        player1.isInSpectatorMode() = false;
        player2.isInSpectatorMode() = false;
        player3.isInSpectatorMode() = false;

        // since we have already saved connHandle of player0, we should find player1 now
        b &= assertTrue(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 4");
        b &= assertEquals(player1.getServerSideConnectionHandle(), camera.cameraGetPlayerConnectionHandleToFollowInSpectatingView(), "connhandle 4");

        b &= assertTrue(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 5");
        b &= assertEquals(player2.getServerSideConnectionHandle(), camera.cameraGetPlayerConnectionHandleToFollowInSpectatingView(), "connhandle 5");

        b &= assertTrue(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 6");
        b &= assertEquals(player3.getServerSideConnectionHandle(), camera.cameraGetPlayerConnectionHandleToFollowInSpectatingView(), "connhandle 6");

        // and now go back to beginning of container
        b &= assertTrue(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 7");
        b &= assertEquals(player0.getServerSideConnectionHandle(), camera.cameraGetPlayerConnectionHandleToFollowInSpectatingView(), "connhandle 7");

        // disconnect 2 players
        b &= assertTrue(m_gm->removePlayer(player1), "remove player 1 from gamemode");
        b &= assertEquals(1u /* number of deleted elements */, m_mapPlayers.erase(player1.getServerSideConnectionHandle()), "remove player 1 from m_mapPlayers");
        b &= assertTrue(m_gm->removePlayer(player2), "remove player 2 from gamemode");
        b &= assertEquals(1u /* number of deleted elements */, m_mapPlayers.erase(player2.getServerSideConnectionHandle()), "remove player 2 from m_mapPlayers");
        b &= assertEquals(nPlayersExpected - 2, m_mapPlayers.size(), "m_mapPlayers size 1");
        b &= assertEquals(nPlayersExpected - 2, m_gm->getPlayersTable().size(), "gamemode size 1");
        if (!b)
        {
            return false;
        }
        // player1 and player2 references are now invalid, only player0 and player3 are still valid

        b &= assertTrue(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 8");
        b &= assertEquals(player3.getServerSideConnectionHandle(), camera.cameraGetPlayerConnectionHandleToFollowInSpectatingView(), "connhandle 8");

        b &= assertTrue(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 9");
        b &= assertEquals(player0.getServerSideConnectionHandle(), camera.cameraGetPlayerConnectionHandleToFollowInSpectatingView(), "connhandle 9");

        // now spectating player0, disconnect it
        b &= assertTrue(m_gm->removePlayer(player0), "remove player 0 from gamemode");
        b &= assertEquals(1u /* number of deleted elements */, m_mapPlayers.erase(player0.getServerSideConnectionHandle()), "remove player 0 from m_mapPlayers");
        b &= assertEquals(nPlayersExpected - 3, m_mapPlayers.size(), "m_mapPlayers size 2");
        b &= assertEquals(nPlayersExpected - 3, m_gm->getPlayersTable().size(), "gamemode size 2");
        if (!b)
        {
            return false;
        }
        // player0, player1 and player2 references are now invalid, only player3 is still valid

        b &= assertTrue(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 10");
        b &= assertEquals(player3.getServerSideConnectionHandle(), camera.cameraGetPlayerConnectionHandleToFollowInSpectatingView(), "connhandle 10");

        // now spectating player3, disconnect it
        b &= assertTrue(m_gm->removePlayer(player3), "remove player 3 from gamemode");
        b &= assertEquals(1u /* number of deleted elements */, m_mapPlayers.erase(player3.getServerSideConnectionHandle()), "remove player 3 from m_mapPlayers");
        b &= assertEquals(nPlayersExpected - 4 /* shall be 0 */, m_mapPlayers.size(), "m_mapPlayers size 3");
        b &= assertEquals(nPlayersExpected - 4 /* shall be 0 */, m_gm->getPlayersTable().size(), "gamemode size 3");

        b &= assertFalse(camera.findNextValidPlayerToFollowInPlayerSpectatingView(m_mapPlayers), "find 11");

        return b;
    }

    bool test_findPrevValidPlayerToFollowInPlayerSpectatingView()
    {
        return false;
    }

    bool test_findAnyValidPlayerToFollowInPlayerSpectatingView()
    {
        return false;
    }

};
