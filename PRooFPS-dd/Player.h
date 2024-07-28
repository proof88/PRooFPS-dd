#pragma once

/*
    ###################################################################################
    Player.h
    Player class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include <chrono>      // requires cpp11
#include <list>
#include <map>
#include <variant>     // requires cpp17
#include <vector>

#include "CConsole.h"

#include "PGE.h" // we use audio also from here so it is easier to just include everything
#include "Config/PgeOldNewValue.h"

#include "Config.h"
#include "Durations.h"
#include "EventLister.h"
#include "PRooFPS-dd-packet.h"
#include "Strafe.h"

namespace proofps_dd
{

    enum PlayerEventId : pge_network::TByte
    {
        FallingFromHigh = 0,
        Landed,
        ItemTake,
        JumppadActivated
    };

    class Player
    {
    public:

        static constexpr char* szCVarClName = "cl_name";

        static constexpr float fObjWidth = 0.95f;
        static constexpr float fObjHeightStanding = 1.88f;
        static constexpr float fObjHeightCrouchScaling = 0.48f;

        // Physics modifies these as per nPhysicsRate
        static constexpr float fBaseSpeedWalk = 2.f;
        static constexpr float fBaseSpeedRun = 4.f;
        static constexpr float fBaseSpeedCrouch = 1.5f;

        /*
          For the future:
          for tickrate 20, this is good, for tickrate 60, 19.f gives identical result.
          However, in Physics::serverGravity(), I'm lerping not this but GAME_GRAVITY_CONST based on tickrate.
          I don't remember why I'm not lerping this between 19 and 20 but anyway that approach is also good.

          WARNING: when value is changed, physics must be manually tested on Warhouse: there are some places
          on that map when we cannot jump HORIZONTALLY in between walls/boxes.
          For example, as of v0.1.6, 20.f and 19.f works fine, but 18.f produces this issue.
          And different tick/physics_min_rate config values should be tested (60 and 20).
        */
        static constexpr float fJumpGravityStartFromStanding = 19.f;

        // WARNING: change this value with same caution as with above const!
        static constexpr float fJumpGravityStartFromCrouching = 15.f;

        static constexpr char* szCVarSvSomersaultMidAirAutoCrouch = "sv_somersault_mid_air_auto_crouch";
        static constexpr char* szCVarSvSomersaultMidAirJumpForceMultiplier = "sv_somersault_mid_air_jump_force_multiplier";

        static constexpr unsigned int nSomersaultTargetDurationMillisecs = 300;
        static_assert(
            nSomersaultTargetDurationMillisecs > 0,
            "Somersault duration cannot be 0.");

        static constexpr float fSomersaultMidAirJumpForceMultiplierMin = 1.f;
        static constexpr float fSomersaultMidAirJumpForceMultiplierMax = 2.f;
        static constexpr float fSomersaultMidAirJumpForceMultiplierDef = Player::fSomersaultMidAirJumpForceMultiplierMax;
        static_assert(
            fSomersaultMidAirJumpForceMultiplierMin <= fSomersaultMidAirJumpForceMultiplierDef,
            "Min somersault mid-air jump force multiplier should not be greater than default somersault mid-air jump force multiplier.");
        static_assert(
            fSomersaultMidAirJumpForceMultiplierMin <= fSomersaultMidAirJumpForceMultiplierMax,
            "Min somersault mid-air jump force multiplier should not be greater than max somersault mid-air jump force multiplier.");
        static_assert(
            fSomersaultMidAirJumpForceMultiplierDef <= fSomersaultMidAirJumpForceMultiplierMax,
            "Max somersault mid-air jump force multiplier should not be smaller than default somersault mid-air jump force multiplier.");

        static constexpr float fSomersaultGroundImpactForceX = 10.f;

        static constexpr unsigned int nSvDmRespawnDelaySecsDef = 3;
        static constexpr char* szCVarSvDmRespawnDelaySecs = "sv_dm_respawn_delay";

        static constexpr unsigned int nSvDmRespawnInvulnerabilityDelaySecsDef = 3;
        static constexpr char* szCVarSvDmRespawnInvulnerabilityDelaySecs = "sv_dm_respawn_invulnerability_delay";

        static const char* getLoggerModuleName();
        static void proofps_dd::Player::genUniqueUserName(
            char szNewUserName[proofps_dd::MsgUserNameChangeAndBootupDone::nUserNameBufferLength],
            const std::string& sNameFromConfig,
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers);

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        // TODO: we should pass the network instance also, so Player can always check if network instance is server or not,
        // and this also enables clearer testing
        explicit Player(
            pge_audio::PgeAudio& audio,
            PGEcfgProfiles& cfgProfiles,
            std::list<Bullet>& bullets,
            EventLister& eventsItemPickup,
            PR00FsUltimateRenderingEngine& gfx,
            pge_network::PgeINetwork& network,
            const pge_network::PgeNetworkConnectionHandle& connHandle,
            const std::string& sIpAddress
            );
        ~Player();

        Player(const Player&);
        Player& operator=(const Player&);

        // TODO: add move ctor and move assignment operator
        Player(Player&&) = delete;
        Player&& operator=(Player&&) = delete;

        const std::chrono::time_point<std::chrono::steady_clock>& getTimeConstructed() const;
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeBootedUp() const;
        void setTimeBootedUp();

        bool isExpectingAfterBootUpDelayedUpdate() const;
        void setExpectingAfterBootUpDelayedUpdate(bool b);

        const pge_network::PgeNetworkConnectionHandle& getServerSideConnectionHandle() const;
        const std::string& getIpAddress() const;
        const std::string& getName() const;
        void setName(const std::string& sName);

        void updateAudioVisuals(const proofps_dd::Config& config, bool bServer);

        void show();
        void hide();
        void setVisibilityState(bool state);

        WeaponManager& getWeaponManager();
        const WeaponManager& getWeaponManager() const;

        bool isDirty() const;
        void updateOldValues();

        bool isNetDirty() const;
        void clearNetDirty();

        const PgeOldNewValue<int>& getArmor() const;
        void setArmor(int value);
        const PgeOldNewValue<int>& getHealth() const;
        void setHealth(int value);
        void doDamage(int dmgAP, int dmgHP);
        PureVector& getImpactForce();
        void die(bool bMe, bool bServer);
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeDied() const;
        PgeOldNewValue<int>& getDeaths();
        const PgeOldNewValue<int>& getDeaths() const;

        bool& getRespawnFlag();
        void respawn(bool bMe, const Weapon& wpnDefaultAvailable, bool bServer);

        const PgeOldNewValue<bool>& getInvulnerability() const;
        void setInvulnerability(const bool& bState, const unsigned int& nSeconds = 0 /* relevant only if bState is true */);
        const unsigned int& getInvulnerabilityDurationSeconds() const;
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeInvulnerabilityStarted() const;

        PgeOldNewValue<PureVector>& getPos();
        const PgeOldNewValue<PureVector>& getPos() const;

        bool isJustCreatedAndExpectingStartPos() const;
        void setJustCreatedAndExpectingStartPos(bool b);

        PgeOldNewValue<TPureFloat>& getAngleY();
        const PgeOldNewValue<TPureFloat>& getAngleY() const;
        PgeOldNewValue<TPureFloat>& getAngleZ();
        const PgeOldNewValue<TPureFloat>& getAngleZ() const;
        PgeOldNewValue<PureVector>& getWeaponAngle();

        PureObject3D* getObject3D() const;

        float getGravity() const;
        void setGravity(float value);

        bool isJumping() const;
        // TODO: canFall() and setCanFall() are a bit fishy. They have been around since the very beginning, but using isFalling() seems to be better.
        bool canFall() const;
        void setCanFall(bool state);
        bool isInAir() const;
        bool getHasJustStartedFallingNaturallyInThisTick() const;
        void setHasJustStartedFallingNaturallyInThisTick(bool val);
        bool getHasJustStartedFallingAfterJumpingStoppedInThisTick() const;
        void setHasJustStartedFallingAfterJumpingStoppedInThisTick(bool val);
        bool isFalling() const;
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeStartedFalling() const;
        const float getHeightStartedFalling() const;
        bool& getHasJustStoppedJumpingInThisTick();
        bool jumpAllowed() const;
        void setJumpAllowed(bool b);
        void jump(const float& fRunSpeedPerTickForJumppadHorizontalForce = 0.f);
        void stopJumping();
        float getWillJumpXInNextTick() const;
        float getWillJumpYInNextTick() const;
        void setWillJumpInNextTick(float factorY, float factorX);
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeLastSetWillJump() const;
        PureVector& getJumpForce();

        PgeOldNewValue<bool>& getCrouchInput();
        bool& getCrouchStateCurrent();
        const bool& isJumpingInitiatedFromCrouching() const;
        const bool& isJumpInitiatedByJumppad() const;
        bool& getWantToStandup();
        float getProposedNewPosYforStandup() const;
        void doCrouchServer();
        void doCrouchShared();

        void doStandupServer();
        void doStandupShared();

        bool getWillSomersaultInNextTick() const;
        void setWillSomersaultInNextTick(bool flag);
        void startSomersaultServer(bool bJumpInduced);
        void setSomersaultClient(float angleZ);
        bool isSomersaulting() const;
        float getSomersaultAngle() const;
        void stepSomersaultAngleServer(float angle);
        void resetSomersaultServer();

        bool isRunning() const;
        void setRun(bool state);
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeLastToggleRun() const;

        const proofps_dd::Strafe& getStrafe() const;
        void setStrafe(const proofps_dd::Strafe& strafe);
        const proofps_dd::Strafe& getPreviousActualStrafe() const;
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeLastActualStrafe() const;

        float& getStrafeSpeed();

        bool& getAttack();
        bool attack();

        PgeOldNewValue<int>& getFrags();
        const PgeOldNewValue<int>& getFrags() const;

        bool canTakeItem(const MapItem& item) const;
        void takeItem(MapItem& item, pge_network::PgePacket& pktWpnUpdate);

        void handleFallingFromHigh(int iServerScream);
        void handleLanded(const float& fFallHeight, bool bDamageTaken, bool bDied);
        void handleTakeNonWeaponItem(const proofps_dd::MapItemType& eMapItemType);
        void handleTakeWeaponItem(
            const proofps_dd::MapItemType& eMapItemType,
            const bool& bJustBecameAvailable,
            const int& nAmmoIncrease);
        void handleJumppadActivated();

    private:

        enum class OldNewValueName
        {
            OvArmor,
            OvHealth,
            OvFrags,
            OvDeaths,
            OvPos,
            OvAngleY,
            OvAngleZ,
            OvWpnAngle,
            OvCrouchInput,
            OvInvulnerability
        };

        static const std::map<MapItemType, std::string> m_mapItemTypeToWeaponFilename;
        static uint32_t m_nPlayerInstanceCntr;

        // these sounds are static because need to load only once and played only for current player
        static SoLoud::Wav* m_sndWpnAmmo;
        static SoLoud::Wav* m_sndWpnNew;
        static SoLoud::Wav* m_sndMedkit;
        static SoLoud::Wav* m_sndJumppad;
        static SoLoud::Wav* m_sndFallYell_1;
        static SoLoud::Wav* m_sndFallYell_2;
        static SoLoud::Wav* m_sndPlayerLandSmallFall;
        static SoLoud::Wav* m_sndPlayerLandBigFall;
        static SoLoud::Wav* m_sndPlayerDamage;

        pge_network::PgeNetworkConnectionHandle m_connHandleServerSide;   /**< Used by both server and clients to identify the connection.
                                                                           Clients don't use it for direct communication.
                                                                           Note: this is the client's handle on server side!
                                                                           This is not the same handle as client has for the connection
                                                                           towards the server, those connection handles are not related
                                                                           to each other! */

        std::string m_sIpAddress; // TODO: this should be either in the engine, or wait until we move this class to the engine
        std::string m_sName;

        std::chrono::time_point<std::chrono::steady_clock> m_timeCtor;
        
        /** Timestamp of server-side processing of UserNameChangeAndBootupDone (after MsgUserConnectedServerSelf and MsgUserSetupFromServer is already processed */
        std::chrono::time_point<std::chrono::steady_clock> m_timeBootedUp;

        bool m_bExpectingAfterBootUpDelayedUpdate = true;

        std::map<OldNewValueName,
            std::variant<
            PgeOldNewValue<int>,
            PgeOldNewValue<bool>,
            PgeOldNewValue<TPureFloat>,
            PgeOldNewValue<PureVector>
            >> m_vecOldNewValues = {
                {OldNewValueName::OvArmor,    PgeOldNewValue<int>(0)},
                {OldNewValueName::OvHealth,   PgeOldNewValue<int>(100)},
                {OldNewValueName::OvFrags,    PgeOldNewValue<int>(0)},
                {OldNewValueName::OvDeaths,   PgeOldNewValue<int>(0)},
                {OldNewValueName::OvPos,      PgeOldNewValue<PureVector>()},
                {OldNewValueName::OvAngleY,   PgeOldNewValue<TPureFloat>(0.f)},
                {OldNewValueName::OvAngleZ,   PgeOldNewValue<TPureFloat>(0.f)},
                {OldNewValueName::OvWpnAngle, PgeOldNewValue<PureVector>()},
                /** Current state of player crouch input, regardless of current crouching state.
                    Player is setting it as per input.
                    Continuous op. */
                {OldNewValueName::OvCrouchInput,   PgeOldNewValue<bool>(false)},
                {OldNewValueName::OvInvulnerability,  PgeOldNewValue<bool>(true)},
        };

        bool m_bNetDirty = false;
        std::chrono::time_point<std::chrono::steady_clock> m_timeDied;
        bool m_bRespawn = false;
        std::chrono::time_point<std::chrono::steady_clock> m_timeStartedInvulnerability;
        unsigned int m_nInvulnerabilityDurationSecs = 0;
        
        PureVector m_vecImpactForce;

        PureObject3D* m_pObj = nullptr;
        PureTexture* m_pTexPlayerStand = nullptr;
        PureTexture* m_pTexPlayerCrouch = nullptr;

        WeaponManager m_wpnMgr;
        pge_audio::PgeAudio& m_audio;
        PGEcfgProfiles& m_cfgProfiles;
        std::list<Bullet>& m_bullets;
        EventLister& m_eventsItemPickup;
        PR00FsUltimateRenderingEngine& m_gfx;
        pge_network::PgeINetwork& m_network;

        PureVector m_vecJumpForce;
        float m_fGravity = 0.f;
        bool m_bJumping = false;
        bool m_bAllowJump = false;
        /**
        * Since v0.2.6 we also record horizontal jump factor which is non-0 for jumppads triggering horizontal force.
        */
        float m_fWillJumpMultFactorX = 0.f;
        /**
        * Before v0.2.6 this was a boolean value, but then I changed it to float so it also means how strong the jump will be.
        * 0 means false in the pre-v0.2.6 world, and any positive value means true (= player will jump).
        * Also I added Y to the end of the name because now we also record FactorX in separate var.
        */
        float m_fWillJumpMultFactorY = 0.f;
        std::chrono::time_point<std::chrono::steady_clock> m_timeLastWillJump;
        bool m_bCanFall = true;
        bool m_bFalling = true;
        bool m_bHasJustStartedFallingNaturally = true;
        bool m_bHasJustStartedFallingAfterJumpingStopped = false;
        std::chrono::time_point<std::chrono::steady_clock> m_timeStartedFalling;
        float m_fHeightStartedFalling = 0.f;
        bool m_bHasJustStoppedJumping = false;
        
        /**
        *   This flag is set by handleFallingFromHigh(), we need to use this flag to make sure the actions are triggered only once when
        *   the player is falling from high. This is because Physics calls handleFallingFromHigh() in each physics step when the player
        *   is falling from high. Reset of this flag should be whenever falling high is interrupted, e.g. when player landed on ground.
        */
        bool m_bFallingHighTriggered = false;

        // these sounds for which we store handles, they are played for any player so other players can also hear them, remember
        // for these we should set proper attenuation model with min/max distance!
        SoLoud::handle m_handleFallYell = 0;
        SoLoud::handle m_handleSndPlayerLandSmallFall = 0;
        SoLoud::handle m_handleSndPlayerLandBigFall = 0;
        SoLoud::handle m_handleSndPlayerDamage = 0;

        /** True when player is crouching currently, regardless of current input (OvCrouchInput).
            This should be replicated to all clients and should affect the visuals of the player.
            Can be set to true by input, but only server physics engine can set it to false, or a respawn event.
            Somersaulting can also set it to true if mid-air auto-crouch is enabled (Player::szCVarSvSomersaultMidAirAutoCrouch).
            Default false. */
        bool m_bCrouchingStateCurrent = false;

        /** We need to save current crouching state at the moment of initiating jump-up, so that we can check in any later moment of
            jumping up if somersaulting can be initiated: it must not be initiated when player was already crouching at the moment of jump-up. */
        bool m_bCrouchingWasActiveWhenInitiatedJump = false;

        /** We need to save if jumping was initiated by jumppad because we should not allow mid-air somersaulting in such case. */
        bool m_bJumpWasInitiatedByJumppad = false;

        /** True when player wants standing position as per input, regardless of currently crouching or not.
            This is an input to the physics engine.
            Unlike getCrouchInput(), this is persistent across frames.
            If this is true and the player is currently crouching, the physics engine will contantly check if there is
            possibility to stand up i.e. there is nothing blocking us from standing up.
            If the physics engine finds enough space to stand up and other conditions are also fulfilled (e.g. not somersaulting), it
            will flip m_bCrouchingStateCurrent to false so higher level layers will know the player is not crouching anymore.
            Default true. */
        bool m_bWantToStandup = true;

        bool m_bWillSomersault = false;
        float m_fSomersaultAngleZ = 0.f;  /**< If non-zero, there is ongoing somersaulting handled by physics. */
        
        bool m_bRunning = true;
        std::chrono::time_point<std::chrono::steady_clock> m_timeLastToggleRun;

        bool m_bJustCreatedAndExpectingStartPos = true;

        proofps_dd::Strafe m_strafe = Strafe::NONE;  // continuous op
        proofps_dd::Strafe m_prevActualStrafe = Strafe::NONE;
        std::chrono::time_point<std::chrono::steady_clock> m_timeLastStrafe;

        /* Server is changing this in every physics tick to target strafe speed to have a smoother strafe start feeling, and to have a workaround
           for situations when player would not be able to fall into a 1-block-wide hole, this way they can when they try strafing from non-strafe status. */
        float m_strafeSpeed = 0.f;

        bool m_bAttack = false;       // continuous op

        // ---------------------------------------------------------------------------

        void BuildPlayerObject(bool blend);

        PgeOldNewValue<int>& getArmor();
        PgeOldNewValue<int>& getHealth();

    }; // class Player

} // namespace proofps_dd
