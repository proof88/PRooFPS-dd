/*
    ###################################################################################
    SharedWithTest.cpp
    Some stuff in common with game code and test code.
    Made by PR00F88, West Whiskhyll Entertainment
    2022 - 2023
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "SharedWithTest.h"

namespace proofps_dd
{
    std::string generateTestDumpFilename(
        bool bServer, unsigned long nPid, unsigned int nTickRate, unsigned int nClUpdateRate, unsigned int nPhysicsRateMin)
    {
        std::string sFilename = bServer ? proofps_dd::GAME_REG_TEST_DUMP_FILE_SERVER_FMT : proofps_dd::GAME_REG_TEST_DUMP_FILE_CLIENT_FMT;
        
        auto iFindStrPos = sFilename.find("%u");
        if (iFindStrPos == std::string::npos)
        {
            return "Error: could not find position of pid in given string!";
        }
        sFilename.replace(iFindStrPos, 2, std::to_string(nPid));

        iFindStrPos = sFilename.find("%u");
        if (iFindStrPos == std::string::npos)
        {
            return "Error: could not find position of tickrate in given string!";
        }
        sFilename.replace(iFindStrPos, 2, std::to_string(nTickRate));

        iFindStrPos = sFilename.find("%u");
        if (iFindStrPos == std::string::npos)
        {
            return "Error: could not find position of cl_updaterate in given string!";
        }
        sFilename.replace(iFindStrPos, 2, std::to_string(nClUpdateRate));

        iFindStrPos = sFilename.find("%u");
        if (iFindStrPos == std::string::npos)
        {
            return "Error: could not find position of physics_rate_min in given string!";
        }
        sFilename.replace(iFindStrPos, 2, std::to_string(nPhysicsRateMin));

        return sFilename;
    }
} // namespace proofps_dd