/*
    ###################################################################################
    GameMode.cpp
    Game Mode class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "GameMode.h"

using namespace proofps_dd;

/*
   ###########################################################################
   GameMode
   ###########################################################################
*/


// ############################### PUBLIC ################################


const char* GameMode::getLoggerModuleName()
{
    return "GameMode";
}

GameMode* GameMode::createGameMode(GameModeType gm)
{
    if (gm != GameModeType::DeathMatch)
    {
        // currently we dont support other game mode type
        return nullptr;
    }

    return new DeathMatchMode();
}

CConsole& GameMode::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

GameMode::~GameMode()
{
}

GameModeType GameMode::getGameModeType() const
{
    return m_gameModeType;
}

const std::chrono::time_point<std::chrono::steady_clock>& GameMode::getResetTime() const
{
    return m_timeReset;
}

void GameMode::Reset()
{
    m_timeReset = std::chrono::steady_clock::now();
}

const std::chrono::time_point<std::chrono::steady_clock>& GameMode::getWinTime() const
{
    return m_timeWin;
}


// ############################## PROTECTED ##############################


GameMode::GameMode(GameModeType gm) :
    m_gameModeType(gm)
{
}


// ############################### PRIVATE ###############################


/*
   ###########################################################################
   DeathMatchMode
   ###########################################################################
*/


// ############################### PUBLIC ################################


DeathMatchMode::DeathMatchMode() :
    GameMode(GameModeType::DeathMatch),
    m_nTimeLimitSecs(0),
    m_nFragLimit(0)
{
}

DeathMatchMode::~DeathMatchMode()
{
}

void DeathMatchMode::Reset()
{
    GameMode::Reset();
    m_players.clear();
    m_timeWin = std::chrono::time_point<std::chrono::steady_clock>(); // reset back to epoch
}

bool DeathMatchMode::checkWinningConditions()
{
    if ((getTimeLimitSecs() == 0) && (getFragLimit() == 0))
    {
        return false;
    }
    
    if (getTimeLimitSecs() > 0)
    {
        const auto durationSecsSinceReset = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - getResetTime());
        if (durationSecsSinceReset.count() >= getTimeLimitSecs())
        {
            if (m_timeWin.time_since_epoch().count() == 0)
            {
                m_timeWin = std::chrono::steady_clock::now();
            }
            return true;
        }
    }

    if (getFragLimit() > 0)
    {
        if (m_players.size() > 0)
        {
            // assume m_players is sorted, since UpdatePlayerData() sorts it
            if (m_players[0].m_nFrags >= static_cast<int>(getFragLimit()))
            {
                if (m_timeWin.time_since_epoch().count() == 0)
                {
                    m_timeWin = std::chrono::steady_clock::now();
                }
                return true;
            }
        }
    }

    return false;
}

unsigned int DeathMatchMode::getTimeLimitSecs() const
{
    return m_nTimeLimitSecs;
}

void DeathMatchMode::SetTimeLimitSecs(unsigned int secs)
{
    m_nTimeLimitSecs = secs;
}

unsigned int DeathMatchMode::getFragLimit() const
{
    return m_nFragLimit;
}

void DeathMatchMode::SetFragLimit(unsigned int limit)
{
    m_nFragLimit = limit;
}

const std::vector<FragTableRow>& proofps_dd::DeathMatchMode::getPlayerData() const
{
    return m_players;
}

void proofps_dd::DeathMatchMode::UpdatePlayerData(const std::vector<FragTableRow>& players)
{
    m_players = players;

    std::sort(
        m_players.begin(), m_players.end(), [](const FragTableRow& a, const FragTableRow& b)
        { return (a.m_nFrags > b.m_nFrags) || ((a.m_nFrags == b.m_nFrags) && (a.m_nDeaths < b.m_nDeaths)); }
    );
}



// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################



