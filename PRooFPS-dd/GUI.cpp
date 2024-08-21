/*
    ###################################################################################
    GUI.cpp
    GUI for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "GUI.h"

#include <cassert>
#include <numeric>

#include "Consts.h"
#include "Maps.h"
#include "Player.h"
#include "PRooFPS-dd-packet.h"
#include "WeaponHandling.h"


static constexpr float fDefaultFontSizePixels = 20.f;

static const ImVec4 imClrTableRowHighlightedVec4(100 / 255.f, 50 / 255.f, 30 / 255.f, 0.7f); /* bg color for typically 1 row within a table to be highlighted */

static constexpr float fGameInfoPagesStartX = 20.f;


// ############################### PUBLIC ################################


proofps_dd::GUI& proofps_dd::GUI::getGuiInstance(
    PGE& pge,
    proofps_dd::Config& config,
    proofps_dd::Maps& maps,
    proofps_dd::Networking& networking,
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers)
{
    // we are expecting a PGE instance which is also static since PGE is singleton, it looks ok a singleton object saves ref to a singleton object ...
    // Note that the following should not be touched here as they are not fully constructed when we are here:
    // config, maps, networking, pge
    // But they can be used in other functions.
    static GUI m_guiInstance;
    m_pPge = &pge;
    m_pConfig = &config;
    m_pMaps = &maps;
    m_pNetworking = &networking;
    m_pMapPlayers = &mapPlayers;
    return m_guiInstance;
}

const char* proofps_dd::GUI::getLoggerModuleName()
{
    return "GUI";
}

CConsole& proofps_dd::GUI::getConsole()
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

void proofps_dd::GUI::initialize()
{
    // these are cannot be null since they point to always-existing instances when getGuiInstance() is called, and GUI is part of their owner: class PRooFPSddPGE.
    // TODO: change these to ref
    assert(m_pPge);
    assert(m_pMaps);
    assert(m_pMapPlayers);

    resetMenuState(false);

    // make the xhair earlier than the loading screen, so whenever loading screen is visible, xhair stays behind it!
    // this is needed because it is not trivial when to show/hide the xhair for the server.
    m_pXHair = new XHair(*m_pPge);

    m_pMinimap = new Minimap(*m_pPge, *m_pMaps, *m_pMapPlayers);

    m_pEventsDeathKill = new DeathKillEventLister();
    m_pEventsItemPickup = new EventLister(5 /* time limit secs */, 10 /* event count limit */);

    // create loading screen AFTER we created the xhair because otherwise in some situations the xhair
    // might appear ABOVE the loading screen ... this is still related to the missing PURE feature: custom Z-ordering of 2D objects.
    // This bg plane is used to cover game objects such as map, players, etc.,
    // so we can refresh the screen without showing those, for example to refresh progress bar during loading.
    m_pObjLoadingScreenBg = m_pPge->getPure().getObject3DManager().createPlane(
        m_pPge->getPure().getCamera().getViewport().size.width,
        m_pPge->getPure().getCamera().getViewport().size.height);
    m_pObjLoadingScreenBg->SetStickedToScreen(true);
    m_pObjLoadingScreenBg->SetDoubleSided(true);
    m_pObjLoadingScreenBg->SetTestingAgainstZBuffer(false);
    m_pObjLoadingScreenBg->SetLit(false);
    PureTexture* const pTexBlack = m_pPge->getPure().getTextureManager().createFromFile(
        (std::string(proofps_dd::GAME_TEXTURES_DIR) + "black.bmp").c_str());
    m_pObjLoadingScreenBg->getMaterial().setTexture(pTexBlack);

    // Logo img size should have an upper limit, otherwise it looks blurry in big window!
    const auto fLoadingScreenLogoImgWidth = std::min(825.f, m_pPge->getPure().getCamera().getViewport().size.width * 0.8f);
    m_pObjLoadingScreenLogoImg = m_pPge->getPure().getObject3DManager().createPlane(
        fLoadingScreenLogoImgWidth,
        (fLoadingScreenLogoImgWidth * 0.5f) * 0.5f);
    m_pObjLoadingScreenLogoImg->getPosVec().SetY(
        (m_pPge->getPure().getCamera().getViewport().size.height / 2) - (m_pObjLoadingScreenLogoImg->getSizeVec().getY() / 2));
    m_pObjLoadingScreenLogoImg->SetStickedToScreen(true);
    m_pObjLoadingScreenLogoImg->SetDoubleSided(true);
    m_pObjLoadingScreenLogoImg->SetTestingAgainstZBuffer(false);
    m_pObjLoadingScreenLogoImg->SetLit(false);
    PureTexture* const pTexLoadingScreenLogoImg = m_pPge->getPure().getTextureManager().createFromFile(
        (std::string(proofps_dd::GAME_TEXTURES_DIR) + "PRooFPS-dd-logo.bmp").c_str());
    m_pObjLoadingScreenLogoImg->getMaterial().setTexture(pTexLoadingScreenLogoImg);

    m_sAvailableMapsListForForceSelectComboBox.clear();
    // we force-create a string from empty " " and append it, otherwise the extra NULL char wont be appended.
    // We are force-inserting the extra NULL characters into the string because this will be actually handled by a multi-element array by Dear ImGUI
    m_sAvailableMapsListForForceSelectComboBox += std::string(" ") + '\0'; // this first elem represents the not-selected map
    for (const auto& sMapName : m_pMaps->getMapcycle().availableMapsNoChangingGet())
    {
        m_sAvailableMapsListForForceSelectComboBox += sMapName + '\0';
        //m_sAvailableMapsListForMapcycleListBox += sMapName + '\0';
    }

    /*
        Useful Dear ImGui links:
         - https://github.com/ocornut/imgui/tree/master/docs
         - https://github.com/ocornut/imgui/blob/master/docs/FAQ.md
         - https://github.com/ocornut/imgui/wiki/Useful-Extensions
         - https://github.com/pthom/imgui_bundle/
         - https://github.com/pthom/hello_imgui
         - Interactive online manual: https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html
         - GUI Editors:
           - online: https://raa.is/ImStudio/
           - https://github.com/Half-People/HImGuiEditor/tree/main
           - https://github.com/tpecholt/imrad
           - https://github.com/Code-Building/ImGuiBuilder
           - https://github.com/iamclint/ImGuiDesigner
    */

    const float fScalingFactor = m_pPge->getPure().getWindow().getClientHeight() / 768.f;

    // note that setRelativeScaling() called with weapon accuracy also has impact on the general scaling so lerp() values
    // there should be also adjusted if we modify base scaling here!
    m_pXHair->setBaseScaling(fScalingFactor * 1.5f);

    // somehow we should use both the width and height of display resolution but I'm not sure exactly how.
    // Anyway, for I will just use height for scaling the default font size.
    // So I used 20 px fonts for 1024x768, so any height bigger than that will use bigger than 20 px font size.
    // And under display resolution I actually mean window client size.
    m_fFontSizePxHudGeneral = fDefaultFontSizePixels * fScalingFactor;
    if (m_fFontSizePxHudGeneral <= 0.f)
    {
        m_fFontSizePxHudGeneral = fDefaultFontSizePixels;
        getConsole().EOLn("GUI::%s(): m_fFontSizePxHudGeneral was non-positive, reset to: %f", __func__, m_fFontSizePxHudGeneral);
    }
    else
    {
        getConsole().EOLn("GUI::%s(): m_fFontSizePxHudGeneral: %f", __func__, m_fFontSizePxHudGeneral);
    }

    ImGui::GetIO().Fonts->AddFontDefault();
    m_pImFontFragTable = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", fDefaultFontSizePixels);
    // TODO: by the time we get here, m_fFontSizePxHudGeneral should be set according to display resolution!!!
    // This also means that upon windowed/fullscreen mode change, we should reinit GUI, but this is already happening.
    /*
    * Currently there is no proper font scaling in Dear ImGui, i.e. different size font needs to be built as different font.
    * See tickets:
    *  - https://github.com/ocornut/imgui/issues/797
    *  - https://github.com/ocornut/imgui/pull/3471
    *  - https://github.com/ocornut/imgui/issues/6967
    */
    m_pImFontHudGeneral = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", m_fFontSizePxHudGeneral);
    assert(m_pImFontFragTable);
    assert(m_pImFontHudGeneral);
    assert(ImGui::GetIO().Fonts->Build());

    // no need to initialize Dear ImGui since its resources are managed by PURE/PGE
    const ImVec4 imColorDefaultGreen(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    const ImVec4 imColorHoveredGray(0.59f, 0.59f, 0.59f, 1.f);
    const ImVec4 imColorActiveGray(0.71f, 0.71f, 0.71f, 1.f);
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.23f, 0.26f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.54f, 0.54f, 0.54f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = imColorDefaultGreen;
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = imColorDefaultGreen;
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(60 / 255.f, 74 / 255.f, 23 / 255.f, 1.f);
    style.Colors[ImGuiCol_Button] = imColorDefaultGreen;
    style.Colors[ImGuiCol_ButtonHovered] = imColorHoveredGray;
    style.Colors[ImGuiCol_ButtonActive] = imColorActiveGray;
    style.Colors[ImGuiCol_Header] = imColorDefaultGreen;
    style.Colors[ImGuiCol_HeaderHovered] = imColorHoveredGray;
    style.Colors[ImGuiCol_HeaderActive] = imColorActiveGray;
    style.Colors[ImGuiCol_Separator] = imColorDefaultGreen;
    style.Colors[ImGuiCol_SeparatorHovered] = imColorDefaultGreen;
    style.Colors[ImGuiCol_SeparatorActive] = imColorDefaultGreen;
    style.Colors[ImGuiCol_ResizeGrip] = imColorDefaultGreen;
    style.Colors[ImGuiCol_ResizeGripHovered] = imColorDefaultGreen;
    style.Colors[ImGuiCol_ResizeGripActive] = imColorDefaultGreen;
    style.Colors[ImGuiCol_Tab] = imColorDefaultGreen;
    style.Colors[ImGuiCol_TabHovered] = imColorHoveredGray;
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.8f);                 // ImGuiCol_FrameBg but bit darker and transparent
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.15f, 0.18f, 0.06f, 0.8f);                 // ImGuiCol_FrameBg but bit darker and transparent
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.23f, 0.26f, 0.14f, 0.8f);              // ImGuiCol_FrameBg but transparent a bit
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.16f, 0.18f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);

    m_pPge->getPure().getUImanager().setDefaultFontSizeLegacy(static_cast<int>(std::lroundf(m_fFontSizePxHudGeneral)));
    m_pPge->getPure().getUImanager().setGuiDrawCallback(drawDearImGuiCb);
} // initialize()

void proofps_dd::GUI::shutdown()
{
    m_sAvailableMapsListForForceSelectComboBox.clear();

    if (m_pObjLoadingScreenBg && m_pObjLoadingScreenLogoImg)
    {
        delete m_pObjLoadingScreenBg;
        delete m_pObjLoadingScreenLogoImg;
        m_pObjLoadingScreenBg = nullptr;
        m_pObjLoadingScreenLogoImg = nullptr;
    }

    if (m_pEventsDeathKill)
    {
        delete m_pEventsDeathKill;
        m_pEventsDeathKill = nullptr;
    }

    if (m_pEventsItemPickup)
    {
        delete m_pEventsItemPickup;
        m_pEventsItemPickup = nullptr;
    }

    if (m_pXHair)
    {
        delete m_pXHair;
        m_pXHair = nullptr;
    }

    if (m_pMinimap)
    {
        delete m_pMinimap;
        m_pMinimap = nullptr;
    }

    // no need to destroy Dear ImGui since its resources are managed by PURE/PGE
}

const proofps_dd::GUI::MenuState& proofps_dd::GUI::getMenuState() const
{
    return m_currentMenu;
}

void proofps_dd::GUI::resetMenuState(bool bExitingFromGameSession)
{
    if (bExitingFromGameSession)
    {
        showBgWithLogo();
        if (m_pPge->getConfigProfiles().getVars()[CVAR_GUI_MAINMENU].getAsBool())
        {
            m_currentMenu = MenuState::Main;
        }
        else
        {
            m_currentMenu = MenuState::Exiting;
            m_pPge->getPure().getWindow().Close();
        }
    }
    else
    {
        if (m_pPge->getConfigProfiles().getVars()[CVAR_GUI_MAINMENU].getAsBool())
        {
            m_currentMenu = MenuState::Main;
            showBgWithLogo();
        }
        else
        {
            m_currentMenu = MenuState::None;
            hideBgWithLogo();
        }
    }
}

void proofps_dd::GUI::showLoadingScreen(int nProgress, const std::string& sMapFilename)
{
    if ( showBgWithLogo() )
    {
        textForNextFrame(
            "Loading Map: " + sMapFilename + " ... " + std::to_string(nProgress) + " %",
            200,
            m_pPge->getPure().getWindow().getClientHeight() / 2 +
            static_cast<int>(m_pObjLoadingScreenLogoImg->getPosVec().getY() -
                m_pObjLoadingScreenLogoImg->getSizeVec().getY() / 2.f));
        m_pPge->getPure().getRenderer()->RenderScene();
    }
}

void proofps_dd::GUI::hideLoadingScreen()
{
    hideBgWithLogo();
}

bool proofps_dd::GUI::showBgWithLogo()
{
    if (m_pObjLoadingScreenBg && m_pObjLoadingScreenLogoImg)
    {
        m_pObjLoadingScreenBg->Show();
        m_pObjLoadingScreenLogoImg->Show();
        return true;
    }
    return false;
}

bool proofps_dd::GUI::hideBgWithLogo()
{
    if (m_pObjLoadingScreenBg && m_pObjLoadingScreenLogoImg)
    {
        m_pObjLoadingScreenBg->Hide();
        m_pObjLoadingScreenLogoImg->Hide();
        return true;
    }
    return false;
}

proofps_dd::XHair* proofps_dd::GUI::getXHair()
{
    return m_pXHair;
}

proofps_dd::Minimap* proofps_dd::GUI::getMinimap()
{
    return m_pMinimap;
}

proofps_dd::DeathKillEventLister* proofps_dd::GUI::getDeathKillEvents()
{
    return m_pEventsDeathKill;
}

proofps_dd::EventLister* proofps_dd::GUI::getItemPickupEvents()
{
    return m_pEventsItemPickup;
}

void proofps_dd::GUI::showGameObjectives()
{
    m_gameInfoPageCurrent = GameInfoPage::FragTable;
}

