#pragma once

/*
    ###################################################################################
    MapItemTest.h
    Unit test for PRooFPS-dd MapItem.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include "UnitTest.h"

#include "MapItem.h"

class MapItemTest :
    public UnitTest
{
public:

    MapItemTest(PGEcfgProfiles& cfgProfiles) :
        UnitTest(__FILE__),
        m_cfgProfiles(cfgProfiles),
        engine(nullptr)
    {}

    MapItemTest(const MapItemTest&) = delete;
    MapItemTest& operator=(const MapItemTest&) = delete;
    MapItemTest(MapItemTest&&) = delete;
    MapItemTest& operator=(MapItemTest&&) = delete;

protected:

    virtual void initialize() override
    {
        //CConsole::getConsoleInstance().SetLoggingState(PureTexture::getLoggerModuleName(), true);
        //CConsole::getConsoleInstance().SetLoggingState(PureTextureManager::getLoggerModuleName(), true);

        PGEInputHandler& inputHandler = PGEInputHandler::createAndGet(m_cfgProfiles);

        engine = &PR00FsUltimateRenderingEngine::createAndGet(m_cfgProfiles, inputHandler);
        engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        addSubTest("test_initially_empty", (PFNUNITSUBTEST)&MapItemTest::test_initially_empty);
        addSubTest("test_reset_global_item_id", (PFNUNITSUBTEST)&MapItemTest::test_reset_global_item_id);
        addSubTest("test_get_item_respawn_time_secs", (PFNUNITSUBTEST)&MapItemTest::test_get_item_respawn_time_secs);
        addSubTest("test_take", (PFNUNITSUBTEST)&MapItemTest::test_take);
        addSubTest("test_untake", (PFNUNITSUBTEST)&MapItemTest::test_untake);
        addSubTest("test_update", (PFNUNITSUBTEST)&MapItemTest::test_update);
    }

    virtual bool setUp() override
    {
        return assertTrue(engine && engine->isInitialized());
    }

    virtual void tearDown() override
    {
        proofps_dd::MapItem::resetGlobalData();
    }

    virtual void finalize() override
    {
        if (engine)
        {
            engine->shutdown();
            engine = NULL;
        }

        CConsole::getConsoleInstance().SetLoggingState(PureTexture::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(PureTextureManager::getLoggerModuleName(), false);
    }

private:

    PGEcfgProfiles& m_cfgProfiles;
    PR00FsUltimateRenderingEngine* engine;

    // ---------------------------------------------------------------------------

    bool test_initially_empty()
    {
        const proofps_dd::MapItem::MapItemId iLastMapItemId = proofps_dd::MapItem::getGlobalMapItemId();

        proofps_dd::MapItem mi(*engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));

        return (assertEquals(mi.getId(), iLastMapItemId, "item id") &
            assertEquals(proofps_dd::MapItem::getGlobalMapItemId(), iLastMapItemId + 1, "global item id") &
            assertEquals(static_cast<int>(proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN), static_cast<int>(mi.getType()), "type") &
            assertEquals(PureVector(1, 2, 3), mi.getPos(), "pos") &
            assertEquals(mi.getPos(), mi.getObject3D().getPosVec(), "obj pos") &
            assertTrue(mi.getObject3D().isRenderingAllowed(), "visible") &
            assertFalse(mi.isTaken(), "taken") &
            assertEquals(0, mi.getTimeTaken().time_since_epoch().count(), "time taken")) != 0;
    }

    bool test_reset_global_item_id()
    {
        proofps_dd::MapItem* mi = new proofps_dd::MapItem(*engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        delete mi;

        proofps_dd::MapItem::resetGlobalData();

        return assertEquals(0u, proofps_dd::MapItem::getGlobalMapItemId(), "global item id");
    }

    bool test_get_item_respawn_time_secs()
    {
        proofps_dd::MapItem miArmor(*engine, proofps_dd::MapItemType::ITEM_ARMOR, PureVector(1, 2, 3));
        proofps_dd::MapItem miHealth(*engine, proofps_dd::MapItemType::ITEM_HEALTH, PureVector(1, 2, 3));
        proofps_dd::MapItem miWpnPistol(*engine, proofps_dd::MapItemType::ITEM_WPN_PISTOL, PureVector(1, 2, 3));
        proofps_dd::MapItem miWpnMchGun(*engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        proofps_dd::MapItem miWpnBazooka(*engine, proofps_dd::MapItemType::ITEM_WPN_BAZOOKA, PureVector(1, 2, 3));
        proofps_dd::MapItem miWpnPusha(*engine, proofps_dd::MapItemType::ITEM_WPN_PUSHA, PureVector(1, 2, 3));
        
        return (assertEquals(proofps_dd::MapItem::ITEM_ARMOR_RESPAWN_SECS, proofps_dd::MapItem::getItemRespawnTimeSecs(miArmor), "armor") & 
            assertEquals(proofps_dd::MapItem::ITEM_HEALTH_RESPAWN_SECS, proofps_dd::MapItem::getItemRespawnTimeSecs(miHealth), "health") &
            assertEquals(proofps_dd::MapItem::ITEM_WPN_PISTOL_RESPAWN_SECS, proofps_dd::MapItem::getItemRespawnTimeSecs(miWpnPistol), "pistol") &
            assertEquals(proofps_dd::MapItem::ITEM_WPN_MACHINEGUN_RESPAWN_SECS, proofps_dd::MapItem::getItemRespawnTimeSecs(miWpnMchGun), "mchgun") &
            assertEquals(proofps_dd::MapItem::ITEM_WPN_BAZOOKA_RESPAWN_SECS, proofps_dd::MapItem::getItemRespawnTimeSecs(miWpnBazooka), "bazooka") &
            assertEquals(proofps_dd::MapItem::ITEM_WPN_PUSHA_RESPAWN_SECS, proofps_dd::MapItem::getItemRespawnTimeSecs(miWpnPusha), "pusha")) != 0;
    }

    bool test_take()
    {
        proofps_dd::MapItem mi(*engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        mi.take();

        return (assertTrue(mi.isTaken(), "taken") &
            assertLess(0, mi.getTimeTaken().time_since_epoch().count(), "time taken") &
            assertFalse(mi.getObject3D().isRenderingAllowed(), "not visible")) != 0;
    }

    bool test_untake()
    {
        proofps_dd::MapItem mi(*engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        mi.unTake();

        bool b = (assertFalse(mi.isTaken(), "not taken") &
            assertTrue(mi.getObject3D().isRenderingAllowed(), "not visible")) != 0;

        mi.take();
        mi.unTake();
        b &= (assertFalse(mi.isTaken(), "not taken") &
            assertTrue(mi.getObject3D().isRenderingAllowed(), "not visible")) != 0;

        return b;
    }

    bool test_update()
    {
        proofps_dd::MapItem mi(*engine, proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, PureVector(1, 2, 3));
        const float fOriginalObjPosY = mi.getPos().getY();
        mi.update(4.f);

        return assertNotEquals(fOriginalObjPosY, mi.getPos().getY());
    }

};
