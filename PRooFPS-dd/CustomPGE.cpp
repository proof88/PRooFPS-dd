/*
    ###################################################################################
    CustomPGE.cpp
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "CustomPGE.h"

#include "../../../PGE/PGE/PRRE/include/external/PRREuiManager.h"
#include "../../../PGE/PGE/PRRE/include/external/Display/PRREWindow.h"
#include "../../../PGE/PGE/PRRE/include/external/PRRECamera.h"
#include "../../../CConsole/CConsole/src/CConsole.h"


static const std::string GAME_NAME    = "PRooFPS-dd";
static const std::string GAME_VERSION = "0.1.0.0 Alpha";


// ############################### PUBLIC ################################


CPlayer::CPlayer()
{
  health = 100;
  obj = PGENULL;
  pGFX = NULL;
  gravity = 0.0;
  jumping = false;
  canfall = true;
  running = false;
}

void CPlayer::ShutDown()
{
    if ( obj != PGENULL )
    {
        delete obj;
        obj = PGENULL;
    }
    pGFX = NULL;
}

/* beállítja a megjelenítõ objektumot */
void CPlayer::SetRendererObject(PR00FsReducedRenderingEngine* gfx)
{
  pGFX = gfx;
}

/* visszaadja a játékos életerejét */
int CPlayer::getHealth() const
{
  return health;
}

PRREVector& CPlayer::getPos1()
{
  return pos;
}

PRREVector& CPlayer::getOPos1()
{
  return oldpos;
}

PRREObject3D* CPlayer::getAttachedObject() const
{
  return obj;
}

float CPlayer::getGravity() const
{
  return gravity;
}

bool CPlayer::isJumping() const
{
  return jumping;
}

bool CPlayer::isFalling() const
{
  return ( gravity == 0.0f );
}

bool CPlayer::canFall() const
{
  return canfall;
}

void CPlayer::UpdateOldPos() {
  oldpos = pos;
}

void CPlayer::SetHealth(int value) {
  health = value;
}


void CPlayer::AttachObject(PRREObject3D* value, bool blend) {
  obj = value;   
  if ( obj != PGENULL )
  {  
      obj->SetDoubleSided(true);
      if ( blend )
          obj->getMaterial(false).setBlendFuncs(PRRE_SRC_ALPHA, PRRE_ONE_MINUS_SRC_ALPHA);
      obj->SetLit(false);
  }
}

void CPlayer::SetGravity(float value) {
  gravity = value;
}

void CPlayer::Jump() {
  jumping = true;
  gravity = GAME_GRAVITY_MAX;
  force.SetX( pos.getX()-oldpos.getX() );
  force.SetY( pos.getY()-oldpos.getY() );
  force.SetZ( pos.getZ()-oldpos.getZ() );
}

void CPlayer::StopJumping() {
  jumping = false;
}

void CPlayer::DoDamage(int dmg) {
  health = health - dmg;
  if ( health < 0 ) health = 0;
}

void CPlayer::SetCanFall(bool state) {
  canfall = state;
}

bool CPlayer::isRunning() const
{
    return running;
}

void CPlayer::SetRun(bool state)
{
    running = state;
}

PRREVector& CPlayer::getForce()
{
    return force;
}

void CPlayer::UpdateForce(float x, float y, float z)
{
    force.SetX(x);
    force.SetY(y);
    force.SetZ(z);
}


CustomPGE* CustomPGE::createAndGetCustomPGEinstance()
{
    static CustomPGE pgeInstance((GAME_NAME + " " + GAME_VERSION).c_str());
    return &pgeInstance;
}


// ############################## PROTECTED ##############################


/**
    This is the only usable ctor, this is used by the static createAndGet().
*/
CustomPGE::CustomPGE(const char* gameTitle) :
    PGE(gameTitle),
    maps(getPRRE())
{
    fps = 0;
    fps_counter = 0;
    fps_lastmeasure = 0;
    fps_ms = 0;

    spacereleased = true;
    ctrlreleased = true;
    shiftreleased = true;
    enterreleased = true;

    won = false;
    cameraMinY = 0.0f;
}

