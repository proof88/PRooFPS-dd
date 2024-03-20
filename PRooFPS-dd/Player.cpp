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
    m_bCanFall(true),
    m_bFalling(true),
    m_bHasJustStartedFallingNaturally(true),
    m_bHasJustStartedFallingAfterJumpingStopped(false),
    m_fHeightStartedFalling(0.f),
    m_bHasJustStoppedJumping(false),
    m_bCrouchingStateCurrent(false),
    m_bCrouchingWasActiveWhenInitiatedJump(false),
    m_bWantToStandup(true),
    m_fSomersaultAngleZ(0.f),
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
    m_bCanFall(other.m_bCanFall),
    m_bFalling(other.m_bFalling),
    m_bHasJustStartedFallingNaturally(other.m_bHasJustStartedFallingNaturally),
    m_bHasJustStartedFallingAfterJumpingStopped(other.m_bHasJustStartedFallingAfterJumpingStopped),
    m_fHeightStartedFalling(other.m_fHeightStartedFalling),
    m_bHasJustStoppedJumping(other.m_bHasJustStoppedJumping),
    m_bCrouchingStateCurrent(other.m_bCrouchingStateCurrent),
    m_bCrouchingWasActiveWhenInitiatedJump(other.m_bCrouchingWasActiveWhenInitiatedJump),
    m_bWantToStandup(other.m_bWantToStandup),
    m_fSomersaultAngleZ(other.m_fSomersaultAngleZ),
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
    m_bCanFall = other.m_bCanFall;
    m_bFalling = other.m_bFalling;
    m_bHasJustStartedFallingNaturally = other.m_bHasJustStartedFallingNaturally;
    m_bHasJustStartedFallingAfterJumpingStopped = other.m_bHasJustStartedFallingAfterJumpingStopped;
    m_fHeightStartedFalling = other.m_fHeightStartedFalling;
    m_bHasJustStoppedJumping = other.m_bHasJustStoppedJumping;
    m_bCrouchingStateCurrent = other.m_bCrouchingStateCurrent;
    m_bCrouchingWasActiveWhenInitiatedJump = other.m_bCrouchingWasActiveWhenInitiatedJump;
    m_bWantToStandup = other.m_bWantToStandup;
    m_fSomersaultAngleZ = other.m_fSomersaultAngleZ;
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

PgeOldNewValue<TPureFloat>& proofps_dd::Player::getAngleZ()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<TPureFloat>>(m_vecOldNewValues.at(OldNewValueName::OvAngleZ));
}

const PgeOldNewValue<TPureFloat>& proofps_dd::Player::getAngleZ() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<TPureFloat>>(m_vecOldNewValues.at(OldNewValueName::OvAngleZ));
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
    return m_bCanFall;
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

/**
 * Somersault aka front-/backflip.
 * The idea is this function sets an initial positive or negative value for the angle based on strafe direction, and then
 * Physics class will take care of the full somersaulting by periodic calls to stepSomersaultAngle().
 */
void proofps_dd::Player::startSomersault()
{
    // sanity check
    if (isSomersaulting() || !isJumping() || isJumpingInitiatedFromCrouching())
    {
        return;
    }

    if (!getCrouchStateCurrent())
    {
        if (m_cfgProfiles.getVars()[CVAR_SV_SOMERSAULT_MID_AIR_AUTO_CROUCH].getAsBool())
        {
            DoCrouchServer(true /* pull up legs because we are jumping */);
        }
        else
        {
            return;
        }
    }

    // just set the initial direction by setting a small value, so stepSomersaultAngle() will know in which direction it should change angle
    switch (m_strafe)
    {
    case Strafe::LEFT:
        m_fSomersaultAngleZ = (getObject3D()->getAngleVec().getY() == 0.f) ? 0.1f : -0.1f;
        break;
    case Strafe::RIGHT:
        m_fSomersaultAngleZ = (getObject3D()->getAngleVec().getY() == 0.f) ? -0.1f : 0.1f;
        break;
    default /* Strafe::NONE */:
        m_fSomersaultAngleZ = 0.1f;
    }
    getAngleZ() = m_fSomersaultAngleZ;

    m_vecJumpForce.SetX( m_vecJumpForce.getX() * 2 );
    m_fGravity *= 2;
}

