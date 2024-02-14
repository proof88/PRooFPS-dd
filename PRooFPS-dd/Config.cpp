/*
    ###################################################################################
    Config.cpp
    Configuration Handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "Config.h"

#include <cassert>

#include "Consts.h"

static constexpr char* CVAR_SV_RECONNECT_DELAY = "sv_reconnect_delay";
static constexpr char* CVAR_CL_RECONNECT_DELAY = "cl_reconnect_delay";


// ############################### PUBLIC ################################


proofps_dd::Config& proofps_dd::Config::getConfigInstance(PGE& pge, proofps_dd::Maps& maps)
{
    // we are expecting a PGE instance which is also static since PGE is singleton, it looks ok a singleton object saves ref to a singleton object ...
    // Note that the following should not be touched here as they are not fully constructed when we are here:
    // maps, pge
    // But they can be used in other functions.
    static Config m_guiInstance(pge, maps);
    return m_guiInstance;
}

const char* proofps_dd::Config::getLoggerModuleName()
{
    return "Config";
}

CConsole& proofps_dd::Config::getConsole()
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

void proofps_dd::Config::validate()
{
    // TODO: too much validation here, validation probably should be done by CVARS themselves.
    // Update this code after implementing: https://github.com/proof88/PRooFPS-dd/issues/251 .

    const bool bPrevLoggingState = getConsole().getLoggingState(getLoggerModuleName());
    getConsole().SetLoggingState(getLoggerModuleName(), true);

    if (!m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsString().empty())
    {
        if ((m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsInt() >= GAME_TICKRATE_MIN) &&
            (m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsUInt() <= m_pge.getGameRunningFrequency()))
        {
            m_nTickrate = m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsUInt();
            getConsole().OLn("Tickrate from config: %u Hz", m_nTickrate);
        }
        else
        {
            m_nTickrate = GAME_TICKRATE_DEF;
            getConsole().EOLn("ERROR: Invalid Tickrate in config: %s, forcing default: %u Hz",
                m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsString().c_str(),
                m_nTickrate);
            m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].Set(GAME_TICKRATE_DEF);
        }
    }
    else
    {
        m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].Set(GAME_TICKRATE_DEF);
        m_nTickrate = GAME_TICKRATE_DEF;
        getConsole().OLn("Missing Tickrate in config, forcing default: %u Hz", m_nTickrate);
    }

    if (!m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsString().empty())
    {
        if ((m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsUInt() >= m_nTickrate) &&
            (m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsUInt() <= GAME_PHYSICS_RATE_MIN_MAX) &&
            /* Physics update distribution in time must be constant/even if we do it more frequently than tickrate. */
            (m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsUInt() % m_nTickrate == 0u))
        {
            m_nPhysicsRateMin = m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsUInt();
            getConsole().OLn("Min. physics rate from config: %u Hz", m_nPhysicsRateMin);
        }
        else
        {
            m_nPhysicsRateMin = m_nTickrate;
            getConsole().EOLn("ERROR: Invalid Min. physics rate in config: %s, forcing tickrate: %u Hz",
                m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsString().c_str(),
                m_nTickrate);
            m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].Set(m_nTickrate);
        }
    }
    else
    {
        m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].Set(GAME_PHYSICS_RATE_MIN_DEF);
        m_nPhysicsRateMin = GAME_PHYSICS_RATE_MIN_DEF;
        getConsole().OLn("Missing Min. physics rate in config, forcing default: %u Hz", m_nPhysicsRateMin);
    }

    if (!m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsString().empty())
    {
        if ((m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsInt() >= GAME_CL_UPDATERATE_MIN) &&
            (m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsUInt() <= m_nTickrate) &&
            /* Clients should receive UPDATED physics results evenly distributed in time. */
            (m_nTickrate % m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsUInt() == 0u))
        {
            m_nClientUpdateRate = m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsUInt();
            getConsole().OLn("Client update rate from config: %u Hz", m_nClientUpdateRate);
        }
        else
        {
            m_nClientUpdateRate = m_nTickrate;
            getConsole().EOLn("ERROR: Invalid Client update rate in config: %s, forcing tickrate: %u Hz",
                m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsString().c_str(),
                m_nClientUpdateRate);
            m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].Set(m_nTickrate);
        }
    }
    else
    {
        m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].Set(GAME_CL_UPDATERATE_DEF);
        m_nClientUpdateRate = GAME_CL_UPDATERATE_DEF;
        getConsole().OLn("Missing Client update rate in config, forcing default: %u Hz", m_nClientUpdateRate);
    }

    const char* const szCVarReconnectDelay = m_pge.getNetwork().isServer() ? CVAR_SV_RECONNECT_DELAY : CVAR_CL_RECONNECT_DELAY;

    if (!m_pge.getConfigProfiles().getVars()[szCVarReconnectDelay].getAsString().empty())
    {
        if (m_pge.getConfigProfiles().getVars()[szCVarReconnectDelay].getAsInt() >= 0)
        {
            m_nSecondsReconnectDelay = m_pge.getConfigProfiles().getVars()[szCVarReconnectDelay].getAsUInt();
            getConsole().OLn("%s from config: %u seconds", szCVarReconnectDelay, m_nSecondsReconnectDelay);
        }
        else
        {
            m_nSecondsReconnectDelay = GAME_NETWORK_RECONNECT_SECONDS;
            getConsole().EOLn("ERROR: Invalid %s in config: %s, forcing: %u seconds",
                szCVarReconnectDelay, m_pge.getConfigProfiles().getVars()[szCVarReconnectDelay].getAsString().c_str(),
                m_nSecondsReconnectDelay);
            m_pge.getConfigProfiles().getVars()[szCVarReconnectDelay].Set(GAME_NETWORK_RECONNECT_SECONDS);
        }
    }
    else
    {
        m_pge.getConfigProfiles().getVars()[szCVarReconnectDelay].Set(GAME_NETWORK_RECONNECT_SECONDS);
        m_nSecondsReconnectDelay = GAME_NETWORK_RECONNECT_SECONDS;
        getConsole().OLn("Missing %s, forcing default: %u seconds", szCVarReconnectDelay, m_nSecondsReconnectDelay);
    }

    if (m_pge.getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].getAsBool() &&
        !m_pge.getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR].getAsBool())
    {
        m_pge.getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].Set(false);
        getConsole().EOLn("ERROR: %s cannot be true when %s is false, forcing false!",
            CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL, CVAR_SV_ALLOW_STRAFE_MID_AIR);
    }
    
    m_bCamFollowsXHair = m_pge.getConfigProfiles().getVars()[CVAR_GFX_CAM_FOLLOWS_XHAIR].getAsBool();
    m_bCamTilting = m_pge.getConfigProfiles().getVars()[CVAR_GFX_CAM_TILTING].getAsBool();

    getConsole().SetLoggingState(getLoggerModuleName(), bPrevLoggingState);
} // initialize()

