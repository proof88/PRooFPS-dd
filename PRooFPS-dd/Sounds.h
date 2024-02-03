#pragma once

/*
    ###################################################################################
    Sounds.h
    Sounds for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

// not nice, included only for the SoLoud headers
#include "PGE.h"

namespace proofps_dd
{

    struct Sounds
    {
        SoLoud::Wav m_sndLetsgo;
        SoLoud::Wav m_sndReloadStart;
        SoLoud::Wav m_sndReloadFinish;
        SoLoud::Wav m_sndShootPistol;
        SoLoud::Wav m_sndShootMchgun;
        SoLoud::Wav m_sndShootDryPistol;
        SoLoud::Wav m_sndShootDryMchgun;
        SoLoud::Wav m_sndChangeWeapon;
        SoLoud::Wav m_sndPlayerDie;
    };

} // namespace proofps_dd