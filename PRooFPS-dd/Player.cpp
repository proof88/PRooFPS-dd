/*
    ###################################################################################
    Player.cpp
    Player and PlayerHandling classes for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "Player.h"
#include "PRooFPS-dd-packet.h"


// #######################################################################
// #                               PLAYER                                #
// #######################################################################


// ############################### PUBLIC ################################


proofps_dd::Player::Player(
    PGEcfgProfiles& cfgProfiles,
    std::list<Bullet>& bullets,
    PR00FsUltimateRenderingEngine& gfx,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const std::string& sIpAddress) :
    m_connHandleServerSide(connHandle),
    m_sIpAddress(sIpAddress),
    m_sName("Player " + std::to_string(++m_nPlayerInstanceCntr)),
    m_pObj(PGENULL),
    m_wpnMgr(cfgProfiles, gfx, bullets),
    m_cfgProfiles(cfgProfiles),
    m_bullets(bullets),
    m_gfx(gfx),
    m_fGravity(0.f),
    m_bJumping(false),
    b_mCanFall(true),
    m_bRunning(true),
    m_bAllowJump(false),
    m_bWillJump(false),
    m_bExpectingStartPos(true),
    m_strafe(proofps_dd::Strafe::NONE),
    m_bRespawn(false)
{
    BuildPlayerObject(true);
}

proofps_dd::Player::Player(const proofps_dd::Player& other) :
    m_connHandleServerSide(other.m_connHandleServerSide),
    m_sIpAddress(other.m_sIpAddress),
    m_sName(other.m_sName),
    m_vecOldNewValues(other.m_vecOldNewValues),
    m_vecForce(other.m_vecForce),
    m_pObj(PGENULL),
    m_wpnMgr(other.m_cfgProfiles, other.m_gfx, other.m_bullets),
    m_cfgProfiles(other.m_cfgProfiles),
    m_bullets(other.m_bullets),
    m_gfx(other.m_gfx),
    m_fGravity(other.m_fGravity),
    m_bJumping(other.m_bJumping),
    b_mCanFall(other.b_mCanFall),
    m_bRunning(other.m_bRunning),
    m_bAllowJump(other.m_bAllowJump),
    m_bWillJump(other.m_bWillJump),
    m_bExpectingStartPos(other.m_bExpectingStartPos),
    m_strafe(other.m_strafe),
    m_timeDied(other.m_timeDied),
    m_bRespawn(other.m_bRespawn)
{
    BuildPlayerObject(true);
}

proofps_dd::Player& proofps_dd::Player::operator=(const proofps_dd::Player& other)
{
    m_connHandleServerSide = other.m_connHandleServerSide;
    m_sIpAddress = other.m_sIpAddress;
    m_sName = other.m_sName;
    m_vecOldNewValues = other.m_vecOldNewValues;
    m_vecForce = other.m_vecForce;
    m_bullets = other.m_bullets;
    m_gfx = other.m_gfx;
    m_fGravity = other.m_fGravity;
    m_bJumping = other.m_bJumping;
    b_mCanFall = other.b_mCanFall;
    m_bRunning = other.m_bRunning;
    m_bAllowJump = other.m_bAllowJump;
    m_bWillJump = other.m_bWillJump;
    m_bExpectingStartPos = other.m_bExpectingStartPos;
    m_strafe = other.m_strafe;
    m_timeDied = other.m_timeDied;
    m_bRespawn = other.m_bRespawn;

    BuildPlayerObject(true);

    return *this;
}

proofps_dd::Player::~Player()
{
    if (m_pObj)
    {
        delete m_pObj;  // yes, dtor will remove this from its Object3DManager too!
    }

    m_wpnMgr.Clear();
}

const pge_network::PgeNetworkConnectionHandle& proofps_dd::Player::getServerSideConnectionHandle() const
{
    return m_connHandleServerSide;
}

const std::string& proofps_dd::Player::getIpAddress() const
{
    return m_sIpAddress;
}

const std::string& proofps_dd::Player::getName() const
{
    return m_sName;
}

void proofps_dd::Player::setName(const std::string& sName)
{
    m_sName = sName;
}

WeaponManager& proofps_dd::Player::getWeaponManager()
{
    return m_wpnMgr;
}

const WeaponManager& proofps_dd::Player::getWeaponManager() const
{
    return m_wpnMgr;
}

bool proofps_dd::Player::isDirty() const
{
    bool bDirtyFound = false;
    for (auto it = m_vecOldNewValues.begin(); (it != m_vecOldNewValues.end()) && !bDirtyFound; it++)
    {
        // this is the "2. value-returning visitor" from example here: https://en.cppreference.com/w/cpp/utility/variant/visit
        bDirtyFound = std::visit([](auto&& oldNewValue) -> bool { return oldNewValue.isDirty(); }, it->second);
    }
    return bDirtyFound;
}

void proofps_dd::Player::updateOldValues()
{
    for (auto& enumVariantPair : m_vecOldNewValues)
    {
        // this is the "1. void visitor" from example here: https://en.cppreference.com/w/cpp/utility/variant/visit
        std::visit([](auto&& oldNewValue) { oldNewValue.commit(); }, enumVariantPair.second);
    }
}

CConsole& proofps_dd::Player::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* proofps_dd::Player::getLoggerModuleName()
{
    return "Player";
}

PgeOldNewValue<int>& proofps_dd::Player::getHealth()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvHealth));
}

const PgeOldNewValue<int>& proofps_dd::Player::getHealth() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvHealth));
}

PgeOldNewValue<PureVector>& proofps_dd::Player::getPos()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<PureVector>>(m_vecOldNewValues.at(OldNewValueName::OvPos));
}

PgeOldNewValue<TPureFloat>& proofps_dd::Player::getAngleY()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<TPureFloat>>(m_vecOldNewValues.at(OldNewValueName::OvAngleY));
}

PureObject3D* proofps_dd::Player::getObject3D() const
{
    return m_pObj;
}

float proofps_dd::Player::getGravity() const
{
    return m_fGravity;
}

bool proofps_dd::Player::isJumping() const
{
    return m_bJumping;
}

bool proofps_dd::Player::isFalling() const
{
    return (m_fGravity == 0.0f);
}

bool proofps_dd::Player::canFall() const
{
    return b_mCanFall;
}

void proofps_dd::Player::SetHealth(int value) {
    getHealth().set(std::max(0, std::min(value, 100)));
}

void proofps_dd::Player::SetGravity(float value) {
    m_fGravity = value;
}

bool proofps_dd::Player::jumpAllowed() const {
    return m_bAllowJump;
}

void proofps_dd::Player::SetJumpAllowed(bool b) {
    m_bAllowJump = b;
}

void proofps_dd::Player::Jump() {
    if (!jumpAllowed())
    {
        return;
    }

    m_bAllowJump = false;
    m_bJumping = true;
    m_bWillJump = false;
    m_fGravity = proofps_dd::GAME_JUMP_GRAVITY_START;
    m_vecForce.SetX(getPos().getNew().getX() - getPos().getOld().getX());
    m_vecForce.SetY(getPos().getNew().getY() - getPos().getOld().getY());
    m_vecForce.SetZ(getPos().getNew().getZ() - getPos().getOld().getZ());
}

void proofps_dd::Player::StopJumping() {
    m_bJumping = false;
}

bool proofps_dd::Player::getWillJump() const
{
    return m_bWillJump;
}

void proofps_dd::Player::setWillJump(bool flag)
{
    if (!jumpAllowed())
    {
        return;
    }

    m_bWillJump = flag;
    m_timeLastWillJump = std::chrono::steady_clock::now();
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::Player::getTimeLastSetWillJump() const
{
    return m_timeLastWillJump;
}

void proofps_dd::Player::DoDamage(int dmg) {
    getHealth().set(getHealth().getNew() - dmg);
    if (getHealth().getNew() < 0)
    {
        getHealth().set(0);
    }
}

void proofps_dd::Player::SetCanFall(bool state) {
    b_mCanFall = state;
}

bool proofps_dd::Player::isRunning() const
{
    return m_bRunning;
}

void proofps_dd::Player::SetRun(bool state)
{
    m_bRunning = state;
    m_timeLastToggleRun = std::chrono::steady_clock::now();
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::Player::getTimeLastToggleRun() const
{
    return m_timeLastToggleRun;
}

const proofps_dd::Strafe& proofps_dd::Player::getStrafe() const
{
    return m_strafe;
}

void proofps_dd::Player::setStrafe(const proofps_dd::Strafe& strafe)
{
    m_strafe = strafe;
}

PureVector& proofps_dd::Player::getForce()
{
    return m_vecForce;
}

bool proofps_dd::Player::isExpectingStartPos() const
{
    return m_bExpectingStartPos;
}

void proofps_dd::Player::SetExpectingStartPos(bool b)
{
    m_bExpectingStartPos = b;
}

PgeOldNewValue<PureVector>& proofps_dd::Player::getWeaponAngle()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<PureVector>>(m_vecOldNewValues.at(OldNewValueName::OvWpnAngle));
}

void proofps_dd::Player::Die(bool bMe, bool bServer)
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
    if (m_wpnMgr.getCurrentWeapon())
    {
        m_wpnMgr.getCurrentWeapon()->getObject3D().Hide();
    }
    if (bServer)
    {
        // server instance has the right to modify death count, clients will just receive it in update
        getDeaths()++;
        //getConsole().OLn("PRooFPSddPGE::%s(): new death count: %d!", __func__, getDeaths());
    }
}

void proofps_dd::Player::Respawn(bool /*bMe*/, const Weapon& wpnDefaultAvailable, bool bServer)
{
    getObject3D()->Show();

    for (auto pWpn : m_wpnMgr.getWeapons())
    {
        if (!pWpn)
        {
            continue;
        }

        pWpn->Reset();
        if (pWpn->getFilename() == wpnDefaultAvailable.getFilename())
        {
            pWpn->SetAvailable(true);
            m_wpnMgr.setCurrentWeapon(pWpn, false, bServer);
            pWpn->UpdatePosition(getObject3D()->getPosVec());
        }
    }
}

