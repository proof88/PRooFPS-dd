#pragma once

/*
    ###################################################################################
    Config.h
    Configuration Handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include <functional>
#include <string>

#include "CConsole.h"

#include "PGE.h"

#include "Consts.h"
#include "Maps.h"

namespace proofps_dd
{
    // TODO: unsure why I'm not using unsigned for these. Anyway, these will need to be handled in different way anyway in near future,
    // to have object for each of these with validation rules defined in object: https://github.com/proof88/PRooFPS-dd/issues/251 .
    static constexpr int   GAME_TICKRATE_DEF = 60;
    static constexpr int   GAME_TICKRATE_MIN = 20;     /* Before modifying this, keep in mind serverGravity() is lerping between 20 and 60! */
    static constexpr int   GAME_TICKRATE_MAX = 60;     /* Before modifying this, keep in mind serverGravity() is lerping between 20 and 60! */
    static_assert(GAME_TICKRATE_MAX <= GAME_MAXFPS_MAX, "Max tickrate is limited by max MaxFPS since onGameRunning() freq is same as max FPS.");
    static_assert(0 < GAME_TICKRATE_MIN, "Min tickrate must be positive.");
    static_assert(GAME_TICKRATE_MIN < GAME_TICKRATE_MAX, "Min tickrate should not less than max tickrate (if equal, division by zero happens in Physics::serverGravity()).");
    static_assert(GAME_TICKRATE_MIN <= GAME_TICKRATE_DEF, "Min tickrate should not be greater than default tickrate.");
    static_assert(GAME_TICKRATE_DEF <= GAME_TICKRATE_MAX, "Max tickrate should not be smaller than default tickrate.");

    static constexpr int   GAME_PHYSICS_RATE_MIN_DEF = 60;
    static constexpr int   GAME_PHYSICS_RATE_MIN_MIN = GAME_TICKRATE_DEF; /* There should be at least 1 physics iteration per tick. */
    static constexpr int   GAME_PHYSICS_RATE_MIN_MAX = 60;                /* Before modifying this, keep in mind serverGravity() is lerping between 20 and 60! */
    static_assert(GAME_PHYSICS_RATE_MIN_MIN <= GAME_PHYSICS_RATE_MIN_DEF, "Min physics_rate_min should not be greater than default physics_rate_min.");
    static_assert(GAME_PHYSICS_RATE_MIN_MIN <= GAME_PHYSICS_RATE_MIN_MAX, "Min physics_rate_min should not be greater than max physics_rate_min.");
    static_assert(GAME_PHYSICS_RATE_MIN_DEF <= GAME_PHYSICS_RATE_MIN_MAX, "Max physics_rate_min should not be smaller than default physics_rate_min.");
    static_assert(GAME_PHYSICS_RATE_MIN_MIN >= 20, "Physics::serverGravity() is lerping between 20 and 60!");
    static_assert(GAME_PHYSICS_RATE_MIN_DEF% GAME_TICKRATE_DEF == 0, "Physics update distribution in time must be constant/even.");

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

    static constexpr char* CVAR_FPS_MAX = "gfx_fps_max";
    static constexpr char* CVAR_TICKRATE = "tickrate";
    static constexpr char* CVAR_PHYSICS_RATE_MIN = "physics_rate_min";
    static constexpr char* CVAR_CL_UPDATERATE = "cl_updaterate";

    static constexpr unsigned int GAME_NETWORK_RECONNECT_SECONDS = 2;

    static constexpr char* CVAR_SV_ALLOW_STRAFE_MID_AIR = "sv_allow_strafe_mid_air";
    static constexpr char* CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL = "sv_allow_strafe_mid_air_full";

    static constexpr char* CVAR_GFX_CAM_FOLLOWS_XHAIR = "gfx_cam_follows_xhair";
    static constexpr char* CVAR_GFX_CAM_TILTING = "gfx_cam_tilting";
    static constexpr char* CVAR_GFX_CAM_ROLLING = "gfx_cam_rolling";

    class Config
    {
    public:

        static Config& getConfigInstance(
            PGE& pge,
            proofps_dd::Maps& maps);   /**< Gets the singleton instance. */

        static const char* getLoggerModuleName();
        static CConsole& getConsole();

        // ---------------------------------------------------------------------------

        void validate();

        const unsigned int& getTickRate() const;
        const unsigned int& getPhysicsRate() const;
        const unsigned int& getClientUpdateRate() const;

        const float& getSomersaultMidAirJumpForceMultiplier() const;

        const unsigned int& getReconnectDelaySeconds() const;

        const bool& getCameraFollowsPlayerAndXHair() const;
        const bool& getCameraTilting() const;
        const bool& getCameraRolling() const;

    protected:

    private:

        PGE& m_pge;
        Maps& m_maps;

        unsigned int m_nTickrate = GAME_TICKRATE_DEF;
        unsigned int m_nPhysicsRateMin = GAME_PHYSICS_RATE_MIN_DEF;
        unsigned int m_nClientUpdateRate = GAME_CL_UPDATERATE_DEF;
        unsigned int m_nSecondsReconnectDelay = GAME_NETWORK_RECONNECT_SECONDS;

        float m_fSomersaultMidAirJumpForceMultiplier /* initialization postponed to .cpp ctor so I dont need to include Player.h here */;

        bool m_bCamFollowsXHair = true;
        bool m_bCamTilting = true;
        bool m_bCamRolling = true;

        // ---------------------------------------------------------------------------

        Config(
            PGE& pge,
            proofps_dd::Maps& maps);
        ~Config();

        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;
        Config(Config&&) = delete;
        Config&& operator=(Config&&) = delete;

    }; // class Config

} // namespace proofps_dd
