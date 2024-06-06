#pragma once

/*
    ###################################################################################
    GUI.h
    GUI for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include <functional>
#include <string>

#include "CConsole.h"

#include "PGE.h"

#include "Config.h"
#include "GameMode.h"
#include "Maps.h"
#include "Minimap.h"
#include "Networking.h"
#include "Player.h"
#include "XHair.h"

// PGE has, but here in application we dont have imconfig.h thus we should not try including it!
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include "imgui.h"

namespace proofps_dd
{

    class GUI
    {
    public:

        enum class MenuState
        {
            None,       /* Menu is not displayed (we are in-game) */
            Main,
            CreateGame,
            JoinGame,
            Settings,
            Exiting     /* User requested closing the app */
        };

        static constexpr char* CVAR_GUI_MAINMENU = "gui_mainmenu";

        static GUI& getGuiInstance(
            PGE& pge,
            proofps_dd::Config& config,
            proofps_dd::Maps& maps,
            proofps_dd::Networking& networking,
            std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);   /**< Gets the singleton instance. */

        static const char* getLoggerModuleName();
        static CConsole& getConsole();

        // ---------------------------------------------------------------------------

        void initialize();
        void shutdown();

        /* Main Menu Handling */

        const MenuState& getMenuState() const;
        void resetMenuState(bool bExitingFromGameSession);

        /* Misc */

        void showLoadingScreen(
            int nProgress,
            const std::string& sMapFilename);
        void hideLoadingScreen();
        bool showBgWithLogo();
        bool hideBgWithLogo();
        XHair* getXHair();
        Minimap* getMinimap();

        void textForNextFrame(const std::string& s, int nPureX, int nPureY) const;
        void textPermanent(const std::string& s, int nPureX, int nPureY) const;

        void showRespawnTimer();
        void hideRespawnTimer();

        void setGameModeInstance(proofps_dd::GameMode& gm);

    protected:

    private:

        static PGE* m_pPge;
        static Config* m_pConfig;
        static Maps* m_pMaps;
        static Networking* m_pNetworking;
        static std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>* m_pMapPlayers;

        /* Main Menu Handling */

        static MenuState m_currentMenu;

        /* In-Game GUI elements */

        static bool m_bShowRespawnTimer;
        static std::chrono::time_point<std::chrono::steady_clock> m_timePlayerDied;

        /* Misc */

        static XHair* m_pXHair;
        static Minimap* m_pMinimap;
        static PureObject3D* m_pObjLoadingScreenBg;
        static PureObject3D* m_pObjLoadingScreenLogoImg;
        static std::string m_sAvailableMapsListForForceSelectComboBox;

        static ImFont* m_pImFont;

        static GameMode* m_pGameMode;

        // ---------------------------------------------------------------------------

        /* Main Menu Handling */

        static void addHintToItemByCVar(std::string& sHint, const PGEcfgVariable& cvar);
        static float calcContentStartY(const float& fContentHeight, const float& fRemainingSpaceY);
        static void drawMainMenu(const float& fRemainingSpaceY);
        static float drawPlayerNameInputBox();
        static void drawCreateGameMenu(const float& fRemainingSpaceY);
        static void drawJoinGameMenu(const float& fRemainingSpaceY);
        static void drawSettingsMenu(const float& fRemainingSpaceY);
        static void drawWindowForMainMenu();

        /* In-Game GUI elements */

        static void drawRespawnTimer();
        static void drawXHairHoverText();
        static void updateXHair();

        /* Misc */

        static float getDearImGui2DposXFromPure2DposX(const float& fPureX);
        static float getDearImGui2DposYFromPure2DposY(const float& fPureY);
        static float getDearImGui2DposXforCenteredText(const std::string& text, const float& fImGuiX);
        static float getDearImGui2DposXforWindowCenteredText(const std::string& text);
        static void drawText(const float& fImGuiX, const float& fImGuiY, const std::string& text);
        static void drawTextShadowed(const float& fImGuiX, const float& fImGuiY, const std::string& text);

        static void drawDearImGuiCb(); // this needs to be static, causing a lot of other members also need to be static

        // ---------------------------------------------------------------------------

        GUI();
        ~GUI();

        GUI(const GUI&) = delete;
        GUI& operator=(const GUI&) = delete;
        GUI(GUI&&) = delete;
        GUI&& operator=(GUI&&) = delete;

    }; // class GUI

} // namespace proofps_dd
