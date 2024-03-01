/*
    ###################################################################################
    GUI.cpp
    GUI for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "GUI.h"

#include <cassert>

// PGE has, but here in application we dont have imconfig.h thus we should not try including it!
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include "imgui.h"

#include "Consts.h"
#include "Maps.h"
#include "PRooFPS-dd-packet.h"

// ############################### PUBLIC ################################


proofps_dd::GUI& proofps_dd::GUI::getGuiInstance(
    PGE& pge,
    proofps_dd::Config& config,
    proofps_dd::Maps& maps,
    proofps_dd::Networking& networking)
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
    resetMenuState(false);

    m_pPge->getPure().getUImanager().setDefaultFontSizeLegacy(20);
    m_pPge->getPure().getUImanager().setGuiDrawCallback(drawMainMenuCb);

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
    //m_sAvailableMapsListForMapcycleListBox.clear();
    // we force-create a string from empty " " and append it, otherwise the extra NULL char wont be appended.
    // We are force-inserting the extra NULL characters into the string because this will be actually handled by a multi-element array by Dear ImGUI
    m_sAvailableMapsListForForceSelectComboBox += std::string(" ") + '\0'; // this first elem represents the not-selected map
    for (const auto& sMapName : m_pMaps->getAvailableMaps())
    {
        m_sAvailableMapsListForForceSelectComboBox += sMapName + '\0';
        //m_sAvailableMapsListForMapcycleListBox += sMapName + '\0';
    }

    m_vszAvailableMapsListForMapcycleListBox = new const char* [m_pMaps->getAvailableMaps().size()];
    size_t i = 0;
    for (const auto& sMapName : m_pMaps->getAvailableMaps())
    {
        m_vszAvailableMapsListForMapcycleListBox[i++] = sMapName.c_str();
    }

    /*
        Useful Dear ImGui links:
         - https://github.com/ocornut/imgui/tree/master/docs
         - https://github.com/ocornut/imgui/blob/master/docs/FAQ.md
         - https://github.com/ocornut/imgui/wiki/Useful-Extensions
         - Interactive online manual: https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html
         - GUI Editors:
           - online: https://raa.is/ImStudio/
           - https://github.com/Half-People/HImGuiEditor/tree/main
           - https://github.com/tpecholt/imrad
           - https://github.com/Code-Building/ImGuiBuilder
           - https://github.com/iamclint/ImGuiDesigner
    */

    // no need to initialize Dear ImGui since its resources are managed by PURE/PGE
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
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.71f, 0.71f, 0.71f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.71f, 0.71f, 0.71f, 1.00f);
    style.Colors[ImGuiCol_Separator] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_Tab] = ImVec4(100 / 255.f, 114 / 255.f, 63 / 255.f, 1.f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.16f, 0.18f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);
} // initialize()

