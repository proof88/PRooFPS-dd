#pragma once

/*
    ###################################################################################
    GameMode.h
    Game Mode class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include <chrono>
#include <memory>
#include <list>
#include <string>

#include "CConsole.h"

#include "Config/PGEcfgProfiles.h"
#include "Network/PgeNetwork.h"
#include "PURE/include/external/PR00FsUltimateRenderingEngine.h"

class GameModeTest;

namespace proofps_dd
{

    // TODO: on the long run, this should be deleted.
    // Derived classes shall implement stuff like isRoundBasedGame() etc ... and game logic should
    // depend on those instead of checking gamemodetype.
    enum class GameModeType
    {
        DeathMatch,
        TeamDeathMatch,
        TeamRoundGame,
        Max            /* last value prefix increment operator allows reaching */
    };

    /** Prefix increment, useful in iterating over different game modes in unit test. */
    GameModeType& operator++(GameModeType& gm);

    struct PlayersTableRow
    {
        std::string m_sName;
        pge_network::PgeNetworkConnectionHandle m_connHandle{};
        unsigned int m_iTeamId{ 0 };  // 0 means no team selected
        bool m_bSpectatorMode{ true };
        int m_nFrags{ 0 };    // frags allowed to be negative due to player doing suicides decreases fragcount
        int m_nDeaths{ 0 };   // TODO: this should be unsigned, but then everywhere else like in CPlayer!
        unsigned int m_nSuicides{ 0 };
        float m_fFiringAcc{ 0.f };
        unsigned int m_nShotsFired{ 0 };
    };

    class Player;

    /**
    * GameMode class represent the Frag Table and the winning condition checks.
    * It identifies players by their name thus it is essential that all players have unique name.
    * 
    * TODO: not sure exactly about my original idea, but definitely the current design should be changed a bit.
    * GameMode should NOT contain anything related to "frags", it should be more abstract.
    * It should be called scores, frag limit shall be score limit, which is the round win limit in TRG but frag limit otherwise.
    * Frags, frag table, etc. should be introduced in derived class such as DeathMatchMode.
    * However this definitely won't be "fixed" in 2024.
    * 
    * TODO: regarding fetchConfig(): probably GameMode should autonomuously handle relevant config, including validation.
    * However, for this to happen, it should implement an IConfigHandler so Config class can invoke its validateConfig()
    * function when Config::validate() is invoked. On the long run this mechanism should be extended to other classes as well
    * where the CVAR definitions are also present.
    */
    class GameMode
    {
    public:

        static constexpr char* szCvarSvGamemode = "sv_gamemode";
        
        static constexpr char* szCvarSvGmTimeLimit = "sv_gm_timelimit_secs";
        
        static constexpr char* szCvarSvDmFragLimit = "sv_dm_fraglimit";
        static constexpr char* szCvarSvRgmRoundWinLimit = "sv_rgm_roundwinlimit";

        static constexpr int nSvGmTimeLimitSecsDef = 0;
        static constexpr int nSvGmTimeLimitSecsMin = 0;
        static constexpr int nSvGmTimeLimitSecsMax = 60 * 60 * 24;
        static_assert(nSvGmTimeLimitSecsMin < nSvGmTimeLimitSecsMax, "Min timelimit should be smaller than max timelimit.");
        static_assert(nSvGmTimeLimitSecsMin <= nSvGmTimeLimitSecsDef, "Min timelimit should not be greater than default timelimit.");
        static_assert(nSvGmTimeLimitSecsDef <= nSvGmTimeLimitSecsMax, "Max timelimit should not be smaller than default timelimit.");

        static constexpr int nSvDmFragLimitDef = 10;
        static constexpr int nSvDmFragLimitMin = 0;
        static constexpr int nSvDmFragLimitMax = 999;
        static_assert(nSvDmFragLimitMin < nSvDmFragLimitMax,  "Min fraglimit should be smaller than max fraglimit.");
        static_assert(nSvDmFragLimitMin <= nSvDmFragLimitDef, "Min fraglimit should not be greater than default fraglimit.");
        static_assert(nSvDmFragLimitDef <= nSvDmFragLimitMax, "Max fraglimit should not be smaller than default fraglimit.");

        static constexpr int nSvRgmRoundWinLimitDef = 5;
        static constexpr int nSvRgmRoundWinLimitMin = 1;
        static constexpr int nSvRgmRoundWinLimitMax = 999;
        static_assert(nSvRgmRoundWinLimitMin < nSvRgmRoundWinLimitMax, "Min round win limit should be smaller than max round win limit.");
        static_assert(nSvRgmRoundWinLimitMin <= nSvRgmRoundWinLimitDef, "Min round win limit should not be greater than default round win limit.");
        static_assert(nSvRgmRoundWinLimitDef <= nSvRgmRoundWinLimitMax, "Max round win limit should not be smaller than default round win limit.");

        static const char* getLoggerModuleName();

        /**
        * Similar to singleton design pattern, there is always maximum one instance.
        * However, if there is an already existing instance, it automatically gets destroyed before the new one is created.
        * This makes sure we always get a fresh object built up from scratch, and we can forget about the previous one.
        * This is exactly the mechanism we need for GameMode, from the application's perspective.
        * 
        * @param gm         Type of game to create.
        * @param mapPlayers Players container, sometimes might be needed to fetch some attributes of players (e.g. HP).
        * 
        * @return Raw pointer to the created GameMode instance.
        *         Shall not be stored by anyone but always queried using getGameMode().
        */
        static GameMode* createGameMode(
            GameModeType gm,
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);

        /**
        * @return The last created GameMode instance created by createGameMode().
        *         nullptr if no instance created yet.
        */
        static GameMode* getGameMode();

        static bool isTeamBasedGame(GameModeType gm);

        static bool isRoundBased(GameModeType gm);

        static const char* getGameModeTypeName(GameModeType gm);

        static GameModeType getGameModeTypeFromConfig(PGEcfgProfiles& cfgProfiles);

        static const char* getRank(const PlayersTableRow& row);

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        virtual ~GameMode();

        /**
        * Fetches configuration from the given PGEcfgProfiles instance.
        * Derived class shall extend this function by overriding and calling this parent implementation from the specialized implementation.
        * 
        * For now it does not do validation, as all validations are currently implemented in the Config class.
        * TODO: on the long run, validation should be also done, by proper planning and implementing an IConfigHandler interface, as
        * described in the comment above.
        *
        * @param cfgProfiles The current user config profile from where we can fetch value of GameMode-specific CVARs.
        * @param network     PGE network instance to be used to know if we are server or client.
        */
        virtual void fetchConfig(PGEcfgProfiles& cfgProfiles, pge_network::PgeINetwork& network);

        GameModeType getGameModeType() const;
        const char* getGameModeTypeName() const;

        const std::chrono::time_point<std::chrono::steady_clock>& getResetTime() const;

        /**
        * @return Configured time limit previously set by setTimeLimitSecs().
        *         0 means no time limit.
        */
        unsigned int getTimeLimitSecs() const;

        /**
        * Set the time limit for the game.
        * If time limit expires, the winner is the player with most frags, even if frag limit is not set or not reached.
        * Note: behavior is unspecified if this value is changed on-the-fly during a game. For now, please also call restart() explicitly.
        *
        * @param secs The time limit in seconds. If 0, there is no time limit.
        */
        void setTimeLimitSecs(unsigned int secs);

        /**
        * @return Milliseconds remaining until time limit is reached, calculated from the current time and last reset time (getResetTime()).
        *         0 if there is no time limit set, or if the game was not yet reset, or if the time limit has been already reached, or the game has been won for any reason.
        */
        unsigned int getTimeRemainingMillisecs() const;

        /**
        * Updates the remaining time on client side, based on the remaining time received from server.
        * Basically it corrects the game restart time on client side so client will have the roughly same game restart time as the server.
        *
        * @param nRemMillisecs Remaining time in milliseconds, from server.
        * @param network       PGE network instance to be used to know if we are server or client.
        */
        void clientUpdateTimeRemainingMillisecs(const unsigned int& nRemMillisecs, pge_network::PgeINetwork& network);

        /**
        * Similar to restartWithoutRemovingPlayers() but it also removes all players from this GameMode instance.
        * 
        * Used by both server and clients. A typical situation for clients to invoke this is when they get disconnected from server.
        * 
        * @param network PGE network instance to be used to send out MsgGameSessionStateFromServer to clients.
        */
        virtual void restart(pge_network::PgeINetwork& network);

        /**
        * Resets winning time and winning condition, also zeros out relevant members of PlayersTableRow for all players.
        * It is recommended to first invoke updatePlayer() for all players with zeroed values and then call this.
        * 
        * Used by both server and clients: clients invoke it upon receiving MsgGameSessionStateFromServer.
        * 
        * @param network PGE network instance to be used to send out MsgGameSessionStateFromServer to clients.
        */
        virtual void restartWithoutRemovingPlayers(pge_network::PgeINetwork& network);

        /**
        * Evaluates conditions to see if game is won or not.
        * Shall be invoked regularly (per tick or per frame) at the beginning of server tick or frame loop
        * so hasJustBeenWonThisTick() can work properly.
        * 
        * This class checks elapsed game session time against time limit, if any set.
        * Derived class shall extend this function by overriding and calling this parent implementation from the specialized implementation.
        * 
        * A game shall not be won without any players, so at least 1 player needs to be present to win any game.
        * Note that once a game is won, it stays won even if all players are removed, until explicit call to restart() or
        * restartWithoutRemovingPlayers().
        * 
        * This function is for server instance only.
        * It also sends out MsgGameSessionStateFromServer to all clients in case game session state is changed as a result of the call.
        * 
        * @param  network PGE network instance to be used to send out MsgGameSessionStateFromServer if needed.
        * @return True if game is won, false otherwise.
        */
        virtual bool serverCheckAndUpdateWinningConditions(pge_network::PgeINetwork& network);

        /**
        * Handles server's update about current game session goal, e.g. game is won.
        * Server sends out such notification occasionally only, when something relevant has changed about the current game session.
        * 
        * This function is for client instance only.
        *
        * @param  bGameSessionWon True if the current game session goal has been just reached, false otherwise.
        */
        void clientReceiveAndUpdateWinningConditions(pge_network::PgeINetwork& network, bool bGameSessionWon);

        /**
        * Shall be invoked regularly (per tick or per frame) at the end of client tick or frame loop so
        * hasJustBeenWonThisTick() can work properly.
        * 
        * This function is for client instance only.
        * 
        */
        void clientTickUpdateWinningConditions();

        /**
        * Returns the current game session win state i.e. game goal is reached or not.
        * 
        * Note that once a game is won, it stays won even if all players are removed, until explicit call to restart() or
        * restartWithoutRemovingPlayers(). 
        * 
        * @return True if current game session is won (goal reached), false otherwise.
        */
        bool isGameWon() const;

        /**
        * Returns if the current game session was already won in the previous tick.
        * The idea is the following:
        *  - server shall invoke serverCheckAndUpdateWinningConditions() in every frame or in every game tick;
        *  - client shall invoke clientTickUpdateWinningConditions() in every frame or in every game tick;
        *  - client shall invoke clientReceiveAndUpdateWinningConditions() when it receives MsgGameSessionStateFromServer
        *    from the server.
        *
        * @return True if current game session was won (goal reached) in the previous tick, false otherwise.
        */
        bool wasGameWonAlreadyInPreviousTick() const;

        /**
        * Returns if the current game session has been just detected as won in this tick i.e. it was not yet won in
        * the previous tick (wasGameWonAlreadyInPreviousTick() returns false) but now is won in the current tick (isGameWon() returns true).
        * 
        * This function is the recommended way for both server and client instances to check for and handle the change
        * of winning state.
        * 
        * It is true only for a short period of time slice which can be 1 tick or 1 frame, depending on how often your
        * game instance invokes either serverCheckAndUpdateWinningConditions() or clientUpdateWinningConditions().
        */
        bool hasJustBeenWonThisTick() const;
        
        /**
        * @return Timestamp of moment when current game was won by a player. It is Epoch time 0 if the game is not yet won.
        */
        const std::chrono::time_point<std::chrono::steady_clock>& getWinTime() const;

        /**
        * @return Player table, which is basically the frag table in specific game modes, but here at this abstract level it has a more general name.
        */
        const std::list<PlayersTableRow>& getPlayersTable() const;

        /**
        * @return The external container storing connected players, that was passed in createGameMode() / ctor.
        */
        const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& getExternalPlayersContainer() const;
        
        /**
        * Adds the specified player.
        * In case of server instance, if the added player is a client, it SHALL immediately send game win condition to this client
        * using serverSendGameSessionStateToClient().
        * In case of server instance, it SHALL automatically evaluate winning condition using serverCheckAndUpdateWinningConditions() after adding the player.
        * Note that once a game is won, it stays won even if players are updated to fail the winning conditions, until explicit call to restart().
        * 
        * Note that GameMode does not ensure that a spectating player cannot win a game.
        * The game shall ensure that the player does not gain any frags while in spectator mode!
        * Therefore, if the added player already has enough frags to flip game won state, the game will go into won state, regardless of the player's spectating
        * state!
        * 
        * Fails if a player with same name is already added.
        * 
        * @return True if added the new player, false otherwise.
        */
        virtual bool addPlayer(
            const Player& player,
            pge_network::PgeINetwork& network) = 0;

        /**
        * Updates data for the specified player.
        * In case of server instance, it SHALL automatically evaluate winning condition using serverCheckAndUpdateWinningConditions() after updating the player.
        * Note that once a game is won, it stays won even if players are updated to fail the winning conditions, until explicit call to restart().
        * 
        * Note that GameMode does not ensure that a spectating player cannot win a game.
        * The game shall ensure that the player does not gain any frags while in spectator mode!
        * Therefore, if the added player already has enough frags to flip game won state, the game will go into won state, regardless of the player's spectating
        * state!
        * 
        * Fails if player with same cannot be found.
        *
        * @return True if updated the existing player, false otherwise.
        */
        virtual bool updatePlayer(
            const Player& player,
            pge_network::PgeINetwork& network) = 0;

        /**
        * Removes data for the specified player.
        * Fails if player with same cannot be found.
        * 
        * Does not evaluate winning conditions, since once a game is won, it stays won even if all players are removed, until explicit call to restart().
        *
        * @return True if removed the existing player, false otherwise.
        */
        virtual bool removePlayer(
            const Player& player,
            pge_network::PgeINetwork& network) = 0;

        /**
        * Renames the player.
        * All players must have unique name.
        * The function fails if there is already a player having the same name as sNewName, or if there was no such player with name as sOldName.
        * 
        * @param sOldName The previous name of the player that we want to change.
        * @param sNewName The new name of the player we want to change to.
        * 
        * @return True if rename was successful, false otherwise.
        */
        bool renamePlayer(const std::string& sOldName, const std::string& sNewName);

        /**
        * Derived class shall return false if it is non-team-based game, otherwise true.
        */
        virtual bool isTeamBasedGame() const = 0;

        /*
        * Derived class shall return false if it is non-round-based game, otherwise true.
        */
        virtual bool isRoundBased() const = 0;

        /*
        * Derived class shall return true if a player is allowed to respawn after dieing (with optional respawn countdown), false otherwise.
        * In case of false, game logic shall skip showing the respawn countdown and shall wait for a different condition to allow respawn.
        */
        virtual bool isRespawnAllowedAfterDie() const = 0;

        /**
        * Checks if given player is allowed for gameplay.
        * For example, in a team-based game mode, server can freeze player actions when no team is assigned to the player.
        * But it also checks for spectator mode.
        *
        * Can be used by both server and client instances.
        * Must also work properly on client-side, since in player-follow spectating view, this function is used to determine
        * which players can be spectated.
        * 
        * @return True if player is ready and active for gameplay in the current game mode, false otherwise.
        */
        virtual bool isPlayerAllowedForGameplay(const Player& player) const;

        /**
        * @return True if player movement is allowed, false otherwise.
        */
        virtual bool isPlayerMovementAllowed() const;

        /**
        * @return Number of players in spectator mode (team id is irrelevant).
        */
        unsigned int getSpectatorModePlayersCount() const;

        void text(PR00FsUltimateRenderingEngine& pure, const std::string& s, int x, int y) const;

    protected:
        // derived class can set these based on their winning conditions and actions
        std::chrono::time_point<std::chrono::steady_clock> m_timeWin;
        std::list<PlayersTableRow> m_players;
        const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayersExternal;
        bool m_bWon{ false };
        bool m_bWonPrevious{ false };
        GameModeType m_gameModeType;

        GameMode(
            GameModeType gm,
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);

        GameMode(const GameMode&) = delete;
        GameMode& operator=(const GameMode&) = delete;
        GameMode(GameMode&&) = delete;
        GameMode&& operator=(GameMode&&) = delete;

        virtual bool serverSendGameSessionStateToClient(pge_network::PgeINetwork& network, const pge_network::PgeNetworkConnectionHandle& connHandle);
        virtual bool serverSendGameSessionStateToClients(pge_network::PgeINetwork& network, bool bGameRestart);
        void handleEventGameWon(pge_network::PgeINetwork& network);

    private:

        static std::unique_ptr<GameMode> m_gamemode; // the last created gamemode is stored here, basically singleton

        std::chrono::time_point<std::chrono::steady_clock> m_timeReset; // can be private again once all time-related functions in DeathMatchMode are moved to this class
        unsigned int m_nTimeLimitSecs{};

        // ---------------------------------------------------------------------------

    }; // class GameMode

    /**
    * In DeathMatch a.k.a. FFA (Free For All) game mode, everyone is shooting everyone, and the winner is
    * whoever has the most frags when either the frag limit or time limit is reached.
    * Note: it is also valid to not to have either frag limit or time limit set, but in such case the game never ends.
    * 
    * Although we have player teamId, we don't use it in this game mode at all.
    */
    class DeathMatchMode : public GameMode
    {
    public:

        /**
        * Used by GameMode::createGameMode(), this way we don't need to be friend with GameMode.
        * The previous GameMode instance is destroyed automatically.
        * See GameMode::createGameMode() for more details.
        * 
        * @return Smart pointer to the created TeamRoundGameMode instance.
        */
        static std::unique_ptr<DeathMatchMode> createGameMode(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);

        // ---------------------------------------------------------------------------

        virtual ~DeathMatchMode();

        DeathMatchMode(const DeathMatchMode&) = delete;
        DeathMatchMode& operator=(const DeathMatchMode&) = delete;
        DeathMatchMode(DeathMatchMode&&) = delete;
        DeathMatchMode&& operator=(DeathMatchMode&&) = delete;

        virtual void fetchConfig(PGEcfgProfiles& cfgProfiles, pge_network::PgeINetwork& network) override;

        /**
        * Extending parent class implementation by checking player frags against frag limit, if any is set.
        */
        virtual bool serverCheckAndUpdateWinningConditions(pge_network::PgeINetwork& network) override;

        /**
        * @return Configured frag limit previously set by setFragLimit(). 0 means no frag limit.
        */
        unsigned int getFragLimit() const;

        /**
        * Set the frag limit for the game.
        * If the frag limit is reached, the winner is with the most frags, even if time limit is not yet reached or there is no time limit set.
        * 
        * Note: behavior is unspecified if this value is changed on-the-fly during a game. For now, please also call restart() explicitly.
        * In general it is not recommended to change this value on-the-fly during a game because it might put the game into won state
        * if there is a spectating player having equal or more frags than the new frag limit!
        * 
        * @param limit The frag limit. If 0, there is no frag limit.
        */
        void setFragLimit(unsigned int limit);

        virtual bool addPlayer(
            const Player& player,
            pge_network::PgeINetwork& network) override;
        virtual bool updatePlayer(
            const Player& player,
            pge_network::PgeINetwork& network) override;
        virtual bool removePlayer(
            const Player& player,
            pge_network::PgeINetwork& network) override;

        virtual bool isTeamBasedGame() const override;
        virtual bool isRoundBased() const override;
        virtual bool isRespawnAllowedAfterDie() const override;

        virtual bool isPlayerAllowedForGameplay(const Player& player) const override;

    protected:
        unsigned int m_nFragLimit{};

        DeathMatchMode(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);

    private:

        static int comparePlayers(int p1frags, int p2frags, int p1deaths, int p2deaths);
        
        // ---------------------------------------------------------------------------

    }; // class DeathMatchMode

    /**
    * In Team DeathMatch game mode, players are grouped into teams. Teammates work together to
    * kill as many players in the enemy team as they can. The team with most total frags is the winner when
    * either the frag limit or time limit is reached.
    * Note: it is also valid to not to have either frag limit or time limit set, but in such case the game never ends.
    * 
    * Player teamId is used to know which player belongs to which team, so team total frags can be calculated.
    * This also means serverCheckAndUpdateWinningConditions() has different implementation than DeathMatchMode has.
    */
    class TeamDeathMatchMode : public DeathMatchMode
    {
    public:

        static constexpr char* szCvarSvTdmFriendlyFire = "sv_tdm_friendlyfire";

        static const PureColor& getTeamColor(unsigned int iTeamId);

        /**
        * Used by GameMode::createGameMode(), this way we don't need to be friend with GameMode.
        * The previous GameMode instance is destroyed automatically.
        * See GameMode::createGameMode() for more details.
        * 
        * @return Smart pointer to the created TeamRoundGameMode instance.
        */
        static std::unique_ptr<TeamDeathMatchMode> createGameMode(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);

        // ---------------------------------------------------------------------------

        virtual ~TeamDeathMatchMode();

        TeamDeathMatchMode(const TeamDeathMatchMode&) = delete;
        TeamDeathMatchMode& operator=(const TeamDeathMatchMode&) = delete;
        TeamDeathMatchMode(TeamDeathMatchMode&&) = delete;
        TeamDeathMatchMode&& operator=(TeamDeathMatchMode&&) = delete;

        /**
        * Altering parent class implementation by checking team total frags (instead of individual player frags) against frag limit, if any is set.
        */
        virtual bool serverCheckAndUpdateWinningConditions(pge_network::PgeINetwork& network) override;

        /**
        * Extending parent class implementation by rejecting player if team id is not in [0-2] range.
        */
        virtual bool addPlayer(
            const Player& player,
            pge_network::PgeINetwork& network) override;

        /**
        * Extending parent class implementation by rejecting player if team id is not in [0-2] range.
        */
        virtual bool updatePlayer(
            const Player& player,
            pge_network::PgeINetwork& network) override;

        virtual bool isTeamBasedGame() const override;

        /**
        * Extending parent class implementation by rejecting player if team id is 0.
        */
        virtual bool isPlayerAllowedForGameplay(const Player& player) const override;

        /**
        * @param iTeamId Team ID for which team we want to get the sum of frags.
        * 
        * @return Sum of assigned player frags in the specified team.
        *         A dead player who is currently forced-spectating in a round-based game mode is also considered since
        *         force-spectating is NOT spectator mode, the player still has assigned team.
        *         Always 0 when iTeamId is 0.
        */
        int getTeamFrags(unsigned int iTeamId) const;

        /**
        * @param iTeamId Team ID for which team we want to get the count of players.
        *
        * @return Number of assigned players (not in spectator mode) in the specified team.
        *         A dead player who is currently forced-spectating in a round-based game mode is also considered since
        *         force-spectating is NOT spectator mode, the player still has assigned team.
        *         Always 0 when iTeamId is 0, so for counting spectators use getSpectatorModePlayersCount() instead!
        */
        unsigned int getTeamPlayersCount(unsigned int iTeamId) const;

        /**
        * @param iTeamId    Team ID for which team we want to get the count of alive players.
        *
        * @return Number of assigned (not in spectator mode) AND alive players in the specified team.
        *         Basically: getTeamPlayersCount(iTeamId) - number of dead players in iTeamId.
        *         Always 0 when iTeamId is 0, so for counting spectators use getSpectatorModePlayersCount() instead!
        */
        unsigned int getAliveTeamPlayersCount(unsigned int iTeamId) const;

    protected:

        TeamDeathMatchMode(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);

    private:

        // ---------------------------------------------------------------------------

    }; // class TeamDeathMatchMode

    struct MsgGameRoundStateFromServer;

    /**
    * In Team Round Game mode, players are grouped into teams, same way as in Team DeathMatch.
    * However, the game mode features rounds.
    * 
    * Unlike in deathmatch-style game modes, here players cannot respawn immediately after being killed:
    * isRespawnAllowedAfterDie() return false.
    * Instead, they have to wait for the next round to start.
    * 
    * Each round ends when any of the teams loses all its players.
    * 
    * The team reaching the predefined round win limit earlier wins the game, or the team with the most
    * round wins if time limit is reached.
    * The round win limit is configured by the server.
    * 
    * Note that although setFragLimit() functionality is inherited from parent classes, it is not used.
    * 
    * Each round has different states, governed by RoundStateFSM.
    * 
    * Note that when the game is won, RoundStateFSM might stay in Prepare or Play states, for example if
    * game won reason is game time limit reached (getTimeLimitSecs()).
    */
    class TeamRoundGameMode : public TeamDeathMatchMode
    {
    public:

        /**
        *                         update():
        *         round won (all players disconnected from a team)
        *           >------------>------------>------------>-->
        *           ^                                         ¡
        *           |       update():                         |          reset(),
        *           |    timeoutPrepare       update():       |          update():
        *           ^       elapsed           round won       ¡     timeoutBeforeReset elapsed,
        * o--->-> Prepare ------------> Play ------------> WaitForReset ----->
        *     ^                          ¡                                   ¡
        *     |         reset()          |                                   |
        *     ^----------<---------------<                                   |
        *     |                                                              |
        *     ^----------<---------------<---------------<---------------<----
        * 
        */
        class RoundStateFSM
        {
        public:
            enum class RoundState
            {
                Prepare,
                Play,
                WaitForReset
            };

            static const char* getLoggerModuleName();

            // ---------------------------------------------------------------------------

            CConsole& getConsole() const;

            RoundStateFSM() = default;
            ~RoundStateFSM() = default;

            RoundStateFSM(const RoundStateFSM&) = default;
            RoundStateFSM& operator=(const RoundStateFSM&) = default;
            RoundStateFSM(RoundStateFSM&&) = default;
            RoundStateFSM& operator=(RoundStateFSM&&) = default;

            /**
            * @return The current round state, controlled by update().
            */
            const RoundState& getState() const;

            /**
            * Expected to be invoked periodically.
            */
            void update();

            void reset();

            void roundWon();

            void transitionToPlayState();

            /**
            * To be used only by client instances as they receive state from server.
            */
            void forceSetState(const RoundState& newState);

            const std::chrono::time_point<std::chrono::steady_clock>& getTimeEnteredCurrentState() const;

        private:
            RoundState m_state{ RoundState::Prepare };
            std::chrono::time_point<std::chrono::steady_clock> m_timeEnteredCurrentState;

            void stateEntered(const RoundState& /*oldState*/, const RoundState& newState);

            /**
            * Expected to be invoked only for events, such as explicit call to reset(), or when update() detects an event.
            * @return True if the state transition is valid, false otherwise.
            *         If false is returned, state transition did not happen.
            */
            bool stateEnter(const RoundState& newState);

            /**
            * Expected to be invoked periodically.
            */
            void stateUpdate();
        };  // class RoundStateFSM

        /**
        * Used by GameMode::createGameMode(), this way we don't need to be friend with GameMode.
        * The previous GameMode instance is destroyed automatically.
        * See GameMode::createGameMode() for more details.
        *
        * @return Smart pointer to the created TeamRoundGameMode instance.
        */
        static std::unique_ptr<TeamRoundGameMode> createGameMode(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);

        // ---------------------------------------------------------------------------

        virtual ~TeamRoundGameMode();

        TeamRoundGameMode(const TeamRoundGameMode&) = delete;
        TeamRoundGameMode& operator=(const TeamRoundGameMode&) = delete;
        TeamRoundGameMode(TeamRoundGameMode&&) = delete;
        TeamRoundGameMode&& operator=(TeamRoundGameMode&&) = delete;

        virtual void fetchConfig(PGEcfgProfiles& cfgProfiles, pge_network::PgeINetwork& network) override;

        virtual void restartWithoutRemovingPlayers(pge_network::PgeINetwork& network);

        /**
        * Altering parent class implementation by checking won rounds per team, instead of frag limit.
        * Frag limit is not considered at all.
        * Time limit is still considered.
        * 
        * This also drives RoundStateFSM. Note that when the game is won, RoundStateFSM might stay in
        * Prepare or Play states, for example if game won reason is game time limit reached (getTimeLimitSecs()).
        */
        virtual bool serverCheckAndUpdateWinningConditions(pge_network::PgeINetwork& network) override;

        /*
        * Extending parent class implementation by forcing check for round- or game winning conditions, since
        * the other team might win if this team does not have anymore players alive after removing this player.
        */
        virtual bool removePlayer(
            const Player& player,
            pge_network::PgeINetwork& network) override;

        virtual bool isRoundBased() const override;
        virtual bool isRespawnAllowedAfterDie() const override;
        virtual bool isPlayerMovementAllowed() const override;

        /**
        * @return Configured round win limit previously set by setRoundWinLimit().
        */
        unsigned int getRoundWinLimit() const;

        /**
        * Set the round win limit for the game.
        * If the round win limit is reached, the winner is the team which reached it, even if
        * time limit is not yet reached or there is no time limit set.
        *
        * Note: behavior is unspecified if this value is changed on-the-fly during a game. For now, please also call restart() explicitly.
        *
        * @param limit The round win limit. Must be positive value.
        */
        void setRoundWinLimit(unsigned int limit);

        /**
        * @param iTeamId Team ID for which team we want to get the total number of won rounds.
        *
        * @return Total number of won rounds for the given team.
        *         Always 0 when iTeamId is 0.
        */
        unsigned int getTeamRoundWins(unsigned int iTeamId) const;

        RoundStateFSM& getFSM();

        bool clientHandleGameRoundStateFromServer(
            pge_network::PgeINetwork& network,
            const MsgGameRoundStateFromServer& msgRoundState);

        bool serverSendRoundStateToClient(pge_network::PgeINetwork& network, const pge_network::PgeNetworkConnectionHandle& connHandle);
        bool serverSendRoundStateToClients(pge_network::PgeINetwork& network);

    protected:

        TeamRoundGameMode(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);

        /**
        * Extends original behavior by also sending out MsgGameRoundStateFromServer to the specific client.
        */
        virtual bool serverSendGameSessionStateToClient(pge_network::PgeINetwork& network, const pge_network::PgeNetworkConnectionHandle& connHandle) override;
        
        /**
        * Extends original behavior by also sending out MsgGameRoundStateFromServer to all clients.
        */
        virtual bool serverSendGameSessionStateToClients(pge_network::PgeINetwork& network, bool bGameRestart) override;

        void setTeamRoundWins(unsigned int iTeamId, unsigned int nRoundWins);

        friend class GameModeTest;

    private:

        // ---------------------------------------------------------------------------

        unsigned int m_nRoundWinLimit{ GameMode::nSvRgmRoundWinLimitDef };
        unsigned int m_nTeam1RoundWins{ 0 };
        unsigned int m_nTeam2RoundWins{ 0 };
        RoundStateFSM m_fsm;

    }; // class TeamRoundGameMode

    std::ostream& operator<< (std::ostream& s, const TeamRoundGameMode::RoundStateFSM::RoundState& rs);  /**< Write to stream. */

} // namespace proofps_dd