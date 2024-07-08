#pragma once

/*
    ###################################################################################
    DeathKillEventLister.h
    Death/Kill Event lister class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "EventLister.h"

namespace proofps_dd
{

    class DeathKillEventLister : public EventLister
    {
    public:

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        DeathKillEventLister();

        void addDeathKillEvent(const std::string& sKiller, const std::string& sKilled);

    protected:

        DeathKillEventLister(const DeathKillEventLister&) = delete;
        DeathKillEventLister& operator=(const DeathKillEventLister&) = delete;
        DeathKillEventLister(DeathKillEventLister&&) = delete;
        DeathKillEventLister&& operator=(DeathKillEventLister&&) = delete;

    private:

        // ---------------------------------------------------------------------------

    }; // class DeathKillEventLister

} // namespace proofps_dd
