/*
    ###################################################################################
    Player.cpp
    Player and PlayerHandling classes for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "Player.h"
#include "PRooFPS-dd-packet.h"


// #######################################################################
// #                               PLAYER                                #
// #######################################################################


// ############################### PUBLIC ################################


const char* proofps_dd::Player::getLoggerModuleName()
{
    return "Player";
}

void proofps_dd::Player::genUniqueUserName(
    char szNewUserName[proofps_dd::MsgUserNameChange::nUserNameBufferLength],
    const std::string& sNameFromConfig,
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers)
{
    std::string sNewPlayerName(sNameFromConfig);
    if (sNewPlayerName.empty())
    {
        sNewPlayerName = "Player";
    }
    sNewPlayerName = sNewPlayerName.substr(0, proofps_dd::MsgUserNameChange::nUserNameBufferLength - 1);

    // if everybody is connecting with the SAME name, the maximum number of unique player names is: 10 ^ nUniqueNumberWidthInName
    constexpr size_t nUniqueNumberWidthInName = 3; // used only if sInitialName collides with another name
    static_assert(nUniqueNumberWidthInName >= 2);
    static_assert(nUniqueNumberWidthInName < 5);

    bool bNameCollision = true;
    while (bNameCollision)
    {
        bNameCollision = false;
        for (const auto& player : m_mapPlayers)
        {
            bNameCollision = (player.second.getName() == sNewPlayerName);
            if (bNameCollision)
            {
                break;
            }
        }

        if (bNameCollision)
        {
            // if we are here, we are even allowed to be a bit invasive with the name ...
            sNewPlayerName = sNewPlayerName.substr(0, proofps_dd::MsgUserNameChange::nUserNameBufferLength - 1 - nUniqueNumberWidthInName) +
                std::to_string(static_cast<size_t>(std::pow(10, nUniqueNumberWidthInName - 1)) + (rand() % static_cast<size_t>(std::pow(10, nUniqueNumberWidthInName))));
            sNewPlayerName = sNewPlayerName.substr(0, proofps_dd::MsgUserNameChange::nUserNameBufferLength - 1);
        }
    };

    strncpy_s(szNewUserName, proofps_dd::MsgUserNameChange::nUserNameBufferLength, sNewPlayerName.c_str(), sNewPlayerName.length());
}


proofps_dd::Player::Player(
    PGEcfgProfiles& cfgProfiles,
    std::list<Bullet>& bullets,
    PR00FsUltimateRenderingEngine& gfx,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const std::string& sIpAddress) :
    m_connHandleServerSide(connHandle),
    m_sIpAddress(sIpAddress),
    m_bNetDirty(false),
    m_pObj(PGENULL),
    m_pTexPlayerStand(PGENULL),
    m_pTexPlayerCrouch(PGENULL),
    m_wpnMgr(cfgProfiles, gfx, bullets),
    m_cfgProfiles(cfgProfiles),
    m_bullets(bullets),
    m_gfx(gfx),
    m_fGravity(0.f),
    m_bJumping(false),
    b_mCanFall(true),
    m_bFalling(true),
    m_bHasJustStartedFallingNaturally(true),
    m_bHasJustStartedFallingAfterJumpingStopped(false),
    m_fHeightStartedFalling(0.f),
    m_bHasJustStoppedJumping(false),
    m_bCrouchingStateCurrent(false),
    m_bWantToStandup(true),
    m_bRunning(true),
    m_bAllowJump(false),
    m_bWillJump(false),
    m_bExpectingStartPos(true),
    m_strafe(proofps_dd::Strafe::NONE),
    m_bAttack(false),
    m_bRespawn(false)
{
    BuildPlayerObject(true);
}

proofps_dd::Player::Player(const proofps_dd::Player& other) :
    m_connHandleServerSide(other.m_connHandleServerSide),
    m_sIpAddress(other.m_sIpAddress),
    m_sName(other.m_sName),
    m_vecOldNewValues(other.m_vecOldNewValues),
    m_bNetDirty(other.m_bNetDirty),
    m_vecJumpForce(other.m_vecJumpForce),
    m_vecImpactForce(other.m_vecImpactForce),
    m_pObj(PGENULL),
    m_pTexPlayerStand(PGENULL),
    m_pTexPlayerCrouch(PGENULL),
    m_wpnMgr(other.m_cfgProfiles, other.m_gfx, other.m_bullets),
    m_cfgProfiles(other.m_cfgProfiles),
    m_bullets(other.m_bullets),
    m_gfx(other.m_gfx),
    m_fGravity(other.m_fGravity),
    m_bJumping(other.m_bJumping),
    b_mCanFall(other.b_mCanFall),
    m_bFalling(other.m_bFalling),
    m_bHasJustStartedFallingNaturally(other.m_bHasJustStartedFallingNaturally),
    m_bHasJustStartedFallingAfterJumpingStopped(other.m_bHasJustStartedFallingAfterJumpingStopped),
    m_fHeightStartedFalling(other.m_fHeightStartedFalling),
    m_bHasJustStoppedJumping(other.m_bHasJustStoppedJumping),
    m_bCrouchingStateCurrent(other.m_bCrouchingStateCurrent),
    m_bWantToStandup(other.m_bWantToStandup),
    m_bRunning(other.m_bRunning),
    m_bAllowJump(other.m_bAllowJump),
    m_bWillJump(other.m_bWillJump),
    m_bExpectingStartPos(other.m_bExpectingStartPos),
    m_strafe(other.m_strafe),
    m_bAttack(other.m_bAttack),
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
    m_bNetDirty = other.m_bNetDirty;
    m_vecJumpForce = other.m_vecJumpForce;
    m_vecImpactForce = other.m_vecImpactForce;
    m_bullets = other.m_bullets;
    m_gfx = other.m_gfx;
    m_fGravity = other.m_fGravity;
    m_bJumping = other.m_bJumping;
    b_mCanFall = other.b_mCanFall;
    m_bFalling = other.m_bFalling;
    m_bHasJustStartedFallingNaturally = other.m_bHasJustStartedFallingNaturally;
    m_bHasJustStartedFallingAfterJumpingStopped = other.m_bHasJustStartedFallingAfterJumpingStopped;
    m_fHeightStartedFalling = other.m_fHeightStartedFalling;
    m_bHasJustStoppedJumping = other.m_bHasJustStoppedJumping;
    m_bCrouchingStateCurrent = other.m_bCrouchingStateCurrent;
    m_bWantToStandup = other.m_bWantToStandup;
    m_bRunning = other.m_bRunning;
    m_bAllowJump = other.m_bAllowJump;
    m_bWillJump = other.m_bWillJump;
    m_bExpectingStartPos = other.m_bExpectingStartPos;
    m_strafe = other.m_strafe;
    m_bAttack = other.m_bAttack;
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

    // Dont telete player textures here because other Player instances refer to the same textures.
    // Unfortunately the engine still doesn't use reference counting for textures.
    // Anyway, the engine takes care of deleting textures at shutdown.

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

/**
 * Check if there is any old-new value pending to be committed.
 * @return True if there is something to be committed, false otherwise.
 */
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