bool proofps_dd::Player::isSomersaulting() const
{
    return m_fSomersaultAngleZ != 0.f;
}

float proofps_dd::Player::getSomersaultAngle() const
{
    return m_fSomersaultAngleZ;
}

/**
 * Stepping the somersault angle i.e. changing the angle in the proper direction.
 * The idea is that we set the initial angle by a call to startSomersault() and then in consecutive physics timesteps
 * we call stepSomersaultAngle() with a value calculated by the physics engine.
 * Somersaulting will stop once stepSomersaultAngle() finishes a complete roll/flip.
 * This also means that currently only 1 full roll/flip is done before isSomersaulting() returns false again.
 */
void proofps_dd::Player::stepSomersaultAngle(float angle)
{
    if (m_fSomersaultAngleZ >= 0.f)
    {
        m_fSomersaultAngleZ += angle;
        if (m_fSomersaultAngleZ >= 360.f)
        {
            m_fSomersaultAngleZ = 0.f;
        }
    }
    else
    {
        m_fSomersaultAngleZ -= angle;
        if (m_fSomersaultAngleZ <= -360.f)
        {
            m_fSomersaultAngleZ = 0.f;
        }
    }

    getObject3D()->getAngleVec().SetZ( m_fSomersaultAngleZ );
    getAngleZ() = m_fSomersaultAngleZ;
    
    // during somersaulting wpn angle is not freely modifiable, it strictly follows player angle!
    getWeaponAngle().set( PureVector(0.f, getObject3D()->getAngleVec().getY(), m_fSomersaultAngleZ) );
    Weapon* const wpn = getWeaponManager().getCurrentWeapon();
    if (wpn)
    {
        wpn->getObject3D().getAngleVec().Set(0.f, getObject3D()->getAngleVec().getY(), m_fSomersaultAngleZ);
    }
    
}

void proofps_dd::Player::resetSomersault()
{
    m_fSomersaultAngleZ = 0.f;
    getAngleZ() = 0.f;
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
    m_bCrouchingWasActiveWhenInitiatedJump = getCrouchInput().getNew();
    m_fGravity = m_bCrouchingWasActiveWhenInitiatedJump ? proofps_dd::GAME_JUMP_GRAVITY_START_FROM_CROUCHING : proofps_dd::GAME_JUMP_GRAVITY_START_FROM_STANDING;

    m_vecJumpForce.SetX(getPos().getNew().getX() - getPos().getOld().getX());
    // we dont use other components of jumpForce vec, since Z-axis is "unused", Y-axis jump force is controlled by m_fGravity 
    //m_vecJumpForce.SetY(getPos().getNew().getY() - getPos().getOld().getY());
    //m_vecJumpForce.SetZ(getPos().getNew().getZ() - getPos().getOld().getZ());
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
    m_bCanFall = state;
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
    return std::get<PgeOldNewValue<bool>>(m_vecOldNewValues.at(OldNewValueName::OvCrouchInput));
}

bool& proofps_dd::Player::getCrouchStateCurrent()
{
    return m_bCrouchingStateCurrent;
}

