#pragma once

/*
    ###################################################################################
    Physics.h
    Physics handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include <map>

#include "CConsole.h"

#include "PGE.h"

#include "Durations.h"
#include "GameMode.h"  /* TODO: get rid of GameMode, Physics should not have it */
#include "GUI.h"
#include "Maps.h"
#include "Player.h"
#include "PlayerHandling.h"
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

        static bool colliding(const PureObject3D& a, const PureObject3D& b);
        static bool colliding_NoZ(const PureObject3D& a, const PureObject3D& b);
        static bool colliding2(
            float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
            float o2px, float o2py, float o2pz, float o2sx, float o2sy, float o2sz);
        static bool colliding2_NoZ(
            float o1px, float o1py, float o1sx, float o1sy,
            float o2px, float o2py, float o2sx, float o2sy);
        static bool colliding3(
            const PureVector& vecPosMin, const PureVector& vecPosMax,
            const PureVector& vecObjPos, const PureVector& vecObjSize);
        static float distance_NoZ(
            float o1px, float o1py,
            float o2px, float o2py);
        static float distance_NoZ(
            float o1px, float o1py,
            float o1sx, float o1sy,
            float o2px, float o2py
        );
        static float distance_NoZ_with_distancePerAxis(
            float o1px, float o1py,
            float o1sx, float o1sy,
            float o2px, float o2py,
            PureVector& vDirPerAxis,
            PureVector& vDistancePerAxis);

    protected:
        
        void serverSetAllowStrafeMidAir(bool bAllow);
        void serverSetAllowStrafeMidAirFull(bool bAllow);
        void serverSetFallDamageMultiplier(int n);
        void serverSetCollisionModeBvh(bool state);
        void serverGravity(
            XHair& xhair,
            const unsigned int& nPhysicsRate,
            proofps_dd::GameMode& gameMode /* TODO: get rid of GameMode, Physics should not have it */);
        void serverPlayerCollisionWithWalls(
            const unsigned int& nPhysicsRate,
            XHair& xhair,
            proofps_dd::GameMode& gameMode /* TODO: get rid of GameMode, Physics should not have it */,
            PureVector& vecCamShakeForce);

    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;
        proofps_dd::Durations& m_durations;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;
        bool m_bAllowStrafeMidAir;
        bool m_bAllowStrafeMidAirFull;
        int m_nFallDamageMultiplier;
        bool m_bCollisionModeBvh;

        void serverPlayerCollisionWithWalls_common_LoopKernelVertical_actualCollHandler(
            Player& player,
            const PureObject3D* obj,
            const int& iJumppad,
            const float& fPlayerHalfHeight,
            const float& fBlockSizeYhalf,
            XHair& xhair,
            PureVector& vecCamShakeForce
        );

        bool serverPlayerCollisionWithWalls_legacy_LoopKernelVertical(
            Player& player,
            const PureObject3D* obj,
            const int& iJumppad,
            const float& fPlayerHalfHeight,
            const float& fPlayerOPos1XMinusHalf,
            const float& fPlayerOPos1XPlusHalf,
            const float& fPlayerPos1YMinusHalf,
            const float& fPlayerPos1YPlusHalf,
            const float& fBlockSizeXhalf,
            const float& fBlockSizeYhalf,
            XHair& xhair,
            PureVector& vecCamShakeForce);

        bool serverPlayerCollisionWithWalls_bvh_LoopKernelVertical(
            Player& player,
            const PureObject3D* obj,
            const int& iJumppad,
            const float& fPlayerHalfHeight,
            const float& fBlockSizeYhalf,
            XHair& xhair,
            PureVector& vecCamShakeForce);

        void serverPlayerCollisionWithWalls_common_verticalCollisionAlreadyHandled(
            bool bVerticalCollisionOccured,
            Player& player,
            const float& fCurrentFallHeight);

        float serverPlayerCollisionWithWalls_legacy_handleStandup(
            Player& player,
            const float& fPlayerOPos1XMinusHalf,
            const float& fPlayerOPos1XPlusHalf);

        float serverPlayerCollisionWithWalls_bvh_handleStandup(
            Player& player);

        void serverPlayerCollisionWithWalls_common_strafe(
            XHair& xhair,
            const unsigned int& nPhysicsRate,
            Player& player,
            PureVector vecOriginalJumpForceBeforeVerticalCollisionHandled /* yes, copy it in */);

        bool serverPlayerCollisionWithWalls_common_horizontal_handleCollisionOccurred(
            bool isBvh,
            Player& player,
            const PureObject3D& wallObj,
            const PureVector& vecWallObjPos,
            const float fRealBlockSizeYhalf,
            const float fPlayerPos1YMinusHalf_2,
            const float& fPlayerHalfHeight,
            const PureVector& vecPlayerScaledSize);

        bool serverPlayerCollisionWithWalls_legacy_vertical(
            const unsigned int& nPhysicsRate,
            Player& player,
            const float& fPlayerHalfHeight,
            const PureVector& vecPlayerScaledSize,
            XHair& xhair,
            PureVector& vecCamShakeForce);

        bool serverPlayerCollisionWithWalls_legacy_horizontal(
            Player& player,
            const float& fPlayerHalfHeight,
            const PureVector& vecPlayerScaledSize);

        bool serverPlayerCollisionWithWalls_bvh_vertical(
            const unsigned int& nPhysicsRate,
            Player& player,
            const float& fPlayerHalfHeight,
            const PureVector& vecPlayerScaledSize,
            XHair& xhair,
            PureVector& vecCamShakeForce);

        bool serverPlayerCollisionWithWalls_bvh_horizontal(
            Player& player,
            const float& fPlayerHalfHeight,
            const PureVector& vecPlayerScaledSize);

        void serverPlayerCollisionWithWalls_common_updatePlayerAfterCollisionHandling(
            Player& player,
            bool bHorizontalCollisionOccured);

        void serverPlayerCollisionWithWalls_legacy(
            const unsigned int& nPhysicsRate,
            XHair& xhair,
            proofps_dd::GameMode& gameMode /* TODO: get rid of GameMode, Physics should not have it */,
            PureVector& vecCamShakeForce);
        void serverPlayerCollisionWithWalls_bvh(
            const unsigned int& nPhysicsRate,
            XHair& xhair,
            proofps_dd::GameMode& gameMode /* TODO: get rid of GameMode, Physics should not have it */,
            PureVector& vecCamShakeForce);

    }; // class Physics

} // namespace proofps_dd
