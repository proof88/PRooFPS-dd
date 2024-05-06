/*
    ###################################################################################
    Player.cpp
    Player class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "Player.h"
#include "PRooFPS-dd-packet.h"


// ############################### PUBLIC ################################


const char* proofps_dd::Player::getLoggerModuleName()
{
    return "Player";
}

void proofps_dd::Player::genUniqueUserName(
    char szNewUserName[proofps_dd::MsgUserNameChangeAndBootupDone::nUserNameBufferLength],
    const std::string& sNameFromConfig,
    const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers)
{
    std::string sNewPlayerName(sNameFromConfig);
    if (sNewPlayerName.empty())
    {
        sNewPlayerName = "Player";
    }
    sNewPlayerName = sNewPlayerName.substr(0, proofps_dd::MsgUserNameChangeAndBootupDone::nUserNameBufferLength - 1);

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
            sNewPlayerName = sNewPlayerName.substr(0, proofps_dd::MsgUserNameChangeAndBootupDone::nUserNameBufferLength - 1 - nUniqueNumberWidthInName) +
                std::to_string(static_cast<size_t>(std::pow(10, nUniqueNumberWidthInName - 1)) + (rand() % static_cast<size_t>(std::pow(10, nUniqueNumberWidthInName))));
            sNewPlayerName = sNewPlayerName.substr(0, proofps_dd::MsgUserNameChangeAndBootupDone::nUserNameBufferLength - 1);
        }
    };

    strncpy_s(szNewUserName, proofps_dd::MsgUserNameChangeAndBootupDone::nUserNameBufferLength, sNewPlayerName.c_str(), sNewPlayerName.length());
}


proofps_dd::Player::Player(
    pge_audio::PgeAudio& audio,
    PGEcfgProfiles& cfgProfiles,
    std::list<Bullet>& bullets,
    PR00FsUltimateRenderingEngine& gfx,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const std::string& sIpAddress) :
    m_audio(audio),
    m_connHandleServerSide(connHandle),
    m_sIpAddress(sIpAddress),
    m_wpnMgr(audio, cfgProfiles, gfx, bullets),
    m_cfgProfiles(cfgProfiles),
    m_bullets(bullets),
    m_gfx(gfx)
{
    BuildPlayerObject(true);
}

proofps_dd::Player::Player(const proofps_dd::Player& other) :
    m_connHandleServerSide(other.m_connHandleServerSide),
    m_sIpAddress(other.m_sIpAddress),
    m_sName(other.m_sName),
    m_vecOldNewValues(other.m_vecOldNewValues),
    m_bNetDirty(other.m_bNetDirty),
    m_timeDied(other.m_timeDied),
    m_bRespawn(other.m_bRespawn),
    m_vecImpactForce(other.m_vecImpactForce),
    m_pObj(PGENULL),
    m_pTexPlayerStand(PGENULL),
    m_pTexPlayerCrouch(PGENULL),
    m_wpnMgr(other.m_audio, other.m_cfgProfiles, other.m_gfx, other.m_bullets),
    m_audio(other.m_audio),
    m_cfgProfiles(other.m_cfgProfiles),
    m_bullets(other.m_bullets),
    m_gfx(other.m_gfx),
    m_vecJumpForce(other.m_vecJumpForce),
    m_fGravity(other.m_fGravity),
    m_bJumping(other.m_bJumping),
    m_bAllowJump(other.m_bAllowJump),
    m_bWillJump(other.m_bWillJump),
    m_bCanFall(other.m_bCanFall),
    m_bFalling(other.m_bFalling),
    m_bHasJustStartedFallingNaturally(other.m_bHasJustStartedFallingNaturally),
    m_bHasJustStartedFallingAfterJumpingStopped(other.m_bHasJustStartedFallingAfterJumpingStopped),
    m_fHeightStartedFalling(other.m_fHeightStartedFalling),
    m_bHasJustStoppedJumping(other.m_bHasJustStoppedJumping),
    m_bCrouchingStateCurrent(other.m_bCrouchingStateCurrent),
    m_bCrouchingWasActiveWhenInitiatedJump(other.m_bCrouchingWasActiveWhenInitiatedJump),
    m_bWantToStandup(other.m_bWantToStandup),
    m_bWillSomersault(other.m_bWillSomersault),
    m_fSomersaultAngleZ(other.m_fSomersaultAngleZ),
    m_bRunning(other.m_bRunning),
    m_bExpectingStartPos(other.m_bExpectingStartPos),
    m_strafe(other.m_strafe),
    m_prevActualStrafe(other.m_prevActualStrafe),
    m_bAttack(other.m_bAttack)
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
    m_timeDied = other.m_timeDied;
    m_bRespawn = other.m_bRespawn;
    m_vecImpactForce = other.m_vecImpactForce;
    //m_audio = other.m_audio;  // deleted function
    //m_cfgProfiles = other.m_cfgProfiles;  // inaccessible
    m_bullets = other.m_bullets;
    m_gfx = other.m_gfx;
    m_vecJumpForce = other.m_vecJumpForce;
    m_fGravity = other.m_fGravity;
    m_bJumping = other.m_bJumping;
    m_bAllowJump = other.m_bAllowJump;
    m_bWillJump = other.m_bWillJump;
    m_bCanFall = other.m_bCanFall;
    m_bFalling = other.m_bFalling;
    m_bHasJustStartedFallingNaturally = other.m_bHasJustStartedFallingNaturally;
    m_bHasJustStartedFallingAfterJumpingStopped = other.m_bHasJustStartedFallingAfterJumpingStopped;
    m_fHeightStartedFalling = other.m_fHeightStartedFalling;
    m_bHasJustStoppedJumping = other.m_bHasJustStoppedJumping;
    m_bCrouchingStateCurrent = other.m_bCrouchingStateCurrent;
    m_bCrouchingWasActiveWhenInitiatedJump = other.m_bCrouchingWasActiveWhenInitiatedJump;
    m_bWantToStandup = other.m_bWantToStandup;
    m_bWillSomersault = other.m_bWillSomersault;
    m_fSomersaultAngleZ = other.m_fSomersaultAngleZ;
    m_bRunning = other.m_bRunning;
    m_bExpectingStartPos = other.m_bExpectingStartPos;
    m_strafe = other.m_strafe;
    m_prevActualStrafe = other.m_prevActualStrafe;
    m_bAttack = other.m_bAttack;

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

const PgeOldNewValue<bool>& proofps_dd::Player::getInvulnerability() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<bool>>(m_vecOldNewValues.at(OldNewValueName::OvInvulnerability));
}

/**
* Both server and client use this function.
* However, only server actually uses the nSeconds argument.
* Client never knows the duration of invulnerability, it just always sets the state received from server.
* Server is responsible for managing invulnerability durations and explicitly notifying clients about invulnerability ending.
* 
* @param bState   True for activating player invulnerability, false to reset it.
* @param nSeconds Duration of invulnerability. Used by server instance only.
*/
void proofps_dd::Player::setInvulnerability(const bool& bState, const unsigned int& nSeconds)
{
    std::get<PgeOldNewValue<bool>>(m_vecOldNewValues.at(OldNewValueName::OvInvulnerability)).set(bState);
    m_nInvulnerabilityDurationSecs = nSeconds;
    m_timeStartedInvulnerability = std::chrono::steady_clock::now();
}

