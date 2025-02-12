#pragma once

/*
    ###################################################################################
    ServerEventLister.h
    General Server Event lister class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2025
    ###################################################################################
*/

#include "DrawableEventLister.h"

namespace proofps_dd
{

    struct ServerEvent
    {
        enum class EventType
        {
            Connected,
            Disconnected,
            TeamChanged
        };

        // constructing and destructing this when TimeEventPair is temporal object, too expensive!
        // consider switching to a more lightweight string, like something implemented in Dear ImGui!

        EventType m_eEventType{};
        std::string m_sPlayerName;
        ImVec4 m_clrPlayerName;
        ImVec4 m_clrSecondary;

        std::string m_sAuxText;

        ServerEvent()
        {}

        ServerEvent(
            const std::string& sPlayerName,
            const ImVec4& clrPlayerName,
            const EventType& eventType) :
            m_eEventType(eventType),
            m_sPlayerName(sPlayerName),
            m_clrPlayerName(clrPlayerName)
        {
            switch (eventType)
            {
            case EventType::Connected:
                m_sAuxText = " connected";
                break;
            default:
                m_sAuxText = " disconnected";
                break;
            }
        }

        ServerEvent(
            const std::string& sPlayerName,
            const ImVec4& clrPlayerName,
            const unsigned int& iNewTeamId,
            const ImVec4& clrNewTeam) :
            m_eEventType(EventType::TeamChanged),
            m_sPlayerName(sPlayerName),
            m_clrPlayerName(clrPlayerName),
            m_clrSecondary(clrNewTeam)
        {
            m_sAuxText = " joined Team " + std::to_string(iNewTeamId);
        }

        ServerEvent(
            std::string&& sPlayerName,
            ImVec4&& clrPlayerName,
            const EventType& eventType) :
            m_eEventType(eventType),
            m_sPlayerName(std::move(sPlayerName)),
            m_clrPlayerName(std::move(clrPlayerName))
        {
            switch (eventType)
            {
            case EventType::Connected:
                m_sAuxText = " connected";
                break;
            default:
                m_sAuxText = " disconnected";
                break;
            }
        }

        ServerEvent(
            std::string&& sPlayerName,
            ImVec4&& clrPlayerName,
            const unsigned int& iNewTeamId,
            ImVec4&& clrNewTeam) :
            m_eEventType(EventType::TeamChanged),
            m_sPlayerName(std::move(sPlayerName)),
            m_clrPlayerName(std::move(clrPlayerName)),
            m_clrSecondary(std::move(clrNewTeam))
        {
            m_sAuxText = " joined Team " + std::to_string(iNewTeamId);
        }

        void draw()
        {
            // TODO: cannot be implemented until drawTextHighlighted() is moved from GUI.cpp to separate unit.
        };
    };

    class ServerEventLister : public DrawableEventLister<ServerEvent>
    {
    public:

        static const char* getLoggerModuleName()
        {
            return "ServerEventLister";
        }

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const
        {
            return CConsole::getConsoleInstance(getLoggerModuleName());
        }

        ServerEventLister() :
            DrawableEventLister(
                3 /* time limit secs */,
                5 /* event count limit */,
                Orientation::Vertical)
        {}

        void addConnectedEvent(
            const std::string& sName)
        {
            addEvent(std::move(ServerEvent(sName, ImVec4(1.f, 1.f, 1.f, 1.f), ServerEvent::EventType::Connected)));
        }

        void addDisconnectedEvent(
            const std::string& sName,
            const ImVec4& clrName)
        {
            addEvent(std::move(ServerEvent(sName, clrName, ServerEvent::EventType::Disconnected)));
        }

        void addTeamChangedEvent(
            const std::string& sName,
            const ImVec4& clrName,
            const unsigned int& iNewTeamId,
            const ImVec4& clrNewTeam)
        {
            addEvent(std::move(ServerEvent(sName, clrName, iNewTeamId, clrNewTeam)));
        }

    protected:

        ServerEventLister(const ServerEventLister&) = delete;
        ServerEventLister& operator=(const ServerEventLister&) = delete;
        ServerEventLister(ServerEventLister&&) = delete;
        ServerEventLister&& operator=(ServerEventLister&&) = delete;

    private:

        // ---------------------------------------------------------------------------

    }; // class ServerEventLister

} // namespace proofps_dd
