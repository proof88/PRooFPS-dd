/*
    ###################################################################################
    PRooFPSddPGE.cpp
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "PRooFPS-dd-PGE.h"

#include <cassert>

#include "../../../PGE/PGE/PRRE/include/external/PRREuiManager.h"
#include "../../../PGE/PGE/PRRE/include/external/Display/PRREWindow.h"
#include "../../../PGE/PGE/PRRE/include/external/PRRECamera.h"
#include "../../../CConsole/CConsole/src/CConsole.h"


static const std::string GAME_NAME    = "PRooFPS-dd";
static const std::string GAME_VERSION = "0.1.0.0 Alpha";


// ############################### PUBLIC ################################


CPlayer::CPlayer()
{
  m_nHealth = 100;
  m_pObj = PGENULL;
  m_pWpn = NULL;
  pGFX = NULL;
  m_fGravity = 0.0;
  m_bJumping = false;
  b_mCanFall = true;
  m_bRunning = false;
  m_bAllowJump = false;
  m_bExpectingStartPos = true;
  m_fPlayerAngleY = 0.f;
  m_fOldPlayerAngleY = 0.f;
}

void CPlayer::ShutDown()
{
    if (m_pObj != PGENULL )
    {
        delete m_pObj;
        m_pObj = PGENULL;
    }

    // do not delete m_pWpn, WeaponManager will take care of it
    m_pWpn = NULL;

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
  return m_nHealth;
}

PRREVector& CPlayer::getPos1()
{
  return m_vecPos;
}

PRREVector& CPlayer::getOPos1()
{
  return m_vecOldPos;
}

TPRREfloat& CPlayer::getAngleY()
{
    return m_fPlayerAngleY;
}

TPRREfloat& CPlayer::getOldAngleY()
{
    return m_fOldPlayerAngleY;
}

PRREObject3D* CPlayer::getAttachedObject() const
{
  return m_pObj;
}

float CPlayer::getGravity() const
{
  return m_fGravity;
}

bool CPlayer::isJumping() const
{
  return m_bJumping;
}

bool CPlayer::isFalling() const
{
  return ( m_fGravity == 0.0f );
}

bool CPlayer::canFall() const
{
  return b_mCanFall;
}

void CPlayer::UpdateOldPos() {
    m_vecOldPos = m_vecPos;
    m_fOldPlayerAngleY = m_fPlayerAngleY;
}

void CPlayer::SetHealth(int value) {
  m_nHealth = value;
}

void CPlayer::AttachObject(PRREObject3D* value, bool blend) {
  m_pObj = value;
  if ( m_pObj != PGENULL )
  {  
      m_pObj->SetDoubleSided(true);
      if ( blend )
          m_pObj->getMaterial(false).setBlendFuncs(PRRE_SRC_ALPHA, PRRE_ONE_MINUS_SRC_ALPHA);
      m_pObj->SetLit(false);
  }
}

void CPlayer::SetGravity(float value) {
  m_fGravity = value;
}

bool CPlayer::jumpAllowed() const {
    return m_bAllowJump;
}

void CPlayer::SetJumpAllowed(bool b) {
    m_bAllowJump = b;
}

void CPlayer::Jump() {
  m_bJumping = true;
  m_fGravity = GAME_GRAVITY_MAX;
  m_vecForce.SetX(m_vecPos.getX() - m_vecOldPos.getX() );
  m_vecForce.SetY(m_vecPos.getY() - m_vecOldPos.getY() );
  m_vecForce.SetZ(m_vecPos.getZ() - m_vecOldPos.getZ() );
}

void CPlayer::StopJumping() {
  m_bJumping = false;
}

void CPlayer::DoDamage(int dmg) {
  m_nHealth = m_nHealth - dmg;
  if ( m_nHealth < 0 ) m_nHealth = 0;
}

void CPlayer::SetCanFall(bool state) {
  b_mCanFall = state;
}

bool CPlayer::isRunning() const
{
    return m_bRunning;
}

void CPlayer::SetRun(bool state)
{
    m_bRunning = state;
}

PRREVector& CPlayer::getForce()
{
    return m_vecForce;
}

void CPlayer::UpdateForce(float x, float y, float z)
{
    m_vecForce.SetX(x);
    m_vecForce.SetY(y);
    m_vecForce.SetZ(z);
}

bool CPlayer::isExpectingStartPos() const
{
    return m_bExpectingStartPos;
}

void CPlayer::SetExpectingStartPos(bool b)
{
    m_bExpectingStartPos = b;
}

Weapon* CPlayer::getWeapon()
{
    return m_pWpn;
}

void CPlayer::SetWeapon(Weapon* wpn)
{
    m_pWpn = wpn;
}


PRooFPSddPGE* PRooFPSddPGE::createAndGetPRooFPSddPGEinstance()
{
    static PRooFPSddPGE pgeInstance((GAME_NAME + " " + GAME_VERSION).c_str());
    return &pgeInstance;
}


// ############################## PROTECTED ##############################


/**
    This is the only usable ctor, this is used by the static createAndGet().
*/
PRooFPSddPGE::PRooFPSddPGE(const char* gameTitle) :
    PGE(gameTitle),
    m_maps(getPRRE())
{
    m_fps = 0;
    m_fps_counter = 0;
    m_fps_lastmeasure = 0;
    m_fps_ms = 0;

    m_bSpaceReleased = true;
    m_bCtrlReleased = true;
    m_bShiftReleased = true;
    m_enterreleased = true;

    m_bWon = false;
    m_fCameraMinY = 0.0f;
}

PRooFPSddPGE::~PRooFPSddPGE()
{

}

CConsole& PRooFPSddPGE::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}


const char* PRooFPSddPGE::getLoggerModuleName()
{
    return "PRooFPSddPGE";
}

