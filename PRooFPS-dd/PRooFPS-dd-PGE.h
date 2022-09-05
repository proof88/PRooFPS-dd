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

#include "../../../PGE/PGE/PGE.h"
#include "../../../PGE/PGE/PRRE/include/external/Object3D/PRREObject3DManager.h"

#include "Consts.h"
#include "Maps.h"
#include "PRooFPS-dd-packet.h"

class CPlayer
{
private:       
    int m_nHealth;
    PRREVector m_vecPos, m_vecOldPos;
    PRREVector m_vecForce;
    PRREObject3D* m_pObj;
    PR00FsReducedRenderingEngine* pGFX;
    float m_fGravity;
    bool m_bJumping;
    bool b_mCanFall;
    bool m_bRunning;

public:
    CPlayer();
    void ShutDown();
    void SetRendererObject(PR00FsReducedRenderingEngine* gfx);
    int getHealth() const;
    PRREVector& getPos1();
    PRREVector& getOPos1();
    PRREObject3D* getAttachedObject() const;
    void UpdatePositions(const PRREVector& targetPos);
    float getGravity() const;
    bool isJumping() const;
    bool isFalling() const;
    bool canFall() const;
    void UpdateOldPos();
    void SetHealth(int value);
    void AttachObject(PRREObject3D* value, bool blend);
    void SetGravity(float value);
    void Jump();
    void StopJumping();
    void DoDamage(int dmg); 
    void SetCanFall(bool state);
    bool isRunning() const;
    void SetRun(bool state);
    PRREVector& getForce();
    void UpdateForce(float x, float y, float z);

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
        m_maps(getPRRE())
    {}

    PRooFPSddPGE(const PRooFPSddPGE&) :
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
    virtual void onGameRunning() override;                    /**< Game logic here. */
    virtual void onPacketReceived(
        pge_network::PgeNetworkConnectionHandle connHandle,
        const pge_network::PgePacket& pkt) override;          /**< Called when a new network packet is received. */
    virtual void onGameDestroying() override;                 /**< Freeing up game content here. */

    void KeyBoard(int fps, bool& won);
    void Mouse(int /*fps*/, bool& won);
    void CameraMovement(int fps);
    void Gravity(int fps);
    bool Colliding(PRREObject3D& a, PRREObject3D& b);
    bool Colliding2( float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
                     float o2px, float o2py, float o2pz, float o2sx, float o2sy, float o2sz );
    void Collision(bool& won);
    void FrameLimiter(int fps_ms);
    void UpdateBullets();

private:

    Maps m_maps;

    int m_fps, m_fps_counter;                 /* fps méréséhez segédváltozók */
    unsigned int m_fps_lastmeasure;         /* - || - */
    unsigned int m_fps_ms;                  /* - || - */

    PRREObject3D* m_pObjXHair;
    bool m_bAllowJump;
    bool m_bSpaceReleased, m_bCtrlReleased;
    bool m_bShiftReleased, m_enterreleased;
    bool m_bWon;
    float m_fCameraMinY;

    std::string m_sUserName;   /**< User name received from server in PgePktUserConnected (server instance also receives this from itself). */
    std::map<std::string, Player_t> m_mapPlayers;  /**< Connected players. Used by both server and clients. Key is user name. */
    // TODO: originally username was planned to be the key for above map, however if we see that we can always use connHandleServerSide to
    // find proper player, then let's change the key to that instead of user name!

    // ---------------------------------------------------------------------------

    void genUniqueUserName(char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength]) const;
    void WritePlayerList();
    void HandleUserSetup(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserSetup& msg);
    void HandleUserConnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserConnected& msg);
    void HandleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnected& msg);
    void HandleUserCmdMove(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserCmdMove& msg);
    void HandleUserUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserUpdate& msg);


}; // class PRooFPSddPGE