void proofps_dd::GUI::hideGameObjectives()
{
    m_gameInfoPageCurrent = GameInfoPage::None;
}

void proofps_dd::GUI::showAndLoopGameInfoPages()
{
    // obviously I'm assuming GameInfoPage is contiguous
    m_gameInfoPageCurrent = static_cast<GameInfoPage>(static_cast<int>(m_gameInfoPageCurrent) + 1);
    if (m_gameInfoPageCurrent == GameInfoPage::COUNT)
    {
        
        m_gameInfoPageCurrent = GameInfoPage::None;
    }
}

void proofps_dd::GUI::textForNextFrame(const std::string& s, int nPureX, int nPureY) const
{
    m_pPge->getPure().getUImanager().textTemporalLegacy(s, nPureX, nPureY)->SetDropShadow(true);
}

void proofps_dd::GUI::textPermanent(const std::string& s, int nPureX, int nPureY) const
{
    m_pPge->getPure().getUImanager().textPermanentLegacy(s, nPureX, nPureY)->SetDropShadow(true);
}

void proofps_dd::GUI::showRespawnTimer(
    const Player* const pKillerPlayer)
{
    assert(m_pGameMode);
    if (m_pGameMode->isGameWon())
    {
        return;
    }

    assert(m_pMapPlayers);
    m_timePlayerDied = std::chrono::steady_clock::now();
    m_bShowRespawnTimer = true;

    m_sRespawnTimerExtraText.clear();
    m_sRespawnTimerExtraText2.clear();
    if (pKillerPlayer)
    {
        m_sRespawnTimerExtraText = pKillerPlayer->getName() + " killed you with";
        m_sRespawnTimerExtraText2 =
            std::to_string(pKillerPlayer->getHealth().getNew()) + "% HP and " +
            std::to_string(pKillerPlayer->getArmor().getNew()) + "% AP remaining.";
    }
}

void proofps_dd::GUI::hideRespawnTimer()
{
    m_bShowRespawnTimer = false;
    m_sRespawnTimerExtraText.clear();
    m_sRespawnTimerExtraText2.clear();
}

void proofps_dd::GUI::setGameModeInstance(proofps_dd::GameMode& gm)
{
    m_pGameMode = &gm;
}

// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


PGE* proofps_dd::GUI::m_pPge = nullptr;
proofps_dd::Config* proofps_dd::GUI::m_pConfig = nullptr;
proofps_dd::Maps* proofps_dd::GUI::m_pMaps = nullptr;
proofps_dd::Networking* proofps_dd::GUI::m_pNetworking = nullptr;
std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>* proofps_dd::GUI::m_pMapPlayers = nullptr;

proofps_dd::GUI::MenuState proofps_dd::GUI::m_currentMenu = proofps_dd::GUI::MenuState::Main;

bool proofps_dd::GUI::m_bShowRespawnTimer = false;
std::chrono::time_point<std::chrono::steady_clock> proofps_dd::GUI::m_timePlayerDied{};
std::string proofps_dd::GUI::m_sRespawnTimerExtraText;
std::string proofps_dd::GUI::m_sRespawnTimerExtraText2;

bool proofps_dd::GUI::m_bShowHealthAndArmor = false;

proofps_dd::XHair* proofps_dd::GUI::m_pXHair = nullptr;
proofps_dd::Minimap* proofps_dd::GUI::m_pMinimap = nullptr;
proofps_dd::DeathKillEventLister* proofps_dd::GUI::m_pEventsDeathKill = nullptr;
proofps_dd::EventLister* proofps_dd::GUI::m_pEventsItemPickup = nullptr;
PureObject3D* proofps_dd::GUI::m_pObjLoadingScreenBg = nullptr;
PureObject3D* proofps_dd::GUI::m_pObjLoadingScreenLogoImg = nullptr;
std::string proofps_dd::GUI::m_sAvailableMapsListForForceSelectComboBox;

ImFont* proofps_dd::GUI::m_pImFontFragTable = nullptr;
ImFont* proofps_dd::GUI::m_pImFontHudGeneral = nullptr;
float proofps_dd::GUI::m_fFontSizePxHudGeneral = fDefaultFontSizePixels; /* after init, should be adjusted based on display resolution */

proofps_dd::GUI::GameInfoPage proofps_dd::GUI::m_gameInfoPageCurrent = proofps_dd::GUI::GameInfoPage::None;
proofps_dd::GameMode* proofps_dd::GUI::m_pGameMode = nullptr;


void proofps_dd::GUI::addHintToItemByCVar(std::string& sHint, const PGEcfgVariable& cvar)
{
    // 1st technique: this will add tooltip to the GUI control itself; invoke it after you draw the specific control!
    //if (ImGui::IsItemHovered())
    //{
    //    if (sHint.empty())
    //    {
    //        sHint += cvar.getShortHint() + '\n' + '\n';
    //        for (const auto& sLongHintLine : cvar.getLongHint())
    //        {
    //            sHint += sLongHintLine + '\n';
    //        }
    //    }
    //    ImGui::SetTooltip("%s", sHint.c_str());
    //}

    // 2nd technique: this will add a grey (?) in front of the GUI control
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        if (sHint.empty())
        {
            sHint = cvar.getShortHint() + '\n';
            if (!cvar.getLongHint().empty())
            {
                sHint += + '\n';
            }
            for (const auto& sLongHintLine : cvar.getLongHint())
            {
                sHint += sLongHintLine + '\n';
            }
        }
        ImGui::SetTooltip("%s", sHint.c_str());
    }
    ImGui::SameLine();
}

float proofps_dd::GUI::calcContentStartY(const float& fContentHeight, const float& fRemainingSpaceY)
{
    const auto& fRenderAreaHeight = m_pPge->getPure().getCamera().getViewport().size.height;
    return (fRenderAreaHeight / 2 < (fRenderAreaHeight - fRemainingSpaceY)) ?
        0 : ((fContentHeight >= fRemainingSpaceY) ?
             0 : (fRenderAreaHeight / 2 - fContentHeight / 2) - (fRenderAreaHeight- fRemainingSpaceY));
}

void proofps_dd::GUI::drawMainMenu(const float& fRemainingSpaceY)
{
    constexpr float fBtnWidth = 150.f;
    constexpr float fBtnHeight = 20.f;
    constexpr float fBtnSpacingY = 30.f;
    // fContentHeight is now calculated manually, in future it should be calculated somehow automatically by pre-defining abstract elements
    constexpr float fContentHeight = 4 * (fBtnHeight + fBtnSpacingY) + fBtnSpacingY + fDefaultFontSizePixels * 2 /* 2 lines for versions texts under buttons */;
    const float fContentStartY = calcContentStartY(fContentHeight, fRemainingSpaceY);

    // in case of buttons, remove size argument (ImVec2) to auto-resize

    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x/2 - fBtnWidth/2, fContentStartY));
    if (ImGui::Button("C R E A T E  G A M E", ImVec2(fBtnWidth, fBtnHeight)))
    {
        m_currentMenu = MenuState::CreateGame;
    }

    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - fBtnWidth / 2, fContentStartY + fBtnSpacingY));
    if (ImGui::Button("J O I N  G A M E", ImVec2(fBtnWidth, fBtnHeight)))
    {
        m_currentMenu = MenuState::JoinGame;
    }

    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - fBtnWidth / 2, fContentStartY + fBtnSpacingY*2));
    if (ImGui::Button("S E T T I N G S", ImVec2(fBtnWidth, fBtnHeight)))
    {
        m_currentMenu = MenuState::Settings;
    }

    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - fBtnWidth / 2, fContentStartY + fBtnSpacingY*3));
    if (ImGui::Button("E X I T", ImVec2(fBtnWidth, fBtnHeight)))
    {
        m_currentMenu = MenuState::Exiting;
        m_pPge->getPure().getWindow().Close();
    }

    const std::string sVersion = std::string("v") + proofps_dd::GAME_VERSION;
    ImGui::SetCursorPosX(getDearImGui2DposXforWindowCenteredText(sVersion));
    ImGui::SetCursorPosY(fContentStartY + fBtnSpacingY*4);
    ImGui::TextUnformatted(sVersion.c_str());

    const std::string sLatestAlpVersion = std::string("(Latest ALP was v") + proofps_dd::GAME_VERSION_LATEST_ALP + ")";
    ImGui::SetCursorPosX(getDearImGui2DposXforWindowCenteredText(sLatestAlpVersion));
    ImGui::TextDisabled("%s", sLatestAlpVersion.c_str());
}

float proofps_dd::GUI::drawPlayerNameInputBox()
{
    PGEcfgVariable& cvarClName = m_pPge->getConfigProfiles().getVars()[Player::szCVarClName];
    ImGui::AlignTextToFramePadding();
    static std::string sHintClName; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintClName, cvarClName);
    ImGui::TextUnformatted("Player Name:");
    ImGui::SameLine();
    const auto fInputBoxPosX = ImGui::GetCursorPosX();
    
    ImGui::PushItemWidth(200);

    static char szPlayerName[MsgUserNameChangeAndBootupDone::nUserNameBufferLength];
    strncpy_s(
        szPlayerName,
        MsgUserNameChangeAndBootupDone::nUserNameBufferLength,
        cvarClName.getAsString().c_str(),
        cvarClName.getAsString().length());
    if (ImGui::InputText("##inputPlayerName", szPlayerName, IM_ARRAYSIZE(szPlayerName)))
    {
        cvarClName.Set(szPlayerName);
    }

    ImGui::PopItemWidth();

    return fInputBoxPosX;
}