const bool& proofps_dd::Player::isJumpingInitiatedFromCrouching() const
{
    return m_bCrouchingWasActiveWhenInitiatedJump;
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
* @param bPullUpLegs True will result in the top Y position of the object won't change thus it will shrink upwards (should be used mid-air).
*                    False will result in both the top and bottom Y positions of the object will change thus it will shrink towards center (should be used on ground).
*/
void proofps_dd::Player::DoCrouchServer(bool bPullUpLegs)
{
    if (getCrouchStateCurrent())
    {
        return;
    }

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
        setStrafe(Strafe::NONE);
        getJumpForce().SetZero();
        resetSomersault();

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
    m_sounds(sounds),
    m_nSendClientUpdatesInEveryNthTick(1),
    m_nSendClientUpdatesCntr(m_nSendClientUpdatesInEveryNthTick)
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

void proofps_dd::PlayerHandling::serverUpdateRespawnTimers(
    proofps_dd::GameMode& gameMode,
    proofps_dd::Durations& durations)
{
    if (gameMode.checkWinningConditions())
    {
        return;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

    for (auto& playerPair : m_mapPlayers)
    {
        if (playerPair.second.getHealth() > 0)
        {
            continue;
        }

        const long long timeDiffSeconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - playerPair.second.getTimeDied()).count();
        if (timeDiffSeconds >= proofps_dd::GAME_PLAYER_RESPAWN_SECONDS)
        {
            ServerRespawnPlayer(playerPair.second, false);
        }
    }

    durations.m_nUpdateRespawnTimersDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
} // serverUpdateRespawnTimers()

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

void proofps_dd::PlayerHandling::WritePlayerList()
{
    getConsole().OLnOI("PRooFPSddPGE::%s()", __func__);
    for (const auto& playerPair : m_mapPlayers)
    {
        getConsole().OLn("Username: %s; connHandleServerSide: %u; address: %s",
            playerPair.second.getName().c_str(), playerPair.second.getServerSideConnectionHandle(), playerPair.second.getIpAddress().c_str());
    }
    getConsole().OO();
}

bool proofps_dd::PlayerHandling::handleUserConnected(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const pge_network::MsgUserConnectedServerSelf& msg,
    PGEcfgProfiles& cfgProfiles,
    std::function<void(int)>& cbDisplayMapLoadingProgressUpdate)
{
    if (!m_pge.getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): client received, CANNOT HAPPEN!", __func__);
        assert(false);
        return false;
    }

    pge_network::PgePacket newPktUserUpdate;

    if (msg.m_bCurrentClient)
    {
        // server is processing its own birth
        if (m_mapPlayers.size() == 0)
        {
            // server already loads the map for itself at this point
            if (m_maps.getFilename() != m_maps.getNextMapToBeLoaded())
            {
                // if we fall here with non-empty m_maps.getFilename(), it is an error, and m_maps.load() will fail as expected.
                if (!m_maps.load(m_maps.getNextMapToBeLoaded().c_str(), cbDisplayMapLoadingProgressUpdate))
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): m_maps.load() failed: %s!", __func__, m_maps.getNextMapToBeLoaded().c_str());
                    assert(false);
                    return false;
                }
            }
            else
            {
                getConsole().OLn("PRooFPSddPGE::%s(): map %s already loaded", __func__, m_maps.getNextMapToBeLoaded().c_str());
            }

            getConsole().OLn("PRooFPSddPGE::%s(): first (local) user connected and I'm server, so this is me (connHandleServerSide: %u)",
                __func__, connHandleServerSide);

            pge_network::PgePacket newPktSetup;
            if (proofps_dd::MsgUserSetupFromServer::initPkt(newPktSetup, connHandleServerSide, true, msg.m_szIpAddress, m_maps.getNextMapToBeLoaded().c_str()))
            {
                const PureVector& vecStartPos = cfgProfiles.getVars()["testing"].getAsBool() ?
                    m_maps.getLeftMostSpawnpoint() :
                    m_maps.getRandomSpawnpoint();

                if (proofps_dd::MsgUserUpdateFromServer::initPkt(
                    newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, 0.f, false, 100, false, 0, 0))
                {
                    // server injects this msg to self so resources for player will be allocated
                    m_pge.getNetwork().getServer().send(newPktSetup);
                    m_pge.getNetwork().getServer().send(newPktUserUpdate);
                }
                else
                {
                    getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                    assert(false);
                    return false;
                }
            }
            else
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
                return false;
            }
        }
        else
        {
            // cannot happen
            getConsole().EOLn("PRooFPSddPGE::%s(): user (connHandleServerSide: %u) connected with bCurrentClient as true but it is not me, CANNOT HAPPEN!",
                __func__, connHandleServerSide);
            assert(false);
            return false;
        }
    }
    else
    {
        if (connHandleServerSide == pge_network::ServerConnHandle)
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): server user (connHandleServerSide: %u) connected but map m_bCurrentClient is false, CANNOT HAPPEN!",
                __func__, connHandleServerSide);
            assert(false);
            return false;
        }
        // server is processing another user's birth
        getConsole().OLn("PRooFPSddPGE::%s(): new remote user (connHandleServerSide: %u) connected (from %s) and I'm server",
            __func__, connHandleServerSide, msg.m_szIpAddress);

        pge_network::PgePacket newPktSetup;
        if (!proofps_dd::MsgUserSetupFromServer::initPkt(newPktSetup, connHandleServerSide, false, msg.m_szIpAddress, m_maps.getFilename()))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }

        const PureVector& vecStartPos = cfgProfiles.getVars()["testing"].getAsBool() ?
            m_maps.getRightMostSpawnpoint() :
            m_maps.getRandomSpawnpoint();

        if (!proofps_dd::MsgUserUpdateFromServer::initPkt(
            newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, 0.f, false, 100, false, 0, 0))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }

        // server injects this msg to self so resources for player will be allocated
        m_pge.getNetwork().getServer().send(newPktSetup);
        m_pge.getNetwork().getServer().send(newPktUserUpdate);

        // inform all other clients about this new user
        m_pge.getNetwork().getServer().sendToAllClientsExcept(newPktSetup, connHandleServerSide);
        m_pge.getNetwork().getServer().sendToAllClientsExcept(newPktUserUpdate, connHandleServerSide);

        // now we send this msg to the client with this bool flag set so client will know it is their connect
        proofps_dd::MsgUserSetupFromServer& msgUserSetup = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserSetupFromServer>(newPktSetup);
        msgUserSetup.m_bCurrentClient = true;
        m_pge.getNetwork().getServer().send(newPktSetup, connHandleServerSide);
        m_pge.getNetwork().getServer().send(newPktUserUpdate);   // TODO: why is this here? we already sent it few lines earlier.
    }

    return true;
}  // handleUserConnected()

