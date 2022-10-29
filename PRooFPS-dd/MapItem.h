#pragma once

/*
    ###################################################################################
    MapItem.h
    Map items for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <chrono>

#include "../../../PGE/PGE/PRRE/include/external/Math/PRREVector.h"
#include "../../../PGE/PGE/PRRE/include/external/PR00FsReducedRenderingEngine.h"

enum class MapItemType
{
    ITEM_WPN_PISTOL,
    ITEM_WPN_MACHINEGUN,
    ITEM_HEALTH
};

std::ostream& operator<< (std::ostream& s, const MapItemType& mi);

class MapItem
{
public:

    // ---------------------------------------------------------------------------

    MapItem(PR00FsReducedRenderingEngine& gfx, const MapItemType& itemType, const PRREVector& pos);
    ~MapItem();

    const MapItemType& getType() const;
    const PRREVector& getPos() const;
    const PRREObject3D& getObject3D() const;
    PRREObject3D& getObject3D();

    bool isTaken() const;
    void Take();
    const std::chrono::time_point<std::chrono::steady_clock>& getTimeTaken() const;

protected:

private:

    PR00FsReducedRenderingEngine&                      m_gfx;
    PRREObject3D*                                      m_obj;
    MapItemType                                        m_itemType;
    bool                                               m_bTaken;
    std::chrono::time_point<std::chrono::steady_clock> m_timeTaken;

    // ---------------------------------------------------------------------------

}; // class MapItem
