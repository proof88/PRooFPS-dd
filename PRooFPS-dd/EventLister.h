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

    struct Event
    {
        std::string m_str;  // constructing and destructing this when TimeEventPair is temporal object, too expensive!
                            // consider switching to a more lightweight string, like something implemented in Dear ImGui!

        Event() :
            m_str{}
        {}

        Event(const std::string& str) :
            m_str{ str }
        {}

        Event(std::string&& str) :
            m_str(std::move(str))
        {}

        // TODO: why is this here, DrawableEvent should introduce it!
        virtual void draw() {};
    };

    // TODO: this should also go to Drawable stuff
    enum class Orientation
    {
        Vertical,
        Horizontal
    };

    /**
    * A specified maximum number of events stored in FIFO container for a limited amount of time.
    * Typical event use cases: who killed who, items picked up by player, etc.
    */
    template <class TEvent = Event>
    class EventLister
    {
    public:

        struct TimeEventPair
        {
            std::chrono::time_point<std::chrono::steady_clock> m_timestamp;
            TEvent m_event;

            TimeEventPair() :
                m_timestamp{},
                m_event{}
            {}

            TimeEventPair(
                const std::chrono::time_point<std::chrono::steady_clock>& ts,
                const std::string& str) :
                m_timestamp(ts),
                m_event(str)
            {}

            TimeEventPair(
                std::chrono::time_point<std::chrono::steady_clock>&& ts,
                std::string&& str) :
                m_timestamp(std::move(ts)),
                m_event(std::move(str))
            {}

            ~TimeEventPair() = default;

            TimeEventPair(const TimeEventPair&) = default;
            TimeEventPair& operator=(const TimeEventPair&) = default;
            TimeEventPair(TimeEventPair&&) = default;
            TimeEventPair& operator=(TimeEventPair&&) = default;

            // TODO: why is this here, DrawableTimeEventPair should introduce it!
            virtual void draw() {};
        };

        static const char* getLoggerModuleName()
        {
            return "EventLister";
        }

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const
        {
            return CConsole::getConsoleInstance(getLoggerModuleName());
        }

        EventLister(
            const unsigned int& nEventTimeLimitSecs,
            const size_t& nEventCountLimit,
            const Orientation& eOrientation = Orientation::Vertical) :
            m_qEvents(nEventCountLimit),
            m_nEventTimeLimitSecs(nEventTimeLimitSecs),
            //m_nEventCountLimit(nEventCountLimit),
            m_eOrientation(eOrientation)
        {}

        const unsigned int& getEventTimeLimitSecs() const
        {
            return m_nEventTimeLimitSecs;
        }

        const size_t& getEventCountLimit() const
        {
            return m_qEvents.capacity();
        }

        const Orientation& getOrientation() const
        {
            return m_eOrientation;
        }

        void show()
        {
            m_bVisible = true;
        }

        void hide()
        {
            m_bVisible = false;
        }

        bool visible() const
        {
            return m_bVisible;
        }

        void update()
        {
            while (!m_qEvents.empty())
            {
                const auto& elem = m_qEvents.front();
                if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - elem.m_timestamp).count() >= m_nEventTimeLimitSecs)
                {
                    m_qEvents.pop_front();
                }
                else
                {
                    return; // other elems are newer so definitely no need to pop anymore elems!
                }
            }
        }

        // TODO: why is this here, DrawableEventLister should introduce it!
        virtual void draw() {};

        void addEvent(std::string&& sEvent)
        {
            m_qEvents.push_back_forced(
                { std::move(std::chrono::steady_clock::now()), std::move(sEvent) });
        }

        void clear()
        {
            m_qEvents.clear();
        }

        const pfl::FixFIFO<TimeEventPair>& getEvents() const
        {
            return m_qEvents;
        }

    protected:

        EventLister(const EventLister&) = delete;
        EventLister& operator=(const EventLister&) = delete;
        EventLister(EventLister&&) = delete;
        EventLister& operator=(EventLister&&) = delete;

    private:

        bool m_bVisible = false;
        //std::deque<TimeEventPair> m_qEvents;
        pfl::FixFIFO<TimeEventPair> m_qEvents;
        unsigned int m_nEventTimeLimitSecs;
        //size_t m_nEventCountLimit;
        Orientation m_eOrientation;

        // ---------------------------------------------------------------------------

    }; // class EventLister

} // namespace proofps_dd
