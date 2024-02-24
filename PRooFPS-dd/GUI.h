#pragma once

/*
    ###################################################################################
    GUI.h
    GUI for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <functional>
#include <string>

#include "../../Console/CConsole/src/CConsole.h"

#include "PGE.h"

#include "Config.h"
#include "Maps.h"

namespace proofps_dd
{

    class GUI
    {
    public:

        enum class MenuState
        {
            None,       /* Menu is not displayed (gaming) */
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
            proofps_dd::Maps& maps);   /**< Gets the singleton instance. */

        static const char* getLoggerModuleName();
        static CConsole& getConsole();

        // ---------------------------------------------------------------------------

        void initialize();
        void shutdown();

        const MenuState& getMenuState() const;
        void resetMenuState(bool bExitingFromGameSession);

        void showLoadingScreen(
            int nProgress,
            const std::string& sMapFilename);
        void hideLoadingScreen();

        void textForNextFrame(const std::string& s, int x, int y) const;
        void textPermanent(const std::string& s, int x, int y) const;

    protected:

    private:

        static PGE* m_pPge;
        static Config* m_pConfig;
        static Maps* m_pMaps;
        static MenuState m_currentMenu;

        static float getCenterPosXForText(const std::string& text);
        static void addHintToItemByCVar(std::string& sLongHint, const PGEcfgVariable& cvar);
        static void drawMainMenu();
        static float drawPlayerNameInputBox();
        static void drawCreateGameMenu();
        static void drawJoinGameMenu();
        static void drawSettingsMenu();
        static void drawMainMenuCb();

        // ---------------------------------------------------------------------------

        PureObject3D* m_pObjLoadingScreenBg;
        PureObject3D* m_pObjLoadingScreenLogoImg;

        GUI();
        ~GUI();

        GUI(const GUI&) = delete;
        GUI& operator=(const GUI&) = delete;
        GUI(GUI&&) = delete;
        GUI&& operator=(GUI&&) = delete;

    }; // class GUI

} // namespace proofps_dd
