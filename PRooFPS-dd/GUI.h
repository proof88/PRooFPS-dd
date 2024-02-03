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

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        GUI(PGE& pge);
        ~GUI();

        GUI(const GUI&) = delete;
        GUI& operator=(const GUI&) = delete;
        GUI(GUI&&) = delete;
        GUI&& operator=(GUI&&) = delete;

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

        PGE& m_pge;

        PureObject3D* m_pObjLoadingScreenBg;
        PureObject3D* m_pObjLoadingScreenImg;

        // ---------------------------------------------------------------------------

        static void drawMainMenuCb();

    }; // class GUI

} // namespace proofps_dd