std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::Player::getTimeDied()
{
    return m_timeDied;
}

bool& proofps_dd::Player::getRespawnFlag()
{
    return m_bRespawn;
}

PgeOldNewValue<int>& proofps_dd::Player::getFrags()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvFrags));
}

const PgeOldNewValue<int>& proofps_dd::Player::getFrags() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvFrags));
}

PgeOldNewValue<int>& proofps_dd::Player::getDeaths()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvDeaths));
}

const PgeOldNewValue<int>& proofps_dd::Player::getDeaths() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvDeaths));
}

bool proofps_dd::Player::canTakeItem(const MapItem& item) const
{
    switch (item.getType())
    {
    case proofps_dd::MapItemType::ITEM_WPN_PISTOL:
    case proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN:
    {
        const auto it = m_mapItemTypeToWeaponFilename.find(item.getType());
        if (it == m_mapItemTypeToWeaponFilename.end())
        {
            getConsole().EOLn(
                "Player::%s(): failed to find weapon by item type %d!", __func__, item.getType());
            return false;
        }
        const std::string& sWeaponName = it->second;
        const Weapon* const pWpn = m_wpnMgr.getWeaponByFilename(sWeaponName);
        if (!pWpn)
        {
            getConsole().EOLn(
                "Player::%s(): failed to find weapon by name %s for item type %d!", __func__, sWeaponName.c_str(), item.getType());
            return false;
        }
        return pWpn->canIncBulletCount();
    }
    case proofps_dd::MapItemType::ITEM_HEALTH:
        return (getHealth() < 100);
    default:
        ;
    }
    return false;
}

