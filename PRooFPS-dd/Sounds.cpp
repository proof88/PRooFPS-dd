/*
    ###################################################################################
    Sounds.h
    Sounds for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "Sounds.h"


// ############################### PUBLIC ################################


const char* proofps_dd::Sounds::getLoggerModuleName()
{
    return "Sounds";
}

CConsole& proofps_dd::Sounds::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::Sounds::Sounds()
{
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################