void proofps_dd::GUI::drawCreateGameMenu(const float& fRemainingSpaceY)
{
    // fContentHeight is now calculated manually, in future it should be calculated somehow automatically by pre-defining abstract elements
    constexpr float fContentHeight = 315.f;
    const float fContentStartY = calcContentStartY(fContentHeight, fRemainingSpaceY);

    ImGui::SetCursorPos(ImVec2(20, fContentStartY));
    ImGui::TextUnformatted("[  C R E A T E  G A M E  ]");

    ImGui::Separator();
    ImGui::Indent();

    drawPlayerNameInputBox();

    ImGui::Separator();  // in newer Dear ImGUI there is another separator that can contain a text too!

    ImGui::TextUnformatted("[ Map Configuration ]");

    ImGui::Indent();
    {
        const auto fBasePosX = ImGui::GetCursorPosX();

        constexpr float fMapMoveBtnsWidth = 30;
        constexpr float fMapMoveBtnsHeight = 19;
        constexpr float fMapMoveBtnsVerticalDistanceFromListBoxes = 10;
        constexpr float fMapMoveBtnsVerticalDistanceFromEachOther = fMapMoveBtnsHeight + 1;
        constexpr float fMapListboxesWidth = 150;
        constexpr int nMapListboxesHeightAsItemCount = 4;
        const float fMapMoveBtnsPosX = fBasePosX + fMapListboxesWidth + fMapMoveBtnsVerticalDistanceFromListBoxes;
        const float fMapsAvailListBoxX = fMapMoveBtnsPosX + fMapMoveBtnsWidth + fMapMoveBtnsVerticalDistanceFromListBoxes;

        ImGui::TextUnformatted("Mapcycle:");

        ImGui::SameLine(fMapsAvailListBoxX);
        ImGui::TextUnformatted("Available Maps:");

        const auto fBasePosY = ImGui::GetCursorPosY();

        // first I draw the 2 listboxes, and only after them I draw the move btns in between them
        ImGui::PushItemWidth(fMapListboxesWidth);
        static int iActiveItemMapcycle = 0;
        ImGui::PushID("listBoxMapcycle"); // TODO: this I also put here as workaround for the BeginDragDropSource() assertion failure (but didnt work)
        ImGui::ListBox(
            "##listBoxMapcycle",
            &iActiveItemMapcycle,
            m_pMaps->getMapcycle().mapcycleGetAsCharPtrArray(),
            static_cast<int>(m_pMaps->getMapcycle().mapcycleGet().size()),
            nMapListboxesHeightAsItemCount);

        // start dragging from mapcycle to available maps
        if ((iActiveItemMapcycle >= 0) && (iActiveItemMapcycle < static_cast<int>(m_pMaps->getMapcycle().mapcycleGet().size())))
        {
            // TODO: I believe I should NOT use ImGuiDragDropFlags_SourceAllowNullID here, but I'm using it as workaround,
            // because without it assertion fails in BeginDragDropSource() about id being 0, even though all my controls
            // here have proper ID. We should try leaving the flag when we update Dear ImGUI.
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID/*ImGuiDragDropFlags_None*/))
            {
                // Set payload to carry the index of our item (could be anything)
                ImGui::SetDragDropPayload("dnd_listBoxMapcycle", &iActiveItemMapcycle, sizeof(int));

                // Display preview (could be anything, e.g. when dragging an image we could decide to display
                // the filename and a small preview of the image, etc.)
                ImGui::Text("Move: %s", m_pMaps->getMapcycle().mapcycleGetAsCharPtrArray()[iActiveItemMapcycle]);
                ImGui::EndDragDropSource();
            }
        }

        // end dragging from available maps to mapcycle
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_listBoxAvailMaps"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                int payload_n = *(const int*)payload->Data;
                m_pMaps->getMapcycle().mapcycleAdd_availableMapsRemove(m_pMaps->getMapcycle().availableMapsGetElem(payload_n));
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopID();

        ImGui::SameLine(fMapsAvailListBoxX);
        static int iActiveItemMapsAvailable = 0;
        ImGui::PushID("listBoxAvailMaps"); // TODO: this I also put here as workaround for the BeginDragDropSource() assertion failure (but didnt work)
        ImGui::ListBox(
            "##listBoxAvailMaps",
            &iActiveItemMapsAvailable,
            m_pMaps->getMapcycle().availableMapsGetAsCharPtrArray(),
            static_cast<int>(m_pMaps->getMapcycle().availableMapsGet().size()),
            nMapListboxesHeightAsItemCount);

        // start dragging from available maps to mapcycle
        if ((iActiveItemMapsAvailable >= 0) && (iActiveItemMapsAvailable < static_cast<int>(m_pMaps->getMapcycle().availableMapsGet().size())))
        {
            // TODO: I believe I should NOT use ImGuiDragDropFlags_SourceAllowNullID here, but I'm using it as workaround,
            // because without it assertion fails in BeginDragDropSource() about id being 0, even though all my controls
            // here have proper ID. We should try leaving the flag when we update Dear ImGUI.
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID/*ImGuiDragDropFlags_None*/))
            {
                // Set payload to carry the index of our item (could be anything)
                ImGui::SetDragDropPayload("dnd_listBoxAvailMaps", &iActiveItemMapsAvailable, sizeof(int));

                // Display preview (could be anything, e.g. when dragging an image we could decide to display
                // the filename and a small preview of the image, etc.)
                ImGui::Text("Move: %s", m_pMaps->getMapcycle().availableMapsGetAsCharPtrArray()[iActiveItemMapsAvailable]);
                ImGui::EndDragDropSource();
            }
        }

        // end dragging from mapcycle to available maps
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_listBoxMapcycle"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                int payload_n = *(const int*)payload->Data;
                m_pMaps->getMapcycle().mapcycleRemove_availableMapsAdd(m_pMaps->getMapcycle().mapcycleGet()[payload_n]);
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopID();

        ImGui::PopItemWidth();

        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY));
        if (ImGui::Button("<", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight)))
        {
            // not sure if it can be -1 but check always anyway!
            if ((iActiveItemMapsAvailable >= 0) && (iActiveItemMapsAvailable < static_cast<int>(m_pMaps->getMapcycle().availableMapsGet().size())))
            {
                // Maps ensures availableMapsGetAsCharPtrArray() and availableMapsGet() have always same number of elements!
                m_pMaps->getMapcycle().mapcycleAdd_availableMapsRemove(m_pMaps->getMapcycle().availableMapsGetElem(iActiveItemMapsAvailable));
                if (iActiveItemMapsAvailable >= static_cast<int>(m_pMaps->getMapcycle().availableMapsGet().size()))
                {
                    // if we dont do this, we might need to click inside the listbox again to have valid active item
                    iActiveItemMapsAvailable--;
                }
            }
            //else
            //{
            //    getConsole().EOLn("ERROR: iActiveItemMapsAvailable invalid index: %d!", iActiveItemMapsAvailable);
            //}
        }

        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY + fMapMoveBtnsVerticalDistanceFromEachOther));
        if (ImGui::Button("<<", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight)))
        {
            m_pMaps->getMapcycle().mapcycleAdd_availableMapsRemove();
        }

        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY + fMapMoveBtnsVerticalDistanceFromEachOther * 2));
        if (ImGui::Button(">", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight)))
        {
            // not sure if it can be -1 but check always anyway!
            if ((iActiveItemMapcycle >= 0) && (iActiveItemMapcycle < static_cast<int>(m_pMaps->getMapcycle().mapcycleGet().size())))
            {
                // Maps ensures availableMapsGetAsCharPtrArray() and availableMapsGet() have always same number of elements!
                m_pMaps->getMapcycle().mapcycleRemove_availableMapsAdd(m_pMaps->getMapcycle().mapcycleGet()[iActiveItemMapcycle]);
                if (iActiveItemMapcycle >= static_cast<int>(m_pMaps->getMapcycle().mapcycleGet().size()))
                {
                    // if we dont do this, we might need to click inside the listbox again to have valid active item
                    iActiveItemMapcycle--;
                }
            }
            //else
            //{
            //    getConsole().EOLn("ERROR: iActiveItemMapcycle invalid index: %d!", iActiveItemMapcycle);
            //}
        }

        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY + fMapMoveBtnsVerticalDistanceFromEachOther * 3));
        if (ImGui::Button(">>", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight)))
        {
            m_pMaps->getMapcycle().mapcycleRemove_availableMapsAdd();
        }

        PGEcfgVariable& cvarSvMap = m_pPge->getConfigProfiles().getVars()[proofps_dd::Maps::szCVarSvMap];

        ImGui::AlignTextToFramePadding();
        static std::string sHintSvMap; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintSvMap, cvarSvMap);
        ImGui::TextUnformatted("Force-Start on Map:");

        ImGui::SameLine();
        ImGui::PushItemWidth(150);
        static int iSelectMapStart = -1;       
        if (iSelectMapStart == -1)
        {
            // initialize current item of comboForceMapStart to what is already set in the CVAR
            iSelectMapStart = 0;
            if (!cvarSvMap.getAsString().empty() && (cvarSvMap.getAsString() != " "))
            {
                int i = 0;
                for (const auto& sTmp : m_pMaps->getMapcycle().availableMapsNoChangingGet())
                {
                    if (sTmp == cvarSvMap.getAsString())
                    {
                        iSelectMapStart = i + 1; // +1 because iSelectMapStart 0 represents first " " elem in m_sAvailableMapsListForForceSelectComboBox
                        break;
                    }
                    i++;
                }
            }
        }
        
        if (ImGui::Combo( /* this is the items_separated_by_zeros version where we don't specify item count */
            "##comboForceMapStart",
            &iSelectMapStart,
            m_sAvailableMapsListForForceSelectComboBox.c_str()))
        {
            if (iSelectMapStart == 0)
            {
                cvarSvMap.Set("");
            }
            else if (iSelectMapStart <= static_cast<int>(m_pMaps->getMapcycle().availableMapsNoChangingGet().size()))
            {
                /* first empty item as index 0 is NOT in availableMapsNoChangingGet(), that is why index can be == size() */
                cvarSvMap.Set(m_pMaps->getMapcycle().availableMapsNoChangingGetElem(iSelectMapStart-1));
            }
            else
            {
                getConsole().EOLn("ERROR: comboForceMapStart invalid index: %d!", iSelectMapStart);
            }
        }
        ImGui::PopItemWidth();
        
    } // end Map Config
    ImGui::Unindent();

    ImGui::Separator();

    ImGui::TextUnformatted("[ Game Goal ]");
    ImGui::Indent();
    {
        PGEcfgVariable& cvarSvDmFragLimit = m_pPge->getConfigProfiles().getVars()[GameMode::szCvarSvDmFragLimit];
        ImGui::AlignTextToFramePadding();
        static std::string sHintSvDmFragLimit; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintSvDmFragLimit, cvarSvDmFragLimit);
        ImGui::TextUnformatted("Frag Limit:");
        ImGui::SameLine();
        int nCvarSvDmFragLimit = cvarSvDmFragLimit.getAsInt();
        ImGui::PushItemWidth(100);
        if (ImGui::InputInt("##inputSvDmFragLimit", &nCvarSvDmFragLimit, 1, 10))
        {
            nCvarSvDmFragLimit = std::max(GameMode::nSvDmFragLimitMin, std::min(GameMode::nSvDmFragLimitMax, nCvarSvDmFragLimit));
            cvarSvDmFragLimit.Set(nCvarSvDmFragLimit);
        }
        ImGui::PopItemWidth();

        PGEcfgVariable& cvarSvDmTimeLimit = m_pPge->getConfigProfiles().getVars()[GameMode::szCvarSvDmTimeLimit];
        ImGui::AlignTextToFramePadding();
        static std::string sHintSvDmTimeLimit; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintSvDmTimeLimit, cvarSvDmTimeLimit);
        ImGui::TextUnformatted("Time Limit:");
        ImGui::SameLine();
        int nCvarSvDmTimeLimit = cvarSvDmTimeLimit.getAsInt();
        ImGui::PushItemWidth(100);
        if (ImGui::InputInt("seconds##inputSvDmTimeLimit", &nCvarSvDmTimeLimit, 1, 60))
        {
            nCvarSvDmTimeLimit = std::max(GameMode::nSvDmTimeLimitSecsMin, std::min(GameMode::nSvDmTimeLimitSecsMax, nCvarSvDmTimeLimit));
            cvarSvDmTimeLimit.Set(nCvarSvDmTimeLimit);
        }
        ImGui::PopItemWidth();
    } // end Game Goal
    ImGui::Unindent();

    ImGui::Separator();

    ImGui::TextUnformatted("[ Miscellaneous ]");
    ImGui::Indent();
    {
        PGEcfgVariable& cvarTickrate = m_pPge->getConfigProfiles().getVars()[CVAR_TICKRATE];
        bool bTR60Hz = cvarTickrate.getAsUInt() == 60u;

        ImGui::BeginGroup();
        {
            ImGui::AlignTextToFramePadding();
            static std::string sHintTickrate; // static so it is built up by addHintToItemByCVar() only once
            addHintToItemByCVar(sHintTickrate, cvarTickrate);
            ImGui::TextUnformatted("Tickrate:");
            
            ImGui::SameLine();
            if (ImGui::RadioButton("High (60 Hz)##tickrate", bTR60Hz))
            {
                cvarTickrate.Set(60u);
                bTR60Hz = false;
                m_pConfig->validate(); // easy way to force other depending CVARs also to have valid value, like CVAR_CL_UPDATERATE in this case
            }

            ImGui::SameLine();
            if (ImGui::RadioButton("Low (20 Hz)##tickrate", !bTR60Hz))
            {
                cvarTickrate.Set(20u);
                bTR60Hz = false;
                m_pConfig->validate(); // easy way to force other depending CVARs also to have valid value, like CVAR_CL_UPDATERATE in this case
            }
        }
        ImGui::EndGroup();

        ImGui::BeginGroup();
        {
            PGEcfgVariable& cvarClientUpdateRate = m_pPge->getConfigProfiles().getVars()[CVAR_CL_UPDATERATE];
            const bool bClUR60Hz = cvarClientUpdateRate.getAsUInt() == 60u;

            ImGui::AlignTextToFramePadding();
            static std::string sHintClientUpdateRate; // static so it is built up by addHintToItemByCVar() only once
            addHintToItemByCVar(sHintClientUpdateRate, cvarClientUpdateRate);
            ImGui::TextUnformatted("Client Updates:");

            ImGui::SameLine();
            // this is configuration logic here forced on the GUI, I dont know how I could avoid this special disabling/enabling behavior here,
            // but it is NOT mandatory since validate() forces all values to be correct, causing proper display of the radiobuttons too, just
            // I want to disable the invalid option too!
            if (!bTR60Hz)
            {
                ImGui::BeginDisabled(true);
            }
            if (ImGui::RadioButton("High (60 Hz)##clupdaterate", bClUR60Hz))
            {
                cvarClientUpdateRate.Set(60u);
                m_pConfig->validate(); // easy way to allow or disallow this change to take effect based on dependee CVARs, like CVAR_TICKRATE in this case
            }
            if (!bTR60Hz)
            {
                ImGui::EndDisabled();
            }

            ImGui::SameLine();
            if (ImGui::RadioButton("Low (20 Hz)##clupdaterate", !bClUR60Hz))
            {
                cvarClientUpdateRate.Set(20u);
                m_pConfig->validate(); // easy way to allow or disallow this change to take effect based on dependee CVARs, like CVAR_TICKRATE in this case
            }
        }
        ImGui::EndGroup();

        PGEcfgVariable& cvarSvFallDamageMultiplier = m_pPge->getConfigProfiles().getVars()[CVAR_SV_FALL_DAMAGE_MULTIPLIER];
        ImGui::AlignTextToFramePadding();
        static std::string sHintSvFallDamageMultiplier; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintSvFallDamageMultiplier, cvarSvFallDamageMultiplier);
        ImGui::TextUnformatted("Fall Damage Multiplier:");
        ImGui::SameLine();
        int nSvFallDamageMultiplier = cvarSvFallDamageMultiplier.getAsInt();
        ImGui::PushItemWidth(70);
        if (ImGui::SliderInt(
            "##sliderSvallDamageMultiplier",
            &nSvFallDamageMultiplier,
            0, 10, "%d",
            ImGuiSliderFlags_AlwaysClamp))
        {
            cvarSvFallDamageMultiplier.Set(nSvFallDamageMultiplier);
        }
        ImGui::PopItemWidth();

        ImGui::BeginGroup();
        {
            PGEcfgVariable& cvarSvAllowStrafeMidAir = m_pPge->getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR];

            ImGui::AlignTextToFramePadding();
            static std::string sHintMidAirStrafe; // static so it is built up by addHintToItemByCVar() only once
            addHintToItemByCVar(sHintMidAirStrafe, cvarSvAllowStrafeMidAir);
            ImGui::TextUnformatted("Mid-Air Strafe:");

            ImGui::SameLine();
            if (ImGui::RadioButton("Full##midairstrafe",
                cvarSvAllowStrafeMidAir.getAsBool() &&
                m_pPge->getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].getAsBool()))
            {
                cvarSvAllowStrafeMidAir.Set(true);
                m_pPge->getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].Set(true);
            }

            ImGui::SameLine();
            if (ImGui::RadioButton("Moderate##midairstrafe",
                cvarSvAllowStrafeMidAir.getAsBool() &&
                !m_pPge->getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].getAsBool()))
            {
                cvarSvAllowStrafeMidAir.Set(true);
                m_pPge->getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].Set(false);
            }

            ImGui::SameLine();
            if (ImGui::RadioButton("Off##midairstrafe", !cvarSvAllowStrafeMidAir.getAsBool()))
            {
                cvarSvAllowStrafeMidAir.Set(false);
                m_pPge->getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR_FULL].Set(false);
            }
        }
        ImGui::EndGroup();

        PGEcfgVariable& cvarSvSomersaultMidAirAutoCrouch = m_pPge->getConfigProfiles().getVars()[Player::szCVarSvSomersaultMidAirAutoCrouch];
        ImGui::AlignTextToFramePadding();
        static std::string sHintSvSomersaultMidAirAutoCrouch; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintSvSomersaultMidAirAutoCrouch, cvarSvSomersaultMidAirAutoCrouch);
        ImGui::TextUnformatted("Mid-Air Somersault Auto-Crouch:");
        ImGui::SameLine();
        bool bSvSomersaultMidAirAutoCrouch = cvarSvSomersaultMidAirAutoCrouch.getAsBool();
        if (ImGui::Checkbox("##cbSomersaultMidAirAutoCrouch", &bSvSomersaultMidAirAutoCrouch))
        {
            cvarSvSomersaultMidAirAutoCrouch.Set(bSvSomersaultMidAirAutoCrouch);
        }

        PGEcfgVariable& cvarSvSomersaultMidAirJumpForceMultiplier = m_pPge->getConfigProfiles().getVars()[Player::szCVarSvSomersaultMidAirJumpForceMultiplier];
        ImGui::AlignTextToFramePadding();
        static std::string sHintSvSomersaultMidAirJumpForceMultiplier; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintSvSomersaultMidAirJumpForceMultiplier, cvarSvSomersaultMidAirJumpForceMultiplier);
        ImGui::TextUnformatted("Mid-Air Somersault Jump Force Multiplier:");
        ImGui::SameLine();
        float fSvSomersaultMidAirJumpForceMultiplier = cvarSvSomersaultMidAirJumpForceMultiplier.getAsFloat();
        ImGui::PushItemWidth(70);
        if (ImGui::SliderFloat(
            "##sliderSomersaultMidAirJumpForceMultiplier",
            &fSvSomersaultMidAirJumpForceMultiplier,
            1.0f, 2.0f, "%.1f",
            ImGuiSliderFlags_AlwaysClamp))
        {
            cvarSvSomersaultMidAirJumpForceMultiplier.Set(fSvSomersaultMidAirJumpForceMultiplier);
        }
        ImGui::PopItemWidth();

        PGEcfgVariable& cvarSvDmPlayerRespawnDelaySecs = m_pPge->getConfigProfiles().getVars()[Player::szCVarSvDmRespawnDelaySecs];
        ImGui::AlignTextToFramePadding();
        static std::string sHintSvDmRespawnDelaySecs; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintSvDmRespawnDelaySecs, cvarSvDmPlayerRespawnDelaySecs);
        ImGui::TextUnformatted("Player Respawn Delay:");
        ImGui::SameLine();
        int nSvDmPlayerRespawnDelaySecs = cvarSvDmPlayerRespawnDelaySecs.getAsInt();
        ImGui::PushItemWidth(70);
        if (ImGui::SliderInt(
            "##sliderSvDmPlayerRespawnDelaySecs",
            &nSvDmPlayerRespawnDelaySecs,
            0, 5, "%d",
            ImGuiSliderFlags_AlwaysClamp))
        {
            cvarSvDmPlayerRespawnDelaySecs.Set(nSvDmPlayerRespawnDelaySecs);
        }
        ImGui::PopItemWidth();

        PGEcfgVariable& cvarSvDmPlayerRespawnInvulnerabilityDelaySecs = m_pPge->getConfigProfiles().getVars()[Player::szCVarSvDmRespawnInvulnerabilityDelaySecs];
        ImGui::AlignTextToFramePadding();
        static std::string sHintSvDmRespawnInvulnerabilityDelaySecs; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintSvDmRespawnInvulnerabilityDelaySecs, cvarSvDmPlayerRespawnInvulnerabilityDelaySecs);
        ImGui::TextUnformatted("Player Respawn Invulnerability Delay:");
        ImGui::SameLine();
        int nSvDmPlayerRespawnInvulnerabilityDelaySecs = cvarSvDmPlayerRespawnInvulnerabilityDelaySecs.getAsInt();
        ImGui::PushItemWidth(70);
        if (ImGui::SliderInt(
            "##sliderSvDmPlayerRespawnInvulnerabilityDelaySecs",
            &nSvDmPlayerRespawnInvulnerabilityDelaySecs,
            0, 3, "%d",
            ImGuiSliderFlags_AlwaysClamp))
        {
            cvarSvDmPlayerRespawnInvulnerabilityDelaySecs.Set(nSvDmPlayerRespawnInvulnerabilityDelaySecs);
        }
        ImGui::PopItemWidth();

    } // end Misc
    ImGui::Unindent();

    ImGui::Separator();

    if (ImGui::Button("< BACK"))
    {
        m_pConfig->validate();
        if (!m_pPge->getConfigProfiles().writeConfiguration())
        {
            getConsole().EOLn("ERROR: failed to save current config profile!");
        }
        if (!m_pMaps->getMapcycle().mapcycleSaveToFile())
        {
            getConsole().EOLn("ERROR: failed to save mapcycle!");
        }
        m_currentMenu = MenuState::Main;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("START >"))
    {
        m_pConfig->validate();

        assert(m_pGameMode);
        m_pGameMode->fetchConfig(m_pPge->getConfigProfiles(), m_pPge->getNetwork());

        if (m_pMaps->serverDecideFirstMapAndUpdateNextMapToBeLoaded().empty())
        {            
            getConsole().EOLn("ERROR: Server is unable to select first map!");
            PGE::showErrorDialog("Server is unable to select first map!");
        }
        else
        {
            m_pPge->getConfigProfiles().getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(true);
            if (!m_pPge->getConfigProfiles().writeConfiguration())
            {
                getConsole().EOLn("ERROR: failed to save current config profile!");
            }
            if (!m_pMaps->getMapcycle().mapcycleSaveToFile())
            {
                getConsole().EOLn("ERROR: failed to save mapcycle!");
            }
            if (!m_pNetworking->isServer() && !m_pNetworking->reinitializeNetworking())
            {
                getConsole().EOLn("ERROR: failed to reinitialize networking subsystem: switch from client to server mode!");
            }
            m_currentMenu = MenuState::None;
            m_pPge->getPure().getWindow().SetCursorVisible(false);
        }
    }

    ImGui::Unindent();
} // drawCreateGameMenu

