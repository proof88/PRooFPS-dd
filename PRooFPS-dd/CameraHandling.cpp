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
#include "GameMode.h"

static constexpr float GAME_CAM_Z = -5.0f;
static constexpr float GAME_CAM_SPEED_X = 0.1f;
static constexpr float GAME_CAM_SPEED_Y = 0.3f;


// ############################### PUBLIC ################################


const char* proofps_dd::CameraHandling::getLoggerModuleName()
{
    return "CameraHandling";
}

CConsole& proofps_dd::CameraHandling::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::CameraHandling::CameraHandling(
    PR00FsUltimateRenderingEngine& pure,
    proofps_dd::Durations& durations,
    proofps_dd::Maps& maps) :
    m_pure(pure),
    m_durations(durations),
    m_maps(maps)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations, maps
    // But they can be used in other functions.

    // Since this class is used to build up the PRooFPSddPGE class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. PRooFPSddPGE initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be existing at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
    assert(!m_pure.isInitialized());
}


proofps_dd::CameraHandling::SpectatingView& proofps_dd::CameraHandling::cameraGetSpectatingView()
{
    return m_eSpectatingView;
}

/**
* Toggles between different camera spectating views.
* Note: even if it toggles to SpectatingView::PlayerFollow view, later cameraUpdatePosAndAngleWhenPlayerIsInSpectatorMode() might toggle it
*       back to SpectatingView::Free view.
*/
void proofps_dd::CameraHandling::cameraToggleSpectatingView()
{
    m_eSpectatingView =
        ((m_eSpectatingView == SpectatingView::Free) ?
          SpectatingView::PlayerFollow : SpectatingView::Free);

    CConsole::getConsoleInstance().EOLn("%s(): toggled spectating view to: %d", __func__, m_eSpectatingView);
    // no need to invoke findAnyValidPlayerToFollowInSpectatingView() in case we just toggled to PlayerFollow mode, since
    // cameraUpdatePosAndAngleWhenPlayerIsInSpectatorMode() does that anyway in every frame!
}

/**
* This is essentially controlled by cameraUpdatePosAndAngle() when we are in spectator mode, however
* it is allowed to be accessed by the user to update it in "free camera spectating view".
*/
PureVector& proofps_dd::CameraHandling::cameraGetPosToFollowInSpectatorMode()
{
    return m_vecPosToFollowInFreeCameraView;
}

/**
* @return Connection handle of the player who was most recently found to be spectated.
*         Might not be valid anymore, therefore it is advised to do a sanity check with the returned value.
*/
const pge_network::PgeNetworkConnectionHandle& proofps_dd::CameraHandling::cameraGetPlayerConnectionHandleToFollowInSpectatingView() const
{
    return m_connHandlePlayerToFollowInSpectatingView;
}

