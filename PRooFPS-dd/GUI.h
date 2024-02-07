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

        static GUI& getGuiInstance(PGE& pge);   /**< Gets the singleton instance. */

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        void initialize();
        void shutdown();

        void showLoadingScreen(
            int nProgress,
            const std::string& sMapFilename);
        void hideLoadingScreen();

        void textForNextFrame(const std::string& s, int x, int y) const;
        void textPermanent(const std::string& s, int x, int y) const;

    protected:

    private:

        enum class Menu
        {
            None,
            Main,
            CreateGame,
            JoinGame,
            Settings
        };

        static PGE* m_pPge;
        static Menu m_currentMenu;

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
