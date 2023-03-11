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

#include <chrono>  // requires cpp11

#include "../../../PGE/PGE/PGE.h"
#include "../../../PGE/PGE/Pure/include/external/Object3D/PureObject3DManager.h"

#include "Consts.h"
#include "GameMode.h"
#include "Maps.h"
#include "PRooFPS-dd-packet.h"

class CPlayer
{
private:       
    int m_nHealth, m_nOldHealth;
    PureVector m_vecPos, m_vecOldPos;
    TPureFloat m_fPlayerAngleY, m_fOldPlayerAngleY;
    PureVector m_vWpnAngle, m_vOldWpnAngle;
    PureVector m_vecForce;
    PureObject3D* m_pObj;
    std::vector<Weapon*> m_weapons;
    Weapon* m_pWpn;
    std::chrono::time_point<std::chrono::steady_clock> m_timeLastWeaponSwitch;
    PR00FsUltimateRenderingEngine* pGFX;
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
    void SetRendererObject(PR00FsUltimateRenderingEngine* gfx);
    int getHealth() const;
    PureVector& getPos1();
    PureVector& getOPos1();
    TPureFloat& getAngleY();
    TPureFloat& getOldAngleY();
    PureObject3D* getAttachedObject() const;
    float getGravity() const;
    bool isJumping() const;
    bool isFalling() const;
    bool canFall() const;
    void UpdateOldPos();
    void SetHealth(int value);
    void UpdateOldHealth();
    int getOldHealth() const;
    void AttachObject(PureObject3D* value, bool blend);
    void SetGravity(float value);
    bool jumpAllowed() const;
    void SetJumpAllowed(bool b);
    void Jump();
    void StopJumping();
    void DoDamage(int dmg); 
    void SetCanFall(bool state);
    bool isRunning() const;
    void SetRun(bool state);
    PureVector& getForce();
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
        m_maps(getPure())
    {}

    PRooFPSddPGE(const PRooFPSddPGE&) :
        m_gameMode(nullptr),
        m_deathMatchMode(nullptr),
        m_maps(getPure())
    {}

    PRooFPSddPGE& operator=(const PRooFPSddPGE&)
    {
        return *this;
    }

    explicit PRooFPSddPGE(const char* gametitle);  /**< This is the only usable ctor, this is used by the static createAndGet(). */
    virtual ~PRooFPSddPGE();

    virtual bool onGameInitializing() override;               /**< Must-have minimal stuff before loading anything. */
    virtual bool onGameInitialized() override;                /**< Loading game content here. */
    virtual void onGameFrameBegin() override;                 /**< Game logic right before the engine would do anything. */
    virtual void onGameRunning() override;                    /**< Game logic for each frame. */
    virtual bool onPacketReceived(
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

    PureObject3D* m_pObjXHair;
    bool m_bSpaceReleased, m_bBackSpaceReleased, m_bCtrlReleased;
    bool m_bShiftReleased, m_enterreleased;
    bool m_bTeleportReleased;
    bool m_bReloadReleased;
    bool m_bWon;
    float m_fCameraMinY;
    bool m_bShowGuiDemo;

    std::string m_sUserName;   /**< User name received from server in PgePktUserConnected (server instance also receives this from itself). */
    std::map<std::string, Player_t> m_mapPlayers;  /**< Connected players. Used by both server and clients. Key is user name. */
    // TODO: originally username was planned to be the key for above map, however if we see that we can always use connHandleServerSide to
    // find proper player, then let's change the key to that instead of user name!

    SoLoud::Wav m_sndLetsgo;
    SoLoud::Wav m_sndReloadStart;
    SoLoud::Wav m_sndReloadFinish;
    SoLoud::Wav m_sndShootPistol;
    SoLoud::Wav m_sndShootMchgun;
    SoLoud::Wav m_sndShootDryPistol;
    SoLoud::Wav m_sndShootDryMchgun;
    SoLoud::Wav m_sndChangeWeapon;
    SoLoud::Wav m_sndPlayerDie;

    unsigned int m_nFramesElapsedSinceLastDurationsReset;
    long long m_nGravityCollisionDurationUSecs;
    long long m_nActiveWindowStuffDurationUSecs;
    long long m_nUpdateWeaponsDurationUSecs;
    long long m_nUpdateBulletsDurationUSecs;
    long long m_nUpdateRespawnTimersDurationUSecs;
    long long m_nPickupAndRespawnItemsDurationUSecs;
    long long m_nUpdateGameModeDurationUSecs;
    long long m_nSendUserUpdatesDurationUSecs;
    long long m_nFullOnGameRunningDurationUSecs;
    long long m_nHandleUserCmdMoveDurationUSecs;
    long long m_nFullOnPacketReceivedDurationUSecs;
    long long m_nFullRoundtripDurationUSecs;
    std::chrono::time_point<std::chrono::steady_clock> m_timeFullRoundtripStart;

    // ---------------------------------------------------------------------------

    void LoadSound(SoLoud::Wav& snd, const char* fname);
    void Text(const std::string& s, int x, int y) const;
    void AddText(const std::string& s, int x, int y) const;
    void KeyBoard(int fps, bool& won, pge_network::PgePacket& pkt);
    bool Mouse(int fps, bool& won, pge_network::PgePacket& pkt);
    void MouseWheel(const short int& nMouseWheelChange, pge_network::PgePacket& pkt);
    void CameraMovement(int fps);
    void Gravity(int fps);
    bool Colliding(const PureObject3D& a, const PureObject3D& b);
    bool Colliding2(
        float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
        float o2px, float o2py, float o2pz, float o2sx, float o2sy, float o2sz);
    bool Colliding2_NoZ(
        float o1px, float o1py, float o1sx, float o1sy,
        float o2px, float o2py, float o2sx, float o2sy);
    bool Colliding3(
        const PureVector& vecPosMin, const PureVector& vecPosMax,
        const PureVector& vecObjPos, const PureVector& vecObjSize);
    void PlayerCollisionWithWalls(bool& won);
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
    bool handleUserSetup(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserSetup& msg);
    bool handleUserConnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserConnected& msg);
    bool handleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnected& msg);
    bool handleUserCmdMove(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserCmdMove& msg);
    bool handleUserUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserUpdate& msg);
    bool handleBulletUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgBulletUpdate& msg);
    bool handleMapItemUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgMapItemUpdate& msg);
    bool handleWpnUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgWpnUpdate& msg);
    bool handleWpnUpdateCurrent(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgWpnUpdateCurrent& msg);
    void RegTestDumpToFile();  // TODO: could be const if m_mapPlayers wouldnt be used with [] operator ...

}; // class PRooFPSddPGE