/**
* Useful when we want to iterate to the next spectatable player in "player follow spectating view".
* 
* If the function returns true, the following function returns valid data:
*  - cameraGetPlayerConnectionHandleToFollowInSpectatingView().
*
* @param mapPlayers List of all connected players where we are searching for a player to be spectated.
*
* @return True if cameraGetPlayerConnectionHandleToFollowInSpectatingView() is still valid or
*         another valid player has been found and cameraGetPlayerConnectionHandleToFollowInSpectatingView() has now been updated,
*         false otherwise.
*         False means we cannot follow anyone, need to switch back to "free camera spectating view".
*/
bool proofps_dd::CameraHandling::findNextValidPlayerToFollowInPlayerSpectatingView(
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers)
{
    if (mapPlayers.empty())
    {
        CConsole::getConsoleInstance().EOLn("%s(): no player can be spectated", __func__);
        return false;
    }

    const GameMode* const gm = GameMode::getGameMode();
    assert(gm);

    // first find the player currently being spectated
    auto it = mapPlayers.find(m_connHandlePlayerToFollowInSpectatingView);
    const auto currentSpectatedPlayerIt = it;

    if (currentSpectatedPlayerIt == mapPlayers.end())
    {
        // invalid current spectated player, just find another one
        it = mapPlayers.begin();
    }
    else
    {
        // valid current spectated player, first we are searching new one until end of container, then
        // from beginning of container until the current spectated player is found again
        ++it;
    }

    // try find a next one
    while (it != mapPlayers.end())
    {
        if (gm->isPlayerAllowedForGameplay(it->second))
        {
            m_connHandlePlayerToFollowInSpectatingView = it->first;
            CConsole::getConsoleInstance().EOLn(
                "%s(): found a player: %u, name: %s",
                __func__, m_connHandlePlayerToFollowInSpectatingView, it->second.getName().c_str());
            return true;
        }
        ++it;
    }

    if (currentSpectatedPlayerIt == mapPlayers.end())
    {
        // reached end of container without finding a next spectatable player, from beginning of container
        CConsole::getConsoleInstance().EOLn("%s(): no player can be spectated", __func__);
        return false;
    }
    else
    {
        // reached end of container without finding a next spectatable player BUT we didnt search from
        // the beginning of container so try that until the current spectated player is found again
        it = mapPlayers.begin();
        while (it != currentSpectatedPlayerIt)
        {
            if (gm->isPlayerAllowedForGameplay(it->second))
            {
                m_connHandlePlayerToFollowInSpectatingView = it->first;
                CConsole::getConsoleInstance().EOLn(
                    "%s(): found a player: %u, name: %s",
                    __func__, m_connHandlePlayerToFollowInSpectatingView, it->second.getName().c_str());
                return true;
            }
            ++it;
        }

        // didn't find anyone, last-resort is the current spectated player
        if (gm->isPlayerAllowedForGameplay(currentSpectatedPlayerIt->second))
        {
            // current m_connHandlePlayerToFollowInSpectatingView is valid
            CConsole::getConsoleInstance().EOLn(
                "%s(): found the currently spectated player only: %u, name: %s",
                __func__, m_connHandlePlayerToFollowInSpectatingView, currentSpectatedPlayerIt->second.getName().c_str());
            return true;
        }
    }

    CConsole::getConsoleInstance().EOLn("%s(): no player can be spectated", __func__);
    return false;
} // findNextValidPlayerToFollowInPlayerSpectatingView()


