#pragma once

/*
    ###################################################################################
    InputHandling.h
    Input handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../../../CConsole/CConsole/src/CConsole.h"

#include "../../../PGE/PGE/PGE.h"

#include "Durations.h"
#include "GameMode.h"
#include "Maps.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"

namespace proofps_dd
{

    class InputHandling : public virtual PGE
    {
    public:

        static const unsigned int m_nWeaponActionMinimumWaitMillisecondsAfterSwitch = 1000;

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        InputHandling(
            proofps_dd::Durations& m_durations,
            proofps_dd::Maps& maps);

        InputHandling(const InputHandling&) = delete;
        InputHandling& operator=(const InputHandling&) = delete;
        InputHandling(InputHandling&&) = delete;
        InputHandling&& operator=(InputHandling&&) = delete;

    protected:

        void keyboard(
            proofps_dd::GameMode& gameMode,
            int fps,
            bool& won,
            pge_network::PgePacket& pkt, Player& player);

        bool mouse(
            proofps_dd::GameMode& gameMode,
            int fps,
            bool& won,
            pge_network::PgePacket& pkt,
            proofps_dd::Player& player,
            PureObject3D& objXHair);

    private:

        // ---------------------------------------------------------------------------

        proofps_dd::Durations& m_durations;
        proofps_dd::Maps& m_maps;

        bool m_bShowGuiDemo;

        void mouseWheel(
            const short int& nMouseWheelChange,
            pge_network::PgePacket& pkt,
            proofps_dd::Player& player);

        void RegTestDumpToFile(
            proofps_dd::GameMode& gameMode,
            proofps_dd::Player& player);  // TODO: could be const if m_mapPlayers wouldnt be used with [] operator ...

    }; // class InputHandling

} // namespace proofps_dd