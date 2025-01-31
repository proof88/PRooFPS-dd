#pragma once

/*
    ###################################################################################
    DrawableEventLister.h
    Event lister class for PRooFPS-dd, with implemented draw function
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "EventLister.h"

// ATTENTION!
// BEFORE CONSIDERING DELETING THIS INTERMEDIATE CLASS, THINK ABOUT UNIT TESTS THAT MIGHT
// BE MUCH HARDER IF draw() APPEARS IN EventLister CLASS WITH IMGUI IMPLEMENTATION!

// PGE has, but here in application we dont have imconfig.h thus we should not try including it!
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include "imgui.h"  // ImVec4

namespace proofps_dd
{

    struct DrawableEvent : public Event
    {
        DrawableEvent() :
            Event()
        {}

        DrawableEvent(const std::string& str) :
            Event(str)
        {}

        DrawableEvent(std::string&& str) :
            Event(str)
        {}

        virtual void draw()
        {
            // TODO: cannot be implemented until drawTextHighlighted() is moved from GUI.cpp to separate unit.
        };
    };

    /**
    * A specified maximum number of events stored in FIFO container for a limited amount of time.
    * Typical event use cases: who killed who, items picked up by player, etc.
    */
    template <class TEvent = DrawableEvent>
    class DrawableEventLister : public EventLister<TEvent>
    {
    public:

        struct DrawableTimeEventPair : public TimeEventPair
        {
            virtual void draw()
            {
                // TODO: cannot be implemented until drawTextHighlighted() is moved from GUI.cpp to separate unit.
            };
        };

        DrawableEventLister(
            const unsigned int& nEventTimeLimitSecs,
            const size_t& nEventCountLimit,
            const Orientation& eOrientation = Orientation::Vertical) :
            EventLister(nEventTimeLimitSecs, nEventCountLimit, eOrientation)
        {

        }

        virtual void draw()
        {
            // TODO: cannot be implemented until drawTextHighlighted() is moved from GUI.cpp to separate unit.
        }

    protected:

        DrawableEventLister(const DrawableEventLister&) = delete;
        DrawableEventLister& operator=(const DrawableEventLister&) = delete;
        DrawableEventLister(DrawableEventLister&&) = delete;
        DrawableEventLister&& operator=(DrawableEventLister&&) = delete;

    private:

        // ---------------------------------------------------------------------------

    }; // class DrawableEventLister

} // namespace proofps_dd
