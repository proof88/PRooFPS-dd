/*
    ###################################################################################
    UserInterface.cpp
    User interface handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>
#include <chrono>

#include "UserInterface.h"


// ############################### PUBLIC ################################


proofps_dd::UserInterface::UserInterface(PGE& pge) :
    m_pge(pge)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge
    // But they can be used in other functions.

    // Since this class is used to build up the PRooFPSddPGE class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. PRooFPSddPGE initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be existing at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
    assert(!pge.isGameRunning());
}

CConsole& proofps_dd::UserInterface::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* proofps_dd::UserInterface::getLoggerModuleName()
{
    return "UserInterface";
}


// ############################## PROTECTED ##############################


void proofps_dd::UserInterface::Text(const std::string& s, int x, int y) const
{
    m_pge.getPure().getUImanager().text(s, x, y)->SetDropShadow(true);
}

void proofps_dd::UserInterface::AddText(const std::string& s, int x, int y) const
{
    m_pge.getPure().getUImanager().addText(s, x, y)->SetDropShadow(true);
}


// ############################### PRIVATE ###############################