const unsigned int& proofps_dd::Config::getTickRate() const
{
    return m_nTickrate;
}

const unsigned int& proofps_dd::Config::getPhysicsRate() const
{
    return m_nPhysicsRateMin;
}

const unsigned int& proofps_dd::Config::getClientUpdateRate() const
{
    return m_nClientUpdateRate;
}

const unsigned int& proofps_dd::Config::getReconnectDelaySeconds() const
{
    return m_nSecondsReconnectDelay;
}

const bool& proofps_dd::Config::getCameraFollowsPlayerAndXHair() const
{
    return m_bCamFollowsXHair;
}

const bool& proofps_dd::Config::getCameraTilting() const
{
    return m_bCamTilting;
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


proofps_dd::Config::Config(
    PGE& pge,
    proofps_dd::Maps& maps) :
    m_pge(pge),
    m_maps(maps),
    m_nTickrate(GAME_TICKRATE_DEF),
    m_nPhysicsRateMin(GAME_PHYSICS_RATE_MIN_DEF),
    m_nClientUpdateRate(GAME_CL_UPDATERATE_DEF),
    m_nSecondsReconnectDelay(GAME_NETWORK_RECONNECT_SECONDS),
    m_bCamFollowsXHair(true),
    m_bCamTilting(true)
{
    // Note that the following should not be touched here as they are not fully constructed when we are here:
    // maps, pge
    // But they can be used in other functions.
}

proofps_dd::Config::~Config()
{
}