static int filterIPv4AddressCb(ImGuiInputTextCallbackData* data)
{
    if ((data->EventChar >= '0') && (data->EventChar <= '9') || (data->EventChar == '.'))
    {
        return 0;
    }
    return 1;
}

static bool isValidIPv4(const char* IPAddress)
{
    // this is a low-effort validity checker, later when IPv6 will be supported I will simply use arpa/inet.h or winsock2.h and call
    // the appropriate converter function to check for validity.
    int a, b, c, d;
    if (sscanf_s(IPAddress, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
    {
        return (a >= 0) && (b >= 0) && (c >= 0) && (d >= 0) && (a < 256) && (b < 256) && (c < 256) && (d < 256);
    }
    return false;
}

void proofps_dd::GUI::drawJoinGameMenu(const float& fRemainingSpaceY)
{
    // fContentHeight is now calculated manually, in future it should be calculated somehow automatically by pre-defining abstract elements
    constexpr float fContentHeight = 105.f;
    const float fContentStartY = calcContentStartY(fContentHeight, fRemainingSpaceY);

    ImGui::SetCursorPos(ImVec2(20, fContentStartY));
    ImGui::TextUnformatted("[  J O I N  G A M E  ]");

    ImGui::Separator();
    ImGui::Indent();

    const auto fInputBoxPosX = drawPlayerNameInputBox();

    ImGui::Separator();

    PGEcfgVariable& cvarClServerIp = m_pPge->getConfigProfiles().getVars()[Networking::szCVarClServerIp];

    ImGui::AlignTextToFramePadding();
    static std::string sHintClServerIp; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintClServerIp, cvarClServerIp);
    ImGui::TextUnformatted("Server IP:");

    ImGui::SameLine(fInputBoxPosX);
    static char szServerIP[16];
    strncpy_s(
        szServerIP,
        sizeof(szServerIP),
        cvarClServerIp.getAsString().c_str(),
        cvarClServerIp.getAsString().length());
    ImGui::PushItemWidth(200);
    const bool bIpAddrValid = isValidIPv4(szServerIP);
    if (!bIpAddrValid)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    }
    if (ImGui::InputText("##inputServerIP", szServerIP, IM_ARRAYSIZE(szServerIP), ImGuiInputTextFlags_CallbackCharFilter, filterIPv4AddressCb))
    {
        cvarClServerIp.Set(szServerIP);
    }
    if (!bIpAddrValid)
    {
        ImGui::PopStyleColor();
    }
    ImGui::PopItemWidth();

    ImGui::Separator();

    if (ImGui::Button("< BACK"))
    {
        m_pConfig->validate();
        if (bIpAddrValid && !m_pPge->getConfigProfiles().writeConfiguration())
        {
            getConsole().EOLn("ERROR: failed to save current config profile!");
        }
        m_currentMenu = MenuState::Main;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("JOIN >"))
    {
        if (bIpAddrValid)
        {
            m_pConfig->validate();
            m_pPge->getConfigProfiles().getVars()[pge_network::PgeINetwork::CVAR_NET_SERVER].Set(false);
            if (!m_pPge->getConfigProfiles().writeConfiguration())
            {
                getConsole().EOLn("ERROR: failed to save current config profile!");
            }
            if (m_pNetworking->isServer() && !m_pNetworking->reinitializeNetworking())
            {
                getConsole().EOLn("ERROR: failed to reinitialize networking subsystem: switch from server to client mode!");
            }
            m_currentMenu = MenuState::None;
            m_pPge->getPure().getWindow().SetCursorVisible(false);
        }
    }

    ImGui::Unindent();
} // drawJoinGameMenu

