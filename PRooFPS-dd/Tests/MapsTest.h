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

#include "Maps.h"
#include "MapTestsCommon.h"

class TestableMaps :
    public proofps_dd::Maps
{
public:
    TestableMaps(
        pge_audio::PgeAudio& audio,
        PGEcfgProfiles& cfgProfiles,
        PR00FsUltimateRenderingEngine& gfx) : Maps(audio, cfgProfiles, gfx)
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
        m_audio(cfgProfiles),
        m_cfgProfiles(cfgProfiles)
    {
        engine = NULL;
    }

    MapsTest(const MapsTest&) = delete;
    MapsTest& operator=(const MapsTest&) = delete;
    MapsTest(MapsTest&&) = delete;
    MapsTest& operator=(MapsTest&&) = delete;

protected:

    virtual void initialize() override
    {
        //CConsole::getConsoleInstance().SetLoggingState(PureTexture::getLoggerModuleName(), true);
        //CConsole::getConsoleInstance().SetLoggingState(PureTextureManager::getLoggerModuleName(), true);
        //CConsole::getConsoleInstance().SetLoggingState(proofps_dd::Maps::getLoggerModuleName(), true);

        PGEInputHandler& inputHandler = PGEInputHandler::createAndGet(m_cfgProfiles);
        
        engine = &PR00FsUltimateRenderingEngine::createAndGet(m_cfgProfiles, inputHandler);
        engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        m_cbDisplayMapLoadingProgressUpdate = [this](int nProgress)
        {
            // no matter which test case we are in, whenever maps.load() invokes this cb, there should be a non-negative progress
            assertLequals(0, nProgress);
        };

        addSubTest("test_initially_empty", (PFNUNITSUBTEST) &MapsTest::test_initially_empty);
        addSubTest("test_map_load_bad_filename", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_filename);
        addSubTest("test_map_load_bad_assignment", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_assignment);
        addSubTest("test_map_load_bad_order", (PFNUNITSUBTEST) &MapsTest::test_map_load_bad_order);
        addSubTest("test_map_load_bad_jumppad_count", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_jumppad_count);
        addSubTest("test_map_load_bad_jumppad_force_value", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_jumppad_force_value);
        addSubTest("test_map_load_bad_spawn_group_contains_all_indices", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_spawn_group_contains_all_indices);
        addSubTest("test_map_load_bad_spawn_group_double_spawn_index", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_spawn_group_double_spawn_index);
        addSubTest("test_map_load_bad_spawn_group_double_spawn_index_2", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_spawn_group_double_spawn_index_2);
        addSubTest("test_map_load_bad_spawn_group_spawn_index", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_spawn_group_spawn_index);
        addSubTest("test_map_load_bad_spawn_group_spawn_word_as_index", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_spawn_group_spawn_word_as_index);
        addSubTest("test_map_load_bad_first_block_in_line_cannot_be_stairs", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_first_block_in_line_cannot_be_stairs);
        addSubTest("test_map_load_bad_last_block_in_line_cannot_be_stairs", (PFNUNITSUBTEST)&MapsTest::test_map_load_bad_last_block_in_line_cannot_be_stairs);
        addSubTest("test_map_load_good", (PFNUNITSUBTEST) &MapsTest::test_map_load_good);
        addSubTest("test_map_unload_and_load_again", (PFNUNITSUBTEST) &MapsTest::test_map_unload_and_load_again);
        addSubTest("test_map_shutdown", (PFNUNITSUBTEST)&MapsTest::test_map_shutdown);
        addSubTest("test_map_server_decide_first_map_to_be_loaded", (PFNUNITSUBTEST)&MapsTest::test_map_server_decide_first_map_to_be_loaded);
        addSubTest("test_map_get_random_spawnpoint_no_teamgame", (PFNUNITSUBTEST) &MapsTest::test_map_get_random_spawnpoint_no_teamgame);
        addSubTest("test_map_get_team_spawnpoints", (PFNUNITSUBTEST)&MapsTest::test_map_get_team_spawnpoints);
        addSubTest("test_map_are_team_spawnpoints_defined", (PFNUNITSUBTEST)&MapsTest::test_map_are_team_spawnpoints_defined);
        addSubTest("test_map_can_use_team_spawnpoints", (PFNUNITSUBTEST)&MapsTest::test_map_can_use_team_spawnpoints);
        addSubTest("test_map_get_random_spawnpoint_teamgame", (PFNUNITSUBTEST)&MapsTest::test_map_get_random_spawnpoint_teamgame);
        addSubTest("test_map_get_random_spawnpoint_teamgame_does_not_select_unassigned", (PFNUNITSUBTEST)&MapsTest::test_map_get_random_spawnpoint_teamgame_does_not_select_unassigned);
        addSubTest("test_map_load_without_team_spawnpoints_good", (PFNUNITSUBTEST)&MapsTest::test_map_load_without_team_spawnpoints_good);
        addSubTest("test_map_get_leftmost_spawnpoint", (PFNUNITSUBTEST)&MapsTest::test_map_get_leftmost_spawnpoint);
        addSubTest("test_map_get_rightmost_spawnpoint", (PFNUNITSUBTEST)&MapsTest::test_map_get_rightmost_spawnpoint);
        addSubTest("test_map_update", (PFNUNITSUBTEST)&MapsTest::test_map_update);
        addSubTest("test_map_handle_map_item_update_from_server", (PFNUNITSUBTEST)&MapsTest::test_map_handle_map_item_update_from_server);
    }

    virtual bool setUp() override
    {
        m_cfgProfiles.getVars()[proofps_dd::Maps::szCVarSvMapCollisionBvhMaxDepth].Set(4); // otherwise Maps::initialize() will fail on value 0
        return assertTrue(engine && engine->isInitialized());
    }

    virtual void tearDown() override
    {
        m_cfgProfiles.getVars().clear();
    }

    virtual void finalize() override
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

    static const unsigned int MAP_TEST_W = 52u;
    static const unsigned int MAP_TEST_H = 10u;

    pge_audio::PgeAudio m_audio;  // we just use it uninitialized, dont deal with sounds in unit tests
    PGEcfgProfiles& m_cfgProfiles;
    PR00FsUltimateRenderingEngine* engine;
    std::function<void(int)> m_cbDisplayMapLoadingProgressUpdate;

    // ---------------------------------------------------------------------------

    bool test_map_has_default_values(proofps_dd::Maps& maps)
    {
        bool b = true;
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
        b &= assertEquals(PureOctree::NodeType::LeafEmpty, maps.getBVH().getNodeType(), "bvh empty");
        b &= assertEquals(maps.getBVH().getPos(), maps.getBVH().getAABB().getPosVec(), "bvh aabb pos");
        b &= assertEquals(
            PureVector(
                maps.getBVH().getSize(),
                maps.getBVH().getSize(),
                maps.getBVH().getSize()),
            maps.getBVH().getAABB().getSizeVec(), "bvh aabb size");

        // variables
        b &= assertTrue(maps.getVars().empty(), "getVars");
        b &= assertEquals(0u, maps.getSpawnpoints().size(), "spawnpoints");

        // items
        b &= assertEquals(0u, maps.getItems().size(), "item count");
        b &= assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id");

        // spawn points
        assertTrue(maps.getSpawnpoints().empty(), "spawn points");
        try {
            const PureVector sp = maps.getRandomSpawnpoint(false);
            b = assertTrue(false, "ex 1"); // shall not reach this
        }
        catch (const std::exception& e) { b &= assertEquals(std::string("No spawnpoints!"), e.what()); }

        try {
            b &= assertTrue(maps.getTeamSpawnpoints(1).empty(), "team 1 spawn points");
            b = assertTrue(false, "ex 2"); // shall not reach this
        }
        catch (const std::exception& e) { b &= assertEquals(std::string("No spawnpoints!"), e.what()); }

        try {
            b &= assertTrue(maps.getTeamSpawnpoints(2).empty(), "team 2 spawn points");
            b = assertTrue(false, "ex 3"); // shall not reach this
        }
        catch (const std::exception& e) { b &= assertEquals(std::string("No spawnpoints!"), e.what()); }

        b &= assertFalse(maps.areTeamSpawnpointsDefined(), "are team spawn points defined");

        m_cfgProfiles.getVars()[proofps_dd::Maps::szCVarSvMapTeamSpawnGroups].Set(true);
        b &= assertFalse(maps.canUseTeamSpawnpoints(true, 1), "can use team 1 spawn points");
        b &= assertFalse(maps.canUseTeamSpawnpoints(true, 2), "can use team 2 spawn points");

        // decals
        b &= assertTrue(maps.getDecals().empty(), "decal count");

        // jump pads
        b &= assertTrue(maps.getJumppads().empty(), "jumppad count");
        b &= assertEquals(0u, maps.getJumppadValidVarsCount(), "jumppad vars count");
        try {
            b &= assertEquals(1.f, maps.getJumppadForceFactors(0).y, "jumppad force factor");
            b &= false; // should not come here
        }
        catch (const std::exception&) { /* should throw, we are good here */ }

        return b;
    }

    bool test_initially_empty()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = (assertFalse(maps.isInitialized(), "inited 1") &
            assertTrue(test_map_has_default_values(maps), "def values 1") &
            assertTrue(maps.getMapcycle().mapcycleGet().empty(), "mapcycle empty 1") &
            assertNull(maps.getMapcycle().mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 1") &
            assertTrue(maps.getMapcycle().availableMapsGet().empty(), "available maps empty 1") &
            assertNull(maps.getMapcycle().availableMapsGetAsCharPtrArray(), "available maps charptrarray 1") &
            assertTrue(maps.getMapcycle().availableMapsNoChangingGet().empty(), "available maps no changing empty 1")) != 0;
        
        b &= assertTrue(maps.initialize(), "init");
        b &= (assertTrue(maps.isInitialized(), "inited 2") &
            assertTrue(test_map_has_default_values(maps), "def values 2") &
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
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("egsdghsdghsdghdsghgds.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_assignment()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_assignment.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_order()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_order.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_jumppad_count()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_jumppad_count.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_jumppad_force_value()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_jumppad_force_value.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_spawn_group_contains_all_indices()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_spawn_group_contains_all_indices.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_spawn_group_double_spawn_index()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_spawn_group_double_spawn_index.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_spawn_group_double_spawn_index_2()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_spawn_group_double_spawn_index_2.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_spawn_group_spawn_index()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_spawn_group_spawn_index.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_spawn_group_spawn_word_as_index()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_spawn_group_spawn_word_as_index.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_first_block_in_line_cannot_be_stairs()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_first_block_in_line_cannot_be_stairs.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_bad_last_block_in_line_cannot_be_stairs()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.load("map_test_bad_last_block_in_line_cannot_be_stairs.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= test_map_has_default_values(maps);

        return b;
    }

    bool test_map_load_good()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
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
        b &= assertEquals(PureOctree::NodeType::Parent, maps.getBVH().getNodeType(), "bvh not empty");
        b &= assertNotEquals(PureVector(), maps.getBVH().getAABB().getPosVec(), "bvh aabb pos");
        b &= assertNotEquals(PureVector(), maps.getBVH().getAABB().getSizeVec(), "bvh aabb size");
        
        // variables
        b &= assertEquals(5u, maps.getVars().size(), "getVars");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 2a");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 2 ex"); }

        // items
        b &= assertEquals(11u, maps.getItems().size(), "item count");
        b &= assertEquals(11u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id");
        if (b)
        {
            auto it = maps.getItems().begin();
            b &= assertNotNull(it->second, "item 0") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_MACHINEPISTOL, it->second->getType(), "item 0 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 0 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 0 tex");
            }

            it++;
            b &= assertNotNull(it->second, "item 1") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_BAZOOKA, it->second->getType(), "item 1 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 1 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 1 tex");
            }

            it++;
            b &= assertNotNull(it->second, "item 2") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_PISTOL, it->second->getType(), "item 2 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 2 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 2 tex");
            }

            it++;
            b &= assertNotNull(it->second, "item 3") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, it->second->getType(), "item 3 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 3 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 3 tex");
            }
            
            it++;
            b &= assertNotNull(it->second, "item 4") &&
                assertEquals(proofps_dd::MapItemType::ITEM_HEALTH, it->second->getType(), "item 4 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 4 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 4 tex");
            }

            it++;
            b &= assertNotNull(it->second, "item 5") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_PUSHA, it->second->getType(), "item 5 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 5 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 5 tex");
            }

            it++;
            b &= assertNotNull(it->second, "item 6") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_GRENADELAUNCHER, it->second->getType(), "item 6 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 6 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 6 tex");
            }

            it++;
            b &= assertNotNull(it->second, "item 7") &&
                assertEquals(proofps_dd::MapItemType::ITEM_ARMOR, it->second->getType(), "item 7 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 7 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 7 tex");
            }

            it++;
            b &= assertNotNull(it->second, "item 8") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_SHOTGUN, it->second->getType(), "item 8 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 8 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 8 tex");
            }
            
            it++;
            b &= assertNotNull(it->second, "item 9") &&
                assertEquals(proofps_dd::MapItemType::ITEM_WPN_PISTOL, it->second->getType(), "item 9 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 9 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 9 tex");
            }
            
            it++;
            b &= assertNotNull(it->second, "item 10") &&
                assertEquals(proofps_dd::MapItemType::ITEM_HEALTH, it->second->getType(), "item 10 type");
            b &= assertNotNull(it->second->getObject3D().getReferredObject(), "item 10 referred obj");
            if (b)
            {
                b &= assertNotNull(it->second->getObject3D().getReferredObject()->getMaterial().getTexture(), "item 10 tex");
            }
        }

        // decals
        b &= assertEquals(2u, maps.getDecals().size(), "decal count");

        // jump pads
        b &= assertEquals(3u, maps.getJumppads().size(), "jumppad count");
        b &= assertEquals(3u, maps.getJumppadValidVarsCount(), "jumppad vars count");
        try {
            b &= assertEquals( 0.f, maps.getJumppadForceFactors(0).x, "jumppad force factor x a 1");
            b &= assertEquals( 1.f, maps.getJumppadForceFactors(0).y, "jumppad force factor y a 1");
            b &= assertEquals( 2.4f, maps.getJumppadForceFactors(1).x, "jumppad force factor x b 1");
            b &= assertEquals( 2.f, maps.getJumppadForceFactors(1).y, "jumppad force factor y b 1");
            b &= assertEquals(-3.3f, maps.getJumppadForceFactors(2).x, "jumppad force factor x c 1");
            b &= assertEquals( 2.2f, maps.getJumppadForceFactors(2).y, "jumppad force factor y c 1");
        }
        catch (const std::exception&) { b = false; /* should not come here */ }

        // almost all visible block objects are just clones of reference objects,
        // exception: stairsteps are unique objects.
        constexpr size_t nStairBlocks = 8; // update this value based on map_test_good layout!
        constexpr size_t nStairsteps = nStairBlocks * proofps_dd::Maps::nStairstepsCount;
        size_t nClonedObjects = 0;
        for (int i = 0; i < maps.getBlockCount(); i++)
        {
            if ((maps.getBlocks()[i])->getReferredObject())
            {
                nClonedObjects++;
            }
        }
        b &= assertEquals(nStairsteps, maps.getBlockCount() - nClonedObjects, "number of cloned objects");

        return b;
    }

    bool test_map_unload_and_load_again()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);

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
        b &= assertEquals(PureOctree::NodeType::Parent, maps.getBVH().getNodeType(), "bvh not empty");
        b &= assertNotEquals(PureVector(), maps.getBVH().getAABB().getPosVec(), "bvh aabb pos");
        b &= assertNotEquals(PureVector(), maps.getBVH().getAABB().getSizeVec(), "bvh aabb size");

        // variables
        b &= assertEquals(5u, maps.getVars().size(), "getVars 1");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints 1");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 1a");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 1 ex"); }

        // items
        b &= assertEquals(11u, maps.getItems().size(), "item count 1");
        b &= assertEquals(11u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 1");

        // decals
        b &= assertEquals(2u, maps.getDecals().size(), "decal count 1");

        // jump pads
        b &= assertEquals(3u, maps.getJumppads().size(), "jumppad count 1");
        b &= assertEquals(3u, maps.getJumppadValidVarsCount(), "jumppad vars count 1");
        try {
            b &= assertEquals(0.f, maps.getJumppadForceFactors(0).x, "jumppad force factor x a 1");
            b &= assertEquals(1.f, maps.getJumppadForceFactors(0).y, "jumppad force factor y a 1");
            b &= assertEquals(2.4f, maps.getJumppadForceFactors(1).x, "jumppad force factor x b 1");
            b &= assertEquals(2.f, maps.getJumppadForceFactors(1).y, "jumppad force factor y b 1");
            b &= assertEquals(-3.3f, maps.getJumppadForceFactors(2).x, "jumppad force factor x c 1");
            b &= assertEquals(2.2f, maps.getJumppadForceFactors(2).y, "jumppad force factor y c 1");
        }
        catch (const std::exception&) { b = false; /* should not come here */ }
        
        // ################################# TRY LOAD AGAIN ###################################
        b &= assertFalse(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load 2");

        // ###################################### UNLOAD ######################################
        maps.unload();
        b &= assertTrue(test_map_has_default_values(maps), "def values after unload");

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
        b &= assertEquals(PureOctree::NodeType::Parent, maps.getBVH().getNodeType(), "bvh not empty 3");
        b &= assertNotEquals(PureVector(), maps.getBVH().getAABB().getPosVec(), "bvh aabb pos 3");
        b &= assertNotEquals(PureVector(), maps.getBVH().getAABB().getSizeVec(), "bvh aabb size 3");

        // variables
        b &= assertEquals(5u, maps.getVars().size(), "getVars 3");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints 3");
        try {
            b &= assertEquals("Test Map", maps.getVars().at("Name").getAsString(), "getVars 3a");
        }
        catch (const std::exception&) { b = assertTrue(false, "getVars 3 ex"); }

        // items
        b &= assertEquals(11u, maps.getItems().size(), "item count 3");
        b &= assertEquals(11u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id 3");

        // decals
        b &= assertEquals(2u, maps.getDecals().size(), "decal count 3");

        // jump pads
        b &= assertEquals(3u, maps.getJumppads().size(), "jumppad count 3");
        b &= assertEquals(3u, maps.getJumppadValidVarsCount(), "jumppad vars count 3");
        try {
            b &= assertEquals(0.f, maps.getJumppadForceFactors(0).x, "jumppad force factor x a 1");
            b &= assertEquals(1.f, maps.getJumppadForceFactors(0).y, "jumppad force factor y a 1");
            b &= assertEquals(2.4f, maps.getJumppadForceFactors(1).x, "jumppad force factor x b 1");
            b &= assertEquals(2.f, maps.getJumppadForceFactors(1).y, "jumppad force factor y b 1");
            b &= assertEquals(-3.3f, maps.getJumppadForceFactors(2).x, "jumppad force factor x c 1");
            b &= assertEquals(2.2f, maps.getJumppadForceFactors(2).y, "jumppad force factor y c 1");
        }
        catch (const std::exception&) { b = false; /* should not come here */ }

        return b;
    }

    bool test_map_shutdown()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
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

        b &= assertTrue(test_map_has_default_values(maps), "def values");
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
        TestableMaps maps(m_audio, m_cfgProfiles, *engine);
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

    bool test_map_get_random_spawnpoint_no_teamgame()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = true;

        try {
            const PureVector sp = maps.getRandomSpawnpoint(false);
            b = assertTrue(false, "ex 1"); // shall not reach this
        }
        catch (const std::exception& e) { b &= assertEquals(std::string("No spawnpoints!"), e.what()); }

        b &= assertTrue(maps.initialize(), "init");
        try {
            const PureVector sp = maps.getRandomSpawnpoint(false);
            b = assertTrue(false, "ex 2"); // shall not reach this
        }
        catch (const std::exception& e) { b &= assertEquals(std::string("No spawnpoints!"), e.what()); }

        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");
        
        if ( b )
        {
            std::set<PureVector> originalSpawnpoints(maps.getSpawnpoints().begin(), maps.getSpawnpoints().end());
            int i = 0;
            try {
                while ( !originalSpawnpoints.empty() && (i < 50) )
                {
                    const PureVector sp = maps.getRandomSpawnpoint(false);
                    originalSpawnpoints.erase(sp);
                    i++;
                }
                b = assertTrue(originalSpawnpoints.empty(), "original empty");
            }
            catch (const std::exception& e) { b = assertTrue(false, e.what()); }
        }

        return b;
    }

    bool test_map_get_team_spawnpoints()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = true;

        try {
            b &= assertTrue(maps.getTeamSpawnpoints(0).empty(), "ex 1");
            b = false; // shall not reach this
        }
        catch (const std::exception& e) { b &= assertEquals(std::string("No spawnpoints!"), e.what()); }

        b &= assertTrue(maps.initialize(), "init");
        try {
            b &= assertTrue(maps.getTeamSpawnpoints(0).empty(), "ex 2");
            b = false; // shall not reach this
        }
        catch (const std::exception& e) { b &= assertEquals(std::string("No spawnpoints!"), e.what()); }
        
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertFalse(maps.getSpawnpoints().empty(), "spawnpoints");

        if (b)
        {
            try {
                b &= assertTrue(maps.getTeamSpawnpoints(0).empty(), "ex 3");
                b = false; // shall not reach this
            }
            catch (const std::exception& e) { b &= assertEquals(std::string("Invalid Team Id!"), e.what()); }

            try {
                b &= assertTrue(maps.getTeamSpawnpoints(3).empty(), "ex 4");
                b = false; // shall not reach this
            }
            catch (const std::exception& e) { b &= assertEquals(std::string("Invalid Team Id!"), e.what()); }

            try {
                b &= assertEquals(1u, maps.getTeamSpawnpoints(1).size(), "group size 1");
                b &= assertEquals(2u, maps.getTeamSpawnpoints(2).size(), "group size 2");
                for (const auto& isp : maps.getTeamSpawnpoints(1))
                {
                    b &= assertLess(isp, maps.getSpawnpoints().size(), (std::string("group 1 index: ") + std::to_string(isp)).c_str());
                }
                for (const auto& isp : maps.getTeamSpawnpoints(2))
                {
                    b &= assertLess(isp, maps.getSpawnpoints().size(), (std::string("group 2 index: ") + std::to_string(isp)).c_str());
                    b &= assertTrue(
                        maps.getTeamSpawnpoints(1).find(isp) == maps.getTeamSpawnpoints(1).end(),
                        (std::string("group 2 index found in group 1: ") + std::to_string(isp)).c_str());
                }
            }
            catch (const std::exception& e) { b = assertTrue(false, e.what()); }
        }

        return b;
    }

    bool test_map_are_team_spawnpoints_defined()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = true;
        b &= assertFalse(maps.areTeamSpawnpointsDefined(), "1");
        b &= assertTrue(maps.initialize(), "init");
        b &= assertFalse(maps.areTeamSpawnpointsDefined(), "2");
        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        b &= assertTrue(maps.areTeamSpawnpointsDefined(), "3");

        return b;
    }

    bool test_map_can_use_team_spawnpoints()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = true;
        b &= assertFalse(maps.canUseTeamSpawnpoints(true, 1), "0");
        
        b &= assertTrue(maps.initialize(), "init") &
            m_cfgProfiles.getVars()[proofps_dd::Maps::szCVarSvMapTeamSpawnGroups].getAsString().empty();

        b &= assertFalse(maps.canUseTeamSpawnpoints(true, 1), "1");

        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        if (!b)
        {
            return false;
        }

        b &= assertFalse(maps.canUseTeamSpawnpoints(true, 1), "2");

        m_cfgProfiles.getVars()[proofps_dd::Maps::szCVarSvMapTeamSpawnGroups].Set(true);
        b &= assertTrue(maps.canUseTeamSpawnpoints(true, 1), "3");
        b &= assertTrue(maps.canUseTeamSpawnpoints(true, 2), "4");

        b &= assertFalse(maps.canUseTeamSpawnpoints(false, 1), "5");
        b &= assertFalse(maps.canUseTeamSpawnpoints(true, 0), "6");
        
        maps.unload();
        b &= assertFalse(maps.canUseTeamSpawnpoints(true, 1), "7");

        return b;
    }

    bool test_map_get_random_spawnpoint_teamgame()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = true;

        try {
            const PureVector sp = maps.getRandomSpawnpoint(true, 1);
            b = assertTrue(false, "ex 1"); // shall not reach this
        }
        catch (const std::exception& e) { b &= assertEquals(std::string("No spawnpoints!"), e.what()); }

        b &= assertTrue(maps.initialize(), "init");
        try {
            const PureVector sp = maps.getRandomSpawnpoint(true, 1);
            b = assertTrue(false, "ex 2"); // shall not reach this
        }
        catch (const std::exception& e) { b &= assertEquals(std::string("No spawnpoints!"), e.what()); }

        b &= assertTrue(maps.load("map_test_good.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        if (!b)
        {
            return false;
        }

        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");

        m_cfgProfiles.getVars()[proofps_dd::Maps::szCVarSvMapTeamSpawnGroups].Set(true);
        b &= assertTrue(maps.canUseTeamSpawnpoints(true, 1), "can use 1");
        b &= assertTrue(maps.canUseTeamSpawnpoints(true, 2), "can use 2");

        if (b)
        {
            std::set<PureVector> originalSpawnpoints(maps.getSpawnpoints().begin(), maps.getSpawnpoints().end());
            try {
                int i = 0;
                while (i < 20)
                {
                    PureVector sp = maps.getRandomSpawnpoint(true, 1);
                    originalSpawnpoints.erase(sp);
                    i++;
                }
                b &= assertFalse(originalSpawnpoints.empty(), "original empty 1");

                i = 0;
                while (i < 20)
                {
                    PureVector sp = maps.getRandomSpawnpoint(true, 2);
                    originalSpawnpoints.erase(sp);
                    i++;
                }
                b &= assertTrue(originalSpawnpoints.empty(), "original empty 2");
            }
            catch (const std::exception& e) { b = assertTrue(false, e.what()); }
        }

        return b;
    }

    bool test_map_get_random_spawnpoint_teamgame_does_not_select_unassigned()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("map_test_good_with_unassigned_sp.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");

        if (!b)
        {
            return false;
        }

        b &= assertEquals("map_test_good_with_unassigned_sp.txt", maps.getNextMapToBeLoaded(), "getNextMapToBeLoaded");
        b &= assertEquals("map_test_good_with_unassigned_sp.txt", maps.getFilename(), "filename");

        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");

        m_cfgProfiles.getVars()[proofps_dd::Maps::szCVarSvMapTeamSpawnGroups].Set(true);

        b &= assertTrue(maps.areTeamSpawnpointsDefined(), "1");
        b &= assertEquals(1u, maps.getTeamSpawnpoints(1).size(), "2");
        b &= assertEquals(1u, maps.getTeamSpawnpoints(2).size(), "3");

        b &= assertTrue(maps.canUseTeamSpawnpoints(true, 1), "4");
        b &= assertTrue(maps.canUseTeamSpawnpoints(true, 2), "5");

        if (b)
        {
            std::set<PureVector> originalSpawnpoints(maps.getSpawnpoints().begin(), maps.getSpawnpoints().end());
            try {
                for (unsigned iTeamId = 1; iTeamId <= 2; iTeamId++)
                {
                    for (int i = 0; i < 50; i++)
                    {
                        PureVector sp = maps.getRandomSpawnpoint(true, iTeamId);
                        originalSpawnpoints.erase(sp);
                    }
                }

                b &= assertEquals(1u, originalSpawnpoints.size(), "original size");
                // in this test map, the last spawn point is the unassigned one, which
                // is the same spawn point that was never selected in the loops above thus the only 1 remained in originalSpawnpoints
                b &= assertEquals(*(--maps.getSpawnpoints().end()), *originalSpawnpoints.begin(), "the only unassigned sp left");
            }
            catch (const std::exception& e) { b = assertTrue(false, e.what()); }
        }

        return b;
    }

    bool test_map_load_without_team_spawnpoints_good()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
        bool b = assertTrue(maps.initialize(), "init");
        b &= assertTrue(maps.load("map_test_good_no_spawn_group.txt", m_cbDisplayMapLoadingProgressUpdate), "load");
        b &= assertTrue(maps.loaded(), "loaded");
        if (!b)
        {
            return false;
        }

        b &= assertEquals("map_test_good_no_spawn_group.txt", maps.getNextMapToBeLoaded(), "getNextMapToBeLoaded");
        b &= assertEquals("map_test_good_no_spawn_group.txt", maps.getFilename(), "filename");

        b &= assertEquals(3u, maps.getSpawnpoints().size(), "spawnpoints");
        
        m_cfgProfiles.getVars()[proofps_dd::Maps::szCVarSvMapTeamSpawnGroups].Set(true);

        b &= assertFalse(maps.areTeamSpawnpointsDefined(), "1");
        b &= assertTrue(maps.getTeamSpawnpoints(1).empty(), "2");
        b &= assertTrue(maps.getTeamSpawnpoints(2).empty(), "3");
        
        b &= assertFalse(maps.canUseTeamSpawnpoints(true, 1), "4");
        b &= assertFalse(maps.canUseTeamSpawnpoints(true, 2), "5");

        return b;
    }

    bool test_map_get_leftmost_spawnpoint()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
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
                b &= assertEquals(spExpected, spChecked, "vec");
            }
            catch (const std::exception& e) { b = assertTrue(false, e.what()); }
        }

        return b;
    }

    bool test_map_get_rightmost_spawnpoint()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
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
                b &= assertEquals(spExpected, spChecked, "vec");
            }
            catch (const std::exception& e) { b = assertTrue(false, e.what()); }
        }

        return b;
    }

    bool test_map_update()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
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

            PureObject3D* const pDummyPlayerObject = engine->getObject3DManager().createBox(1.f, 1.f, 1.f);
            if (!pDummyPlayerObject)
            {
                return assertFalse(true, "pDummyPlayerObject");
            }
            maps.update(60.f, *pDummyPlayerObject);

            int i = 0;
            for (const auto& itemPair : maps.getItems())
            {
                assertNotEquals(vOriginalItemPosY[i], itemPair.second->getPos().getY(), ("item " + std::to_string(i) + " pos y").c_str());
                i++;
            }

            // we could also test if decorations alpha is changing, but I'm just skipping that test
        }

        return b;
    }

    bool test_map_handle_map_item_update_from_server()
    {
        proofps_dd::Maps maps(m_audio, m_cfgProfiles, *engine);
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
