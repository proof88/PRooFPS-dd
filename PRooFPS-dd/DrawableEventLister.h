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

namespace proofps_dd
{

    /**
    * A specified maximum number of events stored in FIFO container for a limited amount of time.
    * Typical event use cases: who killed who, items picked up by player, etc.
    */
    class DrawableEventLister : public EventLister
    {
    public:

        DrawableEventLister(
            const unsigned int& nEventTimeLimitSecs,
            const size_t& nEventCountLimit,
            const EventLister::Orientation& eOrientation = EventLister::Orientation::Vertical);

        virtual void draw() override;

    protected:

        DrawableEventLister(const DrawableEventLister&) = delete;
        DrawableEventLister& operator=(const DrawableEventLister&) = delete;
        DrawableEventLister(DrawableEventLister&&) = delete;
        DrawableEventLister&& operator=(DrawableEventLister&&) = delete;

    private:

        // ---------------------------------------------------------------------------

    }; // class DrawableEventLister

} // namespace proofps_dd
