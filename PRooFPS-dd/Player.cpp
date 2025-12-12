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
#include "Consts.h"


static constexpr float SndPlayerLandedDistMin = 6.f;
static constexpr float SndPlayerLandedDistMax = 8.f;
static constexpr float SndPlayerHighFallYellDistMin = 9.f;
static constexpr float SndPlayerHighFallYellDistMax = 22.f;
static constexpr float SndPlayerItemActivateAntiGravityDistMin = SndPlayerLandedDistMin;
static constexpr float SndPlayerItemActivateAntiGravityDistMax = SndPlayerLandedDistMax;
static constexpr float SndPlayerItemDeactivateAntiGravityDistMin = SndPlayerItemActivateAntiGravityDistMin;
static constexpr float SndPlayerItemDeactivateAntiGravityDistMax = SndPlayerItemActivateAntiGravityDistMax;
static constexpr float SndPlayerItemLowThrustAntiGravityDistMin = SndPlayerLandedDistMin;
static constexpr float SndPlayerItemLowThrustAntiGravityDistMax = SndPlayerLandedDistMax;
static constexpr float SndPlayerItemHighThrustAntiGravityDistMin = SndPlayerItemLowThrustAntiGravityDistMin;
static constexpr float SndPlayerItemHighThrustAntiGravityDistMax = SndPlayerItemLowThrustAntiGravityDistMax;
static constexpr float SndPlayerFootstepDistMin = SndPlayerLandedDistMin;
static constexpr float SndPlayerFootstepDistMax = SndPlayerLandedDistMax;

