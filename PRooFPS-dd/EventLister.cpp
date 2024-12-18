/*
    ###################################################################################
    EventLister.cpp
    Event lister class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "EventLister.h"

// ############################### PUBLIC ################################


const char* proofps_dd::EventLister::getLoggerModuleName()
{
    return "EventLister";
}

CConsole& proofps_dd::EventLister::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::EventLister::EventLister(
    const unsigned int& nEventTimeLimitSecs,
    const size_t& nEventCountLimit,
    const Orientation& eOrientation) :
    m_qEvents(nEventCountLimit),
    m_nEventTimeLimitSecs(nEventTimeLimitSecs),
    //m_nEventCountLimit(nEventCountLimit),
    m_eOrientation(eOrientation)
{
}

const unsigned int& proofps_dd::EventLister::getEventTimeLimitSecs() const
{
    return m_nEventTimeLimitSecs;
}

//const size_t& proofps_dd::EventLister::getEventCountLimit() const
//{
//    return m_nEventCountLimit;
//}

const size_t& proofps_dd::EventLister::getEventCountLimit() const
{
    return m_qEvents.capacity();
}

const proofps_dd::EventLister::Orientation& proofps_dd::EventLister::getOrientation() const
{
    return m_eOrientation;
}

void proofps_dd::EventLister::show()
{
    m_bVisible = true;
}

void proofps_dd::EventLister::hide()
{
    m_bVisible = false;
}

bool proofps_dd::EventLister::visible() const
{
    return m_bVisible;
}

//void proofps_dd::EventLister::update()
//{
//    // expected to be invoked every frame
//
//    auto it = m_qEvents.begin();
//    while (it != m_qEvents.end())
//    {
//        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - it->first).count() >= m_nEventTimeLimitSecs)
//        {
//            //getConsole().EOLn("Expired, removing: %s", it->second.c_str());
//            it = m_qEvents.erase(it);
//        }
//        else
//        {
//            it++;
//        }
//    }
//}

void proofps_dd::EventLister::update()
{
    // expected to be invoked every frame

    // iterating from oldest to newest elem in the queue
    //size_t i = m_qEvents.begin_index();
    //for (size_t n = 0; n < m_qEvents.size(); n++)
    //{
    //    const auto& elem = m_qEvents.underlying_array()[i];
    //    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - elem.first).count() >= m_nEventTimeLimitSecs)
    //    {
    //        // TODO: OPT: return in false branch
    //        m_qEvents.pop_front();
    //    }
    //    i = m_qEvents.next_index(i);
    //}

    // this looks cleaner way:
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

//void proofps_dd::EventLister::addEvent(const std::string& sEvent)
//{
//    if (m_nEventCountLimit && (m_qEvents.size() == m_nEventCountLimit))
//    {
//        m_qEvents.pop_front();
//    }
//    
//    m_qEvents.push_back(
//        { std::chrono::steady_clock::now(), sEvent} );
//}

//void proofps_dd::EventLister::addEvent(const std::string& sEvent)
//{
//    m_qEvents.push_back_forced(
//        { std::chrono::steady_clock::now(), sEvent });
//}

void proofps_dd::EventLister::addEvent(std::string&& sEvent)
{
    m_qEvents.push_back_forced(
        { std::move(std::chrono::steady_clock::now()), std::move(sEvent) });
}

void proofps_dd::EventLister::clear()
{
    m_qEvents.clear();
}

//const std::deque<proofps_dd::EventLister::TimeStringPair>& proofps_dd::EventLister::getEvents() const
//{
//    return m_qEvents;
//}

const pfl::FixFIFO<proofps_dd::EventLister::TimeStringPair>& proofps_dd::EventLister::getEvents() const
{
    return m_qEvents;
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################

