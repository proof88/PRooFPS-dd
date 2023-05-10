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

#include "../../../CConsole/CConsole/src/CConsole.h"

#include "../../../PGE/PGE/PGE.h"
#include "../../../PGE/PGE/Config/PgeOldNewValue.h"

#include "Consts.h"
#include "Durations.h"
#include "Maps.h"
#include "Networking.h"
#include "Sounds.h"
#include "UserInterface.h"

namespace proofps_dd
{

    class Player
    {
    public:

        static const char* getLoggerModuleName();

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

        PgeOldNewValue<int>& getHealth();
        const PgeOldNewValue<int>& getHealth() const;
        PgeOldNewValue<PureVector>& getPos();
        PgeOldNewValue<TPureFloat>& getAngleY();
        PureObject3D* getObject3D() const;
        float getGravity() const;
        bool isJumping() const;
        bool isFalling() const;
        bool canFall() const;
        void SetHealth(int value);
        void SetGravity(float value);
        bool jumpAllowed() const;
        void SetJumpAllowed(bool b);
        void Jump();
        void StopJumping();
        void DoDamage(int dmg);
        void SetCanFall(bool state);
        bool isRunning() const;
        void SetRun(bool state);
        void Die(bool bMe, bool bServer);
        void Respawn(bool bMe, const Weapon& wpnDefaultAvailable, bool bServer);
        PureVector& getForce();
        bool isExpectingStartPos() const;
        void SetExpectingStartPos(bool b);
        PgeOldNewValue<PureVector>& getWeaponAngle();
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
            PgeOldNewValue<TPureFloat>,
            PgeOldNewValue<PureVector>
            >> m_vecOldNewValues = {
                {OldNewValueName::OvHealth,   PgeOldNewValue<int>(100)},
                {OldNewValueName::OvFrags,    PgeOldNewValue<int>(0)},
                {OldNewValueName::OvDeaths,   PgeOldNewValue<int>(0)},
                {OldNewValueName::OvPos,      PgeOldNewValue<PureVector>()},
                {OldNewValueName::OvAngleY,   PgeOldNewValue<TPureFloat>(0.f)},
                {OldNewValueName::OvWpnAngle, PgeOldNewValue<PureVector>()}
        };

        PureVector m_vecForce;
        PureObject3D* m_pObj;
        WeaponManager m_wpnMgr;
        PGEcfgProfiles& m_cfgProfiles;
        std::list<Bullet>& m_bullets;
        PR00FsUltimateRenderingEngine& m_gfx;
        float m_fGravity;
        bool m_bJumping;
        bool b_mCanFall;
        bool m_bRunning;
        bool m_bAllowJump;
        bool m_bExpectingStartPos;
        std::chrono::time_point<std::chrono::steady_clock> m_timeDied;
        bool m_bRespawn;

        // ---------------------------------------------------------------------------

        void BuildPlayerObject(bool blend);

    }; // class Player

    class PlayerHandling :
        protected virtual proofps_dd::Networking,
        protected virtual proofps_dd::UserInterface
    {
    public:

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        PlayerHandling(
            proofps_dd::Durations& durations,
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

    private:

        // ---------------------------------------------------------------------------

        proofps_dd::Maps& m_maps;
        proofps_dd::Sounds& m_sounds;

    }; // class PlayerHandling

} // namespace proofps_dd