void proofps_dd::GUI::showConfigApplyAndRestartDialogBox(PGEcfgVariable& cvar, const std::string& sPopupId /* must be unique! */)
{
    // it is very important: sPopupId must be unique because otherwise we won't exactly know for which setting we
    // are opening popup for, and clicking on Cancel button will most probably flip the wrong CVAR!

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(sPopupId.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("The game will restart now to apply the new configuration.");

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            getConsole().OLn("Initiating game restart to apply new settings ...");
            if (!m_pPge->getConfigProfiles().writeConfiguration())
            {
                getConsole().EOLn("ERROR: failed to save current config profile!");
            }
            m_pPge->setCookie(1); // this will force loop in WinMain() to restart the game with engine reinit
            m_currentMenu = MenuState::Exiting;
            m_pPge->getPure().getWindow().Close();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            // flip the config that we proactively flipped earlier in checkbox handler
            cvar.Set( !cvar.getAsBool() );
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void proofps_dd::GUI::drawSettingsMenu(const float& fRemainingSpaceY)
{
    // fContentHeight is now calculated manually, in future it should be calculated somehow automatically by pre-defining abstract elements
    constexpr float fContentHeight = 350.f;
    const float fContentStartY = calcContentStartY(fContentHeight, fRemainingSpaceY);

    ImGui::SetCursorPos(ImVec2(20, fContentStartY));
    ImGui::TextUnformatted("[  S E T T I N G S  ]");

    ImGui::Separator();
    ImGui::Indent();

    PGEcfgVariable& cvarSfxEnabled = m_pPge->getConfigProfiles().getVars()[pge_audio::PgeAudio::CVAR_SFX_ENABLED];
    bool bSfxEnabled = cvarSfxEnabled.getAsBool();
    ImGui::AlignTextToFramePadding();
    static std::string sHintSfxEnabled; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintSfxEnabled, cvarSfxEnabled);
    ImGui::TextUnformatted("Audio:");
    ImGui::SameLine();
    if (ImGui::Checkbox("##cbSfxEnabled", &bSfxEnabled))
    {
        // because onGameInitialized() also loads sounds, we need to restart the whole game when audio is re-enabled!
        // So anyway I'm restarting the game also if we are disabling it here.

        // even tho the user can still cancel, here we always flip the config because we are rendering the checkbox behind the dialog box based on this
        cvarSfxEnabled.Set(bSfxEnabled);

        // IMPORTANT: SAME STRING AS GIVEN TO OpenPopup() should be given to showConfigApplyAndRestartDialogBox() BELOW!
        // Otherwise in next frame, we might end up invoking showConfigApplyAndRestartDialogBox() with different CVAR somewhere above!
        // TODO: make this kind of checkbox into a single function so no mistake can be made!
        ImGui::OpenPopup("Apply Audio Setting");
    }
    showConfigApplyAndRestartDialogBox(cvarSfxEnabled, "Apply Audio Setting");

    PGEcfgVariable& cvarGfxFullscreen = m_pPge->getConfigProfiles().getVars()[PGE::CVAR_GFX_WINDOWED];
    ImGui::AlignTextToFramePadding();
    static std::string sHintGfxFullscreen; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintGfxFullscreen, cvarGfxFullscreen);
    ImGui::TextUnformatted("Fullscreen:");
    ImGui::SameLine();
    bool bFullscreen = !cvarGfxFullscreen.getAsBool();
    if (ImGui::Checkbox("##cbFullscreen", &bFullscreen))
    {
        // even tho the user can still cancel, here we always flip the config because we are rendering the checkbox behind the dialog box based on this
        cvarGfxFullscreen.Set(!bFullscreen);

        // IMPORTANT: SAME STRING AS GIVEN TO OpenPopup() should be given to showConfigApplyAndRestartDialogBox() BELOW!
        // Otherwise in next frame, we might end up invoking showConfigApplyAndRestartDialogBox) with different CVAR somewhere above!
        // TODO: make this kind of checkbox into a single function so no mistake can be made!
        ImGui::OpenPopup("Apply Video Setting");
    }
    showConfigApplyAndRestartDialogBox(cvarGfxFullscreen, "Apply Video Setting");

    PGEcfgVariable& cvarGfxVSync = m_pPge->getConfigProfiles().getVars()[PureScreen::CVAR_GFX_VSYNC];
    ImGui::AlignTextToFramePadding();
    static std::string sHintGfxVSync; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintGfxVSync, cvarGfxVSync);
    ImGui::TextUnformatted("V-Sync:");
    ImGui::SameLine();
    // TODO: VSync validation should be also moved to Config::validate(), however first the optional string argument support
    // should be implemented for validate(), otherwise it looks bad we always validate everything and try set vsync just because any
    // irrelevant config got changed!
    const bool bVSyncSupported = m_pPge->getPure().getHardwareInfo().getVideo().isVSyncSupported();
    if (!bVSyncSupported)
    {
        ImGui::BeginDisabled(true);
    }
    bool bVSync = cvarGfxVSync.getAsBool();
    if (ImGui::Checkbox("##cbVSync", &bVSync))
    {
        const bool bPrevScreenLogState = getConsole().getLoggingState(m_pPge->getPure().getScreen().getLoggerModuleName());
        getConsole().SetLoggingState(m_pPge->getPure().getScreen().getLoggerModuleName(), true);
        const bool bSetVSyncRet = m_pPge->getPure().getScreen().setVSyncEnabled(bVSync);
        getConsole().SetLoggingState(m_pPge->getPure().getScreen().getLoggerModuleName(), bPrevScreenLogState);

        cvarGfxVSync.Set(bSetVSyncRet);
        if (bSetVSyncRet != bVSync)
        {
            getConsole().EOLn("ERROR: failed to set VSync to: %b, current state: %b!", bVSync, bSetVSyncRet);
        }
    }
    if (!bVSyncSupported)
    {
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextUnformatted("V-Sync is NOT supported on this hardware!");
    }

    ImGui::BeginGroup();
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Camera Follows:");
        ImGui::SameLine();
        // dont forget there is also 3-param version of RadioButton
        if (ImGui::RadioButton("XHair and Player", m_pPge->getConfigProfiles().getVars()[CVAR_GFX_CAM_FOLLOWS_XHAIR].getAsBool()))
        {
            m_pPge->getConfigProfiles().getVars()[CVAR_GFX_CAM_FOLLOWS_XHAIR].Set(true);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Player Only", !m_pPge->getConfigProfiles().getVars()[CVAR_GFX_CAM_FOLLOWS_XHAIR].getAsBool()))
        {
            m_pPge->getConfigProfiles().getVars()[CVAR_GFX_CAM_FOLLOWS_XHAIR].Set(false);
        }
    }
    ImGui::EndGroup();

    PGEcfgVariable& cvarGfxCamTilt = m_pPge->getConfigProfiles().getVars()[CVAR_GFX_CAM_TILTING];
    bool bGfxCamTilting = cvarGfxCamTilt.getAsBool();
    ImGui::AlignTextToFramePadding();
    static std::string sHintGfxCamTilt; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintGfxCamTilt, cvarGfxCamTilt);
    ImGui::TextUnformatted("Camera Tilting:");
    ImGui::SameLine();
    if (ImGui::Checkbox("##cbCamTilt", &bGfxCamTilting))
    {
        cvarGfxCamTilt.Set(bGfxCamTilting);
    }

    PGEcfgVariable& cvarGfxCamRoll = m_pPge->getConfigProfiles().getVars()[CVAR_GFX_CAM_ROLLING];
    bool bGfxCamRolling = cvarGfxCamRoll.getAsBool();
    ImGui::AlignTextToFramePadding();
    static std::string sHintGfxCamRoll; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintGfxCamRoll, cvarGfxCamRoll);
    ImGui::TextUnformatted("Camera Rolling:");
    ImGui::SameLine();
    if (ImGui::Checkbox("##cbCamRoll", &bGfxCamRolling))
    {
        cvarGfxCamRoll.Set(bGfxCamRolling);
    }

    PGEcfgVariable& cvarGuiXHairIdentifiesPlayers = m_pPge->getConfigProfiles().getVars()[XHair::szCvarGuiXHairIdentifiesPlayers];
    bool bGuiXHairIdentifiesPlayers = cvarGuiXHairIdentifiesPlayers.getAsBool();
    ImGui::AlignTextToFramePadding();
    static std::string sHintGuiXHairIdentifiesPlayers; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintGuiXHairIdentifiesPlayers, cvarGuiXHairIdentifiesPlayers);
    ImGui::TextUnformatted("XHair Identifies Players:");
    ImGui::SameLine();
    if (ImGui::Checkbox("##cbGuiXHairIdentifiesPlayers", &bGuiXHairIdentifiesPlayers))
    {
        cvarGuiXHairIdentifiesPlayers.Set(bGuiXHairIdentifiesPlayers);
    }

    PGEcfgVariable& cvarGuiMinimapShow = m_pPge->getConfigProfiles().getVars()[Minimap::szCvarGuiMinimapShow];
    bool bGuiMinimapShow = cvarGuiMinimapShow.getAsBool();
    ImGui::AlignTextToFramePadding();
    static std::string sHintGuiMinimapShow; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintGuiMinimapShow, cvarGuiMinimapShow);
    ImGui::TextUnformatted("Show Minimap:");
    ImGui::SameLine();
    if (ImGui::Checkbox("##cbGuiMinimapShow", &bGuiMinimapShow))
    {
        cvarGuiMinimapShow.Set(bGuiMinimapShow);
    }

    PGEcfgVariable& cvarGuiMinimapTransparent = m_pPge->getConfigProfiles().getVars()[Minimap::szCvarGuiMinimapTransparent];
    bool bGuiMinimapTransparent = cvarGuiMinimapTransparent.getAsBool();
    ImGui::AlignTextToFramePadding();
    static std::string sHintGuiMinimapTransparent; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintGuiMinimapTransparent, cvarGuiMinimapTransparent);
    ImGui::TextUnformatted("Minimap Transparency:");
    ImGui::SameLine();
    if (ImGui::Checkbox("##cbGuiMinimapTransparent", &bGuiMinimapTransparent))
    {
        cvarGuiMinimapTransparent.Set(bGuiMinimapTransparent);
    }

    ImGui::BeginGroup();
    {
        PGEcfgVariable& cvarClWpnAutoSwitchWhenPickedUpNewWeapon = m_pPge->getConfigProfiles().getVars()[szCvarClWpnAutoSwitchWhenPickedUpNewWeapon];
        ImGui::AlignTextToFramePadding();
        static std::string sHintClWpnAutoSwitchWhenPickedUpNewWeapon; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintClWpnAutoSwitchWhenPickedUpNewWeapon, cvarClWpnAutoSwitchWhenPickedUpNewWeapon);
        ImGui::TextUnformatted("Pickup-Induced Auto-Switch to NEW Weapon:");
        ImGui::Indent();

        // dont forget there is also 3-param version of RadioButton
        if (ImGui::RadioButton("Always Auto-Switch to New##PickupNewWpn", cvarClWpnAutoSwitchWhenPickedUpNewWeapon.getAsString() == szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchAlways))
        {
            cvarClWpnAutoSwitchWhenPickedUpNewWeapon.Set(szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchAlways);
        }
        if (ImGui::RadioButton("Auto-Switch to New if Better than Current##PickupNewWpn", cvarClWpnAutoSwitchWhenPickedUpNewWeapon.getAsString() == szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchIfBetter))
        {
            cvarClWpnAutoSwitchWhenPickedUpNewWeapon.Set(szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchIfBetter);
        }
        if (ImGui::RadioButton("Auto-Switch to New if Current is Empty##PickupNewWpn", cvarClWpnAutoSwitchWhenPickedUpNewWeapon.getAsString() == szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchIfEmpty))
        {
            cvarClWpnAutoSwitchWhenPickedUpNewWeapon.Set(szCvarClWpnAutoSwitchWhenPickedUpNewWeaponBehaviorValueAutoSwitchIfEmpty);
        }
        ImGui::Unindent();
    }
    ImGui::EndGroup();

    PGEcfgVariable& cvarClWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag =
        m_pPge->getConfigProfiles().getVars()[szCvarClWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag];
    bool bWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag = cvarClWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag.getAsBool();
    ImGui::AlignTextToFramePadding();
    static std::string sHintClWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintClWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag, cvarClWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag);
    ImGui::TextUnformatted("Pickup-Induced Auto-Switch to ANY, if Current is Empty:");
    ImGui::SameLine();
    if (ImGui::Checkbox("##cbWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag", &bWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag))
    {
        cvarClWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag.Set(bWpnAutoSwitchWhenPickedUpAnyAmmoEmptyMag);
    }

    PGEcfgVariable& cvarClWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag =
        m_pPge->getConfigProfiles().getVars()[szCvarClWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag];
    bool bWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag = cvarClWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag.getAsBool();
    ImGui::AlignTextToFramePadding();
    static std::string sHintClWpnAutoReloadWhenSwitchedToEmptyMagNonemptyUnmag; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintClWpnAutoReloadWhenSwitchedToEmptyMagNonemptyUnmag, cvarClWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag);
    ImGui::TextUnformatted("Pickup-/Switch-Induced Auto-Reload:");
    ImGui::SameLine();
    if (ImGui::Checkbox("##cbWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag", &bWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag))
    {
        cvarClWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag.Set(bWpnAutoReloadWhenSwitchedToOrPickedUpAmmoEmptyMagNonemptyUnmag);
    }

    ImGui::BeginGroup();
    {
        PGEcfgVariable& cvarClWpnEmptyMagNonemptyUnmagBehavior = m_pPge->getConfigProfiles().getVars()[szCvarClWpnEmptyMagNonemptyUnmagBehavior];
        ImGui::AlignTextToFramePadding();
        static std::string sHintClWpnEmptyMagNonemptyUnmagBehavior; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintClWpnEmptyMagNonemptyUnmagBehavior, cvarClWpnEmptyMagNonemptyUnmagBehavior);
        ImGui::TextUnformatted("If Weapon is Empty after Firing, BUT HAS Spare Ammo, then:");
        ImGui::Indent();

        // dont forget there is also 3-param version of RadioButton
        if (ImGui::RadioButton("Auto-Reload Current Weapon##WpnEmptyMagNonempty", cvarClWpnEmptyMagNonemptyUnmagBehavior.getAsString() == szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoReload))
        {
            cvarClWpnEmptyMagNonemptyUnmagBehavior.Set(szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoReload);
        }
        if (ImGui::RadioButton("Auto-Switch to Next Best Non-Empty Weapon##WpnEmptyMagNonempty", cvarClWpnEmptyMagNonemptyUnmagBehavior.getAsString() == szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoSwitchToBestLoaded))
        {
            cvarClWpnEmptyMagNonemptyUnmagBehavior.Set(szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoSwitchToBestLoaded);
        }
        if (ImGui::RadioButton("Auto-Switch to Next Best Reloadable Weapon##WpnEmptyMagNonempty", cvarClWpnEmptyMagNonemptyUnmagBehavior.getAsString() == szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoSwitchToBestReloadable))
        {
            cvarClWpnEmptyMagNonemptyUnmagBehavior.Set(szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueAutoSwitchToBestReloadable);
        }
        if (ImGui::RadioButton("Do Nothing##WpnEmptyMagNonempty", cvarClWpnEmptyMagNonemptyUnmagBehavior.getAsString() == szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueNoop))
        {
            cvarClWpnEmptyMagNonemptyUnmagBehavior.Set(szCvarClWpnEmptyMagNonemptyUnmagBehaviorValueNoop);
        }
        ImGui::Unindent();
    }
    ImGui::EndGroup();

    ImGui::BeginGroup();
    {
        PGEcfgVariable& cvarClWpnEmptyMagEmptyUnmagBehavior = m_pPge->getConfigProfiles().getVars()[szCvarClWpnEmptyMagEmptyUnmagBehavior];
        ImGui::AlignTextToFramePadding();
        static std::string sHintClWpnEmptyMagEmptyUnmagBehavior; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintClWpnEmptyMagEmptyUnmagBehavior, cvarClWpnEmptyMagEmptyUnmagBehavior);
        ImGui::TextUnformatted("If Weapon is Empty after Firing, AND has NO Spare Ammo, then:");
        ImGui::Indent();

        // dont forget there is also 3-param version of RadioButton
        if (ImGui::RadioButton("Auto-Switch to Next Best Non-Empty Weapon##WpnEmptyMagEmpty", cvarClWpnEmptyMagEmptyUnmagBehavior.getAsString() == szCvarClWpnEmptyMagEmptyUnmagBehaviorValueAutoSwitchToBestLoaded))
        {
            cvarClWpnEmptyMagEmptyUnmagBehavior.Set(szCvarClWpnEmptyMagEmptyUnmagBehaviorValueAutoSwitchToBestLoaded);
        }
        if (ImGui::RadioButton("Auto-Switch to Next Best Reloadable Weapon##WpnEmptyMagEmpty", cvarClWpnEmptyMagEmptyUnmagBehavior.getAsString() == szCvarClWpnEmptyMagEmptyUnmagBehaviorValueAutoSwitchToBestReloadable))
        {
            cvarClWpnEmptyMagEmptyUnmagBehavior.Set(szCvarClWpnEmptyMagEmptyUnmagBehaviorValueAutoSwitchToBestReloadable);
        }
        if (ImGui::RadioButton("Do Nothing##WpnEmptyMagEmpty", cvarClWpnEmptyMagEmptyUnmagBehavior.getAsString() == szCvarClWpnEmptyMagEmptyUnmagBehaviorValueNoop))
        {
            cvarClWpnEmptyMagEmptyUnmagBehavior.Set(szCvarClWpnEmptyMagEmptyUnmagBehaviorValueNoop);
        }
        ImGui::Unindent();
    }
    ImGui::EndGroup();

    ImGui::Separator();

    if (ImGui::Button("< BACK"))
    {
        // since there is nothing in validate() relevant to this settings page, dont trigger it for now ... later I will move V-Sync
        // validation to there too, then it will make sense to invoke it here!
        //m_pConfig->validate();
        if (!m_pPge->getConfigProfiles().writeConfiguration())
        {
            getConsole().EOLn("ERROR: failed to save current config profile!");
        }
        m_currentMenu = MenuState::Main;
    }

    ImGui::Unindent();
} // drawSettingsMenu

void proofps_dd::GUI::drawWindowForMainMenu()
{
    m_pPge->getPure().getWindow().SetCursorVisible(true);

    // Dear ImGui coordinates are the same as OS desktop/native coordinates which means that operating with ImGui::GetMainViewport() is
    // different than operating with getPure().getCamera().getViewport():
    // - PURE 2D viewport (0,0) is the CENTER, and positive Y goes UPWARDS from CENTER;
    // - Dear ImGui viewport (0,0) is the TOP LEFT, and positive Y goes DOWNWARDS from the TOP.
    const ImGuiViewport* const main_viewport = ImGui::GetMainViewport();
    const float fLogoImgHeight = m_pObjLoadingScreenLogoImg->getSizeVec().getY();
    constexpr float fMenuWndWidth = 500.f; // just put here the widest submenu's required width
    const float fMenuWndHeight = main_viewport->WorkSize.y - fLogoImgHeight;
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkSize.x / 2 - fMenuWndWidth / 2, fLogoImgHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(fMenuWndWidth, fMenuWndHeight), ImGuiCond_FirstUseEver);

    ImGui::Begin("WndMainMenu", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
    {
        switch (m_currentMenu)
        {
        case MenuState::CreateGame:
            drawCreateGameMenu(fMenuWndHeight);
            break;
        case MenuState::JoinGame:
            drawJoinGameMenu(fMenuWndHeight);
            break;
        case MenuState::Settings:
            drawSettingsMenu(fMenuWndHeight);
            break;
        case MenuState::Main:
            drawMainMenu(fMenuWndHeight);
            break;
        default:
            /* MenuState::None or MenuState::Exiting */
            break;
        }

    }
    ImGui::End();
}

/**
 * Primary GUI draw function called back by PURE.
 */