bool proofps_dd::PlayerHandling::handleUserDisconnected(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const pge_network::MsgUserDisconnectedFromServer&,
    proofps_dd::GameMode& gameMode)
{
    const auto playerIt = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == playerIt)
    {
        // TEMPORARILY COMMENTED DUE TO: https://github.com/proof88/PRooFPS-dd/issues/261
        // When we are trying to join a server but we get bored and user presses ESCAPE, client's disconnect is invoked, which
        // actually starts disconnecting because it thinks we are connected to server, and injects this userDisconnected pkt.
        // 
        //getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        //assert(false); // in debug mode, try to understand this scenario
        return true; // in release mode, dont terminate
    }

    // Server should not remove all players when it it notified with its connHandle being disconnected, because in that case
    // all players are expected to be removed by subsequent calls into this function with their connHandle as their connection state transitions.
    // There will be userDisconnected message for all players, including the server as well, so eventually this way m_mapPlayers will be
    // cleared out in multiple steps on server-side.
    // However, client won't be notified about all clients disconnecting in this case, but it is quite trivial when the server itself disconnects.
    // So that is why we manually get rid of all players in case of client.
    // We need m_mapPlayers to be cleared out by the end of processing all disconnections, the reasion is explained in hasValidConnection().
    const bool bClientShouldRemoveAllPlayers = !m_pge.getNetwork().isServer() && (connHandleServerSide == pge_network::ServerConnHandle);
    if (m_pge.getNetwork().isServer())
    {
        getConsole().OLn(
            "PRooFPSddPGE::%s(): user %s with connHandleServerSide %u disconnected and I'm server",
            __func__, playerIt->second.getName().c_str(), connHandleServerSide);
    }
    else
    {
        getConsole().OLn(
            "PRooFPSddPGE::%s(): user %s with connHandleServerSide %u disconnected and I'm client",
            __func__, playerIt->second.getName().c_str(), connHandleServerSide);
    }

    gameMode.removePlayer(playerIt->second);
    m_mapPlayers.erase(playerIt);

    if (bClientShouldRemoveAllPlayers)
    {
        getConsole().OLn("PRooFPSddPGE::%s(): it was actually the server disconnected so I'm removing every player including myself", __func__);
        gameMode.restart();
        m_mapPlayers.clear();
    }

    m_pge.getNetwork().WriteList();
    WritePlayerList();

    return true;
}  // handleUserDisconnected()

