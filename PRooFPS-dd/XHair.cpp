/*
    ###################################################################################
    XHair.cpp
    Crosshair class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "XHair.h"

static const PureColor clrDefault{ 255, 203, 50, 255 };
static const PureColor clrEmpty{ 255, 0, 0, 255 };
static const PureColor clrCooldown{ 100, 100, 100, 255 };

static constexpr TPureUByte nHighlightAlpha = 80;


// ############################### PUBLIC ################################


const char* proofps_dd::XHair::getLoggerModuleName()
{
    return "XHair";
}

CConsole& proofps_dd::XHair::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::XHair::XHair(PGE& pge) :
    m_pge(pge),
    m_pObjXHair(pge.getPure().getObject3DManager().createPlane(32.f, 32.f)),
    m_pObjDebugCube(pge.getPure().getObject3DManager().createCube(0.02f)),
    m_vHLightRects{
        pge.getPure().getObject3DManager().createPlane(1.f, 1.f),
        pge.getPure().getObject3DManager().createPlane(1.f, 1.f),
        pge.getPure().getObject3DManager().createPlane(1.f, 1.f),
        pge.getPure().getObject3DManager().createPlane(1.f, 1.f) }
{
    if (!m_pObjXHair || !m_pObjDebugCube)
    {
        throw std::exception(std::string("ERROR: m_pObjXHair or m_pObjDebugCube NULL!").c_str());
    }

    for (int i = 0; i < 4; i++)
    {
        if (!m_vHLightRects[i])
        {
            throw std::exception(std::string("ERROR: a highlight obj is NULL!").c_str());
        }
    }

    m_pObjDebugCube->Hide();  // show it when need to debug xhair 2d -> 3d unprojection

    m_pObjXHair->SetStickedToScreen(true);
    m_pObjXHair->SetDoubleSided(true);
    m_pObjXHair->SetTestingAgainstZBuffer(false);
    m_pObjXHair->SetLit(false);
    // for bitmaps not having proper alpha bits (e.g. saved by irfanview or mspaint), use (PURE_SRC_ALPHA, PURE_ONE)
    // otherwise (bitmaps saved by Flash) just use (PURE_SRC_ALPHA, PURE_ONE_MINUS_SRC_ALPHA) to utilize real alpha
    
    m_pObjXHair->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE);
    //m_pObjXHair->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE_MINUS_SRC_ALPHA);

    PureTexture* const xhairtex = m_pge.getPure().getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "hud_xhair_gray.bmp").c_str());
    m_pObjXHair->getMaterial().setTexture(xhairtex);
    m_pObjXHair->Hide();

    PureTexture* const hlighttex = m_pge.getPure().getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "hud_gray.bmp").c_str());
    for (int i = 0; i < 4; i++)
    {
        m_vHLightRects[i]->SetStickedToScreen(true);
        m_vHLightRects[i]->SetDoubleSided(true);
        m_vHLightRects[i]->SetTestingAgainstZBuffer(false);
        m_vHLightRects[i]->SetLit(false);
        m_vHLightRects[i]->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE);
        m_vHLightRects[i]->getMaterial().setTexture(hlighttex);
        m_vHLightRects[i]->Hide();
    }

    handleMagLoaded();
}

proofps_dd::XHair::~XHair()
{
    for (int i = 0; i < 4; i++)
    {
        delete m_vHLightRects[i];
    }

    if (m_pObjDebugCube)
    {
        delete m_pObjDebugCube;
    }

    if (m_pObjXHair)
    {
        delete m_pObjXHair;
    }
}

PureObject3D& proofps_dd::XHair::getObject3D()
{
    assert(m_pObjXHair); // otherwise ctor would had already thrown
    return *m_pObjXHair;
}

void proofps_dd::XHair::show()
{
    assert(m_pObjXHair); // otherwise ctor would had already thrown
    m_pObjXHair->Show();
    m_bVisible = true;
}

void proofps_dd::XHair::showInCenter()
{
    assert(m_pObjXHair); // otherwise ctor would had already thrown

    // this is to get rid of all mouse move messages that were probably queued up in the meantime (e.g. during map loading), otherwise
    // there is no use of setting cursor pos to center if enqueued messages will reposition it when PURE runs the window's processMessages().
    m_pge.getPure().getWindow().ProcessMessages();

    // getInput().getMouse().SetCursorPos() is not triggering any mouse move event and nulls out pending raw input events as well!
    m_pge.getInput().getMouse().SetCursorPos(
        m_pge.getPure().getWindow().getX() + m_pge.getPure().getWindow().getWidth() / 2,
        m_pge.getPure().getWindow().getY() + m_pge.getPure().getWindow().getHeight() / 2);

    m_pObjXHair->getPosVec().Set(0, 0, 0); // reposition to viewport center so it won't appear at random places
    m_pObjXHair->Show();
    m_prevXHairPos.x = 0.f;
    m_prevXHairPos.y = 0.f;
    m_bVisible = true;
}

void proofps_dd::XHair::hide()
{
    assert(m_pObjXHair); // otherwise ctor would had already thrown
    m_pObjXHair->Hide();
    for (int i = 0; i < 4; i++)
    {
        assert(m_vHLightRects[i]); // ctor would throw otherwise
        m_vHLightRects[i]->Hide();
    }

    m_bVisible = false;
    stopBlinking();
}

bool proofps_dd::XHair::visible() const
{
    return m_bVisible;
}

void proofps_dd::XHair::updateUnprojectedCoords(PureCamera& cam)
{
    // TODO: explicit call to this function would NOT be required if we did maintain a dirty flag and explicit setX(), setY() functions, which
    // would set the dirty flag, and the getUnprojectedCoords() would invoke updateUnprojectedCoords() if dirty flag is set!

    assert(m_pObjXHair); // ctor would throw otherwise

    /* 
       Originally, I was unsure about if we really needed to unproject the 2D xhair's position.
       Because of matrix multiplication and inversion, maybe this way is too expensive.
    
       I see 2 other ways of doing this:
        - keep the xhair 2D, and anytime mouse is moved, we should also change 2 variables: fCamPosOffsetX and fCamPosOffsetY
          based on the dx and dy variables in InputHandling::clientMouseWhenConnectedToServer(). Camera will have this position offset applied relative to
          player's position. This looks to be the easiest solution.  
          However, we still need to unproject for other feature, such as displaying name over hovered player object because 2D xhair needs to be 3D to be
          collision-tested with player objects.
    
        - change the xhair to 3D, so in InputHandling::clientMouseWhenConnectedToServer() the dx and dy variable changes will be applied to xhair's 3D position.
          With this method, pick/select can be implemented in a different way: no need to unproject, we just need collision
          logic to find out which player object collides with our xhair object!
          However, this way InputHandling::clientMouseWhenConnectedToServer() is more complicated because dx and dy should be multiplied by some factor related
          to camera FovX and/or FovY, since the linear 2D movement should be non-linear in 3D so it will actually feel linear on the screen!
          However, we still need to project xhair to screen for other feature, such as displaying player name over hovered player object because we need to know
          where exactly render the text above the 3D xhair on the screen.

       So after all, I'm still keeping the 2D xhair, and unproject it, once per frame.
       This unprojected coordinate can be used for multiple things:
        - calculate 3D position between player position and xhair position so camera can be placed there;
        - collide against player objects so I will know whose name should be displayed above xhair's 2D position.
    */
    if (!cam.project2dTo3d(
        static_cast<TPureUInt>(roundf(m_pObjXHair->getPosVec().getX()) + cam.getViewport().size.width / 2),
        static_cast<TPureUInt>(roundf(m_pObjXHair->getPosVec().getY()) + cam.getViewport().size.height / 2),
        /* in v0.2.5 this is player's Z (-1.2f as per GAME_PLAYERS_POS_Z) mapped to depth buffer: 0.9747f,
           0.f Z is 0.981f depth, but
           I was using 0.96f in earlier versions so it was a bit aligned on Z-axis, the camera was less moving away to screen corners,
           but better to use the mapped player Z since the aim of unprojecting xhair is to collide it with player objects */
        0.9747f,
        m_vecUnprojected))
    {
        //getConsole().EOLn("XHair::%s(): project2dTo3d() failed!", __func__);
    }
    else
    {
        m_pObjDebugCube->getPosVec() = m_vecUnprojected;
        //getConsole().EOLn("XHair obj X: %f, Y: %f, m_vecUnprojected X: %f, Y: %f",
        //    m_pObjXHair->getPosVec().getX(), m_pObjXHair->getPosVec().getY(),
        //    m_vecUnprojected.getX(), m_vecUnprojected.getY());
    }

    // this code snippet here is just for finding out which screen depth to use:
    //PureVector vecProjected;
    //if (cam.project3dTo2d(0.f, 0.f, 0.f /* GAME_PLAYERS_POS_Z as defined in Maps */, vecProjected))
    //{
    //    getConsole().EOLn("XHair::%s(): vecProjected X: %f, Y: %f, Z (depth): %f!", __func__, vecProjected.getX(), vecProjected.getY(), vecProjected.getZ());
    //}
}

