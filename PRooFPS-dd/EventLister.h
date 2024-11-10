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

#include "FixFIFO.h"
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

        //typedef std::pair<std::chrono::time_point<std::chrono::steady_clock>, std::string> TimeStringPair;
        struct TimeStringPair
        {
            std::chrono::time_point<std::chrono::steady_clock> m_timestamp;
            std::string m_str; // constructing and destructing this when TimeStringPair is temporal object, too expensive!
                               // consider switching to a more lightweight string, like something implemented in Dear ImGui!

            TimeStringPair() :
                m_timestamp{},
                m_str{}
            {}

            TimeStringPair(
                const std::chrono::time_point<std::chrono::steady_clock>& ts,
                const std::string& str) :
                m_timestamp(ts),
                m_str(str)
            {}

            TimeStringPair(
                std::chrono::time_point<std::chrono::steady_clock>&& ts,
                std::string&& str) :
                m_timestamp(std::move(ts)),
                m_str(std::move(str))
            {}

            ~TimeStringPair() = default;

            TimeStringPair(const TimeStringPair&) = default;
            TimeStringPair& operator=(const TimeStringPair&) = default;
            TimeStringPair(TimeStringPair&&) = default;
            TimeStringPair& operator=(TimeStringPair&&) = default;
        };

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

        //void addEvent(const std::string& sEvent);
        void addEvent(std::string&& sEvent);
        void clear();

        //const std::deque<TimeStringPair>& getEvents() const;
        const pfl::FixFIFO<TimeStringPair>& getEvents() const;

    protected:

        EventLister(const EventLister&) = delete;
        EventLister& operator=(const EventLister&) = delete;
        EventLister(EventLister&&) = delete;
        EventLister& operator=(EventLister&&) = delete;

    private:

        bool m_bVisible = false;
        //std::deque<TimeStringPair> m_qEvents;
        pfl::FixFIFO<TimeStringPair> m_qEvents;
        unsigned int m_nEventTimeLimitSecs;
        //size_t m_nEventCountLimit;
        Orientation m_eOrientation;

        // ---------------------------------------------------------------------------

    }; // class EventLister

} // namespace proofps_dd
