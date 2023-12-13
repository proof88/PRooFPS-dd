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
    static const std::string GAME_VERSION = "0.1.6.0 Private Beta";

    // TODO: unsure why I'm not using unsigned for these. Anyway, these will need to be handled in different way anyway in near future,
    // to have object for each of these with validation rules defined in object: https://github.com/proof88/PRooFPS-dd/issues/251 .

    static constexpr int   GAME_MAXFPS = 60;
    static_assert(0 < GAME_MAXFPS, "Max FPS should be positive.");

    static constexpr int   GAME_TICKRATE_DEF = 60;
    static constexpr int   GAME_TICKRATE_MIN = 20;           /* Before modifying this, keep in mind serverGravity() is lerping between 20 and 60! */
    static constexpr int   GAME_TICKRATE_MAX = GAME_MAXFPS;
    static_assert(GAME_TICKRATE_MAX == GAME_MAXFPS, "Max tickrate is limited by max FPS since onGameRunning() freq is same as max FPS.");
    static_assert(0 < GAME_TICKRATE_MIN, "Min tickrate should be positive.");
    static_assert(GAME_TICKRATE_MIN <= GAME_TICKRATE_MAX, "Min tickrate should not be greater than max tickrate.");
    static_assert(GAME_TICKRATE_MIN <= GAME_TICKRATE_DEF, "Min tickrate should not be greater than default tickrate.");
    static_assert(GAME_TICKRATE_DEF <= GAME_TICKRATE_MAX, "Max tickrate should not be smaller than default tickrate.");

    static constexpr int   GAME_PHYSICS_RATE_MIN_DEF = 60;
    static constexpr int   GAME_PHYSICS_RATE_MIN_MIN = GAME_TICKRATE_DEF; /* There should be at least 1 physics iteration per tick. */
    static constexpr int   GAME_PHYSICS_RATE_MIN_MAX = 60;                /* Before modifying this, keep in mind serverGravity() is lerping between 20 and 60! */
    static_assert(GAME_PHYSICS_RATE_MIN_MIN <= GAME_PHYSICS_RATE_MIN_DEF, "Min physics_rate_min should not be greater than default physics_rate_min.");
    static_assert(GAME_PHYSICS_RATE_MIN_MIN <= GAME_PHYSICS_RATE_MIN_MAX, "Min physics_rate_min should not be greater than max physics_rate_min.");
    static_assert(GAME_PHYSICS_RATE_MIN_DEF <= GAME_PHYSICS_RATE_MIN_MAX, "Max physics_rate_min should not be smaller than default physics_rate_min.");
    static_assert(GAME_PHYSICS_RATE_MIN_MIN >= 20, "Physics::serverGravity() is lerping between 20 and 60!");
    static_assert(GAME_PHYSICS_RATE_MIN_DEF % GAME_TICKRATE_DEF == 0, "Physics update distribution in time must be constant/even.");

    static constexpr int   GAME_CL_UPDATERATE_DEF = 60;
    static constexpr int   GAME_CL_UPDATERATE_MIN = 1;
    static constexpr int   GAME_CL_UPDATERATE_MAX = GAME_TICKRATE_DEF;
    static_assert(
        GAME_CL_UPDATERATE_MAX == GAME_TICKRATE_DEF,
        "Max cl_updaterate is limited by tickrate since we should not update clients more frequently than actual update frequency :).");
    static_assert(0 < GAME_CL_UPDATERATE_MIN, "Min cl_updaterate should be positive.");
    static_assert(GAME_CL_UPDATERATE_MIN <= GAME_CL_UPDATERATE_MAX, "Min cl_updaterate should not be greater than max cl_updaterate.");
    static_assert(GAME_CL_UPDATERATE_MIN <= GAME_CL_UPDATERATE_DEF, "Min cl_updaterate should not be greater than default cl_updaterate.");
    static_assert(GAME_CL_UPDATERATE_DEF <= GAME_CL_UPDATERATE_MAX, "Max cl_updaterate should not be smaller than default cl_updaterate.");
    static_assert(GAME_TICKRATE_DEF % GAME_CL_UPDATERATE_DEF == 0, "Clients should receive UPDATED physics results evenly distributed in time.");

    static constexpr unsigned int GAME_NETWORK_RECONNECT_SECONDS = 2;

    static const float GAME_BLOCK_SIZE_X = 1.0f;
    static const float GAME_BLOCK_SIZE_Y = 1.0f;
    static const float GAME_BLOCK_SIZE_Z = 1.0f;

    /*
      For the future:
      for tickrate 20, this is good, for tickrate 60, 19.f gives identical result.
      However, in Physics::serverGravity(), I'm lerping not this but GAME_GRAVITY_CONST based on tickrate.
      I don't remember why I'm not lerping this between 19 and 20 but anyway that approach is also good.
    */
    static const float GAME_JUMP_GRAVITY_START = 20.f;

    static const float GAME_PLAYER_W = 0.95f;
    static const float GAME_PLAYER_H_STAND  = 1.88f;
    static const float GAME_PLAYER_H_CROUCH_SCALING_Y = 0.5f;
    static const unsigned GAME_PLAYER_RESPAWN_SECONDS = 3;

    static constexpr char* GAME_WPN_DEFAULT = "pistol.txt";

    static constexpr char* GAME_AUDIO_DIR = "gamedata/audio/";
    static constexpr char* GAME_TEXTURES_DIR = "gamedata/textures/";
    static constexpr char* GAME_WEAPONS_DIR = "gamedata/weapons/";
} // namespace proofps_dd