const PureVector& proofps_dd::XHair::getUnprojectedCoords() const
{
    return m_vecUnprojected;
}

void proofps_dd::XHair::showIdText(const std::string& sText)
{
    m_sIdText = sText;
}

void proofps_dd::XHair::hideIdText()
{
    m_sIdText.clear();
}

const std::string& proofps_dd::XHair::getIdText() const
{
    return m_sIdText;
}

void proofps_dd::XHair::startBlinking()
{
    // idea is that game calls either xhair show() or hide(), while GUI can also invoke startBlinking, so game does not deal with blinking,
    // this blinking is allowed only if xhair is visible anyway, a hidden xhair wont be shown by startBlinking().
    m_timeStartedBlinking = std::chrono::steady_clock::now();
    m_bBlinking = m_bVisible;  // visibility state always overrides blinking
}

void proofps_dd::XHair::stopBlinking()
{
    m_bBlinking = false;
    // during blinking, game maybe invoked hide() for whatever reason, later game may invoke stopBlinking() also for whatever reason, anyway we should enable
    // xhair rendering only if game still did NOT invoke hide() in the meantime
    assert(m_pObjXHair); // ctor would throw otherwise
    m_pObjXHair->SetRenderingAllowed(m_bVisible);  // visibility state always overrides after-blinking true visibility state

    for (int i = 0; i < 4; i++)
    {
        assert(m_vHLightRects[i]); // ctor would throw otherwise
        m_vHLightRects[i]->SetRenderingAllowed(m_bVisible);
    }
}