const unsigned int& proofps_dd::Player::getInvulnerabilityDurationSeconds() const
{
    return m_nInvulnerabilityDurationSecs;
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::Player::getTimeInvulnerabilityStarted() const
{
    return m_timeStartedInvulnerability;
}

void proofps_dd::Player::update(const proofps_dd::Config& config, bool bServer)
{
    if (getInvulnerability())
    {
        constexpr auto nBlinkPeriodMillisecs = 100;
        const auto nInvulTimeElapsedMillisecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_timeStartedInvulnerability).count();
        // only server instance is allowed to decide about ENDING invulnerability, client will be notified over network msg
        const bool bEndInvul = bServer && (static_cast<unsigned int>(nInvulTimeElapsedMillisecs) >= config.getPlayerRespawnInvulnerabilityDelaySeconds() * 1000);
        const bool bShowPlayer = bEndInvul || ((nInvulTimeElapsedMillisecs / nBlinkPeriodMillisecs) % 2) == 0;
        setVisibilityState(bShowPlayer);
        if (bEndInvul)
        {
            setInvulnerability(false);
        }
    }
    else
    {
        if (getHealth() > 0)
        {
            show();
        }
    }
}

void proofps_dd::Player::show()
{
    setVisibilityState(true);
}

void proofps_dd::Player::hide()
{
    setVisibilityState(false);
}

void proofps_dd::Player::setVisibilityState(bool state)
{
    getObject3D()->SetRenderingAllowed(state);
    if (m_wpnMgr.getCurrentWeapon())
    {
        m_wpnMgr.getCurrentWeapon()->getObject3D().SetRenderingAllowed(state);
    }
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

const PgeOldNewValue<int>& proofps_dd::Player::getHealth() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvHealth));
}