/**
    Must-have minimal stuff before loading anything.
    Game engine calls this before even finishing its own initialization.
*/
void PRooFPSddPGE::onGameInitializing()
{
    // Earliest we can enable our own logging
    getConsole().Initialize((GAME_NAME + " " + GAME_VERSION + " log").c_str(), true);
    getConsole().SetLoggingState(getLoggerModuleName(), true);
    getConsole().SetFGColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "999999" );
    getConsole().SetIntsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    getConsole().SetStringsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "FFFFFF" );
    getConsole().SetFloatsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    getConsole().SetBoolsColor( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FFFF" );

    // PRooFPSddPGE (game) logs
    getConsole().SetLoggingState(getLoggerModuleName(), true);

    // Network logs
    getConsole().SetLoggingState("PGESysNET", true);
    getConsole().SetLoggingState(getNetwork().getLoggerModuleName(), true);
    getConsole().SetLoggingState(getNetwork().getServer().getLoggerModuleName(), true);
    getConsole().SetLoggingState(getNetwork().getClient().getLoggerModuleName(), true);

    // Misc engine logs
    getConsole().SetLoggingState("PRREWindow", true);

    // Turn everything on for development only
    getConsole().SetLoggingState("4LLM0DUL3S", true);

    // we need PGE::runGame() invoke EVERYTHING even when window is NOT active, and we will decide in onGameRunning() what NOT to do if window is inactive
    SetInactiveLikeActive(true);
}

/** 
    Loading game content here.
*/
void PRooFPSddPGE::onGameInitialized()
{
    getConsole().OLnOI("PRooFPSddPGE::onGameInitialized()");

    getConsole().SetLoggingState("4LLM0DUL3S", false);

    getPRRE().getScreen().SetVSyncEnabled(true);

    getPRRE().getCamera().SetNearPlane(0.1f);
    getPRRE().getCamera().SetFarPlane(100.0f);
    getPRRE().getCamera().getPosVec().Set( 0, 0, GAME_CAM_Z );
    getPRRE().getCamera().getTargetVec().Set( 0, 0, -GAME_BLOCK_SIZE_Z );

    m_maps.initialize();
    assert( m_maps.load("gamedata/maps/map_test_good.txt") );

    m_pObjXHair = getPRRE().getObject3DManager().createPlane(32.f, 32.f);
    m_pObjXHair->SetStickedToScreen(true);
    m_pObjXHair->SetDoubleSided(true);
    m_pObjXHair->SetTestingAgainstZBuffer(false);
    m_pObjXHair->SetLit(false);
    // for bitmaps not having proper alpha bits (e.g. saved by irfanview or mspaint), use (PRRE_SRC_ALPHA, PRRE_ONE)
    // otherwise (bitmaps saved by Flash) just use (PRRE_SRC_ALPHA, PRRE_ONE_MINUS_SRC_ALPHA) to utilize real alpha
    m_pObjXHair->getMaterial(false).setBlendFuncs(PRRE_SRC_ALPHA, PRRE_ONE);
    PRRETexture* xhairtex = getPRRE().getTextureManager().createFromFile( "gamedata\\textures\\hud_xhair.bmp" );
    m_pObjXHair->getMaterial().setTexture( xhairtex );

    getPRRE().WriteList();

    if (getNetwork().isServer())
    {
        // MsgUserSetup is also processed by server, but it injects this pkt into its own queue when needed.
        // MsgUserSetup MUST NOT be received by server over network!
        // MsgUserSetup is received only by clients over network!
        getNetwork().getServer().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserSetup::id));

        // MsgUserUpdate is also processed by server, but it injects this pkt into its own queue when needed.
        // MsgUserUpdate MUST NOT be received by server over network!
        // MsgUserUpdate is received only by clients over network!
        getNetwork().getServer().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserUpdate::id));

        if (!getNetwork().getServer().startListening())
        {
            PGE::showErrorDialog("Server has FAILED to start listening!");
            assert(false);
        }
    }
    else
    {
        getNetwork().getClient().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserCmdMove::id));

        if (!getNetwork().getClient().connectToServer("127.0.0.1"))
        {
            PGE::showErrorDialog("Client has FAILED to establish connection to the server!");
            assert(false);
        }
    }

    getConsole().OOOLn("PRooFPSddPGE::onGameInitialized() done!");

    getInput().getMouse().SetCursorPos(
        getPRRE().getWindow().getX() + getPRRE().getWindow().getWidth()/2,
        getPRRE().getWindow().getY() + getPRRE().getWindow().getHeight()/2);
    getPRRE().getWindow().SetCursorVisible(false);
    
    m_fps_lastmeasure = GetTickCount();
    m_fps = 0;
}

//function bulletProc() {
//	this._x += this.xMove * _root.BULLETSPEED;
//	this._y += this.yMove * _root.BULLETSPEED;
//	for (obj in _root.mc_map) {
//		if (typeof(_root.mc_map[obj])=="movieclip"){
//			if ( _root.collides(this._x, this._y, this._width, this._height,
//								_root.mc_map[obj]._x, _root.mc_map[obj]._y, _root.mc_map[obj]._width, _root.mc_map[obj]._height)
//				)
//			{
//				if ( _root.mc_map[obj]._name.substr(0,8) == "mc_snail" ) {
//					if ( _root.mc_map[obj].m_nHealth > 0 ) {
//						_root.mc_map[obj].m_nHealth -= _root.BULLETDAMAGE;
//						//trace(_root.mc_map[obj]._name);
//						if ( _root.mc_map[obj].m_nHealth <= 0 ) {
//							
//							_root.killedSnails++;
//							//trace(_root.killedSnails);
//							_root.gameEnds += 4000;
//							if ( _root.mc_map[obj]._currentframe == 1 ) { _root.mc_map[obj].gotoAndPlay(11); }
//							else if ( _root.mc_map[obj]._currentframe == 2 ) { _root.mc_map[obj].gotoAndPlay(3); }
//						}
//						_root.mc_map.attachMovie("lnk_mc_blood", "mc_blood"+_root.bloodCount, _root.mc_map.getNextHighestDepth());
//						eval("_root.mc_map.mc_blood"+_root.bloodCount)._x = this._x;
//						eval("_root.mc_map.mc_blood"+_root.bloodCount)._y = this._y;
//						_root.bloodCount++;
//						this.onEnterFrame = null;
//						removeMovieClip( this );
//						break;
//					}
//				}
//				if ( _root.mc_map[obj]._name.substr(0,4) == "inst" ) {
//					this.onEnterFrame = null;
//					removeMovieClip( this );
//					break;
//				}
//			}
//		}
//	}
//}

