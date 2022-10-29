#pragma once

/*
    ###################################################################################
    MapItemTest.h
    Unit test for PRooFPS-dd MapItem.
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../../../../PGE/PGE/UnitTests/UnitTest.h"
#include "../MapItem.h"

class MapItemTest :
    public UnitTest
{
public:

    MapItemTest() :
        UnitTest(__FILE__)
    {}

protected:

    virtual void Initialize()
    {
        //CConsole::getConsoleInstance().SetLoggingState(PRRETexture::getLoggerModuleName(), true);
        //CConsole::getConsoleInstance().SetLoggingState(PRRETextureManager::getLoggerModuleName(), true);

        engine = &PR00FsReducedRenderingEngine::createAndGet();
        engine->initialize(PRRE_RENDERER_HW_FP, 800, 600, PRRE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        AddSubTest("test_initially_empty", (PFNUNITSUBTEST)&MapItemTest::test_initially_empty);
        AddSubTest("test_take", (PFNUNITSUBTEST)&MapItemTest::test_take);
        AddSubTest("test_update", (PFNUNITSUBTEST)&MapItemTest::test_update);
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
        if (engine)
        {
            engine->shutdown();
            engine = NULL;
        }

        CConsole::getConsoleInstance().SetLoggingState(PRRETexture::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(PRRETextureManager::getLoggerModuleName(), false);
    }

private:

    PR00FsReducedRenderingEngine* engine;

    // ---------------------------------------------------------------------------

    MapItemTest(const MapItemTest&)
    {};

    MapItemTest& operator=(const MapItemTest&)
    {
        return *this;
    };

    bool test_initially_empty()
    {
        MapItem mi(*engine, MapItemType::ITEM_WPN_MACHINEGUN, PRREVector(1, 2, 3));

        return assertEquals(static_cast<int>(MapItemType::ITEM_WPN_MACHINEGUN), static_cast<int>(mi.getType()), "type") &
            assertEquals(PRREVector(1, 2, 3), mi.getPos(), "pos") &
            assertEquals(mi.getPos(), mi.getObject3D().getPosVec(), "obj pos") &
            assertTrue(mi.getObject3D().isRenderingAllowed(), "visible") &
            assertFalse(mi.isTaken(), "taken") &
            assertEquals(0, mi.getTimeTaken().time_since_epoch().count(), "time taken");
    }

    bool test_take()
    {
        MapItem mi(*engine, MapItemType::ITEM_WPN_MACHINEGUN, PRREVector(1, 2, 3));
        mi.Take();

        return assertTrue(mi.isTaken(), "taken") &
            assertLess(0, mi.getTimeTaken().time_since_epoch().count(), "time taken") &
            assertFalse(mi.getObject3D().isRenderingAllowed(), "not visible");
    }

    bool test_update()
    {
        MapItem mi(*engine, MapItemType::ITEM_WPN_MACHINEGUN, PRREVector(1, 2, 3));
        const float fOriginalObjPosY = mi.getPos().getY();
        mi.Update(4.f);

        return assertNotEquals(fOriginalObjPosY, mi.getPos().getY());
    }

};