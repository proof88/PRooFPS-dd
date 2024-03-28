#pragma once

/*
    ###################################################################################
    Consts.h
    Common Constants not fitting into a single place
    Made by PR00F88, West Whiskhyll Entertainment
    2022 - 2023
    ###################################################################################
*/

namespace proofps_dd
{

    static const std::string GAME_NAME = "PRooFPS-dd";
    static const std::string GAME_VERSION = "0.2.3.0 Private Beta";

    // TODO: unsure why I'm not using unsigned for these. Anyway, these will need to be handled in different way anyway in near future,
    // to have object for each of these with validation rules defined in object: https://github.com/proof88/PRooFPS-dd/issues/251 .

    static constexpr int   GAME_MAXFPS = 60;
    static_assert(0 < GAME_MAXFPS, "Max FPS should be positive.");

    static constexpr char* GAME_WPN_DEFAULT = "pistol.txt";

    static constexpr char* GAME_AUDIO_DIR = "gamedata/audio/";
    static constexpr char* GAME_MODELS_DIR = "gamedata/models/";
    static constexpr char* GAME_TEXTURES_DIR = "gamedata/textures/";
    static constexpr char* GAME_WEAPONS_DIR = "gamedata/weapons/";

} // namespace proofps_dd
