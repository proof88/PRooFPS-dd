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


// this prefix increment is for if we want to AVOID assigning Max to GameModeType upon incrementing it.
//proofps_dd::GameModeType& proofps_dd::operator++(proofps_dd::GameModeType& gm)
//{
//    gm = static_cast<proofps_dd::GameModeType>(static_cast<int>(gm) + 1);
//    if (gm == proofps_dd::GameModeType::Max)
//    {
//        gm = proofps_dd::GameModeType::DeathMatch;
//    }
//    return gm;
//}

// This version allows assigning Max to GameModeType upon incrementing it.
// We need this so for-loop can terminate when GameModeType is Max.
proofps_dd::GameModeType& proofps_dd::operator++(proofps_dd::GameModeType& gm)
{
    if (gm == proofps_dd::GameModeType::Max)
    {
        gm = proofps_dd::GameModeType::DeathMatch;
    }
    else
    {
        gm = static_cast<proofps_dd::GameModeType>(static_cast<int>(gm) + 1);
    }
    return gm;
}

const char* proofps_dd::GameMode::getLoggerModuleName()
{
    return "GameMode";
}

proofps_dd::GameMode* proofps_dd::GameMode::createGameMode(
    proofps_dd::GameModeType gm,
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers)
{
    // unlike with traditional singleton, here we always destroy the already existing instance, due to
    // reassigning the unique_ptr to the new instance!

    switch (gm)
    {
    case proofps_dd::GameModeType::DeathMatch:
        // here std::move() is not needed to compile, but just to make it clear: unique_ptr is moved here, obviously, since it cannot be copied :)
        m_gamemode = std::move(proofps_dd::DeathMatchMode::createGameMode(mapPlayers));
        break;
    case proofps_dd::GameModeType::TeamDeathMatch:
        m_gamemode = std::move(proofps_dd::TeamDeathMatchMode::createGameMode(mapPlayers));
        break;
    case proofps_dd::GameModeType::TeamRoundGame:
        m_gamemode = std::move(proofps_dd::TeamRoundGameMode::createGameMode(mapPlayers));
        break;
    default:
        m_gamemode = nullptr;
    }

    return m_gamemode.get();
}

proofps_dd::GameMode* proofps_dd::GameMode::getGameMode()
{
    return m_gamemode.get();
}

bool proofps_dd::GameMode::isTeamBasedGame(GameModeType gm)
{
    switch (gm)
    {
    case proofps_dd::GameModeType::TeamDeathMatch:
        [[fallthrough]];
    case proofps_dd::GameModeType::TeamRoundGame:
        return true;
    default:
        return false;
    }
}

bool proofps_dd::GameMode::isRoundBased(GameModeType gm)
{
    switch (gm)
    {
    case proofps_dd::GameModeType::TeamRoundGame:
        return true;
    default:
        return false;
    }
}

const char* proofps_dd::GameMode::getGameModeTypeName(proofps_dd::GameModeType gm)
{
    switch (gm)
    {
    case proofps_dd::GameModeType::DeathMatch:
        return "Deathmatch / Free for All";
    case proofps_dd::GameModeType::TeamDeathMatch:
        return "Team Deathmatch";
    case proofps_dd::GameModeType::TeamRoundGame:
        return "Team Round Game";
    default:
        return "";
    }
}

proofps_dd::GameModeType proofps_dd::GameMode::getGameModeTypeFromConfig(PGEcfgProfiles& cfgProfiles)
{
    switch (cfgProfiles.getVars()[GameMode::szCvarSvGamemode].getAsInt())
    {
    case static_cast<int>(GameModeType::DeathMatch):
        return GameModeType::DeathMatch;
    case static_cast<int>(GameModeType::TeamDeathMatch):
        return GameModeType::TeamDeathMatch;
    case static_cast<int>(GameModeType::TeamRoundGame):
        return GameModeType::TeamRoundGame;
    default:
        // invalid value should default back to DM
        return GameModeType::DeathMatch;
    }
}

const char* proofps_dd::GameMode::getRank(const PlayersTableRow& row)
{
    static constexpr char* const szRankGoat = "G0aT";
    static constexpr char* const szRankGosu = "G0sU";
    static constexpr char* const szRankPro = "Pr0";
    static constexpr char* const szRankNoob = "N00b";
    static constexpr char* const szRankLow = "L0w";
    static constexpr char* const szRankClown = "Cl0wN";

    if (row.m_nFrags < 0)
    {
        return szRankClown;
    }

    if (row.m_nDeaths <= 0)
    {
        // cannot be negative but let's cover that case too together with zero divison
        return szRankGoat; // unsure but whoever not died yet, should be considered as a goat, regardless of the frags
    }

    const float fRatio = row.m_nFrags / static_cast<float>(row.m_nDeaths);
    if (fRatio >= 2.0f /* e.g. 30/15 */)
    {
        return szRankGoat;
    }
    else if (fRatio >= 1.5f /* e.g. 30/20 */)
    {
        return szRankGosu;
    }
    else if (fRatio >= 1.2f /* e.g. 30/25 */)
    {
        return szRankPro;
    }
    else if (fRatio >= 0.83f /* e.g. 25/30 */)
    {
        return szRankNoob;
    }
    else if (fRatio >= 0.5f /* e.g. 15/30 */)
    {
        return szRankLow;
    }

    return szRankClown;
}

CConsole& proofps_dd::GameMode::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::GameMode::~GameMode()
{
}