void proofps_dd::XHair::handleMagEmpty()
{
    assert(m_pObjXHair); // ctor would throw otherwise
    m_pObjXHair->getMaterial(false).getTextureEnvColor().Set(
        clrEmpty.getRed(), clrEmpty.getGreen(), clrEmpty.getBlue(), clrEmpty.getAlpha());

    for (int i = 0; i < 4; i++)
    {
        assert(m_vHLightRects[i]); // ctor would throw otherwise
        m_vHLightRects[i]->getMaterial(false).getTextureEnvColor().Set(
            clrEmpty.getRed(), clrEmpty.getGreen(), clrEmpty.getBlue(), nHighlightAlpha);
    }
}

void proofps_dd::XHair::handleMagLoaded()
{
    assert(m_pObjXHair); // ctor would throw otherwise
    m_pObjXHair->getMaterial(false).getTextureEnvColor().Set(
        clrDefault.getRed(), clrDefault.getGreen(), clrDefault.getBlue(), clrDefault.getAlpha());

    for (int i = 0; i < 4; i++)
    {
        assert(m_vHLightRects[i]); // ctor would throw otherwise
        m_vHLightRects[i]->getMaterial(false).getTextureEnvColor().Set(
            clrDefault.getRed(), clrDefault.getGreen(), clrDefault.getBlue(), nHighlightAlpha);
    }
}

void proofps_dd::XHair::handleCooldownStart()
{
    assert(m_pObjXHair); // ctor would throw otherwise
    m_pObjXHair->getMaterial(false).getTextureEnvColor().Set(
        clrCooldown.getRed(), clrCooldown.getGreen(), clrCooldown.getBlue(), clrCooldown.getAlpha());

    for (int i = 0; i < 4; i++)
    {
        assert(m_vHLightRects[i]); // ctor would throw otherwise
        m_vHLightRects[i]->getMaterial(false).getTextureEnvColor().Set(
            clrCooldown.getRed(), clrCooldown.getGreen(), clrCooldown.getBlue(), nHighlightAlpha);
    }
}

