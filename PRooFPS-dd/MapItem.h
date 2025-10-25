#pragma once

/*
    ###################################################################################
    MapItem.h
    Map items for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include <chrono>
#include <map>

#include "Pure/include/external/Math/PureVector.h"
#include "Pure/include/external/PR00FsUltimateRenderingEngine.h"

namespace proofps_dd
{

    enum class MapItemType
    {
        ITEM_WPN_PISTOL,
        ITEM_WPN_MACHINEGUN,
        ITEM_WPN_BAZOOKA,
        ITEM_WPN_PUSHA,
        ITEM_WPN_MACHINEPISTOL,
        ITEM_WPN_SHOTGUN,
        ITEM_HEALTH,
        ITEM_ARMOR
    };
    
    std::ostream& operator<< (std::ostream& s, const MapItemType& eMapItemType);
    
    class MapItem
    {
    public:
    
        typedef uint32_t MapItemId;
    
        static constexpr uint32_t ITEM_ARMOR_AP_INC = 50;
        static constexpr uint32_t ITEM_ARMOR_RESPAWN_SECS = 10;
        static constexpr uint32_t ITEM_HEALTH_HP_INC = 20;
        static constexpr uint32_t ITEM_HEALTH_RESPAWN_SECS = 10;
        static constexpr uint32_t ITEM_WPN_PISTOL_RESPAWN_SECS = 20;
        static constexpr uint32_t ITEM_WPN_MACHINEGUN_RESPAWN_SECS = 20;
        static constexpr uint32_t ITEM_WPN_BAZOOKA_RESPAWN_SECS = 20;
        static constexpr uint32_t ITEM_WPN_PUSHA_RESPAWN_SECS = 20;
        static constexpr uint32_t ITEM_WPN_MACHINEPISTOL_RESPAWN_SECS = 30;
        static constexpr uint32_t ITEM_WPN_SHOTGUN_RESPAWN_SECS = 30;
    
        static const MapItemId& getGlobalMapItemId();
        static void resetGlobalData();
    
        static uint32_t getItemRespawnTimeSecs(const MapItem& mapItem);
        static std::string toString(const MapItemType& eMapItemType);
    
        // ---------------------------------------------------------------------------
    
        MapItem(PR00FsUltimateRenderingEngine& gfx, const MapItemType& eMapItemType, const PureVector& pos);
        ~MapItem();

        MapItem(const MapItem&) = delete;
        MapItem& operator=(const MapItem&) = delete;
        MapItem(MapItem&&) = delete;
        MapItem&& operator=(MapItem&&) = delete;
    
        const MapItemId&    getId() const;
        const MapItemType&  getType() const;
        const PureVector&   getPos() const;
        const PureObject3D& getObject3D() const;
        //PureObject3D&     getObject3D();
        std::string toString() const;
    
        bool isTaken() const;
        void take();
        void unTake();
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeTaken() const;
        void update(float factor);
    
    protected:
    
    private:
    
        static MapItemId m_globalMapItemId;  /**< Next unique id for identifying. Used by server and client instances. */
    
        static std::map<MapItemType, PureObject3D*> m_mapReferenceObjects;
    
        MapItemId m_id;                      /**< Unique id for identifying. Used by server and client instances.
                                                  Must be equal for same item across server and clients, used in packets too. */
    
        PR00FsUltimateRenderingEngine&                     m_gfx;
        PureObject3D*                                      m_obj;
        float                                              m_fObjPosOriginalY;     /**< The vertical floating movement is relative to this coord. */
        MapItemType                                        m_eMapItemType;
        bool                                               m_bTaken;
        std::chrono::time_point<std::chrono::steady_clock> m_timeTaken;
        float                                              m_fSinusMotionDegrees;  /**< For iterating the vertical floating movement. */
    
        // ---------------------------------------------------------------------------
    
    }; // class MapItem

} // namespace proofps_dd