/*
    ###################################################################################
    SharedWithTest.cpp
    Some stuff in common with game code and test code.
    Made by PR00F88, West Whiskhyll Entertainment
    2022 - 2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "SharedWithTest.h"

namespace proofps_dd
{
    std::string generateTestDumpFilename(bool bServer, unsigned int nTickRate)
    {
        std::string sFilename = bServer ? proofps_dd::GAME_REG_TEST_DUMP_FILE_SERVER_FMT : proofps_dd::GAME_REG_TEST_DUMP_FILE_CLIENT_FMT;
        const auto iTickratePos = sFilename.find("%u");
        if (iTickratePos == std::string::npos)
        {
            return "Error: could not find position of tickrate in given string!";
        }
        sFilename.replace(iTickratePos, 2, std::to_string(nTickRate));
        return sFilename;
    }
} // namespace proofps_dd