void proofps_dd::GUI::drawDearImGuiCb()
{
    if (m_currentMenu == MenuState::None)
    {
        // we are in-game

        const ImGuiViewport* const main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x, main_viewport->WorkSize.y), ImGuiCond_FirstUseEver);

        // this window should cover the full window client area, otherwise getDearImGui2DposXFromPure2DposX() and other functions might not function properly!
        ImGui::Begin("WndInGame", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse);

        // initialize() created these fonts before configuring this as PURE GUI callback function
        assert(m_pImFontFragTable);
        assert(m_pImFontHudGeneral);
        ImGui::PushFont(m_pImFontHudGeneral);

        drawRespawnTimer();
        updateXHair();
        updateDeathKillEvents();
        updateItemPickupEvents();

        assert(m_pMinimap);  // initialize() created it before configuring this to be the callback for PURE
        m_pMinimap->draw();

        assert(m_pMapPlayers);
        const auto it = m_pMapPlayers->find(m_pNetworking->getMyServerSideConnectionHandle());
        if (it != m_pMapPlayers->end())
        {
            drawCurrentPlayerInfo(it->second);
        }

        ImGui::PopFont();

        drawGameInfoPages();

        ImGui::End();

        return;
    }
    else if (m_currentMenu == MenuState::Exiting)
    {
        return;
    }

    // at this point, we are in the menu

    drawWindowForMainMenu();
} // drawDearImGuiCb()

void proofps_dd::GUI::drawRespawnTimer()
{
    if (!m_bShowRespawnTimer)
    {
        return;
    }

    static constexpr char* szRespawnWaitText = "... Waiting to Respawn ...";
    // if we make this static, it will be wrong upon changing screen resolution so now just let it be like this
    const float fTextPosX = getDearImGui2DposXforWindowCenteredText(szRespawnWaitText);

    assert(m_pPge);
    drawTextShadowed(
        fTextPosX,
        (m_pPge->getPure().getCamera().getViewport().size.height / 2.f) - m_fFontSizePxHudGeneral,
        szRespawnWaitText);

    assert(m_pConfig);
    if (m_pConfig->getPlayerRespawnDelaySeconds() == 0)
    {
        return;
    }

    // client doesn't control respawn, countdown is just an estimation, the recorded die time is when client received
    // the death notification from server, so it is just ROUGHLY exact, but good enough! Server controls the respawn.
    // Client knows the server's configured respawn delay, so we have everything for showing a ROUGH countdown.
    const auto timeDiffMillisecs = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - m_timePlayerDied).count());
    
    const float fRespawnProgress = std::min(1.f, timeDiffMillisecs / (static_cast<float>(m_pConfig->getPlayerRespawnDelaySeconds() * 1000)));
    //const int timeRemainingUntilRespawnSecs =
    //    std::max(0, static_cast<int>(m_pConfig->getPlayerRespawnDelaySeconds()) - timeDiffSeconds);

    constexpr float fProgressBarSizeY = 10.f;
    const float fProgressBarSizeX = ImGui::CalcTextSize(szRespawnWaitText).x;
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2.f - (fProgressBarSizeX / 2.f), ImGui::GetCursorPosY()));

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

    ImGui::ProgressBar(fRespawnProgress, ImVec2(fProgressBarSizeX, fProgressBarSizeY), "");

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    if (!m_sRespawnTimerExtraText.empty())
    {
        drawTextShadowed(
            getDearImGui2DposXforWindowCenteredText(m_sRespawnTimerExtraText),
            ImGui::GetCursorPosY(),
            m_sRespawnTimerExtraText);
    }
    if (!m_sRespawnTimerExtraText2.empty())
    {
        drawTextShadowed(
            getDearImGui2DposXforWindowCenteredText(m_sRespawnTimerExtraText2),
            ImGui::GetCursorPosY(),
            m_sRespawnTimerExtraText2);
    }
}

void proofps_dd::GUI::drawXHairHoverText()
{
    assert(m_pXHair); // only updateXHair() calls this

    if (!m_pXHair->visible() || m_pXHair->getIdText().empty())
    {
        return;
    }

    drawTextShadowed(
        getDearImGui2DposXforCenteredText(
            m_pXHair->getIdText(), getDearImGui2DposXFromPure2DposX(m_pXHair->getObject3D().getPosVec().getX())),
            getDearImGui2DposYFromPure2DposY(m_pXHair->getObject3D().getPosVec().getY()) + m_pXHair->getObject3D().getSizeVec().getY() / 2.f,
            m_pXHair->getIdText());
}

void proofps_dd::GUI::updateXHair()
{
    assert(m_pXHair);  // initialize() created it before configuring drawDearImGuiCb() to be the callback for PURE

    // in the future this function can be moved to XHair class with drawXHairHoverText(), but first drawTextShadowed need to be moved to separate class
    // so that both GUI and XHair classes can utilize it
    m_pXHair->updateVisuals();
    drawXHairHoverText();
}

void proofps_dd::GUI::drawCurrentPlayerInfo(const proofps_dd::Player& player)
{
    assert(m_pPge);
    assert(m_pMinimap);  // initialize() created it before configuring drawDearImGuiCb() to be the callback for PURE

    // I think we should show health and armor only when xhair is also visible, however xhair is not always properly controlled but hidden by loading screen,
    // so I would rather stick to minimap visibility now which is controlled more better
    if (!m_pMinimap->visible())
    {
        return;
    }

    // we start at the bottom of the screen, in reverse order from bottom to top
    const float fStartY = m_pPge->getPure().getCamera().getViewport().size.height - m_fFontSizePxHudGeneral - 10 /* spacing from viewport bottom edge */;
    if (m_pNetworking->isServer())
    {
        drawTextHighlighted(
            10,
            fStartY,
            "Server, User name: " + player.getName() +
            (m_pPge->getConfigProfiles().getVars()["testing"].getAsBool() ? "; Testing Mode" : ""));
    }
    else
    {
        drawTextHighlighted(
            10,
            fStartY,
            "Client, User name: " + player.getName() +
            "; IP: " + player.getIpAddress() +
            (m_pPge->getConfigProfiles().getVars()["testing"].getAsBool() ? "; Testing Mode" : ""));
    }

    const float fYdiffBetweenRows = ImGui::GetCursorPos().y - fStartY; // now we know the vertical distance between each row as Dear ImGui calculated

    const Weapon* wpnCurrent = player.getWeaponManager().getCurrentWeapon();
    if (wpnCurrent)
    {
        const auto itCVarWpnName = wpnCurrent->getVars().find("name");

        if (itCVarWpnName != wpnCurrent->getVars().end())
        {
            drawTextHighlighted(
                10,
                ImGui::GetCursorPos().y - 3 * fYdiffBetweenRows,
                itCVarWpnName->second.getAsString() + ": " +
                std::to_string(wpnCurrent->getMagBulletCount()) + " / " +
                std::to_string(wpnCurrent->getUnmagBulletCount()));
        }
    }
    
    drawTextHighlighted(10, ImGui::GetCursorPos().y - 3 * fYdiffBetweenRows, "Armor: " + std::to_string(player.getArmor()) + " %");
    drawTextHighlighted(10, ImGui::GetCursorPos().y - 2 * fYdiffBetweenRows, "Health: " + std::to_string(player.getHealth()) + " %");
}

void proofps_dd::GUI::updateDeathKillEvents()
{
    // similar to updateXHair(), we are doing like this because text drawing is implemented in GUI, it should be somewhere else, then EventLister could be more independent

    assert(m_pEventsDeathKill);  // initialize() created it before configuring drawDearImGuiCb() to be the callback for PURE
    
    m_pEventsDeathKill->update();

    const float fRightPosXlimit = m_pPge->getPure().getCamera().getViewport().size.width - 10;
    ImGui::SetCursorPosY(50 + m_fFontSizePxHudGeneral); /* FPS is somewhere above with legacy text rendering still, we dont exactly know where */
    for (auto it = m_pEventsDeathKill->getEvents().rbegin(); it != m_pEventsDeathKill->getEvents().rend(); ++it)
    {
        drawTextHighlighted(
            getDearImGui2DposXforRightAdjustedText(it->second, fRightPosXlimit),
            ImGui::GetCursorPos().y,
            it->second);
    }
}

void proofps_dd::GUI::updateItemPickupEvents()
{
    assert(m_pEventsItemPickup);  // initialize() created it before configuring drawDearImGuiCb() to be the callback for PURE

    m_pEventsItemPickup->update();

    const float fRightPosXlimit = m_pPge->getPure().getCamera().getViewport().size.width - 10;
    ImGui::SetCursorPosY( /* should be below m_pEventsDeathKill events */ m_pEventsDeathKill->getEventCountLimit() * (m_fFontSizePxHudGeneral + 3) + 20);
    for (auto it = m_pEventsItemPickup->getEvents().rbegin(); it != m_pEventsItemPickup->getEvents().rend(); ++it)
    {
        drawTextHighlighted(
            getDearImGui2DposXforRightAdjustedText(it->second, fRightPosXlimit),
            ImGui::GetCursorPos().y,
            it->second);
    }
}

void proofps_dd::GUI::calculatePlayerNameColWidthAndTableWidthPixels(
    float& fTableWidthPixels,
    float& fPlayerNameColWidthPixels,
    float fPlayerNameColReqWidthPixels /* can be greater than 0 for an initial required width in pixels */,
    const float& fTableColIndentPixels,
    const float& fColsTotalWidthAfterPlayerNameCol)
{
    assert(m_pGameMode);

    // calculating the max required width for player names, I do this to limit the space for column 0 if I can, to make sure table is not too wide.
    // Unlike with other columns having their width determined by their header text, here we allow Player Name column to be flexible, by calculating
    // a good enough width based on player names, but obviously we maximize it so table will fit into 90% width of viewport width.
    for (const auto& player : m_pGameMode->getFragTable())
    {
        const float fPlayerNameReqWidthPixels = ImGui::CalcTextSize(player.m_sName.c_str()).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels;
        if (fPlayerNameReqWidthPixels > fPlayerNameColReqWidthPixels)
        {
            fPlayerNameColReqWidthPixels = fPlayerNameReqWidthPixels;
        }
    }

    const float fTableMaxWidthPixels = ImGui::GetWindowSize().x * 0.9f;
    const float fAvailWidthForPlayerNameColPixels = fTableMaxWidthPixels - fColsTotalWidthAfterPlayerNameCol;
    if (fAvailWidthForPlayerNameColPixels > fPlayerNameColReqWidthPixels)
    {
        fPlayerNameColWidthPixels = fPlayerNameColReqWidthPixels + 20.f /* somehow I need this extra few pixels added */;
        fTableWidthPixels = fPlayerNameColWidthPixels + fColsTotalWidthAfterPlayerNameCol;
    }
    else
    {
        fPlayerNameColWidthPixels = fAvailWidthForPlayerNameColPixels;
        fTableWidthPixels = ImGui::GetWindowSize().x * 0.9f;
    }
}

