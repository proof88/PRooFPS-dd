#pragma once

/*
    ###################################################################################
    EventListerPerfTest.h
    Performance test for PRooFPS-dd EventLister.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include <thread>

#include "Benchmarks.h"
#include "UnitTests/UnitTest.h"

#include "EventLister.h"

class EventListerPerfTest :
    public Benchmark
{
public:

    EventListerPerfTest() :
        Benchmark(__FILE__)
    {
    }

    EventListerPerfTest(const EventListerPerfTest&) = delete;
    EventListerPerfTest& operator=(const EventListerPerfTest&) = delete;
    EventListerPerfTest(EventListerPerfTest&&) = delete;
    EventListerPerfTest&& operator=(EventListerPerfTest&&) = delete;

protected:

    virtual void initialize() override
    {
        //CConsole::getConsoleInstance().SetLoggingState(proofps_dd::EventLister::getLoggerModuleName(), true);

        addSubTest("test_benchmark_add_event", (PFNUNITSUBTEST)&EventListerPerfTest::test_benchmark_add_event);
        addSubTest("test_benchmark_update", (PFNUNITSUBTEST)&EventListerPerfTest::test_benchmark_update);

    }

    virtual void finalize() override
    {
        //CConsole::getConsoleInstance().SetLoggingState(proofps_dd::EventLister::getLoggerModuleName(), false);
    }

private:

    // ---------------------------------------------------------------------------


    bool test_benchmark_add_event()
    {
        constexpr size_t nMaxEventCount = 5u;
        constexpr unsigned int nMaxEventTimeSecs = 1000u; // make sure timeout does not interfere
        proofps_dd::EventLister events(nMaxEventTimeSecs, nMaxEventCount);

        constexpr auto textArray = PFL::std_array_of<const char*>(
            "Example 1 Random Event String for the List",
            "Example 2 Random Event String for the List",
            "Example 3 Random Event String for the List"
        );

        {
            ScopeBenchmarker scopeBm("bm");
            for (int i = 0; i < 1000000; i++)
            {
                events.addEvent(textArray[PFL::random(0, textArray.size()-1)]);
            }
        }

        // const auto& scopeBmData = ScopeBenchmarker::getDataByName("bm");

        return true;
    }

    bool test_benchmark_update()
    {
        constexpr size_t nMaxEventCount = 1000u;  // let this be big so events can accumulate before update() has to remove due to timeout
        constexpr unsigned int nMaxEventTimeSecs = 1u;
        constexpr long long nTestDurationTotalSecs = 10u;
        static_assert(
            nMaxEventTimeSecs < nTestDurationTotalSecs,
            "Max event time should be far less than test duration, otherwise update() won't really do meaningful cleanup job!");
        proofps_dd::EventLister events(nMaxEventTimeSecs, nMaxEventCount);

        constexpr auto textArray = PFL::std_array_of<const char*>(
            "Example 1 Random Event String for the List",
            "Example 2 Random Event String for the List",
            "Example 3 Random Event String for the List"
        );

        // first we fill up the container up to its max size to decrease test dependency on this part
        for (int i = 0; i < 10; i++)
        {
            events.addEvent(textArray[PFL::random(0, textArray.size() - 1)]);
        }

        {
            long long nTestDurationCurrentSecs = 0;
            std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
            while (nTestDurationCurrentSecs < nTestDurationTotalSecs)
            {
                ScopeBenchmarker scopeBmOut("bmOutside");
                // we need to keep adding elements because update() removes expired elems
                for (int i = 0; i < 100; i++)
                {
                    events.addEvent(textArray[PFL::random(0, textArray.size() - 1)]);
                }
                {
                    ScopeBenchmarker scopeBmIn("bmInside"); // we also measure purely here
                    events.update();
                }
                nTestDurationCurrentSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - timeStart).count();
            }
        }

        addToInfoMessages("  Higher iteration count is better (identical for both benchmarkers).");
        addToInfoMessages("  Lower duration values for bmInside is better.");
        // const auto& scopeBmData = ScopeBenchmarker::getDataByName("bm");

        return true;
    }

};
