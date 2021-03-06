#pragma once

/*
    ###################################################################################
    CustomPGE.h
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

class CPlayer
{
private:       
    bool used; 
    int health;
    PRREVector pos, oldpos;
    PRREVector force;
    PRREObject3D* obj;
    PR00FsReducedRenderingEngine* pGFX;
    float gravity;
    bool jumping;
    bool canfall;
    bool running;

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

} ;

/**
    The customized game engine class. This handles the game logic. Singleton.
*/
class CustomPGE :
    public PGE
{

public:

    static CustomPGE* createAndGetCustomPGEinstance();
    static const char* getLoggerModuleName();

    // ---------------------------------------------------------------------------

    CConsole& getConsole() const;
    
   
protected:

    CustomPGE() :
        maps(getPRRE())
    {}

    CustomPGE(const CustomPGE&) :
        maps(getPRRE())
    {}

    CustomPGE& operator=(const CustomPGE&)
    {
        return *this;
    }

    explicit CustomPGE(const char* gametitle);  /**< This is the only usable ctor, this is used by the static createAndGet(). */
    virtual ~CustomPGE();

    virtual void onGameInitializing(); /**< Must-have minimal stuff before loading anything. */
    virtual void onGameInitialized();  /**< Loading game content here. */
    virtual void onGameRunning();      /**< Game logic here. */
    virtual void onGameDestroying();   /**< Freeing up game content here. */

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

    Maps maps;

    int m_fps, fps_counter;                 /* fps m?r?s?hez seg?dv?ltoz?k */
    unsigned int fps_lastmeasure;         /* - || - */
    unsigned int m_fps_ms;                  /* - || - */

    CPlayer player;
    PRREObject3D* xhair;
    PRRETexture* playertex;
    bool bAllowJump;
    bool spacereleased, ctrlreleased;
    bool shiftreleased, enterreleased;
    bool m_won;
    float cameraMinY;

    // ---------------------------------------------------------------------------


}; // class CustomPGE