void proofps_dd::GUI::drawGameObjectivesServer(const std::string& sTableCaption, const float& fStartPosY)
{
    assert(m_pNetworking && m_pNetworking->isServer());
    assert(m_pGameMode);
    assert(m_pPge);
    assert(m_gameInfoPageCurrent == GameInfoPage::FragTable);

    static constexpr auto vecHeaderLabels = PFL::std_array_of<const char*>(
        "Player Name",
        "Frags",
        "Deaths",
        "Ping",
        "Qlty NE/FE",
        "Speed\nTx/Rx (Bps)",
        "Pending\nRl/URl (Bps)",
        "UnAck'd\n(Bps)",
        "tIntQ\n(us)"
    );

    static constexpr float fTableColIndentPixels = 4.f;
    static const auto vecColumnWidthsPixels = PFL::std_array_of<float>(
        0.f /* col 0 width will be calculated later as fPlayerNameColWidthPixels */,
        ImGui::CalcTextSize(vecHeaderLabels[1]).x + 2 * ImGui::GetStyle().ItemSpacing.x /* style item spacing is used as table column padding */ + fTableColIndentPixels,
        ImGui::CalcTextSize(vecHeaderLabels[2]).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels,
        ImGui::CalcTextSize(vecHeaderLabels[3]).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels,
        ImGui::CalcTextSize(vecHeaderLabels[4]).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels,
        ImGui::CalcTextSize(vecHeaderLabels[5]).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels,
        ImGui::CalcTextSize(vecHeaderLabels[6]).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels,
        ImGui::CalcTextSize(vecHeaderLabels[7]).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels,
        ImGui::CalcTextSize(vecHeaderLabels[8]).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels
    );

    assert(vecHeaderLabels.size() == vecColumnWidthsPixels.size());

    static const float fColsTotalWidthAfterPlayerNameCol = std::accumulate(vecColumnWidthsPixels.begin(), vecColumnWidthsPixels.end(), 0.f);

    static const auto imClrTableRowHighlightedU32 = ImGui::GetColorU32(imClrTableRowHighlightedVec4);

    constexpr ImGuiTableFlags tblFlags =
        ImGuiTableFlags_RowBg /* |
         ImGuiTableFlags_Borders used it as cell padding is NOT working without borders flag! Then I changed to ImGuiTableColumnFlags_IndentEnable instead of cell padding! */ |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_SizingStretchProp;

    /*
    * Copied this from imgui.h:
    * // - The DEFAULT sizing policies are:
    * //    - Default to ImGuiTableFlags_SizingFixedFit    if ScrollX is on, or if host window has ImGuiWindowFlags_AlwaysAutoResize.
    * //    - Default to ImGuiTableFlags_SizingStretchSame if ScrollX is off.
    * // - When ScrollX is off:
    * //    - Table defaults to ImGuiTableFlags_SizingStretchSame -> all Columns defaults to ImGuiTableColumnFlags_WidthStretch with same weight.
    * //    - Columns sizing policy allowed: Stretch (default), Fixed/Auto.
    * //    - Fixed Columns (if any) will generally obtain their requested width (unless the table cannot fit them all).
    * //    - Stretch Columns will share the remaining width according to their respective weight.
    * //    - Mixed Fixed/Stretch columns is possible but has various side-effects on resizing behaviors.
    * //      The typical use of mixing sizing policies is: any number of LEADING Fixed columns, followed by one or two TRAILING Stretch columns.
    * //      (this is because the visible order of columns have subtle but necessary effects on how they react to manual resizing).
    * 
    * Based on above info, my table defaults to ImGuiTableFlags_SizingStretchSame since I dont use either ScrollX for the table or ImGuiWindowFlags_AlwaysAutoResize window.
    * But I can still change it to ImGuiTableFlags_SizingFixedFit or ImGuiTableFlags_SizingStretchProp if I want to.
    * Thus columns default to ImGuiTableColumnFlags_WidthStretch but I can change it to ImGuiTableColumnFlags_WidthFixed, I can even mix them.
    * However, for me the leading column (player name) should be stretched to content and all remaining columns should be fixed.
    * But I cannot properly stretch by column since I go row-by-row, not by column-to-column.
    * So anyway, since I'm calculating table width and height below in pixels, I'm using the column width calculated above based on header text,
    * and then calculate col0 width by subtracting their total width from table width.
    * Note that I'm using ImGuiTableColumnFlags_WidthStretch for all columns because I want the calculated widths to be treated as weights and not
    * strict widths in pixels, so ImGui will find out the exact pixels. Also, with ImGuiTableColumnFlags_WidthFixed the widths were wrong for some
    * unknown reason, but I think this weighted config will be just fine.
    */

    float fTableWidthPixels;
    float fPlayerNameColWidthPixels;
    float fPlayerNameColReqWidthPixels = ImGui::CalcTextSize(vecHeaderLabels[0]).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels;
    calculatePlayerNameColWidthAndTableWidthPixels(
        fTableWidthPixels,
        fPlayerNameColWidthPixels,
        fPlayerNameColReqWidthPixels,
        fTableColIndentPixels,
        fColsTotalWidthAfterPlayerNameCol);

    const float fTableStartPosX = std::roundf((ImGui::GetWindowSize().x - fTableWidthPixels) / 2.f);
    
    const float fTableCaptionWidthPixels = ImGui::CalcTextSize(sTableCaption.c_str()).x;
    ImGui::SetCursorPos(
        ImVec2(
            (fTableWidthPixels < fTableCaptionWidthPixels) ?
                std::roundf((ImGui::GetWindowSize().x - fTableCaptionWidthPixels) / 2.f) : 
                fTableStartPosX, 
            fStartPosY));
    drawTextHighlighted(ImGui::GetCursorPosX(), fStartPosY, sTableCaption);

    ImGui::SetCursorPos(ImVec2(fTableStartPosX, ImGui::GetCursorPosY()));
    const float fTableHeightPixels = ImGui::GetWindowSize().y * 0.8f; // fixed height, but since we are not drawing frames, it will look as variable height
    
    // not changing padding anymore since it requires border flags which I dont use now
    //ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.f /* horizontal padding in pixels*/, 2.f /* vertical padding in pixels */));
    
    // not sure about the performance impact of table rendering but Dear ImGui's Table API is so flexible and sophisticated, I decided to use it here!
    if (ImGui::BeginTable("tbl_frag_sv", static_cast<int>(vecHeaderLabels.size()), tblFlags, ImVec2(fTableWidthPixels, fTableHeightPixels)))
    {
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::Indent(fTableColIndentPixels); // applies to all cell contents; set only once, unindent at the end; requires ImGuiTableColumnFlags_IndentEnable
        size_t iHdrCol = 0;
        for (const auto& hdr : vecHeaderLabels)
        {
            // not changing padding anymore since it requires border flags which I dont use now
            //if (iHdrCol < 5)
            //{
            //    // 1-line header texts should be roughly vertically centered
            //    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.f /* horizontal padding in pixels*/, 10.f /* vertical padding in pixels */));
            //}
            ImGui::TableSetupColumn(
                hdr,
                ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_IndentEnable,
                /* due to ImGuiTableFlags_SizingStretchProp, these are not strict pixels but weights */
                iHdrCol == 0 ? fPlayerNameColWidthPixels : vecColumnWidthsPixels[iHdrCol]);
            
            // not changing padding anymore since it requires border flags which I dont use now
            //if (iHdrCol < 5)
            //{
            //    ImGui::PopStyleVar();
            //}

            iHdrCol++;
        }

        // ImGui calculates multi-line header text height properly so we dont need to set custom row height.
        // TODO: unfortunately, I cannot use my centering function for header cells the same way as I can for ordinary cells, even when I tried
        // to emit header cells manually using TableHeader(). This is why all text in header cells are not adjusted.
        // Only WA I can think about is if I simply dont use the header feature, instead I'm manually manipulating properties for row 0 in the loop.
        ImGui::TableHeadersRow();
        for (int iReplicateRowsForExperimenting = 0; iReplicateRowsForExperimenting < 1; iReplicateRowsForExperimenting++)
        {
            for (const auto& player : m_pGameMode->getFragTable())
            {
                ImGui::TableNextRow();
                if (m_pNetworking->isMyConnection(player.m_connHandle))
                {
                    // applies only to the current row, no need to reset
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, imClrTableRowHighlightedU32);
                }

                const int nColumnCount = (player.m_connHandle == pge_network::ServerConnHandle) ?
                    3 : static_cast<int>(vecHeaderLabels.size());
                for (int iCol = 0; iCol < nColumnCount; iCol++)
                {
                    ImGui::TableSetColumnIndex(iCol);
                    switch (iCol)
                    {
                    case 0:
                        ImGuiTextTableCurrentCellShortenedFit(
                            player.m_sName
                            /*"WWWWWWWWW0WWWWWWWWW0WWWWWWWWW0WWWWWWWWW0WWWW"*/
                            /*"megszentsegtelenithetetlensegeskedeseitekert"*/,
                            3);
                        break;
                    case 1:
                        ImGuiTextTableCurrentCellRightAdjusted(std::to_string(player.m_nFrags) /*"999"*/);
                        break;
                    case 2:
                        ImGuiTextTableCurrentCellRightAdjusted(std::to_string(player.m_nDeaths) /*"999"*/);
                        break;
                    case 3:
                        ImGuiTextTableCurrentCellRightAdjusted(
                            std::to_string(m_pPge->getNetwork().getServer().getPing(player.m_connHandle, true)) /*"999"*/);
                        break;
                    case 4:
                    {
                        std::stringstream ssQuality;
                        ssQuality << std::fixed << std::setprecision(2) << m_pPge->getNetwork().getServer().getQualityLocal(player.m_connHandle, false) <<
                            "/" << m_pPge->getNetwork().getServer().getQualityRemote(player.m_connHandle, false);
                        ImGuiTextTableCurrentCellRightAdjusted(ssQuality.str().c_str() /*"0.90/-0.90"*/);
                    }
                    break;
                    case 5:
                        ImGuiTextTableCurrentCellRightAdjusted(
                            (std::to_string(std::lround(m_pPge->getNetwork().getServer().getTxByteRate(player.m_connHandle, false))) + "/" +
                                std::to_string(std::lround(m_pPge->getNetwork().getServer().getRxByteRate(player.m_connHandle, false)))).c_str()
                            /*"999/9999"*/);
                        break;
                    case 6:
                        ImGuiTextTableCurrentCellRightAdjusted(
                            (std::to_string(m_pPge->getNetwork().getServer().getPendingReliableBytes(player.m_connHandle, false)) + "/" +
                                std::to_string(m_pPge->getNetwork().getServer().getPendingUnreliableBytes(player.m_connHandle, false))).c_str()
                            /*"999/9999"*/);
                        break;
                    case 7:
                        ImGuiTextTableCurrentCellRightAdjusted(
                            std::to_string(m_pPge->getNetwork().getServer().getSentButUnAckedReliableBytes(player.m_connHandle, false)).c_str()
                            /*"999"*/);
                        break;
                    case 8:
                        ImGuiTextTableCurrentCellRightAdjusted(
                            std::to_string(m_pPge->getNetwork().getServer().getInternalQueueTimeUSecs(player.m_connHandle, false)).c_str()
                            /*"9999"*/);
                        break;
                    default:
                        assert(false); // crash in debug
                    }
                } // end for iCol
            } // end for players
        } // end for iReplicateRowsForExperimenting
        ImGui::Unindent(fTableColIndentPixels);
        ImGui::EndTable();
    } // end BeginTable
    // not changing padding anymore since it requires border flags which I dont use now
    //ImGui::PopStyleVar();
}  // drawGameObjectivesServer()

void proofps_dd::GUI::drawGameObjectivesClient(const std::string& sTableCaption, const float& fStartPosY)
{
    assert(m_pNetworking && !m_pNetworking->isServer());
    assert(m_pGameMode);
    assert(m_pPge);
    assert(m_gameInfoPageCurrent == GameInfoPage::FragTable);

    static constexpr auto vecHeaderLabels = PFL::std_array_of<const char*>(
        "Player Name",
        "Frags",
        "Deaths"
    );

    static constexpr float fTableColIndentPixels = 4.f;
    static const auto vecColumnWidthsPixels = PFL::std_array_of<float>(
        0.f /* col 0 width will be calculated later as fPlayerNameColWidthPixels */,
        ImGui::CalcTextSize(vecHeaderLabels[1]).x + 2 * ImGui::GetStyle().ItemSpacing.x /* style item spacing is used as table column padding */ + fTableColIndentPixels,
        ImGui::CalcTextSize(vecHeaderLabels[2]).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels
    );

    assert(vecHeaderLabels.size() == vecColumnWidthsPixels.size());

    static const float fColsTotalWidthAfterPlayerNameCol = std::accumulate(vecColumnWidthsPixels.begin(), vecColumnWidthsPixels.end(), 0.f);

    static const auto imClrTableRowHighlightedU32 = ImGui::GetColorU32(imClrTableRowHighlightedVec4);

    constexpr ImGuiTableFlags tblFlags =
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_SizingStretchProp;

    float fTableWidthPixels;
    float fPlayerNameColWidthPixels;
    float fPlayerNameColReqWidthPixels = ImGui::CalcTextSize(vecHeaderLabels[0]).x + 2 * ImGui::GetStyle().ItemSpacing.x + fTableColIndentPixels;
    calculatePlayerNameColWidthAndTableWidthPixels(
        fTableWidthPixels,
        fPlayerNameColWidthPixels,
        fPlayerNameColReqWidthPixels,
        fTableColIndentPixels,
        fColsTotalWidthAfterPlayerNameCol);

    const float fTableStartPosX = std::roundf((ImGui::GetWindowSize().x - fTableWidthPixels) / 2.f);

    const float fTableCaptionWidthPixels = ImGui::CalcTextSize(sTableCaption.c_str()).x;
    ImGui::SetCursorPos(
        ImVec2(
            (fTableWidthPixels < fTableCaptionWidthPixels) ?
            std::roundf((ImGui::GetWindowSize().x - fTableCaptionWidthPixels) / 2.f) :
            fTableStartPosX,
            fStartPosY));
    drawTextHighlighted(ImGui::GetCursorPosX(), fStartPosY, sTableCaption);

    ImGui::SetCursorPos(ImVec2(fTableStartPosX, ImGui::GetCursorPosY()));
    const float fTableHeightPixels = ImGui::GetWindowSize().y * 0.8f; // fixed height, but since we are not drawing frames, it will look as variable height

    // not sure about the performance impact of table rendering but Dear ImGui's Table API is so flexible and sophisticated, I decided to use it here!
    if (ImGui::BeginTable("tbl_frag_cl", static_cast<int>(vecHeaderLabels.size()), tblFlags, ImVec2(fTableWidthPixels, fTableHeightPixels)))
    {
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::Indent(fTableColIndentPixels); // applies to all cell contents; set only once, unindent at the end; requires ImGuiTableColumnFlags_IndentEnable
        size_t iHdrCol = 0;
        for (const auto& hdr : vecHeaderLabels)
        {
            ImGui::TableSetupColumn(
                hdr,
                ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_IndentEnable,
                /* due to ImGuiTableFlags_SizingStretchProp, these are not strict pixels but weights */
                iHdrCol == 0 ? fPlayerNameColWidthPixels : vecColumnWidthsPixels[iHdrCol]);

            iHdrCol++;
        }

        // ImGui calculates multi-line header text height properly so we dont need to set custom row height.
        ImGui::TableHeadersRow();
        for (int iReplicateRowsForExperimenting = 0; iReplicateRowsForExperimenting < 1; iReplicateRowsForExperimenting++)
        {
            for (const auto& player : m_pGameMode->getFragTable())
            {
                ImGui::TableNextRow();
                if (m_pNetworking->isMyConnection(player.m_connHandle))
                {
                    // applies only to the current row, no need to reset
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, imClrTableRowHighlightedU32);
                }

                const int nColumnCount = (player.m_connHandle == pge_network::ServerConnHandle) ?
                    3 : static_cast<int>(vecHeaderLabels.size());
                for (int iCol = 0; iCol < nColumnCount; iCol++)
                {
                    ImGui::TableSetColumnIndex(iCol);
                    switch (iCol)
                    {
                    case 0:
                        ImGuiTextTableCurrentCellShortenedFit(
                            player.m_sName
                            /*"WWWWWWWWW0WWWWWWWWW0WWWWWWWWW0WWWWWWWWW0WWWW"*/
                            /*"megszentsegtelenithetetlensegeskedeseitekert"*/,
                            3);
                        break;
                    case 1:
                        ImGuiTextTableCurrentCellRightAdjusted(std::to_string(player.m_nFrags) /*"999"*/);
                        break;
                    case 2:
                        ImGuiTextTableCurrentCellRightAdjusted(std::to_string(player.m_nDeaths) /*"999"*/);
                        break;
                    default:
                        assert(false); // crash in debug
                    }
                } // end for iCol
            } // end for players
        } // end for iReplicateRowsForExperimenting
        ImGui::Unindent(fTableColIndentPixels);
        ImGui::EndTable();
    } // end BeginTable
}  // drawGameObjectivesClient()

void proofps_dd::GUI::drawGameObjectives()
{
    assert(m_gameInfoPageCurrent == GameInfoPage::FragTable);

    assert(m_pNetworking);
    assert(m_pGameMode);
    assert(m_pPge);
    assert(m_pMinimap);

    proofps_dd::DeathMatchMode* pDeathMatchMode = dynamic_cast<proofps_dd::DeathMatchMode*>(m_pGameMode);
    if (!pDeathMatchMode)
    {
        getConsole().EOLn("ERROR: pDeathMatchMode null!");
        return;
    }

    std::string sTableHeaderText;
    if (m_pGameMode->isGameWon())
    {
        sTableHeaderText = "Game Ended! Waiting for restart ...";
    }
    else
    {
        sTableHeaderText = "DeathMatch";
        if (pDeathMatchMode->getFragLimit() > 0)
        {
            sTableHeaderText += " | Frag Limit: " + std::to_string(pDeathMatchMode->getFragLimit());
        }
        if (pDeathMatchMode->getTimeLimitSecs() > 0)
        {
            sTableHeaderText += " | Time Limit: " + std::to_string(pDeathMatchMode->getTimeLimitSecs()) +
                " s, Remaining: " + std::to_string(pDeathMatchMode->getTimeRemainingMillisecs() / 1000) + " s";
        }
    }

    if (m_pNetworking->isServer())
    {
        drawGameObjectivesServer(sTableHeaderText, std::min(72.f, m_pMinimap->getMinimapSizeInPixels().y) + 20.f);
    }
    else
    {
        drawGameObjectivesClient(sTableHeaderText, std::min(72.f, m_pMinimap->getMinimapSizeInPixels().y) + 20.f);
    }
} // drawGameObjectives()