static constexpr std::chrono::milliseconds::rep TimeBetween2FootstepsMillisecs = 0;  // minimum desired time, if we want longer time than actual length of footstep sound
static constexpr std::chrono::milliseconds::rep TimeBefore1stFootstepCanBePlayedMillisecs = 100;  // when player starts running, don't immediately play 1st footstep but delay it by this


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
    PgeObjectPool<PooledBullet>& bullets,
    EventLister<>& eventsItemPickup,
    EventLister<>& eventsInventoryChange,
    EventLister<>& eventsAmmoChange,
    PR00FsUltimateRenderingEngine& gfx,
    pge_network::PgeINetwork& network,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const std::string& sIpAddress) :
    m_audio(audio),
    m_connHandleServerSide(connHandle),
    m_sIpAddress(sIpAddress),
    m_wpnMgr(audio, cfgProfiles, gfx, bullets),
    m_cfgProfiles(cfgProfiles),
    m_bullets(bullets),
    m_eventsItemPickup(eventsItemPickup),
    m_eventsInventoryChange(eventsInventoryChange),
    m_eventsAmmoChange(eventsAmmoChange),
    m_gfx(gfx),
    m_network(network)
{
    BuildPlayerObject(true);

    // load audio sources only once whenever 1st player object is created, does not matter if that is server or current client player,
    // because these are static instances need to be set up only once for our player only
    if (!m_sndWpnAmmo)
    {
        // no need to destruct in dtor, static destructions will take care anyway when exiting
        m_sndWpnAmmo = new SoLoud::Wav();
        m_sndWpnNew = new SoLoud::Wav();
        m_sndArmor = new SoLoud::Wav();
        m_sndMedkit = new SoLoud::Wav();
        m_sndJetlax = new SoLoud::Wav();
        m_sndJumppad = new SoLoud::Wav();
        m_sndPlayerItemActivateAntiGravity = new SoLoud::Wav();
        m_sndPlayerItemDeactivateAntiGravity = new SoLoud::Wav();
        m_sndPlayerItemLowThrustAntiGravity = new SoLoud::Wav();
        m_sndPlayerItemHighThrustAntiGravity = new SoLoud::Wav();
        m_sndFallYell_1 = new SoLoud::Wav();
        m_sndFallYell_2 = new SoLoud::Wav();
        m_sndPlayerLandSmallFall = new SoLoud::Wav();
        m_sndPlayerLandBigFall = new SoLoud::Wav();
        m_sndPlayerDamage = new SoLoud::Wav();
        for (auto& ptr : m_sndPlayerFootstep)
        {
            ptr = new SoLoud::Wav();
        }
        m_sndPlayerFunnyLaughDiedByLanding = new SoLoud::Wav();
    }

    // note that due to config can be changed in settings, we need to check if sound is actually loaded and try load again if it was
    // not loaded previously, as loadSound() loads only if audio is enabled and initialized!
    if (m_sndWpnAmmo->getLength() == 0.f)
    {
        // AudioSource was not yet loaded from file, let's try it now then.
        // If we load stuff now below, and later user turns audio off, that is also fine, no audio will be played, and if later
        // again they turn it on, we can use the previously loaded AudioSource information to create instances for playing.

        // new would had thrown above in case of alloc failure, no need to check
        m_audio.loadSound(*m_sndWpnAmmo, std::string(proofps_dd::GAME_AUDIO_DIR) + "maps/item_wpn_ammo.wav");
        m_audio.loadSound(*m_sndWpnNew, std::string(proofps_dd::GAME_AUDIO_DIR) + "maps/item_wpn_new.wav");
        m_audio.loadSound(*m_sndArmor, std::string(proofps_dd::GAME_AUDIO_DIR) + "maps/item_armor.wav");
        m_audio.loadSound(*m_sndMedkit, std::string(proofps_dd::GAME_AUDIO_DIR) + "maps/item_medkit.wav");
        m_audio.loadSound(*m_sndJetlax, std::string(proofps_dd::GAME_AUDIO_DIR) + "maps/item_jetlax.wav");
        m_audio.loadSound(*m_sndJumppad, std::string(proofps_dd::GAME_AUDIO_DIR) + "maps/jumppad.wav");
        m_audio.loadSound(*m_sndPlayerItemActivateAntiGravity, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/item_jetlax_activate.wav");
        m_audio.loadSound(*m_sndPlayerItemDeactivateAntiGravity, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/item_jetlax_deactivate.wav");
        m_audio.loadSound(*m_sndPlayerItemLowThrustAntiGravity, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/item_jetlax_low_thrust_loop.wav");
        m_audio.loadSound(*m_sndPlayerItemHighThrustAntiGravity, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/item_jetlax_high_thrust_loop.wav");
        m_audio.loadSound(*m_sndFallYell_1, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/the-howie-scream-2.wav");
        m_audio.loadSound(*m_sndFallYell_2, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/the-wilhelm-scream.wav");
        m_audio.loadSound(*m_sndPlayerLandSmallFall, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/player_land_smallfall.wav");
        m_audio.loadSound(*m_sndPlayerLandBigFall, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/player_land_bigfall.wav");
        m_audio.loadSound(*m_sndPlayerDamage, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/player_damage.wav");

        size_t i = 0;
        for (auto& ptr : m_sndPlayerFootstep)
        {
            i++;
            m_audio.loadSound(*ptr, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/pl_step" + std::to_string(i) + ".wav");
            const int nThisSndDurationMillisecs = static_cast<int>(PFL::roundf( static_cast<float>(ptr->getLength() * 1000) ));
            if (nThisSndDurationMillisecs > m_nMaxSndPlayerFootstepDurationMillisecs)
            {
                m_nMaxSndPlayerFootstepDurationMillisecs = nThisSndDurationMillisecs;
            }
        }
        m_nMinTimeBetweenPlayerWalkSoundsMillisecs = std::max(
            m_nMaxSndPlayerFootstepDurationMillisecs,
            TimeBetween2FootstepsMillisecs
        );

        getConsole().OLn("Player::%s() m_nMaxSndPlayerFootstepDurationMillisecs: %d", __func__, m_nMaxSndPlayerFootstepDurationMillisecs);
        getConsole().OLn("Player::%s() m_nMinTimeBetweenPlayerWalkSoundsMillisecs: %d", __func__, m_nMinTimeBetweenPlayerWalkSoundsMillisecs);

        m_audio.loadSound(*m_sndPlayerFunnyLaughDiedByLanding, std::string(proofps_dd::GAME_AUDIO_DIR) + "player/crazy-dog-laugh-meme.wav");

        // these are played only for self and should be stopped automatically when played again to avoid multiple instances to be played in parallel,
        // without the need for explicit call to AudioSource->stop(). By default these would be played in parallel as many times play() or play3d() is invoked.
        m_sndWpnNew->setSingleInstance(true);
        m_sndWpnAmmo->setSingleInstance(true);
        m_sndArmor->setSingleInstance(true);
        m_sndMedkit->setSingleInstance(true);
        m_sndJetlax->setSingleInstance(true);
        m_sndJumppad->setSingleInstance(true);

        // These are hearable by other players as well so min/max distance is very important.
        // Those can be set for the audio source instances as well after starting playing, but we just set them for the audio sources here because
        // we dont need different values for each instance.
        // And for these obviously we dont need to set "single instance" since they are allowed to be played in multiple instances in parallel.
        m_sndFallYell_1->set3dMinMaxDistance(SndPlayerHighFallYellDistMin, SndPlayerHighFallYellDistMax);
        m_sndFallYell_1->set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);

        m_sndFallYell_2->set3dMinMaxDistance(SndPlayerHighFallYellDistMin, SndPlayerHighFallYellDistMax);
        m_sndFallYell_2->set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);

        m_sndPlayerItemActivateAntiGravity->set3dMinMaxDistance(SndPlayerItemActivateAntiGravityDistMin, SndPlayerItemActivateAntiGravityDistMax);
        m_sndPlayerItemActivateAntiGravity->set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);

        m_sndPlayerItemDeactivateAntiGravity->set3dMinMaxDistance(SndPlayerItemDeactivateAntiGravityDistMin, SndPlayerItemDeactivateAntiGravityDistMax);
        m_sndPlayerItemDeactivateAntiGravity->set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);

        m_sndPlayerItemLowThrustAntiGravity->set3dMinMaxDistance(SndPlayerItemLowThrustAntiGravityDistMin, SndPlayerItemLowThrustAntiGravityDistMax);
        m_sndPlayerItemLowThrustAntiGravity->set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
        m_sndPlayerItemLowThrustAntiGravity->setLooping(true);

        m_sndPlayerItemHighThrustAntiGravity->set3dMinMaxDistance(SndPlayerItemHighThrustAntiGravityDistMin, SndPlayerItemHighThrustAntiGravityDistMax);
        m_sndPlayerItemHighThrustAntiGravity->set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
        m_sndPlayerItemHighThrustAntiGravity->setLooping(true);

        m_sndPlayerLandSmallFall->set3dMinMaxDistance(SndPlayerLandedDistMin, SndPlayerLandedDistMax);
        m_sndPlayerLandSmallFall->set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);

        m_sndPlayerLandBigFall->set3dMinMaxDistance(SndPlayerLandedDistMin, SndPlayerLandedDistMax);
        m_sndPlayerLandBigFall->set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);

        m_sndPlayerDamage->set3dMinMaxDistance(SndPlayerLandedDistMin, SndPlayerLandedDistMax);
        m_sndPlayerDamage->set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);

        for (auto& ptr : m_sndPlayerFootstep)
        {
            ptr->set3dMinMaxDistance(SndPlayerFootstepDistMin, SndPlayerFootstepDistMax);
            ptr->set3dAttenuation(SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
        }
    }
}

proofps_dd::Player::Player(const proofps_dd::Player& other) :
    m_connHandleServerSide(other.m_connHandleServerSide),
    m_sIpAddress(other.m_sIpAddress),
    m_sName(other.m_sName),
    m_bExpectingAfterBootUpDelayedUpdate(other.m_bExpectingAfterBootUpDelayedUpdate),
    m_vecOldNewValues(other.m_vecOldNewValues),
    m_bNetDirty(other.m_bNetDirty),
    m_timeDied(other.m_timeDied),
    m_bRespawn(other.m_bRespawn),
    m_vecImpactForce(other.m_vecImpactForce),
    m_vecAntiGravityForce(other.m_vecAntiGravityForce),
    m_pObj(PGENULL),
    m_pTexPlayerStand(PGENULL),
    m_pTexPlayerCrouch(PGENULL),
    m_wpnMgr(other.m_audio, other.m_cfgProfiles, other.m_gfx, other.m_bullets),
    m_audio(other.m_audio),
    m_cfgProfiles(other.m_cfgProfiles),
    m_bullets(other.m_bullets),
    m_eventsItemPickup(other.m_eventsItemPickup),
    m_eventsInventoryChange(other.m_eventsInventoryChange),
    m_eventsAmmoChange(other.m_eventsAmmoChange),
    m_gfx(other.m_gfx),
    m_network(other.m_network),
    m_vecJumpForce(other.m_vecJumpForce),
    m_fGravity(other.m_fGravity),
    m_bJumping(other.m_bJumping),
    m_bAllowJump(other.m_bAllowJump),
    m_fWillJumpMultFactorX(other.m_fWillJumpMultFactorX),
    m_fWillJumpMultFactorY(other.m_fWillJumpMultFactorY),
    m_bWillWallJump(other.m_bWillWallJump),
    m_nConsecutiveWallJump(other.m_nConsecutiveWallJump),
    m_bCanFall(other.m_bCanFall),
    m_bFalling(other.m_bFalling),
    m_bHasJustStartedFallingNaturally(other.m_bHasJustStartedFallingNaturally),
    m_bHasJustStartedFallingAfterJumpingStopped(other.m_bHasJustStartedFallingAfterJumpingStopped),
    m_fHeightStartedFalling(other.m_fHeightStartedFalling),
    m_bHasJustStoppedJumping(other.m_bHasJustStoppedJumping),
    m_bCrouchingStateCurrent(other.m_bCrouchingStateCurrent),
    m_bCrouchingWasActiveWhenInitiatedJump(other.m_bCrouchingWasActiveWhenInitiatedJump),
    m_bJumpWasInitiatedByJumppad(other.m_bJumpWasInitiatedByJumppad),
    m_bWantToStandup(other.m_bWantToStandup),
    m_bWillSomersault(other.m_bWillSomersault),
    m_fSomersaultAngleZ(other.m_fSomersaultAngleZ),
    m_bRunning(other.m_bRunning),
    m_bJustCreatedAndExpectingStartPos(other.m_bJustCreatedAndExpectingStartPos),
    m_strafe(other.m_strafe),
    m_prevActualStrafe(other.m_prevActualStrafe),
    m_bAttack(other.m_bAttack),
    m_bHasJetLax(other.m_bHasJetLax),
    m_bHasAntiGravityActive(other.m_bHasAntiGravityActive),
    m_nParticleEmitPerNthPhysicsIterCntr(other.m_nParticleEmitPerNthPhysicsIterCntr)
{
    BuildPlayerObject(true);
}

// TODO: delete?
//proofps_dd::Player& proofps_dd::Player::operator=(const proofps_dd::Player& other)
//{
//    m_connHandleServerSide = other.m_connHandleServerSide;
//    m_sIpAddress = other.m_sIpAddress;
//    m_sName = other.m_sName;
//    m_bExpectingAfterBootUpDelayedUpdate = other.m_bExpectingAfterBootUpDelayedUpdate;
//    m_vecOldNewValues = other.m_vecOldNewValues;
//    m_bNetDirty = other.m_bNetDirty;
//    m_timeDied = other.m_timeDied;
//    m_bRespawn = other.m_bRespawn;
//    m_vecImpactForce = other.m_vecImpactForce;
//    m_vecAntiGravityForce = other.m_vecAntiGravityForce;
//    //m_audio = other.m_audio;  // deleted function
//    //m_cfgProfiles = other.m_cfgProfiles;  // inaccessible
//    //m_bullets = other.m_bullets;  // deleted
//    //m_eventsItemPickup = other.m_eventsItemPickup;  // inaccessible
//    //m_eventsInventoryChange = other.m_eventsInventoryChange;
//    //m_eventsAmmoChange = other.m_eventsAmmoChange;  // inaccessible
//    m_gfx = other.m_gfx;
//    m_network = other.m_network;
//    m_vecJumpForce = other.m_vecJumpForce;
//    m_fGravity = other.m_fGravity;
//    m_bJumping = other.m_bJumping;
//    m_bAllowJump = other.m_bAllowJump;
//    m_fWillJumpMultFactorX = other.m_fWillJumpMultFactorX;
//    m_fWillJumpMultFactorY = other.m_fWillJumpMultFactorY;
//    m_bCanFall = other.m_bCanFall;
//    m_bFalling = other.m_bFalling;
//    m_bHasJustStartedFallingNaturally = other.m_bHasJustStartedFallingNaturally;
//    m_bHasJustStartedFallingAfterJumpingStopped = other.m_bHasJustStartedFallingAfterJumpingStopped;
//    m_fHeightStartedFalling = other.m_fHeightStartedFalling;
//    m_bHasJustStoppedJumping = other.m_bHasJustStoppedJumping;
//    m_bCrouchingStateCurrent = other.m_bCrouchingStateCurrent;
//    m_bCrouchingWasActiveWhenInitiatedJump = other.m_bCrouchingWasActiveWhenInitiatedJump;
//    m_bJumpWasInitiatedByJumppad = other.m_bJumpWasInitiatedByJumppad;
//    m_bWantToStandup = other.m_bWantToStandup;
//    m_bWillSomersault = other.m_bWillSomersault;
//    m_fSomersaultAngleZ = other.m_fSomersaultAngleZ;
//    m_bRunning = other.m_bRunning;
//    m_bJustCreatedAndExpectingStartPos = other.m_bJustCreatedAndExpectingStartPos;
//    m_strafe = other.m_strafe;
//    m_prevActualStrafe = other.m_prevActualStrafe;
//    m_bAttack = other.m_bAttack;
//
//    BuildPlayerObject(true);
//
//    return *this;
//}

proofps_dd::Player::~Player()
{
    forceDeactivateCurrentInventoryItem();

    if (m_pObj)
    {
        delete m_pObj;  // yes, dtor will remove this from its Object3DManager too!
    }

    // Dont telete player textures here because other Player instances refer to the same textures.
    // Unfortunately the engine still doesn't use reference counting for textures.
    // Anyway, the engine takes care of deleting textures at shutdown.

    m_wpnMgr.Clear();
}

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::Player::getTimeConstructed() const
{
    return m_timeCtor;
}

/**
* @return The timestamp when this player has been marked as "booted up".
*         See setTimeBootedUp() doc for more details.
*/
const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::Player::getTimeBootedUp() const
{
    return m_timeBootedUp;
}

/**
* Based on PRooFPS v0.5 network protocol, this is set when MsgUserNameChangeAndBootupDone is processed.
* This is processed by all network instances on their side, therefore expected to be valid in every network instance for every player.
*/
void proofps_dd::Player::setTimeBootedUp()
{
    m_timeBootedUp = std::chrono::steady_clock::now();
}

/**
* @return True if there is a valid timestamp set for bootup for this player, false otherwise.
*         See setTimeBootedUp() doc for more details.
*/
bool proofps_dd::Player::hasBootedUp() const
{
    return m_timeBootedUp.time_since_epoch().count() != 0;
}

bool proofps_dd::Player::isExpectingAfterBootUpDelayedUpdate() const
{
    return m_bExpectingAfterBootUpDelayedUpdate;
}

void proofps_dd::Player::setExpectingAfterBootUpDelayedUpdate(bool b)
{
    m_bExpectingAfterBootUpDelayedUpdate = b;
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

/**
* Invoked in every frame.
* 
* @param bAllowedForGameplay Shall come from GameMode::isPlayerAllowedForGameplay().
*/
void proofps_dd::Player::updateAudioVisuals(const proofps_dd::Config& config, bool bServer, bool bAllowedForGameplay)
{
    if (!bAllowedForGameplay)
    {
        hide();
        forceDeactivateCurrentInventoryItem();
        return;
    }

    if (getInvulnerability())
    {
        constexpr auto nBlinkPeriodMillisecs = 100;
        const auto nInvulTimeElapsedMillisecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_timeStartedInvulnerability).count();
        // only server instance is allowed to decide about ENDING invulnerability, client will be notified over network msg
        const bool bEndInvul = bServer && (nInvulTimeElapsedMillisecs >= static_cast<long long>(config.getPlayerRespawnInvulnerabilityDelaySeconds()) * 1000);
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

    // Player-related 3D sounds that are relatively longer than others need continuous position update because the player
    // AND other players need to continuously hear them from the updated position e.g. we want them experience the sound coming down from above with the falling player :)
    const PureVector& playerPos = getPos().getNew();
    if (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleFallYell))
    {
        m_audio.getAudioEngineCore().set3dSourcePosition(m_handleFallYell, playerPos.getX(), playerPos.getY(), playerPos.getZ());
    }
    if (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerItemLowThrustAntiGravity))
    {
        m_audio.getAudioEngineCore().set3dSourcePosition(m_handleSndPlayerItemLowThrustAntiGravity, playerPos.getX(), playerPos.getY(), playerPos.getZ());
    }
    if (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerItemHighThrustAntiGravity))
    {
        m_audio.getAudioEngineCore().set3dSourcePosition(m_handleSndPlayerItemHighThrustAntiGravity, playerPos.getX(), playerPos.getY(), playerPos.getZ());
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

/**
* Sets visibility state for BOTH the player object and the current weapon.
*/
void proofps_dd::Player::setVisibilityState(bool state)
{
    getObject3D()->SetRenderingAllowed(state);
    if (m_wpnMgr.getCurrentWeapon())
    {
        m_wpnMgr.getCurrentWeapon()->getObject3D().SetRenderingAllowed(state);
    }
}

/**
* @return True if either the player object or the current weapon is visible.
*         False if neither the player object nor the current weapon is visible.
*/
bool proofps_dd::Player::isVisible() const
{
    bool bVisible = getObject3D()->isRenderingAllowed();
    if (m_wpnMgr.getCurrentWeapon())
    {
        bVisible = bVisible || m_wpnMgr.getCurrentWeapon()->getObject3D().isRenderingAllowed();
    }
    return bVisible;
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

const PgeOldNewValue<int>& proofps_dd::Player::getArmor() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvArmor));
}

void proofps_dd::Player::setArmor(int value) {
    getArmor().set(std::max(0, std::min(value, 100)));
}

const PgeOldNewValue<int>& proofps_dd::Player::getHealth() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvHealth));
}

