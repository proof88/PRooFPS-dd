#pragma once

/*
    ###################################################################################
    MapsTest.h
    Unit test for PRooFPS-dd Maps.
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../../../../PGE/PGE/UnitTests/UnitTest.h"
#include "../Maps.h"

class MapsTest :
    public UnitTest
{
public:

    MapsTest() :
        UnitTest( __FILE__ )
    {
        engine = NULL;
    } // PGEcfgVariableTest()

protected:

    virtual void Initialize()
    {
        //CConsole::getConsoleInstance().SetLoggingState(PRRETexture::getLoggerModuleName(), true);
        //CConsole::getConsoleInstance().SetLoggingState(PRRETextureManager::getLoggerModuleName(), true);
        CConsole::getConsoleInstance().SetLoggingState(Maps::getLoggerModuleName(), true);
        
        engine = &PR00FsReducedRenderingEngine::createAndGet();
        engine->initialize(PRRE_RENDERER_HW_FP, 800, 600, PRRE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        AddSubTest("test_initially_empty", (PFNUNITSUBTEST) &MapsTest::test_initially_empty);
        AddSubTest("test_map_load_bad_filename", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_filename);
        AddSubTest("test_map_load_bad_assignment", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_assignment);
        AddSubTest("test_map_load_bad_order", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_order);
        AddSubTest("test_map_load_good", (PFNUNITSUBTEST) &MapsTest::test_map_load_good);
        AddSubTest("test_map_unload_and_load_again", (PFNUNITSUBTEST) &MapsTest::test_map_unload_and_load_again);
    }

    virtual bool setUp()
    {
        return assertTrue(engine && engine->isInitialized());
    }

    virtual void TearDown()
    {
    }

    virtual void Finalize()
    {
        if ( engine )
        {
            engine->shutdown();
            engine = NULL;
        }

        CConsole::getConsoleInstance().SetLoggingState(PRRETexture::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(PRRETextureManager::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(Maps::getLoggerModuleName(), false);
    }

private:

    static const unsigned int MAP_TEST_W = 40u;
    static const unsigned int MAP_TEST_H = 10u;

    PR00FsReducedRenderingEngine* engine;

    // ---------------------------------------------------------------------------

    MapsTest(const MapsTest&)
    {};         

    MapsTest& operator=(const MapsTest&)
    {
        return *this;
    };

    bool test_initially_empty()
    {
        Maps maps(*engine);
        bool b = assertFalse(maps.loaded(), "loaded 1") & assertEquals(0u, maps.width(), "width 1") & assertEquals(0u, maps.height(), "height 1") & assertTrue(maps.getVars().empty(), "getVars 1");
        b &= assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.loaded(), "loaded 2") & assertEquals(0u, maps.width(), "width 2") & assertEquals(0u, maps.height(), "height 2") & assertTrue(maps.getVars().empty(), "getVars 2") &
            assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");
        return b;
    }

    bool test_map_load_bad_filename()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("gamedata/maps/egsdghsdghsdghdsghgds.txt"), "load");
        b &= assertFalse(maps.loaded(), "loaded 2");
        b &= assertEquals(0u, maps.width(), "width 2");
        b &= assertEquals(0u, maps.height(), "height 2");
        b &= assertTrue(maps.getVars().empty(), "getVars 2");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");
        return b;
    }

    bool test_map_load_bad_assignment()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("gamedata/maps/map_test_bad_assignment.txt"), "load");
        b &= assertFalse(maps.loaded(), "loaded 2");
        b &= assertEquals(0u, maps.width(), "width 2");
        b &= assertEquals(0u, maps.height(), "height 2");
        b &= assertTrue(maps.getVars().empty(), "getVars 2");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");
        return b;
    }

    bool test_map_load_bad_order()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("gamedata/maps/map_test_bad_order.txt"), "load");
        b &= assertFalse(maps.loaded(), "loaded 2");
        b &= assertEquals(0u, maps.width(), "width 2");
        b &= assertEquals(0u, maps.height(), "height 2");
        b &= assertTrue(maps.getVars().empty(), "getVars 2");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");
        return b;
    }

    bool test_map_load_good()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("gamedata/maps/map_test_good.txt"), "load");
        b &= assertTrue(maps.loaded(), "loaded 2");
        b &= assertEquals(MAP_TEST_W, maps.width(), "width 2");
        b &= assertEquals(MAP_TEST_H, maps.height(), "height 2");
        b &= assertEquals(2u, maps.getVars().size(), "getVars 2");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 2a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 2b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 2 ex"); }

        return b;
    }

    bool test_map_unload_and_load_again()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("gamedata/maps/map_test_good.txt"), "load 1");
        b &= assertTrue(maps.loaded(), "loaded 1");
        b &= assertEquals(MAP_TEST_W, maps.width(), "width 1");
        b &= assertEquals(MAP_TEST_H, maps.height(), "height 1");
        b &= assertEquals(2u, maps.getVars().size(), "getVars 1");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints 1");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 1a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 1b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 1 ex"); }
        
        maps.unload();
        b &= assertFalse(maps.loaded(), "loaded 2");
        b &= assertEquals(0u, maps.width(), "width 2");
        b &= assertEquals(0u, maps.height(), "height 2");
        b &= assertTrue(maps.getVars().empty(), "getVars 2");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints 2");

        b &= assertTrue(maps.load("gamedata/maps/map_test_good.txt"), "load 2");
        b &= assertTrue(maps.loaded(), "loaded 3");
        b &= assertEquals(MAP_TEST_W, maps.width(), "width 3");
        b &= assertEquals(MAP_TEST_H, maps.height(), "height 3");
        b &= assertEquals(2u, maps.getVars().size(), "getVars 3");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints 3");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 3a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 3b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 3 ex"); }

        return b;
    }

}; 