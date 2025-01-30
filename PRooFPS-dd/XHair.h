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

// PGE has, but here in application we dont have imconfig.h thus we should not try including it!
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include "imgui.h" // for ImVec4

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

        void showIdText(
            const std::string& sText,
            const ImVec4& color = {1.f, 1.f, 1.f, 1.f});
        void hideIdText();
        const std::string& getIdText() const;
        const ImVec4& getColor() const;

        void startBlinking();
        void stopBlinking();

        void handleMagEmpty();
        void handleMagLoaded();

        void handleCooldownStart();
        void handleCooldownEnd();

        void setBaseScaling(float scaleFactor);
        void setRelativeScaling(float relativeScaleFactor);

        void updateVisuals();

    protected:

        XHair(const XHair&) = delete;
        XHair& operator=(const XHair&) = delete;
        XHair(XHair&&) = delete;
        XHair&& operator=(XHair&&) = delete;

    private:

        enum class HighlightRect
        {
            Upper = 0,
            Lower,
            Left,
            Right
        };

        PGE& m_pge;
        PureObject3D* m_pObjXHair;
        TPURE_XY m_prevXHairPos;
        PureVector m_vecUnprojected;
        PureObject3D* m_pObjDebugCube;
        PureObject3D* m_vHLightRects[4] = {};
        std::string m_sIdText;
        ImVec4 m_clrIdText;
        std::chrono::time_point<std::chrono::steady_clock> m_timeStartedBlinking;
        bool m_bVisible = false;
        bool m_bBlinking = false;
        float m_fBaseScaling = 1.f;
        float m_fRelativeScaleFactor = 1.f;

        // ---------------------------------------------------------------------------

        void adjustHighlightSize(const bool& bCanShowHighlightPerXHairMovement);
        void updateHighlight();

    }; // class XHair

} // namespace proofps_dd
