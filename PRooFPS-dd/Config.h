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

        float m_fSomersaultMidAirJumpForceMultiplier = GAME_SOMERSAULT_MID_AIR_JUMP_FORCE_MULTIPLIER_DEF;

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