/**
 * Invokes commit() for all maintained old-new values.
 * Also sets the isNetDirty() flag if there was any dirty old-new value.
 * The idea is that the game should invoke this function in every tick/physics iteration, and the game
 * should later check the isNetDirty() flag to decide if it should send out updates to clients or not.
 */
void proofps_dd::Player::updateOldValues()
{
    for (auto& enumVariantPair : m_vecOldNewValues)
    {
        // this is the "1. void visitor" from example here: https://en.cppreference.com/w/cpp/utility/variant/visit
        const bool bDirty = std::visit([](auto&& oldNewValue) -> bool {
                const bool bRet = oldNewValue.isDirty();
                oldNewValue.commit();
                return bRet;
            },
            enumVariantPair.second);
        if (bDirty)
        {
            m_bNetDirty = true;
        }
    }
}

/**
 * Check if there was any old-new value updated in recent updateOldValues() call(s) since the last call to clearNetDirty().
 * The idea is that the game should invoke updateOldValues() in every tick/physics iteration, and the game
 * should later check the isNetDirty() flag to decide if it should send out updates to clients or not.
 * Thus the isNetDirty() flag cannot be set by the game itself, it is always set by the updateOldValues(), and the game
 * is expected to clear it using clearNetDirty() after sending out updates to clients.
 * 
 * @return True if there is something to be sent to clients, false otherwise.
 */
