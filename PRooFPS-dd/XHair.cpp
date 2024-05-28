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
    m_pObjDebugCube(pge.getPure().getObject3DManager().createCube(0.02f))
{
    if (!m_pObjXHair || !m_pObjDebugCube)
    {
        throw std::exception(
            std::string("ERROR: m_pObjXHair NULL!").c_str());
    }

    m_pObjDebugCube->Hide();  // show it when need to debug xhair 2d -> 3d unprojection

    m_pObjXHair->SetStickedToScreen(true);
    m_pObjXHair->SetDoubleSided(true);
    m_pObjXHair->SetTestingAgainstZBuffer(false);
    m_pObjXHair->SetLit(false);
    // for bitmaps not having proper alpha bits (e.g. saved by irfanview or mspaint), use (PURE_SRC_ALPHA, PURE_ONE)
    // otherwise (bitmaps saved by Flash) just use (PURE_SRC_ALPHA, PURE_ONE_MINUS_SRC_ALPHA) to utilize real alpha
    m_pObjXHair->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE);
    PureTexture* const xhairtex = m_pge.getPure().getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "hud_xhair.bmp").c_str());
    m_pObjXHair->getMaterial().setTexture(xhairtex);
    m_pObjXHair->Hide();
}

proofps_dd::XHair::~XHair()
{
    if (m_pObjXHair)
    {
        delete m_pObjXHair;
    }
    if (m_pObjDebugCube)
    {
        delete m_pObjDebugCube;
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
}

void proofps_dd::XHair::hide()
{
    assert(m_pObjXHair); // otherwise ctor would had already thrown
    m_pObjXHair->Hide();
}

bool proofps_dd::XHair::visible() const
{
    return m_pObjXHair->isRenderingAllowed();
}

void proofps_dd::XHair::updateUnprojectedCoords(PureCamera& cam)
{
    // TODO: explicit call to this function would NOT be required if we did maintain a dirty flag and explicit setX(), setY() functions, which
    // would set the dirty flag, and the getUnprojectedCoords() would invoke updateUnprojectedCoords() if dirty flag is set!

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
        //    player.getObject3D()->getPosVec().getX(), player.getObject3D()->getPosVec().getY(),
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


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################

