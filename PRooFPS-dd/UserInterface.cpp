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
    // But they can used in other functions.
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
