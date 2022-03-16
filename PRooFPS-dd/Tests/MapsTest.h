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
        
        engine = NULL;
        AddSubTest("test_initially_empty", (PFNUNITSUBTEST) &MapsTest::test_initially_empty);
        AddSubTest("test_map_load", (PFNUNITSUBTEST) &MapsTest::test_map_load);
        AddSubTest("test_map_load_bad", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad);
        AddSubTest("test_map_unload_and_load_again", (PFNUNITSUBTEST) &MapsTest::test_map_unload_and_load_again);
    }

    virtual bool setUp()
    {
        bool ret = true;
        if ( engine == NULL )
        {
            engine = &PR00FsReducedRenderingEngine::createAndGet();
            ret &= assertEquals((TPRREuint)0, engine->initialize(PRRE_RENDERER_HW_FP, 800, 600, PRRE_WINDOWED, 0, 32, 24, 0, 0), "engine" );  // pretty standard display mode, should work on most systems
        }
        return ret;
    }

    virtual void TearDown()
    {
        if ( engine )
        {
            engine->shutdown();
            engine = NULL;
        }
    }

    virtual void Finalize()
    {
        CConsole::getConsoleInstance().SetLoggingState(PRRETexture::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(PRRETextureManager::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(Maps::getLoggerModuleName(), false);
    }

private:

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
        return assertFalse(maps.loaded(), "loaded") & assertEquals(0u, maps.width(), "width") & assertEquals(0u, maps.height(), "height ");
    }

    bool test_map_load()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.loaded(), "loaded 1");
        b &= assertEquals(0u, maps.width(), "width 1");
        b &= assertEquals(0u, maps.height(), "height 1");
        b &= assertTrue(maps.load("gamedata/maps/map_test.txt"), "load");
        b &= assertTrue(maps.loaded(), "loaded 2");
        b &= assertEquals(74u, maps.width(), "width 2");
        b &= assertEquals(10u, maps.height(), "height 2");
        return b;
    }

    bool test_map_load_bad()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.loaded(), "loaded 1");
        b &= assertEquals(0u, maps.width(), "width 1");
        b &= assertEquals(0u, maps.height(), "height 1");
        b &= assertFalse(maps.load("gamedata/maps/egsdghsdghsdghdsghgds.txt"), "load");
        b &= assertFalse(maps.loaded(), "loaded 2");
        b &= assertEquals(0u, maps.width(), "width 2");
        b &= assertEquals(0u, maps.height(), "height 2");
        return b;
    }

    bool test_map_unload_and_load_again()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("gamedata/maps/map_test.txt"), "load 1");
        b &= assertTrue(maps.loaded(), "loaded 1");
        b &= assertEquals(74u, maps.width(), "width 1");
        b &= assertEquals(10u, maps.height(), "height 1");
        
        maps.unload();
        b &= assertFalse(maps.loaded(), "loaded 2");
        b &= assertEquals(0u, maps.width(), "width 2");
        b &= assertEquals(0u, maps.height(), "height 2");

        b &= assertTrue(maps.load("gamedata/maps/map_test.txt"), "load 2");
        b &= assertTrue(maps.loaded(), "loaded 3");
        b &= assertEquals(74u, maps.width(), "width 3");
        b &= assertEquals(10u, maps.height(), "height 3");
        return b;
    }

}; 