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

        static constexpr char* szCvarGuiXHairIdentifiesPlayers = "gui_xhair_identifies_players";

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        XHair(PGE& pge);
        ~XHair();

        PureObject3D& getObject3D();
        
        void show();
        void showInCenter();
        void hide();
        bool visible() const;

        void updateUnprojectedCoords(PureCamera& cam);
        const PureVector& getUnprojectedCoords() const;

        void showIdText(const std::string& sText);
        void hideIdText();
        const std::string& getIdText() const;

        void startBlinking();
        void stopBlinking();

        void handleMagEmpty();
        void handleMagLoaded();

        void handleCooldownStart();
        void handleCooldownEnd();

        void updateVisuals();

    protected:

        XHair(const XHair&) = delete;
        XHair& operator=(const XHair&) = delete;
        XHair(XHair&&) = delete;
        XHair&& operator=(XHair&&) = delete;

    private:

        PGE& m_pge;
        PureObject3D* m_pObjXHair;
        PureVector m_vecUnprojected;
        PureObject3D* m_pObjDebugCube;
        std::string m_sIdText;
        std::chrono::time_point<std::chrono::steady_clock> m_timeStartedBlinking;
        bool m_bVisible = false;
        bool m_bBlinking = false;

        // ---------------------------------------------------------------------------

    }; // class XHair

} // namespace proofps_dd
