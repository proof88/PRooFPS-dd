/*
    ###################################################################################
    Minimap.cpp
    Minimap class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "Minimap.h"

#include "GameMode.h"


// ############################### PUBLIC ################################


const char* proofps_dd::Minimap::getLoggerModuleName()
{
    return "Minimap";
}

CConsole& proofps_dd::Minimap::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::Minimap::Minimap(
    PGE& pge,
    Maps& maps,
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers) :
    m_pge(pge),
    m_maps(maps),
    m_mapPlayers(mapPlayers),
    m_pObjDebugVpTopLeft(pge.getPure().getObject3DManager().createCube(0.5f)),
    m_pObjDebugVpBottomRight(pge.getPure().getObject3DManager().createCube(0.5f))
{
    if (!m_pObjDebugVpTopLeft || !m_pObjDebugVpBottomRight)
    {
        throw std::exception(
            std::string("ERROR: a debug cube is NULL!").c_str());
    }

    m_pObjDebugVpTopLeft->Hide();    // show it when need to debug
    m_pObjDebugVpBottomRight->Hide();  // show it when need to debug
}

proofps_dd::Minimap::~Minimap()
{
    if (m_pObjDebugVpTopLeft)
    {
        delete m_pObjDebugVpTopLeft;
    }
    if (m_pObjDebugVpBottomRight)
    {
        delete m_pObjDebugVpBottomRight;
    }
}

void proofps_dd::Minimap::show()
{
    m_bVisible = true;
}

void proofps_dd::Minimap::hide()
{
    m_bVisible = false;
}

bool proofps_dd::Minimap::visible() const
{
    return m_bVisible;
}

// TODO: RFR: NOT NICE!!! This function is copied temporarily from GUI class which we dont access from here!
// Common GUI functions should be extracted into a shared common class, GUI class is overgrown now!
static ImVec4 getImVec4fromPureColor(const PureColor& pureColor)
{
    return ImVec4(
        pureColor.getRedAsFloat(),
        pureColor.getGreenAsFloat(),
        pureColor.getBlueAsFloat(),
        pureColor.getAlphaAsFloat());
}

void proofps_dd::Minimap::draw()
{
    // expected to be invoked every frame
    if (!visible())
    {
        return;
    }

    if (!m_pge.getConfigProfiles().getVars()[Minimap::szCvarGuiMinimapShow].getAsBool())
    {
        return;
    }

    if ((m_maps.width() * m_maps.height()) == 0)
    {
        // map not yet loaded but GUI is already rendering the MainMenuState::None path, i.e. we are NOT in main menu
        return;
    }

    static constexpr auto nMinimapPosLeft = 10;
    static constexpr auto nMinimapPosTop = 10;
    const float fAlpha =
        m_pge.getConfigProfiles().getVars()[Minimap::szCvarGuiMinimapTransparent].getAsBool() ? 0.8f : 1.f;
    const ImVec4 clrMinimapBgVec4 = ImVec4(0.f, 0.57f, 0.f, fAlpha);
    const ImVec4 clrMinimapBorderVec4 = ImVec4(0.f, 0.0f, 0.f, fAlpha);
    const ImVec4 clrViewportRectBgVec4 = ImVec4(0.f, 0.69f, 0.f, fAlpha);
    const ImVec4 clrPlayerRectBgVec4 = ImVec4(1.f, 1.f, 1.f, fAlpha);

    const auto minimapSize = getMinimapSizeInPixels();
    if ((minimapSize.x <= 5) || (minimapSize.y <= 5))
    {
        // there is no use of drawing anything if we are on such small map
        return;
    }

    ImDrawList* const dl = ImGui::GetWindowDrawList();
    assert(dl);

    // draw minimap background rect

    const auto clrMinimapBgU32 = ImGui::GetColorU32(clrMinimapBgVec4);
    dl->AddRectFilled(
        ImVec2(nMinimapPosLeft, nMinimapPosTop),
        ImVec2(nMinimapPosLeft + minimapSize.x, nMinimapPosTop + minimapSize.y),
        clrMinimapBgU32);

    // draw inside viewport rect

    /* for viewport rect, we generate world-space coords of the top-left and bottom-right corners of the PURE camera viewport, and from these coords
       we can calculate where the 2D viewport rect should be located on the minimap */
    PureVector vecCamViewportTopLeftInWorldSpace;
    if (!m_pge.getPure().getCamera().project2dTo3d(
        0,
        static_cast<TPureUInt>(roundf(m_pge.getPure().getCamera().getViewport().size.height - 1)),
        /* in v0.2.5 this is player's Z (-1.2f as per GAME_PLAYERS_POS_Z) mapped to depth buffer: 0.9747f,
           I'm using it now, however in the future if camera Z position can be change, we will need a dynamic value here */
        0.9747f,
        vecCamViewportTopLeftInWorldSpace))
    {
        //getConsole().EOLn("drawMinimap::%s(): project2dTo3d(0,0,...) failed!", __func__);
        return;
    }
    m_pObjDebugVpTopLeft->getPosVec() = vecCamViewportTopLeftInWorldSpace;

    PureVector vecCamViewportBottomRightInWorldSpace;
    if (!m_pge.getPure().getCamera().project2dTo3d(
        static_cast<TPureUInt>(roundf(m_pge.getPure().getCamera().getViewport().size.width - 1)),
        0,
        /* in v0.2.5 this is player's Z (-1.2f as per GAME_PLAYERS_POS_Z) mapped to depth buffer: 0.9747f,
           I'm using it now, same way as in XHair, however in the future if camera Z position can be change, we will need a dynamic value here */
        0.9747f,
        vecCamViewportBottomRightInWorldSpace))
    {
        //getConsole().EOLn("drawMinimap::%s(): project2dTo3d(0,0,...) failed!", __func__);
        return;
    }
    m_pObjDebugVpBottomRight->getPosVec() = vecCamViewportBottomRightInWorldSpace;

    //static int i = 0;
    //if (i++ == 300)
    //{
    //    i = 0; // for adding debug breakpoint here, which is hit every 5 seconds @ 60 FPS
    //}

    // although we also calculate rectangles to stay inside the minimap, it is good to use a clip rectangle just in case ...
    ImGui::PushClipRect(
        ImVec2(nMinimapPosLeft, nMinimapPosTop),
        ImVec2(nMinimapPosLeft + minimapSize.x + 1, nMinimapPosTop + minimapSize.y + 1),
        false);

    const ImVec2 fViewportRectTopLeft2D(
        nMinimapPosLeft + getMinimapXfromWorldSpaceX(vecCamViewportTopLeftInWorldSpace.getX()),
        nMinimapPosTop + getMinimapYfromWorldSpaceY(vecCamViewportTopLeftInWorldSpace.getY()));
    const ImVec2 fViewportRectBottomRight2D(
        nMinimapPosLeft + getMinimapXfromWorldSpaceX(vecCamViewportBottomRightInWorldSpace.getX()),
        nMinimapPosTop + getMinimapYfromWorldSpaceY(vecCamViewportBottomRightInWorldSpace.getY()));

    const auto clrViewportRectBgU32 = ImGui::GetColorU32(clrViewportRectBgVec4);
    dl->AddRectFilled(fViewportRectTopLeft2D, fViewportRectBottomRight2D, clrViewportRectBgU32);

    // draw players rects

    for (const auto& playerPair : m_mapPlayers)
    {
        const auto& player = playerPair.second;
        if ((player.getHealth() <= 0) || (!player.getObject3D()->isRenderingAllowed()))
        {
            continue;
        }

        ImVec2 fPlayerRectTopLeft2D(
            nMinimapPosLeft + getMinimapXfromWorldSpaceX(player.getObject3D()->getPosVec().getX()),
            nMinimapPosTop + getMinimapYfromWorldSpaceY(player.getObject3D()->getPosVec().getY()));

        fPlayerRectTopLeft2D.x = std::min(fPlayerRectTopLeft2D.x, nMinimapPosLeft + minimapSize.x - 1);
        fPlayerRectTopLeft2D.y = std::min(fPlayerRectTopLeft2D.y, nMinimapPosTop + minimapSize.y - 1);

        const auto clrPlayerRectBgU32 = ImGui::GetColorU32(getImVec4fromPureColor(TeamDeathMatchMode::getTeamColor(player.getTeamId())));
        dl->AddRectFilled(
            fPlayerRectTopLeft2D,
            ImVec2(fPlayerRectTopLeft2D.x + 2, fPlayerRectTopLeft2D.y + 2),
            clrPlayerRectBgU32);
    }

    // finally draw the border of the minimap, it is last to make sure it is overdrawing everything else on the edges of the minimap rect

    // Note: no need to set dl->Flags to 0 since I'm rounding return values in getMinimap...() functions, but I'm leaving here the commented code for some time ...
    //const auto flagsDrawListOriginal = dl->Flags;
    //dl->Flags = 0;  // in case anti-aliasing is set, we turn it off temporarily because border looks sharper and better without it
    const auto clrMinimapBorderU32 = ImGui::GetColorU32(clrMinimapBorderVec4);
    dl->AddRect(
        ImVec2(nMinimapPosLeft, nMinimapPosTop),
        ImVec2(nMinimapPosLeft + minimapSize.x + 1, nMinimapPosTop + minimapSize.y + 1),
        clrMinimapBorderU32,
        0.f /* rounding */,
        0 /* flags */,
        1.f /* thickness */);
    //dl->Flags = flagsDrawListOriginal;

    ImGui::PopClipRect();
}

