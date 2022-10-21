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

#include "../../../CConsole/CConsole/src/CConsole.h"

namespace proofps_dd
{

    enum class GameModeType
    {
        DeathMatch,
        TeamDeathMatch,
        TeamRoundGame
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

        virtual void Reset();
        virtual bool checkWinningConditions() const = 0;

    protected:

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
        virtual bool checkWinningConditions() const override;

        unsigned int getTimeLimitSecs() const;
        void SetTimeLimitSecs(unsigned int secs);

        unsigned int getFragLimit() const;
        void SetFragLimit(unsigned int limit);

    protected:

        // TODO: explicitly delete these
        DeathMatchMode(const DeathMatchMode& dmm) : GameMode(dmm.getGameModeType())
        {}

        DeathMatchMode& operator=(const DeathMatchMode&)
        {
            return *this;
        }

    private:

        // ---------------------------------------------------------------------------

        unsigned int m_nTimeLimitSecs;
        unsigned int m_nFragLimit;

    }; // class DeathMatchMode

} // namespace proofps_dd