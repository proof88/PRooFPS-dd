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

    struct DeathKillEvent
    {
        // constructing and destructing this when TimeEventPair is temporal object, too expensive!
        // consider switching to a more lightweight string, like something implemented in Dear ImGui!

        std::string m_sKiller;
        ImVec4 m_clrKiller;
        std::string m_sKilled;
        ImVec4 m_clrKilled;

        std::string m_sAuxText;
        float m_fTextWidthPixels{0.f};

        DeathKillEvent()
        {}

        DeathKillEvent(
            const std::string& sKiller,
            const ImVec4& clrKiller,
            const std::string& sKilled,
            const ImVec4& clrKilled) :
            m_sKiller(sKiller),
            m_clrKiller(clrKiller),
            m_sKilled(sKilled),
            m_clrKilled(clrKilled)
        {
            // try to calculate as much as we can only once so these don't need to be dealt with every next few frames ...
            // TODO: UPDATE: we cannot properly calculate width here because when ctor runs, either the default or different font is active!
            //         So, temporarily we calculate the width only once in the GUI drawing code in GUI::updateDeathKillEvents()!
            if (m_sKiller.empty())
            {
                m_sAuxText = " died";
                //m_fTextWidthPixels = ImGui::CalcTextSize((m_sKilled + m_sAuxText).c_str()).x;
            }
            else
            {
                m_sAuxText = " killed ";
                //m_fTextWidthPixels = ImGui::CalcTextSize((m_sKiller + m_sAuxText + m_sKilled).c_str()).x;
            }
        }

        DeathKillEvent(
            std::string&& sKiller,
            ImVec4&& clrKiller,
            std::string&& sKilled,
            ImVec4&& clrKilled) :
            m_sKiller(std::move(sKiller)),
            m_clrKiller(std::move(clrKiller)),
            m_sKilled(std::move(sKilled)),
            m_clrKilled(std::move(clrKilled))
        {
            // try to calculate as much as we can only once so these don't need to be dealt with every next few frames ...
            // TODO: UPDATE: we cannot properly calculate width here because when ctor runs, either the default or different font is active!
            //         So, temporarily we calculate the width only once in the GUI drawing code in GUI::updateDeathKillEvents()!
            if (m_sKiller.empty())
            {
                m_sAuxText = " died";
                //m_fTextWidthPixels = ImGui::CalcTextSize((m_sKilled + m_sAuxText).c_str()).x;
            }
            else
            {
                m_sAuxText = " killed ";
                //m_fTextWidthPixels = ImGui::CalcTextSize((m_sKiller + m_sAuxText + m_sKilled).c_str()).x;
            }
        }

        void draw()
        {
            // TODO: cannot be implemented until drawTextHighlighted() is moved from GUI.cpp to separate unit.
        };
    };

    class DeathKillEventLister : public DrawableEventLister<DeathKillEvent>
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

        void addDeathKillEvent(
            const std::string& sKiller,
            const ImVec4& clrKiller,
            const std::string& sKilled,
            const ImVec4& clrKilled)
        {
            addEvent(std::move(DeathKillEvent(sKiller, clrKiller, sKilled, clrKilled)));
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