void proofps_dd::Player::setHealth(int value) {
    getHealth().set(std::max(0, std::min(value, 100)));
}

/**
* Input can be the calculated damage caused by bullet, or falling from high, etc.
* AP is able to decrease the suffered HP damage, but not vice versa.
* This means a player can die even if AP stays positive.
* 
* @param dmgAP The input damage that will be applied to AP.
*              It simply decreases AP by this value.
* @param dmgHP The input damage that will be applied to HP.
*              This damage is decreased by the decreased AP before being applied to HP.
*/
void proofps_dd::Player::doDamage(int dmgAP, int dmgHP) {
    if ((dmgAP < 0) || (dmgHP < 0))
    {
        getConsole().EOLn("Player::%s(): INVALID dmgAP: %d or dmgHP: %d", __func__, dmgAP, dmgHP);
        return;
    }

    getArmor().set(
        std::max(0, getArmor().getNew() - dmgAP)
    );

    const int newDmgHp = static_cast<int>(std::lroundf((
        1.f - (getArmor().getNew() / 100.f)) * dmgHP
    ));
    getHealth().set(
        std::max(0, getHealth().getNew() - newDmgHp)
    );
}

PureVector& proofps_dd::Player::getImpactForce()
{
    return m_vecImpactForce;
}

void proofps_dd::Player::die(bool bMe, bool bServer)
{
    // TODO: in newer version Player already has Network instance, so bServer here is obsolete!
    m_timeDied = std::chrono::steady_clock::now();
    m_audio.stopSoundInstance(m_handleFallYell);
    m_audio.stopSoundInstance(m_handleSndPlayerItemDeactivateAntiGravity);
    forceDeactivateCurrentInventoryItem();
    if (bMe)
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): I died!", __func__);
    }
    else
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): other player died!", __func__);
    }
    setHealth(0);
    setHasAntiGravityActive(false);
    setHasJetLax(false);
    getAttack() = false;
    hide();
    if (bServer)
    {
        m_bFallingHighTriggered = false;
        setStrafe(Strafe::NONE);
        m_prevActualStrafe = Strafe::NONE;
        setWillJumpInNextTick(0.f, 0.f);
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

void proofps_dd::Player::moveTimeDiedEarlier(std::chrono::milliseconds::rep byMillisecs)
{
    assert(byMillisecs > 0);

    if ((std::chrono::duration_cast<std::chrono::milliseconds>(m_timeDied.time_since_epoch())).count() < byMillisecs)
    {
        return;
    }

    m_timeDied -= std::chrono::milliseconds(byMillisecs);
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

PgeOldNewValue<unsigned int>& proofps_dd::Player::getSuicides()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<unsigned int>>(m_vecOldNewValues.at(OldNewValueName::OvSuicides));
}

const PgeOldNewValue<unsigned int>& proofps_dd::Player::getSuicides() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<unsigned int>>(m_vecOldNewValues.at(OldNewValueName::OvSuicides));
}

PgeOldNewValue<float>& proofps_dd::Player::getFiringAccuracy()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<float>>(m_vecOldNewValues.at(OldNewValueName::OvFiringAccuracy));
}

const PgeOldNewValue<float>& proofps_dd::Player::getFiringAccuracy() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<float>>(m_vecOldNewValues.at(OldNewValueName::OvFiringAccuracy));
}

PgeOldNewValue<unsigned int>& proofps_dd::Player::getShotsFiredCount()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<unsigned int>>(m_vecOldNewValues.at(OldNewValueName::OvShotsFired));
}

const PgeOldNewValue<unsigned int>& proofps_dd::Player::getShotsFiredCount() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<unsigned int>>(m_vecOldNewValues.at(OldNewValueName::OvShotsFired));
}

unsigned int& proofps_dd::Player::getShotsHitTarget()
{
    return m_nShotsHitTarget;
}

const unsigned int& proofps_dd::Player::getShotsHitTarget() const
{
    return m_nShotsHitTarget;
}

bool& proofps_dd::Player::getRespawnFlag()
{
    return m_bRespawn;
}