void proofps_dd::XHair::handleCooldownEnd()
{
    handleMagLoaded();
}

/**
* Sets the base/initial scaling for the crosshair.
* This base scaling is basically the minimum scaling for the crosshair during the entire gameplay.
* This to be called at GUI initialization, once the GUI knows the display resolution dependent scaling factor for GUI elements.
*/
void proofps_dd::XHair::setBaseScaling(float scaleFactor)
{
    m_fBaseScaling = scaleFactor;
    getObject3D().SetScaling(scaleFactor);
}

/**
* Sets the relative scaling for the crosshair.
* This relative scaling basically represents the current weapon's momentary accuracy, you can directly pass it.
* Value of 1.f represents the base scaling which represents the weapon's base accuracy i.e. when the player is standing still and recoil factor is not considered.
* Passing negative value is considered as programming error.
*/
void proofps_dd::XHair::setRelativeScaling(float relativeScaleFactor)
{
    assert(relativeScaleFactor >= 0.f);

    m_fRelativeScaleFactor = relativeScaleFactor;
    getObject3D().SetScaling(m_fBaseScaling * relativeScaleFactor);
}

void proofps_dd::XHair::updateVisuals()
{
    // expected to be invoked every frame, by the game itself, after updating XHair object position by user input

    if (!m_bVisible)
    {
        stopBlinking();
        return;
    }

    if (m_bBlinking)
    {
        constexpr auto nBlinkPeriodMillisecs = 200;
        const auto nElapsedTimeSinceBlinkingStartMillisecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_timeStartedBlinking).count();
        assert(m_pObjXHair); // ctor would throw otherwise
        m_pObjXHair->SetRenderingAllowed(
            ((nElapsedTimeSinceBlinkingStartMillisecs / nBlinkPeriodMillisecs) % 2) == 1);
    }

    updateHighlight();
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