//function Shoot() {
//		_root.mc_bulletpacer.play();
//		var aimShit:Number = _root.randRange(-_root.AIMDIFFICULTY, _root.AIMDIFFICULTY);
//		_root.mc_map.attachMovie("lnk_mc_bullet", "mc_bullet"+_root.bulletCount, _root.mc_map.getNextHighestDepth());
//		eval("_root.mc_map.mc_bullet"+_root.bulletCount)._rotation = this.mc_player_arm1._rotation + aimShit;
//		var neg:Number = 1;
//		if ( this._currentframe == 2 ) { neg = -1 };
//		eval("_root.mc_map.mc_bullet"+_root.bulletCount).xMove = Math.cos((this.mc_player_arm1._rotation+aimShit)*Math.PI/180.0) * neg;
//		eval("_root.mc_map.mc_bullet"+_root.bulletCount).yMove = Math.sin((this.mc_player_arm1._rotation+aimShit)*Math.PI/180.0) * neg;
//		eval("_root.mc_map.mc_bullet"+_root.bulletCount)._x = this.newX + eval("_root.mc_map.mc_bullet"+_root.bulletCount).xMove*this.mc_player_arm1._width;
//		eval("_root.mc_map.mc_bullet"+_root.bulletCount)._y = this.newY+150 + eval("_root.mc_map.mc_bullet"+_root.bulletCount).yMove*this.mc_player_arm1._width;
//		eval("_root.mc_map.mc_bullet"+_root.bulletCount).onEnterFrame = _root.bulletProc;
//		_root.bulletCount++;
//		this.ammo--;
//	}

void PRooFPSddPGE::KeyBoard(int /*fps*/, bool& won, pge_network::PgePacket& pkt)
{
    const PGEInputKeyboard& keybd = getInput().getKeyboard();
  
    if ( keybd.isKeyPressed(VK_ESCAPE) )
    {
        getPRRE().getWindow().Close();
    }
      
    if ( !won )
    {
        proofps_dd::Strafe strafe = proofps_dd::Strafe::NONE;
        if (keybd.isKeyPressed(VK_LEFT) || keybd.isKeyPressed((unsigned char)VkKeyScan('a')))
        {
            strafe = proofps_dd::Strafe::LEFT;
        }
        if (keybd.isKeyPressed(VK_RIGHT) || keybd.isKeyPressed((unsigned char)VkKeyScan('d')))
        {
            strafe = proofps_dd::Strafe::RIGHT;
        }
    
        bool bSendJumpAction = false;
        if ( keybd.isKeyPressed( VK_SPACE ) )
        {
            if (m_bSpaceReleased)
            {
                bSendJumpAction = true;
                m_bSpaceReleased = false;
            }
        }
        else
        {
            m_bSpaceReleased = true;
        }
    
        bool bToggleRunWalk = false;
        if ( keybd.isKeyPressed( VK_SHIFT ) )
        {
            if ( m_bShiftReleased )
            {
                bToggleRunWalk = true;
                m_bShiftReleased = false;
            }
        }
        else
        {
            m_bShiftReleased = true;
        }
    
        if ((strafe != proofps_dd::Strafe::NONE) || bSendJumpAction || bToggleRunWalk)
        {
            proofps_dd::MsgUserCmdMove::setKeybd(pkt, strafe, bSendJumpAction, bToggleRunWalk);
        }
    }
    else
    {
        if ( keybd.isKeyPressed( VK_RETURN ) )
        {
            if ( m_enterreleased )
            {
    
            }
        }
        else
        {
            m_enterreleased = true;
        }
    } // won
}


void PRooFPSddPGE::Mouse(int /*fps*/, bool& /*won*/, pge_network::PgePacket& pkt)
{
    PGEInputMouse& mouse = getInput().getMouse();

    const int oldmx = mouse.getCursorPosX();
    const int oldmy = mouse.getCursorPosY();

    mouse.SetCursorPos(
        getPRRE().getWindow().getX() + getPRRE().getWindow().getWidth()/2,
        getPRRE().getWindow().getY() + getPRRE().getWindow().getHeight()/2);

    const int dx = oldmx - mouse.getCursorPosX();
    const int dy = oldmy - mouse.getCursorPosY();

    if ((dx == 0) && (dy == 0))
    {
        return;
    }

    const TPRREfloat fOldPlayerAngleY = (m_pObjXHair->getPosVec().getX() < 0.f) ? 0.f : 180.f;
    
    m_pObjXHair->getPosVec().Set(
        m_pObjXHair->getPosVec().getX() + dx,
        m_pObjXHair->getPosVec().getY() - dy,
        0.f);
    
    const TPRREfloat fNewPlayerAngleY = (m_pObjXHair->getPosVec().getX() < 0.f) ? 0.f : 180.f;

    if (fOldPlayerAngleY == fNewPlayerAngleY)
    {
        return;
    }

    //if ( mouse.isButtonPressed(PGEInputMouse::MouseButton::MBTN_LEFT) )
    //{
    //    getWeaponManager().getWeapons()[0]->shoot();
    //}

    proofps_dd::MsgUserCmdMove::setAngleY(pkt, fNewPlayerAngleY);
}