void proofps_dd::Player::respawn(bool /*bMe*/, const Weapon& wpnDefaultAvailable, bool bServer)
{
    // Remember this function is NOT invoked when just connected to a new game!
    // For setting initial stuff after connect, use handleUserUpdateFromServer(), where it checks for isJustCreatedAndExpectingStartPos()!
    // 
    //getConsole().EOLn(
    //    "PRooFPSddPGE::%s(): jumpforce: %f, %f, %f, gravity: %f",
    //    __func__,
    //    getJumpForce().getX(),
    //    getJumpForce().getY(),
    //    getJumpForce().getZ(),
    //    getGravity());

    doStandupShared();
    getWantToStandup() = true;
    getImpactForce().SetZero();
    getAntiGravityForce().SetZero();
    getJumpForce().Set(0.f, 0.f, 0.f);
    setGravity(0.f);
    setHasJustStartedFallingNaturallyInThisTick(true);  // make sure vars for calculating high fall are reset
    m_prevActualStrafe = Strafe::NONE;
    setArmor(0);
    forceDeactivateCurrentInventoryItem();
    setHasAntiGravityActive(false);
    setHasJetLax(false);

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

        // and this is when the "default available weapon" concept is not necessarily correct because from v0.3.0 knife also becomes available by default:
        Weapon* const wpnKnife = m_wpnMgr.getWeaponByFilename("knife.txt");
        wpnKnife->SetAvailable(true);
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

bool proofps_dd::Player::isJustCreatedAndExpectingStartPos() const
{
    return m_bJustCreatedAndExpectingStartPos;
}

void proofps_dd::Player::setJustCreatedAndExpectingStartPos(bool b)
{
    m_bJustCreatedAndExpectingStartPos = b;
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

PgeOldNewValue<bool>& proofps_dd::Player::getJumpInput()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<bool>>(m_vecOldNewValues.at(OldNewValueName::OvJumpInput));
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

    if (!state)
    {
        m_bFallingHighTriggered = false;
    }
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

        //getConsole().EOLn("Player::%s(): m_fHeightStartedFalling: %f!", __func__, m_fHeightStartedFalling);
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

bool proofps_dd::Player::jumpAllowed() const
{
    return m_bAllowJump;
}

void proofps_dd::Player::setJumpAllowed(bool b)
{
    m_bAllowJump = b;
}

/**
* Triggers a jump.
* Prior calling to setWillJumpInNextTick() is mandatory for setting up jump multiplier factors for calculating forces.
* 
* Cannot trigger another jump if already jumping or somersaulting.
* 
* Since jump() is also used to trigger jumppad-induced jumps, that could also trigger horizontal force,
* we have fRunSpeedPerTickForJumppadHorizontalForce optional argument for such calculation.
* 
* @param fRunSpeedPerTickForJumppadHorizontalForce Optional, only used in case of jumppad-induced jumps to calculate horizontal jump force.
*/
void proofps_dd::Player::jump(const float& fRunSpeedPerTickForJumppadHorizontalForce)
{
    // 2025-08-20: yes, for some reason I need to zero willjumpmultfactors here even before returning.
    // otherwise, extra jump can be triggered mid-air. Did not have time to debug the reason, but
    // that is why I need to introduce a separate m_bWillWallJump to trigger wall jumping.
    const float fOriginalWillJumpMultFactorX = m_fWillJumpMultFactorX;
    const float fOriginalWillJumpMultFactorY = m_fWillJumpMultFactorY;
    m_fWillJumpMultFactorX = 0.f;
    m_fWillJumpMultFactorY = 0.f;

    if (isJumping() || !jumpAllowed() || isSomersaulting())
    {
        return;
    }

    m_bAllowJump = false;
    m_bWillWallJump = false;
    m_bJumping = true;
    m_bFalling = false;
    m_bCrouchingWasActiveWhenInitiatedJump = getCrouchInput().getNew();

    if (fOriginalWillJumpMultFactorY == 1.f)
    {
        // this looks to be a regular player input-induced jump;
        // it is not an issue if it is actually by a jumppad, because anyway who would make jumppad with 1.f mult factor?!
        // So anyone who want their jumppad-induced jump to be properly identified, should use non-1.f mult factor.
        m_fGravity = m_bCrouchingWasActiveWhenInitiatedJump ? proofps_dd::Player::fJumpGravityStartFromCrouching : proofps_dd::Player::fJumpGravityStartFromStanding;
        m_bJumpWasInitiatedByJumppad = false;
        m_vecJumpForce.SetX(getPos().getNew().getX() - getPos().getOld().getX());
    }
    else
    {
        // this looks to be a jumppad-induced jump
        m_fGravity = fOriginalWillJumpMultFactorY * proofps_dd::Player::fJumpGravityStartFromStanding;
        m_bJumpWasInitiatedByJumppad = true;
        m_vecJumpForce.SetX(fOriginalWillJumpMultFactorX * fRunSpeedPerTickForJumppadHorizontalForce);
    }

    //static int nJumpCounter = 0;
    //nJumpCounter++;
    //getConsole().EOLn("%s() %d: fGravity: %f, crouchInput: %b ", __func__, nJumpCounter, m_fGravity, m_bCrouchingWasActiveWhenInitiatedJump);

    
    // we dont use other components of jumpForce vec, since Z-axis is "unused", Y-axis jump force is controlled by m_fGravity 
    //m_vecJumpForce.SetY(getPos().getNew().getY() - getPos().getOld().getY());
    //m_vecJumpForce.SetZ(getPos().getNew().getZ() - getPos().getOld().getZ());
    //getConsole().EOLn("jump x force: %f", m_vecJumpForce.getX());
}

void proofps_dd::Player::stopJumping()
{
    m_bJumping = false;
    m_bWillWallJump = false;
    m_nConsecutiveWallJump = 0;
}

float proofps_dd::Player::getWillJumpXInNextTick() const
{
    return m_fWillJumpMultFactorX;
}

float proofps_dd::Player::getWillJumpYInNextTick() const
{
    return m_fWillJumpMultFactorY;
}

/**
* Sets the multipliers for jump force that will be used for the next jump.
* The jump will actually happen in the current or in the next tick, depending on WHEN this function is called.
* See Physics code for details.
* 
* @param factorY Multiplier for Player::fJumpGravityStartFromStanding. Use 1.0 to trigger a regular jump from standing position.
*                Any value that is not 0 or 1.0 is considered to be a jumppad-induced jump when jump() is invoked.
*                Use 0 to cancel the jump triggering in the current or in the next tick.
*/
void proofps_dd::Player::setWillJumpInNextTick(float factorY, float factorX)
{
    if (!jumpAllowed())
    {
        return;
    }

    m_fWillJumpMultFactorY = factorY;
    m_fWillJumpMultFactorX = factorX;
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

bool proofps_dd::Player::getWillWallJumpInNextTick() const
{
    return m_bWillWallJump;
}

//bool proofps_dd::Player::getConsecutiveWallJump() const
//{
//    return m_nConsecutiveWallJump != 0;
//}

/**
* Sets the multipliers for jump force that will be used for the next wall jump.
* The wall jump will actually happen in the current or in the next tick, depending on WHEN this function is called.
* See Physics code for details.
* 
* Unlike a regular jump, wall jump can be initiated only if the player has already initiated a regular jump before, and
* the player is still "in" that regular jump i.e. not yet started to fall down or already on ground.
* Additional requirement is that the player must look in the opposite direction of the wall they are strafing against.
* So if player is strafing towards left wall, they shall look to the right when triggering wall jump.
*/
void proofps_dd::Player::setWillWallJumpInNextTick(/*float factorY, float factorX*/)
{
    //getConsole().EOLn("1 set");
    // TODO: input arguments shall be similar to setWillJumpInNextTick(), so PurePosUpTarget shall be
    // calculated outside or even here instead of wallJump(), I'm not 100% sure about this though.
    if (!isJumping())
    {
        return;
    }

    //getConsole().EOLn("2 set");
    // consecutive wall jumps use saved angles
    if (m_nConsecutiveWallJump == 0)
    {
        // WeaponAngle is synced properly with clients too.
        // Also, it is better to use weapon angle for this, because if startSomersaultServer() is triggered
        // also during the jump, it changes the player's Z angle, without altering weapon's Z angle, so
        // here we still have the unchanged weapon angle.
        // Weapon Z angle is changed by somersaulting only in consecutive ticks by stepSomersaultAngleServer().
        // We are not impacted in chained wall jumping either by using this saved value and just negating
        // its direction in consecutive wall jumps.
        m_angleSavedForWallJump = getWeaponAngle().getNew();
    }
    // Z-angle is in [-90, 90] range, let's restrict it a bit so full vertical walljumps will
    // NOT be allowed, because it is unreal.
    // We don't care about this in consecutive wall jumps because those are using saved angles.
    if ((m_nConsecutiveWallJump == 0) &&
        ((m_angleSavedForWallJump.getZ() < -70.f) || (m_angleSavedForWallJump.getZ() > 70.f)))
    {
        return;
    }

    //getConsole().EOLn("3 set");
    // player needs to face opposite direction relative to strafing if this is the first wall jump in a row,
    // since we are pushing ourselves from the walls, however strafing should be considered only
    // during the first jump, consecutive jumps should be easier.
    if ((m_nConsecutiveWallJump == 0) &&
        (((m_strafe == Strafe::LEFT) && (m_angleSavedForWallJump.getY() == 0.f))
         ||
         ((m_strafe == Strafe::RIGHT) && (m_angleSavedForWallJump.getY() == 180.f))
        )
       )
    {
        return;
    }

    static int dssdfgs = 0;
    dssdfgs++;
    //getConsole().EOLn("will wall jump really set %d, consecutive: %d", dssdfgs, m_nConsecutiveWallJump);

    m_bWillWallJump = true;
    m_nConsecutiveWallJump++;
    //m_fWillJumpMultFactorY = factorY;
   // m_fWillJumpMultFactorX = factorX;
    m_timeLastWillJump = std::chrono::steady_clock::now();
}

/**
* Resets getWillWallJumpInNextTick().
* In some case, a triggered wall jump should be cancelled.
* Since wall jumping is triggered by player in input handling, but actually triggered in physics, we
* have to cancel the player-triggered wall jump in physics if we detect no actual horizontal wall collision.
* Otherwise, the wall jump would be really triggered by physics later due to the set flag.
*/
void proofps_dd::Player::cancelWillWallJump()
{
    m_bWillWallJump = false;
}

void proofps_dd::Player::wallJump(/*const float& fRunSpeedPerTickForJumppadHorizontalForce*/)
{
    m_bWillWallJump = false;
    m_bJumping = true;
  
    PurePosUpTarget put;
    put.SetRotation(
        0.f,
        (m_angleSavedForWallJump.getY() > 0.0f) ? 90.f : -90.f /* getY() is either 0 or 180, that is why we set 90 or -90 */,
        (m_angleSavedForWallJump.getY() > 0.0f) ? m_angleSavedForWallJump.getZ() : -m_angleSavedForWallJump.getZ());

    //const float fOldIdeaY = proofps_dd::Player::fJumpGravityStartFromStanding;
    float fNewIdeaY = put.getTargetVec().getY() * proofps_dd::Player::fJumpGravityStartFromStanding * 1.5f;
    fNewIdeaY = PFL::constrain(fNewIdeaY, fNewIdeaY, proofps_dd::Player::fJumpGravityStartFromStanding);
    m_fGravity = fNewIdeaY;

    // unlike with regular jump, here we have to set negative horizontal force since we do it against the wall,
    // and we set bigger force, because it needs to overcome the strafe speed and its negative direction,
    // since we are strafing TOWARDS the wall but we want to jump AGAINST it! And in-air strafe might be enabled, so
    // essentially we are fighting against in-air strafe with this negated force.
    //const float fOldIdeaX = -getStrafeSpeed() * 1.5f;

    const float fNewIdeaX = put.getTargetVec().getX() / 6.f;
    m_vecJumpForce.SetX(fNewIdeaX);

    //getConsole().EOLn("Original Idea: gravity: %f, wall jump x force: %f",
    //    fOldIdeaY,
    //    fOldIdeaX);
    //getConsole().EOLn("New Idea     : put y: %f, put x: %f, angle Z: %f",
    //    fNewIdeaY,
    //    fNewIdeaX,
    //    m_angleSavedForWallJump.getZ());

    // have to negate the horizontal direction so a next consecutive wall jump can be in the reverse direction
    //getConsole().EOLn("old m_angleSavedForWallJump Y: %f, Z: %f", m_angleSavedForWallJump.getY(), m_angleSavedForWallJump.getZ());
    m_angleSavedForWallJump.SetY(
        (m_angleSavedForWallJump.getY() > 0) ?
        0.f :
        180.f);
    //getConsole().EOLn("new m_angleSavedForWallJump Y: %f, Z: %f", m_angleSavedForWallJump.getY(), m_angleSavedForWallJump.getZ());
}

int& proofps_dd::Player::getTicksSinceLastHorizontalCollision()
{
    return m_nTicksSinceLastHorizontalCollision;
}

bool proofps_dd::Player::hasJetLax() const
{
    return m_bHasJetLax;
}

void proofps_dd::Player::setHasJetLax(bool state)
{
    m_bHasJetLax = state;
}

bool proofps_dd::Player::hasAntiGravityActive() const
{
    return m_bHasAntiGravityActive;
}

void proofps_dd::Player::setHasAntiGravityActive(bool state)
{
    m_bHasAntiGravityActive = state && hasJetLax();
    m_timeLastToggleUseItem = std::chrono::steady_clock::now();
}

PureVector& proofps_dd::Player::getAntiGravityForce()
{
    return m_vecAntiGravityForce;
}

/**
* Unlike with other PgeOldNewValue functions, we do not define a non-const version of
* getCurrentInventoryItemPower(), so this way we force that both server- and client instances
* must call this function for updating inventory item power.
* Reason: originally we wanted to use this as a convenient and reliable way to control the
* inventory item sound based on power drop (low drop vs high drop) at a single location.
* But the server still invokes this 2 times per frame, from Physics' serverUpdateAntiGravityForce()
* and that messed up this plan. So we now control sound in a separate function:
* updateCurrentInventoryItemPowerAudioVisualsShared().
*/
void proofps_dd::Player::setCurrentInventoryItemPower(const float& newValue)
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    PgeOldNewValue<float>& ovCurrentInventoryItemPower =
        std::get<PgeOldNewValue<float>>(m_vecOldNewValues.at(OldNewValueName::OvCurrentInventoryItemPower));

    ovCurrentInventoryItemPower = newValue;
}

const PgeOldNewValue<float>& proofps_dd::Player::getCurrentInventoryItemPower() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<float>>(m_vecOldNewValues.at(OldNewValueName::OvCurrentInventoryItemPower));
}

/**
* For this audio-visual control to work properly, this function shall be invoked MAX 1 time per frame!
* 
* Server instance shall invoke it after it updated current inventory item power for the last time in the current frame,
* but before clearing dirtiness of old-new values. Basically it is invoked at tick-rate.
* 
* Client instance shall invoke it before clearing dirtiness of old-new values.
*/
void proofps_dd::Player::updateCurrentInventoryItemPowerAudioVisualsShared(
    const proofps_dd::Config& config,
    PgeObjectPool<Smoke>& smokes)
{   
    // for now this function assumes inventory item can be only JETLAX, will need to generalize more!
    
    if (!hasAntiGravityActive())
    {
        // TODO
        return;
    }

    const PgeOldNewValue<float>& ovCurrentInventoryItemPower = getCurrentInventoryItemPower();
    const float fPowerDiff = ovCurrentInventoryItemPower.getNew() - ovCurrentInventoryItemPower.getOld();
    if (fPowerDiff < 0.f)
    {
        // drop in item power

        //getConsole().EOLn(
        //    "diff: %f",
        //    (ovCurrentInventoryItemPower.getNew() - ovCurrentInventoryItemPower.getOld()));

        emitParticles(config, smokes);

        if (fPowerDiff > -0.05f)
        {
            // low power drop
            if (!m_bSndPlayerItemLowThrustAntiGravityLoopingStarted &&
                !m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerItemLowThrustAntiGravity))
            {
                m_bSndPlayerItemLowThrustAntiGravityLoopingStarted = true;
                m_handleSndPlayerItemLowThrustAntiGravity = m_audio.play3dSound(
                    *m_sndPlayerItemLowThrustAntiGravity, getPos().getNew(), false /* bKillWhenInAudible */);
            }
            m_bSndPlayerItemHighThrustAntiGravityLoopingStarted = false;
            if (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerItemHighThrustAntiGravity))
            {
                m_audio.stopSoundInstance(m_handleSndPlayerItemHighThrustAntiGravity);
            }
        }
        else
        {
            // high power drop
            if (!m_bSndPlayerItemHighThrustAntiGravityLoopingStarted &&
                !m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerItemHighThrustAntiGravity))
            {
                m_bSndPlayerItemHighThrustAntiGravityLoopingStarted = true;
                m_handleSndPlayerItemHighThrustAntiGravity = m_audio.play3dSound(
                    *m_sndPlayerItemHighThrustAntiGravity, getPos().getNew(), false /* bKillWhenInAudible */);
            }
            m_bSndPlayerItemLowThrustAntiGravityLoopingStarted = false;
            if (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerItemLowThrustAntiGravity))
            {
                m_audio.stopSoundInstance(m_handleSndPlayerItemLowThrustAntiGravity);
            }
        }
    }
}

