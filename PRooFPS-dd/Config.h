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

        const unsigned int& getReconnectDelaySeconds() const;

        const bool& getCameraFollowsPlayerAndXHair() const;
        const bool& getCameraTilting() const;

    protected:

    private:

        PGE& m_pge;
        Maps& m_maps;

        unsigned int m_nTickrate;
        unsigned int m_nPhysicsRateMin;
        unsigned int m_nClientUpdateRate;
        unsigned int m_nSecondsReconnectDelay;

        bool m_bCamFollowsXHair;
        bool m_bCamTilting;

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
