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

        static GUI& getGuiInstance(PGE& pge);   /**< Gets the singleton instance. */

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        void initialize();
        void shutdown();

        const MenuState& getMenuState() const;
        void resetMenuState();

        void showLoadingScreen(
            int nProgress,
            const std::string& sMapFilename);
        void hideLoadingScreen();

        void textForNextFrame(const std::string& s, int x, int y) const;
        void textPermanent(const std::string& s, int x, int y) const;

    protected:

    private:

        static PGE* m_pPge;
        static MenuState m_currentMenu;

        static float getCenterPosXForText(const std::string& text);
        static void drawMainMenu();
        static void drawCreateGameMenu();
        static void drawJoinGameMenu();
        static void drawSettingsMenu();
        static void drawMainMenuCb();

        // ---------------------------------------------------------------------------

        PureObject3D* m_pObjLoadingScreenBg;
        PureObject3D* m_pObjLoadingScreenImg;

        GUI();
        ~GUI();

        GUI(const GUI&) = delete;
        GUI& operator=(const GUI&) = delete;
        GUI(GUI&&) = delete;
        GUI&& operator=(GUI&&) = delete;

    }; // class GUI

} // namespace proofps_dd