void proofps_dd::Player::forceDeactivateCurrentInventoryItem()
{
    // invoked by both server and client instances,
    // useful when we need to cut any active item-related audio, etc., for example, when
    // a game ends and we are displaying the results, players don't die but
    // still shall stop their activities, like looping inventory item sounds.

    m_bSndPlayerItemHighThrustAntiGravityLoopingStarted = false;
    if (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerItemHighThrustAntiGravity))
    {
        m_audio.stopSoundInstance(m_handleSndPlayerItemHighThrustAntiGravity);
    }

    m_bSndPlayerItemLowThrustAntiGravityLoopingStarted = false;
    if (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerItemLowThrustAntiGravity))
    {
        m_audio.stopSoundInstance(m_handleSndPlayerItemLowThrustAntiGravity);
    }

    setHasAntiGravityActive(false);
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

const bool& proofps_dd::Player::getCrouchStateCurrent() const
{
    return m_bCrouchingStateCurrent;
}

const bool& proofps_dd::Player::isJumpingInitiatedFromCrouching() const
{
    return m_bCrouchingWasActiveWhenInitiatedJump;
}

const bool& proofps_dd::Player::isJumpInitiatedByJumppad() const
{
    return m_bJumpWasInitiatedByJumppad;
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
        if ((isJumping() && isJumpingInitiatedFromCrouching()) || (isJumping() != bJumpInduced) || isJumpInitiatedByJumppad())
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
        if (bJumpInduced && m_cfgProfiles.getVars()[szCVarSvSomersaultMidAirAutoCrouch].getAsBool())
        {
            doCrouchServer();
        }
        else
        {
            return;
        }
    }

    // just set the initial m_fSomersaultAngleZ by setting a small value, so stepSomersaultAngleServer() will know in which direction it should continue changing angle
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
        m_vecJumpForce.SetX(m_vecJumpForce.getX() * m_cfgProfiles.getVars()[szCVarSvSomersaultMidAirJumpForceMultiplier].getAsFloat());
        //getConsole().EOLn("somersault set jump force x: %f", m_vecJumpForce.getX());
        m_fGravity *= m_cfgProfiles.getVars()[szCVarSvSomersaultMidAirJumpForceMultiplier].getAsFloat();
    }
    else
    {
        // triggering on-ground somersaulting modifies impact force only, not jump force or gravity
        switch (m_strafe)
        {
        case Strafe::LEFT:
            if (m_vecImpactForce.getX() > -fSomersaultGroundImpactForceX)
            {
                m_vecImpactForce.SetX(std::max(-fSomersaultGroundImpactForceX, m_vecImpactForce.getX() - fSomersaultGroundImpactForceX));
            }
            break;
        case Strafe::RIGHT:
            if (m_vecImpactForce.getX() < fSomersaultGroundImpactForceX)
            {
                m_vecImpactForce.SetX(std::min(fSomersaultGroundImpactForceX, m_vecImpactForce.getX() + fSomersaultGroundImpactForceX));
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

const std::chrono::time_point<std::chrono::steady_clock>& proofps_dd::Player::getTimeLastToggleUseItem() const
{
    return m_timeLastToggleUseItem;
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
    else
    {
        m_strafeSpeed = 0.f;
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

float& proofps_dd::Player::getStrafeSpeed()
{
    return m_strafeSpeed;
}

const PgeOldNewValue<bool>& proofps_dd::Player::getActuallyRunningOnGround() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<bool>>(m_vecOldNewValues.at(OldNewValueName::OvActuallyRunningOnGround));
}

PgeOldNewValue<bool>& proofps_dd::Player::getActuallyRunningOnGround()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<bool>>(m_vecOldNewValues.at(OldNewValueName::OvActuallyRunningOnGround));
}

/**
* Tells if the player is stationary or not.
* This function was created to be used in any phase of a frame.
* There are some situations when checking (getPos().getOld() == getPos().getNew()) won't work, for
* example, if we are checking that condition before the next physics tick in the current frame.
* It might use previous frame's data, so it should be used only if the truth of the returned value being
* false for or late by 1 frame is not a problem.
* 
* Can be used only on server instance, as client returns false info due to lack of some data.
* 
* @return True if player is considered to be moving based on previous or current frame data, false otherwise.
*         True also if player cannot strafe due to being blocked but based on the input it tries to strafe.
*/
bool proofps_dd::Player::isMoving() const
{
    return isSomersaulting() ||
        isInAir() /* although we COULD be stationary in air, for now we always treat it as moving */ ||
        (getStrafe() != Strafe::NONE) /* this is based in player's input, NOT NECESSARILY moving, but even we are blocked by a wall, we treat it as moving */ ||
        (getPos().getOld() != getPos().getNew() /* works fine if called AFTER the current physics tick, otherwise won't */);
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

    /* Invoked by serverUpdateWeapons() thus passing bMoving as old vs new positions won't work, because
       the physics tick will come later. Thus, we need to use previous tick's data to understand if we
       are moving or not. */

    const bool bRet = wpn->pullTrigger(
        isMoving() && m_cfgProfiles.getVars()[szCVarSvMovingAffectsAim].getAsBool(),
        isRunning(),
        getCrouchStateCurrent()
    );

    if (bRet && (wpn->getType() != Weapon::Type::Melee))
    {
        // intentionally not counting with melee weapons for aim accuracy stat, let them swing the knife in the air and against walls without affecting their aim accuracy stat!
        ++getShotsFiredCount();
        getFiringAccuracy() =
            (getShotsFiredCount() == 0u) ? /* just in case of overflow which will most probably never happen */
            0.f :
            (getShotsHitTarget() / static_cast<float>(getShotsFiredCount()));
    }

    return bRet;
}

const PgeOldNewValue<float>& proofps_dd::Player::getWeaponMomentaryAccuracy() const
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<float>>(m_vecOldNewValues.at(OldNewValueName::OvWpnMomentaryAccuracy));
}

/**
* This is just for server sending momentary weapon accuracy to all players, so they can scale their xhair accordingly.
* Only server is able to calculate players' aim accuracy, it is easier to simply send it to them.
*/
void proofps_dd::Player::setWeaponMomentaryAccuracy(float value)
{
    assert(value >= 0.f);

    getWeaponMomentaryAccuracy().set(value);
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

PgeOldNewValue<unsigned int>& proofps_dd::Player::getTeamId()
{
    return m_iTeamId;
}

const PgeOldNewValue<unsigned int>& proofps_dd::Player::getTeamId() const
{
    return m_iTeamId;
}

bool proofps_dd::Player::canTakeItem(const MapItem& item) const
{
    switch (item.getType())
    {
    case proofps_dd::MapItemType::ITEM_WPN_PISTOL:
    case proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN:
    case proofps_dd::MapItemType::ITEM_WPN_BAZOOKA:
    case proofps_dd::MapItemType::ITEM_WPN_PUSHA:
    case proofps_dd::MapItemType::ITEM_WPN_MACHINEPISTOL:
    case proofps_dd::MapItemType::ITEM_WPN_SHOTGUN:
    case proofps_dd::MapItemType::ITEM_WPN_GRENADELAUNCHER:
    {
        const Weapon* const pWpn = getWeaponInstanceByMapItemType(item.getType());
        if (!pWpn)
        {
            getConsole().EOLn("Player::%s(): failed to find weapon by item type %d!", __func__, item.getType());
            return false;
        }
        return pWpn->canIncBulletCount();
    }
    case proofps_dd::MapItemType::ITEM_ARMOR:
        return (getArmor() < 100);
    case proofps_dd::MapItemType::ITEM_HEALTH:
        return (getHealth() < 100);
    case proofps_dd::MapItemType::ITEM_JETLAX:
        return !hasJetLax() || (getCurrentInventoryItemPower() < 70.f);
    default:
        ;
    }
    return false;
}

/**
* @param item                    Item being picked up by the player.
* @param pktWpnUpdate            This function writes MsgWpnUpdateFromServer into this packet.
* @param bHasJustBecomeAvailable Set by function only in case of weapon items. True if player did not have this weapon before, false otherwise.
*/
void proofps_dd::Player::takeItem(MapItem& item, pge_network::PgePacket& pktWpnUpdate, bool& bHasJustBecomeAvailable)
{
    assert(m_network.isServer());

    const bool bCurrentPlayer = (getServerSideConnectionHandle() == pge_network::ServerConnHandle);

    switch (item.getType())
    {
    case proofps_dd::MapItemType::ITEM_WPN_PISTOL:
    case proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN:
    case proofps_dd::MapItemType::ITEM_WPN_BAZOOKA:
    case proofps_dd::MapItemType::ITEM_WPN_PUSHA:
    case proofps_dd::MapItemType::ITEM_WPN_MACHINEPISTOL:
    case proofps_dd::MapItemType::ITEM_WPN_SHOTGUN:
    case proofps_dd::MapItemType::ITEM_WPN_GRENADELAUNCHER:
    {
        Weapon* const pWpnBecomingAvailable = getWeaponInstanceByMapItemType(item.getType());
        if (!pWpnBecomingAvailable)
        {
            getConsole().EOLn("Player::%s(): failed to find weapon by item type %d!", __func__, item.getType());
            return;
        }

        item.take();
        int nAmmoIncrease = 0;
        if (pWpnBecomingAvailable->isAvailable())
        {
            bHasJustBecomeAvailable = false;
            // just increase bullet count
            nAmmoIncrease =
                (pWpnBecomingAvailable->getVars()["reloadable"].getAsInt() == 0) ?
                pWpnBecomingAvailable->getVars()["bullets_default"].getAsInt() :
                pWpnBecomingAvailable->getVars()["reloadable"].getAsInt();
            pWpnBecomingAvailable->IncBulletCount(nAmmoIncrease);
            //getConsole().OLn(
            //    "Player::%s(): weapon %s pickup, already available, just inc unmag to: %u",
            //    __func__, sWeaponBecomingAvailable.c_str(), pWpnBecomingAvailable->getUnmagBulletCount());
        }
        else
        {
            bHasJustBecomeAvailable = true;
            // becoming available with default bullet count
            //getConsole().OLn(
            //    "Player::%s(): weapon %s pickup, becomes available with mag: %u, unmag: %u",
            //    __func__, sWeaponBecomingAvailable.c_str(), pWpnBecomingAvailable->getMagBulletCount(), pWpnBecomingAvailable->getUnmagBulletCount());
        }

        // !!! BADDESIGN !!! Not nice, but clients play sounds for these events in handleWpnUpdateFromServer().
        // Server and client could have more shared code, as they have for example in handleTakeNonWeaponItem().
        if (bCurrentPlayer)
        {
            handleTakeWeaponItem(item.getType(), *pWpnBecomingAvailable, bHasJustBecomeAvailable, nAmmoIncrease);
        }

        pWpnBecomingAvailable->SetAvailable(true);  // becomes available on server side
        
        // this is very inconsistent: for weapon items, we inform clients with MsgWpnUpdateFromServer, but for other items, we send PlayerEventFromServer,
        // maybe in future we can clarify ...
        proofps_dd::MsgWpnUpdateFromServer::initPkt(
            pktWpnUpdate,
            0 /* ignored by client anyway */,
            pWpnBecomingAvailable->getFilename() /* TODO: hopefully soon we can get rid of this member from the msg, since item.getType() should be enough!!! */,
            item.getType(),
            pWpnBecomingAvailable->isAvailable(),
            pWpnBecomingAvailable->getState().getNew(),
            pWpnBecomingAvailable->getMagBulletCount(),
            pWpnBecomingAvailable->getUnmagBulletCount(),
            nAmmoIncrease);  // becomes available on client side (after pkt being sent)
        break;
    }
    case proofps_dd::MapItemType::ITEM_ARMOR:
        item.take();
        setArmor(getArmor() + static_cast<int>(MapItem::ITEM_ARMOR_AP_INC)); // client will learn about new AP from the usual UserUpdateFromServer
        handleTakeNonWeaponItem(MapItemType::ITEM_ARMOR, bCurrentPlayer);
        break;
    case proofps_dd::MapItemType::ITEM_HEALTH:
        item.take();
        setHealth(getHealth() + static_cast<int>(MapItem::ITEM_HEALTH_HP_INC)); // client will learn about new HP from the usual UserUpdateFromServer
        handleTakeNonWeaponItem(MapItemType::ITEM_HEALTH, bCurrentPlayer);
        break;
    case proofps_dd::MapItemType::ITEM_JETLAX:
        item.take();
        handleTakeNonWeaponItem(MapItemType::ITEM_JETLAX, bCurrentPlayer);
        break;
    default:
        getConsole().EOLn(
            "Player::%s(): unknown item type %d!", __func__, item.getType());
    }
}

const Weapon* proofps_dd::Player::getWeaponInstanceByMapItemType(const MapItemType& mapItemType) const
{
    switch (mapItemType)
    {
    case proofps_dd::MapItemType::ITEM_WPN_PISTOL:
    case proofps_dd::MapItemType::ITEM_WPN_MACHINEGUN:
    case proofps_dd::MapItemType::ITEM_WPN_BAZOOKA:
    case proofps_dd::MapItemType::ITEM_WPN_PUSHA:
    case proofps_dd::MapItemType::ITEM_WPN_MACHINEPISTOL:
    case proofps_dd::MapItemType::ITEM_WPN_SHOTGUN:
    case proofps_dd::MapItemType::ITEM_WPN_GRENADELAUNCHER:
    {
        const auto it = m_mapItemTypeToWeaponFilename.find(mapItemType);
        if (it == m_mapItemTypeToWeaponFilename.end())
        {
            getConsole().EOLn("Player::%s(): failed to find weapon by item type %d!", __func__, mapItemType);
            return nullptr;
        }

        const std::string& sWpnName = it->second;
        const Weapon* const pWpn = m_wpnMgr.getWeaponByFilename(sWpnName);
        if (!pWpn)
        {
            getConsole().EOLn("Player::%s(): failed to find weapon by name %s for item type %d!", __func__, sWpnName.c_str(), mapItemType);
            return nullptr;
        }

        return pWpn;
    }
    default:
        return nullptr;
    }
}

Weapon* proofps_dd::Player::getWeaponInstanceByMapItemType(const MapItemType& mapItemType)
{
    // simply invoke the const-version of the function above by const-casting:
    return const_cast<Weapon*>((const_cast<const Player* const>(this))->getWeaponInstanceByMapItemType(mapItemType));
}

void proofps_dd::Player::handleFallingFromHigh(int iServerScream /* valid only in case of client, server just sets it within this func */)
{
    assert(m_sndFallYell_1);  // otherwise new operator would had thrown already in ctor
    assert(m_sndFallYell_2);  // otherwise new operator would had thrown already in ctor

    if (m_network.isServer())
    {
        // for now, let just server control and depend on this variable, I'm afraid in some cases client might dont have the proper state, since
        // only server invokes all functions that might change its value
        if (m_bFallingHighTriggered)
        {
            return;
        }
        m_bFallingHighTriggered = true;

        // server selects which scream to play, then this is also sent to clients so clients dont select, just accept server's choice
        iServerScream = PFL::random(0, 1);
        //getConsole().EOLn("Player::%s() server selected: %d", __func__, iServerScream);
    }

    /*
    * SoLoud::play() returns a voice handle for us, that stays valid until the sound is being played.
    * This is good enough for us, but for the future keep in mind the following:
    * SoLoud::isValidVoiceHandle() might return false when the sound is still playing.
    * It might be because of buffered playing, and the handle is invalidated right after SoLoud
    * finished dealing with it (but the sound from buffer is not yet finished, which is not SoLoud's but the
    * backend's territory).
    * Some related issues on github might help understand the problem:
    *  - https://github.com/jarikomppa/soloud/issues/102
    *  - https://github.com/jarikomppa/soloud/issues/252
    *  - https://github.com/jarikomppa/soloud/issues/76
    *
    * Anyway, for us this is not a problem for now.
    */

    // should use WavInstance::hasEnded(), but where is WavInstance ??? I have Wav only ...
    if (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleFallYell))
    {
        // already playing, even though we later introduced m_bFallingHighTriggered flag, I'm still checking if sound
        // instance is being played so later I can easily copy this logic when needed.
        return;
    }

    //getConsole().EOLn("Player::%s() play scream: %d", __func__, iServerScream);
    m_handleFallYell =
        (iServerScream == 0) ?
        m_audio.play3dSound(*m_sndFallYell_1, getPos().getNew()) :
        m_audio.play3dSound(*m_sndFallYell_2, getPos().getNew());
    
    if (!m_network.isServer())
    {
        return;
    }

    //getConsole().EOLn("Player::%s() server sending scream: %d", __func__, iServerScream);
    pge_network::PgePacket pktPlayerEvent;
    proofps_dd::MsgPlayerEventFromServer::initPkt(
        pktPlayerEvent,
        getServerSideConnectionHandle(),
        PlayerEventId::FallingFromHigh,
        iServerScream);
    m_network.getServer().sendToAllClientsExcept(pktPlayerEvent);
}

void proofps_dd::Player::handleLanded(
    const float& fFallHeight,
    bool bDamageTaken,
    bool bDied,
    PureVector& vecCamShakeForce,
    const bool& bMe)
{
    // both server and client execute this function, so be careful with conditions here 

    assert(m_sndPlayerLandSmallFall);  // otherwise new operator would had thrown already in ctor
    assert(m_sndPlayerLandBigFall);  // otherwise new operator would had thrown already in ctor
    assert(m_sndPlayerDamage);  // otherwise new operator would had thrown already in ctor
    assert(m_sndPlayerFunnyLaughDiedByLanding);  // otherwise new operator would had thrown already in ctor

    m_bFallingHighTriggered = false;

    // whichever is currently being played for this player, needs to be stopped first
    m_audio.stopSoundInstance(m_handleSndPlayerLandSmallFall);
    m_audio.stopSoundInstance(m_handleSndPlayerLandBigFall);
    m_audio.stopSoundInstance(m_handleSndPlayerDamage);
    m_audio.stopSoundInstance(m_handleFallYell);

    //getConsole().EOLn("Player::%s() playing sound", __func__);

    const PureVector& playerPos = getPos().getNew();
    if (fFallHeight >= 1.f)
    {
        // because of inaudible sound becomes audible, needs this hack: https://github.com/jarikomppa/soloud/issues/175
        m_handleSndPlayerLandBigFall = m_audio.play3dSound(*m_sndPlayerLandBigFall, playerPos);
        //getConsole().EOLn(
        //    "Player::%s() XYZ: %f, %f, %f; Camera XYZ: %f, %f, %f",
        //    __func__,
        //    playerPos.getX(), playerPos.getY(), playerPos.getZ(),
        //    m_gfx.getCamera().getPosVec().getX(), m_gfx.getCamera().getPosVec().getY(), m_gfx.getCamera().getPosVec().getZ());

        if (bMe)
        {
            vecCamShakeForce.SetY(vecCamShakeForce.getY() + 4 * fFallHeight);
        }
    }
    else
    {
        m_handleSndPlayerLandSmallFall = m_audio.play3dSound(*m_sndPlayerLandSmallFall, playerPos);
    }

    if (bDamageTaken)
    {
        if (bDied)
        {
            m_audio.playSound(*m_sndPlayerFunnyLaughDiedByLanding);
        }
        else
        {
            // no need to play this in case of dieing because die sound is played anyway in PlayerHandling::handlePlayerDied()
            m_handleSndPlayerDamage = m_audio.play3dSound(*m_sndPlayerDamage, playerPos);
        }
    }

    if (!m_network.isServer())
    {
        return;
    }

    //getConsole().EOLn("Player::%s() server sending pkt", __func__);
    pge_network::PgePacket pktPlayerEvent;
    proofps_dd::MsgPlayerEventFromServer::initPkt(
        pktPlayerEvent,
        getServerSideConnectionHandle(),
        PlayerEventId::Landed,
        fFallHeight,
        bDamageTaken,
        bDied);
    m_network.getServer().sendToAllClientsExcept(pktPlayerEvent);
}

void proofps_dd::Player::handleActuallyRunningOnGround()
{
    // both server and client execute this function, so be careful with conditions here 

    const auto nTimeElapsedSinceHasStartedRunningOnGroundMillisecs =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - getActuallyRunningOnGround().getLastTimeNewValueChanged()).count();

    if (nTimeElapsedSinceHasStartedRunningOnGroundMillisecs <= TimeBefore1stFootstepCanBePlayedMillisecs)
    {
        // this amount of time must elapse after starting running, before the 1st footstep can be played
        return;
    }

    const auto nTimeElapsedSinceLastSndWalkPlayStartedMillisecs =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_timeLastSndPlayerFootstepPlayed).count();

    // remember, isValidVoiceHandle() might return false even if the sound is still being played, as explained in handleFallingFromHigh()
    if ((nTimeElapsedSinceLastSndWalkPlayStartedMillisecs <= m_nMinTimeBetweenPlayerWalkSoundsMillisecs) ||
        (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerFootstep)))
    {
        return;
    }

    static constexpr size_t sndPlayerFootstepCount = sizeof(m_sndPlayerFootstep) / sizeof(m_sndPlayerFootstep[0]);
    static_assert(sndPlayerFootstepCount > 0, "There must be at least 1 footstep sound.");
    const size_t iSndWalk = static_cast<size_t>(PFL::random(0, sndPlayerFootstepCount-1));
    assert(m_sndPlayerFootstep[iSndWalk]);  // otherwise new operator would had thrown already in ctor

    m_audio.stopSoundInstance(m_handleSndPlayerFootstep); // just make 1000% sure we dont play it multiple times in parallel for the same player
    
    //getConsole().EOLn("Player::%s() playing sound", __func__);

    m_handleSndPlayerFootstep = m_audio.play3dSound(*m_sndPlayerFootstep[iSndWalk], getPos().getNew());
    m_timeLastSndPlayerFootstepPlayed = std::chrono::steady_clock::now();
}

