/*
    ###################################################################################
    Player.cpp
    Player class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "Player.h"
#include "PRooFPS-dd-packet.h"


// ############################### PUBLIC ################################


Player::Player(
    PR00FsUltimateRenderingEngine& gfx,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const std::string& sIpAddress) :
    m_connHandleServerSide(connHandle),
    m_sIpAddress(sIpAddress),
    m_sName("Player " + std::to_string(++m_nPlayerInstanceCntr)),
    m_nHealth(100),
    m_nOldHealth(100),
    m_fPlayerAngleY(0.f),
    m_fOldPlayerAngleY(0.f),
    m_pObj(PGENULL),
    m_pWpn(NULL),
    m_gfx(gfx),
    m_fGravity(0.f),
    m_bJumping(false),
    b_mCanFall(true),
    m_bRunning(true),
    m_bAllowJump(false),
    m_bExpectingStartPos(true),
    m_bRespawn(false),
    m_nFrags(0),
    m_nOldFrags(0),
    m_nDeaths(0),
    m_nOldDeaths(0)
{
    BuildPlayerObject(true);
}

Player::Player(const Player& other) :
    m_connHandleServerSide(other.m_connHandleServerSide),
    m_sIpAddress(other.m_sIpAddress),
    m_sName(other.m_sName),
    m_nHealth(other.m_nHealth),
    m_nOldHealth(other.m_nOldHealth),
    m_vecPos(other.m_vecPos),
    m_vecOldPos(other.m_vecOldPos),
    m_fPlayerAngleY(other.m_fPlayerAngleY),
    m_vWpnAngle(other.m_vWpnAngle),
    m_vOldWpnAngle(other.m_vOldWpnAngle),
    m_vecForce(other.m_vecForce),
    m_fOldPlayerAngleY(other.m_fOldPlayerAngleY),
    m_pObj(PGENULL),
    m_pWpn(NULL),
    m_timeLastWeaponSwitch(other.m_timeLastWeaponSwitch),
    m_gfx(other.m_gfx),
    m_fGravity(other.m_fGravity),
    m_bJumping(other.m_bJumping),
    b_mCanFall(other.b_mCanFall),
    m_bRunning(other.m_bRunning),
    m_bAllowJump(other.m_bAllowJump),
    m_bExpectingStartPos(other.m_bExpectingStartPos),
    m_timeDied(other.m_timeDied),
    m_bRespawn(other.m_bRespawn),
    m_nFrags(other.m_nFrags),
    m_nOldFrags(other.m_nOldFrags),
    m_nDeaths(other.m_nDeaths),
    m_nOldDeaths(other.m_nOldDeaths)
{
    BuildPlayerObject(true);
}

Player& Player::operator=(const Player& other)
{
    m_connHandleServerSide = other.m_connHandleServerSide;
    m_sIpAddress = other.m_sIpAddress;
    m_sName = other.m_sName;
    m_nHealth = other.m_nHealth;
    m_nOldHealth = other.m_nOldHealth;
    m_vecPos = other.m_vecPos;
    m_vecOldPos = other.m_vecOldPos;
    m_fPlayerAngleY = other.m_fPlayerAngleY;
    m_vWpnAngle = other.m_vWpnAngle;
    m_vOldWpnAngle = other.m_vOldWpnAngle;
    m_vecForce = other.m_vecForce;
    m_fOldPlayerAngleY = other.m_fOldPlayerAngleY;
    m_timeLastWeaponSwitch = other.m_timeLastWeaponSwitch;
    m_gfx = other.m_gfx;
    m_fGravity = other.m_fGravity;
    m_bJumping = other.m_bJumping;
    b_mCanFall = other.b_mCanFall;
    m_bRunning = other.m_bRunning;
    m_bAllowJump = other.m_bAllowJump;
    m_bExpectingStartPos = other.m_bExpectingStartPos;
    m_timeDied = other.m_timeDied;
    m_bRespawn = other.m_bRespawn;
    m_nFrags = other.m_nFrags;
    m_nOldFrags = other.m_nOldFrags;
    m_nDeaths = other.m_nDeaths;
    m_nOldDeaths = other.m_nOldDeaths;

    BuildPlayerObject(true);

    return *this;
}

Player::~Player()
{
    if (m_pObj)
    {
        delete m_pObj;  // yes, dtor will remove this from its Object3DManager too!
    }

    for (auto pWpn : getWeapons())
    {
        if (pWpn)
        {
            delete pWpn;
        }
    }
    getWeapons().clear();
}

const pge_network::PgeNetworkConnectionHandle& Player::getServerSideConnectionHandle() const
{
    return m_connHandleServerSide;
}

const std::string& Player::getIpAddress() const
{
    return m_sIpAddress;
}

const std::string& Player::getName() const
{
    return m_sName;
}

void Player::setName(const std::string& sName)
{
    m_sName = sName;
}

CConsole& Player::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* Player::getLoggerModuleName()
{
    return "Player";
}

int Player::getHealth() const
{
    return m_nHealth;
}

PureVector& Player::getPos()
{
    return m_vecPos;
}

PureVector& Player::getOPos()
{
    return m_vecOldPos;
}

TPureFloat& Player::getAngleY()
{
    return m_fPlayerAngleY;
}

TPureFloat& Player::getOldAngleY()
{
    return m_fOldPlayerAngleY;
}

PureObject3D* Player::getObject3D() const
{
    return m_pObj;
}

float Player::getGravity() const
{
    return m_fGravity;
}

bool Player::isJumping() const
{
    return m_bJumping;
}

bool Player::isFalling() const
{
    return (m_fGravity == 0.0f);
}

bool Player::canFall() const
{
    return b_mCanFall;
}

void Player::UpdateOldPos() {
    m_vecOldPos = m_vecPos;
    m_fOldPlayerAngleY = m_fPlayerAngleY;
    m_vOldWpnAngle = m_vWpnAngle;
}

void Player::SetHealth(int value) {
    m_nHealth = max(0, min(value, 100));
}

void Player::UpdateOldHealth()
{
    m_nOldHealth = m_nHealth;
}

int Player::getOldHealth() const
{
    return m_nOldHealth;
}

void Player::SetGravity(float value) {
    m_fGravity = value;
}

bool Player::jumpAllowed() const {
    return m_bAllowJump;
}

void Player::SetJumpAllowed(bool b) {
    m_bAllowJump = b;
}

void Player::Jump() {
    if (!jumpAllowed())
    {
        return;
    }

    m_bAllowJump = false;
    m_bJumping = true;
    m_fGravity = GAME_GRAVITY_MAX;
    m_vecForce.SetX(m_vecPos.getX() - m_vecOldPos.getX());
    m_vecForce.SetY(m_vecPos.getY() - m_vecOldPos.getY());
    m_vecForce.SetZ(m_vecPos.getZ() - m_vecOldPos.getZ());
}

void Player::StopJumping() {
    m_bJumping = false;
}

void Player::DoDamage(int dmg) {
    m_nHealth = m_nHealth - dmg;
    if (m_nHealth < 0) m_nHealth = 0;
}

void Player::SetCanFall(bool state) {
    b_mCanFall = state;
}

bool Player::isRunning() const
{
    return m_bRunning;
}

void Player::SetRun(bool state)
{
    m_bRunning = state;
}

PureVector& Player::getForce()
{
    return m_vecForce;
}

bool Player::isExpectingStartPos() const
{
    return m_bExpectingStartPos;
}

void Player::SetExpectingStartPos(bool b)
{
    m_bExpectingStartPos = b;
}

Weapon* Player::getWeapon()
{
    return m_pWpn;
}

const Weapon* Player::getWeapon() const
{
    return m_pWpn;
}

void Player::SetWeapon(Weapon* wpn, bool bRecordSwitchTime, bool bServer)
{
    if (!wpn)
    {
        getConsole().EOLn("Player::%s(): CANNOT set nullptr!", __func__);
        return;
    }

    if (bServer /* client should not do availability check since it is not aware of wpn availability of the players */ &&
        !wpn->isAvailable())
    {
        //getConsole().EOLn(
        //    "Player::%s(): wpn %s is NOT available!", __func__, wpn->getFilename().c_str());
        return;
    }

    if (m_pWpn && (m_pWpn != wpn))
    {
        // we already have a current different weapon, so this will be a weapon switch
        m_pWpn->getObject3D().Hide();
        wpn->getObject3D().getAngleVec() = m_pWpn->getObject3D().getAngleVec();

        if (bRecordSwitchTime)
        {
            getTimeLastWeaponSwitch() = std::chrono::steady_clock::now();
        }
    }
    wpn->getObject3D().Show();
    m_pWpn = wpn;
}

