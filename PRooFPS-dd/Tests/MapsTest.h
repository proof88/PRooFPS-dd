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
#include "../Maps.h"
#include "../PRooFPS-dd-packet.h"

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
    public UnitTest
{
public:

    MapsTest(PGEcfgProfiles& cfgProfiles) :
        UnitTest( __FILE__ ),
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

        AddSubTest("test_is_valid_map_filename", (PFNUNITSUBTEST)&MapsTest::test_is_valid_map_filename);

        AddSubTest("test_initially_empty", (PFNUNITSUBTEST) &MapsTest::test_initially_empty);
        AddSubTest("test_map_load_bad_filename", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_filename);
        AddSubTest("test_map_load_bad_assignment", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_assignment);
        AddSubTest("test_map_load_bad_order", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_order);
        AddSubTest("test_map_load_good", (PFNUNITSUBTEST) &MapsTest::test_map_load_good);
        AddSubTest("test_map_unload_and_load_again", (PFNUNITSUBTEST) &MapsTest::test_map_unload_and_load_again);
        AddSubTest("test_map_shutdown", (PFNUNITSUBTEST)&MapsTest::test_map_shutdown);
        AddSubTest("test_map_server_decide_first_map_to_be_loaded", (PFNUNITSUBTEST)&MapsTest::test_map_server_decide_first_map_to_be_loaded);
        AddSubTest("test_map_get_random_spawnpoint", (PFNUNITSUBTEST) &MapsTest::test_map_get_random_spawnpoint);
        AddSubTest("test_map_get_leftmost_spawnpoint", (PFNUNITSUBTEST)&MapsTest::test_map_get_leftmost_spawnpoint);
        AddSubTest("test_map_get_rightmost_spawnpoint", (PFNUNITSUBTEST)&MapsTest::test_map_get_rightmost_spawnpoint);
        AddSubTest("test_map_update", (PFNUNITSUBTEST)&MapsTest::test_map_update);
        
        AddSubTest("test_map_available_maps_get", (PFNUNITSUBTEST)&MapsTest::test_map_available_maps_get);
        AddSubTest("test_map_available_maps_refresh", (PFNUNITSUBTEST)&MapsTest::test_map_available_maps_refresh);
        AddSubTest("test_map_available_maps_add_single_elem", (PFNUNITSUBTEST)&MapsTest::test_map_available_maps_add_single_elem);
        AddSubTest("test_map_available_maps_add_multi_elem", (PFNUNITSUBTEST)&MapsTest::test_map_available_maps_add_multi_elem);
        AddSubTest("test_map_available_maps_remove_by_name", (PFNUNITSUBTEST)&MapsTest::test_map_available_maps_remove_by_name);
        AddSubTest("test_map_available_maps_remove_by_index", (PFNUNITSUBTEST)&MapsTest::test_map_available_maps_remove_by_index);
        AddSubTest("test_map_available_maps_remove_multi_elem", (PFNUNITSUBTEST)&MapsTest::test_map_available_maps_remove_multi_elem);
        
        AddSubTest("test_map_mapcycle_reload", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_reload);
        AddSubTest("test_map_mapcycle_save_to_file", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_save_to_file);
        AddSubTest("test_map_mapcycle_next", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_next);
        AddSubTest("test_map_mapcycle_rewind_to_first", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_rewind_to_first);
        AddSubTest("test_map_mapcycle_forward_to_last", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_forward_to_last);
        AddSubTest("test_map_mapcycle_add_single_elem", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_add_single_elem);
        AddSubTest("test_map_mapcycle_add_multi_elem", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_add_multi_elem);
        AddSubTest("test_map_mapcycle_remove_by_name", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_remove_by_name);
        AddSubTest("test_map_mapcycle_remove_by_index", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_remove_by_index);
        AddSubTest("test_map_mapcycle_remove_multi_elem", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_remove_multi_elem);
        AddSubTest("test_map_mapcycle_clear", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_clear);
        AddSubTest("test_map_mapcycle_remove_non_existing", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_remove_non_existing);

        AddSubTest("test_map_mapcycle_available_maps_synchronize", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_available_maps_synchronize);
        AddSubTest("test_map_mapcycle_add_available_maps_remove_by_name", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_add_available_maps_remove_by_name);
        AddSubTest("test_map_mapcycle_add_available_maps_remove_multi_elem", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_add_available_maps_remove_multi_elem);
        AddSubTest("test_map_mapcycle_add_available_maps_remove_all", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_add_available_maps_remove_all);
        AddSubTest("test_map_mapcycle_remove_available_maps_add_by_name", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_remove_available_maps_add_by_name);
        AddSubTest("test_map_mapcycle_remove_available_maps_add_by_index", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_remove_available_maps_add_by_index);
        AddSubTest("test_map_mapcycle_remove_available_maps_add_multi_elem", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_remove_available_maps_add_multi_elem);
        AddSubTest("test_map_mapcycle_remove_available_maps_add_all", (PFNUNITSUBTEST)&MapsTest::test_map_mapcycle_remove_available_maps_add_all);
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

    bool checkConstCharPtrArrayElemsPointingToContainerElems(
        const std::vector<std::string>& vec,
        const char** vszArray
    )
    {
        bool bRet = true;
        for (size_t i = 0; i < vec.size(); i++)
        {
            bRet &= assertEquals(
                vec[i].c_str(),
                vszArray[i],
                (std::string("bad vszArray elem ") + std::to_string(i)).c_str());
        }
        return bRet;
    }

    bool checkConstCharPtrArrayElemsPointingToContainerElems(
        const std::set<std::string>& settt,
        const char** vszArray
    )
    {
        bool bRet = true;
        auto it = settt.begin();
        for (size_t i = 0; i < settt.size(); i++)
        {
            bRet &= assertEquals(
                it->c_str(),
                vszArray[i],
                (std::string("bad vszArray elem ") + std::to_string(i)).c_str());
            it++;
        }
        return bRet;
    }

    bool test_is_valid_map_filename()
    {
        std::string sTooLooongFilename(proofps_dd::MsgMapChangeFromServer::nMapFilenameMaxLength, 'a');
        return (assertFalse(proofps_dd::Maps::isValidMapFilename("map.txt"), "map.txt") &
            assertFalse(proofps_dd::Maps::isValidMapFilename("map_.tx"), "map_.tx") &
            assertFalse(proofps_dd::Maps::isValidMapFilename("_map.txt"), "_map.txt") &
            assertFalse(proofps_dd::Maps::isValidMapFilename("maps.txt"), "maps.txt") &
            assertFalse(proofps_dd::Maps::isValidMapFilename("mapcycle.txt"), "mapcycle.txt") &
            assertFalse(proofps_dd::Maps::isValidMapFilename("map.txt"), "map.txt") &
            assertFalse(proofps_dd::Maps::isValidMapFilename(sTooLooongFilename), "too long") &
            assertTrue(proofps_dd::Maps::isValidMapFilename("map_.txt"), "ok")) != 0;
    }

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
            assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 1") &
            assertTrue(maps.mapcycleGet().empty(), "mapcycle empty 1") &
            assertNull(maps.mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 1") &
            assertTrue(maps.availableMapsGet().empty(), "available maps empty 1") &
            assertNull(maps.availableMapsGetAsCharPtrArray(), "available maps charptrarray 1") &
            assertTrue(maps.availableMapsNoChangingGet().empty(), "available maps no changing empty 1")) != 0;
        
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
            assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 2") &
            assertFalse(maps.mapcycleGet().empty(), "mapcycle empty 2") &
            assertNotNull(maps.mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 2") &
            assertFalse(maps.availableMapsGet().empty(), "available maps empty 2") &
            assertNotNull(maps.availableMapsGetAsCharPtrArray(), "available maps charptrarray 2") &
            assertFalse(maps.availableMapsNoChangingGet().empty(), "available maps no changing empty 2")) != 0;

        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
            "mapcycleGetAsCharPtrArray()");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
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
        b &= assertEquals(PureVector(1, -static_cast<signed>(MAP_TEST_H)/2, -1), maps.getBlockPosMin(), "objects Min");
        b &= assertEquals(PureVector(MAP_TEST_W, MAP_TEST_H/2-1, 0), maps.getBlockPosMax(), "objects Max");
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
        b &= assertEquals(2u, maps.getVars().size(), "getVars");
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

        // all visible block objects are just clones of referenc objects
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
        b &= assertEquals(PureVector(1, -static_cast<signed>(MAP_TEST_H) / 2, -1), maps.getBlockPosMin(), "objects Min 1");
        b &= assertEquals(PureVector(MAP_TEST_W, MAP_TEST_H / 2 - 1, 0), maps.getBlockPosMax(), "objects Max 1");
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
        b &= assertEquals(2u, maps.getVars().size(), "getVars 1");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints 1");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 1a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 1b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 1 ex"); }

        // items
        b &= assertEquals(6u, maps.getItems().size(), "item count 1");
        b &= assertEquals(6u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 1");
        
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

        // ###################################### LOAD 2 ######################################
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load 3");
        b &= assertTrue(maps.loaded(), "loaded 3");
        b &= assertEquals(MAP_TEST_W, maps.width(), "width 3");
        b &= assertEquals(MAP_TEST_H, maps.height(), "height 3");
        b &= assertEquals("map_test_good.txt", maps.getNextMapToBeLoaded(), "getNextMapToBeLoaded 3");
        b &= assertEquals("map_test_good.txt", maps.getFilename(), "filename 3");

        // block and map boundaries
        b &= assertEquals(PureVector(1, -static_cast<signed>(MAP_TEST_H) / 2, -1), maps.getBlockPosMin(), "objects Min 3");
        b &= assertEquals(PureVector(MAP_TEST_W, MAP_TEST_H / 2 - 1, 0), maps.getBlockPosMax(), "objects Max 3");
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
        b &= assertEquals(2u, maps.getVars().size(), "getVars 3");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints 3");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 3a");
            b &= assertEquals(2.f, maps.getVars().at("Gravity").getAsFloat(), "getVars 3b");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 3 ex"); }

        // items
        b &= assertEquals(6u, maps.getItems().size(), "item count 3");
        b &= assertEquals(6u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 3");

        return b;
    }

    bool test_map_shutdown()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        b &= assertTrue(maps.isInitialized(), "initialized");
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertFalse(maps.mapcycleGet().empty(), "mapcycle");
        b &= assertNotNull(maps.mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 1");
        b &= assertFalse(maps.availableMapsGet().empty(), "available maps empty 1");
        b &= assertNotNull(maps.mapcycleGetAsCharPtrArray(), "available maps charptrarray 1");
        b &= assertFalse(maps.availableMapsGet().empty(), "available maps no changing empty 1");

        maps.shutdown();

        b &= assertFalse(maps.loaded(), "loaded 2");
        b &= assertFalse(maps.isInitialized(), "initialized 2");
        b &= assertTrue(maps.mapcycleGet().empty(), "mapcycle 2");
        b &= assertNull(maps.mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 2");
        b &= assertTrue(maps.getNextMapToBeLoaded().empty(), "getNextMapToBeLoaded");
        b &= assertTrue(maps.getFilename().empty(), "filename");
        b &= assertTrue(maps.availableMapsGet().empty(), "available maps empty 2");
        b &= assertNull(maps.mapcycleGetAsCharPtrArray(), "available maps charptrarray 2");
        b &= assertTrue(maps.availableMapsGet().empty(), "available maps no changing empty 2");

        return b;
    }

    bool test_map_server_decide_first_map_to_be_loaded()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertEquals("", maps.serverDecideFirstMapAndUpdateNextMapToBeLoaded(), "server decide 1");
        b &= assertEquals("", maps.getNextMapToBeLoaded(), "next map to load 1");

        b &= assertTrue(maps.initialize(), "init") &
            m_cfgProfiles.getVars()[proofps_dd::Maps::CVAR_SV_MAP].getAsString().empty();

        if (b)
        {
            std::string sRet = maps.serverDecideFirstMapAndUpdateNextMapToBeLoaded();
            b &= assertEquals(maps.mapcycleGetCurrent(), sRet, "server decide 2");
            b &= assertEquals(sRet, maps.getNextMapToBeLoaded(), "next map to load 2");
            b &= assertFalse(maps.mapcycleIsCurrentLast(), "mapcycle last 1");

            // intentionally changing mapcycle position
            maps.mapcycleNext();
            b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle last 2");
            sRet = maps.serverDecideFirstMapAndUpdateNextMapToBeLoaded();
            b &= assertEquals(maps.mapcycleGetCurrent(), sRet, "server decide 3");
            b &= assertEquals(sRet, maps.getNextMapToBeLoaded(), "next map to load 3");
            b &= assertFalse(maps.mapcycleIsCurrentLast(), "mapcycle last 3");
            
            m_cfgProfiles.getVars()[proofps_dd::Maps::CVAR_SV_MAP].Set("testtest.txt");
            sRet = maps.serverDecideFirstMapAndUpdateNextMapToBeLoaded();
            b &= assertEquals("testtest.txt", sRet, "server decide 4");
            b &= assertEquals(sRet, maps.getNextMapToBeLoaded(), "next map to load 4");
            
            // by design we require to fast-forward to last map, because this way the game will switch to the FIRST mapcycle map AFTER
            // playing on CVAR_SV_MAP
            b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle last 4");
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

            maps.Update(60.f);

            int i = 0;
            for (const auto& itemPair : maps.getItems())
            {
                assertNotEquals(vOriginalItemPosY[i], itemPair.second->getPos().getY(), ("item " + std::to_string(i) + " pos y").c_str());
                i++;
            }
        }

        return b;
    }

    bool test_map_available_maps_get()
    {
        proofps_dd::Maps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        if (!b)
        {
            return false;
        }

        std::set<std::string> vExpectedAvailableMaps = {
            "map_test_bad_assignment.txt",
            "map_test_bad_order.txt",
            "map_test_good.txt"/*,
            "map_warena.txt",
            "map_warhouse.txt"*/ /* commented since Maps::inititalize() does mapcycle-available maps sync */
        };

        const auto& foundAvailableMaps = maps.availableMapsGet();
        for (const auto& sMapName : foundAvailableMaps)
        {
            const auto itFound = vExpectedAvailableMaps.find(sMapName);
            /* mapcycle.txt must not be found, as we require map name to start with "map_" */
            b &= assertFalse(itFound == vExpectedAvailableMaps.end(), (std::string("Unexpected map found:") + sMapName).c_str());
            if (itFound != vExpectedAvailableMaps.end())
            {
                vExpectedAvailableMaps.erase(sMapName);
            }
        }
        b &= assertTrue(vExpectedAvailableMaps.empty(), "Not found all expected maps!");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
            "availableMapsGetAsCharPtrArray()");

        return b;
    }

    bool test_map_available_maps_refresh()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        if (!b)
        {
            return false;
        }

        // trick to clear out available maps, we can refresh them without initializing Maps actually ...
        maps.shutdown();
        maps.availableMapsRefresh();

        std::set<std::string> vExpectedAvailableMaps = {
            "map_test_bad_assignment.txt",
            "map_test_bad_order.txt",
            "map_test_good.txt",
            "map_warena.txt",
            "map_warhouse.txt"
        };

        const auto& foundAvailableMaps = maps.availableMapsGet();
        for (const auto& sMapName : foundAvailableMaps)
        {
            const auto itFound = vExpectedAvailableMaps.find(sMapName);
            /* mapcycle.txt must not be found, as we require map name to start with "map_" */
            b &= assertFalse(itFound == vExpectedAvailableMaps.end(), (std::string("Unexpected map found:") + sMapName).c_str());
            if (itFound != vExpectedAvailableMaps.end())
            {
                vExpectedAvailableMaps.erase(sMapName);
            }
        }
        b &= assertTrue(vExpectedAvailableMaps.empty(), "Not found all expected maps!");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
            "availableMapsGetAsCharPtrArray()");

        return b;
    }

    bool test_map_available_maps_add_single_elem()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        const auto nOriginalSize = maps.availableMapsGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        if (!b)
        {
            return false;
        }

        b &= assertFalse(maps.availableMapsAdd("map_test_good.txt"), "add 1"); // already in
        b &= assertTrue(maps.availableMapsAdd("map_asdasdasd.txt"), "add 2");
        b &= assertEquals(nOriginalSize + 1, maps.availableMapsGet().size(), "size 2");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
            "availableMapsGetAsCharPtrArray()");

        return b;
    }

    bool test_map_available_maps_add_multi_elem()
    {
        const std::vector<std::string> vAddThese = {
            "map_asdasd.txt",
            "map_asdasdasd.txt"
        };

        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        const auto nOriginalSize = maps.availableMapsGet().size();
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        if (!b)
        {
            return false;
        }

        b &= assertTrue(maps.availableMapsAdd(vAddThese), "add 1");
        b &= assertEquals(nOriginalSize + vAddThese.size(), maps.availableMapsGet().size(), "size 2");
        b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size 3");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
            "availableMapsGetAsCharPtrArray() 1");

        if (b)
        {
            // try adding same elements again, should fail
            b &= assertFalse(maps.availableMapsAdd(vAddThese), "add 2");
            b &= assertEquals(nOriginalSize + vAddThese.size(), maps.availableMapsGet().size(), "size 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size 5");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_map_available_maps_remove_by_name()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        const auto nOriginalSize = maps.availableMapsGet().size();
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        if (b)
        {
            b &= assertFalse(maps.availableMapsRemove(""), "remove 1");
            b &= assertEquals(nOriginalSize, maps.availableMapsGet().size(), "size 2");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size 3");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 1");

            b &= assertTrue(maps.availableMapsRemove("map_test_good.txt"), "remove 2");
            b &= assertEquals(nOriginalSize - 1, maps.availableMapsGet().size(), "size 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size 5");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_map_available_maps_remove_by_index()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        const auto nOriginalSize = maps.availableMapsGet().size();
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        if (b)
        {
            b &= assertFalse(maps.availableMapsRemove(maps.availableMapsGet().size()), "remove 1");
            b &= assertEquals(nOriginalSize, maps.availableMapsGet().size(), "size 2");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size 3");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 1");

            const std::string sMapDeleted = maps.availableMapsGetElem(0);
            b &= assertTrue(maps.availableMapsRemove(0), "remove 2");
            b &= assertTrue(
                std::find(maps.availableMapsGet().begin(), maps.availableMapsGet().end(), sMapDeleted) == maps.availableMapsGet().end(),
                "cannot find deleted item");
            b &= assertEquals(nOriginalSize - 1, maps.availableMapsGet().size(), "size 3");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size 4");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_map_available_maps_remove_multi_elem()
    {
        const std::vector<std::string> vRemoveThese = {
           "map_test_bad_assignment.txt",
           "map_test_bad_order.txt"
        };

        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        const auto nOriginalSize = maps.availableMapsGet().size();
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        if (b)
        {
            b &= assertTrue(maps.availableMapsRemove(vRemoveThese), "remove 1");
            b &= assertEquals(nOriginalSize - vRemoveThese.size(), maps.availableMapsGet().size(), "size 2");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size 3");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 1");
        }

        if (b)
        {
            // deleting the same should not succeed for the 2nd time
            b &= assertFalse(maps.availableMapsRemove(vRemoveThese), "remove 2");
            b &= assertEquals(nOriginalSize - vRemoveThese.size(), maps.availableMapsGet().size(), "size 3");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size 4");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_map_mapcycle_reload()
    {
        TestableMaps maps(m_cfgProfiles, *engine);

        // negative test before initialize(), positive tests after initialize()
        // update: even before initialize(), this works, and I'm not changing that now.
        bool b = assertTrue(maps.mapcycleReload(), "reload 1");
        b &= assertNotNull(maps.mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 1");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
            "mapcycleGetAsCharPtrArray() 1");

        b &= assertTrue(maps.initialize(), "init");
        const auto originalMapcycle = maps.mapcycleGet();
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertGreater(maps.mapcycleGet().size(), 1u, "mapcycle size");  // should be at least 2 maps there
        b &= assertFalse(maps.mapcycleIsCurrentLast(), "mapcycle islast 1");

        if (b)
        {
            // just make sure our current is also changed, we are testing it to be the original after reload
            maps.mapcycleNext();
            b &= assertNotEquals(sFirstMapName, maps.mapcycleGetCurrent(), "current 1");
            b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

            b &= assertTrue(maps.mapcycleReload(), "reload 2");
            b &= assertNotNull(maps.mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 2");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray() 2");
            b &= assertTrue(originalMapcycle == maps.mapcycleGet(), "equals");
            b &= assertEquals(sFirstMapName, maps.mapcycleGetCurrent(), "current 2");
            b &= assertFalse(maps.mapcycleIsCurrentLast(), "mapcycle islast 3");
        }

        return b;
    }

    bool test_map_mapcycle_save_to_file()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        const auto originalMapcycle = maps.mapcycleGet();

        b &= assertGreater(maps.mapcycleGet().size(), 1u, "mapcycle size 1");  // should be at least 2 maps there
        maps.mapcycleClear();
        b &= assertEquals(0u, maps.mapcycleGet().size(), "mapcycle size 2");
        b &= assertTrue(maps.mapcycleSaveToFile(), "save 1");

        b &= assertFalse(maps.mapcycleReload(), "reload 1"); // false due to empty file
        b &= assertEquals(0u, maps.mapcycleGet().size(), "mapcycle size 3");
        b &= assertTrue(maps.mapcycleAdd(originalMapcycle), "add");
        b &= assertTrue(maps.mapcycleSaveToFile(), "save 2");

        maps.mapcycleClear();
        b &= assertTrue(maps.mapcycleReload(), "reload 2");
        b &= assertTrue(originalMapcycle == maps.mapcycleGet(), "final equals");

        return b;
    }

    bool test_map_mapcycle_next()
    {
        TestableMaps maps(m_cfgProfiles, *engine);

        // negative test before initialize(), positive tests after initialize()
        bool b = assertEquals("", maps.mapcycleNext(), "next 1");

        b &= assertTrue(maps.initialize(), "init");
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        b &= assertGreater(maps.mapcycleGet().size(), 1u, "mapcycle size");  // should be at least 2 maps there

        if (b)
        {
            size_t nNextCount = 0;
            do
            {
                ++nNextCount;
                const std::string sCurrent = maps.mapcycleNext();
                b &= assertEquals(maps.mapcycleGetCurrent(), sCurrent, "next ret in loop");
            } while ((maps.mapcycleGetCurrent() != sFirstMapName) && (nNextCount < 100u));
            b &= assertNotEquals(100u, nNextCount, "nNextCount 1");
            b &= assertEquals(maps.mapcycleGet().size(), nNextCount, "nNextCount 2");
        }

        return b;
    }

    bool test_map_mapcycle_rewind_to_first()
    {
        TestableMaps maps(m_cfgProfiles, *engine);

        // negative test before initialize(), positive tests after initialize()
        bool b = assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 1");
        b &= assertEquals("", maps.mapcycleRewindToFirst(), "mapcycle rewind to first 1");

        b &= assertTrue(maps.initialize(), "init");
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        b &= assertGreater(maps.mapcycleGet().size(), 1u, "mapcycle size");  // should be at least 2 maps there
        b &= assertFalse(maps.mapcycleIsCurrentLast(), "mapcycle islast 1");

        if (b)
        {
            maps.mapcycleNext();
            b &= assertNotEquals(sFirstMapName, maps.mapcycleGetCurrent(), "current 1");
            b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

            const std::string sAllegedFirstMapName = maps.mapcycleRewindToFirst();
            b &= assertFalse(sAllegedFirstMapName.empty(), "mapcycle rewind to first 2 a");
            b &= assertEquals(sFirstMapName, sAllegedFirstMapName, "mapcycle rewind to first 2 b");
            b &= assertEquals("map_warhouse.txt", sAllegedFirstMapName, "mapcycle rewind to first 2 c specific name");
            b &= assertEquals(sFirstMapName, maps.mapcycleGetCurrent(), "current 2");
            b &= assertFalse(maps.mapcycleIsCurrentLast(), "mapcycle islast 3");
        }

        return b;
    }

    bool test_map_mapcycle_forward_to_last()
    {
        TestableMaps maps(m_cfgProfiles, *engine);

        // negative test before initialize(), positive tests after initialize()
        bool b = assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 1");
        b &= assertEquals("", maps.mapcycleForwardToLast(), "mapcycle forward to last 1");

        b &= assertTrue(maps.initialize(), "init");
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        b &= assertGreater(maps.mapcycleGet().size(), 1u, "mapcycle size");  // should be at least 2 maps there
        b &= assertFalse(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            const std::string sLastMapName = maps.mapcycleForwardToLast();
            b &= assertFalse(sLastMapName.empty(), "mapcycle forward to last 2 a");
            b &= assertNotEquals(sFirstMapName, sLastMapName, "mapcycle forward to last 2 b");
            b &= assertEquals("map_warena.txt", sLastMapName, "mapcycle forward to last 2 c specific name");
            b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 3");
        }

        return b;
    }

    bool test_map_mapcycle_add_single_elem()
    {
        TestableMaps maps(m_cfgProfiles, *engine);

        // even before initialize(), this is working
        bool b = assertTrue(maps.mapcycleGet().empty(), "mapcycle empty 1");
        b &= assertTrue(maps.mapcycleAdd("map_asdasdasd.txt"), "add 1");
        b &= assertFalse(maps.mapcycleGet().empty(), "mapcycle empty 2");

        b &= assertTrue(maps.initialize(), "init");
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        const auto nOriginalSize = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            b &= assertFalse(maps.mapcycleAdd("map_warhouse.txt"), "add neg"); // already there
            b &= assertTrue(maps.mapcycleAdd("map_asdasdasd.txt"), "add 2");
            b &= assertEquals(nOriginalSize + 1, maps.mapcycleGet().size(), "size 2");
            b &= assertEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_add_multi_elem()
    {
        const std::vector<std::string> vAddThese = {
            "map_asdasd.txt",
            "map_asdasdasd.txt"
        };

        TestableMaps maps(m_cfgProfiles, *engine);

        // even before initialize(), this is working
        bool b = assertTrue(maps.mapcycleGet().empty(), "mapcycle empty 1");
        b &= assertTrue(maps.mapcycleAdd(vAddThese), "add 1");
        b &= assertEquals(vAddThese.size(), maps.mapcycleGet().size(), "size 0");

        b &= assertTrue(maps.initialize(), "init");
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        const auto nOriginalSize = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            b &= assertTrue(maps.mapcycleAdd(vAddThese), "add 2");
            b &= assertEquals(nOriginalSize + vAddThese.size(), maps.mapcycleGet().size(), "size 2");
            b &= assertEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray() 1");
        }

        if (b)
        {
            // adding the same should not succeed for the 2nd time
            b &= assertFalse(maps.mapcycleAdd(vAddThese), "add neg");
            b &= assertEquals(nOriginalSize + vAddThese.size(), maps.mapcycleGet().size(), "size 3");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_map_mapcycle_remove_by_name()
    {
        TestableMaps maps(m_cfgProfiles, *engine);

        // negative test before initialize(), positive tests after initialize()
        bool b = assertTrue(maps.mapcycleGet().empty(), "mapcycle empty 1");
        b &= assertFalse(maps.mapcycleRemove("map_asdasdasd.txt"), "remove 1");
        b &= assertTrue(maps.mapcycleGet().empty(), "mapcycle empty 2");

        b &= assertTrue(maps.initialize(), "init");
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        const auto nOriginalSize = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            b &= assertTrue(maps.mapcycleRemove(sFirstMapName), "remove 2");
            b &= assertEquals(nOriginalSize - 1, maps.mapcycleGet().size(), "size 2");
            b &= assertNotEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_remove_by_index()
    {
        TestableMaps maps(m_cfgProfiles, *engine);

        // negative test before initialize(), positive tests after initialize()
        bool b = assertTrue(maps.mapcycleGet().empty(), "mapcycle empty 1");
        b &= assertFalse(maps.mapcycleRemove(0), "remove 1");
        b &= assertTrue(maps.mapcycleGet().empty(), "mapcycle empty 2");

        b &= assertTrue(maps.initialize(), "init");
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        const auto nOriginalSize = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            b &= assertTrue(maps.mapcycleRemove(0), "remove 2");
            b &= assertEquals(nOriginalSize - 1, maps.mapcycleGet().size(), "size 2");
            b &= assertNotEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_remove_multi_elem()
    {
        const std::vector<std::string> vRemoveThese = {
           "map_warhouse.txt",
           "map_warena.txt"
        };

        TestableMaps maps(m_cfgProfiles, *engine);

        // negative test before initialize(), positive tests after initialize()
        bool b = assertTrue(maps.mapcycleGet().empty(), "mapcycle empty 1");
        b &= assertFalse(maps.mapcycleRemove(vRemoveThese), "remove 1");
        b &= assertTrue(maps.mapcycleGet().empty(), "mapcycle empty 2");

        b &= assertTrue(maps.initialize(), "init");
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        const auto nOriginalSize = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            b &= assertTrue(maps.mapcycleRemove(vRemoveThese), "remove 2");
            b &= assertEquals(nOriginalSize - vRemoveThese.size(), maps.mapcycleGet().size(), "size 2");
            b &= assertNotEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray() 1");
        }

        if (b)
        {
            // deleting the same should not succeed for the 2nd time
            b &= assertFalse(maps.mapcycleRemove(vRemoveThese), "remove 3");
            b &= assertEquals(nOriginalSize - vRemoveThese.size(), maps.mapcycleGet().size(), "size 3");
            b &= assertNotEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first 2");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_map_mapcycle_clear()
    {
        TestableMaps maps(m_cfgProfiles, *engine);

        bool b = assertTrue(maps.initialize(), "init");
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        b &= assertGreater(maps.mapcycleGet().size(), 1u, "size 1");  // should be at least 2 maps there

        if (b)
        {
            maps.mapcycleClear();
            b &= assertTrue(maps.mapcycleGet().empty(), "clear");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_remove_non_existing()
    {
        TestableMaps maps(m_cfgProfiles, *engine);

        bool b = assertTrue(maps.initialize(), "init");

        const auto nOriginalSize = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        // calling it now totally empties mapcycle since initialize() already invoked it too
        b &= assertEquals(nOriginalSize, maps.mapcycleRemoveNonExisting(), "remove nonexisting 1");
        b &= assertEquals(0u, maps.mapcycleGet().size(), "size 2");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
            "mapcycleGetAsCharPtrArray()");

        return b;
    }

    bool test_map_mapcycle_available_maps_synchronize()
    {
        TestableMaps maps(m_cfgProfiles, *engine);

        bool b = assertTrue(maps.initialize(), "init");
        const auto nOriginalSize = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        for (const auto& sMapcycleMap : maps.mapcycleGet())
        {
            b &= assertTrue(
                std::find(maps.availableMapsGet().begin(), maps.availableMapsGet().end(), sMapcycleMap) == maps.availableMapsGet().end(),
                "mapcycle item in available maps");
        }

        b &= assertTrue(maps.mapcycleAdd("map_asdasdasd.txt"), "add");
        b &= assertEquals(nOriginalSize + 1, maps.mapcycleGet().size(), "size 2");
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();

        if (b)
        {
            maps.mapcycle_availableMaps_Synchronize();
            
            // fictive map should disappear from mapcycle
            b &= assertEquals(nOriginalSize, maps.mapcycleGet().size(), "size 3");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size 4");
            b &= assertTrue(
                std::find(maps.mapcycleGet().begin(), maps.mapcycleGet().end(), "map_asdasdasd.txt") == maps.mapcycleGet().end(),
                "fictive mapcycle item");

            // available maps should not contain any item present in mapcycle
            for (const auto& sMapcycleMap : maps.mapcycleGet())
            {
                b &= assertTrue(
                    std::find(maps.availableMapsGet().begin(), maps.availableMapsGet().end(), sMapcycleMap) == maps.availableMapsGet().end(),
                    "mapcycle item in available maps");
            }

            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_add_available_maps_remove_by_name()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");

        const auto nOriginalSizeMapCycle = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = maps.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();

        if (b)
        {
            // should fail because map is empty
            b &= assertFalse(maps.mapcycleAdd_availableMapsRemove(""), "add neg 1");
            b &= assertEquals(nOriginalSizeMapCycle, maps.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(nOriginalSizeAvailableMaps, maps.availableMapsGet().size(), "size available maps 2");

            // should fail because map is already in mapcycle
            b &= assertFalse(maps.mapcycleAdd_availableMapsRemove("map_warhouse.txt"), "add neg 2");
            b &= assertEquals(nOriginalSizeMapCycle, maps.mapcycleGet().size(), "size mapcycle 3");
            b &= assertEquals(nOriginalSizeAvailableMaps, maps.availableMapsGet().size(), "size available maps 3");
            
            b &= assertTrue(maps.mapcycleAdd_availableMapsRemove("map_test_good.txt"), "add 1");
            b &= assertEquals(nOriginalSizeMapCycle + 1, maps.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps - 1, maps.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                std::find(maps.availableMapsGet().begin(), maps.availableMapsGet().end(), "map_test_good.txt") == maps.availableMapsGet().end(),
                "can find deleted item in available maps");
            b &= assertFalse(
                std::find(maps.mapcycleGet().begin(), maps.mapcycleGet().end(), "map_test_good.txt") == maps.mapcycleGet().end(),
                "cannot find deleted item in mapcycle");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_add_available_maps_remove_multi_elem()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");

        const auto nOriginalSizeMapCycle = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = maps.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();

        if (b)
        {
            const std::vector<std::string> vAddRemoveThese_Fail = {
               "map_warhouse.txt",
               "map_warena.txt"
            };

            // should fail because maps are already in mapcycle
            b &= assertFalse(maps.mapcycleAdd_availableMapsRemove(vAddRemoveThese_Fail), "add neg 1");
            b &= assertEquals(nOriginalSizeMapCycle, maps.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(nOriginalSizeAvailableMaps, maps.availableMapsGet().size(), "size available maps 2");

            const std::vector<std::string> vAddRemoveThese_Fail2 = {
               ""
            };
            // should fail because of empty string
            b &= assertFalse(maps.mapcycleAdd_availableMapsRemove(vAddRemoveThese_Fail2), "add neg 2");
            b &= assertEquals(nOriginalSizeMapCycle, maps.mapcycleGet().size(), "size mapcycle 3");
            b &= assertEquals(nOriginalSizeAvailableMaps, maps.availableMapsGet().size(), "size available maps 3");

            const std::vector<std::string> vAddRemoveThese_Pass = {
               "map_test_bad_assignment.txt",
               "map_test_bad_order.txt",
               "map_test_good.txt"
            };

            b &= assertTrue(maps.mapcycleAdd_availableMapsRemove(vAddRemoveThese_Pass), "add 1");
            b &= assertEquals(nOriginalSizeMapCycle + vAddRemoveThese_Pass.size(), maps.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps - vAddRemoveThese_Pass.size(), maps.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first");
            for (const auto& sMapCheck : vAddRemoveThese_Pass)
            {
                b &= assertTrue(
                    std::find(maps.availableMapsGet().begin(), maps.availableMapsGet().end(), sMapCheck) == maps.availableMapsGet().end(),
                    "can find deleted item in available maps");
                b &= assertFalse(
                    std::find(maps.mapcycleGet().begin(), maps.mapcycleGet().end(), sMapCheck) == maps.mapcycleGet().end(),
                    "cannot find deleted item in mapcycle");
            }
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_add_available_maps_remove_all()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        const auto nOriginalSizeMapCycle = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = maps.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();

        if (b)
        {
            b &= assertTrue(maps.mapcycleAdd_availableMapsRemove(), "add");
            b &= assertEquals(nOriginalSizeMapCycle + nOriginalSizeAvailableMaps, maps.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(0u, maps.availableMapsGet().size(), "size available maps 2");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(maps.mapcycleGet()[0u], maps.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_remove_available_maps_add_by_name()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        // add this elem so we will have a valid thing to make positive test for
        b &= assertTrue(maps.mapcycleAdd("map_asdasd.txt"), "extend");
        
        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");

        const auto nOriginalSizeMapCycle = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = maps.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();

        if (b)
        {
            // should fail because map is empty
            b &= assertFalse(maps.mapcycleRemove_availableMapsAdd(""), "add neg 1");
            b &= assertEquals(nOriginalSizeMapCycle, maps.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(nOriginalSizeAvailableMaps, maps.availableMapsGet().size(), "size available maps 2");

            // should fail because map is NOT in mapcycle
            b &= assertFalse(maps.mapcycleRemove_availableMapsAdd("map_test_good.txt"), "add neg 2");
            b &= assertEquals(nOriginalSizeMapCycle, maps.mapcycleGet().size(), "size mapcycle 3");
            b &= assertEquals(nOriginalSizeAvailableMaps, maps.availableMapsGet().size(), "size available maps 3");

            b &= assertTrue(maps.mapcycleRemove_availableMapsAdd("map_asdasd.txt"), "add 1");
            b &= assertEquals(nOriginalSizeMapCycle - 1, maps.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps + 1, maps.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first");
            b &= assertFalse(
                std::find(maps.availableMapsGet().begin(), maps.availableMapsGet().end(), "map_asdasd.txt") == maps.availableMapsGet().end(),
                "cannot find deleted item in available maps");
            b &= assertTrue(
                std::find(maps.mapcycleGet().begin(), maps.mapcycleGet().end(), "map_asdasd.txt") == maps.mapcycleGet().end(),
                "can find deleted item in mapcycle");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_remove_available_maps_add_by_index()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        // add this elem so we will have a valid thing to make positive test for
        b &= assertTrue(maps.mapcycleAdd("map_asdasd.txt"), "extend");

        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");

        const auto nOriginalSizeMapCycle = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = maps.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();

        if (b)
        {
            // should fail because of bad index
            b &= assertFalse(maps.mapcycleRemove_availableMapsAdd(maps.mapcycleGet().size()), "add neg 1");
            b &= assertEquals(nOriginalSizeMapCycle, maps.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(nOriginalSizeAvailableMaps, maps.availableMapsGet().size(), "size available maps 2");

            b &= assertTrue(maps.mapcycleRemove_availableMapsAdd(maps.mapcycleGet().size() - 1), "add 1");
            b &= assertEquals(nOriginalSizeMapCycle - 1, maps.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps + 1, maps.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first");
            b &= assertFalse(
                std::find(maps.availableMapsGet().begin(), maps.availableMapsGet().end(), "map_asdasd.txt") == maps.availableMapsGet().end(),
                "cannot find deleted item in available maps");
            b &= assertTrue(
                std::find(maps.mapcycleGet().begin(), maps.mapcycleGet().end(), "map_asdasd.txt") == maps.mapcycleGet().end(),
                "can find deleted item in mapcycle");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_remove_available_maps_add_multi_elem()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        const std::vector<std::string> vRemoveAddThese_Pass = {
               "msp_asdasd.txt",
               "msp_asdasd_2.txt"
        };
        for (const auto& sMapCheck : vRemoveAddThese_Pass)
        {
            // add this elem so we will have a valid thing to make positive test for
            b &= assertTrue(maps.mapcycleAdd(sMapCheck), "extend");
        }

        const std::string sFirstMapName = maps.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");

        const auto nOriginalSizeMapCycle = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = maps.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();

        if (b)
        {
            const std::vector<std::string> vAddRemoveThese_Fail = {
               ""
            };
            // should fail because map is empty
            b &= assertFalse(maps.mapcycleRemove_availableMapsAdd(vAddRemoveThese_Fail), "add neg 1");
            b &= assertEquals(nOriginalSizeMapCycle, maps.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(nOriginalSizeAvailableMaps, maps.availableMapsGet().size(), "size available maps 2");

            const std::vector<std::string> vAddRemoveThese_Fail2 = {
               "map_test_good.txt"
            };
            // should fail because map is NOT in mapcycle
            b &= assertFalse(maps.mapcycleRemove_availableMapsAdd(vAddRemoveThese_Fail2), "add neg 2");
            b &= assertEquals(nOriginalSizeMapCycle, maps.mapcycleGet().size(), "size mapcycle 3");
            b &= assertEquals(nOriginalSizeAvailableMaps, maps.availableMapsGet().size(), "size available maps 3");

            b &= assertTrue(maps.mapcycleRemove_availableMapsAdd(vRemoveAddThese_Pass), "add 1");
            b &= assertEquals(nOriginalSizeMapCycle - vRemoveAddThese_Pass.size(), maps.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps + vRemoveAddThese_Pass.size(), maps.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(sFirstMapName, maps.mapcycleGetCurrent(), "rewind to first");
            for (const auto& sMapCheck : vRemoveAddThese_Pass)
            {
                b &= assertFalse(
                    std::find(maps.availableMapsGet().begin(), maps.availableMapsGet().end(), sMapCheck) == maps.availableMapsGet().end(),
                    "cannot find deleted item in available maps");
                b &= assertTrue(
                    std::find(maps.mapcycleGet().begin(), maps.mapcycleGet().end(), sMapCheck) == maps.mapcycleGet().end(),
                    "can find deleted item in mapcycle");
            }
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_map_mapcycle_remove_available_maps_add_all()
    {
        TestableMaps maps(m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");

        const auto nOriginalSizeMapCycle = maps.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        maps.mapcycleForwardToLast();
        b &= assertTrue(maps.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = maps.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = maps.availableMapsNoChangingGet().size();

        if (b)
        {
            b &= assertTrue(maps.mapcycleRemove_availableMapsAdd(), "add");
            b &= assertEquals(0u, maps.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps + nOriginalSizeMapCycle, maps.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, maps.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals("", maps.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.mapcycleGet(), maps.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(maps.availableMapsGet(), maps.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

}; 