bool proofps_dd::PlayerHandling::handleUserNameChange(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const proofps_dd::MsgUserNameChange& msg,
    proofps_dd::GameMode& gameMode,
    PGEcfgProfiles& cfgProfiles)
{
    // TODO: make sure received user name is properly null-terminated! someone else could had sent that, e.g. malicious client or server

    const auto playerIt = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == playerIt)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return true;
    }

    if (m_pge.getNetwork().isServer())
    {
        // sanity check: connHandle should be server's if bCurrentClient is set
        if ((connHandleServerSide == pge_network::ServerConnHandle) && (!msg.m_bCurrentClient))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): cannot happen: connHandleServerSide != pge_network::ServerConnHandle: %u != %u, programming error!",
                __func__, connHandleServerSide, pge_network::ServerConnHandle);
            assert(false);
            return false;
        }

        // this is a requested name so this is the place where we make sure name is unique!
        char szNewUserName[sizeof(msg.m_szUserName)];
        Player::genUniqueUserName(szNewUserName, msg.m_szUserName, m_mapPlayers);

        if (strncmp(szNewUserName, msg.m_szUserName, sizeof(msg.m_szUserName)) == 0)
        {
            getConsole().OLn("PRooFPSddPGE::%s(): name change request accepted for connHandleServerSide: %u, old name: %s, new name: %s!",
                __func__, connHandleServerSide, playerIt->second.getName().c_str(), szNewUserName);
        }
        else
        {
            getConsole().OLn("PRooFPSddPGE::%s(): name change request denied for connHandleServerSide: %u, old name: %s, requested: %s, new name: %s!",
                __func__, connHandleServerSide, playerIt->second.getName().c_str(), msg.m_szUserName, szNewUserName);
        }

        // server updates player's name first

        playerIt->second.setName(szNewUserName);
        // TODO: these commented lines below will be needed when we are allowing player name change WHILE already connected to the server
        // TODO: check if such name is already present in frag table, if so, then rename
        //if (!gameMode.renamePlayer(playerIt->second.getName().c_str(), szNewUserName))
        //{
        //    getConsole().EOLn("PRooFPSddPGE::%s(): gameMode.renamePlayer() FAILED!", __func__);
        //    assert(false);
        //    return false;
        //}
        if (!gameMode.addPlayer(playerIt->second))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): failed to insert player %s (%u) into GameMode!", __func__, szNewUserName, connHandleServerSide);
            assert(false);
            return false;
        }

        // then we let all clients except this one know about the name change
        pge_network::PgePacket newPktUserNameChange;
        if (!proofps_dd::MsgUserNameChange::initPkt(newPktUserNameChange, connHandleServerSide, false, szNewUserName))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
            assert(false);
            return false;
        }
        m_pge.getNetwork().getServer().sendToAllClientsExcept(newPktUserNameChange, connHandleServerSide);

        if (connHandleServerSide != pge_network::ServerConnHandle)
        {
            m_pge.getNetwork().getServer().setDebugNickname(connHandleServerSide, szNewUserName);
            // we also let this one know its own name change (only if this is not server)
            proofps_dd::MsgUserNameChange& msgUserNameChange = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserNameChange>(newPktUserNameChange);
            msgUserNameChange.m_bCurrentClient = true;
            m_pge.getNetwork().getServer().send(newPktUserNameChange, connHandleServerSide);
        }

        if (msg.m_bCurrentClient)
        {
            m_gui.textPermanent("Server, User name: " + std::string(szNewUserName) +
                (cfgProfiles.getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
                10, 30);
        }
    }
    else
    {
        // if we are client, we MUST NOT receive empty user name from server, so in such case just terminate because there is something fishy!
        if (strnlen(msg.m_szUserName, sizeof(msg.m_szUserName)) == 0)
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): cannot happen: connHandleServerSide: %u, received empty user name from server!",
                __func__, connHandleServerSide);
            assert(false);  // in debug mode, raise the debugger
            return false;   // for release mode
        }


        getConsole().OLn("PRooFPSddPGE::%s(): accepting new name from server for connHandleServerSide: %u (%s), old name: %s, new name: %s!",
            __func__, connHandleServerSide, msg.m_bCurrentClient ? "me" : "not me", playerIt->second.getName().c_str(), msg.m_szUserName);

        playerIt->second.setName(msg.m_szUserName);
        // TODO: these commented lines below will be needed when we are allowing player name change WHILE already connected to the server
        // TODO: check if such name is already present in frag table, if so, then rename
        //if (!gameMode.renamePlayer(playerIt->second.getName(), msg.m_szUserName))
        //{
        //    getConsole().EOLn("PRooFPSddPGE::%s(): gameMode.renamePlayer() FAILED!", __func__);
        //    assert(false);
        //    return false;
        //}
        if (!gameMode.addPlayer(playerIt->second))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): failed to insert player %s (%u) into GameMode!", __func__, msg.m_szUserName, connHandleServerSide);
            assert(false);
            return false;
        }

        if (msg.m_bCurrentClient)
        {
            // due to difficulties caused by m_gui.textPermanent() it is easier to use it here than in handleUserSetupFromServer()
            m_gui.textPermanent("Client, User name: " + playerIt->second.getName() + "; IP: " + playerIt->second.getIpAddress() +
                (cfgProfiles.getVars()["testing"].getAsBool() ? "; Testing Mode" : ""),
                10, 30);
        }
    }

    m_pge.getNetwork().WriteList();
    WritePlayerList();

    return true;

}  // handleUserNameChange()