void proofps_dd::Player::TakeItem(MapItem& item, pge_network::PgePacket& pktWpnUpdate)
{
    // invoked only on server
    switch (item.getType())
    {
    case proofps_dd::MapItemType::ITEM_WPN_PISTOL:
    case proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN:
    {
        const auto it = m_mapItemTypeToWeaponFilename.find(item.getType());
        if (it == m_mapItemTypeToWeaponFilename.end())
        {
            getConsole().EOLn(
                "Player::%s(): failed to find weapon by item type %d!", __func__, item.getType());
            return;
        }
        const std::string& sWeaponBecomingAvailable = it->second;
        Weapon* const pWpnBecomingAvailable = m_wpnMgr.getWeaponByFilename(sWeaponBecomingAvailable);
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
        proofps_dd::MsgWpnUpdateFromServer::initPkt(
            pktWpnUpdate,
            0 /* ignored by client anyway */,
            sWeaponBecomingAvailable,
            pWpnBecomingAvailable->isAvailable(),
            pWpnBecomingAvailable->getMagBulletCount(),
            pWpnBecomingAvailable->getUnmagBulletCount());  // becomes available on client side (after pkt being sent)
        break;
    }
    case proofps_dd::MapItemType::ITEM_HEALTH:
        item.Take();
        SetHealth(getHealth() + static_cast<int>(MapItem::ITEM_HEALTH_HP_INC));
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
const std::map<proofps_dd::MapItemType, std::string> proofps_dd::Player::m_mapItemTypeToWeaponFilename =
{
    {proofps_dd::MapItemType::ITEM_WPN_PISTOL, "pistol.txt"},
    {proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, "machinegun.txt"}
};

uint32_t proofps_dd::Player::m_nPlayerInstanceCntr = 0;

void proofps_dd::Player::BuildPlayerObject(bool blend) {
    m_pObj = m_gfx.getObject3DManager().createPlane(proofps_dd::GAME_PLAYER_W, proofps_dd::GAME_PLAYER_H);
    if (!m_pObj)
    {
        throw std::runtime_error("Failed to create object for new player!");
    }

    m_pObj->SetName(m_pObj->getName() + " (for proofps_dd::Player w connHandle " + std::to_string(static_cast<uint32_t>(m_connHandleServerSide)) + ")");
    m_pObj->SetDoubleSided(true);
    if (blend)
    {
        m_pObj->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE_MINUS_SRC_ALPHA);
    }
    m_pObj->SetLit(false);

    PureTexture* pTexPlayer = m_gfx.getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "giraffe1m.bmp").c_str());
    m_pObj->getMaterial().setTexture(pTexPlayer);
}


