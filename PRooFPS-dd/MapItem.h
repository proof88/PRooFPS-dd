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

    typedef uint32_t MapItemId;

    static const uint32_t ITEM_HEALTH_HP_INC = 20;
    static const uint32_t ITEM_HEALTH_RESPAWN_SECS = 5;

    static const MapItemId& getGlobalMapItemId();
    static void ResetGlobalMapItemId();

    // ---------------------------------------------------------------------------

    MapItem(PR00FsReducedRenderingEngine& gfx, const MapItemType& itemType, const PRREVector& pos);
    ~MapItem();

    const MapItemId&    getId() const;
    const MapItemType&  getType() const;
    const PRREVector&   getPos() const;
    const PRREObject3D& getObject3D() const;
    //PRREObject3D&     getObject3D();

    bool isTaken() const;
    void Take();
    void UnTake();
    const std::chrono::time_point<std::chrono::steady_clock>& getTimeTaken() const;
    void Update(float factor);

protected:

private:

    static MapItemId m_globalMapItemId;  /**< Next unique id for identifying. Used by server and client instances. */

    MapItemId m_id;                      /**< Unique id for identifying. Used by server and client instances.
                                              Must be equal for same item across server and clients, used in packets too. */

    PR00FsReducedRenderingEngine&                      m_gfx;
    PRREObject3D*                                      m_obj;
    float                                              m_fObjPosOriginalY;     /**< The vertical floating movement is relative to this coord. */
    MapItemType                                        m_itemType;
    bool                                               m_bTaken;
    std::chrono::time_point<std::chrono::steady_clock> m_timeTaken;
    float                                              m_fSinusMotionDegrees;  /**< For iterating the vertical floating movement. */

    // ---------------------------------------------------------------------------

}; // class MapItem
