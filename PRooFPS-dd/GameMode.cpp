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

const std::vector<FragTableRow>& GameMode::getPlayerData() const
{
    return m_players;
}

void GameMode::Text(PR00FsUltimateRenderingEngine& pure, const std::string& s, int x, int y) const
{
    pure.getUImanager().text(s, x, y)->SetDropShadow(true);
}

void GameMode::ShowObjectives(PR00FsUltimateRenderingEngine& pure, pge_network::PgeNetwork& network)
{
    const int nXPosPlayerName = 20;
    const int nXPosFrags = 200;
    const int nXPosDeaths = 250;
    int nYPosStart = pure.getWindow().getClientHeight() - 20;
    if (checkWinningConditions())
    {
        Text(pure, "Game Ended! Waiting for restart ...", nXPosPlayerName, nYPosStart);
        nYPosStart -= 2 * pure.getUImanager().getDefaultFontSize();
    }

    int nThisRowY = nYPosStart;
    Text(pure, "Player Name", nXPosPlayerName, nThisRowY);
    Text(pure, "Frags", nXPosFrags, nThisRowY);
    Text(pure, "Deaths", nXPosDeaths, nThisRowY);

    nThisRowY -= pure.getUImanager().getDefaultFontSize();
    Text(pure, "========================================================", nXPosPlayerName, nThisRowY);

    int i = 0;
    for (const auto& player : getPlayerData())
    {
        i++;
        nThisRowY = nYPosStart - (i + 1) * pure.getUImanager().getDefaultFontSize();
        Text(pure, player.m_sName, nXPosPlayerName, nThisRowY);
        Text(pure, std::to_string(player.m_nFrags), nXPosFrags, nThisRowY);
        Text(pure, std::to_string(player.m_nDeaths), nXPosDeaths, nThisRowY);
    }

    if (!network.isServer())
    {
        nThisRowY -= 2 * pure.getUImanager().getDefaultFontSize();
        Text(pure, "Ping: " + std::to_string(network.getClient().getPing(true)) + " ms",
            nXPosPlayerName, nThisRowY);

        nThisRowY -= pure.getUImanager().getDefaultFontSize();
        Text(pure, "Quality: local: " + std::to_string(network.getClient().getQualityLocal(false)) +
            "; remote: " + std::to_string(network.getClient().getQualityRemote(false)),
            nXPosPlayerName, nThisRowY);

        nThisRowY -= pure.getUImanager().getDefaultFontSize();
        Text(pure, "Tx Speed: " + std::to_string(std::lround(network.getClient().getTxByteRate(false))) +
            " Bps; Rx Speed: " + std::to_string(std::lround(network.getClient().getRxByteRate(false))) + " Bps",
            nXPosPlayerName, nThisRowY);

        nThisRowY -= pure.getUImanager().getDefaultFontSize();
        Text(pure, "Internal Queue Time: " + std::to_string(network.getClient().getInternalQueueTimeUSecs(false)) + " us",
            nXPosPlayerName, nThisRowY);
    }
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

void DeathMatchMode::UpdatePlayerData(const std::vector<FragTableRow>& players)
{
    m_players = players;

    std::sort(
        m_players.begin(), m_players.end(), [](const FragTableRow& a, const FragTableRow& b)
        { return (a.m_nFrags > b.m_nFrags) || ((a.m_nFrags == b.m_nFrags) && (a.m_nDeaths < b.m_nDeaths)); }
    );
}



// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################



