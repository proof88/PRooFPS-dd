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
        SoLoud::Wav m_sndLetsgo;
        SoLoud::Wav m_sndChangeWeapon;
        SoLoud::Wav m_sndPlayerDie;

    protected:

    private:

    }; // class Sounds

} // namespace proofps_dd