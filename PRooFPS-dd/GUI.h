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
#include "DeathKillEventLister.h"
#include "GameMode.h"
#include "Maps.h"
#include "Minimap.h"
#include "Networking.h"
#include "Player.h"
#include "Smoke.h"
#include "XHair.h"

// PGE has, but here in application we dont have imconfig.h thus we should not try including it!
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include "imgui.h"

namespace proofps_dd
{

    class GUI
    {
    public:

        enum class MainMenuState
        {
            None,       /* Menu is not displayed (we are in-game) */
            Main,
            CreateGame,
            JoinGame,
            Settings,
            About,
            Exiting     /* User requested closing the app */
        };

        enum class InGameMenuState
        {
            None,
            TeamSelect
        };

        /** As of v0.5, values are looped by pressing TAB in-game. */
        enum class GameInfoPage
        {
            None,
            FragTable,
            AllPlayersDebugDataServer,
            ServerConfig,
            COUNT
        };

        static constexpr char* CVAR_GUI_MAINMENU = "gui_mainmenu";

        static GUI& getGuiInstance(
            PGE& pge,
            proofps_dd::Config& config,
            proofps_dd::Maps& maps,
            proofps_dd::Networking& networking,
            std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            const PgeObjectPool<proofps_dd::Smoke>& smokes);   /**< Gets the singleton instance. */

        static const char* getLoggerModuleName();
        static CConsole& getConsole();

        // ---------------------------------------------------------------------------

        void initialize();
        void shutdown();

        /* Main Menu Handling */

        const MainMenuState& getMainMenuState() const;

        /* In-Game Menu Handling */

        const InGameMenuState& getInGameMenuState() const;
        static void hideInGameMenu();
        static void showHideInGameTeamSelectMenu();

        /**
        * Primarily for main menu control, but shall reset any other menu as well.
        * Typical use cases:
        *  - bExitingFromGameSession is false: init at startup,
        *  - bExitingFromGameSession is true : disconnecting from current game session.
        * 
        * Behavior depends on CVAR_GUI_MAINMENU as well:
        *  - if true : MainMenuState::Main will be selected.
        *  - if false: MainMenuState::None or MainMenuState::Exiting will be selected, depending on bExitingFromGameSession.
        */
        void resetMenuStates(bool bExitingFromGameSession);

        /* Misc */

        void showLoadingScreen(
            int nProgress,
            const std::string& sMapFilename);
        void hideLoadingScreen();
        bool showBgWithLogo();
        bool hideBgWithLogo();
        XHair* getXHair();
        Minimap* getMinimap();
        DeathKillEventLister* getDeathKillEvents();
        EventLister* getItemPickupEvents();
        EventLister* getPlayerHpChangeEvents();
        EventLister* getPlayerApChangeEvents();
        EventLister* getPlayerAmmoChangeEvents();
        static void showGameObjectives();
        static void hideGameObjectives();
        static void showAndLoopGameInfoPages();

        void textForNextFrame(const std::string& s, int nPureX, int nPureY) const;
        void textPermanent(const std::string& s, int nPureX, int nPureY) const;

        void showRespawnTimer(
            const Player* const pKillerPlayer);
        void hideRespawnTimer();
        void fastForwardRespawnTimer(
            std::chrono::milliseconds::rep byMillisecs);

    protected:

    private:

        static PGE* m_pPge;
        static Config* m_pConfig;
        static Maps* m_pMaps;
        static Networking* m_pNetworking;
        static std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>* m_pMapPlayers;
        static const PgeObjectPool<proofps_dd::Smoke>* m_pSmokes;

        /* Main Menu Handling */

        static MainMenuState m_currentMenuInMainMenu;

        /* In-Game GUI elements */

        static bool m_bShowRespawnTimer;
        static std::chrono::time_point<std::chrono::steady_clock> m_timePlayerDied;
        static std::string m_sRespawnTimerExtraText;
        static std::string m_sRespawnTimerExtraText2;
        static bool m_bShowHealthAndArmor;

        /* In-Game Menu Handling */

        static InGameMenuState m_currentMenuInInGameMenu;

        /* Misc */

        static XHair* m_pXHair;
        static Minimap* m_pMinimap;
        static DeathKillEventLister* m_pEventsDeathKill;
        static EventLister* m_pEventsItemPickup;
        static EventLister* m_pEventsPlayerHpChange;
        static EventLister* m_pEventsPlayerApChange;
        static EventLister* m_pEventsPlayerAmmoChange;
        static PureObject3D* m_pObjLoadingScreenBg;
        static PureObject3D* m_pObjLoadingScreenLogoImg;
        static std::string m_sAvailableMapsListForForceSelectComboBox;

        static ImFont* m_pImFontFragTableNonScaled;
        static ImFont* m_pImFontHudGeneralScaled;
        static float m_fFontSizePxHudGeneralScaled;

        static GameInfoPage m_gameInfoPageCurrent;

        // ---------------------------------------------------------------------------

        /* Main Menu Handling */

