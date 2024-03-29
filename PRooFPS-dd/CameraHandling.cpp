/*
    ###################################################################################
    CameraHandling.h
    Camera handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>
#include <chrono>

#include "PURE/include/external/Math/PureTransformMatrix.h"

#include "CameraHandling.h"

static constexpr float GAME_CAM_Z = -5.0f;
static constexpr float GAME_CAM_SPEED_X = 0.1f;
static constexpr float GAME_CAM_SPEED_Y = 0.3f;


// ############################### PUBLIC ################################


proofps_dd::CameraHandling::CameraHandling(
    PGE& pge,
    proofps_dd::Durations& durations,
    proofps_dd::Maps& maps) :
    m_pge(pge),
    m_durations(durations),
    m_maps(maps)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations, maps
    // But they can be used in other functions.

    // Since this class is used to build up the PRooFPSddPGE class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. PRooFPSddPGE initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be existing at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
    assert(!pge.isGameRunning());
}


// ############################## PROTECTED ##############################


void proofps_dd::CameraHandling::cameraInitForGameStart()
{
    // we cannot set these in ctor because at that point PURE is not yet initialized
    m_pge.getPure().getCamera().SetNearPlane(0.1f);
    m_pge.getPure().getCamera().SetFarPlane(100.0f);
    m_pge.getPure().getCamera().getPosVec().Set(0, 0, GAME_CAM_Z);
    m_pge.getPure().getCamera().getTargetVec().Set(0, 0, -proofps_dd::Maps::fMapBlockSizeDepth);
}

void proofps_dd::CameraHandling::cameraPositionToMapCenter()
{
    auto& cam = m_pge.getPure().getCamera();

    cam.getPosVec().Set(
        (m_maps.getBlockPosMin().getX() + m_maps.getBlockPosMax().getX()) / 2.f,
        (m_maps.getBlockPosMin().getY() + m_maps.getBlockPosMax().getY()) / 2.f,
        GAME_CAM_Z);
    cam.getTargetVec().Set(
        cam.getPosVec().getX(),
        cam.getPosVec().getY(),
        -proofps_dd::Maps::fMapBlockSizeDepth);
}

void proofps_dd::CameraHandling::cameraUpdatePosAndAngle(
    const Player& player,
    const PureObject3D& objXHair,
    const float& fFps,
    bool bCamFollowsXHair,
    bool bCamTiltingAllowed,
    bool bCamRollAllowed)
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    cameraSmoothShakeForceTowardsZero(fFps);
    cameraUpdateShakeFactorXY(fFps);

    auto& cam = m_pge.getPure().getCamera();

    if (bCamRollAllowed && player.isSomersaulting())
    {
        cameraUpdatePosAndAngleWhenPlayerIsSomersaulting(cam, player);

        // return early, as during somersault camera rolling we don't need sophisticated camera movement
        m_durations.m_nCameraMovementDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
        return;
    }

    cameraUpdatePosAndAngleWhenPlayerIsInNormalSituation(cam, player, objXHair, fFps, bCamFollowsXHair, bCamTiltingAllowed);

    m_durations.m_nCameraMovementDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();

}

PureVector& proofps_dd::CameraHandling::cameraGetShakeForce()
{
    return m_vecCamShakeForce;
}


// ############################### PRIVATE ###############################


void proofps_dd::CameraHandling::cameraSmoothShakeForceTowardsZero(const float& fFps)
{
    const float GAME_FPS_RATE_LERP_FACTOR = (fFps - GAME_TICKRATE_MIN) / static_cast<float>(GAME_TICKRATE_MAX - GAME_TICKRATE_MIN);
    const float GAME_IMPACT_FORCE_CHANGE = PFL::lerp(2150.f, 2160.f, GAME_FPS_RATE_LERP_FACTOR);
    const float fCamShakeForceChangePerFrame = GAME_IMPACT_FORCE_CHANGE / 36.f / fFps; /* smaller number means longer shaking in time */
    if (m_vecCamShakeForce.getX() > 0.f)
    {
        m_vecCamShakeForce.SetX(m_vecCamShakeForce.getX() - fCamShakeForceChangePerFrame);
        if (m_vecCamShakeForce.getX() < 0.f)
        {
            m_vecCamShakeForce.SetX(0.f);
        }
    }
    if (m_vecCamShakeForce.getY() > 0.f)
    {
        m_vecCamShakeForce.SetY(m_vecCamShakeForce.getY() - fCamShakeForceChangePerFrame);
        if (m_vecCamShakeForce.getY() < 0.f)
        {
            m_vecCamShakeForce.SetY(0.f);
        }
    }
}