CustomPGE::~CustomPGE()
{

}

CConsole& CustomPGE::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}


const char* CustomPGE::getLoggerModuleName()
{
    return "CustomPGE";
}

/**
    Must-have minimal stuff before loading anything.
    Game engine calls this before even finishing its own initialization.
*/
void CustomPGE::onGameInitializing()
{
    // Earliest we can enable our own logging
    getConsole().Initialize((GAME_NAME + " " + GAME_VERSION + " log").c_str(), true);
    getConsole().SetLoggingState(getLoggerModuleName(), true);
    getConsole().SetFGColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "999999" );
    getConsole().SetIntsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    getConsole().SetStringsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "FFFFFF" );
    getConsole().SetFloatsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    getConsole().SetBoolsColor( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FFFF" );

    // Turn everything on for development only
    //getConsole().SetLoggingState("4LLM0DUL3S", true);

    CConsole::getConsoleInstance().SetLoggingState(Maps::getLoggerModuleName(), true);
}

/** 
    Loading game content here.
*/
void CustomPGE::onGameInitialized()
{
    getConsole().OLnOI("CustomPGE::onGameInitialized()");

    getPRRE().getCamera().SetNearPlane(0.1f);
    getPRRE().getCamera().SetFarPlane(100.0f);
    getPRRE().getCamera().getPosVec().Set( 0, 0, GAME_CAM_Z );
    getPRRE().getCamera().getTargetVec().Set( 0, 0, -GAME_BLOCK_SIZE_Z );

    maps.initialize();
    maps.load("gamedata/maps/map_test_good.txt");

    player.AttachObject( getPRRE().getObject3DManager().createPlane(GAME_PLAYER_W, GAME_PLAYER_H), true );
    playertex = getPRRE().getTextureManager().createFromFile( "gamedata\\textures\\giraffe1.bmp" );
    player.getAttachedObject()->getMaterial().setTexture( playertex );
    player.getPos1() = maps.getRandomSpawnpoint();

    getPRRE().WriteList();
    getConsole().OOOLn("CustomPGE::onGameInitialized() done!");

    fps_lastmeasure = GetTickCount();
    fps = 0;
}

/** 
    Freeing up game content here.
    Free up everything that has been allocated in onGameInitialized() and onGameRunning().
*/
void CustomPGE::onGameDestroying()
{
    getConsole().OLnOI("CustomPGE::onGameDestroying() ...");
    maps.shutdown();
    getPRRE().getObject3DManager().DeleteAll();

    getConsole().OOOLn("CustomPGE::onGameDestroying() done!");
    getConsole().Deinitialize();
}

