#pragma once

/*
    ###################################################################################
    Minimap.h
    Minimap class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "CConsole.h"

#include "PGE.h"

// PGE has, but here in application we dont have imconfig.h thus we should not try including it!
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include "imgui.h"

#include "Maps.h"
#include "Player.h"

namespace proofps_dd
{

    class Minimap
    {
    public:

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        Minimap(
            PGE& pge,
            Maps& maps,
            std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers);
        ~Minimap();

        void show();
        void hide();
        bool visible() const;
        void updateVisuals();

    protected:

        Minimap(const Minimap&) = delete;
        Minimap& operator=(const Minimap&) = delete;
        Minimap(Minimap&&) = delete;
        Minimap&& operator=(Minimap&&) = delete;

    private:

        PGE& m_pge;
        Maps& m_maps;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        PureObject3D* m_pObjDebugVpTopLeft;
        PureObject3D* m_pObjDebugVpBottomRight;
        bool m_bVisible = false;

        ImVec2 getMinimapSizeInPixels() const;

        float getMinimapXfromWorldSpaceX(const float& posWorldX) const;
        float getMinimapYfromWorldSpaceY(const float& posWorldY) const;

        void drawBackAndViewportRectangles();

        // ---------------------------------------------------------------------------

    }; // class Minimap

} // namespace proofps_dd
