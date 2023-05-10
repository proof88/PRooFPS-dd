/*
    ###################################################################################
    Networking.cpp
    Network handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>
#include <chrono>

#include "Networking.h"


// ############################### PUBLIC ################################


proofps_dd::Networking::Networking(
    proofps_dd::Durations& durations) : 
    /* due to virtual inheritance, we don't invoke ctor of PGE, PRooFPSddPGE invokes it only */
    m_nServerSideConnectionHandle(pge_network::ServerConnHandle),
    m_durations(durations)
{
}

CConsole& proofps_dd::Networking::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* proofps_dd::Networking::getLoggerModuleName()
{
    return "Networking";
}


// ############################## PROTECTED ##############################


bool proofps_dd::Networking::isMyConnection(const pge_network::PgeNetworkConnectionHandle& connHandleServerSide) const
{
    // TODO: it would be much better if this function was part of PGE and not application.
    // However, before any refactor could be done, PgeGsnClient::m_hConnectionServerSide should be properly filled, and then
    // getters could be added to classes to retrieve this info. Then we can have a function in PGE which can tell if this
    // is our connection or not.
    return m_nServerSideConnectionHandle == connHandleServerSide;
}


// ############################### PRIVATE ###############################
