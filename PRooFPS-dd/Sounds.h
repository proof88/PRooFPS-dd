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
        SoLoud::Wav m_sndLetsgo;
        SoLoud::Wav m_sndReloadStart;
        SoLoud::Wav m_sndReloadFinish;
        SoLoud::Wav m_sndShootPistol;
        SoLoud::Wav m_sndReloadBazooka;
        SoLoud::Wav m_sndChangeWeapon;
        SoLoud::Wav m_sndPlayerDie;
        SoLoud::Wav m_sndExplosion;

    protected:

    private:

    }; // class Sounds

} // namespace proofps_dd