bool proofps_dd::Player::isNetDirty() const
{
    return m_bNetDirty;
}

/**
 * Clears the isNetDirty() flag.
 * The idea is that this should be called by the game only after sending out updates to clients.
 */
void proofps_dd::Player::clearNetDirty()
{
    m_bNetDirty = false;
}

CConsole& proofps_dd::Player::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
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

const PgeOldNewValue<PureVector>& proofps_dd::Player::getPos() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<PureVector>>(m_vecOldNewValues.at(OldNewValueName::OvPos));
}

PgeOldNewValue<TPureFloat>& proofps_dd::Player::getAngleY()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<TPureFloat>>(m_vecOldNewValues.at(OldNewValueName::OvAngleY));
}

const PgeOldNewValue<TPureFloat>& proofps_dd::Player::getAngleY() const
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

bool proofps_dd::Player::canFall() const
{
    return b_mCanFall;
}

bool proofps_dd::Player::getHasJustStartedFallingNaturallyInThisTick() const
{
    return m_bHasJustStartedFallingNaturally;
}

void proofps_dd::Player::setHasJustStartedFallingNaturallyInThisTick(bool val)
{
    m_bHasJustStartedFallingNaturally = val;
    if (val)
    {
        m_timeStartedFalling = std::chrono::steady_clock::now();
        m_fHeightStartedFalling = getPos().getNew().getY();
        m_bFalling = true;
    }
}

bool proofps_dd::Player::getHasJustStartedFallingAfterJumpingStoppedInThisTick() const
{
    return m_bHasJustStartedFallingAfterJumpingStopped;
}

void proofps_dd::Player::setHasJustStartedFallingAfterJumpingStoppedInThisTick(bool val)
{
    m_bHasJustStartedFallingAfterJumpingStopped = val;
    if (val)
    {
        m_timeStartedFalling = std::chrono::steady_clock::now();
        m_fHeightStartedFalling = getPos().getNew().getY();
        m_bFalling = true;
    }
}