void proofps_dd::GUI::shutdown()
{
    if (m_pObjLoadingScreenBg && m_pObjLoadingScreenLogoImg)
    {
        delete m_pObjLoadingScreenBg;
        delete m_pObjLoadingScreenLogoImg;
        m_pObjLoadingScreenBg = nullptr;
        m_pObjLoadingScreenLogoImg = nullptr;

        // no need to destroy Dear ImGui since its resources are managed by PURE/PGE
    }
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

void proofps_dd::GUI::textForNextFrame(const std::string& s, int x, int y) const
{
    m_pPge->getPure().getUImanager().textTemporalLegacy(s, x, y)->SetDropShadow(true);
}

void proofps_dd::GUI::textPermanent(const std::string& s, int x, int y) const
{
    m_pPge->getPure().getUImanager().textPermanentLegacy(s, x, y)->SetDropShadow(true);
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


PGE* proofps_dd::GUI::m_pPge = nullptr;
proofps_dd::Config* proofps_dd::GUI::m_pConfig = nullptr;
proofps_dd::Maps* proofps_dd::GUI::m_pMaps = nullptr;
proofps_dd::Networking* proofps_dd::GUI::m_pNetworking = nullptr;
proofps_dd::GUI::MenuState proofps_dd::GUI::m_currentMenu = proofps_dd::GUI::MenuState::Main;

PureObject3D* proofps_dd::GUI::m_pObjLoadingScreenBg = nullptr;
PureObject3D* proofps_dd::GUI::m_pObjLoadingScreenLogoImg = nullptr;
std::string proofps_dd::GUI::m_sAvailableMapsListForForceSelectComboBox;
//std::string proofps_dd::GUI::m_sAvailableMapsListForMapcycleListBox;
const char** proofps_dd::GUI::m_vszAvailableMapsListForMapcycleListBox = nullptr;


float proofps_dd::GUI::getCenterPosXForText(const std::string& text)
{
    return (ImGui::GetWindowSize().x - ImGui::CalcTextSize(text.c_str()).x) * 0.5f;
}

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
            sHint += cvar.getShortHint() + '\n' + '\n';
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
    constexpr float fContentHeight = 4 * (fBtnHeight + fBtnSpacingY) + fBtnSpacingY + 20 /* this should be version text size Y */;
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
    ImGui::SetCursorPosX(getCenterPosXForText(sVersion));
    ImGui::SetCursorPosY(fContentStartY + fBtnSpacingY*4);
    ImGui::Text("%s", sVersion.c_str());
}

float proofps_dd::GUI::drawPlayerNameInputBox()
{
    PGEcfgVariable& cvarClName = m_pPge->getConfigProfiles().getVars()[CVAR_CL_NAME];
    ImGui::AlignTextToFramePadding();
    static std::string sHintClName; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintClName, cvarClName);
    ImGui::Text("Player Name:");
    ImGui::SameLine();
    const auto fInputBoxPosX = ImGui::GetCursorPosX();
    
    ImGui::PushItemWidth(200);

    static char szPlayerName[MsgUserNameChange::nUserNameBufferLength];
    strncpy_s(
        szPlayerName,
        MsgUserNameChange::nUserNameBufferLength,
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
    ImGui::Text("[  C R E A T E  G A M E  ]");

    ImGui::Separator();
    ImGui::Indent();

    drawPlayerNameInputBox();

    ImGui::Separator();

    ImGui::Text("[ Map Configuration ]");

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

        ImGui::Text("Mapcycle:");
        ImGui::SameLine(fMapsAvailListBoxX);
        ImGui::Text("Available Maps:");

        const auto fBasePosY = ImGui::GetCursorPosY();

        // first I draw the 2 listboxes, and only after them I draw the move btns in between them
        ImGui::PushItemWidth(fMapListboxesWidth);
        int iActiveItemMapcycle = 0;
        const char* pszMapcycleItems[] = { "Map 1", "Map 2" };
        ImGui::ListBox("##listBoxMapcycle", &iActiveItemMapcycle, pszMapcycleItems, IM_ARRAYSIZE(pszMapcycleItems), nMapListboxesHeightAsItemCount);
       
        ImGui::SameLine(fMapsAvailListBoxX);
        static int iActiveItemMapsAvailable = 0;
        //const char* pszMapsAvailableItems[] = { "Map 1", "Map 2" };
        //ImGui::ListBox("##listBoxAvailMaps", &iActiveItemMapsAvailable, pszMapsAvailableItems, IM_ARRAYSIZE(pszMapsAvailableItems), nMapListboxesHeightAsItemCount);
        //const char* label, int* current_item, const char* const items[], int items_count, int height_items

        //const char* pm_sAvailableMapsListForMapcycleListBox[] = m_sAvailableMapsListForMapcycleListBox.c_str();
        ImGui::ListBox(
            "##listBoxAvailMaps",
            &iActiveItemMapsAvailable,
            m_vszAvailableMapsListForMapcycleListBox,
            static_cast<int>(m_pMaps->getAvailableMaps().size()),
            nMapListboxesHeightAsItemCount);
        

        ImGui::PopItemWidth();
        
        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY));
        ImGui::Button("<", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight));

        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY + fMapMoveBtnsVerticalDistanceFromEachOther));
        ImGui::Button("<<", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight));

        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY + fMapMoveBtnsVerticalDistanceFromEachOther * 2));
        ImGui::Button(">", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight));

        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY + fMapMoveBtnsVerticalDistanceFromEachOther * 3));
        ImGui::Button("<<", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight));

        // TODO: prefill by found maps; if left empty then mapcycle will govern it; CVAR: sv_map.
        PGEcfgVariable& cvarSvMap = m_pPge->getConfigProfiles().getVars()[proofps_dd::Maps::CVAR_SV_MAP];

        ImGui::AlignTextToFramePadding();
        static std::string sHintSvMap; // static so it is built up by addHintToItemByCVar() only once
        addHintToItemByCVar(sHintSvMap, cvarSvMap);
        ImGui::Text("Force-Start on Map:");

        ImGui::SameLine();
        ImGui::PushItemWidth(150);
        static int iSelectMapStart = -1;
        //const char* pszSelectMapStart[] = { "Map 1", "Map 2" };
        //ImGui::Combo("##comboForceMapStart", &iSelectMapStart, pszSelectMapStart, IM_ARRAYSIZE(pszSelectMapStart));
        
        if (iSelectMapStart == -1)
        {
            // initialize current item of comboForceMapStart to what is already set it CVAR
            iSelectMapStart = 0;
            if (!cvarSvMap.getAsString().empty() && (cvarSvMap.getAsString() != " "))
            {
                for (int i = 0; i < static_cast<int>(m_pMaps->getAvailableMaps().size()); i++)
                {
                    if (m_pMaps->getAvailableMaps()[i] == cvarSvMap.getAsString())
                    {
                        iSelectMapStart = i + 1; // +1 because iSelectMapStart 0 represents first " " elem in m_sAvailableMapsListForForceSelectComboBox
                        break;
                    }
                }
            }
        }

        if (ImGui::Combo("##comboForceMapStart", &iSelectMapStart, m_sAvailableMapsListForForceSelectComboBox.c_str(), m_sAvailableMapsListForForceSelectComboBox.size() + 1 /* +1 is the first empty item */))
        {
            if (iSelectMapStart == 0)
            {
                cvarSvMap.Set("");
            }
            else if (iSelectMapStart <= static_cast<int>(m_pMaps->getAvailableMaps().size()) /* first empty item as index 0 is NOT in getAvailableMaps(), that is why index can be == size() */)
            {
                cvarSvMap.Set(m_pMaps->getAvailableMaps()[iSelectMapStart-1]);
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

    ImGui::Text("[ Miscellaneous ]");
    ImGui::Indent();
    {
        PGEcfgVariable& cvarTickrate = m_pPge->getConfigProfiles().getVars()[CVAR_TICKRATE];
        bool bTR60Hz = cvarTickrate.getAsUInt() == 60u;

        ImGui::BeginGroup();
        {
            ImGui::AlignTextToFramePadding();
            static std::string sHintTickrate; // static so it is built up by addHintToItemByCVar() only once
            addHintToItemByCVar(sHintTickrate, cvarTickrate);
            ImGui::Text("Tickrate:");
            
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
            ImGui::Text("Client Updates:");

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

        ImGui::BeginGroup();
        {
            PGEcfgVariable& cvarSvAllowStrafeMidAir = m_pPge->getConfigProfiles().getVars()[CVAR_SV_ALLOW_STRAFE_MID_AIR];

            ImGui::AlignTextToFramePadding();
            static std::string sHintMidAirStrafe; // static so it is built up by addHintToItemByCVar() only once
            addHintToItemByCVar(sHintMidAirStrafe, cvarSvAllowStrafeMidAir);
            ImGui::Text("Mid-Air Strafe:");

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
        m_currentMenu = MenuState::Main;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("START >"))
    {
        m_pConfig->validate();
        if (m_pMaps->serverDecideWhichMapToLoad().empty())
        {            
            getConsole().EOLn("ERROR: Server is unable to select first map!");
            PGE::showErrorDialog("Server is unable to select first map!");
        }
        else
        {
            m_pPge->getConfigProfiles().getVars()[pge_network::PgeNetwork::CVAR_NET_SERVER].Set(true);
            if (!m_pPge->getConfigProfiles().writeConfiguration())
            {
                getConsole().EOLn("ERROR: failed to save current config profile!");
            }
            if (!m_pNetworking->isServer() && !m_pNetworking->reinitialize())
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
    ImGui::Text("[  J O I N  G A M E  ]");

    ImGui::Separator();
    ImGui::Indent();

    const auto fInputBoxPosX = drawPlayerNameInputBox();

    ImGui::Separator();

    PGEcfgVariable& cvarClServerIp = m_pPge->getConfigProfiles().getVars()[CVAR_CL_SERVER_IP];

    ImGui::AlignTextToFramePadding();
    static std::string sHintClServerIp; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintClServerIp, cvarClServerIp);
    ImGui::Text("Server IP:");

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
            m_pPge->getConfigProfiles().getVars()[pge_network::PgeNetwork::CVAR_NET_SERVER].Set(false);
            if (!m_pPge->getConfigProfiles().writeConfiguration())
            {
                getConsole().EOLn("ERROR: failed to save current config profile!");
            }
            if (m_pNetworking->isServer() && !m_pNetworking->reinitialize())
            {
                getConsole().EOLn("ERROR: failed to reinitialize networking subsystem: switch from server to client mode!");
            }
            m_currentMenu = MenuState::None;
            m_pPge->getPure().getWindow().SetCursorVisible(false);
        }
    }

    ImGui::Unindent();
} // drawJoinGameMenu

void proofps_dd::GUI::drawSettingsMenu(const float& fRemainingSpaceY)
{
    // fContentHeight is now calculated manually, in future it should be calculated somehow automatically by pre-defining abstract elements
    constexpr float fContentHeight = 145.f;
    const float fContentStartY = calcContentStartY(fContentHeight, fRemainingSpaceY);

    ImGui::SetCursorPos(ImVec2(20, fContentStartY));
    ImGui::Text("[  S E T T I N G S  ]");

    ImGui::Separator();
    ImGui::Indent();

    PGEcfgVariable& cvarGfxFullscreen = m_pPge->getConfigProfiles().getVars()[PGE::CVAR_GFX_WINDOWED];
    ImGui::AlignTextToFramePadding();
    static std::string sHintGfxFullscreen; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintGfxFullscreen, cvarGfxFullscreen);
    ImGui::Text("Fullscreen:");
    ImGui::SameLine();
    bool bFullscreen = !cvarGfxFullscreen.getAsBool();
    if (ImGui::Checkbox("##cbFullscreen", &bFullscreen))
    {
        // even tho the user can still cancel, here we always flip the config because we are rendering the checkbox based on this
        cvarGfxFullscreen.Set(!bFullscreen);

        ImGui::OpenPopup("Apply Setting");
    }

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Apply Setting", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("The game will restart now to apply the new configuration.");

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            getConsole().OLn("Initiating game restart to apply new settings ...");
            if (!m_pPge->getConfigProfiles().writeConfiguration())
            {
                getConsole().EOLn("ERROR: failed to save current config profile!");
            }
            m_pPge->setCookie(1); // this will force loop in WinMain() to restart the game
            m_currentMenu = MenuState::Exiting;
            m_pPge->getPure().getWindow().Close();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            // flip the config that we proactively flipped earlier in checkbox handler above
            m_pPge->getConfigProfiles().getVars()[PGE::CVAR_GFX_WINDOWED].Set(bFullscreen);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    PGEcfgVariable& cvarGfxVSync = m_pPge->getConfigProfiles().getVars()[PureScreen::CVAR_GFX_VSYNC];
    ImGui::AlignTextToFramePadding();
    static std::string sHintGfxVSync; // static so it is built up by addHintToItemByCVar() only once
    addHintToItemByCVar(sHintGfxVSync, cvarGfxVSync);
    ImGui::Text("V-Sync:");
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
        ImGui::Text("V-Sync is NOT supported on this hardware!");
    }

    ImGui::BeginGroup();
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Camera Follows:");
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

    bool bGfxCamTilting = m_pPge->getConfigProfiles().getVars()[CVAR_GFX_CAM_TILTING].getAsBool();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Camera Tilting:");
    ImGui::SameLine();
    if (ImGui::Checkbox("##cbCamTilt", &bGfxCamTilting))
    {
        m_pPge->getConfigProfiles().getVars()[CVAR_GFX_CAM_TILTING].Set(bGfxCamTilting);
    }

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

void proofps_dd::GUI::drawMainMenuCb()
{

    /*
        There should be a CVAR for enabling/disabling the menu.
        Reg tests will disable it from command line.
    */
    //pge.getPure().getWindow().SetCursorVisible(true/false);

    if ((m_currentMenu == MenuState::None) || (m_currentMenu == MenuState::Exiting))
    {
        return;
    }

    m_pPge->getPure().getWindow().SetCursorVisible(true);

    // Dear ImGui coordinates are the same as OS desktop/native coordinates which means that operating with ImGui::GetMainViewport() is
    // different than operating with getPure().getCamera().getViewport():
    // - PURE 2D viewport (0,0) is the CENTER, and positive Y goes UPWARDS from CENTER;
    // - Dear ImGui viewport (0,0) is the TOP LEFT, and positive Y goes DOWNWARDS from the TOP.
    const ImGuiViewport* const main_viewport = ImGui::GetMainViewport();
    const float fLogoImgHeight = m_pObjLoadingScreenLogoImg->getSizeVec().getY();
    constexpr float fMenuWndWidth = 470.f; // just put here the widest submenu's required width
    const float fMenuWndHeight = main_viewport->WorkSize.y - fLogoImgHeight;
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkSize.x/2 - fMenuWndWidth/2, fLogoImgHeight), ImGuiCond_FirstUseEver);
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
} // drawMainMenuCb()

proofps_dd::GUI::GUI()
{

}

proofps_dd::GUI::~GUI()
{
    shutdown();
}
