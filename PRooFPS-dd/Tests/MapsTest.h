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
    }

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
        AddSubTest("test_map_get_random_spawnpoint", (PFNUNITSUBTEST) &MapsTest::test_map_get_random_spawnpoint);
        AddSubTest("test_map_update", (PFNUNITSUBTEST)&MapsTest::test_map_update);
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

    static const unsigned int MAP_TEST_W = 44u;
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
        bool b = assertFalse(maps.loaded(), "loaded 1") &
            assertEquals(0u, maps.width(), "width 1") &
            assertEquals(0u, maps.height(), "height 1") &
            assertTrue(maps.getVars().empty(), "getVars 1") &
            assertEquals(PRREVector(0,0,0), maps.getObjectsPosMin(), "objects Min 1") &
            assertEquals(PRREVector(0,0,0), maps.getObjectsPosMax(), "objects Max 1") &
            assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMin(), "vertex Min 1") &
            assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMax(), "vertex Max 1");
        
        b &= assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.loaded(), "loaded 2") &
            assertEquals(0u, maps.width(), "width 2") &
            assertEquals(0u, maps.height(), "height 2") &
            assertTrue(maps.getVars().empty(), "getVars 2") &
            assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints") &
            assertEquals(PRREVector(0, 0, 0), maps.getObjectsPosMin(), "objects Min 2") &
            assertEquals(PRREVector(0, 0, 0), maps.getObjectsPosMax(), "objects Max 2") &
            assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMin(), "vertex Min 2") &
            assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMax(), "vertex Max 2");
        return b;
    }

    bool test_map_load_bad_filename()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("gamedata/maps/egsdghsdghsdghdsghgds.txt"), "load");
        b &= assertFalse(maps.loaded(), "loaded");

        // block and map boundaries
        b &= assertEquals(0u, maps.width(), "width");
        b &= assertEquals(0u, maps.height(), "height");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsPosMin(), "objects Min");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsPosMax(), "objects Max");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMin(), "vertex Min");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMax(), "vertex Max");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count");

        return b;
    }

    bool test_map_load_bad_assignment()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("gamedata/maps/map_test_bad_assignment.txt"), "load");
        b &= assertFalse(maps.loaded(), "loaded");

        // block and map boundaries
        b &= assertEquals(0u, maps.width(), "width");
        b &= assertEquals(0u, maps.height(), "height");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsPosMin(), "objects Min");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsPosMax(), "objects Max");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMin(), "vertex Min");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMax(), "vertex Max");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count");

        return b;
    }

    bool test_map_load_bad_order()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("gamedata/maps/map_test_bad_order.txt"), "load");
        b &= assertFalse(maps.loaded(), "loaded");

        // block and map boundaries
        b &= assertEquals(0u, maps.width(), "width");
        b &= assertEquals(0u, maps.height(), "height");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsPosMin(), "objects Min");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsPosMax(), "objects Max");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMin(), "vertex Min");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMax(), "vertex Max");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count");

        return b;
    }

    bool test_map_load_good()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("gamedata/maps/map_test_good.txt"), "load");
        b &= assertTrue(maps.loaded(), "loaded");

        // block and map boundaries
        b &= assertEquals(MAP_TEST_W, maps.width(), "width");
        b &= assertEquals(MAP_TEST_H, maps.height(), "height");
        b &= assertEquals(PRREVector(1, -static_cast<signed>(MAP_TEST_H)/2, -1), maps.getObjectsPosMin(), "objects Min");
        b &= assertEquals(PRREVector(MAP_TEST_W, MAP_TEST_H/2-1, 0), maps.getObjectsPosMax(), "objects Max");
        b &= assertEquals(
            PRREVector(
                maps.getObjectsPosMin().getX() - 1/*blocksize_X*/ / 2.f,
                maps.getObjectsPosMin().getY() - 1/*blocksize_Y*/ / 2.f,
                maps.getObjectsPosMin().getZ() - 1/*blocksize_Z*/ / 2.f),
            maps.getObjectsVertexPosMin(), "vertex Min");
        b &= assertEquals(
            PRREVector(
                maps.getObjectsPosMax().getX() + 1/*blocksize_X*/ / 2.f,
                maps.getObjectsPosMax().getY() + 1/*blocksize_Y*/ / 2.f,
                maps.getObjectsPosMax().getZ() + 1/*blocksize_Z*/ / 2.f),
            maps.getObjectsVertexPosMax(), "vertex Max");
        
        // variables
        b &= assertEquals(2u, maps.getVars().size(), "getVars");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 2a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 2b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 2 ex"); }

        // items
        b &= assertEquals(5u, maps.getItems().size(), "item count");
        if (b)
        {
            b &= assertNotNull(maps.getItems()[0], "item 1") &&
                assertEquals(MapItemType::ITEM_WPN_PISTOL, maps.getItems()[0]->getType(), "item 1 type") &&
                assertNotNull(maps.getItems()[0]->getObject3D().getMaterial().getTexture(), "item 1 tex");
            b &= assertNotNull(maps.getItems()[1], "item 2") &&
                assertEquals(MapItemType::ITEM_WPN_MACHINEGUN, maps.getItems()[1]->getType(), "item 2 type") &&
                assertNotNull(maps.getItems()[1]->getObject3D().getMaterial().getTexture(), "item 2 tex");
            b &= assertNotNull(maps.getItems()[2], "item 3") &&
                assertEquals(MapItemType::ITEM_HEALTH, maps.getItems()[2]->getType(), "item 3 type") &&
                assertNotNull(maps.getItems()[2]->getObject3D().getMaterial().getTexture(), "item 3 tex");
            b &= assertNotNull(maps.getItems()[3], "item 4") &&
                assertEquals(MapItemType::ITEM_WPN_PISTOL, maps.getItems()[3]->getType(), "item 4 type") &&
                assertNotNull(maps.getItems()[3]->getObject3D().getMaterial().getTexture(), "item 4 tex");
            b &= assertNotNull(maps.getItems()[4], "item 5") &&
                assertEquals(MapItemType::ITEM_HEALTH, maps.getItems()[4]->getType(), "item 5 type") &&
                assertNotNull(maps.getItems()[4]->getObject3D().getMaterial().getTexture(), "item 5 tex");
        }

        return b;
    }

    bool test_map_unload_and_load_again()
    {
        Maps maps(*engine);

        // ###################################### LOAD 1 ######################################
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("gamedata/maps/map_test_good.txt"), "load 1");
        b &= assertTrue(maps.loaded(), "loaded 1");

        // block and map boundaries
        b &= assertEquals(MAP_TEST_W, maps.width(), "width 1");
        b &= assertEquals(MAP_TEST_H, maps.height(), "height 1");
        b &= assertEquals(PRREVector(1, -static_cast<signed>(MAP_TEST_H) / 2, -1), maps.getObjectsPosMin(), "objects Min 1");
        b &= assertEquals(PRREVector(MAP_TEST_W, MAP_TEST_H / 2 - 1, 0), maps.getObjectsPosMax(), "objects Max 1");
        b &= assertEquals(
            PRREVector(
                maps.getObjectsPosMin().getX() - 1/*blocksize_X*/ / 2.f,
                maps.getObjectsPosMin().getY() - 1/*blocksize_Y*/ / 2.f,
                maps.getObjectsPosMin().getZ() - 1/*blocksize_Z*/ / 2.f),
            maps.getObjectsVertexPosMin(), "vertex Min 1");
        b &= assertEquals(
            PRREVector(
                maps.getObjectsPosMax().getX() + 1/*blocksize_X*/ / 2.f,
                maps.getObjectsPosMax().getY() + 1/*blocksize_Y*/ / 2.f,
                maps.getObjectsPosMax().getZ() + 1/*blocksize_Z*/ / 2.f),
            maps.getObjectsVertexPosMax(), "vertex Max 1");

        // variables
        b &= assertEquals(2u, maps.getVars().size(), "getVars 1");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints 1");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 1a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 1b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 1 ex"); }

        // items
        b &= assertEquals(5u, maps.getItems().size(), "item count 1");
        
        // ###################################### UNLOAD ######################################
        maps.unload();
        b &= assertFalse(maps.loaded(), "loaded 2");
        b &= assertEquals(0u, maps.width(), "width 2");
        b &= assertEquals(0u, maps.height(), "height 2");

        // block and map boundaries
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsPosMin(), "objects Min 2");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsPosMax(), "objects Max 2");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMin(), "vertex Min 2");
        b &= assertEquals(PRREVector(0, 0, 0), maps.getObjectsVertexPosMax(), "vertex Max 2");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars 2");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints 2");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count 2");

        // ###################################### LOAD 2 ######################################
        b &= assertTrue(maps.load("gamedata/maps/map_test_good.txt"), "load 2");
        b &= assertTrue(maps.loaded(), "loaded 3");
        b &= assertEquals(MAP_TEST_W, maps.width(), "width 3");
        b &= assertEquals(MAP_TEST_H, maps.height(), "height 3");

        // block and map boundaries
        b &= assertEquals(PRREVector(1, -static_cast<signed>(MAP_TEST_H) / 2, -1), maps.getObjectsPosMin(), "objects Min 3");
        b &= assertEquals(PRREVector(MAP_TEST_W, MAP_TEST_H / 2 - 1, 0), maps.getObjectsPosMax(), "objects Max 3");
        b &= assertEquals(
            PRREVector(
                maps.getObjectsPosMin().getX() - 1/*blocksize_X*/ / 2.f,
                maps.getObjectsPosMin().getY() - 1/*blocksize_Y*/ / 2.f,
                maps.getObjectsPosMin().getZ() - 1/*blocksize_Z*/ / 2.f),
            maps.getObjectsVertexPosMin(), "vertex Min 3");
        b &= assertEquals(
            PRREVector(
                maps.getObjectsPosMax().getX() + 1/*blocksize_X*/ / 2.f,
                maps.getObjectsPosMax().getY() + 1/*blocksize_Y*/ / 2.f,
                maps.getObjectsPosMax().getZ() + 1/*blocksize_Z*/ / 2.f),
            maps.getObjectsVertexPosMax(), "vertex Max 3");

        // variables
        b &= assertEquals(2u, maps.getVars().size(), "getVars 3");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints 3");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 3a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 3b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 3 ex"); }

        // items
        b &= assertEquals(5u, maps.getItems().size(), "item count 3");

        return b;
    }

    bool test_map_get_random_spawnpoint()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("gamedata/maps/map_test_good.txt"), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");
        
        if ( b )
        {
            std::set<PRREVector> originalSpawnpoints = maps.getSpawnpoints();
            int i = 0;
            try {
                while ( !originalSpawnpoints.empty() && (i < 50) )
                {
                    const PRREVector sp = maps.getRandomSpawnpoint();
                    originalSpawnpoints.erase(sp);
                    i++;
                }
                b = assertTrue(originalSpawnpoints.empty(), "original empty");
            }
            catch (const std::exception& e) { b = assertTrue(false, e.what()); }
        }

        return b;
    }

    
    bool test_map_update()
    {
        Maps maps(*engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("gamedata/maps/map_test_good.txt"), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertLess(0u, maps.getItems().size(), "items");

        if (b)
        {
            std::vector<float> vOriginalItemPosY;
            for (size_t i = 0; i < maps.getItems().size(); i++)
            {
                vOriginalItemPosY.push_back(maps.getItems()[i]->getPos().getY());
            }

            maps.Update();

            for (size_t i = 0; i < maps.getItems().size(); i++)
            {
                assertNotEquals(vOriginalItemPosY[i], maps.getItems()[i]->getPos().getY(), ("item " + std::to_string(i) + " pos y").c_str());
            }
        }

        return b;
    }

}; 