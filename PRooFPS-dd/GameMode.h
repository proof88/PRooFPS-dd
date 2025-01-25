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

namespace proofps_dd
{

    enum class GameModeType
    {
        DeathMatch,
        TeamDeathMatch,
        Max,            /* last value prefix increment operator allows reaching */
        TeamRoundGame   /* no support yet */
    };

    /** Prefix increment, useful in iterating over different game modes in unit test. */
    GameModeType& operator++(GameModeType& gm);

    struct PlayersTableRow
    {
        std::string m_sName;
        pge_network::PgeNetworkConnectionHandle m_connHandle{};
        unsigned int m_iTeamId{ 0 };  // 0 means no team selected
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

        static const char* getLoggerModuleName();

        /**
        * Similar to singleton design pattern, there is always maximum one instance.
        * However, if there is an already existing instance, it automatically gets destroyed before the new one is created.
        */
        static GameMode* createGameMode(GameModeType gm);

        /**
        * @return The last created GameMode instance created by createGameMode().
        *         nullptr if no instance created yet.
        */
        static GameMode* getGameMode();

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
        * 
        * This class checks elapsed game session time against time limit, if any set.
        * Derived class shall extend this function by overriding and calling this parent implementation from the specialized implementation.
        * 
        * A game shall not be won without any players, so at least 1 player needs to be present to win any game.
        * Note that once a game is won, it stays won even if all players are removed, until explicit call to restart().
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
        * Returns the current game session win state i.e. game goal is reached or not.
        * 
        * @return True if current game session is won (goal reached), false otherwise.
        */
        bool isGameWon() const;
        
        /**
        * @return Timestamp of moment when current game was won by a player. It is Epoch time 0 if the game is not yet won.
        */
        const std::chrono::time_point<std::chrono::steady_clock>& getWinTime() const;

        /**
        * @return Player table, which is basically the frag table in specific game modes, but here at this abstract level it has a more general name.
        */
        const std::list<PlayersTableRow>& getPlayersTable() const;
        
        /**
        * Adds the specified player.
        * In case of server instance, if the added player is a client, it SHALL immediately send game win condition to this client
        * using serverSendGameSessionStateToClient().
        * In case of server instance, it SHALL automatically evaluate winning condition using serverCheckAndUpdateWinningConditions() after adding the player.
        * Note that once a game is won, it stays won even if players are updated to fail the winning conditions, until explicit call to restart().
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
        virtual bool removePlayer(const Player& player) = 0;

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

        /**
        * Checks if given player is allowed for gameplay.
        * Primarily this is for server instance.
        * For example, in a team-based game mode, server can freeze player actions when no team is assigned to the player.
        * 
        * @return True if player is ready for gameplay in the current game mode, false otherwise.
        */
        virtual bool isPlayerAllowedForGameplay(const Player& player) const = 0;

        void text(PR00FsUltimateRenderingEngine& pure, const std::string& s, int x, int y) const;

    protected:
        // derived class can set these based on their winning conditions and actions
        std::chrono::time_point<std::chrono::steady_clock> m_timeWin;
        std::list<PlayersTableRow> m_players;
        bool m_bWon{ false };
        GameModeType m_gameModeType;

        GameMode(GameModeType gm);

        GameMode(const GameMode&) = delete;
        GameMode& operator=(const GameMode&) = delete;
        GameMode(GameMode&&) = delete;
        GameMode&& operator=(GameMode&&) = delete;

        bool serverSendGameSessionStateToClient(pge_network::PgeINetwork& network, const pge_network::PgeNetworkConnectionHandle& connHandle);
        bool serverSendGameSessionStateToClients(pge_network::PgeINetwork& network);
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

        DeathMatchMode();
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
        * Note: behavior is unspecified if this value is changed on-the-fly during a game. For now, please also call restart() explicitly.
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
        virtual bool removePlayer(const Player& player) override;

        virtual bool isTeamBasedGame() const override;

        virtual bool isPlayerAllowedForGameplay(const Player& player) const override;

    protected:
        unsigned int m_nFragLimit{};

    private:

        static int comparePlayers(int p1frags, int p2frags, int p1deaths, int p2deaths);
        
        // ---------------------------------------------------------------------------

    }; // class DeathMatchMode

    /**
    * In Team DeathMatch game mode, players are grouped into teams. Teammates work together to
    * shoot as many players in the enemy team as they can. The team with most total frags is the winner when
    * either the frag limit or time limit is reached.
    * Note: it is also valid to not to have either frag limit or time limit set, but in such case the game never ends.
    * 
    * Player teamId is used to know which player belongs to which team, so team total frags can be calculated.
    * This also means serverCheckAndUpdateWinningConditions() has different implementation than DeathMatchMode has.
    */
    class TeamDeathMatchMode : public DeathMatchMode
    {
    public:

        TeamDeathMatchMode();
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

        virtual bool isPlayerAllowedForGameplay(const Player& player) const override;

        /**
        * @param iTeamId Team ID for which team we want to get the sum of frags.
        * 
        * @return Sum of player frags in the specified team.
        *         Always 0 when iTeamId is 0.
        */
        int getTeamFrags(unsigned int iTeamId) const;

        /**
        * @param iTeamId Team ID for which team we want to get the count of players.
        *
        * @return Number of players in the specified team.
        */
        unsigned int getTeamPlayersCount(unsigned int iTeamId) const;

    protected:

    private:

        // ---------------------------------------------------------------------------

    }; // class TeamDeathMatchMode

} // namespace proofps_dd