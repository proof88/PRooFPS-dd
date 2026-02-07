#pragma once

/*
    ###################################################################################
    CameraHandling.h
    Camera handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include <map>
#include "CConsole.h"

#include "PGE.h"

#include "Durations.h"
#include "Maps.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"
#include "XHair.h"

namespace proofps_dd
{

    class CameraHandling
    {
    public:

        enum class SpectatingView
        {
            Free,
            PlayerFollow
        };

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        CameraHandling(
            PR00FsUltimateRenderingEngine& pure,
            Durations& durations,
            Maps& maps);

        CameraHandling(const CameraHandling&) = delete;
        CameraHandling& operator=(const CameraHandling&) = delete;
        CameraHandling(CameraHandling&&) = delete;
        CameraHandling&& operator=(CameraHandling&&) = delete;

        SpectatingView& cameraGetSpectatingView();
        void cameraToggleSpectatingView();

        PureVector& cameraGetPosToFollowInSpectatorMode();
        const pge_network::PgeNetworkConnectionHandle& cameraGetPlayerConnectionHandleToFollowInSpectatingView() const;

        bool findNextValidPlayerToFollowInPlayerSpectatingView(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);
        bool findPrevValidPlayerToFollowInPlayerSpectatingView(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);
        bool findAnyValidPlayerToFollowInPlayerSpectatingView(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            PureVector& posPlayerToFollow);

    protected:
        void cameraInitForGameStart();

        void cameraPositionToMapCenter();
        
        void cameraUpdatePosAndAngle(
            pge_audio::PgeAudio& audio,
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            const Player& player,
            XHair& xhair,
            const float& fFps,
            bool bCamFollowsXHair,
            bool bCamTiltingAllowed,
            bool bCamRollAllowed);

        PureVector& cameraGetShakeForce();
    
    private:
        PR00FsUltimateRenderingEngine& m_pure;
        Durations& m_durations;
        Maps& m_maps;

        float m_fCameraMinY = 0.f;
        PureVector m_vecCamShakeForce;
        float m_fShakeFactorX = 0.f;
        float m_fShakeFactorY = 0.f;
        float m_fShakeDegree = 0.f;
        PureVector m_vecPosToFollowInFreeCameraView{};
        SpectatingView m_eSpectatingView{ SpectatingView::Free };
        pge_network::PgeNetworkConnectionHandle m_connHandlePlayerToFollowInSpectatingView{ pge_network::ServerConnHandle };

        void cameraSmoothShakeForceTowardsZero(const float& fFps);
        void cameraUpdateShakeFactorXY(const float& fFps);
        void cameraUpdatePosAndAngleToFollowPos(
            PureCamera& cam,
            const PureVector& vecFollowPos,
            const XHair& xhair,
            const float& fFps,
            bool bCamFollowsXHair,
            bool bCamTiltingAllowed);
        void cameraUpdatePosAndAngleWhenPlayerIsInSpectatorMode(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            PureCamera& cam,
            XHair& xhair,
            const float& fFps,
            bool bCamFollowsXHair,
            bool bCamTiltingAllowed);
        void cameraUpdatePosAndAngleWhenPlayerIsSomersaulting(
            PureCamera& cam,
            const Player& player);
        void cameraUpdatePosAndAngleWhenPlayerIsInNormalSituation(
            PureCamera& cam,
            const Player& player,
            const XHair& xhair,
            const float& fFps,
            bool bCamFollowsXHair,
            bool bCamTiltingAllowed);

    }; // class CameraHandling

} // namespace proofps_dd
