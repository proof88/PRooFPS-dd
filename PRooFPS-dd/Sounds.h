#pragma once

/*
    ###################################################################################
    Sounds.h
    Sounds for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

// not nice, included only for the SoLoud headers
#include "PGE.h"

namespace proofps_dd
{

    class Sounds
    {
    public:

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        Sounds();

        Sounds(const Sounds&) = delete;
        Sounds& operator=(const Sounds&) = delete;
        Sounds(Sounds&&) = delete;
        Sounds&& operator=(Sounds&&) = delete;

        // TODO: to be moved
        SoLoud::Wav m_sndMenuMusic;
        SoLoud::handle m_sndMenuMusicHandle{};
        SoLoud::Wav m_sndEndgameMusic;
        SoLoud::handle m_sndEndgameMusicHandle{};
        SoLoud::Wav m_sndRoundStart1;
        SoLoud::Wav m_sndRoundStart2;
        SoLoud::Wav m_sndRoundStart3;
        SoLoud::handle m_sndRoundStartHandle{};
        SoLoud::Wav m_sndRoundWin;
        SoLoud::handle m_sndRoundWinHandle{};
        SoLoud::Wav m_sndRoundEnd;
        SoLoud::handle m_sndRoundEndHandle{};
        SoLoud::Wav m_sndBassImpact;
        SoLoud::handle m_sndBassImpactHandle{};
        SoLoud::Wav m_sndCountdown_1s_long;
        SoLoud::Wav m_sndCountdown_1s_short;
        SoLoud::Wav m_sndChangeWeapon;
        SoLoud::Wav m_sndPlayerDie;
        SoLoud::Wav m_sndPlayerBruh;

    protected:

    private:

    }; // class Sounds

} // namespace proofps_dd