std::chrono::time_point<std::chrono::steady_clock>& Player::getTimeLastWeaponSwitch()
{
    return m_timeLastWeaponSwitch;
}

std::vector<Weapon*>& Player::getWeapons()
{
    return m_weapons;
}

const std::vector<Weapon*>& Player::getWeapons() const
{
    return m_weapons;
}

const Weapon* Player::getWeaponByFilename(const std::string& sFilename) const
{
    for (const auto pWpn : m_weapons)
    {
        if (pWpn)
        {
            if (pWpn->getFilename() == sFilename)
            {
                return pWpn;
            }
        }
    }

    return nullptr;
}

Weapon* Player::getWeaponByFilename(const std::string& sFilename)
{
    for (const auto pWpn : m_weapons)
    {
        if (pWpn)
        {
            if (pWpn->getFilename() == sFilename)
            {
                return pWpn;
            }
        }
    }

    return nullptr;
}

PureVector& Player::getOldWeaponAngle()
{
    return m_vOldWpnAngle;
}

PureVector& Player::getWeaponAngle()
{
    return m_vWpnAngle;
}

void Player::Die(bool bMe, bool bServer)
{
    getTimeDied() = std::chrono::steady_clock::now();
    if (bMe)
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): I died!", __func__);
    }
    else
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): other player died!", __func__);
    }
    SetHealth(0);
    getObject3D()->Hide();
    getWeapon()->getObject3D().Hide();
    if (bServer)
    {
        // server instance has the right to modify death count, clients will just receive it in update
        getDeaths()++;
        //getConsole().OLn("PRooFPSddPGE::%s(): new death count: %d!", __func__, getDeaths());
    }
}

