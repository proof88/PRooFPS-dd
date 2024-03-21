#pragma once

/*
    ###################################################################################
    Player.h
    Player and PlayerHandling classes for PRooFPS-dd
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

#include "PGE.h"
#include "Config/PgeOldNewValue.h"

#include "Config.h"
#include "Consts.h"
#include "Durations.h"
#include "GameMode.h"
#include "GUI.h"
#include "Maps.h"
#include "Networking.h"
#include "PRooFPS-dd-packet.h"
#include "Sounds.h"
#include "Strafe.h"

namespace proofps_dd
{

    class Player
    {
    public:

        static constexpr char* CVAR_SV_SOMERSAULT_MID_AIR_AUTO_CROUCH = "sv_somersault_mid_air_auto_crouch";

        static const char* getLoggerModuleName();
        static void proofps_dd::Player::genUniqueUserName(
            char szNewUserName[proofps_dd::MsgUserNameChange::nUserNameBufferLength],
            const std::string& sNameFromConfig,
            const std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers);

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        // TODO: we should pass the network instance also, so Player can always check if network instance is server or not,
        // and this also enables clearer testing
        explicit Player(
            PGEcfgProfiles& cfgProfiles,
            std::list<Bullet>& bullets,
            PR00FsUltimateRenderingEngine& gfx,
            const pge_network::PgeNetworkConnectionHandle& connHandle,
            const std::string& sIpAddress);
        ~Player();

        Player(const Player&);
        Player& operator=(const Player&);

        // TODO: add move ctor and move assignment operator
        Player(Player&&) = delete;
        Player&& operator=(Player&&) = delete;

        const pge_network::PgeNetworkConnectionHandle& getServerSideConnectionHandle() const;
        const std::string& getIpAddress() const;
        const std::string& getName() const;
        void setName(const std::string& sName);

        WeaponManager& getWeaponManager();
        const WeaponManager& getWeaponManager() const;

        bool isDirty() const;
        void updateOldValues();

        bool isNetDirty() const;
        void clearNetDirty();

        const PgeOldNewValue<int>& getHealth() const;
        void setHealth(int value);
        void doDamage(int dmg);
        PureVector& getImpactForce();
        void die(bool bMe, bool bServer);
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeDied() const;
        PgeOldNewValue<int>& getDeaths();
        const PgeOldNewValue<int>& getDeaths() const;

        bool& getRespawnFlag();
        void respawn(bool bMe, const Weapon& wpnDefaultAvailable, bool bServer);

        PgeOldNewValue<PureVector>& getPos();
        const PgeOldNewValue<PureVector>& getPos() const;
        bool isExpectingStartPos() const;
        void setExpectingStartPos(bool b);

        PgeOldNewValue<TPureFloat>& getAngleY();
        const PgeOldNewValue<TPureFloat>& getAngleY() const;
        PgeOldNewValue<TPureFloat>& getAngleZ();
        const PgeOldNewValue<TPureFloat>& getAngleZ() const;
        PgeOldNewValue<PureVector>& getWeaponAngle();

        PureObject3D* getObject3D() const;

        float getGravity() const;
        void setGravity(float value);

        bool isJumping() const;
        bool canFall() const;
        void setCanFall(bool state);
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
        void jump();
        void stopJumping();
        bool getWillJumpInNextTick() const;
        void setWillJumpInNextTick(bool flag);
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeLastSetWillJump() const;
        PureVector& getJumpForce();

        PgeOldNewValue<bool>& getCrouchInput();
        bool& getCrouchStateCurrent();
        const bool& isJumpingInitiatedFromCrouching() const;
        bool& getWantToStandup();
        void doCrouchServer(bool bPullUpLegs);
        void doCrouchShared();

        void doStandupServer(const float& fNewPosY);
        void doStandupShared();

        void startSomersaultServer();
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

        bool& getAttack();
        bool attack();

        PgeOldNewValue<int>& getFrags();
        const PgeOldNewValue<int>& getFrags() const;

        bool canTakeItem(const MapItem& item) const;
        void takeItem(MapItem& item, pge_network::PgePacket& pktWpnUpdate);

    private:

        enum class OldNewValueName
        {
            OvHealth,
            OvFrags,
            OvDeaths,
            OvPos,
            OvAngleY,
            OvAngleZ,
            OvWpnAngle,
            OvCrouchInput
        };

        static const std::map<MapItemType, std::string> m_mapItemTypeToWeaponFilename;
        static uint32_t m_nPlayerInstanceCntr;

        pge_network::PgeNetworkConnectionHandle m_connHandleServerSide;   /**< Used by both server and clients to identify the connection.
                                                                           Clients don't use it for direct communication.
                                                                           Note: this is the client's handle on server side!
                                                                           This is not the same handle as client has for the connection
                                                                           towards the server, those connection handles are not related
                                                                           to each other! */

        std::string m_sIpAddress; // TODO: this should be either in the engine, or wait until we move this class to the engine
        std::string m_sName;

        std::map<OldNewValueName,
            std::variant<
            PgeOldNewValue<int>,
            PgeOldNewValue<bool>,
            PgeOldNewValue<TPureFloat>,
            PgeOldNewValue<PureVector>
            >> m_vecOldNewValues = {
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
        };
        bool m_bNetDirty;
        std::chrono::time_point<std::chrono::steady_clock> m_timeDied;
        bool m_bRespawn;
        
        PureVector m_vecImpactForce;

        PureObject3D* m_pObj;
        PureTexture* m_pTexPlayerStand;
        PureTexture* m_pTexPlayerCrouch;

        WeaponManager m_wpnMgr;
        PGEcfgProfiles& m_cfgProfiles;
        std::list<Bullet>& m_bullets;
        PR00FsUltimateRenderingEngine& m_gfx;

        PureVector m_vecJumpForce;
        float m_fGravity;
        bool m_bJumping;
        bool m_bAllowJump;
        bool m_bWillJump;
        std::chrono::time_point<std::chrono::steady_clock> m_timeLastWillJump;
        bool m_bCanFall;
        bool m_bFalling;
        bool m_bHasJustStartedFallingNaturally;
        bool m_bHasJustStartedFallingAfterJumpingStopped;
        std::chrono::time_point<std::chrono::steady_clock> m_timeStartedFalling;
        float m_fHeightStartedFalling;
        bool m_bHasJustStoppedJumping;
        
        /** True when player is crouching currently, regardless of current input (OvCrouchInput).
            This should be replicated to all clients and should affect the visuals of the player.
            Can be set to true by input, but only server physics engine can set it to false, or a respawn event.
            Somersaulting can also set it to true if mid-air auto-crouch is enabled (CVAR_SV_SOMERSAULT_MID_AIR_AUTO_CROUCH).
            Default false. */
        bool m_bCrouchingStateCurrent;

        /** We need to save current crouching state at the moment of initiating jump-up, so that we can check in any later moment of
            jumping up if somersaulting can be initiated: it must not be initiated when player was already crouching at the moment of jump-up. */
        bool m_bCrouchingWasActiveWhenInitiatedJump;

        /** True when player wants standing position as per input, regardless of currently crouching or not.
            This is an input to the physics engine.
            Unlike getCrouchInput(), this is persistent across frames.
            If this is true and the player is currently crouching, the physics engine will contantly check if there is
            possibility to stand up i.e. there is nothing blocking us from standing up.
            If the physics engine finds enough space to stand up and other conditions are also fulfilled (e.g. not somersaulting), it
            will flip m_bCrouchingStateCurrent to false so higher level layers will know the player is not crouching anymore.
            Default true. */
        bool m_bWantToStandup;

        float m_fSomersaultAngleZ;  /**< If non-zero, there is ongoing somersaulting handled by physics. */
        
        bool m_bRunning;
        std::chrono::time_point<std::chrono::steady_clock> m_timeLastToggleRun;

        bool m_bExpectingStartPos;

        proofps_dd::Strafe m_strafe;  // continuous op

        bool m_bAttack;               // continuous op

        // ---------------------------------------------------------------------------

        void BuildPlayerObject(bool blend);

        PgeOldNewValue<int>& getHealth();

    }; // class Player

    class PlayerHandling :
        protected proofps_dd::Networking
    {
    public:

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        PlayerHandling(
            PGE& pge,
            proofps_dd::Durations& durations,
            proofps_dd::GUI& gui,
            std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& mapPlayers,
            proofps_dd::Maps& maps,
            proofps_dd::Sounds& sounds);

        PlayerHandling(const PlayerHandling&) = delete;
        PlayerHandling& operator=(const PlayerHandling&) = delete;
        PlayerHandling(PlayerHandling&&) = delete;
        PlayerHandling&& operator=(PlayerHandling&&) = delete;

    protected:

        void HandlePlayerDied(
            Player& player,
            PureObject3D& objXHair,
            pge_network::PgeNetworkConnectionHandle nKillerConnHandleServerSide);
        void HandlePlayerRespawned(Player& player, PureObject3D& objXHair);
        void ServerRespawnPlayer(Player& player, bool restartGame);
        void serverUpdateRespawnTimers(
            proofps_dd::GameMode& gameMode,
            proofps_dd::Durations& durations);
        void updatePlayersOldValues();
        void WritePlayerList();
        bool handleUserConnected(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const pge_network::MsgUserConnectedServerSelf& msg,
            PGEcfgProfiles& cfgProfiles,
            std::function<void(int)>& cbDisplayMapLoadingProgressUpdate);
        bool handleUserDisconnected(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const pge_network::MsgUserDisconnectedFromServer& msg,
            proofps_dd::GameMode& gameMode);
        bool handleUserNameChange(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserNameChange& msg,
            proofps_dd::GameMode& gameMode,
            PGEcfgProfiles& cfgProfiles);
        void resetSendClientUpdatesCounter(proofps_dd::Config& config);
        void serverSendUserUpdates(proofps_dd::Durations& durations);
        bool handleUserUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle connHandleServerSide,
            const proofps_dd::MsgUserUpdateFromServer& msg,
            PureObject3D& objXHair,
            proofps_dd::GameMode& gameMode);
        bool handleDeathNotificationFromServer(
            pge_network::PgeNetworkConnectionHandle nDeadConnHandleServerSide, const proofps_dd::MsgDeathNotificationFromServer& msg);

    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;
        proofps_dd::GUI& m_gui;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;

        unsigned int m_nSendClientUpdatesInEveryNthTick;
        unsigned int m_nSendClientUpdatesCntr;

    }; // class PlayerHandling

} // namespace proofps_dd
