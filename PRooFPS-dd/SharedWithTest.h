#pragma once

/*
    ###################################################################################
    SharedWithTest.h
    Some stuff in common with game code and test code.
    Made by PR00F88, West Whiskhyll Entertainment
    2022 - 2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <string>

namespace proofps_dd
{
    static constexpr char* GAME_REG_TEST_DUMP_FILE_SERVER_FMT = "RegTestDumpServer_tickrate_%u_cl_updaterate_%u.txt";
    static constexpr char* GAME_REG_TEST_DUMP_FILE_CLIENT_FMT = "RegTestDumpClient_tickrate_%u_cl_updaterate_%u.txt";

    std::string generateTestDumpFilename(bool bServer, unsigned int nTickRate, unsigned int nClUpdateRate);
} // namespace proofps_dd