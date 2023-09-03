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
    PGE& pge,
    proofps_dd::Durations& durations) : 
    m_nServerSideConnectionHandle(pge_network::ServerConnHandle),
    m_pge(pge),
    m_durations(durations)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations
    // But they can used in other functions.

    // Since this class is used to build up the PRooFPSddPGE class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. PRooFPSddPGE initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be extisting at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
    assert(!pge.isGameRunning());
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
    // However, before any refactor could be done, PgeGnsClient::m_hConnectionServerSide should be properly filled, and then
    // getters could be added to classes to retrieve this info. Then we can have a function in PGE which can tell if this
    // is our connection or not.
    return m_nServerSideConnectionHandle == connHandleServerSide;
}


// ############################### PRIVATE ###############################
