#pragma once

/*
    ###################################################################################
    PRooFPSddPGE.h
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <chrono>

#include "../../../PGE/PGE/PGE.h"
#include "../../../PGE/PGE/PRRE/include/external/Object3D/PRREObject3DManager.h"

#include "Consts.h"
#include "GameMode.h"
#include "Maps.h"
#include "PRooFPS-dd-packet.h"

class CPlayer
{
private:       
    int m_nHealth, m_nOldHealth;
    PRREVector m_vecPos, m_vecOldPos;
    TPRREfloat m_fPlayerAngleY, m_fOldPlayerAngleY;
    PRREVector m_vWpnAngle, m_vOldWpnAngle;
    PRREVector m_vecForce;
    PRREObject3D* m_pObj;
    std::vector<Weapon*> m_weapons;
    Weapon* m_pWpn;
    std::chrono::time_point<std::chrono::steady_clock> m_timeLastWeaponSwitch;
    PR00FsReducedRenderingEngine* pGFX;
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

public:
    CPlayer();
    void ShutDown();
    void SetRendererObject(PR00FsReducedRenderingEngine* gfx);
    int getHealth() const;
    PRREVector& getPos1();
    PRREVector& getOPos1();
    TPRREfloat& getAngleY();
    TPRREfloat& getOldAngleY();
    PRREObject3D* getAttachedObject() const;
    float getGravity() const;
    bool isJumping() const;
    bool isFalling() const;
    bool canFall() const;
    void UpdateOldPos();
    void SetHealth(int value);
    void UpdateOldHealth();
    int getOldHealth() const;
    void AttachObject(PRREObject3D* value, bool blend);
    void SetGravity(float value);
    bool jumpAllowed() const;
    void SetJumpAllowed(bool b);
    void Jump();
    void StopJumping();
    void DoDamage(int dmg); 
    void SetCanFall(bool state);
    bool isRunning() const;
    void SetRun(bool state);
    PRREVector& getForce();
    void UpdateForce(float x, float y, float z);
    bool isExpectingStartPos() const;
    void SetExpectingStartPos(bool b);
    Weapon* getWeapon();
    const Weapon* getWeapon() const;
    void SetWeapon(Weapon* wpn, bool bRecordSwitchTime);
    std::chrono::time_point<std::chrono::steady_clock>& getTimeLastWeaponSwitch();
    std::vector<Weapon*>& getWeapons();
    const std::vector<Weapon*>& getWeapons() const;
    const Weapon* getWeaponByFilename(const std::string& sFilename) const;
    Weapon* getWeaponByFilename(const std::string& sFilename);
    PRREVector& getOldWeaponAngle();
    PRREVector& getWeaponAngle();
    std::chrono::time_point<std::chrono::steady_clock>& getTimeDied();
    bool& getRespawnFlag();
    int& getFrags();
    const int& getFrags() const;
    int& getOldFrags();
    int& getDeaths();
    const int& getDeaths() const;
    int& getOldDeaths();
    void UpdateFragsDeaths();
    bool canTakeItem(const MapItem& item, const std::map<MapItemType, std::string>& mapItemTypeToWeaponName) const;
    void TakeItem(MapItem& item, const std::map<MapItemType, std::string>& mapItemTypeToWeaponName, pge_network::PgePacket& pktWpnUpdate);
};

struct Player_t
{
    pge_network::PgeNetworkConnectionHandle m_connHandleServerSide;   /**< Used by both server and clients to identify the connection.
                                                                           Clients don't use it for direct communication.
                                                                           Note: this is the client's handle on server side!
                                                                           This is not the same handle as client have for the connection
                                                                           towards the server! Those connection handles are not related
                                                                           to each other! */
    std::string m_sIpAddress;

    CPlayer m_legacyPlayer;
};