void CustomPGE::KeyBoard(int /*fps*/, bool& won)
{
    PRREWindow& wnd = getPRRE().getWindow();
    PGEInputKeyboard& keybd = getInput().getKeyboard();
  float speed;

  if ( player.isRunning() )
      speed = GAME_PLAYER_SPEED2/60.0f;
  else
      speed = GAME_PLAYER_SPEED1/60.0f;
  
  if ( keybd.isKeyPressed(VK_ESCAPE) )
      wnd.Close();
      
  if ( !won )
  {
    if ( keybd.isKeyPressed( VK_LEFT ) || keybd.isKeyPressed((unsigned char)VkKeyScan('a')) )
    {
        if ( !player.isJumping() && !player.isFalling() && bAllowJump )
        {
            player.getPos1().SetX( player.getPos1().getX() - speed );
            player.getAttachedObject()->getAngleVec().SetY( 180.0f );
        }
    }
    if ( keybd.isKeyPressed( VK_RIGHT ) || keybd.isKeyPressed((unsigned char)VkKeyScan('d')) )
    {
        if ( !player.isJumping() && !player.isFalling() && bAllowJump )
        {
            player.getPos1().SetX( player.getPos1().getX() + speed );
            player.getAttachedObject()->getAngleVec().SetY( 0.0f );
        }
    }

    if ( keybd.isKeyPressed( VK_SPACE ) )
    {
        if ( spacereleased )
        {
            if ( !player.isJumping() && bAllowJump && !player.isFalling() )
            {
                player.Jump();
                
                spacereleased = false;
            }
        }
    }
    else
        spacereleased = true;

    if ( keybd.isKeyPressed( VK_CONTROL ) )
    {
          if ( ctrlreleased )
          {
            //BulletManager->Create(Player->GetX(),Player->GetY()-0.15,Player->GetZ(),-1);
            ctrlreleased = false;
          }
    }
    else 
       ctrlreleased = true;

    if ( keybd.isKeyPressed( VK_SHIFT ) )
    {
          if ( shiftreleased )
          {
            //BulletManager->Create(Player->GetX(),Player->GetY()-0.15,Player->GetZ(),-1);
            shiftreleased = false;
            if ( player.isRunning() )
                player.SetRun(false);
            else
                player.SetRun(true);
          }
    }
    else 
       shiftreleased = true;
  }
  else
  {
        if ( keybd.isKeyPressed( VK_RETURN ) )
        {
              if ( enterreleased )
              {

              }
        }
        else 
           enterreleased = true;  
  } // won
}


// két adott objektum ütközik-e egymással
bool CustomPGE::Colliding(PRREObject3D& a, PRREObject3D& b)
{
  if (
       ( 
         (a.getPosVec().getX() - a.getSizeVec().getX()/2 <= b.getPosVec().getX() + b.getSizeVec().getX()/2)
           &&
         (a.getPosVec().getX() + a.getSizeVec().getX()/2 >= b.getPosVec().getX() - b.getSizeVec().getX()/2)
       )
         &&
       ( 
         (a.getPosVec().getY() - a.getSizeVec().getY()/2 <= b.getPosVec().getY() + b.getSizeVec().getY()/2)
           &&
         (a.getPosVec().getY() + a.getSizeVec().getY()/2 >= b.getPosVec().getY() - b.getSizeVec().getY()/2)
       )
         &&
       ( 
         (a.getPosVec().getZ() - a.getSizeVec().getZ()/2 <= b.getPosVec().getZ() + b.getSizeVec().getZ()/2)
           &&
         (a.getPosVec().getZ() + a.getSizeVec().getZ()/2 >= b.getPosVec().getZ() - b.getSizeVec().getZ()/2)
       )
     )
     {
       return true;
     }
    else
     {
       return false; 
     }
}       

// két térbeli terület ütközik-e egymással
bool CustomPGE::Colliding2( float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
                      float o2px, float o2py, float o2pz, float o2sx, float o2sy, float o2sz )
{
  if (
       ( 
         (o1px - o1sx/2 <= o2px + o2sx/2)
           &&
         (o1px + o1sx/2 >= o2px - o2sx/2)
       )
         &&
       ( 
         (o1py - o1sy/2 <= o2py + o2sy/2)
           &&
         (o1py + o1sy/2 >= o2py - o2sy/2)
       )
         &&
       ( 
         (o1pz - o1sz/2 <= o2pz + o2sz/2)
           &&
         (o1pz + o1sz/2 >= o2pz - o2sz/2)
       )
     )
     {
       return true;
     }
    else
     {
       return false; 
     }              
}