// két adott objektum ütközik-e egymással
bool PRooFPSddPGE::Colliding(PRREObject3D& a, PRREObject3D& b)
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
bool PRooFPSddPGE::Colliding2( float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
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

// a m_player ütközéseit kezeli
void PRooFPSddPGE::Collision(bool& /*won*/)
{ 
    for (auto& player : m_mapPlayers)
    {
        auto& legacyPlayer = player.second.m_legacyPlayer;

        const PRREObject3D* const plobj = legacyPlayer.getAttachedObject();

        legacyPlayer.getPos1().SetX(legacyPlayer.getPos1().getX() + legacyPlayer.getForce().getX());

        if (legacyPlayer.getOPos1().getY() != legacyPlayer.getPos1().getY())
        {
            for (int i = 0; i < getPRRE().getObject3DManager().getSize(); i++)
            {
                PRREObject3D* obj = (PRREObject3D*)getPRRE().getObject3DManager().getAttachedAt(i);
                if ((obj != PGENULL) && (obj != plobj) && (obj->isColliding_TO_BE_REMOVED()))
                {
                    if (Colliding2(obj->getPosVec().getX(), obj->getPosVec().getY(), obj->getPosVec().getZ(), obj->getSizeVec().getX(), obj->getSizeVec().getY(), obj->getSizeVec().getZ(),
                        legacyPlayer.getOPos1().getX(), legacyPlayer.getPos1().getY(), legacyPlayer.getOPos1().getZ(), plobj->getSizeVec().getX(), plobj->getSizeVec().getY(), plobj->getSizeVec().getZ())
                        )
                    {
                        legacyPlayer.getPos1().SetY(legacyPlayer.getOPos1().getY());
                        if (obj->getPosVec().getY() + obj->getSizeVec().getY() / 2 <= legacyPlayer.getPos1().getY() - GAME_PLAYER_H / 2.0f + 0.01f)
                        {
                            legacyPlayer.SetCanFall(false);
                            legacyPlayer.getPos1().SetY(obj->getPosVec().getY() + obj->getSizeVec().getY() / 2 + GAME_PLAYER_H / 2.0f + 0.01f);
                            legacyPlayer.UpdateForce(0.0f, 0.0f, 0.0f);
                        }
                        else
                        {
                            legacyPlayer.SetCanFall(true);

                        }
                        break;
                    }
                }
            }
        }

        if (legacyPlayer.getOPos1().getX() != legacyPlayer.getPos1().getX())
        {
            for (int i = 0; i < getPRRE().getObject3DManager().getSize(); i++)
            {
                PRREObject3D* obj = (PRREObject3D*)getPRRE().getObject3DManager().getAttachedAt(i);
                if ((obj != PGENULL) && (obj != plobj) && (obj->isColliding_TO_BE_REMOVED()))
                {
                    if (Colliding2(obj->getPosVec().getX(), obj->getPosVec().getY(), obj->getPosVec().getZ(), obj->getSizeVec().getX(), obj->getSizeVec().getY(), obj->getSizeVec().getZ(),
                        legacyPlayer.getPos1().getX(), legacyPlayer.getPos1().getY(), legacyPlayer.getOPos1().getZ(), plobj->getSizeVec().getX(), plobj->getSizeVec().getY(), plobj->getSizeVec().getZ())
                        )
                    {
                        legacyPlayer.getPos1().SetX(legacyPlayer.getOPos1().getX());
                        break;
                    }
                }
            }
        }
    }
}

void PRooFPSddPGE::CameraMovement(int /*fps*/)
{
    PRREVector campos = getPRRE().getCamera().getPosVec();
    float celx, cely;
    float speed = GAME_CAM_SPEED / 60.0f;

    /* ne mehessen túlságosan balra vagy jobbra a kamera */
    //if ( m_player.getPos1().getX() < m_maps.getStartPos().getX() )
    //    celx = m_maps.getStartPos().getX();
    //else
    //    if ( m_player.getPos1().getX() > m_maps.getEndPos().getX() )
    //        celx = m_maps.getEndPos().getX();
    //     else
            celx = m_mapPlayers[m_sUserName].m_legacyPlayer.getAttachedObject()->getPosVec().getX();

    /* ne mehessen túlságosan le és fel a kamera */
    //if ( m_player.getPos1().getY() < m_fCameraMinY )
    //    cely = m_fCameraMinY;
    //else
    //    if ( m_player.getPos1().getY() > GAME_CAM_MAX_Y )
    //        cely = GAME_CAM_MAX_Y;
    //    else
            cely = m_mapPlayers[m_sUserName].m_legacyPlayer.getAttachedObject()->getPosVec().getY();

    /* a játékoshoz igazítjuk a kamerát */
    if (celx != campos.getX() )
    {
        campos.SetX(campos.getX() + ((celx - campos.getX())/speed) );
    }
    if (cely != campos.getY() )
    {
        campos.SetY(campos.getY() + ((cely - campos.getY())/speed) );
    }

    getPRRE().getCamera().getPosVec().Set( campos.getX(), campos.getY(), GAME_CAM_Z );
    getPRRE().getCamera().getTargetVec().Set( campos.getX(), campos.getY(), m_mapPlayers[m_sUserName].m_legacyPlayer.getAttachedObject()->getPosVec().getZ() );

} // CameraMovement()


void PRooFPSddPGE::Gravity(int fps)
{
    for (auto& player : m_mapPlayers)
    {
        auto& legacyPlayer = player.second.m_legacyPlayer;

        if (legacyPlayer.isJumping())
        {
            legacyPlayer.SetGravity(legacyPlayer.getGravity() - GAME_JUMPING_SPEED / (float)fps);
            if (legacyPlayer.getGravity() < 0.0)
            {
                legacyPlayer.SetGravity(0.0);
                legacyPlayer.StopJumping();
            }
        }
        else
        {
            if (legacyPlayer.getGravity() > GAME_GRAVITY_MIN)
            {
                legacyPlayer.SetGravity(legacyPlayer.getGravity() - GAME_FALLING_SPEED / (float)fps);
                if (legacyPlayer.getGravity() < GAME_GRAVITY_MIN)
                {
                    legacyPlayer.SetGravity(GAME_GRAVITY_MIN);
                }
            }
        }
        legacyPlayer.getPos1().SetY(legacyPlayer.getPos1().getY() + legacyPlayer.getGravity());
        if (legacyPlayer.getPos1().getY() < m_maps.getObjectsMinY() - 5.0f)
        {
            legacyPlayer.SetHealth(0);
        }
    }
}

void PRooFPSddPGE::FrameLimiter(int fps_ms)
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


void PRooFPSddPGE::UpdateBullets()
{
    // on the long run this function needs to be part of the game engine itself, however currently game engine doesn't handle collisions,
    // so once we introduce the collisions to the game engine, it will be an easy move of this function as well there
    std::list<Bullet>& bullets = getWeaponManager().getBullets();
    for (auto& bullet : bullets)
    {
        bullet.Update();
    }
}

void PRooFPSddPGE::SendUserUpdates()
{
    if (!getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): NOT server!", __func__);
        return;
    }

    for (auto& player : m_mapPlayers)
    {
        auto& legacyPlayer = player.second.m_legacyPlayer;

        if ((legacyPlayer.getPos1() != legacyPlayer.getOPos1()) || (legacyPlayer.getOldAngleY() != legacyPlayer.getAngleY()))
        {
            pge_network::PgePacket newPktUserUpdate;
            proofps_dd::MsgUserUpdate::initPkt(
                newPktUserUpdate,
                player.second.m_connHandleServerSide,
                legacyPlayer.getPos1().getX(),
                legacyPlayer.getPos1().getY(),
                legacyPlayer.getPos1().getZ(),
                legacyPlayer.getAngleY());

            for (const auto& sendToThisPlayer : m_mapPlayers)
            {
                if (sendToThisPlayer.second.m_connHandleServerSide == 0)
                {
                    // server injects this packet to own queue
                    getNetwork().getServer().getPacketQueue().push_back(newPktUserUpdate);
                }
                else
                {
                    getNetwork().getServer().SendPacketToClient(sendToThisPlayer.second.m_connHandleServerSide, newPktUserUpdate);
                }
            }
        }
    }
}

