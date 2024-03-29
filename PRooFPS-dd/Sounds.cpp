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


void proofps_dd::Sounds::loadSound(SoLoud::Wav& snd, const char* fname)
{
    const SoLoud::result resSoloud = snd.load(fname);
    if (resSoloud == SoLoud::SOLOUD_ERRORS::SO_NO_ERROR)
    {
        getConsole().OLn("%s: %s loaded, length: %f secs!", __func__, fname, snd.getLength());
    }
    else
    {
        getConsole().EOLn("%s: %s load error: %d!", __func__, fname, resSoloud);
    }
}


// ############################### PRIVATE ###############################