ImVec2 proofps_dd::Minimap::getMinimapSizeInPixels() const
{
    if (m_maps.width() * m_maps.height() == 0)
    {
        // map not yet loaded but GUI is already rendering the MainMenuState::None path, i.e. we are NOT in main menu
        return ImVec2();
    }

    /*
    * With fScreenDimensionPercentageMax = 0.2f (meaning max 20% of screen dimension can be allocated for minimap, either on horizontal or vertical axis).
    * if ( map.width > map.height)
    * {
    *   nWidth  = screen width * fScreenDimensionPercentageMax;
    *   nHeight = nWidth * (map.height / map.width);
    * }
    * else
    * {
    *   nHeight = screen height * fScreenDimensionPercentageMax;
    *   nWidth  = nHeight * (map.width / map.height);
    * }
    *
    * Example Scenario 1:
    *  - screen width x height (pixels): 1980 x 1080
    *  - map width x height (blocks): 50 x 70
    *  - nHeight = 1080 * 0.2f = 216;
    *  - nWidth = 216 * (50/70) = 154.
    *
    * Example Scenario 2:
    *  - screen width x height (pixels): 1024 x 768
    *  - map width x height (blocks): 70 x 50
    *  - nWidth = 1024 * 0.2f = 205;
    *  - nHeight = 205 * (50/70) = 146.
    */

    static constexpr auto fScreenDimensionPercentageMax = 0.2f;
    ImVec2 retSize;
    if (m_maps.width() > m_maps.height())
    {
        retSize.x = std::roundf(ImGui::GetWindowWidth() * fScreenDimensionPercentageMax);
        retSize.y = std::roundf(retSize.x * (m_maps.height() / static_cast<float>(m_maps.width())));
    }
    else
    {
        retSize.y = std::roundf(ImGui::GetWindowHeight() * fScreenDimensionPercentageMax);
        retSize.x = std::roundf(retSize.y * (m_maps.width() / static_cast<float>(m_maps.height())));
    }

    return retSize;
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


float proofps_dd::Minimap::getMinimapXfromWorldSpaceX(const float& posWorldX) const
{
    /* make sure the input world-space coordinate is not out of map world-space bounds, where
       map world-space bounds on X-axis are: [0, m_maps.width() * m_maps.fMapBlockSizeWidth] */
    const float fConstrainedPosWorldX = std::min(
        m_maps.width() * m_maps.fMapBlockSizeWidth,
        std::max(0.f, posWorldX));

    // PURE World-space to Dear ImGui Minimap space is easy on X-axis since 0 means left, increasing X means going to the RIGHT in both spaces
    return std::roundf((fConstrainedPosWorldX / (m_maps.width() * m_maps.fMapBlockSizeWidth)) * getMinimapSizeInPixels().x);
}

float proofps_dd::Minimap::getMinimapYfromWorldSpaceY(const float& posWorldY) const
{
    const float fLowestMapPosWorldY = -1.f * static_cast<float>(m_maps.height()) * m_maps.fMapBlockSizeHeight /* 0 or negative value */;
    if (fLowestMapPosWorldY == 0.f)
    {
        // do not divide by 0, this is possible with a map where there is only 1 row of map elements!
        return 0.f;
    }
    
    /* make sure the input world-space coordinate is not out of map world-space bounds, where
       map world-space bounds on Y-axis are: [-m_maps.width() * m_maps.fMapBlockSizeWidth, 0], since
       the 1st row of map elems have Y-pos 0, the rest have decreasing thus negative Y-pos! */
    const float fConstrainedPosWorldY = /* 0 or negative value */ std::min(
        0.f,
        std::max(fLowestMapPosWorldY, posWorldY));

    // PURE World-space to Dear ImGui Minimap space is difficult on Y-axis since:
    // - PURE 2D viewport (0,0) is the CENTER, and positive Y goes UPWARDS from CENTER;
    // - Dear ImGui viewport (0,0) is the TOP LEFT, and positive Y goes DOWNWARDS from the TOP.

    // positive value since these are both negative or become 0
    const float fPosWorldYtoLowestMapPosWorldYRatio = fConstrainedPosWorldY / fLowestMapPosWorldY;
    assert(fPosWorldYtoLowestMapPosWorldYRatio <= 1.f);
    assert(fPosWorldYtoLowestMapPosWorldYRatio >= 0.f);

    /* note that the UPWARDS vs DOWNWARDS difference between PURE and Dear ImGui has been implicitly handled above where
       we created positive value from the 2 negative values division*/
    return std::roundf(fPosWorldYtoLowestMapPosWorldYRatio * getMinimapSizeInPixels().y);
}