void proofps_dd::GameMode::fetchConfig(PGEcfgProfiles& cfgProfiles, pge_network::PgeINetwork& /*network*/)
{
    // assuming config is correct, because Config instance invokes us after its own validation is done
    // (Config instance validates data in cfgProfiles instance)
    setTimeLimitSecs(cfgProfiles.getVars()[GameMode::szCvarSvGmTimeLimit].getAsUInt());
}

proofps_dd::GameModeType proofps_dd::GameMode::getGameModeType() const
{
    return m_gameModeType;
}

const char* proofps_dd::GameMode::getGameModeTypeName() const
{
    return getGameModeTypeName(getGameModeType());
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::GameMode::getResetTime() const
{
    return m_timeReset;
}

unsigned int proofps_dd::GameMode::getTimeLimitSecs() const
{
    return m_nTimeLimitSecs;
}

void proofps_dd::GameMode::setTimeLimitSecs(unsigned int secs)
{
    m_nTimeLimitSecs = secs;
}

unsigned int proofps_dd::GameMode::getTimeRemainingMillisecs() const
{
    if (m_bWon || (getResetTime().time_since_epoch().count() == 0) || (getTimeLimitSecs() == 0))
    {
        return 0;
    }

    const auto nMillisecondsElapsedSinceReset = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - getResetTime()).count();
    const std::chrono::milliseconds::rep nTimeLimitMilliseconds = getTimeLimitSecs() * 1000;

    if (nTimeLimitMilliseconds <= nMillisecondsElapsedSinceReset)
    {
        return 0;
    }

    return static_cast<unsigned int>(nTimeLimitMilliseconds - nMillisecondsElapsedSinceReset);
}

void proofps_dd::GameMode::clientUpdateTimeRemainingMillisecs(const unsigned int& nRemMillisecs, pge_network::PgeINetwork& network)
{
    assert(!network.isServer());

    const unsigned int nRemSecs = static_cast<unsigned int>(std::floor(nRemMillisecs / 1000));
    if (nRemSecs > m_nTimeLimitSecs)
    {
        // should not happen, but should log as error
        getConsole().EOLn(
            "GameMode::%s(): SHOULD NOT HAPPEN (expected error in unit test): nRemSecs > m_nTimeLimitSecs: %u > %u!",
            __func__, nRemSecs, m_nTimeLimitSecs);
    }

    m_timeReset = std::chrono::steady_clock::now() - std::chrono::seconds(m_nTimeLimitSecs - std::min(nRemSecs, m_nTimeLimitSecs));
}

void proofps_dd::GameMode::restart(pge_network::PgeINetwork& network)
{
    m_players.clear();
    restartWithoutRemovingPlayers(network);
    // extended in derived class
}

void proofps_dd::GameMode::restartWithoutRemovingPlayers(pge_network::PgeINetwork& network)
{
    // invoked by both server and client

    m_timeReset = std::chrono::steady_clock::now();
    m_timeWin = std::chrono::time_point<std::chrono::steady_clock>(); // reset back to epoch
    m_bWon = false;
    for (auto& player : m_players)
    {
        player.m_bSpectatorMode = true;
        player.m_nFrags = 0;
        player.m_nDeaths = 0;
        player.m_nSuicides = 0;
        player.m_fFiringAcc = 0.f;
        player.m_nShotsFired = 0;
    }

    if (network.isServer())
    {
        serverSendGameSessionStateToClients(network, true /* restart flag */);
    }
    // extended in derived class
}

bool proofps_dd::GameMode::serverCheckAndUpdateWinningConditions(pge_network::PgeINetwork& network)
{
    assert(network.isServer());

    m_bWonPrevious = m_bWon;
    if (m_bWon)
    {
        // once it is won, it stays won until next restart()
        return m_bWon;
    }

    if (getTimeLimitSecs() > 0)
    {
        if (getTimeRemainingMillisecs() == 0)
        {
            handleEventGameWon(network);
            return true;
        }
    }

    return false;
}