/**
* Useful when we want to iterate to the previous spectatable player in "player follow spectating view".
* 
* If the function returns true, the following function returns valid data:
*  - cameraGetPlayerConnectionHandleToFollowInSpectatingView().
*
* @param mapPlayers List of all connected players where we are searching for a player to be spectated.
*
* @return True if cameraGetPlayerConnectionHandleToFollowInSpectatingView() is still valid or
*         another valid player has been found and cameraGetPlayerConnectionHandleToFollowInSpectatingView() has now been updated,
*         false otherwise.
*         False means we cannot follow anyone, need to switch back to "free camera spectating view".
*/
bool proofps_dd::CameraHandling::findPrevValidPlayerToFollowInPlayerSpectatingView(
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers)
{
    if (mapPlayers.empty())
    {
        CConsole::getConsoleInstance().EOLn("%s(): no player can be spectated", __func__);
        return false;
    }

    const GameMode* const gm = GameMode::getGameMode();
    assert(gm);

    // first find the player currently being spectated
    auto it = mapPlayers.find(m_connHandlePlayerToFollowInSpectatingView);
    const auto currentSpectatedPlayerIt = it;

    if (currentSpectatedPlayerIt == mapPlayers.end())
    {
        // invalid current spectated player, just find another one
        --it; // cannot fail since container is not empty
    }
    else
    {
        // valid current spectated player, first we are searching new one until beginning of container, then
        // from end of container until the current spectated player is found again
        if (mapPlayers.size() > 1)
        {
            --it;
            if (it == mapPlayers.end())
            {
                --it;  // cannot fail since container is not empty
            }
        }
        else
        {
            // only the currently spectated player is in the container
            if (gm->isPlayerAllowedForGameplay(it->second))
            {
                // current m_connHandlePlayerToFollowInSpectatingView is valid
                CConsole::getConsoleInstance().EOLn(
                    "%s(): found the currently spectated player only: %u, name: %s",
                    __func__, m_connHandlePlayerToFollowInSpectatingView, it->second.getName().c_str());
                return true;
            }
            CConsole::getConsoleInstance().EOLn("%s(): no player can be spectated", __func__);
            return false;
        }
    }

    // try find a next one
    while (it != mapPlayers.begin())
    {
        if (gm->isPlayerAllowedForGameplay(it->second))
        {
            m_connHandlePlayerToFollowInSpectatingView = it->first;
            CConsole::getConsoleInstance().EOLn(
                "%s(): found a player: %u, name: %s",
                __func__, m_connHandlePlayerToFollowInSpectatingView, it->second.getName().c_str());
            return true;
        }
        --it;
    } 

    // it == mapPlayers.begin()
    if (gm->isPlayerAllowedForGameplay(it->second))
    {
        m_connHandlePlayerToFollowInSpectatingView = it->first;
        CConsole::getConsoleInstance().EOLn(
            "%s(): found a player: %u, name: %s",
            __func__, m_connHandlePlayerToFollowInSpectatingView, it->second.getName().c_str());
        return true;
    }

    if (currentSpectatedPlayerIt == mapPlayers.end())
    {
        // reached beginning of container without finding a next spectatable player, from end of container
        CConsole::getConsoleInstance().EOLn("%s(): no player can be spectated", __func__);
        return false;
    }
    else
    {
        // reached beginning of container without finding a next spectatable player BUT we didnt search from
        // the end of container so try that until the current spectated player is found again
        it = mapPlayers.end();
        --it; // cannot fail since container is not empty
        while (it != currentSpectatedPlayerIt)
        {
            if (gm->isPlayerAllowedForGameplay(it->second))
            {
                m_connHandlePlayerToFollowInSpectatingView = it->first;
                CConsole::getConsoleInstance().EOLn(
                    "%s(): found a player: %u, name: %s",
                    __func__, m_connHandlePlayerToFollowInSpectatingView, it->second.getName().c_str());
                return true;
            }
            --it;
        }

        // didn't find anyone, last-resort is the current spectated player
        if (gm->isPlayerAllowedForGameplay(currentSpectatedPlayerIt->second))
        {
            // current m_connHandlePlayerToFollowInSpectatingView is valid
            CConsole::getConsoleInstance().EOLn(
                "%s(): found the currently spectated player only: %u, name: %s",
                __func__, m_connHandlePlayerToFollowInSpectatingView, currentSpectatedPlayerIt->second.getName().c_str());
            return true;
        }
    }

    CConsole::getConsoleInstance().EOLn("%s(): no player can be spectated", __func__);
    return false;
} // findPrevValidPlayerToFollowInPlayerSpectatingView()

/**
* Useful when we are toggling spectating view to player follow view, or in each frame to check if followed player is still valid in player follow view.
* First checks validity of already saved player to follow (who we have been following), and finds different player only if that player is not valid anymore.
* 
* If the function returns true, the following function returns valid data:
*  - cameraGetPlayerConnectionHandleToFollowInSpectatingView().
*
* @param mapPlayers        List of all connected players where we are searching for a player to be spectated.
* @param posPlayerToFollow A valid position of the player selected to be spectated.
*                          Valid only if the function returns true.
*
* @return True if cameraGetPlayerConnectionHandleToFollowInSpectatingView() is still valid or
*         another valid player has been found and cameraGetPlayerConnectionHandleToFollowInSpectatingView() has now become valid again,
*         false otherwise.
*         False means we cannot follow anyone, need to switch back to free camera spectating view.
*/
bool proofps_dd::CameraHandling::findAnyValidPlayerToFollowInPlayerSpectatingView(
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    PureVector& posPlayerToFollow)
{
    const GameMode* const gm = GameMode::getGameMode();
    assert(gm);

    // first try to spectate the same player who we were spectating previously
    const auto playerIt = mapPlayers.find(m_connHandlePlayerToFollowInSpectatingView);
    if (mapPlayers.end() != playerIt)
    {
        if (gm->isPlayerAllowedForGameplay(playerIt->second))
        {
            // current m_connHandlePlayerToFollowInSpectatingView is valid
            posPlayerToFollow = playerIt->second.getObject3D()->getPosVec();
            return true;
        }
    }

    // try to find another player
    for (const auto& pairConnHandlePlayer : mapPlayers)
    {
        if (gm->isPlayerAllowedForGameplay(pairConnHandlePlayer.second))
        {
            m_connHandlePlayerToFollowInSpectatingView = pairConnHandlePlayer.first;
            posPlayerToFollow = pairConnHandlePlayer.second.getObject3D()->getPosVec();
            CConsole::getConsoleInstance().EOLn(
                "%s(): found a player: %u, name: %s",
                __func__,
                m_connHandlePlayerToFollowInSpectatingView,
                pairConnHandlePlayer.second.getName().c_str());
            return true;
        }
    }

    return false;
} // findAnyValidPlayerToFollowInPlayerSpectatingView()