/**
    Game logic right before the engine would do anything.
    This is invoked at the very beginning of the main game loop, before processing window messages and incoming network packets.
*/
void PRooFPSddPGE::onGameFrameBegin()
{
    if (getNetwork().isServer())
    {
        for (auto& player : m_mapPlayers)
        {
            auto& legacyPlayer = player.second.m_legacyPlayer;
            if (legacyPlayer.getPos1().getY() != legacyPlayer.getOPos1().getY())
            { // elõzõ frame-ben még tudott zuhanni, tehát egyelõre nem ugorhatunk
                legacyPlayer.SetJumpAllowed(false);
            }
            else
            {
                legacyPlayer.SetJumpAllowed(true);
            }

            legacyPlayer.UpdateOldPos();
        }
    }
}

/** 
    Game logic here.
    Game engine invokes this in every frame.
    DO NOT make any unnecessary operations here, as this function must always complete below 16 msecs to keep stable 60 fps!
*/
void PRooFPSddPGE::onGameRunning()
{
    PRREWindow& window = getPRRE().getWindow();

    if ( m_fps == 0 ) {
        m_fps = 60;
    }
    m_fps_ms = GetTickCount();

    // having a user name means that server accepted the connection and sent us a user name, for which we have initialized our player;
    // otherwise m_mapPlayers[m_sUserName] is dangerous as it implicitly creates entry even with empty username ...
    const bool bValidConnection = !m_sUserName.empty();

    if (bValidConnection)
    {
        if (getNetwork().isServer())
        {
            if (!m_bWon)
            {
                Gravity(m_fps);
                Collision(m_bWon);
                SendUserUpdates();
            }
        }
        
        if (window.isActive())
        {
            pge_network::PgePacket pkt;
            proofps_dd::MsgUserCmdMove::initPkt(pkt);

            KeyBoard(m_fps, m_bWon, pkt);
            Mouse(m_fps, m_bWon, pkt);

            if (proofps_dd::MsgUserCmdMove::shouldSend(pkt))
            {
                if (getNetwork().isServer())
                {
                    // inject this packet to server's queue
                    // server will properly update its own position and send update to all clients too based on current state of HandleUserCmdMove()
                    getNetwork().getServer().getPacketQueue().push_back(pkt);
                }
                else
                {
                    getNetwork().getClient().SendToServer(pkt);
                }
            }
        }

        CameraMovement(m_fps);

        for (auto& player : m_mapPlayers)
        {
            Weapon* const wpn = player.second.m_legacyPlayer.getWeapon();
            if (!wpn)
            {
                continue;
            }

            if (m_sUserName == player.first)
            {
                // my xhair is used to update weapon angle
                wpn->UpdatePositions(player.second.m_legacyPlayer.getAttachedObject()->getPosVec(), m_pObjXHair->getPosVec());
                // I control only my weapon
                wpn->Update();
            }
            else
            {
                wpn->UpdatePosition(player.second.m_legacyPlayer.getAttachedObject()->getPosVec());
            }
        }
        
        if (getNetwork().isServer())
        {
            UpdateBullets();
        }

        //map.UpdateVisibilitiesForRenderer();
    }

    // képkockaszám limitáló (akkor kell, ha nincs vsync)
    m_fps_ms = GetTickCount() - m_fps_ms;
    //FrameLimiter(m_fps_ms);
    // fps mérõ frissítése 
    m_fps_counter++;
    if ( GetTickCount() - GAME_FPS_INTERVAL >= m_fps_lastmeasure )
    {
        m_fps = m_fps_counter * (1000/GAME_FPS_INTERVAL);
        m_fps_counter = 0;
        m_fps_lastmeasure = GetTickCount();
    } 

    std::stringstream str;
    //str << GAME_NAME << " " << GAME_VERSION << " :: " << wpn.getMagBulletCount() << " / " << wpn.getUnmagBulletCount() << " :: FPS: " << m_fps << " :: angleZ: " << wpn.getObject3D().getAngleVec().getZ();
    window.SetCaption(str.str());
}

