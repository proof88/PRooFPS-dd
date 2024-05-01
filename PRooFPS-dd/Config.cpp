/*
    ###################################################################################
    Config.cpp
    Configuration Handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "Config.h"

#include <cassert>

#include "Player.h"

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
    // TODO: add an optional string argument containing the CVAR name, in that case only that CVAR and its dependers will be validated!

    // TODO: too much validation here, validation probably should be done by CVARS themselves.
    // Update this code after implementing: https://github.com/proof88/PRooFPS-dd/issues/251 .

    const bool bPrevLoggingState = getConsole().getLoggingState(getLoggerModuleName());
    getConsole().SetLoggingState(getLoggerModuleName(), true);

    getConsole().OLnOI("Config validation ...");

    if (!m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsString().empty())
    {
        if ((m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsInt() >= GAME_MAXFPS_MIN) &&
            (m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsUInt() <= GAME_MAXFPS_MAX))
        {
            getConsole().OLn("MaxFPS from config: %u FPS", m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsUInt());
        }
        else
        {
            getConsole().EOLn("ERROR: Invalid MaxFPS in config: %s, forcing default: %d FPS",
                m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsString().c_str(),
                GAME_MAXFPS_DEF);
            m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].Set(GAME_MAXFPS_DEF);
        }
    }
    else
    {
        m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].Set(GAME_MAXFPS_DEF);
        getConsole().OLn("Missing MaxFPS in config, forcing default: %d FPS", GAME_MAXFPS_DEF);
    }

    if (!m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsString().empty())
    {
        if ((m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsInt() >= GAME_TICKRATE_MIN) &&
            (m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsInt() <= GAME_TICKRATE_MAX))
        {
            if (m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsUInt() > 0)
            {
                if (m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsInt() > m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsInt())
                {
                    m_nTickrate = m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsUInt();
                    getConsole().EOLn("ERROR: Tickrate greater than MaxFPS in config: %s, forcing to: %u Hz",
                        m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsString().c_str(),
                        m_nTickrate);
                    m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].Set(m_nTickrate);
                }
                else
                {
                    m_nTickrate = m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsUInt();
                    getConsole().OLn("Tickrate from config: %u Hz", m_nTickrate);
                }
            }
            else
            {
                m_nTickrate = m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsUInt();
                getConsole().OLn("Tickrate from config: %u Hz", m_nTickrate);
            }
        }
        else
        {
            // we cannot default tickrate greater than configured maxFPS
            if (m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsUInt() > 0)
            {
                m_nTickrate = std::min(GAME_TICKRATE_DEF, static_cast<int>(m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsUInt()) /* int cannot overflow due to GAME_MAXFPS_MAX above */);
            }
            else
            {
                m_nTickrate = GAME_TICKRATE_DEF;
            }
            
            getConsole().EOLn("ERROR: Invalid Tickrate in config: %s, forcing to: %u Hz",
                m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].getAsString().c_str(),
                m_nTickrate);
            m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].Set(m_nTickrate);
        }
    }
    else
    {
        // we cannot default tickrate greater than configured maxFPS
        if (m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsUInt() > 0)
        {
            m_nTickrate = std::min(GAME_TICKRATE_DEF, static_cast<int>(m_pge.getConfigProfiles().getVars()[CVAR_FPS_MAX].getAsUInt()) /* int cannot overflow due to GAME_MAXFPS_MAX above */);
        }
        else
        {
            m_nTickrate = GAME_TICKRATE_DEF;
        }

        m_pge.getConfigProfiles().getVars()[CVAR_TICKRATE].Set(m_nTickrate);
        getConsole().OLn("Missing Tickrate in config, forcing to: %u Hz", m_nTickrate);
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
            getConsole().EOLn("ERROR: Invalid Min. physics rate in config: %s, forcing to Tickrate: %u Hz",
                m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].getAsString().c_str(),
                m_nTickrate);
            m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].Set(m_nTickrate);
        }
    }
    else
    {
        m_pge.getConfigProfiles().getVars()[CVAR_PHYSICS_RATE_MIN].Set(m_nTickrate);
        m_nPhysicsRateMin = m_nTickrate;
        getConsole().OLn("Missing Min. physics rate in config, forcing to Tickrate: %u Hz", m_nPhysicsRateMin);
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
            getConsole().EOLn("ERROR: Invalid Client update rate in config: %s, forcing to Tickrate: %u Hz",
                m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].getAsString().c_str(),
                m_nClientUpdateRate);
            m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].Set(m_nTickrate);
        }
    }
    else
    {
        m_pge.getConfigProfiles().getVars()[CVAR_CL_UPDATERATE].Set(m_nTickrate);
        m_nClientUpdateRate = m_nTickrate;
        getConsole().OLn("Missing Client update rate in config, forcing to Tickrate: %u Hz", m_nClientUpdateRate);
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

    if (!m_pge.getConfigProfiles().getVars()[Player::szCVarSvSomersaultMidAirJumpForceMultiplier].getAsString().empty())
    {
        if ((m_pge.getConfigProfiles().getVars()[Player::szCVarSvSomersaultMidAirJumpForceMultiplier].getAsFloat() >= Player::fSomersaultMidAirJumpForceMultiplierMin) &&
            (m_pge.getConfigProfiles().getVars()[Player::szCVarSvSomersaultMidAirJumpForceMultiplier].getAsFloat() <= Player::fSomersaultMidAirJumpForceMultiplierMax))
        {
            m_fSomersaultMidAirJumpForceMultiplier = m_pge.getConfigProfiles().getVars()[Player::szCVarSvSomersaultMidAirJumpForceMultiplier].getAsFloat();
            getConsole().OLn("Somersault Mid-Air Jump Force Multiplier from config: %f", m_fSomersaultMidAirJumpForceMultiplier);
        }
        else
        {
            m_fSomersaultMidAirJumpForceMultiplier = Player::fSomersaultMidAirJumpForceMultiplierDef;
            getConsole().EOLn("ERROR: Invalid Somersault Mid-Air Jump Force Multiplier in config: %s, forcing default: %f",
                m_pge.getConfigProfiles().getVars()[Player::szCVarSvSomersaultMidAirJumpForceMultiplier].getAsString().c_str(),
                m_fSomersaultMidAirJumpForceMultiplier);
            m_pge.getConfigProfiles().getVars()[Player::szCVarSvSomersaultMidAirJumpForceMultiplier].Set(Player::fSomersaultMidAirJumpForceMultiplierDef);
        }
    }
    else
    {
        m_pge.getConfigProfiles().getVars()[Player::szCVarSvSomersaultMidAirJumpForceMultiplier].Set(Player::fSomersaultMidAirJumpForceMultiplierDef);
        m_fSomersaultMidAirJumpForceMultiplier = Player::fSomersaultMidAirJumpForceMultiplierDef;
        getConsole().OLn("Missing Somersault Mid-Air Jump Force Multiplier in config, forcing default: %f", m_fSomersaultMidAirJumpForceMultiplier);
    }
    
    m_bCamFollowsXHair = m_pge.getConfigProfiles().getVars()[CVAR_GFX_CAM_FOLLOWS_XHAIR].getAsBool();
    m_bCamTilting = m_pge.getConfigProfiles().getVars()[CVAR_GFX_CAM_TILTING].getAsBool();
    m_bCamRolling = m_pge.getConfigProfiles().getVars()[CVAR_GFX_CAM_ROLLING].getAsBool();

    // obviously for clients, m_nPlayerRespawnDelaySecs will be overrid when receiving MsgServerInfoFromServer, see: clientHandleServerInfoFromServer()
    if (!m_pge.getConfigProfiles().getVars()[Player::szCVarSvDmRespawnDelaySecs].getAsString().empty())
    {
        m_nPlayerRespawnDelaySecs = m_pge.getConfigProfiles().getVars()[Player::szCVarSvDmRespawnDelaySecs].getAsUInt();
        getConsole().OLn("Player Respawn Delay from config: %u seconds", m_nPlayerRespawnDelaySecs);
    }
    else
    {
        m_pge.getConfigProfiles().getVars()[Player::szCVarSvDmRespawnDelaySecs].Set(Player::nSvDmRespawnDelaySecs);
        m_nPlayerRespawnDelaySecs = Player::nSvDmRespawnDelaySecs;
        getConsole().OLn("Missing Player Respawn Delay in config, forcing default: %u seconds", m_nPlayerRespawnDelaySecs);
    }

    getConsole().OOOLn("Config validation finished!");

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

const float& proofps_dd::Config::getSomersaultMidAirJumpForceMultiplier() const
{
    return m_fSomersaultMidAirJumpForceMultiplier;
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

const bool& proofps_dd::Config::getCameraRolling() const
{
    return m_bCamRolling;
}

const unsigned int& proofps_dd::Config::getPlayerRespawnDelaySeconds() const
{
    return m_nPlayerRespawnDelaySecs;
}

bool proofps_dd::Config::clientHandleServerInfoFromServer(
    pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/,
    const proofps_dd::MsgServerInfoFromServer& msgServerInfo)
{
    if (m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("Config::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    m_serverInfo = msgServerInfo;
    m_nPlayerRespawnDelaySecs = m_serverInfo.m_nRespawnTimeSecs;

    const bool bPrevLoggingState = getConsole().getLoggingState(getLoggerModuleName());
    getConsole().SetLoggingState(getLoggerModuleName(), true);

    getConsole().OLnOI("Config::%s(): received the following server config:", __func__);
    getConsole().OLn("nMaxFps           : %u FPS", m_serverInfo.m_nMaxFps);
    getConsole().OLn("nTickrate         : %u Hz",  m_serverInfo.m_nTickrate);
    getConsole().OLn("nPhysicsRateMin   : %u Hz",  m_serverInfo.m_nPhysicsRateMin);
    getConsole().OLn("nClientUpdateRate : %u Hz",  m_serverInfo.m_nClientUpdateRate);
    getConsole().OLn("iGameModeType     : %d",     m_serverInfo.m_iGameModeType);
    getConsole().OLn("nFragLimit        : %u",     m_serverInfo.m_nFragLimit);
    getConsole().OLn("nTimeLimitSecs    : %u s",   m_serverInfo.m_nTimeLimitSecs);
    getConsole().OLn("nTimeRemainingSecs: %u s",   m_serverInfo.m_nTimeRemainingSecs);
    getConsole().OLn("nRespawnTimeSecs  : %u s",   m_serverInfo.m_nRespawnTimeSecs);
    getConsole().OLnOO("");

    getConsole().SetLoggingState(getLoggerModuleName(), bPrevLoggingState);

    return true;
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


proofps_dd::Config::Config(
    PGE& pge,
    proofps_dd::Maps& maps) :
    m_pge(pge),
    m_maps(maps),
    m_fSomersaultMidAirJumpForceMultiplier(Player::fSomersaultMidAirJumpForceMultiplierDef)
{
    // Note that the following should not be touched here as they are not fully constructed when we are here:
    // maps, pge
    // But they can be used in other functions.
}

proofps_dd::Config::~Config()
{
}