void proofps_dd::PlayerHandling::resetSendClientUpdatesCounter(proofps_dd::Config& config)
{
    // Config::validate() makes sure neither getTickRate() nor getClientUpdateRate() return 0
    m_nSendClientUpdatesInEveryNthTick = config.getTickRate() / config.getClientUpdateRate();
    m_nSendClientUpdatesCntr = m_nSendClientUpdatesInEveryNthTick;
}

void proofps_dd::PlayerHandling::serverSendUserUpdates(proofps_dd::Durations& durations)
{
    if (!m_pge.getNetwork().isServer())
    {
        // should not happen, but we log it anyway, if in future we might mess up something during a refactor ...
        getConsole().EOLn("PRooFPSddPGE::%s(): NOT server!", __func__);
        assert(false);
        return;
    }

    const std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
    const bool bSendUserUpdates = (m_nSendClientUpdatesCntr == m_nSendClientUpdatesInEveryNthTick);

    for (auto& playerPair : m_mapPlayers)
    {
        auto& player = playerPair.second;

        if (bSendUserUpdates && player.isNetDirty())
        {
            pge_network::PgePacket newPktUserUpdate;
            //getConsole().EOLn("PRooFPSddPGE::%s(): send 1!", __func__);
            if (proofps_dd::MsgUserUpdateFromServer::initPkt(
                newPktUserUpdate,
                playerPair.second.getServerSideConnectionHandle(),
                player.getPos().getNew().getX(),
                player.getPos().getNew().getY(),
                player.getPos().getNew().getZ(),
                player.getAngleY(),
                player.getAngleZ(),
                player.getWeaponAngle().getNew().getZ(),
                player.getCrouchStateCurrent(),
                player.getHealth(),
                player.getRespawnFlag(),
                player.getFrags(),
                player.getDeaths()))
            {
                player.clearNetDirty();

                // we always reset respawn flag here
                playerPair.second.getRespawnFlag() = false;

                // Note that health is not needed by server since it already has the updated health, but for convenience
                // we put that into MsgUserUpdateFromServer and send anyway like all the other stuff.
                m_pge.getNetwork().getServer().sendToAll(newPktUserUpdate);
                //getConsole().EOLn("PRooFPSddPGE::%s(): send 2!", __func__);
            }
            else
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): initPkt() FAILED at line %d!", __func__, __LINE__);
                assert(false);
            }
        }
    }  // for playerPair

    if (bSendUserUpdates)
    {
        m_nSendClientUpdatesCntr = 0;
        // measure duration only if we really sent the user updates to clients
        durations.m_nSendUserUpdatesDurationUSecs += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
    }

    ++m_nSendClientUpdatesCntr;
} // serverSendUserUpdates()