/**
    The customized game engine class. This handles the game logic. Singleton.
*/
class PRooFPSddPGE :
    public PGE
{

public:

    static PRooFPSddPGE* createAndGetPRooFPSddPGEinstance();
    static const char* getLoggerModuleName();

    // ---------------------------------------------------------------------------

    CConsole& getConsole() const;
    
   
protected:

    PRooFPSddPGE() :
        m_gameMode(nullptr),
        m_deathMatchMode(nullptr),
        m_maps(getPRRE())
    {}

    PRooFPSddPGE(const PRooFPSddPGE&) :
        m_gameMode(nullptr),
        m_deathMatchMode(nullptr),
        m_maps(getPRRE())
    {}

    PRooFPSddPGE& operator=(const PRooFPSddPGE&)
    {
        return *this;
    }

    explicit PRooFPSddPGE(const char* gametitle);  /**< This is the only usable ctor, this is used by the static createAndGet(). */
    virtual ~PRooFPSddPGE();

    virtual void onGameInitializing() override;               /**< Must-have minimal stuff before loading anything. */
    virtual void onGameInitialized() override;                /**< Loading game content here. */
    virtual void onGameFrameBegin() override;                 /**< Game logic right before the engine would do anything. */
    virtual void onGameRunning() override;                    /**< Game logic for each frame. */
    virtual void onPacketReceived(
        pge_network::PgeNetworkConnectionHandle connHandle,
        const pge_network::PgePacket& pkt) override;          /**< Called when a new network packet is received. */
    virtual void onGameDestroying() override;                 /**< Freeing up game content here. */

private:

    static const std::map<MapItemType, std::string> m_mapItemTypeToWeaponFilename;

    static const unsigned int m_nWeaponActionMinimumWaitMillisecondsAfterSwitch = 1000;

    struct KeyReleasedAndWeaponFilenamePair
    {
        bool m_bReleased;
        std::string m_sWpnFilename;
    };
    static std::map<unsigned char, KeyReleasedAndWeaponFilenamePair> m_mapKeypressToWeapon;

    proofps_dd::GameMode* m_gameMode;
    proofps_dd::DeathMatchMode* m_deathMatchMode;
    std::string m_sServerMapFilenameToLoad;
    Maps m_maps;

    int m_fps, m_fps_counter;               /* fps méréséhez segédváltozók */
    unsigned int m_fps_lastmeasure;         /* - || - */
    unsigned int m_fps_ms;                  /* - || - */

    PRREObject3D* m_pObjXHair;
    bool m_bSpaceReleased, m_bBackSpaceReleased, m_bCtrlReleased;
    bool m_bShiftReleased, m_enterreleased;
    bool m_bTeleportReleased;
    bool m_bWon;
    float m_fCameraMinY;
    bool m_bShowGuiDemo;

    std::string m_sUserName;   /**< User name received from server in PgePktUserConnected (server instance also receives this from itself). */
    std::map<std::string, Player_t> m_mapPlayers;  /**< Connected players. Used by both server and clients. Key is user name. */
    // TODO: originally username was planned to be the key for above map, however if we see that we can always use connHandleServerSide to
    // find proper player, then let's change the key to that instead of user name!

    // ---------------------------------------------------------------------------

    void KeyBoard(int fps, bool& won, pge_network::PgePacket& pkt);
    bool Mouse(int fps, bool& won, pge_network::PgePacket& pkt);
    void MouseWheel(const short int& nMouseWheelChange, pge_network::PgePacket& pkt);
    void CameraMovement(int fps);
    void Gravity(int fps);
    bool Colliding(const PRREObject3D& a, const PRREObject3D& b);
    bool Colliding2(
        float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
        float o2px, float o2py, float o2pz, float o2sx, float o2sy, float o2sz);
    bool Colliding3(
        const PRREVector& vecPosMin, const PRREVector& vecPosMax,
        const PRREVector& vecObjPos, const PRREVector& vecObjSize);
    void Collision(bool& won);
    void ShowFragTable(bool bWin) const;
    void UpdateWeapons();
    void UpdateBullets();
    void SendUserUpdates();
    void HandlePlayerDied(bool bMe, CPlayer& player);
    void HandlePlayerRespawned(bool bMe, CPlayer& player);
    void UpdateRespawnTimers();
    void UpdateGameMode();
    void PickupAndRespawnItems();
    void genUniqueUserName(char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength]) const;
    std::map<std::string, Player_t>::iterator getPlayerMapItByConnectionHandle(pge_network::PgeNetworkConnectionHandle connHandleServerSide);
    void WritePlayerList();
    void HandleUserSetup(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserSetup& msg);
    void HandleUserConnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserConnected& msg);
    void HandleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnected& msg);
    void HandleUserCmdMove(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserCmdMove& msg);
    void HandleUserUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserUpdate& msg);
    void HandleBulletUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgBulletUpdate& msg);
    void HandleMapItemUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgMapItemUpdate& msg);
    void HandleWpnUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgWpnUpdate& msg);
    void HandleWpnUpdateCurrent(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgWpnUpdateCurrent& msg);

}; // class PRooFPSddPGE