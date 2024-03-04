#pragma once

/*
    ###################################################################################
    Player.h
    Player and PlayerHandling classes for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <chrono>      // requires cpp11
#include <list>
#include <map>
#include <variant>     // requires cpp17
#include <vector>

#include "../../Console/CConsole/src/CConsole.h"

#include "PGE.h"
#include "Config/PgeOldNewValue.h"

#include "Consts.h"
#include "Durations.h"
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

        PgeOldNewValue<int>& getHealth();
        const PgeOldNewValue<int>& getHealth() const;
        PgeOldNewValue<PureVector>& getPos();
        const PgeOldNewValue<PureVector>& getPos() const;
        PgeOldNewValue<TPureFloat>& getAngleY();
        const PgeOldNewValue<TPureFloat>& getAngleY() const;
        PureObject3D* getObject3D() const;
        float getGravity() const;
        bool isJumping() const;
        bool canFall() const;
        bool& getHasJustStartedFallingNaturallyInThisTick();
        bool& getHasJustStartedFallingAfterJumpingStoppedInThisTick();
        bool& getHasJustStoppedJumpingInThisTick();
        void SetHealth(int value);
        void SetGravity(float value);
        bool jumpAllowed() const;
        void SetJumpAllowed(bool b);
        void Jump();
        void StopJumping();
        bool getWillJumpInNextTick() const;
        void setWillJumpInNextTick(bool flag);
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeLastSetWillJump() const;
        void DoDamage(int dmg);
        void SetCanFall(bool state);
        bool isRunning() const;
        void SetRun(bool state);
        const std::chrono::time_point<std::chrono::steady_clock>& getTimeLastToggleRun() const;
        const proofps_dd::Strafe& getStrafe() const;
        void setStrafe(const proofps_dd::Strafe& strafe);
        bool& getAttack();
        bool attack();
        void Die(bool bMe, bool bServer);
        void Respawn(bool bMe, const Weapon& wpnDefaultAvailable, bool bServer);
        PureVector& getJumpForce();
        PureVector& getImpactForce();
        bool isExpectingStartPos() const;
        void SetExpectingStartPos(bool b);
        PgeOldNewValue<PureVector>& getWeaponAngle();
        PgeOldNewValue<bool>& getCrouchInput();
        bool& getCrouchStateCurrent();
        bool& getWantToStandup();
        void DoCrouchServer(bool bPullUpLegs);
        void DoCrouchShared();
        void DoStandupServer(const float& fNewPosY);
        void DoStandupShared();
        std::chrono::time_point<std::chrono::steady_clock>& getTimeDied();
        bool& getRespawnFlag();
        PgeOldNewValue<int>& getFrags();
        const PgeOldNewValue<int>& getFrags() const;
        PgeOldNewValue<int>& getDeaths();
        const PgeOldNewValue<int>& getDeaths() const;
        bool canTakeItem(const MapItem& item) const;
        void TakeItem(MapItem& item, pge_network::PgePacket& pktWpnUpdate);

    private:

        enum class OldNewValueName
        {
            OvHealth,
            OvFrags,
            OvDeaths,
            OvPos,
            OvAngleY,
            OvWpnAngle,
            OvCrouch
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
                {OldNewValueName::OvWpnAngle, PgeOldNewValue<PureVector>()},
                /** Current state of player crouch input, regardless of current crouching state.
                    Player is setting it as per input.
                    Continuous op. */
                {OldNewValueName::OvCrouch,   PgeOldNewValue<bool>(false)},
        };
        bool m_bNetDirty;

        PureVector m_vecJumpForce;
        PureVector m_vecImpactForce;
        PureObject3D* m_pObj;
        PureTexture* m_pTexPlayerStand;
        PureTexture* m_pTexPlayerCrouch;
        WeaponManager m_wpnMgr;
        PGEcfgProfiles& m_cfgProfiles;
        std::list<Bullet>& m_bullets;
        PR00FsUltimateRenderingEngine& m_gfx;
        float m_fGravity;
        bool m_bJumping;
        bool b_mCanFall;
        bool m_bHasJustStartedFallingNaturally;
        bool m_bHasJustStartedFallingAfterJumpingStopped;
        bool m_bHasJustStoppedJumping;
        
        /** True when player is crouching currently, regardless of current input.
            This should be replicated to all clients, this should affect the visuals of the player.
            Can be set to true by input, but only server physics engine can set it to false, or a respawn event.
            Default false. */
        bool m_bCrouchingStateCurrent;   

        /** True when player wants standing position as per input, regardless of currently crouching or not.
            This is an input to the physics engine.
            Unlike getCrouchInput(), this is persistent across frames.
            If this is true and the player is currently crouching, the physics engine will contantly check if there is
            possibility to stand up i.e. there is nothing blocking us from standing up.
            If the physics engine finds enough space to stand up, it will flip m_bCrouchingStateCurrent to false so
            higher level layers will know the player is not crouching anymore.
            Default true. */
        bool m_bWantToStandup;
        
        bool m_bRunning;
        std::chrono::time_point<std::chrono::steady_clock> m_timeLastToggleRun;
        bool m_bAllowJump;
        bool m_bWillJump;
        std::chrono::time_point<std::chrono::steady_clock> m_timeLastWillJump;
        bool m_bExpectingStartPos;
        proofps_dd::Strafe m_strafe;  // continuous op
        bool m_bAttack;               // continuous op
        std::chrono::time_point<std::chrono::steady_clock> m_timeDied;
        bool m_bRespawn;

        // ---------------------------------------------------------------------------

        void BuildPlayerObject(bool blend);

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

        void HandlePlayerDied(Player& player, PureObject3D& objXHair);
        void HandlePlayerRespawned(Player& player, PureObject3D& objXHair);
        void ServerRespawnPlayer(Player& player, bool restartGame);
        void updatePlayersOldValues();

    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;
        proofps_dd::GUI& m_gui;
        std::map<pge_network::PgeNetworkConnectionHandle, proofps_dd::Player>& m_mapPlayers;
        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;

    }; // class PlayerHandling

} // namespace proofps_dd