bool proofps_dd::Player::isFalling() const
{
    return m_bFalling;
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::Player::getTimeStartedFalling() const
{
    return m_timeStartedFalling;
}

const float proofps_dd::Player::getHeightStartedFalling() const
{
    return m_fHeightStartedFalling;
}

bool& proofps_dd::Player::getHasJustStoppedJumpingInThisTick()
{
    return m_bHasJustStoppedJumping;
}

void proofps_dd::Player::SetHealth(int value) {
    getHealth().set(std::max(0, std::min(value, 100)));
}

void proofps_dd::Player::SetGravity(float value) {
    m_fGravity = value;
    if (value >= 0.f)
    {
        m_bFalling = false;
    }
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
    m_bFalling = false;
    m_fGravity = getCrouchInput().getNew() ? proofps_dd::GAME_JUMP_GRAVITY_START_FROM_CROUCHING : proofps_dd::GAME_JUMP_GRAVITY_START_FROM_STANDING;
    m_vecJumpForce.SetX(getPos().getNew().getX() - getPos().getOld().getX());
    m_vecJumpForce.SetY(getPos().getNew().getY() - getPos().getOld().getY());
    m_vecJumpForce.SetZ(getPos().getNew().getZ() - getPos().getOld().getZ());
    //getConsole().EOLn("jump x force: %f", m_vecJumpForce.getX());
}

void proofps_dd::Player::StopJumping() {
    m_bJumping = false;
}

bool proofps_dd::Player::getWillJumpInNextTick() const
{
    return m_bWillJump;
}

void proofps_dd::Player::setWillJumpInNextTick(bool flag)
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

bool& proofps_dd::Player::getAttack()
{
    return m_bAttack;
}

bool proofps_dd::Player::attack()
{
    if ((getHealth() <= 0) || !getAttack())
    {
        return false;
    }

    Weapon* const wpn = getWeaponManager().getCurrentWeapon();
    if (!wpn)
    {
        return false;
    }

    return wpn->pullTrigger();
}

PureVector& proofps_dd::Player::getJumpForce()
{
    return m_vecJumpForce;
}

PureVector& proofps_dd::Player::getImpactForce()
{
    return m_vecImpactForce;
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

PgeOldNewValue<bool>& proofps_dd::Player::getCrouchInput()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<bool>>(m_vecOldNewValues.at(OldNewValueName::OvCrouch));
}

bool& proofps_dd::Player::getCrouchStateCurrent()
{
    return m_bCrouchingStateCurrent;
}

bool& proofps_dd::Player::getWantToStandup()
{
    return m_bWantToStandup;
}

/**
* Changing player properties for physics.
* Invokes getPos().commit() too so not be called careless from anywhere.
* Should be invoked only by server physics at the beginning of physics calculations.
* 
* @param bPullUpLegs True will result in the top Y position of the object won't change thus it will shrink upwards.
*                    False will result in both the top and bottom Y positions of the object will change thus it will shrink towards center.
*/
void proofps_dd::Player::DoCrouchServer(bool bPullUpLegs)
{
    // Scaling change is legal since player object will be smaller thus no unexpected collision can happen;
    // position change is also legal since player area stays within previous standing area.
    // We need to set Object3D scaling since that is used in physics calculations also in serverPlayerCollisionWithWalls(),
    // but we dont need to set Object3D position because Player object has its own position vector that is used in physics.
    // On the long run we should use colliders so physics does not depend on graphics.
    getObject3D()->SetScaling(PureVector(1.f, GAME_PLAYER_H_CROUCH_SCALING_Y, 1.f));
    getCrouchStateCurrent() = true;  // can always go to crouching immediately
    if (bPullUpLegs)
    {
        // reposition is allowed only if being in the air: pulling up the legs, so the head supposed to stay in same position as before,
        // however we don't reposition if being on ground because it looks bad
        // PPPKKKGGGGGG
        PureVector playerPos = getPos().getNew();
        playerPos.SetY(playerPos.getY() + GAME_PLAYER_H_STAND / 2.f - (GAME_PLAYER_H_STAND * GAME_PLAYER_H_CROUCH_SCALING_Y) / 2.f);
        getPos().set(playerPos);
        // since we are at the beginning of a tick, it is legal to commit the position now, as old and new positions supposed to be the same at this point
        getPos().commit();
    }
}

void proofps_dd::Player::DoCrouchShared()
{
    getObject3D()->SetScaling(PureVector(1.f, GAME_PLAYER_H_CROUCH_SCALING_Y, 1.f));
    getObject3D()->getMaterial().setTexture(m_pTexPlayerCrouch);
    getCrouchStateCurrent() = true; // since this is replicated from server, it is valid
    // getWantToStandup() stays updated on server-side only, in handleInputWhenConnectedAndSendUserCmdMove(), do not modify anywhere else!
    //getWantToStandup() = false;
}

/**
* Changing player properties for physics.
* Invokes getPos().set() too so not be called careless from anywhere.
* Should be invoked only by server physics during physics calculations.
*
* @param fNewPosY The new Y position for the player.
*/
void proofps_dd::Player::DoStandupServer(const float& fNewPosY)
{
    getCrouchStateCurrent() = false;
    getObject3D()->SetScaling(PureVector(1.f, 1.f, 1.f));
    // reposition so the legs will stay at the same position as we stand up, so we are essentially growing up from the ground
    getPos().set(
        PureVector(
            getPos().getNew().getX(),
            fNewPosY,
            getPos().getNew().getZ()
        ));

    // WA: This line is only needed to get rid of the phenomenon on server side: for 1 frame, player object will overlap the ground object below it.
    // This is because after physics iteration there will be 1 frame rendered, and only at the beginning of next frame the server will
    // process the handleUserUpdateFromServer() where it repositions the player object.
    // So this line here is just a workaround to get rid of it.
    // Clients don't see this phenomenon since they dont run this physics, just get position updates from server.
    // The only way to fix this is to introduce colliders so we dont mess with graphical entities in physics.
    // A bug ticket has been opened for this: https://github.com/proof88/PRooFPS-dd/issues/265.
    getObject3D()->getPosVec().SetY(getPos().getNew().getY());
}

void proofps_dd::Player::DoStandupShared()
{
    getObject3D()->SetScaling(PureVector(1.f, 1.f, 1.f));
    getObject3D()->getMaterial().setTexture(m_pTexPlayerStand);
    getCrouchStateCurrent() = false;  // since this is replicated from server, it is valid
    // getWantToStandup() stays updated on server-side only, in handleInputWhenConnectedAndSendUserCmdMove(), do not modify anywhere else!
    //getWantToStandup() = true;
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
    getAttack() = false;
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
    DoStandupShared();
    getWantToStandup() = true;
    getImpactForce().SetZero();

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
    case proofps_dd::MapItemType::ITEM_WPN_BAZOOKA:
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
    case proofps_dd::MapItemType::ITEM_WPN_BAZOOKA:
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
    {proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN, "machinegun.txt"},
    {proofps_dd::MapItemType::ITEM_WPN_BAZOOKA, "bazooka.txt"},
};

uint32_t proofps_dd::Player::m_nPlayerInstanceCntr = 0;

void proofps_dd::Player::BuildPlayerObject(bool blend) {
    m_pObj = m_gfx.getObject3DManager().createPlane(proofps_dd::GAME_PLAYER_W, proofps_dd::GAME_PLAYER_H_STAND);
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
    
    m_pTexPlayerStand = m_gfx.getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "giraffe1m.bmp").c_str());
    m_pTexPlayerCrouch = m_gfx.getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "giraffe_crouch.bmp").c_str());
    m_pObj->getMaterial().setTexture(m_pTexPlayerStand);
}