// #######################################################################
// #                           PLAYERHANDLING                            #
// #######################################################################


// ############################### PUBLIC ################################


proofps_dd::PlayerHandling::PlayerHandling(
    PGE& pge,
    proofps_dd::Durations& durations,
    proofps_dd::Maps& maps,
    proofps_dd::Sounds& sounds) :
    proofps_dd::Networking(pge, durations),
    proofps_dd::UserInterface(pge),
    m_pge(pge),
    m_maps(maps),
    m_sounds(sounds)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations, maps, sounds
    // But they can used in other functions.

    // Since this class is used to build up the PRooFPSddPGE class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. PRooFPSddPGE initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be extisting at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
    assert(!pge.isGameRunning());
}

CConsole& proofps_dd::PlayerHandling::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* proofps_dd::PlayerHandling::getLoggerModuleName()
{
    return "PlayerHandling";
}


// ############################## PROTECTED ##############################


static constexpr char* szWaitingToRespawn = "Waiting to respawn ...";

void proofps_dd::PlayerHandling::HandlePlayerDied(Player& player, PureObject3D& objXHair)
{
    player.Die(isMyConnection(player.getServerSideConnectionHandle()), m_pge.getNetwork().isServer());
    if (isMyConnection(player.getServerSideConnectionHandle()))
    {
        m_pge.getAudio().play(m_sounds.m_sndPlayerDie);
        objXHair.Hide();
        AddText(szWaitingToRespawn, 200, m_pge.getPure().getWindow().getClientHeight() / 2);
    }
}



void proofps_dd::PlayerHandling::HandlePlayerRespawned(Player& player, PureObject3D& objXHair)
{
    const Weapon* const wpnDefaultAvailable = player.getWeaponManager().getWeaponByFilename(player.getWeaponManager().getDefaultAvailableWeaponFilename());
    assert(wpnDefaultAvailable);  // cannot be null since it is already verified in handleUserSetupFromServer()
    player.Respawn(isMyConnection(player.getServerSideConnectionHandle()), *wpnDefaultAvailable, m_pge.getNetwork().isServer());

    if (isMyConnection(player.getServerSideConnectionHandle()))
    {
        objXHair.Show();
        // well, this won't work if clientHeight is being changed in the meantime, but anyway this supposed to be a temporal feature ...
        m_pge.getPure().getUImanager().RemoveText(
            szWaitingToRespawn, 200, m_pge.getPure().getWindow().getClientHeight() / 2, m_pge.getPure().getUImanager().getDefaultFontSize());
    }
}

void proofps_dd::PlayerHandling::ServerRespawnPlayer(Player& player, bool restartGame)
{
    // to respawn, we just need to set these values, because SendUserUpdates() will automatically send out changes to everyone
    player.getPos() = m_maps.getRandomSpawnpoint();
    player.SetHealth(100);
    player.getRespawnFlag() = true;
    if (restartGame)
    {
        player.getFrags() = 0;
        player.getDeaths() = 0;
    }
}


// ############################### PRIVATE ###############################