void proofps_dd::Player::handleTakeNonWeaponItem(
    const proofps_dd::MapItemType& eMapItemType,
    const bool& bMe)
{
    // both server and client execute this function, so be careful with conditions here

    // ALL instances execute this function, but not for all items:
    // - for inventory items, all players are informed,
    // - for other items (health, armor), only the affected player is informed!

    // this function is not invoked for all taken items, because this was introduced in v0.2.6, far later than MsgWpnUpdateFromServer,
    // so for example it does not get invoked for picked up weapons.
    // But it gets invoked for player inventory items too.

    assert(m_sndArmor);   // otherwise new operator would had thrown already in ctor
    assert(m_sndMedkit);
    assert(m_sndJetlax);

    switch (eMapItemType)
    {
    /* all instances must be aware of what inventory items other players have */
    case MapItemType::ITEM_JETLAX:
        setHasJetLax(true);
        setCurrentInventoryItemPower(100.f); // this is sent in MsgUserUpdateFromServer too when scheduled, but anyway why not set here too ...
        //getConsole().EOLn("Player::%s(): %s now has JetLax!", __func__, getName().c_str());
        break;
    default:
        break; /* no-op */
    }

    if (bMe)
    {
        switch (eMapItemType)
        {
        case MapItemType::ITEM_ARMOR:
            //getConsole().EOLn("Player::%s() playing sound", __func__);
            m_audio.play3dSound(*m_sndArmor, getPos().getNew());
            m_eventsItemPickup.addEvent("Armor: +" + std::to_string(MapItem::ITEM_ARMOR_AP_INC) + " AP");
            break;
        case MapItemType::ITEM_HEALTH:
            //getConsole().EOLn("Player::%s() playing sound", __func__);
            m_audio.play3dSound(*m_sndMedkit, getPos().getNew());
            m_eventsItemPickup.addEvent("Medkit: +" + std::to_string(MapItem::ITEM_HEALTH_HP_INC) + " HP");
            break;
        case MapItemType::ITEM_JETLAX:
            //getConsole().EOLn("Player::%s() playing sound", __func__);
            m_audio.play3dSound(*m_sndJetlax, getPos().getNew());
            m_eventsItemPickup.addEvent("JetLax added to inventory");
            m_eventsInventoryChange.addEvent("+ JetLax");
            break;
        default:
            getConsole().EOLn(
                "Player::%s(): unhandled item type %d!", __func__, eMapItemType);
        }
    }

    if (!m_network.isServer())
    {
        return;
    }

    // need to inform all players about inventory item pickups
    if (eMapItemType == MapItemType::ITEM_JETLAX)
    {
        pge_network::PgePacket pktPlayerEvent;
        proofps_dd::MsgPlayerEventFromServer::initPkt(
            pktPlayerEvent,
            getServerSideConnectionHandle(),
            PlayerEventId::ItemTake,
            static_cast<int>(eMapItemType));
        m_network.getServer().sendToAllClientsExcept(pktPlayerEvent);
    }
    else if (!bMe /* no need to inform them about server player took non-inventory item */)
    {
        pge_network::PgePacket pktPlayerEvent;
        proofps_dd::MsgPlayerEventFromServer::initPkt(
            pktPlayerEvent,
            getServerSideConnectionHandle(),
            PlayerEventId::ItemTake,
            static_cast<int>(eMapItemType));
        m_network.getServer().send(pktPlayerEvent, getServerSideConnectionHandle());
    }
    
} // handleTakeNonWeaponItem()

