/*
    ###################################################################################
    MapItem.cpp
    Map items for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "MapItem.h"


// ############################### PUBLIC ################################


std::ostream& proofps_dd::operator<< (std::ostream& s, const proofps_dd::MapItemType& mit)
{
    switch (mit)
    {
    case proofps_dd::MapItemType::ITEM_WPN_PISTOL:
        return (s << "WpnPistol");
    case proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN:
        return (s << "WpnMchgun");
    case proofps_dd::MapItemType::ITEM_WPN_BAZOOKA:
        return (s << "WpnBazooka");
    case proofps_dd::MapItemType::ITEM_HEALTH:
        return (s << "Health");
    default:
        break;
    }
    return (s << "Unknown Item");
}

const uint32_t proofps_dd::MapItem::ITEM_HEALTH_HP_INC;
const uint32_t proofps_dd::MapItem::ITEM_HEALTH_RESPAWN_SECS;

const uint32_t proofps_dd::MapItem::ITEM_WPN_PISTOL_RESPAWN_SECS;

const uint32_t proofps_dd::MapItem::ITEM_WPN_MACHINEGUN_RESPAWN_SECS;

const uint32_t proofps_dd::MapItem::ITEM_WPN_BAZOOKA_RESPAWN_SECS;

const proofps_dd::MapItem::MapItemId& proofps_dd::MapItem::getGlobalMapItemId()
{
    return m_globalMapItemId;
}

void proofps_dd::MapItem::resetGlobalData()
{
    m_globalMapItemId = 0;
    for (auto& refPair : m_mapReferenceObjects)
    {
        delete refPair.second;
    }
    m_mapReferenceObjects.clear();
}

uint32_t proofps_dd::MapItem::getItemRespawnTimeSecs(const proofps_dd::MapItem& mapItem)
{
    switch (mapItem.getType())
    {
    case proofps_dd::MapItemType::ITEM_WPN_PISTOL:
        return ITEM_WPN_PISTOL_RESPAWN_SECS;
    case proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN:
        return ITEM_WPN_MACHINEGUN_RESPAWN_SECS;
    case proofps_dd::MapItemType::ITEM_WPN_BAZOOKA:
        return ITEM_WPN_BAZOOKA_RESPAWN_SECS;
    case proofps_dd::MapItemType::ITEM_HEALTH:
        return ITEM_HEALTH_RESPAWN_SECS;
    default:
        return 10000;  // dont let unhandled item respawn soon, so we will see there must be a problem
    }
}

proofps_dd::MapItem::MapItem(PR00FsUltimateRenderingEngine& gfx, const proofps_dd::MapItemType& itemType, const PureVector& pos) :
    m_id(m_globalMapItemId++),
    m_gfx(gfx),
    m_obj(nullptr),
    m_fObjPosOriginalY(pos.getY()),
    m_itemType(itemType),
    m_bTaken(false),
    m_fSinusMotionDegrees(0.f)
{
    const auto it = m_mapReferenceObjects.find(itemType);
    if (it == m_mapReferenceObjects.end())
    {
        // TODO: throw when createPlane() fails
        m_mapReferenceObjects[itemType] = gfx.getObject3DManager().createPlane(0.5f, 0.5f);
        m_mapReferenceObjects[itemType]->Hide();
        PureTexture* tex = nullptr;
        switch (itemType)
        {
        case proofps_dd::MapItemType::ITEM_WPN_PISTOL:
            tex = gfx.getTextureManager().createFromFile("gamedata\\textures\\map_item_wpn_pistol.bmp");
            break;
        case proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN:
            tex = gfx.getTextureManager().createFromFile("gamedata\\textures\\map_item_wpn_mchgun.bmp");
            break;
        case proofps_dd::MapItemType::ITEM_WPN_BAZOOKA:
            tex = gfx.getTextureManager().createFromFile("gamedata\\textures\\map_item_wpn_bazooka.bmp");
            break;
        case proofps_dd::MapItemType::ITEM_HEALTH:
            tex = gfx.getTextureManager().createFromFile("gamedata\\textures\\map_item_health.bmp");
            break;
        default:
            // TODO: throw for unhandled type
            break;
        }

        if (tex)
        {
            m_mapReferenceObjects[itemType]->getMaterial().setTexture(tex);
        }
    }

    m_obj = gfx.getObject3DManager().createCloned(*m_mapReferenceObjects[itemType]);
    m_obj->Show();
    m_obj->getPosVec() = pos;
}

proofps_dd::MapItem::~MapItem()
{
    // m_mapReferenceObjects is cleared when Maps invokes ResetGlobalData()
    if (m_obj)
    {
        delete m_obj;
        m_obj = nullptr;
    }
}

const proofps_dd::MapItem::MapItemId& proofps_dd::MapItem::getId() const
{
    return m_id;
}

const proofps_dd::MapItemType& proofps_dd::MapItem::getType() const
{
    return m_itemType;
}

const PureVector& proofps_dd::MapItem::getPos() const
{
    return m_obj->getPosVec();
}

const PureObject3D& proofps_dd::MapItem::getObject3D() const
{
    return *m_obj;
}

//PureObject3D& proofps_dd::MapItem::getObject3D()
//{
//    return *m_obj;
//}

bool proofps_dd::MapItem::isTaken() const
{
    return m_bTaken;
}

void proofps_dd::MapItem::take()
{
    // executed by both server and clients, only when server says so, however only server is responsible for checking m_timeTaken
    if (isTaken())
    {
        return;
    }

    m_timeTaken = std::chrono::steady_clock::now();
    m_bTaken = true;
    m_obj->Hide();
}

void proofps_dd::MapItem::unTake()
{
    // executed by both server and clients, only when server says so
    if (!isTaken())
    {
        return;
    }

    m_bTaken = false;
    m_obj->Show();
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::MapItem::getTimeTaken() const
{
    return m_timeTaken;
}

void proofps_dd::MapItem::update(float factor)
{
    // executed by both server and clients, always, and this is not synchronized between players
    m_fSinusMotionDegrees += factor;
    if (m_fSinusMotionDegrees >= 359.9f)
    {
        m_fSinusMotionDegrees = 0.f;
    }
    m_obj->getPosVec().SetY(m_fObjPosOriginalY + sin(PFL::degToRad(m_fSinusMotionDegrees)) * 0.2f);
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


proofps_dd::MapItem::MapItemId proofps_dd::MapItem::m_globalMapItemId = 0;
std::map<proofps_dd::MapItemType, PureObject3D*> proofps_dd::MapItem::m_mapReferenceObjects;