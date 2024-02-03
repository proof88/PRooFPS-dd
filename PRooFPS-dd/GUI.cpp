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

// ############################### PUBLIC ################################


const char* proofps_dd::GUI::getLoggerModuleName()
{
    return "GUI";
}

CConsole& proofps_dd::GUI::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::GUI::GUI(PGE& pge) :
    m_pge(pge),
    m_pObjLoadingScreenBg(nullptr),
    m_pObjLoadingScreenImg(nullptr)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge
    // But they can be used in other functions.
}

proofps_dd::GUI::~GUI()
{
    shutdown();
}

void proofps_dd::GUI::initialize()
{
    m_pge.getPure().getUImanager().setDefaultFontSizeLegacy(20);
    m_pge.getPure().getUImanager().setGuiDrawCallback(drawMainMenuCb);

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
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);

    m_pObjLoadingScreenBg = m_pge.getPure().getObject3DManager().createPlane(
        m_pge.getPure().getCamera().getViewport().size.width,
        m_pge.getPure().getCamera().getViewport().size.height);
    m_pObjLoadingScreenBg->SetStickedToScreen(true);
    m_pObjLoadingScreenBg->SetDoubleSided(true);
    m_pObjLoadingScreenBg->SetTestingAgainstZBuffer(false);
    m_pObjLoadingScreenBg->SetLit(false);
    PureTexture* pTexBlack = m_pge.getPure().getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "black.bmp").c_str());
    m_pObjLoadingScreenBg->getMaterial().setTexture(pTexBlack);

    const auto fLoadingScreenImgWidth = m_pge.getPure().getCamera().getViewport().size.width * 0.8f;
    m_pObjLoadingScreenImg = m_pge.getPure().getObject3DManager().createPlane(
        fLoadingScreenImgWidth,
        (fLoadingScreenImgWidth * 0.5f) * 0.5f);
    m_pObjLoadingScreenImg->SetStickedToScreen(true);
    m_pObjLoadingScreenImg->SetDoubleSided(true);
    m_pObjLoadingScreenImg->SetTestingAgainstZBuffer(false);
    m_pObjLoadingScreenImg->SetLit(false);
    PureTexture* pTexLoadingScreen = m_pge.getPure().getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "PRooFPS-dd-logo.bmp").c_str());
    m_pObjLoadingScreenImg->getMaterial().setTexture(pTexLoadingScreen);

    
} // initialize()

void proofps_dd::GUI::shutdown()
{
    if (m_pObjLoadingScreenBg && m_pObjLoadingScreenImg)
    {
        delete m_pObjLoadingScreenBg;
        delete m_pObjLoadingScreenImg;
        m_pObjLoadingScreenBg = nullptr;
        m_pObjLoadingScreenImg = nullptr;
    }
}

void proofps_dd::GUI::showLoadingScreen(int nProgress, const std::string& sMapFilename)
{
    if (m_pObjLoadingScreenBg && m_pObjLoadingScreenImg)
    {
        m_pObjLoadingScreenBg->Show();
        m_pObjLoadingScreenImg->Show();
        textForNextFrame(
            "Loading Map: " + sMapFilename + " ... " + std::to_string(nProgress) + " %",
            200,
            m_pge.getPure().getWindow().getClientHeight() / 2 + static_cast<int>(m_pObjLoadingScreenImg->getPosVec().getY() - m_pObjLoadingScreenImg->getSizeVec().getY() / 2.f));
        m_pge.getPure().getRenderer()->RenderScene();
    }
}

void proofps_dd::GUI::hideLoadingScreen()
{
    if (m_pObjLoadingScreenBg && m_pObjLoadingScreenImg)
    {
        m_pObjLoadingScreenBg->Hide();
        m_pObjLoadingScreenImg->Hide();
    }
}

void proofps_dd::GUI::textForNextFrame(const std::string& s, int x, int y) const
{
    m_pge.getPure().getUImanager().textTemporalLegacy(s, x, y)->SetDropShadow(true);
}

