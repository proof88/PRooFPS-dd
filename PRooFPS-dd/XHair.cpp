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
    m_pObjXHair(pge.getPure().getObject3DManager().createPlane(32.f, 32.f))
{
    if (!m_pObjXHair)
    {
        throw std::exception(
            std::string("ERROR: m_pObjXHair NULL!").c_str());
    }

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


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################

