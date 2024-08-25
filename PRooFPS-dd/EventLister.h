#pragma once

/*
    ###################################################################################
    EventLister.h
    Event lister class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include <deque>

#include "CConsole.h"

#include "Consts.h"

namespace proofps_dd
{

    /**
    * A specified maximum number of events stored in FIFO container for a limited amount of time.
    * Typical event use cases: who killed who, items picked up by player, etc.
    */
    class EventLister
    {
    public:

        enum class Orientation
        {
            Vertical,
            Horizontal
        };

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        EventLister(
            const unsigned int& nEventTimeLimitSecs,
            const size_t& nEventCountLimit,
            const Orientation& eOrientation = Orientation::Vertical);

        const unsigned int& getEventTimeLimitSecs() const;
        const size_t& getEventCountLimit() const;
        const Orientation& getOrientation() const;

        void show();
        void hide();
        bool visible() const;

        void update();

        virtual void draw() {};

        void addEvent(const std::string& sEvent);
        void clear();

        const std::deque<std::pair<std::chrono::time_point<std::chrono::steady_clock>, std::string>>& getEvents() const;

    protected:

        EventLister(const EventLister&) = delete;
        EventLister& operator=(const EventLister&) = delete;
        EventLister(EventLister&&) = delete;
        EventLister&& operator=(EventLister&&) = delete;

    private:

        bool m_bVisible = false;
        std::deque<std::pair<std::chrono::time_point<std::chrono::steady_clock>, std::string>> m_qEvents;
        unsigned int m_nEventTimeLimitSecs;
        size_t m_nEventCountLimit;
        Orientation m_eOrientation;

        // ---------------------------------------------------------------------------

    }; // class EventLister

} // namespace proofps_dd
