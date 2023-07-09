#pragma once

/*
    ###################################################################################
    GameMode.h
    Game Mode class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <chrono>
#include <list>
#include <string>

#include "../../Console/CConsole/src/CConsole.h"

#include "../../PGE/PGE/Network/PgeNetwork.h"
#include "../../PGE/PGE/PURE/include/external/PR00FsUltimateRenderingEngine.h"

#include "Player.h"

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
    };

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
        * It is recommended to first invoke updatePlayer() for all players with zeroed values and then call this.
        */
        virtual void restart();

        /**
        * Evaluates conditions to see if game is won or not.
        * Since conditions depend on game mode, the actual implementation must be in the specific derived game mode class.
        * Note that once a game is won, it stays won even if all players are removed, until explicit call to restart().
        * 
        * @return True if game is won, false otherwise.
        */
        virtual bool checkWinningConditions() = 0;
        
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
        * Automatically evaluates winning condition, in case of winning it also updates winning time.
        * Fails if a player with same name is already added.
        * 
        * @return True if added the new player, false otherwise.
        */
        virtual bool addPlayer(const Player& player) = 0;

        /**
        * Updates data for the specified player.
        * Automatically evaluates winning condition, in case of winning it also updates winning time.
        * Note that once a game is won, it stays won even if players are updated to fail the winning conditions, until explicit call to restart().
        * Fails if player with same cannot be found.
        *
        * @return True if updated the existing player, false otherwise.
        */
        virtual bool updatePlayer(const Player& player) = 0;

        /**
        * Removes data for the specified player.
        * Fails if player with same cannot be found.
        * Note that once a game is won, it stays won even if all players are removed, until explicit call to restart().
        *
        * @return True if removed the existing player, false otherwise.
        */
        virtual bool removePlayer(const Player& player) = 0;

        void text(PR00FsUltimateRenderingEngine& pure, const std::string& s, int x, int y) const;

        /**
        * Shows the objectives of the current game mode.
        * For example, in a deathmatch game, it might show a frag table, or in a single player game it might show mission objectives.
        */
        virtual void showObjectives(PR00FsUltimateRenderingEngine& pure, pge_network::PgeNetwork& network) = 0;

    protected:
        std::chrono::time_point<std::chrono::steady_clock> m_timeWin;
        std::list<FragTableRow> m_players;

        GameMode(GameModeType gm);

        GameMode(const GameMode&) = delete;
        GameMode& operator=(const GameMode&) = delete;
        GameMode(GameMode&&) = delete;
        GameMode&& operator=(GameMode&&) = delete;

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
        virtual bool checkWinningConditions() override;

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

        virtual bool addPlayer(const Player& player) override;
        virtual bool updatePlayer(const Player& player) override;
        virtual bool removePlayer(const Player& player) override;
        virtual void showObjectives(PR00FsUltimateRenderingEngine& pure, pge_network::PgeNetwork& network) override;

    protected:

    private:

        static int comparePlayers(int p1frags, int p2frags, int p1deaths, int p2deaths);
        
        // ---------------------------------------------------------------------------

        unsigned int m_nTimeLimitSecs;
        unsigned int m_nFragLimit;
        bool m_bWon;

    }; // class DeathMatchMode

} // namespace proofps_dd