// #######################################################################
// #                           PLAYERHANDLING                            #
// #######################################################################


// ############################### PUBLIC ################################


proofps_dd::PlayerHandling::PlayerHandling(
    PGE& pge,
    proofps_dd::Durations& durations,
    proofps_dd::GUI& gui,
    std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
    proofps_dd::Maps& maps,
    proofps_dd::Sounds& sounds) :
    proofps_dd::Networking(pge, durations),
    m_pge(pge),
    m_gui(gui),
    m_mapPlayers(mapPlayers),
    m_maps(maps),
    m_sounds(sounds)
{
    // note that the following should not be touched here as they are not fully constructed when we are here:
    // pge, durations, gui, maps, sounds
    // But they can used in other functions.

    // Since this class is used to build up the PRooFPSddPGE class which is derived from PGE class, PGE is not yet initialized
    // when this ctor is invoked. PRooFPSddPGE initializes PGE later. Furthermore, even the pimpl object inside PGE might not
    // be existing at this point, only isGameRunning() is safe to call. The following assertion is reminding me of that:
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

void proofps_dd::PlayerHandling::HandlePlayerDied(
    Player& player,
    PureObject3D& objXHair,
    pge_network::PgeNetworkConnectionHandle nKillerConnHandleServerSide)
{
    player.Die(isMyConnection(player.getServerSideConnectionHandle()), m_pge.getNetwork().isServer());
    if (isMyConnection(player.getServerSideConnectionHandle()))
    {
        m_pge.getAudio().play(m_sounds.m_sndPlayerDie);
        objXHair.Hide();
        m_gui.textPermanent(szWaitingToRespawn, 200, m_pge.getPure().getWindow().getClientHeight() / 2);
    }

    if (m_pge.getNetwork().isServer())
    {
        // TODO: server should display death notification on gui here (client should display in handleDeathNotificationFromServer())

        // important: killer info is just for informational purpose so we can display it on client side, however
        // if the killer got disconnected already, conn handle will be set as same as player's connhandle, so we can still
        // show the player dieing without a killer, however it is not suicide in such case, that is why we should never
        // decrease frags based only on this value!
        pge_network::PgePacket pktDeathNotificationFromServer;
        proofps_dd::MsgDeathNotificationFromServer::initPkt(
            pktDeathNotificationFromServer,
            player.getServerSideConnectionHandle(),
            nKillerConnHandleServerSide);
        m_pge.getNetwork().getServer().sendToAllClientsExcept(pktDeathNotificationFromServer);
    }
}

void proofps_dd::PlayerHandling::HandlePlayerRespawned(Player& player, PureObject3D& objXHair)
{
    const Weapon* const wpnDefaultAvailable = player.getWeaponManager().getWeaponByFilename(
        player.getWeaponManager().getDefaultAvailableWeaponFilename());
    assert(wpnDefaultAvailable);  // cannot be null since it is already verified in handleUserSetupFromServer()
    player.Respawn(isMyConnection(player.getServerSideConnectionHandle()), *wpnDefaultAvailable, m_pge.getNetwork().isServer());

    if (isMyConnection(player.getServerSideConnectionHandle()))
    {
        // camera must be repositioned immediately so players can see themselves ASAP
        auto& camera = m_pge.getPure().getCamera();
        camera.getPosVec().SetX(player.getObject3D()->getPosVec().getX());
        camera.getPosVec().SetY(player.getObject3D()->getPosVec().getY());
        camera.getTargetVec().SetX(camera.getPosVec().getX());
        camera.getTargetVec().SetY(camera.getPosVec().getY());

        objXHair.Show();
        // well, this won't work if clientHeight is being changed in the meantime, but anyway this supposed to be a temporal feature ...
        m_pge.getPure().getUImanager().removeTextPermanentLegacy(
            szWaitingToRespawn, 200, m_pge.getPure().getWindow().getClientHeight() / 2, m_pge.getPure().getUImanager().getDefaultFontSizeLegacy());
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

void proofps_dd::PlayerHandling::updatePlayersOldValues()
{
    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;
        // From v0.1.5 clients invoke updateOldValues() in handleUserUpdateFromServer() thus they are also good to use old vs new values and isDirty().
        // Server invokes it here in every physics iteration. 2 reasons:
        // - consecutive physics iterations require old and new values to be properly set;
        // - player.isNetDirty() thus serverSendUserUpdates() rely on player.updateOldValues().
        player.updateOldValues();
    }
}

bool proofps_dd::PlayerHandling::handleDeathNotificationFromServer(pge_network::PgeNetworkConnectionHandle nDeadConnHandleServerSide, const proofps_dd::MsgDeathNotificationFromServer& msg)
{
    if (m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("PlayerHandling::%s(): server received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    //getConsole().EOLn("PlayerHandling::%s(): player %u killed by %u",  __func__, nDeadConnHandleServerSide, msg.m_nKillerConnHandleServerSide);

    const auto itPlayerDied = m_mapPlayers.find(nDeadConnHandleServerSide);
    if (m_mapPlayers.end() == itPlayerDied)
    {
        getConsole().EOLn("PlayerHandling::%s(): failed to find dead with connHandleServerSide: %u!", __func__, nDeadConnHandleServerSide);
        assert(false);
        return false; // crash in both debug and release mode: if someone dies on server side, must still exist at that moment in every network instance
    }

    const auto itPlayerKiller = m_mapPlayers.find(msg.m_nKillerConnHandleServerSide);
    if (m_mapPlayers.end() == itPlayerKiller)
    {
        getConsole().EOLn("PlayerHandling::%s(): failed to find killer with connHandleServerSide: %u!", __func__, msg.m_nKillerConnHandleServerSide);
        assert(false); // crash in debug, ignore in release mode: a bullet killing someone might be shot by a killer already disconnected before the impact,
                       // however this is not likely to happen in debug mode when I'm testing regular gameplay!
    }

    // Server does death notification on GUI in HandlePlayerDied(), clients do here.
    // TODO: add notification to GUI!

    return true;
}


// ############################### PRIVATE ###############################