        static void addHintToItemByCVar(std::string& sHint, const PGEcfgVariable& cvar);
        static float calcContentStartY(const float& fContentHeight, const float& fRemainingSpaceY);
        static void drawMainMenu(const float& fRemainingSpaceY);
        static float drawPlayerNameInputBox();
        static void drawCreateGameServerMapSelection();
        static void drawTabCreateGameServerSettings();
        static void drawTabCreateGameServerTweaks();
        static void drawCreateGameMenu(const float& fRemainingSpaceY);
        static void drawJoinGameMenu(const float& fRemainingSpaceY);
        static void drawTabWeaponSettings();
        static void drawTabMiscSettings();
        static void showConfigApplyAndRestartDialogBox(PGEcfgVariable& cvar, const std::string& sPopupId);
        static void drawSettingsMenu(const float& fRemainingSpaceY);
        static void drawAboutMenu(const float& fRemainingSpaceY);
        static void drawWindowForMainMenu();

        /* In-Game GUI elements */

        static void drawRespawnTimer();
        static void drawXHairHoverText();
        static void updateXHair();
        static void drawCurrentPlayerInfo(const proofps_dd::Player& player);
        static void updateDeathKillEvents();
        static void updateItemPickupEvents();
        static void updatePlayerHpChangeEvents();
        static void updatePlayerApChangeEvents();
        static void updatePlayerAmmoChangeEvents();
        
        static void calculatePlayerNameColWidthAndTableWidthPixels(
            float& fTableWidthPixels,
            float& fPlayerNameColWidthPixels,
            float fPlayerNameColReqWidthPixels,
            const float& fTableColIndentPixels,
            const float& fColsTotalWidthAfterPlayerNameCol);
        static void calculatePlayersTableGeometry(
            const std::vector<const char*>& vecHeaderLabels,
            const float& fTableColIndentPixels,
            std::vector<float>& vecColumnWidthsPixels,
            float& fTableStartPosX,
            float& fTableWidthPixels,
            float& fPlayerNameColWidthPixels,
            float& fTableHeightPixels);
        static void drawFragTable_columnLoopForPlayer(
            const proofps_dd::PlayersTableRow& player,
            const std::vector<const char*>& vecHeaderLabels,
            const int& iColNetworkDataStart);
        static void drawTableCaption(
            const std::string& sTableCaption,
            const float& fStartPosY,
            const float& fTableStartPosX,
            const float& fTableWidthPixels);
        
        /**
        * See explanation at GUI::drawPlayersTable().
        */
        typedef std::function<
            void(
                const proofps_dd::PlayersTableRow& player,
                const std::vector<const char*>& vecHeaderLabels,
                const int& iColNetworkDataStart
                )> CbColumnLoopForPlayerFunc;
        
        /**
        * See explanation at GUI::drawPlayersTable().
        */
        typedef std::function<
            bool(
                const proofps_dd::PlayersTableRow& player
                )> CbIsPlayerValidForCurrentRowFunc;

        static void drawPlayersTable(
            const std::vector<const char*>& vecHeaderLabels,
            const float& fTableColIndentPixels,
            const std::vector<float>& vecColumnWidthsPixels,
            const float& fTableStartPosX,
            const float& fTableWidthPixels,
            const float& fPlayerNameColWidthPixels,
            const float& fTableHeightPixels,
            const int& iColNetworkDataStart,
            CbIsPlayerValidForCurrentRowFunc cbIsPlayerValidForCurrentRowFunc,
            CbColumnLoopForPlayerFunc cbColumnLoopForPlayerFunc);
        static void drawFragTable(
            const std::string& sTableCaption,
            const float& fStartPosY,
            const std::vector<const char*>& vecHeaderLabels,
            const int& iColNetworkDataStart);
        
        static void drawGameObjectivesServer(const std::string& sTableCaption, const float& fStartPosY);
        static void drawGameObjectivesClient(const std::string& sTableCaption, const float& fStartPosY);
        
        static void drawAllPlayersDebugDataTableServer_columnLoopForPlayer(
            const proofps_dd::PlayersTableRow& player,
            const std::vector<const char*>& vecHeaderLabels,
            const int& iColNetworkDataStart);
        static void drawAllPlayersDebugDataServer();
        
        static void drawGameObjectives();
        static float drawClientConnectionDebugInfo(float fThisRowY);
        static void drawGameServerConfig();
        static void drawGameInfoPages();

        /* In-Game Menu Handling */

        static void drawInGameTeamSelectMenu(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>::iterator& itCurrentPlayer);
        static void drawInGameMenu(
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>::iterator& itCurrentPlayer);

        /* Misc */

        static float getDearImGui2DposXFromPure2DposX(const float& fPureX);
        static float getDearImGui2DposYFromPure2DposY(const float& fPureY);
        static float getDearImGui2DposXforCenteredText(const std::string& text, const float& fImGuiX);
        static float getDearImGui2DposXforRightAdjustedText(const std::string& text, const float& fImGuiX);
        static float getDearImGui2DposXforTableCurrentCellCenteredText(const std::string& text);
        static float getDearImGui2DposXforTableCurrentCellRightAdjustedText(const std::string& text);
        static float getDearImGui2DposXforWindowCenteredText(const std::string& text);
        static void drawText(const float& fImGuiX, const float& fImGuiY, const std::string& text);
        static void drawTextShadowed(const float& fImGuiX, const float& fImGuiY, const std::string& text);
        static void drawTextHighlighted(const float& fImGuiX, const float& fImGuiY, const std::string& text);
        static void ImGuiTextTableCurrentCellShortenedFit(const std::string& text, size_t nAppendLastNChars = 0);
        static void ImGuiTextTableCurrentCellCentered(const std::string& text);
        static void ImGuiTextTableCurrentCellRightAdjusted(const std::string& text);

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