void proofps_dd::CameraHandling::cameraUpdateShakeFactorXY(const float& fFps)
{
    if ((m_vecCamShakeForce.getX() == 0.f) && (m_vecCamShakeForce.getY() == 0.f))
    {
        m_fShakeFactorX = 0.f;
        m_fShakeFactorY = 0.f;
        m_fShakeDegree = 0.f;
        return;
    }

    assert(fFps > 0.f);  // PRooFPSddPGE::updateFramesPerSecond() makes sure fFps is never 0
    m_fShakeDegree += 1200 / fFps;
    while (m_fShakeDegree >= 360.f)
    {
        m_fShakeDegree -= 360.f;
    }
    float fShakeSine = sin(m_fShakeDegree * PFL::PI / 180.f);

    m_fShakeFactorX = fShakeSine * m_vecCamShakeForce.getX() / fFps;
    m_fShakeFactorY = fShakeSine * m_vecCamShakeForce.getY() / fFps;
}

void proofps_dd::CameraHandling::cameraUpdatePosAndAngleWhenPlayerIsSomersaulting(PureCamera& cam, const Player& player)
{
    cam.getPosVec().Set(
        player.getObject3D()->getPosVec().getX() + m_fShakeFactorX,
        player.getObject3D()->getPosVec().getY() + m_fShakeFactorY,
        GAME_CAM_Z);
    cam.getTargetVec().Set(
        player.getObject3D()->getPosVec().getX() + m_fShakeFactorX,
        player.getObject3D()->getPosVec().getY() + m_fShakeFactorY,
        player.getObject3D()->getPosVec().getZ()
    );

    PureVector vecNewUp(0.f, 1.f, 0.f);
    PureTransformMatrix matRotZ;
    matRotZ.SetRotationZ((player.getObject3D()->getAngleVec().getY() == 0.f) ? -player.getSomersaultAngle() : player.getSomersaultAngle());
    vecNewUp *= matRotZ;
    cam.getUpVec() = vecNewUp;
}

