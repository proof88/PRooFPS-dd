/*
    ###################################################################################
    MapItem.cpp
    Map items for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "MapItem.h"


// ############################### PUBLIC ################################


std::ostream& operator<< (std::ostream& s, const MapItemType& mit)
{
    switch (mit)
    {
    case MapItemType::ITEM_WPN_PISTOL:
        return (s << "WpnPistol");
    case MapItemType::ITEM_WPN_MACHINEGUN:
        return (s << "WpnMchgun");
    case MapItemType::ITEM_HEALTH:
        return (s << "Health");
    default:
        break;
    }
    return (s << "Unknown Item");
}

MapItem::MapItem(PR00FsReducedRenderingEngine& gfx, const MapItemType& itemType, const PRREVector& pos) :
    m_gfx(gfx),
    m_obj(nullptr),
    m_fObjPosOriginalY(pos.getY()),
    m_itemType(itemType),
    m_bTaken(false),
    m_fSinusMotionDegrees(0.f)
{
    // TODO: throw when createPlane() fails
    m_obj = gfx.getObject3DManager().createPlane(0.5f, 0.5f);
    m_obj->getPosVec() = pos;
    PRRETexture* tex = nullptr;
    switch (itemType)
    {
    case MapItemType::ITEM_WPN_PISTOL:
        tex = gfx.getTextureManager().createFromFile("gamedata\\textures\\map_item_wpn_pistol.bmp");
        break;
    case MapItemType::ITEM_WPN_MACHINEGUN:
        tex = gfx.getTextureManager().createFromFile("gamedata\\textures\\map_item_wpn_mchgun.bmp");
        break;
    case MapItemType::ITEM_HEALTH:
        tex = gfx.getTextureManager().createFromFile("gamedata\\textures\\map_item_health.bmp");
        break;
    default:
        break;
    }
    
    if (tex)
    {
        m_obj->getMaterial().setTexture(tex);
    }
}

MapItem::~MapItem()
{
    if (m_obj)
    {
        delete m_obj;
        m_obj = nullptr;
    }
}

const MapItemType& MapItem::getType() const
{
    return m_itemType;
}

const PRREVector& MapItem::getPos() const
{
    return m_obj->getPosVec();
}

const PRREObject3D& MapItem::getObject3D() const
{
    return *m_obj;
}

PRREObject3D& MapItem::getObject3D()
{
    return *m_obj;
}

bool MapItem::isTaken() const
{
    return m_bTaken;
}

void MapItem::Take()
{
    if (isTaken())
    {
        return;
    }

    m_timeTaken = std::chrono::steady_clock::now();
    m_bTaken = true;
    m_obj->Hide();
}

const std::chrono::time_point<std::chrono::steady_clock>& MapItem::getTimeTaken() const
{
    return m_timeTaken;
}

void MapItem::Update(float factor)
{
    m_fSinusMotionDegrees += factor;
    if (m_fSinusMotionDegrees >= 359.9f)
    {
        m_fSinusMotionDegrees = 0.f;
    }
    m_obj->getPosVec().SetY(m_fObjPosOriginalY + sin(PFL::degToRad(m_fSinusMotionDegrees)) * 0.2f);
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################