void proofps_dd::Player::setHealth(int value) {
    getHealth().set(std::max(0, std::min(value, 100)));
}

void proofps_dd::Player::doDamage(int dmg) {
    getHealth().set(getHealth().getNew() - dmg);
    if (getHealth().getNew() < 0)
    {
        getHealth().set(0);
    }
}

PureVector& proofps_dd::Player::getImpactForce()
{
    return m_vecImpactForce;
}

void proofps_dd::Player::die(bool bMe, bool bServer)
{
    m_timeDied = std::chrono::steady_clock::now();
    if (bMe)
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): I died!", __func__);
    }
    else
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): other player died!", __func__);
    }
    setHealth(0);
    getAttack() = false;
    hide();
    if (bServer)
    {
        setStrafe(Strafe::NONE);
        m_prevActualStrafe = Strafe::NONE;
        setWillJumpInNextTick(false);
        getJumpForce().SetZero();
        resetSomersaultServer();
        setWillSomersaultInNextTick(false);

        // server instance has the right to modify death count, clients will just receive it in update
        getDeaths()++;
        //getConsole().OLn("PRooFPSddPGE::%s(): new death count: %d!", __func__, getDeaths());
    }
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::Player::getTimeDied() const
{
    return m_timeDied;
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

bool& proofps_dd::Player::getRespawnFlag()
{
    return m_bRespawn;
}

void proofps_dd::Player::respawn(bool /*bMe*/, const Weapon& wpnDefaultAvailable, bool bServer)
{
    doStandupShared();
    getWantToStandup() = true;
    getImpactForce().SetZero();
    m_prevActualStrafe = Strafe::NONE;

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
            pWpn->UpdatePosition(getObject3D()->getPosVec(), false);
        }
    }

    show();
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

bool proofps_dd::Player::isExpectingStartPos() const
{
    return m_bExpectingStartPos;
}

void proofps_dd::Player::setExpectingStartPos(bool b)
{
    m_bExpectingStartPos = b;
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

PgeOldNewValue<PureVector>& proofps_dd::Player::getWeaponAngle()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<PureVector>>(m_vecOldNewValues.at(OldNewValueName::OvWpnAngle));
}

PureObject3D* proofps_dd::Player::getObject3D() const
{
    return m_pObj;
}

float proofps_dd::Player::getGravity() const
{
    return m_fGravity;
}

void proofps_dd::Player::setGravity(float value) {
    m_fGravity = value;
    if (value >= 0.f)
    {
        m_bFalling = false;
    }
}

bool proofps_dd::Player::isJumping() const
{
    return m_bJumping;
}

bool proofps_dd::Player::canFall() const
{
    return m_bCanFall;
}

void proofps_dd::Player::setCanFall(bool state) {
    m_bCanFall = state;
}