// a player ütközéseit kezeli
void CustomPGE::Collision(bool& /*won*/)
{ 
    PRREObject3D* plobj = player.getAttachedObject();
    PRREVector pos, oldpos;
    pos = player.getOPos1();
    oldpos = player.getPos1();

    player.getPos1().SetX( player.getPos1().getX() + player.getForce().getX() );

    if ( player.getOPos1().getY() != player.getPos1().getY() )
    {
        for (int i = 0; i < getPRRE().getObject3DManager().getSize(); i++)
        {
            PRREObject3D* obj = (PRREObject3D*) getPRRE().getObject3DManager().getAttachedAt(i);
            if ( (obj != PGENULL) && (obj != plobj) && (obj->isColliding_TO_BE_REMOVED()) )
            {
              if ( Colliding2( obj->getPosVec().getX(), obj->getPosVec().getY(), obj->getPosVec().getZ(), obj->getSizeVec().getX(), obj->getSizeVec().getY(), obj->getSizeVec().getZ(),
                               player.getOPos1().getX(), player.getPos1().getY(), player.getOPos1().getZ(), plobj->getSizeVec().getX(), plobj->getSizeVec().getY(), plobj->getSizeVec().getZ() )
                 )
              {  
                player.getPos1().SetY( player.getOPos1().getY() );
                if ( obj->getPosVec().getY()+obj->getSizeVec().getY()/2 <= player.getPos1().getY()-GAME_PLAYER_H/2.0f + 0.01f )
                {
                  player.SetCanFall( false );
                  player.getPos1().SetY( obj->getPosVec().getY()+obj->getSizeVec().getY()/2 + GAME_PLAYER_H/2.0f + 0.01f);
                  player.UpdateForce(0.0f, 0.0f, 0.0f);
                }
                else
                {
                    player.SetCanFall( true );
                    
                }
                break;
              }
            }
        }
    }

    if ( player.getOPos1().getX() != player.getPos1().getX() )
    {
        for (int i = 0; i < getPRRE().getObject3DManager().getSize(); i++)
          {
            PRREObject3D* obj = (PRREObject3D*) getPRRE().getObject3DManager().getAttachedAt(i);
            if ( (obj != PGENULL) && (obj != plobj) && (obj->isColliding_TO_BE_REMOVED()) )
            {
              if ( Colliding2( obj->getPosVec().getX(), obj->getPosVec().getY(), obj->getPosVec().getZ(), obj->getSizeVec().getX(), obj->getSizeVec().getY(), obj->getSizeVec().getZ(),
                               player.getPos1().getX(), player.getPos1().getY(), player.getOPos1().getZ(), plobj->getSizeVec().getX(), plobj->getSizeVec().getY(), plobj->getSizeVec().getZ() )
                 )
              {
                player.getPos1().SetX( player.getOPos1().getX() );
                break;
              }
            }
          }
    }
}

void CustomPGE::CameraMovement(int /*fps*/)
{
    PRREVector campos = getPRRE().getCamera().getPosVec();
    float celx, cely;
    float speed = GAME_CAM_SPEED / 60.0f;

    /* ne mehessen túlságosan balra vagy jobbra a kamera */
    //if ( player.getPos1().getX() < maps.getStartPos().getX() )
    //    celx = maps.getStartPos().getX();
    //else
    //    if ( player.getPos1().getX() > maps.getEndPos().getX() )
    //        celx = maps.getEndPos().getX();
    //     else
            celx = player.getPos1().getX();

    /* ne mehessen túlságosan le és fel a kamera */
    //if ( player.getPos1().getY() < cameraMinY )
    //    cely = cameraMinY;
    //else
    //    if ( player.getPos1().getY() > GAME_CAM_MAX_Y )
    //        cely = GAME_CAM_MAX_Y;
    //    else
            cely = player.getPos1().getY();

    /* a játékoshoz igazítjuk a kamerát */
    if ( player.getPos1().getX() != campos.getX() )
    {
        campos.SetX(campos.getX() + ((celx - campos.getX())/speed) );
    }
    if ( player.getPos1().getY() != campos.getY() )
    {
        campos.SetY(campos.getY() + ((cely - campos.getY())/speed) );
    }

    getPRRE().getCamera().getPosVec().Set( campos.getX(), campos.getY(), GAME_CAM_Z );
    getPRRE().getCamera().getTargetVec().Set( campos.getX(), campos.getY(), player.getPos1().getZ() );

} // CameraMovement()