// ############################## PROTECTED ##############################


void proofps_dd::CameraHandling::cameraInitForGameStart()
{
    // we cannot set these in ctor because at that point PURE is not yet initialized
    m_pure.getCamera().SetNearPlane(0.1f);
    m_pure.getCamera().SetFarPlane(100.0f);
    m_pure.getCamera().getPosVec().Set(0, 0, GAME_CAM_Z);
    m_pure.getCamera().getTargetVec().Set(0, 0, -proofps_dd::Maps::fMapBlockSizeDepth);
}

void proofps_dd::CameraHandling::cameraPositionToMapCenter()
{
    auto& cam = m_pure.getCamera();

    cam.getPosVec().Set(
        (m_maps.getBlockPosMin().getX() + m_maps.getBlockPosMax().getX()) / 2.f,
        (m_maps.getBlockPosMin().getY() + m_maps.getBlockPosMax().getY()) / 2.f,
        GAME_CAM_Z);
    cam.getTargetVec().Set(
        cam.getPosVec().getX(),
        cam.getPosVec().getY(),
        -proofps_dd::Maps::fMapBlockSizeDepth);

    m_vecPosToFollowInFreeCameraView.Set(
        cam.getPosVec().getX(),
        cam.getPosVec().getY(),
        proofps_dd::Maps::GAME_PLAYERS_POS_Z);
}

