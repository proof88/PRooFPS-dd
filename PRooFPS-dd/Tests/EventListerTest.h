#pragma once

/*
    ###################################################################################
    EventListerTest.h
    Unit test for PRooFPS-dd EventLister.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include <thread>

#include "UnitTest.h"

#include "EventLister.h"

class EventListerTest :
    public UnitTest
{
public:

    EventListerTest() :
        UnitTest(__FILE__)
    {
    }

    EventListerTest(const EventListerTest&) = delete;
    EventListerTest& operator=(const EventListerTest&) = delete;
    EventListerTest(EventListerTest&&) = delete;
    EventListerTest& operator=(EventListerTest&&) = delete;

protected:

    virtual void initialize() override
    {
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::EventLister<>::getLoggerModuleName(), true);

        addSubTest("test_initial_values", (PFNUNITSUBTEST)&EventListerTest::test_initial_values);
        addSubTest("test_show_hide", (PFNUNITSUBTEST)&EventListerTest::test_show_hide);
        addSubTest("test_add_event", (PFNUNITSUBTEST)&EventListerTest::test_add_event);
        addSubTest("test_clear", (PFNUNITSUBTEST)&EventListerTest::test_clear);
        addSubTest("test_update", (PFNUNITSUBTEST)&EventListerTest::test_update);

    }

    virtual void finalize() override
    {
        CConsole::getConsoleInstance().SetLoggingState(proofps_dd::EventLister<>::getLoggerModuleName(), false);
    }

private:

    static constexpr size_t nMaxEventCount = 3u;
    static constexpr unsigned int nMaxEventTimeSecs = 2u;

    // ---------------------------------------------------------------------------

    bool does_event_lister_has_same_elems_as_vector(
        const proofps_dd::EventLister<>& events,
        const std::vector<std::string>& vecExpectedStrings)
    {
        if (events.getEvents().size() != vecExpectedStrings.size())
        {
            return false;
        }

        bool b = true;
        //CConsole::getConsoleInstance().EOLn("does_event_lister_has_same_elems_as_vector start");
        auto& eventsQ = events.getEvents();
        size_t fifoIndex = eventsQ.begin_index();
        for (size_t i = 0; i < vecExpectedStrings.size(); i++)
        {
            const auto& elem = eventsQ.underlying_array()[fifoIndex];
            //CConsole::getConsoleInstance().EOLn("  check: %s == %s", vecExpectedStrings[i].c_str(), elem.second.c_str());
            b &= (vecExpectedStrings[i] == elem.m_event.m_str);
            fifoIndex = eventsQ.next_index(fifoIndex);
        }
        //CConsole::getConsoleInstance().EOLn("does_event_lister_has_same_elems_as_vector end");
        //CConsole::getConsoleInstance().EOLn("");

        return b;
    }

    bool test_initial_values()
    {
        proofps_dd::EventLister eventsVertical(nMaxEventTimeSecs, nMaxEventCount);
        proofps_dd::EventLister eventsHorizontal(nMaxEventTimeSecs, nMaxEventCount, proofps_dd::Orientation::Horizontal);

        bool b = (assertTrue(eventsVertical.getEvents().empty(), "empty") &
            assertFalse(eventsVertical.visible(), "visible") &
            assertEquals(nMaxEventTimeSecs, eventsVertical.getEventTimeLimitSecs(), "time limit") &
            assertEquals(nMaxEventCount, eventsVertical.getEventCountLimit(), "count limit") &
            assertTrue(proofps_dd::Orientation::Vertical == eventsVertical.getOrientation(), "orientation vertical") &
            assertTrue(proofps_dd::Orientation::Horizontal == eventsHorizontal.getOrientation(), "orientation horizontal")) != 0;

        return b;
    }

    bool test_show_hide()
    {
        proofps_dd::EventLister events(nMaxEventTimeSecs, nMaxEventCount);

        bool b = assertFalse(events.visible(), "visible 1");

        events.show();
        b &= assertTrue(events.visible(), "visible 2");

        events.hide();
        b &= assertFalse(events.visible(), "visible 3");

        events.show();
        b &= assertTrue(events.visible(), "visible 4");

        return b;
    }

    bool test_add_event()
    {
        proofps_dd::EventLister events(nMaxEventTimeSecs, nMaxEventCount);

        const std::vector<std::string> vecExpectedStrings =
        { {"event 1"},
          {"event 2"},
          {"event 3"} };

        events.addEvent("event 1");
        events.addEvent("event 2");
        events.addEvent("event 3");

        bool b = assertFalse(events.getEvents().empty(), "empty");
        b &= assertTrue(does_event_lister_has_same_elems_as_vector(events, vecExpectedStrings), "case 1");

        events.addEvent("event 4");

        const std::vector<std::string> vecExpectedStrings2 =
        { {"event 2"},
          {"event 3"},
          {"event 4"} };

        b &= assertTrue(does_event_lister_has_same_elems_as_vector(events, vecExpectedStrings2), "case 2");

        return b;
    }

    bool test_clear()
    {
        proofps_dd::EventLister events(nMaxEventTimeSecs, nMaxEventCount);

        events.addEvent("event 1");
        events.addEvent("event 2");
        
        events.clear();

        return events.getEvents().empty();
    }

    bool test_update()
    {
        proofps_dd::EventLister events(nMaxEventTimeSecs, nMaxEventCount);

        const std::vector<std::string> vecExpectedStrings1 =
        { {"event 1"},
          {"event 2"},
          {"event 3"} };

        const std::vector<std::string> vecExpectedStrings2 =
        { {"event 2"},
          {"event 3"} };

        const std::vector<std::string> vecExpectedStrings3 =
        { {"event 3"} };

        const std::vector<std::vector<std::string>> vecExpectedStringCases = {
            vecExpectedStrings1,
            vecExpectedStrings2,
            vecExpectedStrings3
        };

        std::set<int> expectedStringCasesObserved;

        events.addEvent("event 1");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        events.addEvent("event 2");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        events.addEvent("event 3");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        bool b = true;
        int nSleepedMilliSecs = 0;
        while ((nSleepedMilliSecs < 1500) && !events.getEvents().empty())
        {
            nSleepedMilliSecs += 100;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            events.update();

            for (size_t i = 0; i < vecExpectedStringCases.size(); i++)
            {
                if (does_event_lister_has_same_elems_as_vector(events, vecExpectedStringCases[i]))
                {
                    expectedStringCasesObserved.insert(i+1);
                    break;
                }
            }
        }

        const std::set<int> expectedStringCasesExpected = { 1, 2, 3 };
        b = assertTrue(expectedStringCasesExpected == expectedStringCasesObserved, "expectedString cases observed");

        return b;
    }

};