bool proofps_dd::Player::isInAir() const
{
    return isJumping() || canFall();
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

bool proofps_dd::Player::jumpAllowed() const {
    return m_bAllowJump;
}

void proofps_dd::Player::setJumpAllowed(bool b) {
    m_bAllowJump = b;
}

void proofps_dd::Player::jump() {
    m_bWillJump = false;

    if (isJumping() || !jumpAllowed() || isSomersaulting())
    {
        return;
    }

    m_bAllowJump = false;
    m_bJumping = true;
    m_bFalling = false;
    m_bCrouchingWasActiveWhenInitiatedJump = getCrouchInput().getNew();
    m_fGravity = m_bCrouchingWasActiveWhenInitiatedJump ? proofps_dd::Player::fJumpGravityStartFromCrouching : proofps_dd::Player::fJumpGravityStartFromStanding;

    //static int nJumpCounter = 0;
    //nJumpCounter++;
    //getConsole().EOLn("%s() %d: fGravity: %f, crouchInput: %b ", __func__, nJumpCounter, m_fGravity, m_bCrouchingWasActiveWhenInitiatedJump);

    m_vecJumpForce.SetX(getPos().getNew().getX() - getPos().getOld().getX());
    // we dont use other components of jumpForce vec, since Z-axis is "unused", Y-axis jump force is controlled by m_fGravity 
    //m_vecJumpForce.SetY(getPos().getNew().getY() - getPos().getOld().getY());
    //m_vecJumpForce.SetZ(getPos().getNew().getZ() - getPos().getOld().getZ());
    //getConsole().EOLn("jump x force: %f", m_vecJumpForce.getX());
}

void proofps_dd::Player::stopJumping() {
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

PureVector& proofps_dd::Player::getJumpForce()
{
    return m_vecJumpForce;
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

float proofps_dd::Player::getProposedNewPosYforStandup() const
{
    constexpr float fProposedNewPlayerHalfHeight = Player::fObjHeightStanding / 2.f;
    bool bPushDownLegs = isInAir(); // growth is inverse to bPullUpLegs in doCrouchServer()

    return bPushDownLegs ?
        getPos().getNew().getY() + (Player::fObjHeightStanding * Player::fObjHeightCrouchScaling) / 2.f - fProposedNewPlayerHalfHeight :
        getPos().getNew().getY() - (Player::fObjHeightStanding * Player::fObjHeightCrouchScaling) / 2.f + fProposedNewPlayerHalfHeight + 0.01f;
}

/**
* Changing player properties for physics.
* Invokes getPos().commit() too so not be called careless from anywhere.
* Should be invoked only by server physics at the beginning of physics calculations.
*
* @param bPullUpLegs True will result in the top Y position of the object won't change thus it will shrink upwards (should be used mid-air).
*                    False will result in both the top and bottom Y positions of the object will change thus it will shrink towards center (should be used on ground).
*/
void proofps_dd::Player::doCrouchServer()
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
    getObject3D()->SetScaling(PureVector(1.f, Player::fObjHeightCrouchScaling, 1.f));
    getCrouchStateCurrent() = true;  // can always go to crouching immediately

    const bool bPullUpLegs = isInAir();
    if (bPullUpLegs)
    {
        // reposition is allowed only if being in the air: pulling up the legs, so the head supposed to stay in same position as before,
        // however we don't reposition if being on ground because it looks bad
        // PPPKKKGGGGGG
        PureVector playerPos = getPos().getNew();
        playerPos.SetY(playerPos.getY() + Player::fObjHeightStanding / 2.f - (Player::fObjHeightStanding * Player::fObjHeightCrouchScaling) / 2.f);
        getPos().set(playerPos);
        // since we are at the beginning of a tick, it is legal to commit the position now, as old and new positions supposed to be the same at this point
        getPos().commit();
    }
}

void proofps_dd::Player::doCrouchShared()
{
    // we don't change player position in this "shared" version since only "server" versions do that and positions are replicated to clients in the usual way
    getObject3D()->SetScaling(PureVector(1.f, Player::fObjHeightCrouchScaling, 1.f));
    getObject3D()->getMaterial().setTexture(m_pTexPlayerCrouch);
    getCrouchStateCurrent() = true; // since this is replicated from server, it is valid
    // getWantToStandup() stays updated on server-side only, in clientHandleInputWhenConnectedAndSendUserCmdMoveToServer(), do not modify anywhere else!
    //getWantToStandup() = false;
}

/**
* Changing player properties for physics.
* Invokes getPos().set() too so not be called careless from anywhere.
* Should be invoked only by server physics during physics calculations.
*/
void proofps_dd::Player::doStandupServer()
{
    getCrouchStateCurrent() = false;
    getObject3D()->SetScaling(PureVector(1.f, 1.f, 1.f));
    // Physics engine calls us if verification for enough free space has passed, it is not our job to check that
    getPos().set(
        PureVector(
            getPos().getNew().getX(),
            getProposedNewPosYforStandup(),
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

void proofps_dd::Player::doStandupShared()
{
    // we don't change player position in this "shared" version since only "server" versions do that and positions are replicated to clients in the usual way
    getObject3D()->SetScaling(PureVector(1.f, 1.f, 1.f));
    getObject3D()->getMaterial().setTexture(m_pTexPlayerStand);
    getCrouchStateCurrent() = false;  // since this is replicated from server, it is valid
    // getWantToStandup() stays updated on server-side only, in clientHandleInputWhenConnectedAndSendUserCmdMoveToServer(), do not modify anywhere else!
    //getWantToStandup() = true;
}

bool proofps_dd::Player::getWillSomersaultInNextTick() const
{
    return m_bWillSomersault;
}

void proofps_dd::Player::setWillSomersaultInNextTick(bool flag)
{
    m_bWillSomersault = flag;
}

/**
 * Salto/somersault aka front-/backflip.
 * The idea is this function sets an initial positive or negative value for the angle based on strafe direction, and then
 * Physics class will take care of the full somersaulting by periodic calls to stepSomersaultAngleServer().
 * 
 * @param bJumpInduced Set it to true if somersault is initiated during jump-up so it will be a mid-air salto/somersault.
 *                     Set it to false if somersault is initiated on-ground so it will be an on-ground somersault.
 */
void proofps_dd::Player::startSomersaultServer(bool bJumpInduced)
{
    m_bWillSomersault = false;

    // sanity check
    if (isSomersaulting())
    {
        return;
    }

    if (bJumpInduced)
    {
        // mid-air sanity check
        if ((isJumping() && isJumpingInitiatedFromCrouching()) || (isJumping() != bJumpInduced))
        {
            return;
        }
    }
    else
    {
        // on-ground sanity check
        if (isInAir() || (m_strafe == Strafe::NONE))
        {
            return;
        }
    }

    if (!getCrouchStateCurrent())
    {
        // for somersaulting on ground, we always require manual crouch, however for mid-air somersaulting, crouching depends on server config!
        if (bJumpInduced && m_cfgProfiles.getVars()[Player::szCVarSvSomersaultMidAirAutoCrouch].getAsBool())
        {
            doCrouchServer();
        }
        else
        {
            return;
        }
    }

    // just set the initial m_fSomersaultAngleZ by setting a small value, so stepSomersaultAngleServer() will know in which direction it should change angle
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

    if (bJumpInduced)
    {
        // triggering mid-air somersaulting modifies player jump force and gravity, not the impact force
        // TODO: this should be accessed thru Config::getSomersaultMidAirJumpForceMultiplier(), however that introduces unforeseen mass of compilation problems now!
        m_vecJumpForce.SetX(m_vecJumpForce.getX() * m_cfgProfiles.getVars()[Player::szCVarSvSomersaultMidAirJumpForceMultiplier].getAsFloat());
        m_fGravity *= m_cfgProfiles.getVars()[Player::szCVarSvSomersaultMidAirJumpForceMultiplier].getAsFloat();
    }
    else
    {
        // triggering on-ground somersaulting modifies impact force only, not jump force or gravity
        switch (m_strafe)
        {
        case Strafe::LEFT:
            if (m_vecImpactForce.getX() > -Player::fSomersaultGroundImpactForceX)
            {
                m_vecImpactForce.SetX(std::max(-Player::fSomersaultGroundImpactForceX, m_vecImpactForce.getX() - Player::fSomersaultGroundImpactForceX));
            }
            break;
        case Strafe::RIGHT:
            if (m_vecImpactForce.getX() < Player::fSomersaultGroundImpactForceX)
            {
                m_vecImpactForce.SetX(std::min(Player::fSomersaultGroundImpactForceX, m_vecImpactForce.getX() + Player::fSomersaultGroundImpactForceX));
            }
            break;
        default /* Strafe::NONE */:
            break; // no-op
        }
    }
}

/**
 * Since server calculates somersault angle, client just receives the angle periodically from server.
 * This function is for that case when server sends the updated angle.
 */
void proofps_dd::Player::setSomersaultClient(float angleZ)
{
    m_fSomersaultAngleZ = angleZ;
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
 * The idea is that we set the initial angle by a call to startSomersaultServer() and then in consecutive physics timesteps
 * we call stepSomersaultAngleServer() with a value calculated by the physics engine.
 * Somersaulting will stop once stepSomersaultAngleServer() finishes a complete roll/flip.
 * This also means that currently only 1 full roll/flip is done before isSomersaulting() returns false again.
 */
void proofps_dd::Player::stepSomersaultAngleServer(float angle)
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

void proofps_dd::Player::resetSomersaultServer()
{
    m_fSomersaultAngleZ = 0.f;
    getAngleZ() = 0.f;
}

bool proofps_dd::Player::isRunning() const
{
    return m_bRunning;
}

void proofps_dd::Player::setRun(bool state)
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
    if (m_strafe != Strafe::NONE)
    {
        m_prevActualStrafe = m_strafe;
    }
    m_strafe = strafe;
    if (strafe != Strafe::NONE)
    {
        m_timeLastStrafe = std::chrono::steady_clock::now();
    }
}

/**
 * Gets previous non-NONE strafe value registered before the last NONE strafe value and the current non-NONE strafe state.
 * It is Strafe::NONE only right after constructing the Player object or after respawn().
 */
const proofps_dd::Strafe& proofps_dd::Player::getPreviousActualStrafe() const
{
    return m_prevActualStrafe;
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::Player::getTimeLastActualStrafe() const
{
    return m_timeLastStrafe;
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

void proofps_dd::Player::takeItem(MapItem& item, pge_network::PgePacket& pktWpnUpdate)
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

        item.take();
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
        item.take();
        setHealth(getHealth() + static_cast<int>(MapItem::ITEM_HEALTH_HP_INC));
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
    m_pObj = m_gfx.getObject3DManager().createPlane(proofps_dd::Player::fObjWidth, proofps_dd::Player::fObjHeightStanding);
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

PgeOldNewValue<int>& proofps_dd::Player::getHealth()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvHealth));
}