/**
    Called when a new network packet is received.
*/
void PRooFPSddPGE::onPacketReceived(pge_network::PgeNetworkConnectionHandle m_connHandleServerSide, const pge_network::PgePacket& pkt)
{
    switch (pkt.pktId)
    {
    case pge_network::MsgUserConnected::id:
        HandleUserConnected(m_connHandleServerSide, pkt.msg.userConnected);
        break;
    case pge_network::MsgUserDisconnected::id:
        HandleUserDisconnected(m_connHandleServerSide, pkt.msg.userDisconnected);
        break;
    case pge_network::MsgApp::id:
    {
        switch (static_cast<proofps_dd::ElteFailMsgId>(pkt.msg.app.msgId))
        {
        case proofps_dd::MsgUserSetup::id:
            HandleUserSetup(m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgUserSetup&>(pkt.msg.app.cData));
            break;
        case proofps_dd::MsgUserCmdMove::id:
            HandleUserCmdMove(m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData));
            break;
        case proofps_dd::MsgUserUpdate::id:
            HandleUserUpdate(m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgUserUpdate&>(pkt.msg.app.cData));
            break;
        default:
            getConsole().EOLn("CustomPGE::%s(): unknown msgId %u in MsgApp!", __func__, pkt.pktId);
        }
        break;
    }
    default:
        getConsole().EOLn("CustomPGE::%s(): unknown pktId %u!", __func__, pkt.pktId);
    }
}

/**
    Freeing up game content here.
    Free up everything that has been allocated in onGameInitialized() and onGameRunning().
*/
void PRooFPSddPGE::onGameDestroying()
{
    getConsole().OLnOI("PRooFPSddPGE::onGameDestroying() ...");
    m_maps.shutdown();
    getPRRE().getObject3DManager().DeleteAll();
    getPRRE().getWindow().SetCursorVisible(true);

    getConsole().OOOLn("PRooFPSddPGE::onGameDestroying() done!");
    getConsole().Deinitialize();
}


// ############################### PRIVATE ###############################


void PRooFPSddPGE::genUniqueUserName(char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength]) const
{
    bool found = false;
    do
    {
        sprintf_s(szNewUserName, proofps_dd::MsgUserSetup::nUserNameMaxLength, "User%d", 10000 + (rand() % 100000));
        for (const auto& client : m_mapPlayers)
        {
            found = (client.first == szNewUserName);
            if (found)
            {
                break;
            }
        }
    } while (found);
}

void PRooFPSddPGE::WritePlayerList()
{
    getConsole().OLnOI("PRooFPSddPGE::%s()", __func__);
    for (const auto& player : m_mapPlayers)
    {
        getConsole().OLn("Username: %s; connHandleServerSide: %u; address: %s",
            player.first.c_str(), player.second.m_connHandleServerSide, player.second.m_sIpAddress.c_str());
    }
    getConsole().OO();
}

void PRooFPSddPGE::HandleUserSetup(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserSetup& msg)
{
    if ((strnlen(msg.m_szUserName, proofps_dd::MsgUserSetup::nUserNameMaxLength) > 0) && (m_mapPlayers.end() != m_mapPlayers.find(msg.m_szUserName)))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): cannot happen: user %s (connHandleServerSide: %u) is already present in players list!",
            __func__, msg.m_szUserName, connHandleServerSide);
        assert(false);
        return;
    }

    assert(0 != strnlen(msg.m_szUserName, sizeof(msg.m_szUserName)));

    if (msg.m_bCurrentClient)
    {
        getConsole().OLn("PRooFPSddPGE::%s(): this is me, my name is %s, connHandleServerSide: %u, my IP: %s",
            __func__, msg.m_szUserName, connHandleServerSide, msg.m_szIpAddress);
        // store our username so we can refer to it anytime later
        m_sUserName = msg.m_szUserName;

        if (getNetwork().isServer())
        {
            getPRRE().getUImanager().addText("Server, User name: " + m_sUserName, 10, 30);
        }
        else
        {
            getPRRE().getUImanager().addText("Client, User name: " + m_sUserName + "; IP: " + msg.m_szIpAddress, 10, 30);
        }
    }
    else
    {
        getConsole().OLn("PRooFPSddPGE::%s(): new user %s (connHandleServerSide: %u; IP: %s) connected",
            __func__, msg.m_szUserName, connHandleServerSide, msg.m_szIpAddress);
    }

    // insert user into map using wacky syntax
    m_mapPlayers[msg.m_szUserName];
    m_mapPlayers[msg.m_szUserName].m_connHandleServerSide = connHandleServerSide;
    m_mapPlayers[msg.m_szUserName].m_sIpAddress = msg.m_szIpAddress;

    PRREObject3D* const plane = getPRRE().getObject3DManager().createPlane(GAME_PLAYER_W, GAME_PLAYER_H);
    if (!plane)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to create object for user %s!", __func__, msg.m_szUserName);
        // TODO: should exit or sg
        return;
    }

    m_mapPlayers[msg.m_szUserName].m_legacyPlayer.AttachObject(plane, true);
    PRRETexture* pTexPlayer = getPRRE().getTextureManager().createFromFile("gamedata\\textures\\giraffe1m.bmp");
    plane->getMaterial().setTexture(pTexPlayer);

    assert(getWeaponManager().load("gamedata/weapons/machinegun.txt"));
    m_mapPlayers[msg.m_szUserName].m_legacyPlayer.SetWeapon(getWeaponManager().getWeapons()[getWeaponManager().getWeapons().size() - 1]);

    getNetwork().WriteList();
    WritePlayerList();
}