void CustomPGE::Gravity(int fps)
{
  if ( player.isJumping() )
  {
    player.SetGravity( player.getGravity()-GAME_JUMPING_SPEED/(float)fps );
    if ( player.getGravity() < 0.0 )
    {
      player.SetGravity( 0.0 );
      player.StopJumping();
    }
  }
  else
  {
    if ( player.getGravity() > GAME_GRAVITY_MIN )
    {
      player.SetGravity( player.getGravity()-GAME_FALLING_SPEED/(float)fps );
      if ( player.getGravity() < GAME_GRAVITY_MIN ) player.SetGravity( GAME_GRAVITY_MIN );
    }
  }
  player.getPos1().SetY( player.getPos1().getY() + player.getGravity() );
  if ( player.getPos1().getY() < maps.getObjectsMinY()-5.0f )
      player.SetHealth(0);
}

void CustomPGE::FrameLimiter(int fps_ms)
{
  //if ( GAME_MAXFPS > 0 )
  {
    if ( (1000/(float)GAME_MAXFPS) - fps_ms > 0.0f )
        Sleep(PFL::roundi((1000/(float)GAME_MAXFPS) - fps_ms));
    else {
        Sleep(1);
    }
  }
}

/** 
    Game logic here.
    Game engine invokes this in every frame.
    DO NOT make any unnecessary operations here, as this function must always complete below 16 msecs to keep stable 60 fps!
*/
void CustomPGE::onGameRunning()
{
    PRREWindow& window = getPRRE().getWindow();
    const PGEInputHandler& input = PGEInputHandler::createAndGet();

    if ( fps == 0 ) {
        fps = 60;
    }
    fps_ms = GetTickCount();

    /*const int oldmx = input.getMouse().getCursorPosX();
    const int oldmy = input.getMouse().getCursorPosY();*/

    input.getMouse().SetCursorPos(window.getX() + window.getWidth()/2,
                                  window.getY() + window.getHeight()/2);

    if ( input.getKeyboard().isKeyPressed( VK_LEFT ) )
    {
        getPRRE().getCamera().Strafe(-0.1f);
    }
    if ( input.getKeyboard().isKeyPressed( VK_RIGHT ) )
    {
        getPRRE().getCamera().Strafe(0.1f);
    }

    if ( player.getPos1().getY() != player.getOPos1().getY() )
    { // elõzõ frame-ben még tudott zuhanni, tehát egyelõre nem ugorhatunk
        bAllowJump = false;
    }
    else
    {
        bAllowJump = true;
    }

    player.UpdateOldPos();
    KeyBoard(fps, won);
    if ( !won )
    {
        Gravity(fps);
        Collision(won);
    }
    CameraMovement( fps );
    if ( player.getAttachedObject() != PGENULL )
    {
        player.getAttachedObject()->getPosVec().Set( player.getPos1().getX(), player.getPos1().getY(), player.getPos1().getZ() );
    }
    //map.UpdateVisibilitiesForRenderer();

    // képkockaszám limitáló (akkor kell, ha nincs vsync)
    fps_ms = GetTickCount() - fps_ms;
    FrameLimiter(fps_ms);
    // fps mérõ frissítése 
    fps_counter++;
    if ( GetTickCount() - GAME_FPS_INTERVAL >= fps_lastmeasure )
    {
        fps = fps_counter * (1000/GAME_FPS_INTERVAL); 
        fps_counter = 0;
        fps_lastmeasure = GetTickCount();
    } 

    std::stringstream str;
    str << fps;
    window.SetCaption(str.str());
}


// ############################### PRIVATE ###############################

