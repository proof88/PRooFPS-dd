/*
    ###################################################################################
    GameMode.cpp
    Game Mode class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "GameMode.h"

#include "Player.h"
#include "PRooFPS-dd-packet.h"

/*
   ###########################################################################
   proofps_dd::GameMode
   ###########################################################################
*/


// ############################### PUBLIC ################################


const char* proofps_dd::GameMode::getLoggerModuleName()
{
    return "GameMode";
}

proofps_dd::GameMode* proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType gm)
{
    if (gm != proofps_dd::GameModeType::DeathMatch)
    {
        // currently we dont support other game mode type
        return nullptr;
    }

    return new proofps_dd::DeathMatchMode();
}

CConsole& proofps_dd::GameMode::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::GameMode::~GameMode()
{
}

proofps_dd::GameModeType proofps_dd::GameMode::getGameModeType() const
{
    return m_gameModeType;
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::GameMode::getResetTime() const
{
    return m_timeReset;
}

void proofps_dd::GameMode::restart(pge_network::PgeINetwork& network)
{
    m_players.clear();
    restartWithoutRemovingPlayers(network);
    // extended in derived class
}

void proofps_dd::GameMode::restartWithoutRemovingPlayers(pge_network::PgeINetwork& network)
{
    m_timeReset = std::chrono::steady_clock::now();
    m_timeWin = std::chrono::time_point<std::chrono::steady_clock>(); // reset back to epoch
    m_bWon = false;
    for (auto& player : m_players)
    {
        player.m_nFrags = 0;
        player.m_nDeaths = 0;
    }

    if (network.isServer())
    {
        serverSendGameSessionStateToClients(network);
    }
    // extended in derived class
}

bool proofps_dd::GameMode::isGameWon() const
{
    return m_bWon;
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::GameMode::getWinTime() const
{
    return m_timeWin;
}

const std::list<proofps_dd::FragTableRow>& proofps_dd::GameMode::getFragTable() const
{
    return m_players;
}

bool proofps_dd::GameMode::renamePlayer(const std::string& sOldName, const std::string& sNewName)
{
    if (sOldName.empty() || sNewName.empty())
    {
        return false;
    }

    FragTableRow* pPlayerToRename = nullptr;
    
    // loop over the all players and check for name collision, at the same time save ptr to target player
    for (auto& player : m_players)
    {
        if (player.m_sName == sOldName)
        {
            pPlayerToRename = &player;  // will rename this
        }
        else if (player.m_sName == sNewName)
        {
            return false;  // name collision
        }
    }

    if (pPlayerToRename)
    {
        pPlayerToRename->m_sName = sNewName;
        return true;
    }

    // no such player found
    return false;
}

void proofps_dd::GameMode::text(PR00FsUltimateRenderingEngine& pure, const std::string& s, int x, int y) const
{
    pure.getUImanager().textTemporalLegacy(s, x, y)->SetDropShadow(true);
}


// ############################## PROTECTED ##############################


proofps_dd::GameMode::GameMode(proofps_dd::GameModeType gm) :
    m_gameModeType(gm)
{
}

bool proofps_dd::GameMode::serverSendGameSessionStateToClient(pge_network::PgeINetwork& network, const pge_network::PgeNetworkConnectionHandle& connHandle)
{
    assert(network.isServer());
    pge_network::PgePacket pktGameSessionState;
    if (!proofps_dd::MsgGameSessionStateFromServer::initPkt(
        pktGameSessionState,
        m_bWon))
    {
        getConsole().EOLn("GameMode::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
        assert(false);
        return false;
    }
    network.getServer().send(pktGameSessionState, connHandle);

    return true;
}

bool proofps_dd::GameMode::serverSendGameSessionStateToClients(pge_network::PgeINetwork& network)
{
    assert(network.isServer());
    pge_network::PgePacket pktGameSessionState;
    if (!proofps_dd::MsgGameSessionStateFromServer::initPkt(
        pktGameSessionState,
        m_bWon))
    {
        getConsole().EOLn("GameMode::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
        assert(false);
        return false;
    }
    network.getServer().sendToAllClientsExcept(pktGameSessionState);

    return true;
}


// ############################### PRIVATE ###############################


/*
   ###########################################################################
   proofps_dd::DeathMatchMode
   ###########################################################################
*/


// ############################### PUBLIC ################################


static constexpr int nXPosPlayerName = 20;
static constexpr int nXPosFrags = 200;
static constexpr int nXPosDeaths = 250;

proofps_dd::DeathMatchMode::DeathMatchMode() :
    proofps_dd::GameMode(proofps_dd::GameModeType::DeathMatch)
{
}

proofps_dd::DeathMatchMode::~DeathMatchMode()
{
}

void proofps_dd::DeathMatchMode::fetchConfig(PGEcfgProfiles& cfgProfiles, pge_network::PgeINetwork& network)
{
    // assuming config is correct, because Config instance invokes us after its own validation is done
    setFragLimit( cfgProfiles.getVars()[GameMode::szCvarSvDmFragLimit].getAsUInt() );
    setTimeLimitSecs( cfgProfiles.getVars()[GameMode::szCvarSvDmTimeLimit].getAsUInt() );

    if (!network.isServer())
    {
        // TODO: client needs a trick to be able to show remaining time
    }
}

bool proofps_dd::DeathMatchMode::serverCheckAndUpdateWinningConditions(pge_network::PgeINetwork& network)
{
    assert(network.isServer());

    if (m_bWon)
    {
        // once it is won, it stays won until next restart()
        return m_bWon;
    }
    
    if (getTimeLimitSecs() > 0)
    {
        if (getTimeRemainingSecs() == 0)
        {
            m_bWon = true;
            m_timeWin = std::chrono::steady_clock::now();
            serverSendGameSessionStateToClients(network);
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
            serverSendGameSessionStateToClients(network);
            return true;
        }
    }

    return false;
}

void proofps_dd::DeathMatchMode::clientReceiveAndUpdateWinningConditions(pge_network::PgeINetwork& network, bool bGameSessionWon)
{
    assert(!network.isServer());

    getConsole().EOLn("DeathMatchMode::%s(): client received new win state: %b!", __func__, bGameSessionWon);

    m_bWon = bGameSessionWon;
    if (m_bWon)
    {
        m_timeWin = std::chrono::steady_clock::now();
    }
    else
    {
        restartWithoutRemovingPlayers(network);
    }
}

unsigned int proofps_dd::DeathMatchMode::getTimeLimitSecs() const
{
    return m_nTimeLimitSecs;
}

void proofps_dd::DeathMatchMode::setTimeLimitSecs(unsigned int secs)
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

void proofps_dd::DeathMatchMode::clientUpdateTimeRemainingSecs(const unsigned int& nRemSecs, pge_network::PgeINetwork& network)
{
    assert(!network.isServer());

    if (nRemSecs > m_nTimeLimitSecs)
    {
        // should not happen, but should log as error
        getConsole().EOLn("GameMode::%s(): nRemSecs > m_nTimeLimitSecs: %u > %u!", __func__, nRemSecs, m_nTimeLimitSecs);
    }

    m_timeReset = std::chrono::steady_clock::now() - std::chrono::seconds(m_nTimeLimitSecs - std::min(nRemSecs, m_nTimeLimitSecs));
}

unsigned int proofps_dd::DeathMatchMode::getFragLimit() const
{
    return m_nFragLimit;
}

void proofps_dd::DeathMatchMode::setFragLimit(unsigned int limit)
{
    m_nFragLimit = limit;
}

bool proofps_dd::DeathMatchMode::addPlayer(const Player& player, pge_network::PgeINetwork& network)
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
        m_players.insert(it, proofps_dd::FragTableRow{ player.getName(), player.getFrags(), player.getDeaths(), player.getServerSideConnectionHandle() });

        if (network.isServer() && (player.getServerSideConnectionHandle() != pge_network::ServerConnHandle))
        {
            // we automatically inform the new player about the current game goal/win state since maybe they are connecting when game is already won;
            // inform them ONLY IF GAME IS WON, since otherwise they are good already, and sending them "NOT WON" now would cause them to zero out already updated frag table data!
            if (isGameWon())
            {
                serverSendGameSessionStateToClient(network, player.getServerSideConnectionHandle());
            }
        }
    }

    if (network.isServer())
    {
        serverCheckAndUpdateWinningConditions(network);  // to make sure winning time is updated if game has just been won!
    }
    return bRet;
}

bool proofps_dd::DeathMatchMode::updatePlayer(const Player& player, pge_network::PgeINetwork& network)
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
        // move 'itFound' before 'it' (proofps_dd::FragTableRow is not copied, only internal pointers are updated)
        m_players.splice(it, m_players, itFound);
    }
    itFound->m_nFrags = player.getFrags();
    itFound->m_nDeaths = player.getDeaths();

    if (network.isServer())
    {
        serverCheckAndUpdateWinningConditions(network);  // to make sure winning time is updated if game has just been won!
    }

    return true;
}