void proofps_dd::GUI::drawClientConnectionDebugInfo(float fThisRowY)
{
    assert(m_gameInfoPageCurrent == GameInfoPage::ServerConfig);
    assert(m_pNetworking && !m_pNetworking->isServer());
    assert(m_pPge);

    drawTextHighlighted(fGameInfoPagesStartX, fThisRowY, "Client Live Network Data");

    fThisRowY += 2 * m_fFontSizePxHudGeneral;

    static constexpr float fIndentX = 20.f;

    drawTextHighlighted(fGameInfoPagesStartX + fIndentX, fThisRowY, "Ping: " + std::to_string(m_pPge->getNetwork().getClient().getPing(true)) + " ms");

    fThisRowY += m_fFontSizePxHudGeneral;
    std::stringstream ssQuality;
    ssQuality << "Quality: near: " << std::fixed << std::setprecision(2) << m_pPge->getNetwork().getClient().getQualityLocal(false) <<
        "; far: " << m_pPge->getNetwork().getClient().getQualityRemote(false);
    drawTextHighlighted(fGameInfoPagesStartX + fIndentX, fThisRowY, ssQuality.str());

    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX,
        fThisRowY,
        "Tx Speed: " + std::to_string(std::lround(m_pPge->getNetwork().getClient().getTxByteRate(false))) +
        " Bps; Rx Speed: " + std::to_string(std::lround(m_pPge->getNetwork().getClient().getRxByteRate(false))) + " Bps");

    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX,
        fThisRowY,
        "Pending Bytes: Reliable: " + std::to_string(m_pPge->getNetwork().getClient().getPendingReliableBytes(false)) +
        "; Unreliable: " + std::to_string(m_pPge->getNetwork().getClient().getPendingUnreliableBytes(false)));

    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX,
        fThisRowY,
        "UnAck'd Bytes: " + std::to_string(m_pPge->getNetwork().getClient().getSentButUnAckedReliableBytes(false)));

    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX,
        fThisRowY,
        "Internal Queue Time: " + std::to_string(m_pPge->getNetwork().getClient().getInternalQueueTimeUSecs(false)) + " us");
}

void proofps_dd::GUI::drawGameServerConfig()
{
    assert(m_gameInfoPageCurrent == GameInfoPage::ServerConfig);
    assert(m_pConfig);
    assert(m_pNetworking);

    float fThisRowY = m_pMinimap->getMinimapSizeInPixels().y + 20.f;
    drawTextHighlighted(fGameInfoPagesStartX, fThisRowY, "Server Config");

    static constexpr float fIndentX = 20.f;

    if (!m_pNetworking->isServer())
    {
        fThisRowY += 2 * m_fFontSizePxHudGeneral;
        drawTextHighlighted(fGameInfoPagesStartX + fIndentX, fThisRowY, std::string("Received: ") + (m_pConfig->isServerInfoReceived() ? "YES" : "NO"));
    }

    fThisRowY += 2 * m_fFontSizePxHudGeneral;

    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX, fThisRowY,
        std::string("Max Framerate: ") + std::to_string(m_pConfig->getServerInfo().m_nMaxFps) + " FPS");
    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX, fThisRowY,
        std::string("Tickrate: ") + std::to_string(m_pConfig->getServerInfo().m_nTickrate) + " Hz");
    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX, fThisRowY,
        std::string("Min Physics Rate: ") + std::to_string(m_pConfig->getServerInfo().m_nPhysicsRateMin) + " Hz");
    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX, fThisRowY,
        std::string("Client Update Rate: ") + std::to_string(m_pConfig->getServerInfo().m_nClientUpdateRate) + " Hz");
    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX, fThisRowY,
        std::string("Game Mode Type: ") + std::to_string(static_cast<int>(m_pConfig->getServerInfo().m_iGameModeType)));
    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX, fThisRowY,
        std::string("Frag Limit: ") + std::to_string(m_pConfig->getServerInfo().m_nFragLimit));
    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX, fThisRowY,
        std::string("Time Limit: ") + std::to_string(m_pConfig->getServerInfo().m_nTimeLimitSecs) + " s");
    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX, fThisRowY,
        std::string("Fall Damage Multiplier: ") + std::to_string(m_pConfig->getServerInfo().m_nFallDamageMultiplier) + "x");
    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX, fThisRowY,
        std::string("Respawn Time: ") + std::to_string(m_pConfig->getServerInfo().m_nRespawnTimeSecs) + " s");
    fThisRowY += m_fFontSizePxHudGeneral;
    drawTextHighlighted(
        fGameInfoPagesStartX + fIndentX, fThisRowY,
        std::string("Respawn Invulnerability Time: ") + std::to_string(m_pConfig->getServerInfo().m_nRespawnInvulnerabilityTimeSecs) + " s");
    
    if (!m_pNetworking->isServer())
    {
        fThisRowY += 2 * m_fFontSizePxHudGeneral;
        drawClientConnectionDebugInfo(fThisRowY);
    }
}

void proofps_dd::GUI::drawGameInfoPages()
{
    switch (m_gameInfoPageCurrent)
    {
    case proofps_dd::GUI::GameInfoPage::FragTable:
        ImGui::PushFont(m_pImFontFragTable);
        drawGameObjectives();
        ImGui::PopFont();
        break;
    case proofps_dd::GUI::GameInfoPage::ServerConfig:
        ImGui::PushFont(m_pImFontHudGeneral);
        drawGameServerConfig();
        ImGui::PopFont();
        break;
    case proofps_dd::GUI::GameInfoPage::None:
        // fall-through
    case proofps_dd::GUI::GameInfoPage::COUNT:
        // fall-through
    default:
        break;
    }
}

/**
* Converts the given X position specified in PURE 2D coordinate system to an X position in ImGui's 2D coordinate system.
* 
* Dear ImGui coordinates are the same as OS desktop/native coordinates which means that operating with ImGui::GetMainViewport() is
* different than operating with getPure().getCamera().getViewport():
* - PURE 2D viewport (0,0) is the CENTER, and positive Y goes UPWARDS from CENTER;
* - Dear ImGui viewport (0,0) is the TOP LEFT, and positive Y goes DOWNWARDS from the TOP.
* 
* @param fPureX The input X position in PURE 2D coordinate system.
* 
* @return The X position in ImGui's 2D coordinate system equivalent to the input PURE 2D X position.
*/
float proofps_dd::GUI::getDearImGui2DposXFromPure2DposX(const float& fPureX)
{
    // considering ImGui::GetWindowSize() covering the full client area of the window
    return fPureX + ImGui::GetWindowSize().x * 0.5f;
}


/**
* Converts the given Y position specified in PURE 2D coordinate system to an Y position in ImGui's 2D coordinate system.
*
* Dear ImGui coordinates are the same as OS desktop/native coordinates which means that operating with ImGui::GetMainViewport() is
* different than operating with getPure().getCamera().getViewport():
* - PURE 2D viewport (0,0) is the CENTER, and positive Y goes UPWARDS from CENTER;
* - Dear ImGui viewport (0,0) is the TOP LEFT, and positive Y goes DOWNWARDS from the TOP.
*
* @param fPureY The input Y position in PURE 2D coordinate system.
*
* @return The Y position in ImGui's 2D coordinate system equivalent to the input PURE 2D Y position.
*/
float proofps_dd::GUI::getDearImGui2DposYFromPure2DposY(const float& fPureY)
{
    // considering ImGui::GetWindowSize() covering the full client area of the window
    return ImGui::GetWindowSize().y * 0.5f - fPureY;
}

float proofps_dd::GUI::getDearImGui2DposXforCenteredText(const std::string& text, const float& fImGuiX)
{
    return fImGuiX - ImGui::CalcTextSize(text.c_str()).x * 0.5f;
}

float proofps_dd::GUI::getDearImGui2DposXforRightAdjustedText(const std::string& text, const float& fImGuiX)
{
    return fImGuiX - ImGui::CalcTextSize(text.c_str()).x;
}

float proofps_dd::GUI::getDearImGui2DposXforTableCurrentCellCenteredText(const std::string& text)
{
    const float fPosX = ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(text.c_str()).x
        - ImGui::GetScrollX() /* amount of scrolling */ - /* 2 * */ ImGui::GetStyle().ItemSpacing.x) / 2.f;

    return fPosX > ImGui::GetCursorPosX() ? fPosX : ImGui::GetCursorPosX();
}

float proofps_dd::GUI::getDearImGui2DposXforTableCurrentCellRightAdjustedText(const std::string& text)
{
    const float fPosX = ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text.c_str()).x
        - ImGui::GetScrollX() /* amount of scrolling */ - /* 2 * */ ImGui::GetStyle().ItemSpacing.x;

    return fPosX > ImGui::GetCursorPosX() ? fPosX : ImGui::GetCursorPosX();
}

float proofps_dd::GUI::getDearImGui2DposXforWindowCenteredText(const std::string& text)
{
    return getDearImGui2DposXforCenteredText(text, ImGui::GetWindowSize().x * 0.5f);
}

void proofps_dd::GUI::drawText(const float& fImGuiX, const float& fImGuiY, const std::string& text)
{
    ImGui::SetCursorPos(ImVec2(fImGuiX, fImGuiY));
    ImGui::TextUnformatted(text.c_str());
}

void proofps_dd::GUI::drawTextShadowed(const float& fImGuiX, const float& fImGuiY, const std::string& text)
{
    ImGui::SetCursorPos(ImVec2(fImGuiX + 1, fImGuiY + 1));
    // instead of TextColored(), I'm changing StyleColor + call TextUnformatted() because I dont use format string so this is faster 
    //ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), "%s", text.c_str());
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopStyleColor();
    drawText(fImGuiX, fImGuiY, text);
}

void proofps_dd::GUI::drawTextHighlighted(const float& fImGuiX, const float& fImGuiY, const std::string& text)
{
    // first we draw an invisible text so we can use its itemrect when drawing our bg rect
    ImGui::SetCursorPos(ImVec2(fImGuiX, fImGuiY));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopStyleColor();

    static const auto imClrTableHeaderBgU32 = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_TableHeaderBg]);
    
    ImDrawList* const dl = ImGui::GetWindowDrawList();
    assert(dl);
    dl->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), imClrTableHeaderBgU32);

    ImGui::SetCursorPos(ImVec2(fImGuiX, fImGuiY));
    ImGui::TextUnformatted(text.c_str());
}

/**
* Renders the given text in the current cell, with automatic shortening if cell width is not enough for the whole text.
* I dont know why Dear ImGui is not always automatically shortening too long text in cells with "..." so I made this function.
* 
* However, this function is very useful especially when shortening player names, since in case of colliding player names, server
* appends a 3-digit unique number to the end of each player name, and we can see those numbers even if name is too long, if we set
* nAppendLastNChars to 3, to be able to distinguish players with too long names!
* 
* @param text              The text to be rendered in current cell.
* @param nAppendLastNChars How many last characters of 'text' should be rendered in the cell after "..." if we are shortening. 
*/
void proofps_dd::GUI::ImGuiTextTableCurrentCellShortenedFit(const std::string& text, size_t nAppendLastNChars)
{
    if (text.empty())
    {
        return;
    }

    const auto fFullTextRequiredWidth = ImGui::CalcTextSize(text.c_str()).x;
    if (fFullTextRequiredWidth == 0.f)
    {
        assert(false); // should never happen since text is not empty
        return;
    }
    
    const auto fAvailWidthInCell = ImGui::GetContentRegionAvail().x;
    const int nFirstNCharsFitInCell = static_cast<int>((std::floor(std::min(1.f, fAvailWidthInCell / fFullTextRequiredWidth) * text.length())));
    if (nFirstNCharsFitInCell <= 0)
    {
        // can happen if column width is too small
        ImGui::TextUnformatted("#");  // even if this '#' does not fit, it should be partially rendered!
        return;
    }

    if (nFirstNCharsFitInCell == static_cast<int>(text.length()))
    {
        ImGui::TextUnformatted(text.c_str());
    }
    else
    {
        if (text.length() < nAppendLastNChars)
        {
            ImGui::TextUnformatted("#");  // even if this '#' does not fit, it should be partially rendered!
            return;
        }

        const std::string sShortenedTextDottedEnd = std::string("...") + text.substr(text.length() - nAppendLastNChars);
        const auto fShortenedDottedTextEndRequiredWidth = ImGui::CalcTextSize(sShortenedTextDottedEnd.c_str()).x;
        if (fShortenedDottedTextEndRequiredWidth == 0.f)
        {
            assert(false); // should never happen since text is not empty
            return;
        }
        if (fShortenedDottedTextEndRequiredWidth >= fFullTextRequiredWidth)
        {
            // might happen if text end is something like "   ..." or "      " (only spaces)
            ImGui::TextUnformatted(text.c_str());
            return;
        }
        if (fAvailWidthInCell < fShortenedDottedTextEndRequiredWidth)
        {
            // can happen if column width is too small
            ImGui::TextUnformatted("###");  // even if this '###' does not fit, it should be partially rendered!
            return;
        }

        const int nShortenedFirstNCharsFitInCell = static_cast<int>(
            (std::floor(
                std::min(1.f, (fAvailWidthInCell - fShortenedDottedTextEndRequiredWidth) / (fFullTextRequiredWidth)) * text.length())
            ));
        const std::string sShortened = text.substr(0, nShortenedFirstNCharsFitInCell) + sShortenedTextDottedEnd;
        ImGui::TextUnformatted(sShortened.c_str());
    }
} // ImGuiTextTableCurrentCellShortenedFit()

void proofps_dd::GUI::ImGuiTextTableCurrentCellCentered(const std::string& text)
{
    ImGui::SetCursorPosX(getDearImGui2DposXforTableCurrentCellCenteredText(text));
    ImGui::TextUnformatted(text.c_str());
}

void proofps_dd::GUI::ImGuiTextTableCurrentCellRightAdjusted(const std::string& text)
{
    ImGui::SetCursorPosX(getDearImGui2DposXforTableCurrentCellRightAdjustedText(text));
    ImGui::TextUnformatted(text.c_str());
}

proofps_dd::GUI::GUI()
{

}

proofps_dd::GUI::~GUI()
{
    shutdown();
}