bool proofps_dd::PlayerHandling::handleUserUpdateFromServer(
    pge_network::PgeNetworkConnectionHandle connHandleServerSide,
    const proofps_dd::MsgUserUpdateFromServer& msg,
    PureObject3D& objXHair,
    proofps_dd::GameMode& gameMode)
{
    const auto it = m_mapPlayers.find(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return true;  // might NOT be fatal error in some circumstances, although I cannot think about any, but dont terminate the app for this ...
    }

    //getConsole().OLn("PRooFPSddPGE::%s(): user %s received MsgUserUpdateFromServer: %f", __func__, it->second.getName().c_str(), msg.m_pos.x);

    const bool bOriginalExpectingStartPos = it->second.isExpectingStartPos();
    if (it->second.isExpectingStartPos())
    {
        it->second.SetExpectingStartPos(false);
        // PPPKKKGGGGGG
        it->second.getPos().set(PureVector(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z));
        it->second.getPos().commit(); // both server and client commits in this case
    }

    it->second.getPos().set(PureVector(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z)); // server does not commit here, client commits few lines below by invoking updateOldValues()
    it->second.getObject3D()->getPosVec().Set(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);
    it->second.getWeaponManager().getCurrentWeapon()->UpdatePosition(it->second.getObject3D()->getPosVec());

    if (msg.m_fPlayerAngleY != -1.f)
    {
        //it->second.getAngleY() = msg.m_fPlayerAngleY;  // not sure why this is commented
        it->second.getObject3D()->getAngleVec().SetY(msg.m_fPlayerAngleY);
    }
    it->second.getObject3D()->getAngleVec().SetZ(msg.m_fPlayerAngleZ);

    it->second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().SetY(it->second.getObject3D()->getAngleVec().getY());
    it->second.getWeaponManager().getCurrentWeapon()->getObject3D().getAngleVec().SetZ(msg.m_fWpnAngleZ);

    //getConsole().OLn("PRooFPSddPGE::%s(): rcvd crouch: %b", __func__, msg.m_bCrouch);
    if (msg.m_bCrouch)
    {
        // server had already set stuff since it relayed this to clients, however
        // there is no use of adding extra condition for checking if we are server or client
        it->second.DoCrouchShared();
    }
    else
    {
        it->second.DoStandupShared();
    }

    it->second.getFrags() = msg.m_nFrags;
    it->second.getDeaths() = msg.m_nDeaths;

    //getConsole().OLn("PRooFPSddPGE::%s(): rcvd health: %d, health: %d, old health: %d",
    //    __func__, msg.m_nHealth, it->second.getHealth(), it->second.getHealth().getOld());
    it->second.SetHealth(msg.m_nHealth);

    if (msg.m_bRespawn)
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): player %s has respawned!", __func__, it->second.getName().c_str());
        HandlePlayerRespawned(it->second, objXHair);
    }
    else
    {
        if ((it->second.getHealth().getOld() > 0) && (it->second.getHealth() == 0))
        {
            // only clients fall here, since server already set oldhealth to 0 at the beginning of this frame
            // because it had already set health to 0 in previous frame
            //getConsole().OLn("PRooFPSddPGE::%s(): player %s has died!", __func__, it->second.getName().c_str());
            HandlePlayerDied(it->second, objXHair, it->second.getServerSideConnectionHandle() /* ignored by client anyway */);

            // TODO: until v0.2.0.0 this was the only location where client could figure out if any player died, however
            // now we have handleDeathNotificationFromServer(), we could simply move this code to there!
            // Client does not invoke HandlePlayerDied() anywhere else.
        }
    }

    // the only situation when game mode does not contain the player but we already receive update for the player is
    // when isExpectingStartPos() is true, because the userNameChange will be handled a bit later;
    // note that it can also happen that we receive update here for a player who has not yet handshaked its name
    // with the server, in that case the name is empty, that is why we also need to check emptiness!
    if (!it->second.getName().empty() && !bOriginalExpectingStartPos && !gameMode.updatePlayer(it->second))
    {
        getConsole().EOLn("%s: failed to update player %s in GameMode!", __func__, it->second.getName().c_str());
    }

    if (!m_pge.getNetwork().isServer())
    {
        // server already invoked updateOldValues() when it sent out this update message
        it->second.updateOldValues();
    }

    return true;
}  // handleUserUpdateFromServer()

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
