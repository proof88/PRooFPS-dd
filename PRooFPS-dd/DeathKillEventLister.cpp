/*
    ###################################################################################
    DeathKillEventLister.cpp
    Death/Kill Event lister class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "DeathKillEventLister.h"

// ############################### PUBLIC ################################


const char* proofps_dd::DeathKillEventLister::getLoggerModuleName()
{
    return "DeathKillEventLister";
}

CConsole& proofps_dd::DeathKillEventLister::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::DeathKillEventLister::DeathKillEventLister(
    PR00FsUltimateRenderingEngine& gfx) :
    EventLister(gfx, 5 /* time limit secs */, 8 /* event count limit */)
{

}

void proofps_dd::DeathKillEventLister::addDeathKillEvent(const std::string& sKiller, const std::string& sKilled)
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


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################

