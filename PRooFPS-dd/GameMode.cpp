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

const std::list<FragTableRow>& GameMode::getFragTable() const
{
    return m_players;
}

void GameMode::Text(PR00FsUltimateRenderingEngine& pure, const std::string& s, int x, int y) const
{
    pure.getUImanager().text(s, x, y)->SetDropShadow(true);
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
    m_nFragLimit(0),
    m_bWon(false)
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
    m_bWon = false;
}

bool DeathMatchMode::checkWinningConditions()
{
    if (m_bWon)
    {
        // once it is won, it stays won until next Reset()
        return m_bWon;
    }
    
    if (getTimeLimitSecs() > 0)
    {
        if (getTimeRemainingSecs() == 0)
        {
            m_bWon = true;
            m_timeWin = std::chrono::steady_clock::now();
            return true;
        }
    }

    if (getFragLimit() > 0)
    {
        // assume m_players is sorted, since add/updatePlayer() use insertion sort
        if ((m_players.size() > 0) && (m_players.begin()->m_nFrags >= static_cast<int>(getFragLimit())))
        {
            m_bWon = true;
            m_timeWin = std::chrono::steady_clock::now();
            return true;
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

unsigned int proofps_dd::DeathMatchMode::getTimeRemainingSecs() const
{
    if (m_bWon || (getResetTime().time_since_epoch().count() == 0) || (getTimeLimitSecs() == 0))
    {
        return 0;
    }
    return getTimeLimitSecs() - static_cast<unsigned int>((std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - getResetTime())).count());
}

unsigned int DeathMatchMode::getFragLimit() const
{
    return m_nFragLimit;
}

void DeathMatchMode::SetFragLimit(unsigned int limit)
{
    m_nFragLimit = limit;
}

bool DeathMatchMode::addPlayer(const Player& player)
{
    bool bRet = true;
    auto it = m_players.cbegin();
    while (bRet &&
        (it != m_players.cend()) &&
        (comparePlayers(it->m_nFrags, player.getFrags(), it->m_nDeaths, player.getDeaths()) <= 0))
    {
        bRet = it->m_sName != player.getName();
        ++it;
    }

    // even though we not yet found a player with same name, we need to check remaining players, before inserting this one ...
    auto itCheck = it;
    while (bRet &&
        (itCheck != m_players.cend()))
    {
        bRet = itCheck->m_sName != player.getName();
        ++itCheck;
    }

    if (bRet)
    {
        m_players.insert(it, FragTableRow{ player.getName(), player.getFrags(), player.getDeaths() });
    }

    checkWinningConditions();  // to make sure winning time is updated if game has just been won!
    return bRet;
}

bool DeathMatchMode::updatePlayer(const Player& player)
{
    const auto itFound = std::find_if(
        std::begin(m_players),
        std::end(m_players),
        [&player](const auto& row) { return row.m_sName == player.getName(); });
    if (itFound == m_players.end())
    {
        // this player doesn't even exist in the frag table
        return false;
    }

    if ((player.getFrags() == itFound->m_nFrags) && (player.getDeaths() == itFound->m_nDeaths))
    {
        // nothing to update
        return true;
    }

    // In case of players having equal nFrags and nDeaths, we need to apply different rules based on how values are being updated:
    // - if new nFrags > old nFrags, player has to be placed behind other players having equal nFrags, since they had it earlier;
    // - if new nFrags < old nFrags, player has to be placed in front of other players having equals nFrags, since player had more earlier.

    const bool bPutBehindEquals = player.getFrags() > itFound->m_nFrags;
    auto it = m_players.begin();
    if (bPutBehindEquals)
    {
        while ((it != m_players.end())
            && (comparePlayers(it->m_nFrags, player.getFrags(), it->m_nDeaths, player.getDeaths()) <= 0))
        {
            ++it;
        }
    }
    else
    {
        while ((it != m_players.end())
            && (comparePlayers(it->m_nFrags, player.getFrags(), it->m_nDeaths, player.getDeaths()) < 0))
        {
            ++it;
        }
    }

    if (itFound != it)
    {
        // move 'itFound' before 'it' (FragTableRow is not copied, only internal pointers are updated)
        m_players.splice(it, m_players, itFound);
    }
    itFound->m_nFrags = player.getFrags();
    itFound->m_nDeaths = player.getDeaths();

    checkWinningConditions();  // to make sure winning time is updated if game has just been won!
    return true;
}

bool DeathMatchMode::removePlayer(const Player& player)
{
    const auto itFound = std::find_if(
        std::begin(m_players),
        std::end(m_players),
        [&player](const auto& row) { return row.m_sName == player.getName(); });
    if (itFound == m_players.end())
    {
        // this player doesn't even exist in the frag table
        return false;
    }

    m_players.erase(itFound);

    return true;
}

void DeathMatchMode::ShowObjectives(PR00FsUltimateRenderingEngine& pure, pge_network::PgeNetwork& network)
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
    else
    {
        std::string sLimits;
        if (getFragLimit() > 0)
        {
            sLimits = "Frag Limit: " + std::to_string(getFragLimit());
        }
        if (getTimeLimitSecs() > 0)
        {
            if (!sLimits.empty())
            {
                sLimits += ", ";
            }
            sLimits += "Time Limit: " + std::to_string(getTimeLimitSecs()) + " s, Remaining: " + std::to_string(getTimeRemainingSecs()) + " s";
        }
        if (!sLimits.empty())
        {
            Text(pure, sLimits, nXPosPlayerName, nYPosStart);
            nYPosStart -= 2 * pure.getUImanager().getDefaultFontSize();
        }
    }

    int nThisRowY = nYPosStart;
    Text(pure, "Player Name", nXPosPlayerName, nThisRowY);
    Text(pure, "Frags", nXPosFrags, nThisRowY);
    Text(pure, "Deaths", nXPosDeaths, nThisRowY);

    nThisRowY -= pure.getUImanager().getDefaultFontSize();
    Text(pure, "========================================================", nXPosPlayerName, nThisRowY);

    int i = 0;
    for (const auto& player : getFragTable())
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

int DeathMatchMode::comparePlayers(int p1frags, int p2frags, int p1deaths, int p2deaths)
{
    if (p1frags == p2frags)
    {
        return p1deaths - p2deaths;
    }
    else
    {
        return p2frags - p1frags;
    }
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################



