#pragma once

/*
    ###################################################################################
    Player.h
    Player class for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <chrono>  // requires cpp11
#include <map>
#include <vector>

#include "../../../CConsole/CConsole/src/CConsole.h"

#include "../../../PGE/PGE/Network/PgePacket.h"
#include "../../../PGE/PGE/Pure/include/external/PR00FsUltimateRenderingEngine.h"
#include "../../../PGE/PGE/Weapons/WeaponManager.h"

#include "Consts.h"
#include "MapItem.h"

class Player
{
public:

    static const char* getLoggerModuleName();

    // ---------------------------------------------------------------------------

    CConsole& getConsole() const;

    // TODO: we should pass the network instance also, so Player can always check if network instance is server or not,
    // and this also enables clearer testing
    explicit Player(
        PR00FsUltimateRenderingEngine& gfx,
        const pge_network::PgeNetworkConnectionHandle& connHandle,
        const std::string& sIpAddress);
    Player(const Player&);
    Player& operator=(const Player&);
    // TODO: add move ctor and move assignment operator
    ~Player();

    const pge_network::PgeNetworkConnectionHandle& getServerSideConnectionHandle() const;
    const std::string& getIpAddress() const;

    int getHealth() const;
    PureVector& getPos();
    PureVector& getOPos();
    TPureFloat& getAngleY();
    TPureFloat& getOldAngleY();
    PureObject3D* getObject3D() const;
    float getGravity() const;
    bool isJumping() const;
    bool isFalling() const;
    bool canFall() const;
    void UpdateOldPos();
    void SetHealth(int value);
    void UpdateOldHealth();
    int getOldHealth() const;
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
    void Respawn(bool bMe, const Weapon& wpnDefaultAvailable);
    PureVector& getForce();
    bool isExpectingStartPos() const;
    void SetExpectingStartPos(bool b);
    Weapon* getWeapon();
    const Weapon* getWeapon() const;
    void SetWeapon(Weapon* wpn, bool bRecordSwitchTime);
    std::chrono::time_point<std::chrono::steady_clock>& getTimeLastWeaponSwitch();
    std::vector<Weapon*>& getWeapons(); // TODO: this will be get rid of after Player will have its own WeaponManager instance
    const std::vector<Weapon*>& getWeapons() const; // TODO: this will be get rid of after Player will have its own WeaponManager instance
    const Weapon* getWeaponByFilename(const std::string& sFilename) const; // TODO: this will be get rid of after Player will have its own WeaponManager instance
    Weapon* getWeaponByFilename(const std::string& sFilename); // TODO: this will be get rid of after Player will have its own WeaponManager instance
    PureVector& getOldWeaponAngle();
    PureVector& getWeaponAngle();
    std::chrono::time_point<std::chrono::steady_clock>& getTimeDied();
    bool& getRespawnFlag();
    int& getFrags();
    const int& getFrags() const;
    int& getOldFrags();
    int& getDeaths();
    const int& getDeaths() const;
    int& getOldDeaths();
    void UpdateFragsDeaths();
    bool canTakeItem(const MapItem& item) const;
    void TakeItem(MapItem& item, pge_network::PgePacket& pktWpnUpdate);

private:

    static const std::map<MapItemType, std::string> m_mapItemTypeToWeaponFilename;

    pge_network::PgeNetworkConnectionHandle m_connHandleServerSide;   /**< Used by both server and clients to identify the connection.
                                                                       Clients don't use it for direct communication.
                                                                       Note: this is the client's handle on server side!
                                                                       This is not the same handle as client has for the connection
                                                                       towards the server, those connection handles are not related
                                                                       to each other! */

    

    std::string m_sIpAddress; // TODO: this should be either in the engine, or wait until we move this class to the engine

    int m_nHealth, m_nOldHealth;
    PureVector m_vecPos, m_vecOldPos;
    TPureFloat m_fPlayerAngleY, m_fOldPlayerAngleY;
    PureVector m_vWpnAngle, m_vOldWpnAngle;
    PureVector m_vecForce;
    PureObject3D* m_pObj;
    std::vector<Weapon*> m_weapons;
    Weapon* m_pWpn;
    std::chrono::time_point<std::chrono::steady_clock> m_timeLastWeaponSwitch;
    PR00FsUltimateRenderingEngine& m_gfx;
    float m_fGravity;
    bool m_bJumping;
    bool b_mCanFall;
    bool m_bRunning;
    bool m_bAllowJump;
    bool m_bExpectingStartPos;
    std::chrono::time_point<std::chrono::steady_clock> m_timeDied;
    bool m_bRespawn;
    int m_nFrags, m_nOldFrags;
    int m_nDeaths, m_nOldDeaths;

    // ---------------------------------------------------------------------------

    void BuildPlayerObject(bool blend);

}; // class Player
