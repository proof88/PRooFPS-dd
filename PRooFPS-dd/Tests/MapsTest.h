#pragma once

/*
    ###################################################################################
    MapsTest.h
    Unit test for PRooFPS-dd Maps.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include "UnitTests/UnitTest.h"

#include "Maps.h"

#include "MapTestsCommon.h"

class TestableMaps :
    public proofps_dd::Maps
{
public:
    TestableMaps(
        PGEcfgProfiles& cfgProfiles,
        PR00FsUltimateRenderingEngine& gfx) : Maps(cfgProfiles, gfx)
    {};

    virtual ~TestableMaps() {};

    friend class MapsTest;
};

class MapsTest :
    public MapTestsCommon
{
public:

    MapsTest(PGEcfgProfiles& cfgProfiles) :
        MapTestsCommon( __FILE__ ),
        m_cfgProfiles(cfgProfiles)
    {
        engine = NULL;
    }

    MapsTest(const MapsTest&) = delete;
    MapsTest& operator=(const MapsTest&) = delete;
    MapsTest(MapsTest&&) = delete;
    MapsTest&& operator=(MapsTest&&) = delete;

protected:

    virtual void Initialize() override
    {
        //CConsole::getConsoleInstance().SetLoggingState(PureTexture::getLoggerModuleName(), true);
        //CConsole::getConsoleInstance().SetLoggingState(PureTextureManager::getLoggerModuleName(), true);
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::Maps::getLoggerModuleName(), true);

        PGEInputHandler& inputHandler = PGEInputHandler::createAndGet(m_cfgProfiles);
        
        engine = &PR00FsUltimateRenderingEngine::createAndGet(m_cfgProfiles, inputHandler);
        engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        m_cbDisplayMapLoadingProgressUpdate = [this](int nProgress)
        {
            // no matter which test case we are in, whenever maps.load() invokes this cb, there should be a non-negative progress
            assertLequals(0, nProgress);
        };

        AddSubTest("test_initially_empty", (PFNUNITSUBTEST) &MapsTest::test_initially_empty);
        AddSubTest("test_map_load_bad_filename", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_filename);
        AddSubTest("test_map_load_bad_assignment", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_assignment);
        AddSubTest("test_map_load_bad_order", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_order);
        AddSubTest("test_map_load_bad_jumppad_count", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_jumppad_count);
        AddSubTest("test_map_load_bad_jumppad_force_value", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_jumppad_force_value);
        AddSubTest("test_map_load_good", (PFNUNITSUBTEST) &MapsTest::test_map_load_good);
        AddSubTest("test_map_unload_and_load_again", (PFNUNITSUBTEST) &MapsTest::test_map_unload_and_load_again);
        AddSubTest("test_map_shutdown", (PFNUNITSUBTEST)&MapsTest::test_map_shutdown);
        AddSubTest("test_map_server_decide_first_map_to_be_loaded", (PFNUNITSUBTEST)&MapsTest::test_map_server_decide_first_map_to_be_loaded);
        AddSubTest("test_map_get_random_spawnpoint", (PFNUNITSUBTEST) &MapsTest::test_map_get_random_spawnpoint);
        AddSubTest("test_map_get_leftmost_spawnpoint", (PFNUNITSUBTEST)&MapsTest::test_map_get_leftmost_spawnpoint);
        AddSubTest("test_map_get_rightmost_spawnpoint", (PFNUNITSUBTEST)&MapsTest::test_map_get_rightmost_spawnpoint);
        AddSubTest("test_map_update", (PFNUNITSUBTEST)&MapsTest::test_map_update);
        AddSubTest("test_map_handle_map_item_update_from_server", (PFNUNITSUBTEST)&MapsTest::test_map_handle_map_item_update_from_server);
    }

    virtual bool setUp() override
    {
        return assertTrue(engine && engine->isInitialized());
    }

    virtual void TearDown() override
    {
    }

    virtual void Finalize() override
    {
        if ( engine )
        {
            engine->shutdown();
            engine = NULL;
        }

        CConsole::getConsoleInstance().SetLoggingState(PureTexture::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(PureTextureManager::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::Maps::getLoggerModuleName(), false);
    }

private:

    static const unsigned int MAP_TEST_W = 44u;
    static const unsigned int MAP_TEST_H = 10u;

    PGEcfgProfiles& m_cfgProfiles;
    PR00FsUltimateRenderingEngine* engine;
    std::function<void(int)> m_cbDisplayMapLoadingProgressUpdate;

    // ---------------------------------------------------------------------------

    bool test_initially_empty()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = (assertFalse(maps.isInitialized(), "inited 1") &
            assertFalse(maps.loaded(), "loaded 1") &
            assertTrue(maps.getNextMapToBeLoaded().empty(), "getNextMapToBeLoaded 1") &
            assertTrue(maps.getFilename().empty(), "filename 1") &
            assertEquals(0u, maps.width(), "width 1") &
            assertEquals(0u, maps.height(), "height 1") &
            assertTrue(maps.getVars().empty(), "getVars 1") &
            assertEquals(PureVector(0, 0, 0), maps.getBlockPosMin(), "objects Min 1") &
            assertEquals(PureVector(0, 0, 0), maps.getBlockPosMax(), "objects Max 1") &
            assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMin(), "vertex Min 1") &
            assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMax(), "vertex Max 1") &
            assertNull(maps.getBlocks(), "blocks 1") &
            assertEquals(0, maps.getBlockCount(), "block count 1") &
            assertNull(maps.getForegroundBlocks(), "foreground blocks 1") &
            assertEquals(0, maps.getForegroundBlockCount(), "foreground block count 1") &
            assertEquals(0u, maps.getItems().size(), "item count 1") &
            assertTrue(maps.getJumppads().empty(), "jumppad count 1") &
            assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 1") &
            assertTrue(maps.getMapcycle().mapcycleGet().empty(), "mapcycle empty 1") &
            assertNull(maps.getMapcycle().mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 1") &
            assertTrue(maps.getMapcycle().availableMapsGet().empty(), "available maps empty 1") &
            assertNull(maps.getMapcycle().availableMapsGetAsCharPtrArray(), "available maps charptrarray 1") &
            assertTrue(maps.getMapcycle().availableMapsNoChangingGet().empty(), "available maps no changing empty 1")) != 0;
        
        b &= assertTrue(maps.initialize(), "init");
        b &= (assertTrue(maps.isInitialized(), "inited 2") &
            assertFalse(maps.loaded(), "loaded 2") &
            assertTrue(maps.getNextMapToBeLoaded().empty(), "getNextMapToBeLoaded 2") &
            assertTrue(maps.getFilename().empty(), "filename 2") &
            assertEquals(0u, maps.width(), "width 2") &
            assertEquals(0u, maps.height(), "height 2") &
            assertTrue(maps.getVars().empty(), "getVars 2") &
            assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints") &
            assertEquals(PureVector(0, 0, 0), maps.getBlockPosMin(), "objects Min 2") &
            assertEquals(PureVector(0, 0, 0), maps.getBlockPosMax(), "objects Max 2") &
            assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMin(), "vertex Min 2") &
            assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMax(), "vertex Max 2") &
            assertNull(maps.getBlocks(), "blocks 2") &
            assertEquals(0, maps.getBlockCount(), "block count 2") &
            assertNull(maps.getForegroundBlocks(), "foreground blocks 2") &
            assertEquals(0, maps.getForegroundBlockCount(), "foreground block count 2") &
            assertEquals(0u, maps.getItems().size(), "item count 2") &
            assertTrue(maps.getJumppads().empty(), "jumppad count 2") &
            assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 2") &
            assertFalse(maps.getMapcycle().mapcycleGet().empty(), "mapcycle empty 2") &
            assertNotNull(maps.getMapcycle().mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 2") &
            assertFalse(maps.getMapcycle().availableMapsGet().empty(), "available maps empty 2") &
            assertNotNull(maps.getMapcycle().availableMapsGetAsCharPtrArray(), "available maps charptrarray 2") &
            assertFalse(maps.getMapcycle().availableMapsNoChangingGet().empty(), "available maps no changing empty 2")) != 0;

        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(maps.getMapcycle().mapcycleGet(), maps.getMapcycle().mapcycleGetAsCharPtrArray()),
            "mapcycleGetAsCharPtrArray()");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(maps.getMapcycle().availableMapsGet(), maps.getMapcycle().availableMapsGetAsCharPtrArray()),
            "availableMapsGetAsCharPtrArray()");

        return b;
    }

    bool test_map_load_bad_filename()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("egsdghsdghsdghdsghgds.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertFalse(maps.loaded(), "loaded");
        b &= assertTrue(maps.getNextMapToBeLoaded().empty(), "getNextMapToBeLoaded");
        b &= assertTrue(maps.getFilename().empty(), "filename");

        // block and map boundaries
        b &= assertEquals(0u, maps.width(), "width");
        b &= assertEquals(0u, maps.height(), "height");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMin(), "objects Min");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMax(), "objects Max");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMin(), "vertex Min");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMax(), "vertex Max");
        b &= assertNull(maps.getBlocks(), "blocks");
        b &= assertEquals(0, maps.getBlockCount(), "block count");
        b &= assertNull(maps.getForegroundBlocks(), "foreground blocks");
        b &= assertEquals(0, maps.getForegroundBlockCount(), "foreground block count");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count");
        b &= assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id");

        // jump pads
        b &= assertTrue(maps.getJumppads().empty(), "jumppad count");
        b &= assertEquals(0u, maps.getJumppadValidVarsCount(), "jumppad vars count");

        return b;
    }

    bool test_map_load_bad_assignment()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_assignment.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertFalse(maps.loaded(), "loaded");
        b &= assertTrue(maps.getNextMapToBeLoaded().empty(), "getNextMapToBeLoaded");
        b &= assertTrue(maps.getFilename().empty(), "filename");

        // block and map boundaries
        b &= assertEquals(0u, maps.width(), "width");
        b &= assertEquals(0u, maps.height(), "height");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMin(), "objects Min");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMax(), "objects Max");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMin(), "vertex Min");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMax(), "vertex Max");
        b &= assertNull(maps.getBlocks(), "blocks");
        b &= assertEquals(0, maps.getBlockCount(), "block count");
        b &= assertNull(maps.getForegroundBlocks(), "foreground blocks");
        b &= assertEquals(0, maps.getForegroundBlockCount(), "foreground block count");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count");
        b &= assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id");

        // jump pads
        b &= assertTrue(maps.getJumppads().empty(), "jumppad count");
        b &= assertEquals(0u, maps.getJumppadValidVarsCount(), "jumppad vars count");

        return b;
    }

    bool test_map_load_bad_order()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_order.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertFalse(maps.loaded(), "loaded");
        b &= assertTrue(maps.getNextMapToBeLoaded().empty(), "getNextMapToBeLoaded");
        b &= assertTrue(maps.getFilename().empty(), "filename");

        // block and map boundaries
        b &= assertEquals(0u, maps.width(), "width");
        b &= assertEquals(0u, maps.height(), "height");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMin(), "objects Min");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMax(), "objects Max");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMin(), "vertex Min");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMax(), "vertex Max");
        b &= assertNull(maps.getBlocks(), "blocks");
        b &= assertEquals(0, maps.getBlockCount(), "block count");
        b &= assertNull(maps.getForegroundBlocks(), "foreground blocks");
        b &= assertEquals(0, maps.getForegroundBlockCount(), "foreground block count");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count");
        b &= assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id");

        // jump pads
        b &= assertTrue(maps.getJumppads().empty(), "jumppad count");
        b &= assertEquals(0u, maps.getJumppadValidVarsCount(), "jumppad vars count");

        return b;
    }

    bool test_map_load_bad_jumppad_count()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_jumppad_count.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertFalse(maps.loaded(), "loaded");
        b &= assertTrue(maps.getNextMapToBeLoaded().empty(), "getNextMapToBeLoaded");
        b &= assertTrue(maps.getFilename().empty(), "filename");

        // block and map boundaries
        b &= assertEquals(0u, maps.width(), "width");
        b &= assertEquals(0u, maps.height(), "height");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMin(), "objects Min");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMax(), "objects Max");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMin(), "vertex Min");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMax(), "vertex Max");
        b &= assertNull(maps.getBlocks(), "blocks");
        b &= assertEquals(0, maps.getBlockCount(), "block count");
        b &= assertNull(maps.getForegroundBlocks(), "foreground blocks");
        b &= assertEquals(0, maps.getForegroundBlockCount(), "foreground block count");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count");
        b &= assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id");

        // jump pads
        b &= assertTrue(maps.getJumppads().empty(), "jumppad count");
        b &= assertEquals(0u, maps.getJumppadValidVarsCount(), "jumppad vars count");

        return b;
    }

    bool test_map_load_bad_jumppad_force_value()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_jumppad_force_value.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertFalse(maps.loaded(), "loaded");
        b &= assertTrue(maps.getNextMapToBeLoaded().empty(), "getNextMapToBeLoaded");
        b &= assertTrue(maps.getFilename().empty(), "filename");

        // block and map boundaries
        b &= assertEquals(0u, maps.width(), "width");
        b &= assertEquals(0u, maps.height(), "height");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMin(), "objects Min");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMax(), "objects Max");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMin(), "vertex Min");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMax(), "vertex Max");
        b &= assertNull(maps.getBlocks(), "blocks");
        b &= assertEquals(0, maps.getBlockCount(), "block count");
        b &= assertNull(maps.getForegroundBlocks(), "foreground blocks");
        b &= assertEquals(0, maps.getForegroundBlockCount(), "foreground block count");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count");
        b &= assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id");

        // jump pads
        b &= assertTrue(maps.getJumppads().empty(), "jumppad count");
        b &= assertEquals(0u, maps.getJumppadValidVarsCount(), "jumppad vars count");

        return b;
    }

    bool test_map_load_good()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertEquals("map_test_good.txt", maps.getNextMapToBeLoaded(), "getNextMapToBeLoaded");
        b &= assertEquals("map_test_good.txt", maps.getFilename(), "filename");

        // block and map boundaries
        b &= assertEquals(MAP_TEST_W, maps.width(), "width");
        b &= assertEquals(MAP_TEST_H, maps.height(), "height");
        b &= assertEquals(PureVector(1, -static_cast<signed>(MAP_TEST_H) + 1, -1), maps.getBlockPosMin(), "objects Min");
        b &= assertEquals(PureVector(MAP_TEST_W, 0, 0), maps.getBlockPosMax(), "objects Max");
        b &= assertEquals(
            PureVector(
                maps.getBlockPosMin().getX() - 1/*blocksize_X*/ / 2.f,
                maps.getBlockPosMin().getY() - 1/*blocksize_Y*/ / 2.f,
                maps.getBlockPosMin().getZ() - 1/*blocksize_Z*/ / 2.f),
            maps.getBlocksVertexPosMin(), "vertex Min");
        b &= assertEquals(
            PureVector(
                maps.getBlockPosMax().getX() + 1/*blocksize_X*/ / 2.f,
                maps.getBlockPosMax().getY() + 1/*blocksize_Y*/ / 2.f,
                maps.getBlockPosMax().getZ() + 1/*blocksize_Z*/ / 2.f),
            maps.getBlocksVertexPosMax(), "vertex Max");
        b &= assertNotNull(maps.getBlocks(), "blocks");
        b &= assertLess(0, maps.getBlockCount(), "block count");
        b &= assertNotNull(maps.getForegroundBlocks(), "foreground blocks");
        b &= assertLess(0, maps.getForegroundBlockCount(), "foreground block count");
        
        // variables
        b &= assertEquals(4u, maps.getVars().size(), "getVars");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 2a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 2b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 2 ex"); }

        // items
        b &= assertEquals(6u, maps.getItems().size(), "item count");
        b &= assertEquals(6u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id");
        if (b)
        {
            auto it = maps.getItems().begin();
            b &= assertNotNull(it->second, "item 0") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_BAZOOKA, it->second->getType(), "item 0 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 0 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 0 tex");
            }

            it++;
            b &= assertNotNull(it->second, "item 1") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_PISTOL, it->second->getType(), "item 1 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 1 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 1 tex");
            }

            it++;
            b &= assertNotNull(it->second, "item 2") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, it->second->getType(), "item 2 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 2 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 2 tex");
            }
            
            it++;
            b &= assertNotNull(it->second, "item 3") &&
                assertEquals(proofps_dd::MapItemType::ITEM_HEALTH, it->second->getType(), "item 3 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 3 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 3 tex");
            }
            
            it++;
            b &= assertNotNull(it->second, "item 4") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_PISTOL, it->second->getType(), "item 4 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 4 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 4 tex");
            }
            
            it++;
            b &= assertNotNull(it->second, "item 5") &&
                assertEquals(proofps_dd::MapItemType::ITEM_HEALTH, it->second->getType(), "item 5 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 5 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 5 tex");
            }
        }

        // jump pads
        b &= assertEquals(2u, maps.getJumppads().size(), "jumppad count");
        b &= assertEquals(2u, maps.getJumppadValidVarsCount(), "jumppad vars count");

        // all visible block objects are just clones of reference objects
        for (int i = 0; i < maps.getBlockCount(); i++)
        {
            assertNotNull((maps.getBlocks()[i])->getReferredObject(), (std::string("block ") + std::to_string(i) + " is NOT cloned object!").c_str());
        }

        return b;
    }

    bool test_map_unload_and_load_again()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);

        // ###################################### LOAD 1 ######################################
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load 1");
        b &= assertTrue(maps.loaded(), "loaded 1");
        b &= assertEquals("map_test_good.txt", maps.getNextMapToBeLoaded(), "getNextMapToBeLoaded 1");
        b &= assertEquals("map_test_good.txt", maps.getFilename(), "filename 1");

        // block and map boundaries
        b &= assertEquals(MAP_TEST_W, maps.width(), "width 1");
        b &= assertEquals(MAP_TEST_H, maps.height(), "height 1");
        b &= assertEquals(PureVector(1, -static_cast<signed>(MAP_TEST_H) + 1, -1), maps.getBlockPosMin(), "objects Min 1");
        b &= assertEquals(PureVector(MAP_TEST_W, 0, 0), maps.getBlockPosMax(), "objects Max 1");
        b &= assertEquals(
            PureVector(
                maps.getBlockPosMin().getX() - 1/*blocksize_X*/ / 2.f,
                maps.getBlockPosMin().getY() - 1/*blocksize_Y*/ / 2.f,
                maps.getBlockPosMin().getZ() - 1/*blocksize_Z*/ / 2.f),
            maps.getBlocksVertexPosMin(), "vertex Min 1");
        b &= assertEquals(
            PureVector(
                maps.getBlockPosMax().getX() + 1/*blocksize_X*/ / 2.f,
                maps.getBlockPosMax().getY() + 1/*blocksize_Y*/ / 2.f,
                maps.getBlockPosMax().getZ() + 1/*blocksize_Z*/ / 2.f),
            maps.getBlocksVertexPosMax(), "vertex Max 1");
        b &= assertNotNull(maps.getBlocks(), "blocks 1");
        b &= assertLess(0, maps.getBlockCount(), "block count 1");
        b &= assertNotNull(maps.getForegroundBlocks(), "foreground blocks 1");
        b &= assertLess(0, maps.getForegroundBlockCount(), "foreground block count 1");

        // variables
        b &= assertEquals(4u, maps.getVars().size(), "getVars 1");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints 1");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 1a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 1b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 1 ex"); }

        // items
        b &= assertEquals(6u, maps.getItems().size(), "item count 1");
        b &= assertEquals(6u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 1");

        // jump pads
        b &= assertEquals(2u, maps.getJumppads().size(), "jumppad count 1");
        b &= assertEquals(2u, maps.getJumppadValidVarsCount(), "jumppad vars count 1");
        
        // ################################# TRY LOAD AGAIN ###################################
        b &= assertFalse(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load 2");

        // ###################################### UNLOAD ######################################
        maps.unload();
        b &= assertFalse(maps.loaded(), "loaded 2");
        b &= assertEquals(0u, maps.width(), "width 2");
        b &= assertEquals(0u, maps.height(), "height 2");
        b &= assertTrue(maps.getFilename().empty(), "filename 2");
        b &= assertTrue(maps.getNextMapToBeLoaded().empty(), "getNextMapToBeLoaded 2");

        // block and map boundaries
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMin(), "objects Min 2");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlockPosMax(), "objects Max 2");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMin(), "vertex Min 2");
        b &= assertEquals(PureVector(0, 0, 0), maps.getBlocksVertexPosMax(), "vertex Max 2");
        b &= assertNull(maps.getBlocks(), "blocks 2");
        b &= assertEquals(0, maps.getBlockCount(), "block count 2");
        b &= assertNull(maps.getForegroundBlocks(), "foreground blocks 2");
        b &= assertEquals(0, maps.getForegroundBlockCount(), "foreground block count 2");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars 2");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints 2");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count 2");
        b &= assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 2");

        // jump pads
        b &= assertEquals(0u, maps.getJumppads().size(), "jumppad count 2");
        b &= assertEquals(0u, maps.getJumppadValidVarsCount(), "jumppad vars count 2");

        // ###################################### LOAD 2 ######################################
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load 3");
        b &= assertTrue(maps.loaded(), "loaded 3");
        b &= assertEquals(MAP_TEST_W, maps.width(), "width 3");
        b &= assertEquals(MAP_TEST_H, maps.height(), "height 3");
        b &= assertEquals("map_test_good.txt", maps.getNextMapToBeLoaded(), "getNextMapToBeLoaded 3");
        b &= assertEquals("map_test_good.txt", maps.getFilename(), "filename 3");

        // block and map boundaries
        b &= assertEquals(PureVector(1, -static_cast<signed>(MAP_TEST_H) + 1, -1), maps.getBlockPosMin(), "objects Min 3");
        b &= assertEquals(PureVector(MAP_TEST_W, 0, 0), maps.getBlockPosMax(), "objects Max 3");
        b &= assertEquals(
            PureVector(
                maps.getBlockPosMin().getX() - 1/*blocksize_X*/ / 2.f,
                maps.getBlockPosMin().getY() - 1/*blocksize_Y*/ / 2.f,
                maps.getBlockPosMin().getZ() - 1/*blocksize_Z*/ / 2.f),
            maps.getBlocksVertexPosMin(), "vertex Min 3");
        b &= assertEquals(
            PureVector(
                maps.getBlockPosMax().getX() + 1/*blocksize_X*/ / 2.f,
                maps.getBlockPosMax().getY() + 1/*blocksize_Y*/ / 2.f,
                maps.getBlockPosMax().getZ() + 1/*blocksize_Z*/ / 2.f),
            maps.getBlocksVertexPosMax(), "vertex Max 3");
        b &= assertNotNull(maps.getBlocks(), "blocks 3");
        b &= assertLess(0, maps.getBlockCount(), "block count 3");
        b &= assertNotNull(maps.getForegroundBlocks(), "foreground blocks 3");
        b &= assertLess(0, maps.getForegroundBlockCount(), "foreground block count 3");

        // variables
        b &= assertEquals(4u, maps.getVars().size(), "getVars 3");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints 3");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 3a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 3b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 3 ex"); }

        // items
        b &= assertEquals(6u, maps.getItems().size(), "item count 3");
        b &= assertEquals(6u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 3");

        // jump pads
        b &= assertEquals(2u, maps.getJumppads().size(), "jumppad count 3");
        b &= assertEquals(2u, maps.getJumppadValidVarsCount(), "jumppad vars count 3");

        return b;
    }

    bool test_map_shutdown()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        b &= assertTrue(maps.isInitialized(), "initialized");
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertFalse(maps.getMapcycle().mapcycleGet().empty(), "mapcycle");
        b &= assertNotNull(maps.getMapcycle().mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 1");
        b &= assertFalse(maps.getMapcycle().availableMapsGet().empty(), "available maps empty 1");
        b &= assertNotNull(maps.getMapcycle().mapcycleGetAsCharPtrArray(), "available maps charptrarray 1");
        b &= assertFalse(maps.getMapcycle().availableMapsGet().empty(), "available maps no changing empty 1");

        maps.shutdown();

        b &= assertFalse(maps.loaded(), "loaded 2");
        b &= assertFalse(maps.isInitialized(), "initialized 2");
        b &= assertTrue(maps.getMapcycle().mapcycleGet().empty(), "mapcycle 2");
        b &= assertNull(maps.getMapcycle().mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 2");
        b &= assertTrue(maps.getNextMapToBeLoaded().empty(), "getNextMapToBeLoaded");
        b &= assertTrue(maps.getFilename().empty(), "filename");
        b &= assertTrue(maps.getMapcycle().availableMapsGet().empty(), "available maps empty 2");
        b &= assertNull(maps.getMapcycle().mapcycleGetAsCharPtrArray(), "available maps charptrarray 2");
        b &= assertTrue(maps.getMapcycle().availableMapsGet().empty(), "available maps no changing empty 2");

        return b;
    }

    bool test_map_server_decide_first_map_to_be_loaded()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertEquals("", maps.serverDecideFirstMapAndUpdateNextMapToBeLoaded(), "server decide 1");
        b &= assertEquals("", maps.getNextMapToBeLoaded(), "next map to load 1");

        b &= assertTrue(maps.initialize(), "init") &
            m_cfgProfiles.getVars()[proofps_dd::Maps::szCVarSvMap].getAsString().empty();

        if (b)
        {
            std::string sRet = maps.serverDecideFirstMapAndUpdateNextMapToBeLoaded();
            b &= assertEquals(maps.getMapcycle().mapcycleGetCurrent(), sRet, "server decide 2");
            b &= assertEquals(sRet, maps.getNextMapToBeLoaded(), "next map to load 2");
            b &= assertFalse(maps.getMapcycle().mapcycleIsCurrentLast(), "mapcycle last 1");

            // intentionally changing mapcycle position
            maps.getMapcycle().mapcycleNext();
            sRet = maps.serverDecideFirstMapAndUpdateNextMapToBeLoaded();
            b &= assertEquals(maps.getMapcycle().mapcycleGetCurrent(), sRet, "server decide 3");
            b &= assertEquals(sRet, maps.getNextMapToBeLoaded(), "next map to load 3");
            b &= assertFalse(maps.getMapcycle().mapcycleIsCurrentLast(), "mapcycle last 2");
            
            m_cfgProfiles.getVars()[proofps_dd::Maps::szCVarSvMap].Set("testtest.txt");
            sRet = maps.serverDecideFirstMapAndUpdateNextMapToBeLoaded();
            b &= assertEquals("testtest.txt", sRet, "server decide 4");
            b &= assertEquals(sRet, maps.getNextMapToBeLoaded(), "next map to load 4");
            
            // by design we require to fast-forward to last map, because this way the game will switch to the FIRST mapcycle map AFTER
            // playing on szCVarSvMap
            b &= assertTrue(maps.getMapcycle().mapcycleIsCurrentLast(), "mapcycle last 3");
        }

        return b;
    }

    bool test_map_get_random_spawnpoint()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");
        
        if ( b )
        {
            std::set<PureVector> originalSpawnpoints = maps.getSpawnpoints();
            int i = 0;
            try {
                while ( !originalSpawnpoints.empty() && (i < 50) )
                {
                    const PureVector sp = maps.getRandomSpawnpoint();
                    originalSpawnpoints.erase(sp);
                    i++;
                }
                b = assertTrue(originalSpawnpoints.empty(), "original empty");
            }
            catch (const std::exception& e) { b = assertTrue(false, e.what()); }
        }

        return b;
    }

    bool test_map_get_leftmost_spawnpoint()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");

        if (b)
        {
            try {
                const PureVector& spChecked = maps.getLeftMostSpawnpoint();

                PureVector spExpected = maps.getBlockPosMax();
                for (const auto& sp : maps.getSpawnpoints())
                {
                    if (sp.getX() < spExpected.getX())
                    {
                        spExpected = sp;
                    }
                }
                b = assertEquals(spExpected, spChecked, "vec");
            }
            catch (const std::exception& e) { b = assertTrue(false, e.what()); }
        }

        return b;
    }

    bool test_map_get_rightmost_spawnpoint()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");

        if (b)
        {
            try {
                const PureVector& spChecked = maps.getRightMostSpawnpoint();

                PureVector spExpected = maps.getBlockPosMin();
                for (const auto& sp : maps.getSpawnpoints())
                {
                    if (sp.getX() > spExpected.getX())
                    {
                        spExpected = sp;
                    }
                }
                b = assertEquals(spExpected, spChecked, "vec");
            }
            catch (const std::exception& e) { b = assertTrue(false, e.what()); }
        }

        return b;
    }

    bool test_map_update()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertLess(0u, maps.getItems().size(), "items");

        if (b)
        {
            std::vector<float> vOriginalItemPosY;
            for (const auto& itemPair : maps.getItems())
            {
                vOriginalItemPosY.push_back(itemPair.second->getPos().getY());
            }

            maps.update(60.f);

            int i = 0;
            for (const auto& itemPair : maps.getItems())
            {
                assertNotEquals(vOriginalItemPosY[i], itemPair.second->getPos().getY(), ("item " + std::to_string(i) + " pos y").c_str());
                i++;
            }
        }

        return b;
    }

    bool test_map_handle_map_item_update_from_server()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertFalse(maps.getItems().empty(), "items");

        if (b)
        {
            const auto& iLastValidItemIdInThisMap = (--maps.getItems().end())->first;
            const auto iInvalidItemIdInThisMap = iLastValidItemIdInThisMap + 1;

            // negative test with invalid item id
            proofps_dd::MsgMapItemUpdateFromServer msg{ iInvalidItemIdInThisMap, true /*taken*/};
            b &= assertFalse(maps.handleMapItemUpdateFromServer(0u /*irrelevant*/, msg), "invalid id");

            // positive test for updating item as taken
            msg.m_mapItemId = iLastValidItemIdInThisMap;
            b &= assertTrue(maps.handleMapItemUpdateFromServer(0u /*irrelevant*/, msg), "valid id 1");
            b &= assertTrue(maps.getItems().at(iLastValidItemIdInThisMap)->isTaken(), "taken");

            // positive test for updating item as untaken
            msg.m_bTaken = false;
            b &= assertTrue(maps.handleMapItemUpdateFromServer(0u /*irrelevant*/, msg), "valid id 2");
            b &= assertFalse(maps.getItems().at(iLastValidItemIdInThisMap)->isTaken(), "taken");
        }

        return b;
    }

}; 