void PRooFPSddPGE::HandleUserConnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserConnected& msg)
{
    if (!getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): client received MsgUserConnected, CANNOT HAPPEN!", __func__);
        assert(false);
        return;
    }

    const char* szConnectedUserName = nullptr;

    const PRREVector& vecStartPos = m_maps.getRandomSpawnpoint();
    pge_network::PgePacket newPktUserUpdate;
    proofps_dd::MsgUserUpdate::initPkt(newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f);

    if (msg.bCurrentClient)
    {
        // server is processing its own birth
        if (m_mapPlayers.size() == 0)
        {
            char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength];
            genUniqueUserName(szNewUserName);
            getConsole().OLn("PRooFPSddPGE::%s(): first (local) user %s connected and I'm server, so this is me (connHandleServerSide: %u)",
                __func__, szNewUserName, connHandleServerSide);

            szConnectedUserName = szNewUserName;

            pge_network::PgePacket newPktSetup;
            proofps_dd::MsgUserSetup::initPkt(newPktSetup, connHandleServerSide, true, szConnectedUserName, msg.szIpAddress);

            // server injects this msg to self so resources for player will be allocated
            getNetwork().getServer().getPacketQueue().push_back(newPktSetup);
            getNetwork().getServer().getPacketQueue().push_back(newPktUserUpdate);
        }
        else
        {
            // cannot happen
            getConsole().EOLn("PRooFPSddPGE::%s(): user (connHandleServerSide: %u) connected with bCurrentClient as true but it is not me, CANNOT HAPPEN!",
                __func__, connHandleServerSide);
            assert(false);
            return;
        }
    }
    else
    {
        // server is processing another user's birth
        if (m_mapPlayers.empty())
        {
            // cannot happen because at least the user of the server should be in the map!
            // this should happen only if we are dedicated server but currently only listen-server is supported!
            getConsole().EOLn("PRooFPSddPGE::%s(): non-server user (connHandleServerSide: %u) connected but map of players is still empty, CANNOT HAPPEN!",
                __func__, connHandleServerSide);
            assert(false);
            return;
        }

        char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength];
        genUniqueUserName(szNewUserName);
        szConnectedUserName = szNewUserName;
        getConsole().OLn("PRooFPSddPGE::%s(): new remote user %s (connHandleServerSide: %u) connected (from %s) and I'm server",
            __func__, szConnectedUserName, connHandleServerSide, msg.szIpAddress);

        pge_network::PgePacket newPktSetup;
        proofps_dd::MsgUserSetup::initPkt(newPktSetup, connHandleServerSide, false, szConnectedUserName, msg.szIpAddress);

        // server injects this msg to self so resources for player will be allocated
        getNetwork().getServer().getPacketQueue().push_back(newPktSetup);
        getNetwork().getServer().getPacketQueue().push_back(newPktUserUpdate);

        // inform all other clients about this new user
        getNetwork().getServer().SendPacketToAllClients(newPktSetup, connHandleServerSide);
        getNetwork().getServer().SendPacketToAllClients(newPktUserUpdate, connHandleServerSide);

        // now we send this msg to the client with this bool flag set so client will know it is their connect
        proofps_dd::MsgUserSetup& msgUserSetup = reinterpret_cast<proofps_dd::MsgUserSetup&>(newPktSetup.msg.app.cData);
        msgUserSetup.m_bCurrentClient = true;
        getNetwork().getServer().SendPacketToClient(connHandleServerSide, newPktSetup);
        getNetwork().getServer().getPacketQueue().push_back(newPktUserUpdate);

        // we also send as many MsgUserSetup pkts to the client as the number of already connected players,
        // otherwise client won't know about them, so this way the client will detect them as newly connected users;
        // we also send MsgUserUpdate about each player so new client will immediately have their positions updated.
        for (const auto& it : m_mapPlayers)
        {
            proofps_dd::MsgUserSetup::initPkt(
                newPktSetup,
                it.second.m_connHandleServerSide,
                false,
                it.first, it.second.m_sIpAddress);
            getNetwork().getServer().SendPacketToClient(connHandleServerSide, newPktSetup);

            proofps_dd::MsgUserUpdate::initPkt(
                newPktUserUpdate,
                it.second.m_connHandleServerSide,
                it.second.m_legacyPlayer.getAttachedObject()->getPosVec().getX(),
                it.second.m_legacyPlayer.getAttachedObject()->getPosVec().getY(),
                it.second.m_legacyPlayer.getAttachedObject()->getPosVec().getZ(),
                it.second.m_legacyPlayer.getAttachedObject()->getAngleVec().getY());
            getNetwork().getServer().SendPacketToClient(connHandleServerSide, newPktUserUpdate);
        }
    }
}

