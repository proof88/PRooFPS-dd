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

        GUI(const GUI&) = delete;
        GUI& operator=(const GUI&) = delete;
        GUI(GUI&&) = delete;
        GUI&& operator=(GUI&&) = delete;

        void initialize();

    protected:

    private:

        PGE& m_pge;

        // ---------------------------------------------------------------------------

        static void drawMainMenuCb();

    }; // class GUI

} // namespace proofps_dd