void proofps_dd::CameraHandling::cameraUpdatePosAndAngleWhenPlayerIsInNormalSituation(
    PureCamera& cam,
    const Player& player,
    const PureObject3D& objXHair,
    const float& fFps,
    bool bCamFollowsXHair,
    bool bCamTiltingAllowed)
{
    constexpr unsigned int nBlocksToKeepCameraWithinMapBoundsHorizontally = 3;
    constexpr unsigned int nBlocksToKeepCameraWithinMapBottom = 5;
    constexpr unsigned int nBlocksToKeepCameraWithinMapTop = 1;

    const float fCamMinAllowedPosX =
        m_maps.width() < (nBlocksToKeepCameraWithinMapBoundsHorizontally * 2 + 1) ?
        m_maps.getBlocksVertexPosMin().getX() :
        m_maps.getBlocksVertexPosMin().getX() + (proofps_dd::Maps::fMapBlockSizeWidth * (nBlocksToKeepCameraWithinMapBoundsHorizontally));
    const float fCamMaxAllowedPosX =
        m_maps.width() < (nBlocksToKeepCameraWithinMapBoundsHorizontally * 2 + 1) ?
        m_maps.getBlocksVertexPosMax().getX() :
        m_maps.getBlocksVertexPosMin().getX() + (proofps_dd::Maps::fMapBlockSizeWidth * (m_maps.width() - nBlocksToKeepCameraWithinMapBoundsHorizontally));

    const float fCamMinAllowedPosY =
        m_maps.height() < (nBlocksToKeepCameraWithinMapBottom + 1) ?
        m_maps.getBlocksVertexPosMin().getY() :
        m_maps.getBlocksVertexPosMin().getY() + (proofps_dd::Maps::fMapBlockSizeHeight * (nBlocksToKeepCameraWithinMapBottom - 1));
    const float fCamMaxAllowedPosY =
        m_maps.height() < (nBlocksToKeepCameraWithinMapTop + 1) ?
        m_maps.getBlocksVertexPosMax().getY() :
        m_maps.getBlocksVertexPosMin().getY() + (proofps_dd::Maps::fMapBlockSizeHeight * (m_maps.height() - nBlocksToKeepCameraWithinMapTop + 1));

    float fCamPosXTarget, fCamPosYTarget;
    if (bCamFollowsXHair)
    {
        /* Unsure about if we really need to unproject the 2D xhair's position.
           Because of matrix multiplication and inversion, maybe this way is too expensive.

           I see 2 other ways of doing this:
            - keep the xhair 2D, and anytime mouse is moved, we should also change 2 variables: fCamPosOffsetX and fCamPosOffsetY
              based on the dx and dy variables in InputHandling::clientMouseWhenConnectedToServer(). Camera will have this position offset applied relative to
              player's position. This looks to be the easiest solution.
              However, on the long run we will need 3D position of xhair since we want to use pick/select method, for example,
              when hovering the xhair over other player, we should be able to tell which player is that.

            - change the xhair to 3D, so in InputHandling::clientMouseWhenConnectedToServer() the dx and dy variable changes will be applied to xhair's 3D position.
              With this method, pick/select can be implemented in a different way: no need to unproject, we just need collision
              logic to find out which player object collides with our xhair object!
        */
        PureVector vecUnprojected;
        if (!cam.project2dTo3d(
            static_cast<TPureUInt>(roundf(objXHair.getPosVec().getX()) + cam.getViewport().size.width / 2),
            static_cast<TPureUInt>(roundf(objXHair.getPosVec().getY()) + cam.getViewport().size.height / 2),
            /* in v0.1.5 this is player's Z mapped to depth buffer: 0.9747f,*/
            0.96f,
            vecUnprojected))
        {
            //getConsole().EOLn("PRooFPSddPGE::%s(): project2dTo3d() failed!", __func__);
        }
        else
        {
            //getConsole().EOLn("obj X: %f, Y: %f, vecUnprojected X: %f, Y: %f",
            //    player.getObject3D()->getPosVec().getX(), player.getObject3D()->getPosVec().getY(),
            //    vecUnprojected.getX(), vecUnprojected.getY());
        }

        fCamPosXTarget = std::min(
            fCamMaxAllowedPosX,
            std::max(fCamMinAllowedPosX, (player.getObject3D()->getPosVec().getX() + vecUnprojected.getX()) / 2));

        fCamPosYTarget = std::min(
            fCamMaxAllowedPosY,
            std::max(fCamMinAllowedPosY, (player.getObject3D()->getPosVec().getY() + vecUnprojected.getY()) / 2));
    }
    else
    {
        fCamPosXTarget = std::min(
            fCamMaxAllowedPosX,
            std::max(fCamMinAllowedPosX, player.getObject3D()->getPosVec().getX()));

        fCamPosYTarget = std::min(
            fCamMaxAllowedPosY,
            std::max(fCamMinAllowedPosY, player.getObject3D()->getPosVec().getY()));
    }

    fCamPosXTarget += m_fShakeFactorX;
    fCamPosYTarget += m_fShakeFactorY;

    PureVector vecCamPos{
        PFL::smooth(
            cam.getPosVec().getX() + m_fShakeFactorX, fCamPosXTarget, GAME_CAM_SPEED_X * fFps),
        PFL::smooth(
            cam.getPosVec().getY() + m_fShakeFactorY,
            fCamPosYTarget,
            /* if we are not following xhair, we want an eased vertical camera movement because it looks nice */
            (bCamFollowsXHair ? (GAME_CAM_SPEED_X * fFps) : (GAME_CAM_SPEED_Y * fFps))),
        GAME_CAM_Z
    };

    cam.getPosVec() = vecCamPos;
    cam.getTargetVec().Set(
        bCamTiltingAllowed ? ((vecCamPos.getX() + fCamPosXTarget) / 2.f) : vecCamPos.getX(),
        bCamTiltingAllowed ? ((vecCamPos.getY() + fCamPosYTarget) / 2.f) : vecCamPos.getY(),
        player.getObject3D()->getPosVec().getZ()
    );
    // we can always reset like this, since Up vector doesn't have effect on camera pitch/yaw
    cam.getUpVec().Set(0.f, 1.f, 0.f);
}