void proofps_dd::GameMode::clientReceiveAndUpdateWinningConditions(pge_network::PgeINetwork& network, bool bGameSessionWon)
{
    assert(!network.isServer());

    //getConsole().EOLn("GameMode::%s(): client received new win state: %b!", __func__, bGameSessionWon);

    if (bGameSessionWon)
    {
        // this way hasJustBeenWonThisTick() will correctly work for clients connecting to an already won game
        m_bWonPrevious = false;
    }
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

void proofps_dd::GameMode::serverTickUpdateWinningConditions(pge_network::PgeINetwork& network)
{
    assert(network.isServer());
    /* no-op */
}

void proofps_dd::GameMode::clientTickUpdateWinningConditions(pge_network::PgeINetwork& network)
{
    assert(!network.isServer());
    m_bWonPrevious = m_bWon;
}

bool proofps_dd::GameMode::isGameWon() const
{
    return m_bWon;
}

bool proofps_dd::GameMode::wasGameWonAlreadyInPreviousTick() const
{
    return m_bWonPrevious;
}

bool proofps_dd::GameMode::hasJustBeenWonThisTick() const
{
    return !m_bWonPrevious && m_bWon;
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::GameMode::getWinTime() const
{
    return m_timeWin;
}

const std::list<proofps_dd::PlayersTableRow>& proofps_dd::GameMode::getPlayersTable() const
{
    return m_players;
}

const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& proofps_dd::GameMode::getExternalPlayersContainer() const
{
    return m_mapPlayersExternal;
}

bool proofps_dd::GameMode::renamePlayer(const std::string& sOldName, const std::string& sNewName)
{
    if (sOldName.empty() || sNewName.empty())
    {
        return false;
    }

    PlayersTableRow* pPlayerToRename = nullptr;
    
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

bool proofps_dd::GameMode::isPlayerAllowedForGameplay(const Player& player) const
{
    return !player.isInSpectatorMode();
}

bool proofps_dd::GameMode::isPlayerMovementAllowed() const
{
    return true;
}

unsigned int proofps_dd::GameMode::getSpectatorModePlayersCount() const
{
    unsigned int nCount = 0;
    for (const auto& player : m_players)
    {
        if (player.m_bSpectatorMode)
        {
            ++nCount;
        }
    }
    return nCount;
}

void proofps_dd::GameMode::text(PR00FsUltimateRenderingEngine& pure, const std::string& s, int x, int y) const
{
    pure.getUImanager().textTemporalLegacy(s, x, y)->SetDropShadow(true);
}


// ############################## PROTECTED ##############################


proofps_dd::GameMode::GameMode(
    proofps_dd::GameModeType gm,
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers) :
    m_mapPlayersExternal(mapPlayers),
    m_gameModeType(gm)
{
}

bool proofps_dd::GameMode::serverSendGameSessionStateToClient(pge_network::PgeINetwork& network, const pge_network::PgeNetworkConnectionHandle& connHandle)
{
    assert(network.isServer());
    pge_network::PgePacket pktGameSessionState;
    if (!proofps_dd::MsgGameSessionStateFromServer::initPkt(
        pktGameSessionState,
        m_bWon,
        false /* this is never a restart case when we are sending this to a single client only */))
    {
        getConsole().EOLn("GameMode::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
        assert(false);
        return false;
    }
    network.getServer().send(pktGameSessionState, connHandle);

    return true;
}

bool proofps_dd::GameMode::serverSendGameSessionStateToClients(pge_network::PgeINetwork& network, bool bGameRestart)
{
    assert(network.isServer());
    pge_network::PgePacket pktGameSessionState;
    if (!proofps_dd::MsgGameSessionStateFromServer::initPkt(
        pktGameSessionState,
        m_bWon,
        bGameRestart))
    {
        getConsole().EOLn("GameMode::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
        assert(false);
        return false;
    }
    network.getServer().sendToAllClientsExcept(pktGameSessionState);

    return true;
}

void proofps_dd::GameMode::handleEventGameWon(pge_network::PgeINetwork& network)
{
    m_bWon = true;
    m_timeWin = std::chrono::steady_clock::now();
    serverSendGameSessionStateToClients(
        network, false /* game winning is never a restart case */);
}


// ############################### PRIVATE ###############################


std::unique_ptr<proofps_dd::GameMode> proofps_dd::GameMode::m_gamemode{};


/*
   ###########################################################################
   proofps_dd::DeathMatchMode
   ###########################################################################
*/


// ############################### PUBLIC ################################


std::unique_ptr<proofps_dd::DeathMatchMode> proofps_dd::DeathMatchMode::createGameMode(
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers)
{
    // instead of std::make_unique, because that cannot access protected DeathMatchMode() ctor
    return std::unique_ptr<proofps_dd::DeathMatchMode>(new proofps_dd::DeathMatchMode(mapPlayers));
}

proofps_dd::DeathMatchMode::~DeathMatchMode()
{
}

void proofps_dd::DeathMatchMode::fetchConfig(PGEcfgProfiles& cfgProfiles, pge_network::PgeINetwork& network)
{
    GameMode::fetchConfig(cfgProfiles, network);
    // assuming config is correct, because Config instance invokes us after its own validation is done
    // (Config instance validates data in cfgProfiles instance)
    setFragLimit( cfgProfiles.getVars()[GameMode::szCvarSvDmFragLimit].getAsUInt() );
}

bool proofps_dd::DeathMatchMode::serverCheckAndUpdateWinningConditions(pge_network::PgeINetwork& network)
{
    if ( GameMode::serverCheckAndUpdateWinningConditions(network) )
    {
        return true;
    }

    if ((getFragLimit() > 0) && !m_players.empty())
    {
        // assume m_players is sorted, since add/updatePlayer() use insertion sort;
        // note that we don't need to check for spectating state since:
        // - spectators cannot gain frags (and this shall be ensured by the game as well: if someone shoots a bullet and
        //   then quickly goes to spectator mode, the bullet killing anyone shall not increase the frags of the now spectating player!),
        // - whoever is gaining frag, is not spectating,
        // - a spectator cannot have more frag than the non-zero frag limit without
        //   GameMode detecting game won state BEFORE the player would go into spectating mode.
        if (m_players.begin()->m_nFrags >= static_cast<int>(getFragLimit()))
        {
            handleEventGameWon(network);
            return true;
        }
    }

    return false;
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
        m_players.insert(
            it,
            proofps_dd::PlayersTableRow{
                player.getName(),
                player.getServerSideConnectionHandle(),
                player.getTeamId(),
                player.isInSpectatorMode(),
                player.getFrags(),
                player.getDeaths()
                /* rest are default 0 */});

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

    // quickly update some data (even if no change because it shall be fast enough) which do not contribute to ordering
    itFound->m_iTeamId = player.getTeamId();
    itFound->m_bSpectatorMode = player.isInSpectatorMode();
    itFound->m_nSuicides = player.getSuicides();
    itFound->m_fFiringAcc = player.getFiringAccuracy();
    itFound->m_nShotsFired = player.getShotsFiredCount();

    if ((player.getFrags() != itFound->m_nFrags) || (player.getDeaths() != itFound->m_nDeaths))
    {

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
            // move 'itFound' before 'it' (proofps_dd::PlayersTableRow is not copied, only internal pointers are updated)
            m_players.splice(it, m_players, itFound);
        }
        itFound->m_nFrags = player.getFrags();
        itFound->m_nDeaths = player.getDeaths();
    }

    if (network.isServer())
    {
        serverCheckAndUpdateWinningConditions(network);  // to make sure winning time is updated if game has just been won!
    }

    return true;
}

bool proofps_dd::DeathMatchMode::removePlayer(const Player& player, pge_network::PgeINetwork& /*network*/)
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

bool proofps_dd::DeathMatchMode::isTeamBasedGame() const
{
    return false;
}

bool proofps_dd::DeathMatchMode::isRoundBased() const
{
    return false;
}

bool proofps_dd::DeathMatchMode::isRespawnAllowedAfterDie() const
{
    return true;
}

bool proofps_dd::DeathMatchMode::isPlayerAllowedForGameplay(const Player& player) const
{
    return GameMode::isPlayerAllowedForGameplay(player);
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


// ############################## PROTECTED ##############################


proofps_dd::DeathMatchMode::DeathMatchMode(
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers) :
    proofps_dd::GameMode(proofps_dd::GameModeType::DeathMatch, mapPlayers)
{
}


// ############################### PRIVATE ###############################


/*
   ###########################################################################
   proofps_dd::TeamDeathMatchMode
   ###########################################################################
*/


// ############################### PUBLIC ################################


static const PureColor vecTeamColors[] =
{
    PureColor(255, 255, 255, 255), /* team id 0 = unassigned */
    PureColor(127, 255, 255, 255), /* team id 1 */
    PureColor(255, 127, 127, 255)  /* team id 2 */
};

const PureColor& proofps_dd::TeamDeathMatchMode::getTeamColor(unsigned int iTeamId)
{
    assert(iTeamId < (sizeof(vecTeamColors) / sizeof(vecTeamColors[0])));

    return vecTeamColors[iTeamId];
}

std::unique_ptr<proofps_dd::TeamDeathMatchMode> proofps_dd::TeamDeathMatchMode::createGameMode(
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers)
{
    // instead of std::make_unique, because that cannot access protected TeamDeathMatchMode() ctor
    return std::unique_ptr<proofps_dd::TeamDeathMatchMode>(new proofps_dd::TeamDeathMatchMode(mapPlayers));
}

proofps_dd::TeamDeathMatchMode::~TeamDeathMatchMode()
{
}

bool proofps_dd::TeamDeathMatchMode::serverCheckAndUpdateWinningConditions(pge_network::PgeINetwork& network)
{
    if (GameMode::serverCheckAndUpdateWinningConditions(network))
    {
        return true;
    }

    if ((getFragLimit() > 0) && !m_players.empty())
    {
        for (unsigned int iTeam = 1; iTeam <= 2; iTeam++)
        {
            if (getTeamFrags(iTeam) >= static_cast<int>(getFragLimit()))
            {
                handleEventGameWon(network);
                return true;
            }
        }
    }

    return false;
}

bool proofps_dd::TeamDeathMatchMode::addPlayer(const Player& player, pge_network::PgeINetwork& network)
{
    if (player.getTeamId() > 2)
    {
        return false;
    }

    return DeathMatchMode::addPlayer(player, network);
}

bool proofps_dd::TeamDeathMatchMode::updatePlayer(const Player& player, pge_network::PgeINetwork& network)
{
    if (player.getTeamId() > 2)
    {
        return false;
    }

    return DeathMatchMode::updatePlayer(player, network);
}

bool proofps_dd::TeamDeathMatchMode::isTeamBasedGame() const
{
    return true;
}

bool proofps_dd::TeamDeathMatchMode::isPlayerAllowedForGameplay(const Player& player) const
{
    return DeathMatchMode::isPlayerAllowedForGameplay(player) &&
        (player.getTeamId() != 0u);
}

int proofps_dd::TeamDeathMatchMode::getTeamFrags(unsigned int iTeamId) const
{
    if (iTeamId == 0)
    {
        return 0;
    }

    int nTeamTotalFrags = 0;
    for (const auto& player : m_players)
    {
        if ((player.m_iTeamId == iTeamId) && !player.m_bSpectatorMode)
        {
            nTeamTotalFrags += player.m_nFrags;
        }
    }
    return nTeamTotalFrags;
}

unsigned int proofps_dd::TeamDeathMatchMode::getTeamPlayersCount(unsigned int iTeamId) const
{
    if (iTeamId == 0)
    {
        return 0;
    }

    unsigned int nTeamTotalPlayers = 0;
    for (const auto& player : m_players)
    {
        if ((player.m_iTeamId == iTeamId) && !player.m_bSpectatorMode)
        {
            ++nTeamTotalPlayers;
        }
    }
    return nTeamTotalPlayers;
}

unsigned int proofps_dd::TeamDeathMatchMode::getAliveTeamPlayersCount(unsigned int iTeamId) const
{
    if (iTeamId == 0)
    {
        return 0;
    }
    
    unsigned int nTeamTotalAlivePlayers = 0;
    for (const auto& player : m_players)
    {
        const auto playerIt = m_mapPlayersExternal.find(player.m_connHandle);
        if (playerIt == m_mapPlayersExternal.end())
        {
            //getConsole().EOLn(
            //    "TeamDeathMatchMode::%s(): WARNING: player with connHandle %u is not found in m_mapPlayersExternal!",
            //    __func__, player.m_connHandle);
            continue;
        }

        if ((player.m_iTeamId == iTeamId) && !player.m_bSpectatorMode && (std::as_const(playerIt->second).getHealth() > 0))
        {
            ++nTeamTotalAlivePlayers;
        }
    }
    return nTeamTotalAlivePlayers;
}


// ############################## PROTECTED ##############################


proofps_dd::TeamDeathMatchMode::TeamDeathMatchMode(
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers) :
    proofps_dd::DeathMatchMode(mapPlayers)
{
    m_gameModeType = proofps_dd::GameModeType::TeamDeathMatch;
}


// ############################### PRIVATE ###############################


/*
   ###########################################################################
   proofps_dd::TeamRoundGameMode::RoundStateFSM
   ###########################################################################
*/


// ############################### PUBLIC ################################


const char* proofps_dd::TeamRoundGameMode::RoundStateFSM::getLoggerModuleName()
{
    return "RoundStateFSM";
}

CConsole& proofps_dd::TeamRoundGameMode::RoundStateFSM::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState& proofps_dd::TeamRoundGameMode::RoundStateFSM::getState() const
{
    return m_state;
}

void proofps_dd::TeamRoundGameMode::RoundStateFSM::update()
{
    stateUpdate();
}

void proofps_dd::TeamRoundGameMode::RoundStateFSM::reset()
{
    stateEnter(RoundState::Prepare);
    // manually update this timestamp because if we are in Prepare state already, it wont be updated
    m_timeEnteredCurrentState = std::chrono::steady_clock::now();
}

void proofps_dd::TeamRoundGameMode::RoundStateFSM::roundWon()
{
    stateEnter(RoundState::WaitForReset);
}

void proofps_dd::TeamRoundGameMode::RoundStateFSM::transitionToPlayState()
{
    // TODO: shall be invoked only from tests!
    stateEnter(RoundState::Play);
}

void proofps_dd::TeamRoundGameMode::RoundStateFSM::clientForceSetState(const RoundState& newState)
{
    getConsole().EOLn("RoundStateFSM::%s(): %d -> %d, bypassed stateEnter()!", __func__, m_state, newState);
    const RoundState oldState = m_state;
    m_state = newState;
    if (oldState != newState)
    {
        // clientUpdateTimeRemainingInCurrentStateMillisecs() is invoked separately after clientForceSetState()
        stateEntered(oldState, newState);
    }
}

std::chrono::seconds::rep proofps_dd::TeamRoundGameMode::RoundStateFSM::getTimeLimitInCurrentStateSeconds() const
{
    switch (m_state)
    {
    case RoundState::Prepare:
        return 3ll;
    case RoundState::Play:
        return 999ll;
    case RoundState::WaitForReset:
        return 5ll;
    default:
        getConsole().EOLn("RoundStateFSM::%s(): ERROR: unhandled new state: %d!", __func__, m_state);
        return 0ll;
    }
}

void proofps_dd::TeamRoundGameMode::RoundStateFSM::clientUpdateTimeRemainingInCurrentStateMillisecs(
    const unsigned int& nRemMillisecs, pge_network::PgeINetwork& network)
{
    assert(!network.isServer());

    const unsigned int nRemSecs = static_cast<unsigned int>(std::floor(nRemMillisecs / 1000));
    const auto nTimeLimitCurrentRoundStateSecs = getTimeLimitInCurrentStateSeconds();
    if (static_cast<unsigned int>(nRemSecs) > static_cast<unsigned int>(nTimeLimitCurrentRoundStateSecs))
    {
        // should not happen, but should log as error
        getConsole().EOLn(
            "GameMode::%s(): SHOULD NOT HAPPEN (expected error in unit test): nRemSecs > nTimeLimitCurrentRoundStateSecs: %u > %u!",
            __func__, nRemSecs, static_cast<unsigned int>(nTimeLimitCurrentRoundStateSecs));
    }

    m_timeEnteredCurrentState =
        std::chrono::steady_clock::now() -
        std::chrono::seconds(nTimeLimitCurrentRoundStateSecs - std::min(static_cast<unsigned int>(nRemSecs), static_cast<unsigned int>(nTimeLimitCurrentRoundStateSecs)));
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::TeamRoundGameMode::RoundStateFSM::getTimeEnteredCurrentState() const
{
    return m_timeEnteredCurrentState;
}

std::chrono::seconds::rep proofps_dd::TeamRoundGameMode::RoundStateFSM::getTimeRemainingInCurrentStateSeconds() const
{
    return static_cast<std::chrono::seconds::rep>(std::lroundf(std::floorf(getTimeRemainingInCurrentStateMilliseconds() / 1000.f)));
}

std::chrono::milliseconds::rep proofps_dd::TeamRoundGameMode::RoundStateFSM::getTimeRemainingInCurrentStateMilliseconds() const
{
    const auto nMillisecondsSpentInCurrentState = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - m_timeEnteredCurrentState).count();

    switch (m_state)
    {
    case RoundState::Prepare:
        return std::max(3000ll - nMillisecondsSpentInCurrentState, 0ll);
    case RoundState::Play:
        return 999000ll;
    case RoundState::WaitForReset:
        return std::max(5000ll - nMillisecondsSpentInCurrentState, 0ll);
    default:
        getConsole().EOLn("RoundStateFSM::%s(): ERROR: unhandled new state: %d!", __func__, m_state);
        return 0;
    }
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


void proofps_dd::TeamRoundGameMode::RoundStateFSM::stateEntered(
    const proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState& /*oldState*/,
    const proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState& newState)
{
    switch (newState)
    {
    case RoundState::Prepare:
        break;
    case RoundState::Play:
        break;
    case RoundState::WaitForReset:
        break;
    default:
        getConsole().EOLn("RoundStateFSM::%s(): ERROR: unhandled new state: %d!", __func__, newState);
    }
};

bool proofps_dd::TeamRoundGameMode::RoundStateFSM::stateEnter(
    const proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState& newState)
{
    getConsole().EOLn("RoundStateFSM::%s(): %d -> %d", __func__, m_state, newState);

    switch (newState)
    {
    case RoundState::Prepare:
        /* transition to this state is always allowed, e.g.: reset() */
        break;
    case RoundState::Play:
        if (m_state == RoundState::WaitForReset)
        {
            getConsole().EOLn("RoundStateFSM::%s(): ERROR: unhandled new state: %d!", __func__, newState);
            return false;
        }
        break;
    case RoundState::WaitForReset:
        /* transition to this state is always allowed */
        break;
    default:
        getConsole().EOLn("RoundStateFSM::%s(): ERROR: unhandled new state: %d!", __func__, newState);
        return false;
    }

    const RoundState oldState = m_state;
    m_state = newState;

    if (oldState != newState)
    {
        m_timeEnteredCurrentState = std::chrono::steady_clock::now();
    }

    stateEntered(oldState, newState);

    return true;
} // stateEnter()

void proofps_dd::TeamRoundGameMode::RoundStateFSM::stateUpdate()
{
    const auto nSecondsSpentInCurrentState = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - m_timeEnteredCurrentState).count();

    switch (m_state)
    {
    case RoundState::Prepare:
        if (nSecondsSpentInCurrentState >= 3)
        {
            stateEnter(RoundState::Play);
        }
        break;
    case RoundState::Play:
        //stateEnter();
        break;
    case RoundState::WaitForReset:
        if (nSecondsSpentInCurrentState >= 5)
        {
            /* no need to check game won state since FSM is driven by GameMode, but once game is won FSM is not driven */
            stateEnter(RoundState::Prepare);
        }
        break;
    default:
        getConsole().EOLn("RoundStateFSM::%s(): ERROR: unhandled current state: %d!", __func__, m_state);
    }
} // stateUpdate()


/*
   ###########################################################################
   proofps_dd::TeamRoundGameMode
   ###########################################################################
*/


std::ostream& proofps_dd::operator<<(std::ostream& s, const TeamRoundGameMode::RoundStateFSM::RoundState& rs)
{
    switch (rs)
    {
    case proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Prepare:
        s << "RoundState::Prepare (";
        break;
    case proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::Play:
        s << "RoundState::Play (";
        break;
    case proofps_dd::TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset:
        s << "RoundState::WaitForReset (";
        break;
    default:
        s << "Unknown RoundState (";
    }

    s << std::to_string(static_cast<int>(rs)) << ")";

    return s;
}


// ############################### PUBLIC ################################


std::unique_ptr<proofps_dd::TeamRoundGameMode> proofps_dd::TeamRoundGameMode::createGameMode(
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers)
{
    // instead of std::make_unique, because that cannot access protected TeamRoundGameMode() ctor
    return std::unique_ptr<proofps_dd::TeamRoundGameMode>(new proofps_dd::TeamRoundGameMode(mapPlayers));
}

proofps_dd::TeamRoundGameMode::~TeamRoundGameMode()
{
}

void proofps_dd::TeamRoundGameMode::fetchConfig(PGEcfgProfiles& cfgProfiles, pge_network::PgeINetwork& network)
{
    TeamDeathMatchMode::fetchConfig(cfgProfiles, network);
    // assuming config is correct, because Config instance invokes us after its own validation is done
    // (Config instance validates data in cfgProfiles instance)
    setRoundWinLimit(cfgProfiles.getVars()[GameMode::szCvarSvRgmRoundWinLimit].getAsUInt());
}

void proofps_dd::TeamRoundGameMode::restartWithoutRemovingPlayers(pge_network::PgeINetwork& network)
{
    m_nTeam1RoundWins = 0;
    m_nTeam2RoundWins = 0;
    m_fsm.reset();
    m_bFirstTick = true;
    m_bFsmStateTransitionHasJustHappenedThisTick_Sticky = true;
    m_bCurrentRoundHasJustBeenWon_Sticky = false;
    TeamDeathMatchMode::restartWithoutRemovingPlayers(network); /* due to overrides, this also sends out MsgGameRoundStateFromServer */
}

bool proofps_dd::TeamRoundGameMode::serverCheckAndUpdateWinningConditions(pge_network::PgeINetwork& network)
{
    if (GameMode::serverCheckAndUpdateWinningConditions(network))
    {
        // We dont care about RoundStateFSM once the game is won.
        return true;
    }

    static unsigned int nOldTeam1AlivePlayers = 0;
    static unsigned int nOldTeam2AlivePlayers = 0;
    
    m_oldFsmState = m_fsm.getState();
    m_fsm.update();
    if (m_fsm.getState() == RoundStateFSM::RoundState::Play)
    {
        const unsigned int nCurrentTeam1Players = getTeamPlayersCount(1);
        const unsigned int nCurrentTeam2Players = getTeamPlayersCount(2);
        // except round time limit reach, a round cannot be ended if both teams are empty
        if ((nCurrentTeam1Players != 0) || (nCurrentTeam2Players != 0))
        {
            const unsigned int nCurrentTeam1AlivePlayers = getAliveTeamPlayersCount(1);
            const unsigned int nCurrentTeam2AlivePlayers = getAliveTeamPlayersCount(2);

            // team win due to no more alive players in the other team is achievable only if both teams have assigned players!
            if ((nCurrentTeam1Players != 0) && (nCurrentTeam2Players != 0))
            {
                if ((nOldTeam1AlivePlayers > 0) && (nCurrentTeam1AlivePlayers == 0))
                {
                    getConsole().EOLn("TeamRoundGameMode::%s(): Round Win for Team 2!", __func__);
                    m_nTeam2RoundWins++;
                    m_fsm.roundWon();
                    m_bCurrentRoundHasJustBeenWon_Sticky = true;
                }
                if ((nOldTeam2AlivePlayers > 0) && (nCurrentTeam2AlivePlayers == 0))
                {
                    getConsole().EOLn("TeamRoundGameMode::%s(): Round Win for Team 1!", __func__);
                    m_nTeam1RoundWins++;
                    m_fsm.roundWon();
                    m_bCurrentRoundHasJustBeenWon_Sticky = true;
                }

                if ((getTeamRoundWins(1) == getRoundWinLimit()) || (getTeamRoundWins(2) == getRoundWinLimit()))
                {
                    getConsole().EOLn("TeamRoundGameMode::%s(): Round Win Limit Reached!", __func__);
                    handleEventGameWon(network);  /* due to overrides, this also sends out MsgGameRoundStateFromServer */
                    return true;
                }
            }
            else
            {
                // if maximum 1 team has player(s) and no more alive players left, we also need to transition to next round but without actual win
                if (((nCurrentTeam1Players != 0) && (nOldTeam1AlivePlayers > 0) && (nCurrentTeam1AlivePlayers == 0))
                    ||
                    ((nCurrentTeam2Players != 0) && (nOldTeam2AlivePlayers > 0) && (nCurrentTeam2AlivePlayers == 0)))
                {
                    getConsole().EOLn("TeamRoundGameMode::%s(): Round End without Win", __func__);
                    m_fsm.roundWon();
                }
            }

            nOldTeam1AlivePlayers = nCurrentTeam1AlivePlayers;
            nOldTeam2AlivePlayers = nCurrentTeam2AlivePlayers;
        }  // end if there is at least 1 player in a team
    }

    if (!m_bFsmStateTransitionHasJustHappenedThisTick_Sticky)
    {
        m_bFsmStateTransitionHasJustHappenedThisTick_Sticky = m_bFirstTick || (m_oldFsmState != m_fsm.getState());
    }
    if (m_oldFsmState != m_fsm.getState())
    {
        serverSendRoundStateToClients(network);
    }
    m_bFirstTick = false;
    
    return false;
}

void proofps_dd::TeamRoundGameMode::serverTickUpdateWinningConditions(pge_network::PgeINetwork& network)
{
    GameMode::serverTickUpdateWinningConditions(network);
    m_bFsmStateTransitionHasJustHappenedThisTick_Sticky = false;
    m_bCurrentRoundHasJustBeenWon_Sticky = false;
}

void proofps_dd::TeamRoundGameMode::clientTickUpdateWinningConditions(pge_network::PgeINetwork& network)
{
    GameMode::clientTickUpdateWinningConditions(network);
    // when client receives new state in clientHandleGameRoundStateFromServer(), it sets
    // m_bFsmStateTransitionHasJustHappenedThisTick_Sticky to true. This is at the beginning of client game loop.
    // Since clientTickUpdateWinningConditions() is invoked at end of client game loop, we set it to false here.
    m_bFsmStateTransitionHasJustHappenedThisTick_Sticky = false;
    m_bCurrentRoundHasJustBeenWon_Sticky = false;
}

bool proofps_dd::TeamRoundGameMode::addPlayer(const Player& player, pge_network::PgeINetwork& network)
{
    if (TeamDeathMatchMode::addPlayer(player, network))
    {
        if (network.isServer() && (player.getServerSideConnectionHandle() != pge_network::ServerConnHandle))
        {
            return serverSendRoundStateToClient(network, player.getServerSideConnectionHandle());
        }
        return true;
    }
    return false;
}

bool proofps_dd::TeamRoundGameMode::removePlayer(const Player& player, pge_network::PgeINetwork& network)
{
    const bool bRet = TeamDeathMatchMode::removePlayer(player, network);

    // deathmatch modes don't check automatically when removePlayer() is invoked but we have to
    if (network.isServer())
    {
        serverCheckAndUpdateWinningConditions(network);
    }

    return bRet;
}

bool proofps_dd::TeamRoundGameMode::isRoundBased() const
{
    return true;
}

bool proofps_dd::TeamRoundGameMode::isRespawnAllowedAfterDie() const
{
    return false;
}

bool proofps_dd::TeamRoundGameMode::isPlayerMovementAllowed() const
{
    return (m_fsm.getState() != TeamRoundGameMode::RoundStateFSM::RoundState::Prepare);
}

unsigned int proofps_dd::TeamRoundGameMode::getRoundWinLimit() const
{
    return m_nRoundWinLimit;
}

void proofps_dd::TeamRoundGameMode::setRoundWinLimit(unsigned int limit)
{
    if (limit > 0)
    {
        m_nRoundWinLimit = limit;
        //getConsole().EOLn("TeamRoundGameMode::%s(): changed to: %u!", __func__, m_nRoundWinLimit);
    }
    //else
    //{
    //    getConsole().EOLn("TeamRoundGameMode::%s(): invalid limit: %u, stays: %u!", __func__, limit, m_nRoundWinLimit);
    //}
}

unsigned int proofps_dd::TeamRoundGameMode::getTeamRoundWins(unsigned int iTeamId) const
{
    assert((iTeamId == 1) || (iTeamId == 2));
    return (iTeamId == 1) ? m_nTeam1RoundWins : m_nTeam2RoundWins;
}

proofps_dd::TeamRoundGameMode::RoundStateFSM& proofps_dd::TeamRoundGameMode::getFSM()
{
    return m_fsm;
}

const proofps_dd::TeamRoundGameMode::RoundStateFSM& proofps_dd::TeamRoundGameMode::getFSM() const
{
    return m_fsm;
}

bool proofps_dd::TeamRoundGameMode::hasJustTransitionedTo_RoundPrepareState_InThisTick() const
{
    return m_bFsmStateTransitionHasJustHappenedThisTick_Sticky && (m_fsm.getState() == RoundStateFSM::RoundState::Prepare);
}

bool proofps_dd::TeamRoundGameMode::hasJustTransitionedTo_RoundPlayState_InThisTick() const
{
    return m_bFsmStateTransitionHasJustHappenedThisTick_Sticky && (m_fsm.getState() == RoundStateFSM::RoundState::Play);
}

bool proofps_dd::TeamRoundGameMode::hasJustTransitionedTo_RoundWaitForResetState_InThisTick() const
{
    return m_bFsmStateTransitionHasJustHappenedThisTick_Sticky && (m_fsm.getState() == RoundStateFSM::RoundState::WaitForReset);
}

bool proofps_dd::TeamRoundGameMode::hasCurrentRoundJustBeenWon_InThisTick() const
{
    return m_bCurrentRoundHasJustBeenWon_Sticky;
}

bool proofps_dd::TeamRoundGameMode::clientHandleGameRoundStateFromServer(
    pge_network::PgeINetwork& network,
    const MsgGameRoundStateFromServer& msgRoundState)
{
    if (network.isServer())
    {
        getConsole().EOLn("TeamRoundGameMode::%s(): ERROR: server received, CANNOT HAPPEN!", __func__);
        assert(false);  // crash in debug
        return false;
    }

    if ( (msgRoundState.m_fsmState == TeamRoundGameMode::RoundStateFSM::RoundState::WaitForReset) &&
         ((m_nTeam1RoundWins < msgRoundState.m_nTeam1RoundWins) || (m_nTeam2RoundWins < msgRoundState.m_nTeam2RoundWins))
       )
    {
        m_bCurrentRoundHasJustBeenWon_Sticky = true;
    }
    m_nTeam1RoundWins = msgRoundState.m_nTeam1RoundWins;
    m_nTeam2RoundWins = msgRoundState.m_nTeam2RoundWins;
    if (msgRoundState.m_fsmState != m_fsm.getState())
    {
        m_oldFsmState = m_fsm.getState();
        m_bFsmStateTransitionHasJustHappenedThisTick_Sticky = true;
    }
    m_fsm.clientForceSetState(msgRoundState.m_fsmState);
    m_fsm.clientUpdateTimeRemainingInCurrentStateMillisecs(msgRoundState.m_nTimeRemainingInCurrentStateMillisecs, network);

    return true;
}

bool proofps_dd::TeamRoundGameMode::serverSendRoundStateToClient(pge_network::PgeINetwork& network, const pge_network::PgeNetworkConnectionHandle& connHandle)
{
    assert(network.isServer());
    pge_network::PgePacket pktRoundState;
    if (!proofps_dd::MsgGameRoundStateFromServer::initPkt(
        pktRoundState,
        m_fsm.getState(),
        static_cast<unsigned int>(m_fsm.getTimeRemainingInCurrentStateMilliseconds()),
        m_nTeam1RoundWins,
        m_nTeam2RoundWins))
    {
        getConsole().EOLn("TeamRoundGameMode::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
        assert(false);
        return false;
    }
    network.getServer().send(pktRoundState, connHandle);

    return true;
}

bool proofps_dd::TeamRoundGameMode::serverSendRoundStateToClients(pge_network::PgeINetwork& network)
{
    assert(network.isServer());
    pge_network::PgePacket pktRoundState;
    if (!proofps_dd::MsgGameRoundStateFromServer::initPkt(
        pktRoundState,
        m_fsm.getState(),
        static_cast<unsigned int>(m_fsm.getTimeRemainingInCurrentStateMilliseconds()),
        m_nTeam1RoundWins,
        m_nTeam2RoundWins))
    {
        getConsole().EOLn("TeamRoundGameMode::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
        assert(false);
        return false;
    }
    network.getServer().sendToAllClientsExcept(pktRoundState);

    return true;
}


// ############################## PROTECTED ##############################


proofps_dd::TeamRoundGameMode::TeamRoundGameMode(
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers) :
    proofps_dd::TeamDeathMatchMode(mapPlayers)
{
    m_gameModeType = proofps_dd::GameModeType::TeamRoundGame;
    m_fsm.reset();
}

bool proofps_dd::TeamRoundGameMode::serverSendGameSessionStateToClient(pge_network::PgeINetwork& network, const pge_network::PgeNetworkConnectionHandle& connHandle)
{
    if (GameMode::serverSendGameSessionStateToClient(network, connHandle))
    {
        return serverSendRoundStateToClient(network, connHandle);
    }
    return false;
}

bool proofps_dd::TeamRoundGameMode::serverSendGameSessionStateToClients(pge_network::PgeINetwork& network, bool bGameRestart)
{
    if (GameMode::serverSendGameSessionStateToClients(network, bGameRestart))
    {
        return serverSendRoundStateToClients(network);
    }
    return false;
}

void proofps_dd::TeamRoundGameMode::setTeamRoundWins(unsigned int iTeamId, unsigned int nRoundWins)
{
    assert((iTeamId == 1) || (iTeamId == 2));
    
    if (iTeamId == 1)
    {
        m_nTeam1RoundWins = nRoundWins;
    }
    else
    {
        m_nTeam2RoundWins = nRoundWins;
    }
}


// ############################### PRIVATE ###############################