void proofps_dd::CameraHandling::cameraUpdatePosAndAngle(
    pge_audio::PgeAudio& audio,
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    const Player& player,
    const XHair& xhair,
    const float& fFps,
    bool bCamFollowsXHair,
    bool bCamTiltingAllowed,
    bool bCamRollAllowed)
{
    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    cameraSmoothShakeForceTowardsZero(fFps);
    cameraUpdateShakeFactorXY(fFps);

    auto& cam = m_pure.getCamera();

    if (player.isInSpectatorMode())
    {
        cameraUpdatePosAndAngleWhenPlayerIsInSpectatorMode(mapPlayers, cam, xhair, fFps, bCamFollowsXHair, bCamTiltingAllowed);
    }
    else
    {
        if (bCamRollAllowed && player.isSomersaulting())
        {
            cameraUpdatePosAndAngleWhenPlayerIsSomersaulting(cam, player);
        }
        else
        {
            cameraUpdatePosAndAngleWhenPlayerIsInNormalSituation(cam, player, xhair, fFps, bCamFollowsXHair, bCamTiltingAllowed);
        }
    }

    /*
    * Looks like it is enough if I pass (0,0,1) as At vector.
    * Anyway, in the future if there is any fishy 3D audio issue related to orientation, remember these tickets:
    *  - https://github.com/jarikomppa/soloud/issues/94
    *  - https://github.com/jarikomppa/soloud/issues/105
    */
    audio.getAudioEngineCore().set3dListenerParameters(
        cam.getPosVec().getX(), cam.getPosVec().getY(), cam.getPosVec().getZ(),
        0, 0, 1 /*cam.getTargetVec().getX(), cam.getTargetVec().getY(), cam.getTargetVec().getZ()*/,
        cam.getUpVec().getX(), cam.getUpVec().getY(), cam.getUpVec().getZ());
    audio.getAudioEngineCore().update3dAudio();

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

void proofps_dd::CameraHandling::cameraUpdatePosAndAngleToFollowPos(
    PureCamera& cam,
    const PureVector& vecFollowPos,
    const XHair& xhair,
    const float& fFps,
    bool bCamFollowsXHair,
    bool bCamTiltingAllowed)
{
    constexpr unsigned int nBlocksToKeepCameraWithinMapBoundsHorizontally = 3;
    constexpr unsigned int nBlocksToKeepCameraWithinMapBottom = 5;
    constexpr unsigned int nBlocksToKeepCameraWithinMapTop = 1;

    const float fCamMinAllowedPosXtoKeepAwayFromMapHorizontalBounds =
        m_maps.width() < (nBlocksToKeepCameraWithinMapBoundsHorizontally * 2 + 1) ?
        m_maps.getBlocksVertexPosMin().getX() :
        m_maps.getBlocksVertexPosMin().getX() + (proofps_dd::Maps::fMapBlockSizeWidth * (nBlocksToKeepCameraWithinMapBoundsHorizontally));
    const float fCamMaxAllowedPosXtoKeepAwayFromMapHorizontalBounds =
        m_maps.width() < (nBlocksToKeepCameraWithinMapBoundsHorizontally * 2 + 1) ?
        m_maps.getBlocksVertexPosMax().getX() :
        m_maps.getBlocksVertexPosMin().getX() + (proofps_dd::Maps::fMapBlockSizeWidth * (m_maps.width() - nBlocksToKeepCameraWithinMapBoundsHorizontally));

    const float fCamMinAllowedPosYtoKeepAwayFromMapVerticalBounds =
        m_maps.height() < (nBlocksToKeepCameraWithinMapBottom + 1) ?
        m_maps.getBlocksVertexPosMin().getY() :
        m_maps.getBlocksVertexPosMin().getY() + (proofps_dd::Maps::fMapBlockSizeHeight * (nBlocksToKeepCameraWithinMapBottom - 1));
    const float fCamMaxAllowedPosYtoKeepAwayFromMapVerticalBounds =
        m_maps.height() < (nBlocksToKeepCameraWithinMapTop + 1) ?
        m_maps.getBlocksVertexPosMax().getY() :
        m_maps.getBlocksVertexPosMin().getY() + (proofps_dd::Maps::fMapBlockSizeHeight * (m_maps.height() - nBlocksToKeepCameraWithinMapTop + 1));

    m_vecPosToFollowInFreeCameraView.SetX(
        std::min(
            fCamMaxAllowedPosXtoKeepAwayFromMapHorizontalBounds,
            std::max(fCamMinAllowedPosXtoKeepAwayFromMapHorizontalBounds, m_vecPosToFollowInFreeCameraView.getX())));
    
    m_vecPosToFollowInFreeCameraView.SetY(
        std::min(
            fCamMaxAllowedPosYtoKeepAwayFromMapVerticalBounds,
            std::max(fCamMinAllowedPosYtoKeepAwayFromMapVerticalBounds, m_vecPosToFollowInFreeCameraView.getY())));

    // TODO: multipliers 6 and 4 are roughly okay for 1024x768 and 1080p, but later ratio should be introduced based on actual screen width and height
    constexpr float fCamMaxXDistanceFromPlayer = Maps::fMapBlockSizeWidth * 6;
    constexpr float fCamMaxYDistanceFromPlayer = Maps::fMapBlockSizeHeight * 4;
    float fCamPosXTarget, fCamPosYTarget;
    if (bCamFollowsXHair)
    {
        const float fPosXBetweenPlayerAndCamera =
            (xhair.getUnprojectedCoords().getX() >= vecFollowPos.getX()) ?
            (std::min(
                vecFollowPos.getX() + vecFollowPos.getX() + fCamMaxXDistanceFromPlayer,
                vecFollowPos.getX() + xhair.getUnprojectedCoords().getX()) / 2.f) :
            (std::max(
                vecFollowPos.getX() + vecFollowPos.getX() - fCamMaxXDistanceFromPlayer,
                vecFollowPos.getX() + xhair.getUnprojectedCoords().getX()) / 2.f);

        const float fPosYBetweenPlayerAndCamera =
            (xhair.getUnprojectedCoords().getY() >= vecFollowPos.getY()) ?
            (std::min(
                vecFollowPos.getY() + vecFollowPos.getY() + fCamMaxYDistanceFromPlayer,
                vecFollowPos.getY() + xhair.getUnprojectedCoords().getY()) / 2.f) :
            (std::max(
                vecFollowPos.getY() + vecFollowPos.getY() - fCamMaxYDistanceFromPlayer,
                vecFollowPos.getY() + xhair.getUnprojectedCoords().getY()) / 2.f);
       
        fCamPosXTarget = std::min(
            fCamMaxAllowedPosXtoKeepAwayFromMapHorizontalBounds,
            std::max(fCamMinAllowedPosXtoKeepAwayFromMapHorizontalBounds, fPosXBetweenPlayerAndCamera));

        fCamPosYTarget = std::min(
            fCamMaxAllowedPosYtoKeepAwayFromMapVerticalBounds,
            std::max(fCamMinAllowedPosYtoKeepAwayFromMapVerticalBounds, fPosYBetweenPlayerAndCamera));
    }
    else
    {
        fCamPosXTarget = std::min(
            fCamMaxAllowedPosXtoKeepAwayFromMapHorizontalBounds,
            std::max(fCamMinAllowedPosXtoKeepAwayFromMapHorizontalBounds, vecFollowPos.getX()));

        fCamPosYTarget = std::min(
            fCamMaxAllowedPosYtoKeepAwayFromMapVerticalBounds,
            std::max(fCamMinAllowedPosYtoKeepAwayFromMapVerticalBounds, vecFollowPos.getY()));
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
        vecFollowPos.getZ()
    );

    // we can always reset Up like this, since Up vector doesn't have effect on camera pitch/yaw, only on roll!
    cam.getUpVec().Set(0.f, 1.f, 0.f);
}

void proofps_dd::CameraHandling::cameraUpdatePosAndAngleWhenPlayerIsInSpectatorMode(
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    PureCamera& cam,
    const XHair& xhair,
    const float& fFps,
    bool bCamFollowsXHair,
    bool bCamTiltingAllowed)
{
    if (m_eSpectatingView == SpectatingView::PlayerFollow)
    {
        // even we have already selected the player to be spectated, we need to check if this player
        // is still valid in every frame since they might disconnect, go spectate, etc.
        if (!findAnyValidPlayerToFollowInPlayerSpectatingView(mapPlayers, m_vecPosToFollowInFreeCameraView))
        {
            // no player found to be spectated
            m_eSpectatingView = SpectatingView::Free;
            CConsole::getConsoleInstance().EOLn("%s(): toggled spectating view from PlayerFollow back to Free due to no spectatable players!", __func__);
        }
    }

    // both free- and spectating camera views boil down to here, the difference between their path is:
    // - in free camera view, m_vecPosToFollowInFreeCameraView is controlled by the user,
    // - in player follow view, m_vecPosToFollowInFreeCameraView is set to the spectated player's position.
    cameraUpdatePosAndAngleToFollowPos(
        cam,
        m_vecPosToFollowInFreeCameraView,
        xhair,
        fFps,
        bCamFollowsXHair,
        bCamTiltingAllowed);
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
    const XHair& xhair,
    const float& fFps,
    bool bCamFollowsXHair,
    bool bCamTiltingAllowed)
{
    cameraUpdatePosAndAngleToFollowPos(
        cam,
        player.getObject3D()->getPosVec(),
        xhair,
        fFps,
        bCamFollowsXHair,
        bCamTiltingAllowed);
}
