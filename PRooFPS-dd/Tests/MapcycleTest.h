#pragma once

/*
    ###################################################################################
    MapcycleTest.h
    Unit test for PRooFPS-dd Mapcycle and Available Maps Handling.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "UnitTest.h"

#include "Mapcycle.h"
#include "PRooFPS-dd-packet.h"

#include "MapTestsCommon.h"

class TestableMapcycle :
    public proofps_dd::Mapcycle
{
public:
    TestableMapcycle()
    {};

    virtual ~TestableMapcycle() {};

    friend class MapcycleTest;
};

class MapcycleTest :
    public MapTestsCommon
{
public:

    MapcycleTest() :
        MapTestsCommon(__FILE__)
    {
    }

    MapcycleTest(const MapcycleTest&) = delete;
    MapcycleTest& operator=(const MapcycleTest&) = delete;
    MapcycleTest(MapcycleTest&&) = delete;
    MapcycleTest& operator=(MapcycleTest&&) = delete;

protected:

    virtual void initialize() override
    {
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::Mapcycle::getLoggerModuleName(), true);

        addSubTest("test_is_valid_map_filename", (PFNUNITSUBTEST)&MapcycleTest::test_is_valid_map_filename);

        addSubTest("test_initially_empty", (PFNUNITSUBTEST)&MapcycleTest::test_initially_empty);

        addSubTest("test_available_maps_get", (PFNUNITSUBTEST)&MapcycleTest::test_available_maps_get);
        addSubTest("test_available_maps_refresh", (PFNUNITSUBTEST)&MapcycleTest::test_available_maps_refresh);
        addSubTest("test_available_maps_add_single_elem", (PFNUNITSUBTEST)&MapcycleTest::test_available_maps_add_single_elem);
        addSubTest("test_available_maps_add_multi_elem", (PFNUNITSUBTEST)&MapcycleTest::test_available_maps_add_multi_elem);
        addSubTest("test_available_maps_remove_by_name", (PFNUNITSUBTEST)&MapcycleTest::test_available_maps_remove_by_name);
        addSubTest("test_available_maps_remove_by_index", (PFNUNITSUBTEST)&MapcycleTest::test_available_maps_remove_by_index);
        addSubTest("test_available_maps_remove_multi_elem", (PFNUNITSUBTEST)&MapcycleTest::test_available_maps_remove_multi_elem);

        addSubTest("test_mapcycle_reload", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_reload);
        addSubTest("test_mapcycle_save_to_file", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_save_to_file);
        addSubTest("test_mapcycle_next", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_next);
        addSubTest("test_mapcycle_rewind_to_first", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_rewind_to_first);
        addSubTest("test_mapcycle_forward_to_last", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_forward_to_last);
        addSubTest("test_mapcycle_add_single_elem", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_add_single_elem);
        addSubTest("test_mapcycle_add_multi_elem", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_add_multi_elem);
        addSubTest("test_mapcycle_remove_by_name", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_remove_by_name);
        addSubTest("test_mapcycle_remove_by_index", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_remove_by_index);
        addSubTest("test_mapcycle_remove_multi_elem", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_remove_multi_elem);
        addSubTest("test_mapcycle_clear", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_clear);
        addSubTest("test_mapcycle_remove_non_existing", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_remove_non_existing);

        addSubTest("test_mapcycle_available_maps_synchronize", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_available_maps_synchronize);
        addSubTest("test_mapcycle_add_available_maps_remove_by_name", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_add_available_maps_remove_by_name);
        addSubTest("test_mapcycle_add_available_maps_remove_multi_elem", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_add_available_maps_remove_multi_elem);
        addSubTest("test_mapcycle_add_available_maps_remove_all", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_add_available_maps_remove_all);
        addSubTest("test_mapcycle_remove_available_maps_add_by_name", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_remove_available_maps_add_by_name);
        addSubTest("test_mapcycle_remove_available_maps_add_by_index", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_remove_available_maps_add_by_index);
        addSubTest("test_mapcycle_remove_available_maps_add_multi_elem", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_remove_available_maps_add_multi_elem);
        addSubTest("test_mapcycle_remove_available_maps_add_all", (PFNUNITSUBTEST)&MapcycleTest::test_mapcycle_remove_available_maps_add_all);
    }

    virtual void finalize() override
    {
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::Mapcycle::getLoggerModuleName(), false);
    }

private:

    // ---------------------------------------------------------------------------

    bool test_is_valid_map_filename()
    {
        std::string sTooLooongFilename(proofps_dd::MsgMapChangeFromServer::nMapFilenameMaxLength, 'a');
        return (assertFalse(proofps_dd::Mapcycle::isValidMapFilename("map.txt"), "map.txt") &
            assertFalse(proofps_dd::Mapcycle::isValidMapFilename("map_.tx"), "map_.tx") &
            assertFalse(proofps_dd::Mapcycle::isValidMapFilename("_map.txt"), "_map.txt") &
            assertFalse(proofps_dd::Mapcycle::isValidMapFilename("mapcycle.txt"), "mapcycle.txt") &
            assertFalse(proofps_dd::Mapcycle::isValidMapFilename("mapcycle.txt"), "mapcycle.txt") &
            assertFalse(proofps_dd::Mapcycle::isValidMapFilename("map.txt"), "map.txt") &
            assertFalse(proofps_dd::Mapcycle::isValidMapFilename(sTooLooongFilename), "too long") &
            assertTrue(proofps_dd::Mapcycle::isValidMapFilename("map_.txt"), "ok")) != 0;
    }

    bool test_initially_empty()
    {
        proofps_dd::Mapcycle mapcycle;
        bool b = (assertFalse(mapcycle.isInitialized(), "inited 1") &
            assertTrue(mapcycle.mapcycleGet().empty(), "mapcycle empty 1") &
            assertNull(mapcycle.mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 1") &
            assertTrue(mapcycle.availableMapsGet().empty(), "available maps empty 1") &
            assertNull(mapcycle.availableMapsGetAsCharPtrArray(), "available maps charptrarray 1") &
            assertTrue(mapcycle.availableMapsNoChangingGet().empty(), "available maps no changing empty 1") &
            assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 1")) != 0;

        b &= assertTrue(mapcycle.initialize(), "init");
        b &= (assertTrue(mapcycle.isInitialized(), "inited 2") &
            assertFalse(mapcycle.mapcycleGet().empty(), "mapcycle empty 2") &
            assertNotNull(mapcycle.mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 2") &
            assertFalse(mapcycle.availableMapsGet().empty(), "available maps empty 2") &
            assertNotNull(mapcycle.availableMapsGetAsCharPtrArray(), "available maps charptrarray 2") &
            assertFalse(mapcycle.availableMapsNoChangingGet().empty(), "available maps no changing empty 2") &
            assertFalse(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2")) != 0;

        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
            "mapcycleGetAsCharPtrArray()");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
            "availableMapsGetAsCharPtrArray()");

        return b;
    }

    bool test_available_maps_get()
    {
        proofps_dd::Mapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");

        if (!b)
        {
            return false;
        }

        std::set<std::string> vExpectedAvailableMaps = {
            "map_concept01.txt",
            "map_test_bad_assignment.txt",
            "map_test_bad_jumppad_count.txt",
            "map_test_bad_jumppad_force_value.txt",
            "map_test_bad_order.txt",
            "map_test_good.txt"/*,
            "map_warena.txt",
            "map_warhouse.txt"*/ /* commented since Maps::inititalize() does mapcycle-available maps sync */
        };

        const auto& foundAvailableMaps = mapcycle.availableMapsGet();
        for (const auto& sMapName : foundAvailableMaps)
        {
            const auto itFound = vExpectedAvailableMaps.find(sMapName);
            /* mapcycle.txt must not be found, as we require map name to start with "map_" */
            b &= assertTrue(
                (itFound != vExpectedAvailableMaps.end() ||
                /* we allow some test maps so we dont have to update
                   vExpectedAvailableMaps every time we add a new test map */
                ((sMapName.find("map_test_") != std::string::npos) &&
                 (sMapName.find(".txt") != std::string::npos))), (std::string("Unexpected map found:") + sMapName).c_str());
            if (itFound != vExpectedAvailableMaps.end())
            {
                vExpectedAvailableMaps.erase(sMapName);
            }
        }
        b &= assertTrue(vExpectedAvailableMaps.empty(), "Not found all expected maps!");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
            "availableMapsGetAsCharPtrArray()");

        return b;
    }

    bool test_available_maps_refresh()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");

        if (!b)
        {
            return false;
        }

        // trick to clear out available maps, we can refresh them without initializing Maps actually ...
        mapcycle.shutdown();
        mapcycle.availableMapsRefresh();

        std::set<std::string> vExpectedAvailableMaps = {
            "map_concept01.txt",
            "map_mutans.txt",
            "map_test_bad_assignment.txt",
            "map_test_bad_jumppad_count.txt",
            "map_test_bad_jumppad_force_value.txt",
            "map_test_bad_order.txt",
            "map_test_good.txt",
            "map_warena.txt",
            "map_warhouse.txt",
            "map_construction.txt"
        };

        const auto& foundAvailableMaps = mapcycle.availableMapsGet();
        for (const auto& sMapName : foundAvailableMaps)
        {
            const auto itFound = vExpectedAvailableMaps.find(sMapName);
            /* mapcycle.txt must not be found, as we require map name to start with "map_" */
            b &= assertTrue(
                (itFound != vExpectedAvailableMaps.end() ||
                    /* we allow some test maps so we dont have to update
                       vExpectedAvailableMaps every time we add a new test map */
                    ((sMapName.find("map_test_") != std::string::npos) &&
                        (sMapName.find(".txt") != std::string::npos))), (std::string("Unexpected map found:") + sMapName).c_str());
            if (itFound != vExpectedAvailableMaps.end())
            {
                vExpectedAvailableMaps.erase(sMapName);
            }
        }
        b &= assertTrue(vExpectedAvailableMaps.empty(), "Not found all expected maps!");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
            "availableMapsGetAsCharPtrArray()");

        return b;
    }

    bool test_available_maps_add_single_elem()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");
        const auto nOriginalSize = mapcycle.availableMapsGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        if (!b)
        {
            return false;
        }

        b &= assertFalse(mapcycle.availableMapsAdd("map_test_good.txt"), "add 1"); // already in
        b &= assertTrue(mapcycle.availableMapsAdd("map_asdasdasd.txt"), "add 2");
        b &= assertEquals(nOriginalSize + 1, mapcycle.availableMapsGet().size(), "size 2");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
            "availableMapsGetAsCharPtrArray()");

        return b;
    }

    bool test_available_maps_add_multi_elem()
    {
        const std::vector<std::string> vAddThese = {
            "map_asdasd.txt",
            "map_asdasdasd.txt"
        };

        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");
        const auto nOriginalSize = mapcycle.availableMapsGet().size();
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        if (!b)
        {
            return false;
        }

        b &= assertTrue(mapcycle.availableMapsAdd(vAddThese), "add 1");
        b &= assertEquals(nOriginalSize + vAddThese.size(), mapcycle.availableMapsGet().size(), "size 2");
        b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size 3");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
            "availableMapsGetAsCharPtrArray() 1");

        if (b)
        {
            // try adding same elements again, should fail
            b &= assertFalse(mapcycle.availableMapsAdd(vAddThese), "add 2");
            b &= assertEquals(nOriginalSize + vAddThese.size(), mapcycle.availableMapsGet().size(), "size 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size 5");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_available_maps_remove_by_name()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");
        const auto nOriginalSize = mapcycle.availableMapsGet().size();
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        if (b)
        {
            b &= assertFalse(mapcycle.availableMapsRemove(""), "remove 1");
            b &= assertEquals(nOriginalSize, mapcycle.availableMapsGet().size(), "size 2");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size 3");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 1");

            b &= assertTrue(mapcycle.availableMapsRemove("map_test_good.txt"), "remove 2");
            b &= assertEquals(nOriginalSize - 1, mapcycle.availableMapsGet().size(), "size 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size 5");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_available_maps_remove_by_index()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");
        const auto nOriginalSize = mapcycle.availableMapsGet().size();
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        if (b)
        {
            b &= assertFalse(mapcycle.availableMapsRemove(mapcycle.availableMapsGet().size()), "remove 1");
            b &= assertEquals(nOriginalSize, mapcycle.availableMapsGet().size(), "size 2");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size 3");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 1");

            const std::string sMapDeleted = mapcycle.availableMapsGetElem(0);
            b &= assertTrue(mapcycle.availableMapsRemove(0), "remove 2");
            b &= assertTrue(
                std::find(mapcycle.availableMapsGet().begin(), mapcycle.availableMapsGet().end(), sMapDeleted) == mapcycle.availableMapsGet().end(),
                "cannot find deleted item");
            b &= assertEquals(nOriginalSize - 1, mapcycle.availableMapsGet().size(), "size 3");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size 4");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_available_maps_remove_multi_elem()
    {
        const std::vector<std::string> vRemoveThese = {
           "map_test_bad_assignment.txt",
           "map_test_bad_order.txt"
        };

        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");
        const auto nOriginalSize = mapcycle.availableMapsGet().size();
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        if (b)
        {
            b &= assertTrue(mapcycle.availableMapsRemove(vRemoveThese), "remove 1");
            b &= assertEquals(nOriginalSize - vRemoveThese.size(), mapcycle.availableMapsGet().size(), "size 2");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size 3");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 1");
        }

        if (b)
        {
            // deleting the same should not succeed for the 2nd time
            b &= assertFalse(mapcycle.availableMapsRemove(vRemoveThese), "remove 2");
            b &= assertEquals(nOriginalSize - vRemoveThese.size(), mapcycle.availableMapsGet().size(), "size 3");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size 4");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_mapcycle_reload()
    {
        TestableMapcycle mapcycle;

        // negative test before initialize(), positive tests after initialize()
        // update: even before initialize(), this works, and I'm not changing that now.
        bool b = assertTrue(mapcycle.mapcycleReload(), "reload 1");
        b &= assertNotNull(mapcycle.mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 1");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
            "mapcycleGetAsCharPtrArray() 1");

        b &= assertTrue(mapcycle.initialize(), "init");
        const auto originalMapcycle = mapcycle.mapcycleGet();
        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertGreater(mapcycle.mapcycleGet().size(), 1u, "mapcycle size");  // should be at least 2 maps there
        b &= assertFalse(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 1");

        if (b)
        {
            // just make sure our current is also changed, we are testing it to be the original after reload
            mapcycle.mapcycleNext();
            b &= assertNotEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "current 1");

            b &= assertTrue(mapcycle.mapcycleReload(), "reload 2");
            b &= assertNotNull(mapcycle.mapcycleGetAsCharPtrArray(), "mapcycle charptrarray 2");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray() 2");
            b &= assertTrue(originalMapcycle == mapcycle.mapcycleGet(), "equals");
            b &= assertEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "current 2");
            b &= assertFalse(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");
        }

        return b;
    }

    bool test_mapcycle_save_to_file()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");
        const auto originalMapcycle = mapcycle.mapcycleGet();

        b &= assertGreater(mapcycle.mapcycleGet().size(), 1u, "mapcycle size 1");  // should be at least 2 maps there
        mapcycle.mapcycleClear();
        b &= assertEquals(0u, mapcycle.mapcycleGet().size(), "mapcycle size 2");
        b &= assertTrue(mapcycle.mapcycleSaveToFile(), "save 1");

        b &= assertFalse(mapcycle.mapcycleReload(), "reload 1"); // false due to empty file
        b &= assertEquals(0u, mapcycle.mapcycleGet().size(), "mapcycle size 3");
        b &= assertTrue(mapcycle.mapcycleAdd(originalMapcycle), "add");
        b &= assertTrue(mapcycle.mapcycleSaveToFile(), "save 2");

        mapcycle.mapcycleClear();
        b &= assertTrue(mapcycle.mapcycleReload(), "reload 2");
        b &= assertTrue(originalMapcycle == mapcycle.mapcycleGet(), "final equals");

        return b;
    }

    bool test_mapcycle_next()
    {
        TestableMapcycle mapcycle;

        // negative test before initialize(), positive tests after initialize()
        bool b = assertEquals("", mapcycle.mapcycleNext(), "next 1");

        b &= assertTrue(mapcycle.initialize(), "init");
        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        b &= assertGreater(mapcycle.mapcycleGet().size(), 1u, "mapcycle size");  // should be at least 2 maps there

        if (b)
        {
            size_t nNextCount = 0;
            do
            {
                ++nNextCount;
                const std::string sCurrent = mapcycle.mapcycleNext();
                b &= assertEquals(mapcycle.mapcycleGetCurrent(), sCurrent, "next ret in loop");
            } while ((mapcycle.mapcycleGetCurrent() != sFirstMapName) && (nNextCount < 100u));
            b &= assertNotEquals(100u, nNextCount, "nNextCount 1");
            b &= assertEquals(mapcycle.mapcycleGet().size(), nNextCount, "nNextCount 2");
        }

        return b;
    }

    bool test_mapcycle_rewind_to_first()
    {
        proofps_dd::Mapcycle mapcycle;

        // negative test before initialize(), positive tests after initialize()
        bool b = assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 1");
        b &= assertEquals("", mapcycle.mapcycleRewindToFirst(), "mapcycle rewind to first 1");

        b &= assertTrue(mapcycle.initialize(), "init");
        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        b &= assertGreater(mapcycle.mapcycleGet().size(), 1u, "mapcycle size");  // should be at least 2 maps there
        b &= assertFalse(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 1");

        if (b)
        {
            mapcycle.mapcycleNext();
            b &= assertNotEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "current 1");

            const std::string sAllegedFirstMapName = mapcycle.mapcycleRewindToFirst();
            b &= assertFalse(sAllegedFirstMapName.empty(), "mapcycle rewind to first 2 a");
            b &= assertEquals(sFirstMapName, sAllegedFirstMapName, "mapcycle rewind to first 2 b");
            b &= assertEquals("map_warhouse.txt", sAllegedFirstMapName, "mapcycle rewind to first 2 c specific name");
            b &= assertEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "current 2");
            b &= assertFalse(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");
        }

        return b;
    }

    bool test_mapcycle_forward_to_last()
    {
        proofps_dd::Mapcycle mapcycle;

        // negative test before initialize(), positive tests after initialize()
        bool b = assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 1");
        b &= assertEquals("", mapcycle.mapcycleForwardToLast(), "mapcycle forward to last 1");

        b &= assertTrue(mapcycle.initialize(), "init");
        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        b &= assertGreater(mapcycle.mapcycleGet().size(), 1u, "mapcycle size");  // should be at least 2 maps there
        b &= assertFalse(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            const std::string sLastMapName = mapcycle.mapcycleForwardToLast();
            b &= assertFalse(sLastMapName.empty(), "mapcycle forward to last 2 a");
            b &= assertNotEquals(sFirstMapName, sLastMapName, "mapcycle forward to last 2 b");
            b &= assertEquals("map_construction.txt", sLastMapName, "mapcycle forward to last 2 c specific name");
            b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 3");
        }

        return b;
    }

    bool test_mapcycle_add_single_elem()
    {
        TestableMapcycle mapcycle;

        // even before initialize(), this is working
        bool b = assertTrue(mapcycle.mapcycleGet().empty(), "mapcycle empty 1");
        b &= assertTrue(mapcycle.mapcycleAdd("map_asdasdasd.txt"), "add 1");
        b &= assertFalse(mapcycle.mapcycleGet().empty(), "mapcycle empty 2");

        b &= assertTrue(mapcycle.initialize(), "init");
        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        const auto nOriginalSize = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            b &= assertFalse(mapcycle.mapcycleAdd("map_warhouse.txt"), "add neg"); // already there
            b &= assertTrue(mapcycle.mapcycleAdd("map_asdasdasd.txt"), "add 2");
            b &= assertEquals(nOriginalSize + 1, mapcycle.mapcycleGet().size(), "size 2");
            b &= assertEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_add_multi_elem()
    {
        const std::vector<std::string> vAddThese = {
            "map_asdasd.txt",
            "map_asdasdasd.txt"
        };

        TestableMapcycle mapcycle;

        // even before initialize(), this is working
        bool b = assertTrue(mapcycle.mapcycleGet().empty(), "mapcycle empty 1");
        b &= assertTrue(mapcycle.mapcycleAdd(vAddThese), "add 1");
        b &= assertEquals(vAddThese.size(), mapcycle.mapcycleGet().size(), "size 0");

        b &= assertTrue(mapcycle.initialize(), "init");
        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        const auto nOriginalSize = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            b &= assertTrue(mapcycle.mapcycleAdd(vAddThese), "add 2");
            b &= assertEquals(nOriginalSize + vAddThese.size(), mapcycle.mapcycleGet().size(), "size 2");
            b &= assertEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray() 1");
        }

        if (b)
        {
            // adding the same should not succeed for the 2nd time
            b &= assertFalse(mapcycle.mapcycleAdd(vAddThese), "add neg");
            b &= assertEquals(nOriginalSize + vAddThese.size(), mapcycle.mapcycleGet().size(), "size 3");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_mapcycle_remove_by_name()
    {
        TestableMapcycle mapcycle;

        // negative test before initialize(), positive tests after initialize()
        bool b = assertTrue(mapcycle.mapcycleGet().empty(), "mapcycle empty 1");
        b &= assertFalse(mapcycle.mapcycleRemove("map_asdasdasd.txt"), "remove 1");
        b &= assertTrue(mapcycle.mapcycleGet().empty(), "mapcycle empty 2");

        b &= assertTrue(mapcycle.initialize(), "init");
        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        const auto nOriginalSize = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            b &= assertTrue(mapcycle.mapcycleRemove(sFirstMapName), "remove 2");
            b &= assertEquals(nOriginalSize - 1, mapcycle.mapcycleGet().size(), "size 2");
            b &= assertNotEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_remove_by_index()
    {
        TestableMapcycle mapcycle;

        // negative test before initialize(), positive tests after initialize()
        bool b = assertTrue(mapcycle.mapcycleGet().empty(), "mapcycle empty 1");
        b &= assertFalse(mapcycle.mapcycleRemove(0), "remove 1");
        b &= assertTrue(mapcycle.mapcycleGet().empty(), "mapcycle empty 2");

        b &= assertTrue(mapcycle.initialize(), "init");
        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        const auto nOriginalSize = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            b &= assertTrue(mapcycle.mapcycleRemove(0), "remove 2");
            b &= assertEquals(nOriginalSize - 1, mapcycle.mapcycleGet().size(), "size 2");
            b &= assertNotEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_remove_multi_elem()
    {
        const std::vector<std::string> vRemoveThese = {
           "map_warhouse.txt",
           "map_warena.txt"
        };

        TestableMapcycle mapcycle;

        // negative test before initialize(), positive tests after initialize()
        bool b = assertTrue(mapcycle.mapcycleGet().empty(), "mapcycle empty 1");
        b &= assertFalse(mapcycle.mapcycleRemove(vRemoveThese), "remove 1");
        b &= assertTrue(mapcycle.mapcycleGet().empty(), "mapcycle empty 2");

        b &= assertTrue(mapcycle.initialize(), "init");
        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        const auto nOriginalSize = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        if (b)
        {
            b &= assertTrue(mapcycle.mapcycleRemove(vRemoveThese), "remove 2");
            b &= assertEquals(nOriginalSize - vRemoveThese.size(), mapcycle.mapcycleGet().size(), "size 2");
            b &= assertNotEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray() 1");
        }

        if (b)
        {
            // deleting the same should not succeed for the 2nd time
            b &= assertFalse(mapcycle.mapcycleRemove(vRemoveThese), "remove 3");
            b &= assertEquals(nOriginalSize - vRemoveThese.size(), mapcycle.mapcycleGet().size(), "size 3");
            b &= assertNotEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first 2");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray() 2");
        }

        return b;
    }

    bool test_mapcycle_clear()
    {
        TestableMapcycle mapcycle;

        bool b = assertTrue(mapcycle.initialize(), "init");
        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");
        b &= assertGreater(mapcycle.mapcycleGet().size(), 1u, "size 1");  // should be at least 2 maps there

        if (b)
        {
            mapcycle.mapcycleClear();
            b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast");
            b &= assertTrue(mapcycle.mapcycleGet().empty(), "clear");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_remove_non_existing()
    {
        TestableMapcycle mapcycle;

        bool b = assertTrue(mapcycle.initialize(), "init");

        const auto nOriginalSize = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        // calling it now totally empties mapcycle since initialize() already invoked it too
        b &= assertEquals(nOriginalSize, mapcycle.mapcycleRemoveNonExisting(), "remove nonexisting 1");
        b &= assertEquals(0u, mapcycle.mapcycleGet().size(), "size 2");
        b &= assertTrue(
            checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
            "mapcycleGetAsCharPtrArray()");

        return b;
    }

    bool test_mapcycle_available_maps_synchronize()
    {
        TestableMapcycle mapcycle;

        bool b = assertTrue(mapcycle.initialize(), "init");
        const auto nOriginalSize = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSize, 1u, "size 1");  // should be at least 2 maps there

        for (const auto& sMapcycleMap : mapcycle.mapcycleGet())
        {
            b &= assertTrue(
                std::find(mapcycle.availableMapsGet().begin(), mapcycle.availableMapsGet().end(), sMapcycleMap) == mapcycle.availableMapsGet().end(),
                "mapcycle item in available maps");
        }

        b &= assertTrue(mapcycle.mapcycleAdd("map_asdasdasd.txt"), "add");
        b &= assertEquals(nOriginalSize + 1, mapcycle.mapcycleGet().size(), "size 2");
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();

        if (b)
        {
            mapcycle.mapcycle_availableMaps_Synchronize();

            // fictive map should disappear from mapcycle
            b &= assertEquals(nOriginalSize, mapcycle.mapcycleGet().size(), "size 3");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size 4");
            b &= assertTrue(
                std::find(mapcycle.mapcycleGet().begin(), mapcycle.mapcycleGet().end(), "map_asdasdasd.txt") == mapcycle.mapcycleGet().end(),
                "fictive mapcycle item");

            // available maps should not contain any item present in mapcycle
            for (const auto& sMapcycleMap : mapcycle.mapcycleGet())
            {
                b &= assertTrue(
                    std::find(mapcycle.availableMapsGet().begin(), mapcycle.availableMapsGet().end(), sMapcycleMap) == mapcycle.availableMapsGet().end(),
                    "mapcycle item in available maps");
            }

            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_add_available_maps_remove_by_name()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");

        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");

        const auto nOriginalSizeMapCycle = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = mapcycle.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();

        if (b)
        {
            // should fail because map is empty
            b &= assertFalse(mapcycle.mapcycleAdd_availableMapsRemove(""), "add neg 1");
            b &= assertEquals(nOriginalSizeMapCycle, mapcycle.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(nOriginalSizeAvailableMaps, mapcycle.availableMapsGet().size(), "size available maps 2");

            // should fail because map is already in mapcycle
            b &= assertFalse(mapcycle.mapcycleAdd_availableMapsRemove("map_warhouse.txt"), "add neg 2");
            b &= assertEquals(nOriginalSizeMapCycle, mapcycle.mapcycleGet().size(), "size mapcycle 3");
            b &= assertEquals(nOriginalSizeAvailableMaps, mapcycle.availableMapsGet().size(), "size available maps 3");

            b &= assertTrue(mapcycle.mapcycleAdd_availableMapsRemove("map_test_good.txt"), "add 1");
            b &= assertEquals(nOriginalSizeMapCycle + 1, mapcycle.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps - 1, mapcycle.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                std::find(mapcycle.availableMapsGet().begin(), mapcycle.availableMapsGet().end(), "map_test_good.txt") == mapcycle.availableMapsGet().end(),
                "can find deleted item in available maps");
            b &= assertFalse(
                std::find(mapcycle.mapcycleGet().begin(), mapcycle.mapcycleGet().end(), "map_test_good.txt") == mapcycle.mapcycleGet().end(),
                "cannot find deleted item in mapcycle");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_add_available_maps_remove_multi_elem()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");

        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");

        const auto nOriginalSizeMapCycle = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = mapcycle.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();

        if (b)
        {
            const std::vector<std::string> vAddRemoveThese_Fail = {
               "map_warhouse.txt",
               "map_warena.txt"
            };

            // should fail because maps are already in mapcycle
            b &= assertFalse(mapcycle.mapcycleAdd_availableMapsRemove(vAddRemoveThese_Fail), "add neg 1");
            b &= assertEquals(nOriginalSizeMapCycle, mapcycle.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(nOriginalSizeAvailableMaps, mapcycle.availableMapsGet().size(), "size available maps 2");

            const std::vector<std::string> vAddRemoveThese_Fail2 = {
               ""
            };
            // should fail because of empty string
            b &= assertFalse(mapcycle.mapcycleAdd_availableMapsRemove(vAddRemoveThese_Fail2), "add neg 2");
            b &= assertEquals(nOriginalSizeMapCycle, mapcycle.mapcycleGet().size(), "size mapcycle 3");
            b &= assertEquals(nOriginalSizeAvailableMaps, mapcycle.availableMapsGet().size(), "size available maps 3");

            const std::vector<std::string> vAddRemoveThese_Pass = {
               "map_test_bad_assignment.txt",
               "map_test_bad_order.txt",
               "map_test_good.txt"
            };

            b &= assertTrue(mapcycle.mapcycleAdd_availableMapsRemove(vAddRemoveThese_Pass), "add 1");
            b &= assertEquals(nOriginalSizeMapCycle + vAddRemoveThese_Pass.size(), mapcycle.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps - vAddRemoveThese_Pass.size(), mapcycle.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first");
            for (const auto& sMapCheck : vAddRemoveThese_Pass)
            {
                b &= assertTrue(
                    std::find(mapcycle.availableMapsGet().begin(), mapcycle.availableMapsGet().end(), sMapCheck) == mapcycle.availableMapsGet().end(),
                    "can find deleted item in available maps");
                b &= assertFalse(
                    std::find(mapcycle.mapcycleGet().begin(), mapcycle.mapcycleGet().end(), sMapCheck) == mapcycle.mapcycleGet().end(),
                    "cannot find deleted item in mapcycle");
            }
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_add_available_maps_remove_all()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");

        const auto nOriginalSizeMapCycle = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = mapcycle.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();

        if (b)
        {
            b &= assertTrue(mapcycle.mapcycleAdd_availableMapsRemove(), "add");
            b &= assertEquals(nOriginalSizeMapCycle + nOriginalSizeAvailableMaps, mapcycle.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(0u, mapcycle.availableMapsGet().size(), "size available maps 2");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(mapcycle.mapcycleGet()[0u], mapcycle.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_remove_available_maps_add_by_name()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");

        // add this elem so we will have a valid thing to make positive test for
        b &= assertTrue(mapcycle.mapcycleAdd("map_asdasd.txt"), "extend");

        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");

        const auto nOriginalSizeMapCycle = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = mapcycle.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();

        if (b)
        {
            // should fail because map is empty
            b &= assertFalse(mapcycle.mapcycleRemove_availableMapsAdd(""), "add neg 1");
            b &= assertEquals(nOriginalSizeMapCycle, mapcycle.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(nOriginalSizeAvailableMaps, mapcycle.availableMapsGet().size(), "size available maps 2");

            // should fail because map is NOT in mapcycle
            b &= assertFalse(mapcycle.mapcycleRemove_availableMapsAdd("map_test_good.txt"), "add neg 2");
            b &= assertEquals(nOriginalSizeMapCycle, mapcycle.mapcycleGet().size(), "size mapcycle 3");
            b &= assertEquals(nOriginalSizeAvailableMaps, mapcycle.availableMapsGet().size(), "size available maps 3");

            b &= assertTrue(mapcycle.mapcycleRemove_availableMapsAdd("map_asdasd.txt"), "add 1");
            b &= assertEquals(nOriginalSizeMapCycle - 1, mapcycle.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps + 1, mapcycle.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first");
            b &= assertFalse(
                std::find(mapcycle.availableMapsGet().begin(), mapcycle.availableMapsGet().end(), "map_asdasd.txt") == mapcycle.availableMapsGet().end(),
                "cannot find deleted item in available maps");
            b &= assertTrue(
                std::find(mapcycle.mapcycleGet().begin(), mapcycle.mapcycleGet().end(), "map_asdasd.txt") == mapcycle.mapcycleGet().end(),
                "can find deleted item in mapcycle");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_remove_available_maps_add_by_index()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");

        // add this elem so we will have a valid thing to make positive test for
        b &= assertTrue(mapcycle.mapcycleAdd("map_asdasd.txt"), "extend");

        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");

        const auto nOriginalSizeMapCycle = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = mapcycle.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();

        if (b)
        {
            // should fail because of bad index
            b &= assertFalse(mapcycle.mapcycleRemove_availableMapsAdd(mapcycle.mapcycleGet().size()), "add neg 1");
            b &= assertEquals(nOriginalSizeMapCycle, mapcycle.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(nOriginalSizeAvailableMaps, mapcycle.availableMapsGet().size(), "size available maps 2");

            b &= assertTrue(mapcycle.mapcycleRemove_availableMapsAdd(mapcycle.mapcycleGet().size() - 1), "add 1");
            b &= assertEquals(nOriginalSizeMapCycle - 1, mapcycle.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps + 1, mapcycle.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first");
            b &= assertFalse(
                std::find(mapcycle.availableMapsGet().begin(), mapcycle.availableMapsGet().end(), "map_asdasd.txt") == mapcycle.availableMapsGet().end(),
                "cannot find deleted item in available maps");
            b &= assertTrue(
                std::find(mapcycle.mapcycleGet().begin(), mapcycle.mapcycleGet().end(), "map_asdasd.txt") == mapcycle.mapcycleGet().end(),
                "can find deleted item in mapcycle");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_remove_available_maps_add_multi_elem()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");

        const std::vector<std::string> vRemoveAddThese_Pass = {
               "msp_asdasd.txt",
               "msp_asdasd_2.txt"
        };
        for (const auto& sMapCheck : vRemoveAddThese_Pass)
        {
            // add this elem so we will have a valid thing to make positive test for
            b &= assertTrue(mapcycle.mapcycleAdd(sMapCheck), "extend");
        }

        const std::string sFirstMapName = mapcycle.mapcycleGetCurrent();
        b &= assertFalse(sFirstMapName.empty(), "empty");

        const auto nOriginalSizeMapCycle = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = mapcycle.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();

        if (b)
        {
            const std::vector<std::string> vAddRemoveThese_Fail = {
               ""
            };
            // should fail because map is empty
            b &= assertFalse(mapcycle.mapcycleRemove_availableMapsAdd(vAddRemoveThese_Fail), "add neg 1");
            b &= assertEquals(nOriginalSizeMapCycle, mapcycle.mapcycleGet().size(), "size mapcycle 2");
            b &= assertEquals(nOriginalSizeAvailableMaps, mapcycle.availableMapsGet().size(), "size available maps 2");

            const std::vector<std::string> vAddRemoveThese_Fail2 = {
               "map_test_good.txt"
            };
            // should fail because map is NOT in mapcycle
            b &= assertFalse(mapcycle.mapcycleRemove_availableMapsAdd(vAddRemoveThese_Fail2), "add neg 2");
            b &= assertEquals(nOriginalSizeMapCycle, mapcycle.mapcycleGet().size(), "size mapcycle 3");
            b &= assertEquals(nOriginalSizeAvailableMaps, mapcycle.availableMapsGet().size(), "size available maps 3");

            b &= assertTrue(mapcycle.mapcycleRemove_availableMapsAdd(vRemoveAddThese_Pass), "add 1");
            b &= assertEquals(nOriginalSizeMapCycle - vRemoveAddThese_Pass.size(), mapcycle.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps + vRemoveAddThese_Pass.size(), mapcycle.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals(sFirstMapName, mapcycle.mapcycleGetCurrent(), "rewind to first");
            for (const auto& sMapCheck : vRemoveAddThese_Pass)
            {
                b &= assertFalse(
                    std::find(mapcycle.availableMapsGet().begin(), mapcycle.availableMapsGet().end(), sMapCheck) == mapcycle.availableMapsGet().end(),
                    "cannot find deleted item in available maps");
                b &= assertTrue(
                    std::find(mapcycle.mapcycleGet().begin(), mapcycle.mapcycleGet().end(), sMapCheck) == mapcycle.mapcycleGet().end(),
                    "can find deleted item in mapcycle");
            }
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

    bool test_mapcycle_remove_available_maps_add_all()
    {
        TestableMapcycle mapcycle;
        bool b = assertTrue(mapcycle.initialize(), "init");

        const auto nOriginalSizeMapCycle = mapcycle.mapcycleGet().size();
        b &= assertGreater(nOriginalSizeMapCycle, 1u, "size mapcycle 1");  // should be at least 2 maps there
        mapcycle.mapcycleForwardToLast();
        b &= assertTrue(mapcycle.mapcycleIsCurrentLast(), "mapcycle islast 2");

        const auto nOriginalSizeAvailableMaps = mapcycle.availableMapsGet().size();
        b &= assertGreater(nOriginalSizeAvailableMaps, 1u, "size available maps 1");  // should be at least 2 maps there
        const auto nOriginalSizeAvailableMapsNoChanging = mapcycle.availableMapsNoChangingGet().size();

        if (b)
        {
            b &= assertTrue(mapcycle.mapcycleRemove_availableMapsAdd(), "add");
            b &= assertEquals(0u, mapcycle.mapcycleGet().size(), "size mapcycle 4");
            b &= assertEquals(nOriginalSizeAvailableMaps + nOriginalSizeMapCycle, mapcycle.availableMapsGet().size(), "size available maps 4");
            b &= assertEquals(nOriginalSizeAvailableMapsNoChanging, mapcycle.availableMapsNoChangingGet().size(), "size available maps nochanging");
            b &= assertEquals("", mapcycle.mapcycleGetCurrent(), "rewind to first");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.mapcycleGet(), mapcycle.mapcycleGetAsCharPtrArray()),
                "mapcycleGetAsCharPtrArray()");
            b &= assertTrue(
                checkConstCharPtrArrayElemsPointingToContainerElems(mapcycle.availableMapsGet(), mapcycle.availableMapsGetAsCharPtrArray()),
                "availableMapsGetAsCharPtrArray()");
        }

        return b;
    }

};
