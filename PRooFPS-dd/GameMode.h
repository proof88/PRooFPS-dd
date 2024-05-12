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
#include <list>
#include <string>

#include "CConsole.h"

#include "Network/PgeNetwork.h"
#include "PURE/include/external/PR00FsUltimateRenderingEngine.h"

namespace proofps_dd
{

    enum class GameModeType
    {
        DeathMatch,
        TeamDeathMatch,
        TeamRoundGame
    };

    struct FragTableRow
    {
        std::string m_sName;
        int m_nFrags;    // frags allowed to be negative due to player doing suicides decreases fragcount
        int m_nDeaths;   // TODO: this should be unsigned, but then everywhere else like in CPlayer!
        pge_network::PgeNetworkConnectionHandle m_connHandle;
    };

    class Player;

    /**
    * GameMode class represent the Frag Table and the winning condition checks.
    * It identifies players by their name thus it is essential that all players have unique name.
    */
    class GameMode
    {
    public:

        static const char* getLoggerModuleName();

        static GameMode* createGameMode(GameModeType gm);

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        virtual ~GameMode();

        GameModeType getGameModeType() const;

        const std::chrono::time_point<std::chrono::steady_clock>& getResetTime() const;

        /**
        * Resets winning time and winning condition.
        * It is recommended to first invoke updatePlayer() for all players with zeroed values and then call this or
        * use restartWithoutRemovingPlayers() instead.
        */
        virtual void restart();

        /**
        * Resets winning time and winning condition.
        * It is recommended to first invoke updatePlayer() for all players with zeroed values and then call this.
        */
        virtual void restartWithoutRemovingPlayers();

        /**
        * Evaluates conditions to see if game is won or not.
        * Since conditions depend on game mode, the actual implementation must be in the specific derived game mode class.
        * Note that once a game is won, it stays won even if all players are removed, until explicit call to restart().
        * 
        * This function is for server instance only.
        * It also sends out MsgGameSessionStateFromServer to all clients in case game session state is changed as a result of the call.
        * 
        * @param  network PGE network instance to be used to send out MsgGameSessionStateFromServer if needed.
        * @return True if game is won, false otherwise.
        */
        virtual bool checkAndUpdateWinningConditionsServer(pge_network::PgeINetwork& network) = 0;

        /**
        * Handles server's update about current game session goal, e.g. game is won.
        * Server sends out such notification occasionally only, when something relevant has changed about the current game session.
        * 
        * This function is for client instance only.
        *
        * @param  bGameSessionWon True if the current game session goal has been just reached, false otherwise.
        */
        virtual void receiveAndUpdateWinningConditionsClient(pge_network::PgeINetwork& network, bool bGameSessionWon) = 0;

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
        * @return List of frag table entries, which is basically a list of added players with some statistics.
        */
        const std::list<FragTableRow>& getFragTable() const;
        
        /**
        * Adds the specified player.
        * In case of server instance, it automatically evaluates winning condition, and in case of winning it also updates winning time and
        * sends out MsgGameSessionStateFromServer to all clients.
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
        * In case of server instance, it automatically evaluates winning condition, and in case of winning it also updates winning time and
        * sends out MsgGameSessionStateFromServer to all clients.
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

        void text(PR00FsUltimateRenderingEngine& pure, const std::string& s, int x, int y) const;

        /**
        * Shows the objectives of the current game mode.
        * For example, in a deathmatch game, it might show a frag table, or in a single player game it might show mission objectives.
        */
        virtual void showObjectives(PR00FsUltimateRenderingEngine& pure, pge_network::PgeINetwork& network) = 0;

    protected:
        std::chrono::time_point<std::chrono::steady_clock> m_timeWin;
        std::list<FragTableRow> m_players;
        bool m_bWon{ false };

        GameMode(GameModeType gm);

        GameMode(const GameMode&) = delete;
        GameMode& operator=(const GameMode&) = delete;
        GameMode(GameMode&&) = delete;
        GameMode&& operator=(GameMode&&) = delete;

        bool sendGameSessionStateToClients(pge_network::PgeINetwork& network);

    private:

        GameModeType m_gameModeType;
        std::chrono::time_point<std::chrono::steady_clock> m_timeReset;

        // ---------------------------------------------------------------------------

    }; // class GameMode

    /**
    * In DeathMatch a.k.a. FFA (Free For All) game mode, everyone is shooting everyone, and the winner is
    * whoever has the most frags when either the frag limit or time limit is reached.
    * Note: it is also valid to not to have either frag limit or time limit set, but in such case the game never ends.
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

        virtual void restart() override;
        virtual void restartWithoutRemovingPlayers() override;
        virtual bool checkAndUpdateWinningConditionsServer(pge_network::PgeINetwork& network) override;
        virtual void receiveAndUpdateWinningConditionsClient(pge_network::PgeINetwork& network, bool bGameSessionWon) override;

        /**
        * @return Configured time limit previously set by setTimeLimitSecs(). 0 means no time limit.
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
        * @return Seconds remaining until time limit is reached, calculated from the current time and last reset time (getResetTime()).
        *         0 if there is no time limit set, or the game was not yet reset or when the time limit has been reached or the game has been won.
        */
        unsigned int getTimeRemainingSecs() const;

        /**
        * @return Configured frag limit previously set by setFragLimit(). 0 means no frag limit.
        */
        unsigned int getFragLimit() const;

        /**
        * Set the frag limit for the game.
        * If the frag limit is reached, the winner is the player with most frags, even if time limit is not yet reached or there is no time limit set.
        * Note: behavior is unspecified if this value is changed on-the-fly during a game. For now, please also call restart() explicitly.
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
        virtual void showObjectives(PR00FsUltimateRenderingEngine& pure, pge_network::PgeINetwork& network) override;

    protected:

    private:

        static int comparePlayers(int p1frags, int p2frags, int p1deaths, int p2deaths);
        
        // ---------------------------------------------------------------------------

        unsigned int m_nTimeLimitSecs{};
        unsigned int m_nFragLimit{};

        void showObjectivesServer(PR00FsUltimateRenderingEngine& pure, pge_network::PgeINetwork& network, int nThisRowY);
        void showObjectivesClient(PR00FsUltimateRenderingEngine& pure, pge_network::PgeINetwork& network, int nThisRowY);

    }; // class DeathMatchMode

} // namespace proofps_dd