bool proofps_dd::DeathMatchMode::removePlayer(const Player& player)
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

void proofps_dd::DeathMatchMode::showObjectives(PR00FsUltimateRenderingEngine& pure, pge_network::PgeINetwork& network)
{
    int nYPosStart = pure.getWindow().getClientHeight() - 20;
    
    if (isGameWon())
    {
        text(pure, "Game Ended! Waiting for restart ...", nXPosPlayerName, nYPosStart);
        nYPosStart -= 2 * pure.getUImanager().getDefaultFontSizeLegacy();
    }
    else
    {
        std::string sLimits = "DeathMatch";
        if (getFragLimit() > 0)
        {
            sLimits += " | Frag Limit: " + std::to_string(getFragLimit());
        }
        if (getTimeLimitSecs() > 0)
        {
            sLimits += " | Time Limit: " + std::to_string(getTimeLimitSecs()) + " s, Remaining: " + std::to_string(getTimeRemainingSecs()) + " s";
        }
        text(pure, sLimits, nXPosPlayerName, nYPosStart);
        nYPosStart -= 2 * pure.getUImanager().getDefaultFontSizeLegacy();
    }

    text(pure, "Player Name", nXPosPlayerName, nYPosStart);
    text(pure, "Frags", nXPosFrags, nYPosStart);
    text(pure, "Deaths", nXPosDeaths, nYPosStart);

    if (network.isServer())
    {
        showObjectivesServer(pure, network, nYPosStart);
    }
    else
    {
        showObjectivesClient(pure, network, nYPosStart);
    }
}