void PRooFPSddPGE::HandleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnected&)
{
    auto it = m_mapPlayers.begin();
    while (it != m_mapPlayers.end())
    {
        if (it->second.m_connHandleServerSide == connHandleServerSide)
        {
            break;
        }
        it++;
    }

    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return;
    }

    const std::string& sClientUserName = it->first;

    if (getNetwork().isServer())
    {
        getConsole().OLn("PRooFPSddPGE::%s(): user %s disconnected and I'm server", __func__, sClientUserName.c_str());
    }
    else
    {
        getConsole().OLn("PRooFPSddPGE::%s(): user %s disconnected and I'm client", __func__, sClientUserName.c_str());
    }

    if (it->second.m_legacyPlayer.getAttachedObject())
    {
        delete it->second.m_legacyPlayer.getAttachedObject();  // yes, dtor will remove this from its Object3DManager too!
    }

    // do not delete wpn object associated to player, because it would invalidate other players' ptr to their weapon in the vector
    // instead all weapons will be deleted anyway at game shutdown

    m_mapPlayers.erase(it);

    getNetwork().WriteList();
    WritePlayerList();
}

void PRooFPSddPGE::HandleUserCmdMove(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserCmdMove& pktUserCmdMove)
{
    if (!getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): client received MsgUserCmdMove, CANNOT HAPPEN!", __func__);
        assert(false);
        return;
    }

    auto it = m_mapPlayers.begin();
    while (it != m_mapPlayers.end())
    {
        if (it->second.m_connHandleServerSide == connHandleServerSide)
        {
            break;
        }
        it++;
    }

    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return;
    }

    const std::string& sClientUserName = it->first;

    if ((pktUserCmdMove.m_strafe == proofps_dd::Strafe::NONE) &&
        (!pktUserCmdMove.m_bJumpAction) && (!pktUserCmdMove.m_bSendSwitchToRunning) &&
        (pktUserCmdMove.m_fPlayerAngleY == -1.f) && (!pktUserCmdMove.m_bShouldSend))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): user %s sent invalid cmdMove!", __func__, sClientUserName.c_str());
        return;
    }

    auto& legacyPlayer = it->second.m_legacyPlayer;

    if (pktUserCmdMove.m_bSendSwitchToRunning)
    {
        if (legacyPlayer.isRunning())
        {
            legacyPlayer.SetRun(false);
        }
        else
        {
            legacyPlayer.SetRun(true);
        }
    }

    float fSpeed;
    if (legacyPlayer.isRunning())
    {
        fSpeed = GAME_PLAYER_SPEED2 / 60.0f;
    }
    else
    {
        fSpeed = GAME_PLAYER_SPEED1 / 60.0f;
    }

    if ( pktUserCmdMove.m_strafe == proofps_dd::Strafe::LEFT )
    {
        if (!legacyPlayer.isJumping() && !legacyPlayer.isFalling() && legacyPlayer.jumpAllowed())
        {
            //getConsole().OLn("PRooFPSddPGE::%s(): user %s sent valid cmdMove", __func__, sClientUserName.c_str());
            legacyPlayer.getPos1().SetX(legacyPlayer.getPos1().getX() - fSpeed);
        }
    }
    if ( pktUserCmdMove.m_strafe == proofps_dd::Strafe::RIGHT )
    {
        if (!legacyPlayer.isJumping() && !legacyPlayer.isFalling() && legacyPlayer.jumpAllowed())
        {
            legacyPlayer.getPos1().SetX(legacyPlayer.getPos1().getX() + fSpeed);
        }
    }

    if (pktUserCmdMove.m_bJumpAction)
    {
        if (!legacyPlayer.isJumping() &&
            legacyPlayer.jumpAllowed() &&
            !legacyPlayer.isFalling())
        {
            legacyPlayer.Jump();
        }
    }

    if (pktUserCmdMove.m_fPlayerAngleY != -1.f)
    {
        legacyPlayer.getAngleY() = pktUserCmdMove.m_fPlayerAngleY;
    }

    //pge_network::PgePacket pktOut;
    //proofps_dd::MsgUserUpdate::initPkt(
    //    pktOut,
    //    connHandleServerSide,
    //    legacyPlayer.getPos1().getX(),
    //    legacyPlayer.getPos1().getY(),
    //    legacyPlayer.getPos1().getZ());
    //getNetwork().getServer().SendPacketToAllClients(pktOut);
    //// this msgUserUpdate should be also sent to server as self
    //// maybe the SendPacketToAllClients() should be enhanced to contain packet injection for server's packet queue!
    //getNetwork().getServer().getPacketQueue().push_back(pktOut);
}

void PRooFPSddPGE::HandleUserUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserUpdate& msg)
{
    auto it = m_mapPlayers.begin();
    while (it != m_mapPlayers.end())
    {
        if (it->second.m_connHandleServerSide == connHandleServerSide)
        {
            break;
        }
        it++;
    }

    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return;
    }

    //getConsole().OLn("PRooFPSddPGE::%s(): user %s received MsgUserUpdate: %f", __func__, it->first.c_str(), msg.m_pos.x);

    //it->second.m_legacyPlayer.getPos1().Set(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);

    if (it->second.m_legacyPlayer.isExpectingStartPos())
    {
        it->second.m_legacyPlayer.SetExpectingStartPos(false);
        it->second.m_legacyPlayer.getPos1().Set(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);
        it->second.m_legacyPlayer.getOPos1().Set(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);
    }

    it->second.m_legacyPlayer.getAttachedObject()->getPosVec().Set(
        msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);

    if (msg.m_fPlayerAngleY != -1.f)
    {
        it->second.m_legacyPlayer.getAngleY() = msg.m_fPlayerAngleY;
        it->second.m_legacyPlayer.getAttachedObject()->getAngleVec().SetY(msg.m_fPlayerAngleY);
    }
}