void proofps_dd::Player::handleToggleInventoryItem(
    const proofps_dd::MapItemType& eMapItemType,
    const bool& bSyncHistory)
{
    // both server and client execute this function, so be careful with conditions here;
    // ALL instances execute this function!

    // NOT invoked when player dies or respawns because there explicit false is set for both item use and availability.
    // Triggered by player input (toggle on/off) or when server wants to turn it off because of loss of power.
    // Also triggered when server informs a newly connected client about another player's ACTIVE inventory item state, and in
    // this case, bSyncHistory flag is set to true.

    assert(eMapItemType == MapItemType::ITEM_JETLAX); // for now this is the only allowed item here

    // bSyncHistory is for informing late-connecting clients about active item state, therefore server shall never have it set here.
    assert(
        (m_network.isServer() && !bSyncHistory) ||
        !m_network.isServer());

    const bool bOldAntiGravityActive = hasAntiGravityActive();
    bool bNewAntiGravityActive = !bOldAntiGravityActive; // first set it like this just to have the assertion possible below
    if (bNewAntiGravityActive && !hasJetLax())
    {
        // should not happen because then this is programmer error
        getConsole().EOLn("Player::%s(): SHALL NOT HAPPEN: bNewAntiGravityActive is true but does not have item in inventory!", __func__);
        assert(false); // crash in debug
    }

    // now set it more properly so we dont try enable something we dont even have
    bNewAntiGravityActive =
        !bOldAntiGravityActive &&
        hasJetLax() &&
        (getCurrentInventoryItemPower() > 0.f) &&
        !getCrouchStateCurrent();

    setHasAntiGravityActive(bNewAntiGravityActive);
    
    if (!bSyncHistory)
    {
        if (!bOldAntiGravityActive && bNewAntiGravityActive)
        {
            // a player has just activated the item
            assert(m_sndPlayerItemActivateAntiGravity);  // otherwise new operator would had thrown already in ctor
            m_audio.play3dSound(*m_sndPlayerItemActivateAntiGravity, getPos().getNew());

            if (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerItemDeactivateAntiGravity))
            {
                m_audio.stopSoundInstance(m_handleSndPlayerItemDeactivateAntiGravity);
            }

            // if we have already started yelling due to falling from high, stop it! :D
            if (m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleFallYell))
            {
                m_audio.stopSoundInstance(m_handleFallYell);
            }
        }
        else if (bOldAntiGravityActive && !bNewAntiGravityActive)
        {
            // a player has just deactivated the item
            assert(m_sndPlayerItemDeactivateAntiGravity);  // otherwise new operator would had thrown already in ctor
            if (!m_audio.getAudioEngineCore().isValidVoiceHandle(m_handleSndPlayerItemDeactivateAntiGravity))
            {
                m_handleSndPlayerItemDeactivateAntiGravity = m_audio.play3dSound(*m_sndPlayerItemDeactivateAntiGravity, getPos().getNew());
            }

            forceDeactivateCurrentInventoryItem();
        }
    }
    //else
    //{
    //    getConsole().EOLn("Player::%s(): synced bNewAntiGravityActive %b for player %s!", __func__, bNewAntiGravityActive, getName().c_str());
    //}

    if (!m_network.isServer())
    {
        return;
    }
    
    if (bOldAntiGravityActive && !bNewAntiGravityActive)
    {
        // just disabled antigravity, from now on jumpforce takes over
        getJumpForce() = getAntiGravityForce() / 100.f;
        setGravity(getAntiGravityForce().getY() * 2.f);
    }
    else if (!bOldAntiGravityActive && bNewAntiGravityActive)
    {
        // just enabled antigravity,
        // if there was any non-zero jumpforce (turned antigravity on in the middle of a jump), save it but
        // jumpforce must be zeroed since it is not used in antigravity fly!
        getAntiGravityForce() = getJumpForce() * 10 /* antigravity is decreased in different way that is why we multiply jumpforce */;
        getJumpForce().Set(0.f, 0.f, 0.f);

        // horizontal AG-force shall also take any strafe speed so momentum is carried on
        // Note: this strafe speed is from previous frame but it is not a problem.
        getAntiGravityForce().SetX(getAntiGravityForce().getX() + getStrafeSpeed() * 20 /* why multiply: same reason as with jumpforce above */);
        getStrafeSpeed() = 0.f;

        // vertical AG-force shall also take current player gravity into account to carry on the momentum (fall/jump)
        getAntiGravityForce().SetY(
            getAntiGravityForce().getY() +
            getGravity() / 10.f +
            1.f /* bump-lift the player on activation because why not? It looks cool */);
        setGravity(0.f);

        // satisfy isInAir()
        setCanFall(true); /* we are NOT on-ground, so definitely can fall */
        stopJumping(); /* definitely not jumping anymore even if we were not even jumping at this moment */

        // otherwise it will trigger yelling only once
        m_bFallingHighTriggered = false;
    }

    // here we can send out this update to ALL clients so they will be updated about each player's just activated or deactivated inventory item.
    pge_network::PgePacket pktPlayerEvent;
    proofps_dd::MsgPlayerEventFromServer::initPkt(
        pktPlayerEvent,
        getServerSideConnectionHandle(),
        PlayerEventId::InventoryItemToggle,
        static_cast<int>(eMapItemType),
        false /* bSyncHistory */);
    m_network.getServer().sendToAllClientsExcept(pktPlayerEvent);
} // handleToggleInventoryItem()