int proofps_dd::DeathMatchMode::comparePlayers(int p1frags, int p2frags, int p1deaths, int p2deaths)
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

void proofps_dd::DeathMatchMode::showObjectivesServer(PR00FsUltimateRenderingEngine& pure, pge_network::PgeINetwork& network, int nThisRowY)
{
    constexpr int nXPosPing = 320;
    constexpr int nXPosQuality = 370;
    constexpr int nXPosSpeed = 480;
    constexpr int nXPosPending = 640;
    constexpr int nXPosUnAckd = 780;
    constexpr int nXPosInternalQueueTime = 870;

    assert(network.isServer());

    text(pure, "Ping", nXPosPing, nThisRowY);
    text(pure, "Qlty", nXPosQuality, nThisRowY);
    text(pure, "NE/FE", nXPosQuality, nThisRowY - pure.getUImanager().getDefaultFontSizeLegacy());
    text(pure, "Speed", nXPosSpeed, nThisRowY);
    text(pure, "Tx/Rx(Bps)", nXPosSpeed, nThisRowY - pure.getUImanager().getDefaultFontSizeLegacy());
    text(pure, "Pending", nXPosPending, nThisRowY);
    text(pure, "Rel/Unrel(Bps)", nXPosPending, nThisRowY - pure.getUImanager().getDefaultFontSizeLegacy());
    text(pure, "UnAck'd", nXPosUnAckd, nThisRowY);
    text(pure, "(Bps)", nXPosUnAckd, nThisRowY - pure.getUImanager().getDefaultFontSizeLegacy());
    text(pure, "Int. Q", nXPosInternalQueueTime, nThisRowY);
    text(pure, "Time (us)", nXPosInternalQueueTime, nThisRowY - pure.getUImanager().getDefaultFontSizeLegacy());

    nThisRowY -= 2 * pure.getUImanager().getDefaultFontSizeLegacy();

    text(pure, "============================================================================================", nXPosPlayerName, nThisRowY);

    for (const auto& player : getFragTable())
    {
        nThisRowY = nThisRowY - pure.getUImanager().getDefaultFontSizeLegacy();
        text(pure, player.m_sName, nXPosPlayerName, nThisRowY);
        text(pure, std::to_string(player.m_nFrags), nXPosFrags, nThisRowY);
        text(pure, std::to_string(player.m_nDeaths), nXPosDeaths, nThisRowY);
        
        /* debug data */
        if (player.m_connHandle != pge_network::ServerConnHandle)
        {
            text(pure,
                std::to_string(network.getServer().getPing(player.m_connHandle, true)),
                nXPosPing, nThisRowY);
            std::stringstream ssQuality;
            ssQuality << std::fixed << std::setprecision(2) << network.getServer().getQualityLocal(player.m_connHandle, false) <<
                "/" << network.getServer().getQualityRemote(player.m_connHandle, false);
            text(pure,
                ssQuality.str(),
                nXPosQuality, nThisRowY);
            text(pure,
                std::to_string(std::lround(network.getServer().getTxByteRate(player.m_connHandle, false))) + "/" +
                std::to_string(std::lround(network.getServer().getRxByteRate(player.m_connHandle, false))),
                nXPosSpeed, nThisRowY);
            text(pure,
                std::to_string(network.getServer().getPendingReliableBytes(player.m_connHandle, false)) + "/" +
                std::to_string(network.getServer().getPendingUnreliableBytes(player.m_connHandle, false)),
                nXPosPending, nThisRowY);
            text(pure,
                std::to_string(network.getServer().getSentButUnAckedReliableBytes(player.m_connHandle, false)),
                nXPosUnAckd, nThisRowY);
            text(pure,
                std::to_string(network.getServer().getInternalQueueTimeUSecs(player.m_connHandle, false)),
                nXPosInternalQueueTime, nThisRowY);
        }
    }

}  // showObjectivesServer()

