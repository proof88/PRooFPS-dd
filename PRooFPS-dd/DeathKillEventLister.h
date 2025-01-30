#pragma once

/*
    ###################################################################################
    DeathKillEventLister.h
    Death/Kill Event lister class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "DrawableEventLister.h"

namespace proofps_dd
{

    class DeathKillEventLister : public DrawableEventLister<>
    {
    public:

        static const char* getLoggerModuleName()
        {
            return "DeathKillEventLister";
        }

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const
        {
            return CConsole::getConsoleInstance(getLoggerModuleName());
        }

        DeathKillEventLister() :
            DrawableEventLister(
                5 /* time limit secs */,
                8 /* event count limit */,
                Orientation::Vertical)
        {}

        void addDeathKillEvent(const std::string& sKiller, const std::string& sKilled)
        {
            if (sKiller.empty())
            {
                addEvent(sKilled + " died");
            }
            else
            {
                addEvent(sKiller + " killed " + sKilled);
            }
        }

        void addDeathKillEvent(
            const std::string& sKiller,
            const ImVec4& clrKiller,
            const std::string& sKilled,
            const ImVec4& clrKilled)
        {
            (void)sKiller;
            (void)clrKiller;
            (void)sKilled;
            (void)clrKilled;

            // TODO: can be implemented once we have a struct DeathKillEvent that can be used as template argument to DrawableEventLister
        }

    protected:

        DeathKillEventLister(const DeathKillEventLister&) = delete;
        DeathKillEventLister& operator=(const DeathKillEventLister&) = delete;
        DeathKillEventLister(DeathKillEventLister&&) = delete;
        DeathKillEventLister&& operator=(DeathKillEventLister&&) = delete;

    private:

        // ---------------------------------------------------------------------------

    }; // class DeathKillEventLister

} // namespace proofps_dd