void Player::Respawn(bool /*bMe*/, const Weapon& wpnDefaultAvailable, bool bServer)
{
    getObject3D()->Show();

    for (auto pWpn : getWeapons())
    {
        if (!pWpn)
        {
            continue;
        }

        pWpn->Reset();
        if (pWpn->getFilename() == wpnDefaultAvailable.getFilename())
        {
            pWpn->SetAvailable(true);
            SetWeapon(pWpn, false, bServer);
            pWpn->UpdatePosition(getObject3D()->getPosVec());
        }
    }
}

std::chrono::time_point<std::chrono::steady_clock>& Player::getTimeDied()
{
    return m_timeDied;
}

bool& Player::getRespawnFlag()
{
    return m_bRespawn;
}

int& Player::getFrags()
{
    return m_nFrags;
}

const int& Player::getFrags() const
{
    return m_nFrags;
}

int& Player::getOldFrags()
{
    return m_nOldFrags;
}

int& Player::getDeaths()
{
    return m_nDeaths;
}

const int& Player::getDeaths() const
{
    return m_nDeaths;
}

int& Player::getOldDeaths()
{
    return m_nOldDeaths;
}

void Player::UpdateFragsDeaths()
{
    m_nOldFrags = m_nFrags;
    m_nOldDeaths = m_nDeaths;
}

bool Player::canTakeItem(const MapItem& item) const
{
    switch (item.getType())
    {
    case MapItemType::ITEM_WPN_PISTOL:
    case MapItemType::ITEM_WPN_MACHINEGUN:
    {
        const auto it = m_mapItemTypeToWeaponFilename.find(item.getType());
        if (it == m_mapItemTypeToWeaponFilename.end())
        {
            getConsole().EOLn(
                "Player::%s(): failed to find weapon by item type %d!", __func__, item.getType());
            return false;
        }
        const std::string& sWeaponName = it->second;
        const Weapon* const pWpn = getWeaponByFilename(sWeaponName);
        if (!pWpn)
        {
            getConsole().EOLn(
                "Player::%s(): failed to find weapon by name %s for item type %d!", __func__, sWeaponName.c_str(), item.getType());
            return false;
        }
        return pWpn->canIncBulletCount();
    }
    case MapItemType::ITEM_HEALTH:
        return (getHealth() < 100);
    default:
        ;
    }
    return false;
}

