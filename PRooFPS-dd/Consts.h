#pragma once

/*
    ###################################################################################
    Consts.h
    Common Constants not fitting into a single place
    Made by PR00F88, West Whiskhyll Entertainment
    2022 - 2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

namespace proofps_dd
{

    static const std::string GAME_NAME = "PRooFPS-dd";
    static const std::string GAME_VERSION = "0.1.3.0 Private Beta";

    static constexpr int   GAME_MAXFPS = 60;
    static constexpr int   GAME_TICKRATE_DEFAULT = 20;
    static constexpr int   GAME_TICKRATE_MIN = 20;
    static constexpr int   GAME_TICKRATE_MAX = GAME_MAXFPS;

    static_assert(GAME_TICKRATE_MAX == GAME_MAXFPS, "Max tickrate is limited by max FPS since onGameRunning() freq is same as max FPS.");
    static_assert(GAME_TICKRATE_MIN <= GAME_TICKRATE_MAX, "Min tickrate should not be greater than max tickrate.");
    static_assert(GAME_TICKRATE_MIN <= GAME_TICKRATE_DEFAULT, "Min tickrate should not be greater than default tickrate");
    static_assert(GAME_TICKRATE_DEFAULT <= GAME_TICKRATE_MAX, "Max tickrate should not be smaller than default tickrate");

    static const float GAME_BLOCK_SIZE_X = 1.0f;
    static const float GAME_BLOCK_SIZE_Y = 1.0f;
    static const float GAME_BLOCK_SIZE_Z = 1.0f;

    static const float GAME_JUMP_GRAVITY_START = 20.f; // for tickrate 20, this is good, for tickrate 60, 19.f gives identical result

    static const float GAME_PLAYER_W = 0.95f;
    static const float GAME_PLAYER_H = 1.88f;
    static const unsigned GAME_PLAYER_RESPAWN_SECONDS = 3;

    static constexpr char* GAME_WPN_DEFAULT = "pistol.txt";

    static constexpr char* GAME_AUDIO_DIR = "gamedata/audio/";
    static constexpr char* GAME_MAPS_DIR = "gamedata/maps/";
    static constexpr char* GAME_TEXTURES_DIR = "gamedata/textures/";
    static constexpr char* GAME_WEAPONS_DIR = "gamedata/weapons/";
} // namespace proofps_dd
