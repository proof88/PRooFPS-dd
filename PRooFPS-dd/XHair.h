#pragma once

/*
    ###################################################################################
    XHair.h
    Crosshair class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "CConsole.h"

#include "PGE.h"

#include "Consts.h"

namespace proofps_dd
{

    /**
    * Finally a dedicated class for crosshair was needed, because from PRooFPS-dd v0.2.5, the crosshair
    * becomes smarter and smarter, and for the related functionalities we should keep some logic separate.
    */
    class XHair
    {
    public:

        //static constexpr char* szCvarGuiXHairIdentifiesPlayers = "gui_xhair_identifies_players";

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        XHair(PGE& pge);
        ~XHair();

        PureObject3D& getObject3D();
        
        void show();
        void showInCenter();
        void hide();

    protected:

        XHair(const XHair&) = delete;
        XHair& operator=(const XHair&) = delete;
        XHair(XHair&&) = delete;
        XHair&& operator=(XHair&&) = delete;

    private:

        PGE& m_pge;
        PureObject3D* m_pObjXHair;

        // ---------------------------------------------------------------------------

    }; // class XHair

} // namespace proofps_dd