void Player::TakeItem(MapItem& item, pge_network::PgePacket& pktWpnUpdate)
{
    // invoked only on server
    switch (item.getType())
    {
    case MapItemType::ITEM_WPN_PISTOL:
    case MapItemType::ITEM_WPN_MACHINEGUN:
    {
        const auto it = m_mapItemTypeToWeaponFilename.find(item.getType());
        if (it == m_mapItemTypeToWeaponFilename.end())
        {
            getConsole().EOLn(
                "Player::%s(): failed to find weapon by item type %d!", __func__, item.getType());
            return;
        }
        const std::string& sWeaponBecomingAvailable = it->second;
        Weapon* const pWpnBecomingAvailable = getWeaponByFilename(sWeaponBecomingAvailable);
        if (!pWpnBecomingAvailable)
        {
            getConsole().EOLn(
                "Player::%s(): failed to find weapon by name %s for item type %d!", __func__, sWeaponBecomingAvailable.c_str(), item.getType());
            return;
        }

        item.Take();
        if (pWpnBecomingAvailable->isAvailable())
        {
            // just increase bullet count
            // TODO: this will be a problem for non-reloadable wpns such as rail gun, because there this value will be 0,
            // but we will think about it later then ... probably in such case bullets_default will be used
            pWpnBecomingAvailable->IncBulletCount(pWpnBecomingAvailable->getVars()["reloadable"].getAsInt());
            //getConsole().OLn(
            //    "Player::%s(): weapon %s pickup, already available, just inc unmag to: %u",
            //    __func__, sWeaponBecomingAvailable.c_str(), pWpnBecomingAvailable->getUnmagBulletCount());
        }
        else
        {
            // becoming available with default bullet count
            //getConsole().OLn(
            //    "Player::%s(): weapon %s pickup, becomes available with mag: %u, unmag: %u",
            //    __func__, sWeaponBecomingAvailable.c_str(), pWpnBecomingAvailable->getMagBulletCount(), pWpnBecomingAvailable->getUnmagBulletCount());
        }
        pWpnBecomingAvailable->SetAvailable(true);  // becomes available on server side
        proofps_dd::MsgWpnUpdate::initPkt(
            pktWpnUpdate,
            0 /* ignored by client anyway */,
            sWeaponBecomingAvailable,
            pWpnBecomingAvailable->isAvailable(),
            pWpnBecomingAvailable->getMagBulletCount(),
            pWpnBecomingAvailable->getUnmagBulletCount());  // becomes available on client side (after pkt being sent)
        break;
    }
    case MapItemType::ITEM_HEALTH:
        item.Take();
        SetHealth(getHealth() + MapItem::ITEM_HEALTH_HP_INC);
        break;
    default:
        getConsole().EOLn(
            "Player::%s(): unknown item type %d!", __func__, item.getType());
    }
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


// The game engine's Weapon system and the game's Map system are 2 independent subsystems.
// This map provides the logical connection between pickupable MapItems and actual weapons.
// So when player picks up a specific MapItem, we know which weapon should become available for the player.
// I'm not planning to move Map stuff to the game engine because this kind of Map is very game-specific.
const std::map<MapItemType, std::string> Player::m_mapItemTypeToWeaponFilename =
{
    {MapItemType::ITEM_WPN_PISTOL, "pistol.txt"},
    {MapItemType::ITEM_WPN_MACHINEGUN, "machinegun.txt"}
};

uint32_t Player::m_nPlayerInstanceCntr = 0;

void Player::BuildPlayerObject(bool blend) {
    m_pObj = m_gfx.getObject3DManager().createPlane(GAME_PLAYER_W, GAME_PLAYER_H);
    if (!m_pObj)
    {
        throw std::runtime_error("Failed to create object for new player!");
    }

    m_pObj->SetName(m_pObj->getName() + " (for Player w connHandle " + std::to_string(static_cast<uint32_t>(m_connHandleServerSide)) + ")");
    m_pObj->SetDoubleSided(true);
    if (blend)
    {
        m_pObj->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE_MINUS_SRC_ALPHA);
    }
    m_pObj->SetLit(false);

    PureTexture* pTexPlayer = m_gfx.getTextureManager().createFromFile((std::string(GAME_TEXTURES_DIR) + "giraffe1m.bmp").c_str());
    m_pObj->getMaterial().setTexture(pTexPlayer);
}
