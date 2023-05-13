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

#include <map>
#include "../../../CConsole/CConsole/src/CConsole.h"

#include "../../../PGE/PGE/PGE.h"

#include "Durations.h"
#include "GameMode.h"
#include "Maps.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"
#include "Sounds.h"

namespace proofps_dd
{

    class InputHandling
    {
    public:

        static const unsigned int m_nWeaponActionMinimumWaitMillisecondsAfterSwitch = 1000;

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        InputHandling(
            PGE& pge,
            proofps_dd::Durations& durations,
            std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            proofps_dd::Maps& maps,
            proofps_dd::Sounds& sounds);

        InputHandling(const InputHandling&) = delete;
        InputHandling& operator=(const InputHandling&) = delete;
        InputHandling(InputHandling&&) = delete;
        InputHandling&& operator=(InputHandling&&) = delete;

    protected:

        void keyboard(
            proofps_dd::GameMode& gameMode,
            int fps,
            bool& won,
            pge_network::PgePacket& pkt, proofps_dd::Player& player);

        bool mouse(
            proofps_dd::GameMode& gameMode,
            int fps,
            bool& won,
            pge_network::PgePacket& pkt,
            proofps_dd::Player& player,
            PureObject3D& objXHair);

        bool handleUserCmdMove(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserCmdMove& msg);

    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;
        proofps_dd::Durations& m_durations;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;

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