void proofps_dd::GUI::textPermanent(const std::string& s, int x, int y) const
{
    m_pge.getPure().getUImanager().textPermanentLegacy(s, x, y)->SetDropShadow(true);
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


//static float getCenterX(const std::string& text)
//{
//    return (ImGui::GetWindowSize().x - ImGui::CalcTextSize(text.c_str()).x) * 0.5f;
//}
//
void proofps_dd::GUI::drawMainMenuCb()
{
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
    /*
        There should be a CVAR for enabling/disabling the menu.
        Reg tests will disable it from command line.
    */
    //pge.getPure().getWindow().SetCursorVisible(true/false);

    //bool bOpen = true;
    //ImGui::ShowDemoWindow(&bOpen);

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x, main_viewport->WorkSize.y), ImGuiCond_FirstUseEver);

    ImGui::Begin("WndMainMenu", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
    {
        // main menu
        //{
        //    /*
        //            CREATE GAME
        //             JOIN GAME
        //             SETTINGS
        //               EXIT
        //    */
        //    constexpr float fBtnWidth = 150.f;
        //    constexpr float fBtnHeight = 20.f;
        //    constexpr float fBtnStartY = 100.f;
        //    
        //    ImGui::SetCursorPos(ImVec2(ImGui::GetMainViewport()->GetCenter().x - fBtnWidth / 2.f, fBtnStartY));
        //    ImGui::Button("C R E A T E  G A M E", ImVec2(fBtnWidth, fBtnHeight)); // remove size argument (ImVec2) to auto-resize
        //    
        //    ImGui::SetCursorPos(ImVec2(ImGui::GetMainViewport()->GetCenter().x - fBtnWidth / 2.f, fBtnStartY + 30));
        //    ImGui::Button("J O I N  G A M E", ImVec2(fBtnWidth, fBtnHeight));
        //    
        //    ImGui::SetCursorPos(ImVec2(ImGui::GetMainViewport()->GetCenter().x - fBtnWidth / 2.f, fBtnStartY + 60));
        //    ImGui::Button("S E T T I N G S", ImVec2(fBtnWidth, fBtnHeight));
        //    
        //    ImGui::SetCursorPos(ImVec2(ImGui::GetMainViewport()->GetCenter().x - fBtnWidth / 2.f, fBtnStartY + 90));
        //    ImGui::Button("E X I T", ImVec2(fBtnWidth, fBtnHeight));
        //    
        //    const std::string sVersion = std::string("v") + proofps_dd::GAME_VERSION;
        //    ImGui::SetCursorPosX(getCenterX(sVersion));
        //    ImGui::SetCursorPosY(fBtnStartY + 120);
        //    ImGui::Text("%s", sVersion.c_str());
        //} // end main menu

        // create game
        //{
        //    ImGui::SetCursorPos(ImVec2(20, 38));
        //    ImGui::Text("[  C R E A T E  G A M E  ]");
        //
        //    ImGui::Separator();
        //    ImGui::Indent();
        //
        //    {
        //        // TODO: max length limited by MsgUserNameChange::nUserNameBufferLength; CVAR: cl_name.
        //        ImGui::AlignTextToFramePadding();
        //        ImGui::Text("Player Name:");
        //        ImGui::SameLine();
        //        ImGui::PushItemWidth(200);
        //        char szPlayerName[128] = "";
        //        ImGui::InputText("##inputPlayerName", szPlayerName, IM_ARRAYSIZE(szPlayerName));
        //        ImGui::PopItemWidth();
        //    } // end Player
        //
        //    ImGui::Separator();
        //    ImGui::Text("[ Map Configuration ]");
        //
        //    ImGui::Indent();
        //    {
        //        const auto fBasePosX = ImGui::GetCursorPosX();
        //
        //        constexpr float fMapMoveBtnsWidth = 30;
        //        constexpr float fMapMoveBtnsHeight = 19;
        //        constexpr float fMapMoveBtnsVerticalDistanceFromListBoxes = 10;
        //        constexpr float fMapMoveBtnsVerticalDistanceFromEachOther = fMapMoveBtnsHeight + 1;
        //        constexpr float fMapListboxesWidth = 150;
        //        constexpr int nMapListboxesHeightAsItemCount = 4;
        //        const float fMapMoveBtnsPosX = fBasePosX + fMapListboxesWidth + fMapMoveBtnsVerticalDistanceFromListBoxes;
        //        const float fMapsAvailListBoxX = fMapMoveBtnsPosX + fMapMoveBtnsWidth + fMapMoveBtnsVerticalDistanceFromListBoxes;
        //
        //        ImGui::Text("Mapcycle:");
        //        ImGui::SameLine(fMapsAvailListBoxX);
        //        ImGui::Text("Available Maps:");
        //
        //        const auto fBasePosY = ImGui::GetCursorPosY();
        //
        //        // first I draw the 2 listboxes, and only after them I draw the move btns in between them
        //        ImGui::PushItemWidth(fMapListboxesWidth);
        //        int iActiveItemMapcycle = 0;
        //        const char* pszMapcycleItems[] = { "Map 1", "Map 2" };
        //        ImGui::ListBox("##listBoxMapcycle", &iActiveItemMapcycle, pszMapcycleItems, IM_ARRAYSIZE(pszMapcycleItems), nMapListboxesHeightAsItemCount);
        //       
        //        ImGui::SameLine(fMapsAvailListBoxX);
        //        int iActiveItemMapsAvailable = 0;
        //        const char* pszMapsAvailableItems[] = { "Map 1", "Map 2" };
        //        ImGui::ListBox("##listBoxAvailMaps", &iActiveItemMapsAvailable, pszMapsAvailableItems, IM_ARRAYSIZE(pszMapsAvailableItems), nMapListboxesHeightAsItemCount);
        //        ImGui::PopItemWidth();
        //        
        //        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY));
        //        ImGui::Button("<", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight));
        //
        //        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY + fMapMoveBtnsVerticalDistanceFromEachOther));
        //        ImGui::Button("<<", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight));
        //
        //        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY + fMapMoveBtnsVerticalDistanceFromEachOther * 2));
        //        ImGui::Button(">", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight));
        //
        //        ImGui::SetCursorPos(ImVec2(fMapMoveBtnsPosX, fBasePosY + fMapMoveBtnsVerticalDistanceFromEachOther * 3));
        //        ImGui::Button("<<", ImVec2(fMapMoveBtnsWidth, fMapMoveBtnsHeight));
        //
        //        // TODO: prefill by found maps; if left empty then mapcycle will govern it; CVAR: sv_map.
        //        ImGui::AlignTextToFramePadding();
        //        ImGui::Text("Force-Start on Map:");
        //        ImGui::SameLine();
        //        ImGui::PushItemWidth(150);
        //        int iSelectMapStart = 0;
        //        const char* pszSelectMapStart[] = { "Map 1", "Map 2" };
        //        ImGui::Combo("##comboForceMapStart", &iSelectMapStart, pszSelectMapStart, IM_ARRAYSIZE(pszSelectMapStart));
        //        ImGui::PopItemWidth();
        //        
        //    } // end Map Config
        //    ImGui::Unindent();
        //
        //    ImGui::Separator();
        //
        //    ImGui::Text("[ Miscellaneous ]");
        //    ImGui::Indent();
        //    {
        //        ImGui::BeginGroup();
        //        {
        //            // TODO: CVAR: tickrate
        //            ImGui::AlignTextToFramePadding();
        //            ImGui::Text("Tickrate:");
        //            ImGui::SameLine();
        //            ImGui::RadioButton("High (60 Hz)", false);
        //            ImGui::SameLine();
        //            ImGui::RadioButton("Low (20 Hz)", false);
        //        }
        //        ImGui::EndGroup();
        //
        //        ImGui::BeginGroup();
        //        {
        //            // TODO: CVAR: cl_updaterate.
        //            ImGui::AlignTextToFramePadding();
        //            ImGui::Text("Client Updates:");
        //            ImGui::SameLine();
        //            ImGui::RadioButton("High (60 Hz)", false);
        //            ImGui::SameLine();
        //            ImGui::RadioButton("Low (20 Hz)", false);
        //        }
        //        ImGui::EndGroup();
        //
        //        ImGui::BeginGroup();
        //        {
        //            // TODO: CVAR: sv_allow_strafe_mid_air and sv_allow_strafe_mid_air_full.
        //            ImGui::AlignTextToFramePadding();
        //            ImGui::Text("Mid-Air Strafe:");
        //            ImGui::SameLine();
        //            ImGui::RadioButton("Full", false);
        //            ImGui::SameLine();
        //            ImGui::RadioButton("Moderate", false);
        //            ImGui::SameLine();
        //            ImGui::RadioButton("Off", false);
        //        }
        //        ImGui::EndGroup();
        //    } // end Misc
        //    ImGui::Unindent();
        //
        //    ImGui::Separator();
        //
        //    // TODO: Cfg file to be saved.
        //    ImGui::Button("< BACK");
        //    ImGui::SameLine();
        //    // TODO: When clicking on START, CVAR net_server should become true. Cfg file to be saved.
        //    ImGui::Button("START >");
        //
        //    ImGui::Unindent();
        //} // end create game

        // join game
        //{
        //    ImGui::SetCursorPos(ImVec2(20, 38));
        //    ImGui::Text("[  J O I N  G A M E  ]");
        //
        //    ImGui::Separator();
        //    ImGui::Indent();
        //
        //    // TODO: max length limited by MsgUserNameChange::nUserNameBufferLength; CVAR: cl_name.
        //    ImGui::AlignTextToFramePadding();
        //    ImGui::Text("Player Name:");
        //    ImGui::SameLine();
        //    const auto fInputBoxPosX = ImGui::GetCursorPosX();
        //    ImGui::PushItemWidth(200);
        //    char szPlayerName[128] = "";
        //    ImGui::InputText("##inputPlayerName", szPlayerName, IM_ARRAYSIZE(szPlayerName));
        //    ImGui::PopItemWidth();
        //
        //    ImGui::Separator();
        //
        //    // TODO: to be validated for ip address format, CVAR: cl_server_ip.
        //    ImGui::AlignTextToFramePadding();
        //    ImGui::Text("Server IP:");
        //    ImGui::SameLine(fInputBoxPosX);
        //    ImGui::PushItemWidth(200);
        //    char szServerIP[128] = "";
        //    ImGui::InputText("##inputServerIP", szServerIP, IM_ARRAYSIZE(szServerIP));
        //    ImGui::PopItemWidth();
        //
        //    ImGui::Separator();
        //
        //    // TODO: Cfg file to be saved.
        //    ImGui::Button("< BACK");
        //    ImGui::SameLine();
        //    // TODO: CVAR net_server should become false. Cfg file to be saved.
        //    ImGui::Button("JOIN >");
        //
        //    ImGui::Unindent();
        //} // end join game

        // settings
        {
            /*
                Camera Target - radio group: xhair and player, player only; .

                Camera Tilting - checkbox; CVAR: .

                BACK - Cfg file to be saved.
            */
            ImGui::SetCursorPos(ImVec2(20, 38));
            ImGui::Text("[  S E T T I N G S  ]");

            ImGui::Separator();
            ImGui::Indent();

            // TODO: CVAR: gfx_windowed.
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Fullscreen:");
            ImGui::SameLine();
            bool bFullscreen = false;
            ImGui::Checkbox("##cbFullscreen", &bFullscreen);

            // TODO: CVAR: gfx_vsync.
            ImGui::AlignTextToFramePadding();
            ImGui::Text("V-Sync:");
            ImGui::SameLine();
            bool bVSync = false;
            ImGui::Checkbox("##cbVSync", &bVSync);

            ImGui::BeginGroup();
            {
                // TODO: CVAR: CVAR: gfx_cam_follows_xhair
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Camera Follows:");
                ImGui::SameLine();
                ImGui::RadioButton("XHair and Player", false);
                ImGui::SameLine();
                ImGui::RadioButton("Player Only", false);
            }
            ImGui::EndGroup();

            // TODO: CVAR: gfx_cam_tilting.
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Camera Tilting:");
            ImGui::SameLine();
            bool bCamTilt = false;
            ImGui::Checkbox("##cbCamTilt", &bCamTilt);

            ImGui::Separator();

            // TODO: Cfg file to be saved.
            ImGui::Button("< BACK");

            ImGui::Unindent();
        } // end settings
    }
    ImGui::End();
} // drawMainMenuCb()