void proofps_dd::DeathMatchMode::showObjectivesClient(PR00FsUltimateRenderingEngine& pure, pge_network::PgeINetwork& network, int nThisRowY)
{
    assert(!network.isServer());

    nThisRowY -= pure.getUImanager().getDefaultFontSizeLegacy();

    text(pure, "=======================================", nXPosPlayerName, nThisRowY);

    for (const auto& player : getFragTable())
    {
        nThisRowY = nThisRowY - pure.getUImanager().getDefaultFontSizeLegacy();
        text(pure, player.m_sName, nXPosPlayerName, nThisRowY);
        text(pure, std::to_string(player.m_nFrags), nXPosFrags, nThisRowY);
        text(pure, std::to_string(player.m_nDeaths), nXPosDeaths, nThisRowY);
    }

    /* debug data */

    nThisRowY -= 2 * pure.getUImanager().getDefaultFontSizeLegacy();
    text(pure, "Ping: " + std::to_string(network.getClient().getPing(true)) + " ms",
        nXPosPlayerName, nThisRowY);

    nThisRowY -= pure.getUImanager().getDefaultFontSizeLegacy();
    std::stringstream ssQuality;
    ssQuality << "Quality: near: " << std::fixed << std::setprecision(2) << network.getClient().getQualityLocal(false) <<
        "; far: " << network.getClient().getQualityRemote(false);
    text(pure,
        ssQuality.str(),
        nXPosPlayerName, nThisRowY);

    nThisRowY -= pure.getUImanager().getDefaultFontSizeLegacy();
    text(pure, "Tx Speed: " + std::to_string(std::lround(network.getClient().getTxByteRate(false))) +
        " Bps; Rx Speed: " + std::to_string(std::lround(network.getClient().getRxByteRate(false))) + " Bps",
        nXPosPlayerName, nThisRowY);

    nThisRowY -= pure.getUImanager().getDefaultFontSizeLegacy();
    text(pure, "Pending Bytes: Reliable: " + std::to_string(network.getClient().getPendingReliableBytes(false)) +
        "; Unreliable: " + std::to_string(network.getClient().getPendingUnreliableBytes(false)),
        nXPosPlayerName, nThisRowY);

    nThisRowY -= pure.getUImanager().getDefaultFontSizeLegacy();
    text(pure, "UnAck'd Bytes: " + std::to_string(network.getClient().getSentButUnAckedReliableBytes(false)),
        nXPosPlayerName, nThisRowY);

    nThisRowY -= pure.getUImanager().getDefaultFontSizeLegacy();
    text(pure, "Internal Queue Time: " + std::to_string(network.getClient().getInternalQueueTimeUSecs(false)) + " us",
        nXPosPlayerName, nThisRowY);
}  // showObjectivesClient()


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################



