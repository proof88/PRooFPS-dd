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

#include "../../../CConsole/CConsole/src/CConsole.h"

#include "../../../PGE/PGE/Network/PgeNetwork.h"
#include "../../../PGE/PGE/PURE/include/external/PR00FsUltimateRenderingEngine.h"

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
        * Removes all added players, resets winning time and winning condition.
        */
        virtual void Reset();

        /**
        * Evaluates conditions to see if game is won or not.
        * Since conditions depend on game mode, the actual implementation must be in the specific derived game mode class.
        * Note that once a game is won, it stays won even if all players are removed, until explicit call to Reset().
        * 
        * @return True if game is won, false otherwise.
        */
        virtual bool checkWinningConditions() = 0;
        
        const std::chrono::time_point<std::chrono::steady_clock>& getWinTime() const;

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
        * Note that once a game is won, it stays won even if players are updated to fail the winning conditions, until explicit call to Reset().
        * Fails if player with same cannot be found.
        *
        * @return True if updated the existing player, false otherwise.
        */
        virtual bool updatePlayer(const Player& player) = 0;

        /**
        * Removes data for the specified player.
        * Fails if player with same cannot be found.
        * Note that once a game is won, it stays won even if all players are removed, until explicit call to Reset().
        *
        * @return True if removed the existing player, false otherwise.
        */
        virtual bool removePlayer(const Player& player) = 0;

        void Text(PR00FsUltimateRenderingEngine& pure, const std::string& s, int x, int y) const;
        virtual void ShowObjectives(PR00FsUltimateRenderingEngine& pure, pge_network::PgeNetwork& network) = 0;

    protected:
        std::chrono::time_point<std::chrono::steady_clock> m_timeWin;
        std::list<FragTableRow> m_players;

        GameMode(GameModeType gm);

        // TODO: explicitly delete these
        GameMode(const GameMode&)
        {}

        GameMode& operator=(const GameMode&)
        {
            return *this;
        }

    private:

        GameModeType m_gameModeType;
        std::chrono::time_point<std::chrono::steady_clock> m_timeReset;

        // ---------------------------------------------------------------------------

    }; // class GameMode

    class DeathMatchMode : public GameMode
    {
    public:

        DeathMatchMode();
        virtual ~DeathMatchMode();

        virtual void Reset() override;
        virtual bool checkWinningConditions() override;

        unsigned int getTimeLimitSecs() const;
        void SetTimeLimitSecs(unsigned int secs);

        unsigned int getFragLimit() const;
        void SetFragLimit(unsigned int limit);

        virtual bool addPlayer(const Player& player) override;
        virtual bool updatePlayer(const Player& player) override;
        virtual bool removePlayer(const Player& player) override;
        virtual void ShowObjectives(PR00FsUltimateRenderingEngine& pure, pge_network::PgeNetwork& network) override;

    protected:

        // TODO: explicitly delete these
        DeathMatchMode(const DeathMatchMode& dmm) : GameMode(dmm.getGameModeType())
        {}

        DeathMatchMode& operator=(const DeathMatchMode&)
        {
            return *this;
        }

    private:

        static int comparePlayers(int p1frags, int p2frags, int p1deaths, int p2deaths);
        
        // ---------------------------------------------------------------------------

        unsigned int m_nTimeLimitSecs;
        unsigned int m_nFragLimit;
        bool m_bWon;

    }; // class DeathMatchMode

} // namespace proofps_dd