#pragma once

/*
    ###################################################################################
    SharedWithTest.h
    Some stuff in common with game code and test code.
    Made by PR00F88, West Whiskhyll Entertainment
    2022 - 2023
    ###################################################################################
*/

#include <string>

namespace proofps_dd
{
    static constexpr char* GAME_REG_TEST_DUMP_FILE_SERVER_FMT = "RegTestDumpServer_pid_%u_tickrate_%u_cl_updaterate_%u_physics_rate_min_%u.txt";
    static constexpr char* GAME_REG_TEST_DUMP_FILE_CLIENT_FMT = "RegTestDumpClient_pid_%u_tickrate_%u_cl_updaterate_%u_physics_rate_min_%u.txt";

    std::string generateTestDumpFilename(bool bServer, unsigned long nPid, unsigned int nTickRate, unsigned int nClUpdateRate, unsigned int nPhysicsRateMin);
} // namespace proofps_dd