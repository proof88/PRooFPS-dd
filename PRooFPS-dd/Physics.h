#pragma once

/*
    ###################################################################################
    Physics.h
    Physics handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <map>

#include "../../Console/CConsole/src/CConsole.h"

#include "PGE.h"

#include "Durations.h"
#include "GameMode.h"
#include "GUI.h"
#include "Maps.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"
#include "Sounds.h"

namespace proofps_dd
{

    class Physics :
        protected virtual proofps_dd::PlayerHandling
    {
    public:

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        Physics(
            PGE& pge,
            proofps_dd::Durations& durations,
            proofps_dd::GUI& gui,
            std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            proofps_dd::Maps& maps,
            proofps_dd::Sounds& sounds);

        Physics(const Physics&) = delete;
        Physics& operator=(const Physics&) = delete;
        Physics(Physics&&) = delete;
        Physics&& operator=(Physics&&) = delete;

    protected:

        static bool Colliding(const PureObject3D& a, const PureObject3D& b);
        static bool Colliding_NoZ(const PureObject3D& a, const PureObject3D& b);
        static bool Colliding2(
            float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
            float o2px, float o2py, float o2pz, float o2sx, float o2sy, float o2sz);
        static bool Colliding2_NoZ(
            float o1px, float o1py, float o1sx, float o1sy,
            float o2px, float o2py, float o2sx, float o2sy);
        static bool Colliding3(
            const PureVector& vecPosMin, const PureVector& vecPosMax,
            const PureVector& vecObjPos, const PureVector& vecObjSize);
        
        void serverSetAllowStrafeMidAir(bool bAllow);
        void serverSetAllowStrafeMidAirFull(bool bAllow);
        void serverGravity(PureObject3D& objXHair, const unsigned int& nPhysicsRate);
        void serverPlayerCollisionWithWalls(bool& won, const unsigned int& nPhysicsRate);

    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;
        proofps_dd::Durations& m_durations;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;
        bool m_bAllowStrafeMidAir;
        bool m_bAllowStrafeMidAirFull;

    }; // class Physics

} // namespace proofps_dd