void proofps_dd::Player::handleUntakeInventoryItem(
    const proofps_dd::MapItemType& eMapItemType)
{
    // both server and client execute this function, so be careful with conditions here;
    // ALL instances execute this function!

    // NOT invoked when player dies or respawns because there explicit false is set for both item use and availability.
    // Triggered when server wants player to untake it because of loss of power.

    assert(eMapItemType == MapItemType::ITEM_JETLAX); // for now this is the only allowed item here

    if (hasAntiGravityActive())
    {
        // should not happen because then this is programmer error, server shall always turn item off before triggering untake!
        getConsole().EOLn("Player::%s(): SHALL NOT HAPPEN: hasAntiGravityActive is still true!", __func__);
        assert(false); // crash in debug
    }

    //getConsole().EOLn("Player::%s(): %u (%s) does not have jetlax anymore", __func__, getServerSideConnectionHandle(), getName().c_str());
    setHasJetLax(false);

    if (!m_network.isServer())
    {
        return;
    }

    pge_network::PgePacket pktPlayerEvent;
    proofps_dd::MsgPlayerEventFromServer::initPkt(
        pktPlayerEvent,
        getServerSideConnectionHandle(),
        PlayerEventId::ItemUntake,
        static_cast<int>(eMapItemType));
    m_network.getServer().sendToAllClientsExcept(pktPlayerEvent);
} // handleUntakeInventoryItem()

void proofps_dd::Player::handleTakeWeaponItem(
    const proofps_dd::MapItemType& eMapItemType,
    const Weapon& wpnTaken /* TODO: either this or eMapItemType should be removed in the future, redundant info! */,
    const bool& bJustBecameAvailable,
    const int& nAmmoIncrease /* valid only if bJustBecameAvailable is false */)
{
    // both server and client execute this function, so be careful with conditions here

    //getConsole().EOLn("Player::%s(): play sound", __func__);
    
    // ITEM_HEALTH is used for weapon updates that are not item pickups, here we should always have the actually valid weapon item type
    assert(eMapItemType != MapItemType::ITEM_HEALTH);

    if (bJustBecameAvailable)
    {
        assert(m_sndWpnNew);  // otherwise new operator would had thrown already in ctor
        m_audio.play3dSound(*m_sndWpnNew, getPos().getNew());
        m_eventsItemPickup.addEvent("+ NEW Weapon: " + MapItem::toString(eMapItemType));
    }
    else
    {
        assert(m_sndWpnAmmo);  // otherwise new operator would had thrown already in ctor
        m_audio.play3dSound(*m_sndWpnAmmo, getPos().getNew());
        m_eventsItemPickup.addEvent("+" + std::to_string(nAmmoIncrease) + " Ammo (" + MapItem::toString(eMapItemType) + ")");

        if (getWeaponManager().getCurrentWeapon() && (getWeaponManager().getCurrentWeapon() == &wpnTaken))
        {
            // we picked up ammo for the current weapon which was already available for the player
            m_eventsAmmoChange.addEvent(std::to_string(nAmmoIncrease));
        }
    }
} // handleTakeWeaponItem()

void proofps_dd::Player::handleJumppadActivated()
{
    assert(m_sndJumppad);  // otherwise new operator would had thrown already in ctor

    // due to assertion in handlePlayerEventFromServer(), this always gets invoked only for the relevant player who took the item!

    if (!m_network.isServer() || (getServerSideConnectionHandle() == pge_network::ServerConnHandle))
    {
        // both server and clients fall here if connHandle matches, and for clients connHandle will match only for them for now ...

        //getConsole().EOLn("PRooFPSddPGE::%s(): play sound", __func__);

        m_audio.play3dSound(*m_sndJumppad, getPos().getNew());
    }
    else if (m_network.isServer())
    {
        pge_network::PgePacket pktPlayerEvent;
        proofps_dd::MsgPlayerEventFromServer::initPkt(
            pktPlayerEvent,
            getServerSideConnectionHandle(),
            PlayerEventId::JumppadActivated);
        m_network.getServer().send(pktPlayerEvent, getServerSideConnectionHandle());
    }
}

void proofps_dd::Player::handleTeamIdChanged(const unsigned int& iTeamId)
{
    // both server and client execute this function, so be careful with conditions here

    // server invoked this func from PlayerHandling::serverHandleUserInGameMenuCmd(),
    // clients will invoke it from PlayerHandling::handlePlayerEventFromServer() which is sent by server at the end of this function!

    //getConsole().EOLn("Player::%s(%u): for player with connHandleServerSide: %u!", __func__, iTeamId, getServerSideConnectionHandle());
    
    if (iTeamId > 2)
    {
        // GameMode does not accept it so we dont either!
        getConsole().EOLn("Player::%s(%u): is invalid team id for player with connHandleServerSide: %u!", __func__, iTeamId, getServerSideConnectionHandle());
        return;
    }
    
    m_iTeamId = iTeamId;

    if (!m_network.isServer())
    {
        return;
    }

    pge_network::PgePacket pktPlayerEvent;
    proofps_dd::MsgPlayerEventFromServer::initPkt(
        pktPlayerEvent,
        getServerSideConnectionHandle(),
        PlayerEventId::TeamIdChanged,
        static_cast<int>(iTeamId));
    m_network.getServer().sendToAllClientsExcept(pktPlayerEvent);
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
    {proofps_dd::MapItemType::ITEM_WPN_PUSHA, "pusha.txt"},
    {proofps_dd::MapItemType::ITEM_WPN_MACHINEPISTOL, "machinepistol.txt"},
    {proofps_dd::MapItemType::ITEM_WPN_SHOTGUN, "shotgun.txt"},
    {proofps_dd::MapItemType::ITEM_WPN_GRENADELAUNCHER, "grenadelauncher.txt"}
};

uint32_t proofps_dd::Player::m_nPlayerInstanceCntr = 0;
SoLoud::Wav* proofps_dd::Player::m_sndWpnAmmo = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndWpnNew = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndArmor = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndMedkit = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndJetlax = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndJumppad = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndPlayerItemActivateAntiGravity = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndPlayerItemDeactivateAntiGravity = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndPlayerItemLowThrustAntiGravity = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndPlayerItemHighThrustAntiGravity = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndFallYell_1 = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndFallYell_2 = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndPlayerLandSmallFall = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndPlayerLandBigFall = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndPlayerDamage = nullptr;
SoLoud::Wav* proofps_dd::Player::m_sndPlayerFootstep[4] = { nullptr };
SoLoud::Wav* proofps_dd::Player::m_sndPlayerFunnyLaughDiedByLanding = nullptr;
std::chrono::milliseconds::rep proofps_dd::Player::m_nMaxSndPlayerFootstepDurationMillisecs = 0;
std::chrono::milliseconds::rep proofps_dd::Player::m_nMinTimeBetweenPlayerWalkSoundsMillisecs = 0;

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

    m_timeCtor = std::chrono::steady_clock::now();
}

int& proofps_dd::Player::getParticleEmitPerNthPhysicsIterationCntr()
{
    return m_nParticleEmitPerNthPhysicsIterCntr;
}

const int& proofps_dd::Player::getParticleEmitPerNthPhysicsIterationCntr() const
{
    return m_nParticleEmitPerNthPhysicsIterCntr;
}

void proofps_dd::Player::emitParticles(
    const proofps_dd::Config& config,
    PgeObjectPool<Smoke>& smokes)
{
    if (!getObject3D() || (config.getSmokeConfigAmount() == Smoke::SmokeConfigAmount::None))
    {
        return;
    }

    getParticleEmitPerNthPhysicsIterationCntr()++;
    assert(static_cast<int>(m_config.getSmokeConfigAmount()) < static_cast<int>(Smoke::smokeEmitOperValues.size()));

    if (getParticleEmitPerNthPhysicsIterationCntr() >= Smoke::smokeEmitOperValues[static_cast<int>(config.getSmokeConfigAmount())].m_nEmitInEveryNPhysicsIteration)
    {
        getParticleEmitPerNthPhysicsIterationCntr() = 0;
        PureVector vecFartPos = getPos().getNew();
        vecFartPos.SetY(vecFartPos.getY() - getObject3D()->getSizeVec().getY() / 2.f);
        // not the best targetVec to use here but I need to hurry up with the release :)
        PurePosUpTarget playerPut(vecFartPos, PureVector(), PureVector(0, 1, 0));
        smokes.create(
            playerPut,
            (getObject3D()->getAngleVec().getY() == 0.f) /* goingLeft, otherwise it would be 180.f */,
            107.f/255.f, 88.f/255.f, 52.f/255.f /* r,g,b as floats*/);
    }
}

PgeOldNewValue<int>& proofps_dd::Player::getArmor()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvArmor));
}

PgeOldNewValue<int>& proofps_dd::Player::getHealth()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<int>>(m_vecOldNewValues.at(OldNewValueName::OvHealth));
}

PgeOldNewValue<float>& proofps_dd::Player::getWeaponMomentaryAccuracy()
{
    // m_vecOldNewValues.at() should not throw due to how m_vecOldNewValues is initialized in class
    return std::get<PgeOldNewValue<float>>(m_vecOldNewValues.at(OldNewValueName::OvWpnMomentaryAccuracy));
}