void proofps_dd::XHair::adjustHighlightSize(const bool& bCanShowHighlightPerXHairMovement)
{
    const bool bHighlightVisibleFinal = bCanShowHighlightPerXHairMovement && m_pObjXHair->isRenderingAllowed();
    for (int i = 0; i < 4; i++)
    {
        assert(m_vHLightRects[i]); // ctor would throw otherwise
        m_vHLightRects[i]->SetRenderingAllowed(bHighlightVisibleFinal);
    }

    if (!bHighlightVisibleFinal)
    {
        return;
    }

    constexpr float fDistanceFromXHair = 10.f;
    const float fHLlineThickness = 10.f * m_fBaseScaling;

    const float fHLupperHeight = std::max(0.f,
        (m_pge.getPure().getWindow().getClientHeight() / 2.f -
            m_pObjXHair->getPosVec().getY() -
            m_pObjXHair->getScaledSizeVec().getY() / 2.f -
            fDistanceFromXHair));
    m_vHLightRects[static_cast<int>(HighlightRect::Upper)]->SetScaling(PureVector(fHLlineThickness * m_fRelativeScaleFactor, fHLupperHeight, 1.f));
    //getConsole().EOLn("XHair::%s(): fHLupperHeight: %f!", __func__, fHLupperHeight);
    m_vHLightRects[static_cast<int>(HighlightRect::Upper)]->getPosVec().SetX(m_pObjXHair->getPosVec().getX());
    m_vHLightRects[static_cast<int>(HighlightRect::Upper)]->getPosVec().SetY(
        m_pObjXHair->getPosVec().getY() +
        fHLupperHeight / 2.f +
        m_pObjXHair->getScaledSizeVec().getY() / 2.f +
        fDistanceFromXHair);

    const float fHLlowerHeight = std::max(0.f,
        (m_pge.getPure().getWindow().getClientHeight() / 2.f +
            m_pObjXHair->getPosVec().getY() +
            m_pObjXHair->getScaledSizeVec().getY() / 2.f +
            fDistanceFromXHair));
    m_vHLightRects[static_cast<int>(HighlightRect::Lower)]->SetScaling(PureVector(fHLlineThickness * m_fRelativeScaleFactor, fHLlowerHeight, 1.f));
    //getConsole().EOLn("XHair::%s(): fHLlowerHeight: %f!", __func__, fHLlowerHeight);
    m_vHLightRects[static_cast<int>(HighlightRect::Lower)]->getPosVec().SetX(m_pObjXHair->getPosVec().getX());
    m_vHLightRects[static_cast<int>(HighlightRect::Lower)]->getPosVec().SetY(
        m_pObjXHair->getPosVec().getY() -
        fHLlowerHeight / 2.f -
        m_pObjXHair->getScaledSizeVec().getY() / 2.f -
        fDistanceFromXHair);

    const float fHLleftWidth = std::max(0.f,
        (m_pge.getPure().getWindow().getClientWidth() / 2.f +
            m_pObjXHair->getPosVec().getX() +
            m_pObjXHair->getScaledSizeVec().getX() / 2.f +
            fDistanceFromXHair));
    m_vHLightRects[static_cast<int>(HighlightRect::Left)]->SetScaling(PureVector(fHLleftWidth, fHLlineThickness * m_fRelativeScaleFactor, 1.f));
    //getConsole().EOLn("XHair::%s(): fHLleftWidth: %f!", __func__, fHLleftWidth);
    m_vHLightRects[static_cast<int>(HighlightRect::Left)]->getPosVec().SetY(m_pObjXHair->getPosVec().getY());
    m_vHLightRects[static_cast<int>(HighlightRect::Left)]->getPosVec().SetX(
        m_pObjXHair->getPosVec().getX() -
        fHLleftWidth / 2.f -
        m_pObjXHair->getScaledSizeVec().getX() / 2.f -
        fDistanceFromXHair);

    const float fHLrightWidth = std::max(0.f,
        (m_pge.getPure().getWindow().getClientWidth() / 2.f -
            m_pObjXHair->getPosVec().getX() -
            m_pObjXHair->getScaledSizeVec().getX() / 2.f -
            fDistanceFromXHair));
    m_vHLightRects[static_cast<int>(HighlightRect::Right)]->SetScaling(PureVector(fHLrightWidth, fHLlineThickness * m_fRelativeScaleFactor, 1.f));
    //getConsole().EOLn("XHair::%s(): fHLrightWidth: %f!", __func__, fHLrightWidth);
    m_vHLightRects[static_cast<int>(HighlightRect::Right)]->getPosVec().SetY(m_pObjXHair->getPosVec().getY());
    m_vHLightRects[static_cast<int>(HighlightRect::Right)]->getPosVec().SetX(
        m_pObjXHair->getPosVec().getX() +
        fHLrightWidth / 2.f +
        m_pObjXHair->getScaledSizeVec().getX() / 2.f +
        fDistanceFromXHair);
}

void proofps_dd::XHair::updateHighlight()
{
    // expected to be invoked every frame

    static std::chrono::time_point<std::chrono::steady_clock> timeXHairLastMoved;
    bool bCanShowHighlightPerXHairMovement = true;
    if ((m_pObjXHair->getPosVec().getX() != m_prevXHairPos.x)
        ||
        (m_pObjXHair->getPosVec().getY() != m_prevXHairPos.y))
    {
        timeXHairLastMoved = std::chrono::steady_clock::now();

        m_prevXHairPos.x = m_pObjXHair->getPosVec().getX();
        m_prevXHairPos.y = m_pObjXHair->getPosVec().getY();
    }
    else
    {
        constexpr std::chrono::milliseconds::rep nXHairPostMoveTimeoutMillisecs = 1000;
        if ((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timeXHairLastMoved)).count() >= nXHairPostMoveTimeoutMillisecs)
        {
            bCanShowHighlightPerXHairMovement = false;
            for (int i = 0; i < 4; i++)
            {
                assert(m_vHLightRects[i]); // ctor would throw otherwise
                m_vHLightRects[i]->SetRenderingAllowed(false);
            }
        }
    }

    // must keep adjusting highlight here, since it is needed even if xhair not being moved, still its scale can change due to shooting, etc.
    adjustHighlightSize(bCanShowHighlightPerXHairMovement);
}

