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

#include <algorithm>
#include <cassert>
#include <filesystem>  // requires cpp17
#include <functional>

#include "../../../PGE/PGE/PRRE/include/external/Render/PRRERendererHWfixedPipe.h"  // for rendering hints
#include "../../../PGE/PGE/PRRE/include/external/PRREuiManager.h"
#include "../../../PGE/PGE/PRRE/include/external/Display/PRREWindow.h"
#include "../../../PGE/PGE/PRRE/include/external/PRRECamera.h"
#include "../../../CConsole/CConsole/src/CConsole.h"

static const std::string GAME_NAME    = "PRooFPS-dd";
static const std::string GAME_VERSION = "0.1.0.0 Private Beta";


// ############################### PUBLIC ################################


CPlayer::CPlayer() :
  m_nHealth(100),
  m_nOldHealth(100),
  m_fPlayerAngleY(0.f),
  m_fOldPlayerAngleY(0.f),
  m_pObj(PGENULL),
  m_pWpn(NULL),
  pGFX(NULL),
  m_fGravity(0.f),
  m_bJumping(false),
  b_mCanFall(true),
  m_bRunning(false),
  m_bAllowJump(false),
  m_bExpectingStartPos(true),
  m_bRespawn(false),
  m_nFrags(0),
  m_nOldFrags(0),
  m_nDeaths(0),
  m_nOldDeaths(0)
{
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
    m_vOldWpnAngle = m_vWpnAngle;
}

void CPlayer::SetHealth(int value) {
    m_nHealth = min(value, 100);
}

void CPlayer::UpdateOldHealth()
{
    m_nOldHealth = m_nHealth;
}

int CPlayer::getOldHealth() const
{
    return m_nOldHealth;
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

const Weapon* CPlayer::getWeapon() const
{
    return m_pWpn;
}

void CPlayer::SetWeapon(Weapon* wpn)
{
    m_pWpn = wpn;
}

std::vector<Weapon*>& CPlayer::getWeapons()
{
    return m_weapons;
}

const std::vector<Weapon*>& CPlayer::getWeapons() const
{
    return m_weapons;
}

Weapon* CPlayer::getWeaponByName(const std::string& wpnName)
{
    for (const auto pWpn : m_weapons)
    {
        if (pWpn)
        {
            if (pWpn->getFilename() == wpnName)
            {
                return pWpn;
            }
        }
    }

    return nullptr;
}

PRREVector& CPlayer::getOldWeaponAngle()
{
    return m_vOldWpnAngle;
}

PRREVector& CPlayer::getWeaponAngle()
{
    return m_vWpnAngle;
}

std::chrono::time_point<std::chrono::steady_clock>& CPlayer::getTimeDied()
{
    return m_timeDied;
}

bool& CPlayer::getRespawnFlag()
{
    return m_bRespawn;
}

int& CPlayer::getFrags()
{
    return m_nFrags;
}

const int& CPlayer::getFrags() const
{
    return m_nFrags;
}

int& CPlayer::getOldFrags()
{
    return m_nOldFrags;
}

int& CPlayer::getDeaths()
{
    return m_nDeaths;
}

const int& CPlayer::getDeaths() const
{
    return m_nDeaths;
}

int& CPlayer::getOldDeaths()
{
    return m_nOldDeaths;
}

void CPlayer::UpdateFragsDeaths()
{
    m_nOldFrags = m_nFrags;
    m_nOldDeaths = m_nDeaths;
}

bool CPlayer::canTakeItem(const MapItem& item) const
{
    switch (item.getType())
    {
    case MapItemType::ITEM_WPN_PISTOL:
        return true;
    case MapItemType::ITEM_WPN_MACHINEGUN:
        return true;
    case MapItemType::ITEM_HEALTH:
        return (getHealth() < 100);
    default:
        ;
    }
    return false;
}

void CPlayer::TakeItem(MapItem& item, const std::map<MapItemType, std::string>& mapItemTypeToWeaponName)
{
    // invoked only on server
    switch (item.getType())
    {
    case MapItemType::ITEM_WPN_PISTOL:
    case MapItemType::ITEM_WPN_MACHINEGUN:
    {
        const auto it = mapItemTypeToWeaponName.find(item.getType());
        if (it == mapItemTypeToWeaponName.end())
        {
            CConsole::getConsoleInstance(PRooFPSddPGE::getLoggerModuleName()).EOLn(
                "CPlayer::%s(): failed to find weapon by item type %d!", __func__, item.getType());
            return;
        }
        const std::string& sWeaponBecomingAvailable = it->second;
        Weapon* const pWpnBecomingAvailable = getWeaponByName(sWeaponBecomingAvailable);
        if (!pWpnBecomingAvailable)
        {
            CConsole::getConsoleInstance(PRooFPSddPGE::getLoggerModuleName()).EOLn(
                "CPlayer::%s(): failed to find weapon by name %s for item type %d!", __func__, sWeaponBecomingAvailable.c_str(), item.getType());
            return;
        }

        item.Take();
        if (pWpnBecomingAvailable->isAvailable())
        {
            // just increase mag/unmag count
            // TODO
            CConsole::getConsoleInstance(PRooFPSddPGE::getLoggerModuleName()).OLn(
                "CPlayer::%s(): weapon %s pickup, already available, just inc stuff", __func__, sWeaponBecomingAvailable.c_str());
        }
        else
        {
            // becoming available with default mag/unmag count
            pWpnBecomingAvailable->SetAvailable(true);
            CConsole::getConsoleInstance(PRooFPSddPGE::getLoggerModuleName()).OLn(
                "CPlayer::%s(): weapon %s pickup, becomes available", __func__, sWeaponBecomingAvailable.c_str());
        }
        break;
    }
    case MapItemType::ITEM_HEALTH:
        item.Take();
        SetHealth(getHealth() + MapItem::ITEM_HEALTH_HP_INC);
        break;
    default:
        CConsole::getConsoleInstance(PRooFPSddPGE::getLoggerModuleName()).EOLn(
            "CPlayer::%s(): unknown item type %d!", __func__, item.getType());
        ;
    }
}


PRooFPSddPGE* PRooFPSddPGE::createAndGetPRooFPSddPGEinstance()
{
    static PRooFPSddPGE pgeInstance((GAME_NAME + " " + GAME_VERSION).c_str());
    return &pgeInstance;
}

CConsole& PRooFPSddPGE::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}


const char* PRooFPSddPGE::getLoggerModuleName()
{
    return "PRooFPSddPGE";
}


// ############################## PROTECTED ##############################


/**
    This is the only usable ctor, this is used by the static createAndGet().
*/
PRooFPSddPGE::PRooFPSddPGE(const char* gameTitle) :
    PGE(gameTitle),
    m_maps(getPRRE()),
    m_fps(0),
    m_fps_counter(0),
    m_fps_lastmeasure(0),
    m_fps_ms(0),
    m_pObjXHair(NULL),
    m_bSpaceReleased(true),
    m_bBackSpaceReleased(true),
    m_bCtrlReleased(true),
    m_bShiftReleased(true),
    m_enterreleased(true),
    m_bTeleportReleased(true),
    m_bWon(false),
    m_fCameraMinY(0.0f),
    m_bShowGuiDemo(false)
{
    
}

PRooFPSddPGE::~PRooFPSddPGE()
{

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

    // basically I turn everything off, I could simply set 0, but still want to set bits in a clear way;
    // I need to use legacy rendering path, because if I use occlusion culling methods, it will be slow
    // for ~1000 cubes, since PRRE still doesn't implement hierarchical occlusion culling ...
    // And a normal map like Warhouse already contains ~1000 cubes.
    getPRRE().getRenderer()->SetRenderHints(
        BITF_PREP(PRRE_RH_RP_LEGACY_PR00FPS, PRRE_RH_RENDER_PATH_BITS, 3) |
        BITF_PREP(PRRE_RH_OQ_METHOD_ASYNC, PRRE_RH_OQ_METHOD_BITS, 2) |
        PRRE_RH_OQ_DRAW_BOUNDING_BOXES_OFF |
        PRRE_RH_OQ_DRAW_IF_QUERY_PENDING_OFF |
        PRRE_RH_ORDERING_BY_DISTANCE_OFF);
    
    getPRRE().getScreen().SetVSyncEnabled(true);
    setGameRunningFrequency(GAME_MAXFPS);

    getPRRE().getCamera().SetNearPlane(0.1f);
    getPRRE().getCamera().SetFarPlane(100.0f);
    getPRRE().getCamera().getPosVec().Set( 0, 0, GAME_CAM_Z );
    getPRRE().getCamera().getTargetVec().Set( 0, 0, -GAME_BLOCK_SIZE_Z );

    m_gameMode = proofps_dd::GameMode::createGameMode(proofps_dd::GameModeType::DeathMatch);
    assert(m_gameMode);

    m_deathMatchMode = dynamic_cast<proofps_dd::DeathMatchMode*>(m_gameMode);
    assert(m_deathMatchMode);

    const bool bMapsInited = m_maps.initialize();
    assert(bMapsInited);

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

        getNetwork().getServer().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgBulletUpdate::id));
        getNetwork().getServer().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgMapItemUpdate::id));

        getPRRE().getUImanager().text("Starting Server ...", 200, getPRRE().getWindow().getClientHeight() / 2);
        getPRRE().getRenderer()->RenderScene();

        if (!getNetwork().getServer().startListening())
        {
            PGE::showErrorDialog("Server has FAILED to start listening!");
            assert(false);
        }

        std::ifstream f;
        f.open("server.txt", std::ifstream::in);
        if (!f.good())
        {
            getConsole().OLn("No server.txt found!");
            m_sServerMapFilenameToLoad = "map_warena.txt";
        }
        else
        {
            getConsole().OLn("Found server.txt");
            const std::streamsize nBuffSize = 1024;
            char cLine[nBuffSize];
            int i = 0;
            while (!f.eof())
            {
                f.getline(cLine, nBuffSize);
                // TODO: we should finally have a strClr() version for std::string or FINALLY UPGRADE TO NEWER CPP THAT MAYBE HAS THIS FUNCTIONALITY!!!
                PFL::strClrLeads(cLine);
                const std::string sTrimmedLine(cLine);
                switch (i)
                {
                case 0:
                    m_sServerMapFilenameToLoad = sTrimmedLine;
                    getConsole().OLn("Server map override: %s", m_sServerMapFilenameToLoad.c_str());
                    break;
                case 1:
                    if (sTrimmedLine == "vsync_off")
                    {
                        getConsole().OLn("VSync override: off");
                        getPRRE().getScreen().SetVSyncEnabled(false);
                    }
                    break;
                default:
                    getConsole().OLn("Logging override: %s", sTrimmedLine.c_str());
                    getConsole().SetLoggingState(sTrimmedLine.c_str(), true);
                } // switch
                i++;
            };
            f.close();
        }
    }
    else
    {
        getNetwork().getClient().getBlackListedAppMessages().insert(static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserCmdMove::id));

        std::string sIp = "127.0.0.1";
        std::ifstream f;
        f.open("gyorsan.txt", std::ifstream::in);
        if (!f.good())
        {
            getConsole().OLn("No gyorsan.txt found!");
        }
        else
        {
            getConsole().OLn("Found gyorsan.txt");
            const std::streamsize nBuffSize = 1024;
            char cLine[nBuffSize];
            int i = 0;
            while (!f.eof())
            {
                f.getline(cLine, nBuffSize);
                // TODO: we should finally have a strClr() version for std::string or FINALLY UPGRADE TO NEWER CPP THAT MAYBE HAS THIS FUNCTIONALITY!!!
                PFL::strClrLeads(cLine);
                const std::string sTrimmedLine(cLine);
                switch (i)
                {
                case 0:
                    sIp = sTrimmedLine;
                    getConsole().OLn("IP override: %s", sIp.c_str());
                    break;
                case 1:
                    if (sTrimmedLine == "vsync_off")
                    {
                        getConsole().OLn("VSync override: off");
                        getPRRE().getScreen().SetVSyncEnabled(false);
                    }
                    break;
                default:
                    getConsole().OLn("Logging override: %s", sTrimmedLine.c_str());
                    getConsole().SetLoggingState(sTrimmedLine.c_str(), true);
                } // switch
                i++;
            };
            f.close();
        }

        getPRRE().getUImanager().text("Connecting to " + sIp + " ...", 200, getPRRE().getWindow().getClientHeight() / 2);
        getPRRE().getRenderer()->RenderScene();

        if (!getNetwork().getClient().connectToServer(sIp))
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

    m_deathMatchMode->SetFragLimit(3);
    //m_deathMatchMode->SetTimeLimitSecs(5);
    m_gameMode->Reset();
    
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


// ############################### PRIVATE ###############################


// The game engine's Weapon system and the game's Map system are 2 independent subsystems.
// This map provides the logical connection between pickupable MapItems and actual weapons.
// So when player picks up a specific MapItem, we know which weapon should become available for the player.
// I'm not planning to move Map stuff to the game engine because this kind of Map is very game-specific.
const std::map<MapItemType, std::string> PRooFPSddPGE::m_mapItemTypeToWeaponName = {
    {MapItemType::ITEM_WPN_PISTOL, "pistol"},
    {MapItemType::ITEM_WPN_MACHINEGUN, "machinegun"}
};

void PRooFPSddPGE::KeyBoard(int /*fps*/, bool& won, pge_network::PgePacket& pkt)
{
    const PGEInputKeyboard& keybd = getInput().getKeyboard();
  
    if ( keybd.isKeyPressed(VK_ESCAPE) )
    {
        getPRRE().getWindow().Close();
    }

    if (m_gameMode->checkWinningConditions())
    {
        return;
    }

    if (keybd.isKeyPressed(VK_TAB))
    {
        ShowFragTable(false);
    }

    if (keybd.isKeyPressed(VK_BACK))
    {
        if (m_bBackSpaceReleased)
        {
            m_bShowGuiDemo = !m_bShowGuiDemo;
            getPRRE().ShowGuiDemo(m_bShowGuiDemo);
            getPRRE().getWindow().SetCursorVisible(m_bShowGuiDemo);
            m_bBackSpaceReleased = false;
        }
    }
    else
    {
        m_bBackSpaceReleased = true;
    }

    if (m_bShowGuiDemo)
    {
        return;
    }

    if (m_mapPlayers[m_sUserName].m_legacyPlayer.getHealth() == 0)
    {
        return;
    }
      
    if ( !won )
    {
        if (getNetwork().isServer() )
        {
            if (keybd.isKeyPressed((unsigned char)VkKeyScan('t')))
            {
                if (m_bTeleportReleased)
                {
                    // for testing purpose only, we can teleport server player to random spawn point
                    m_mapPlayers[m_sUserName].m_legacyPlayer.getPos1() = m_maps.getRandomSpawnpoint();
                    m_mapPlayers[m_sUserName].m_legacyPlayer.getRespawnFlag() = true;
                    // log some stats too
                    getConsole().SetLoggingState("PRRERendererHWfixedPipe", true);
                    getPRRE().getRenderer()->ResetStatistics();
                    getConsole().SetLoggingState("PRRERendererHWfixedPipe", false);

                    m_bTeleportReleased = false;
                }
            }
            else
            {
                m_bTeleportReleased = true;
            }
        }

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


bool PRooFPSddPGE::Mouse(int /*fps*/, bool& /*won*/, pge_network::PgePacket& pkt)
{
    if (m_gameMode->checkWinningConditions())
    {
        return false;
    }

    if (m_bShowGuiDemo)
    {
        return false;
    }

    PGEInputMouse& mouse = getInput().getMouse();

    if (mouse.isButtonPressed(PGEInputMouse::MouseButton::MBTN_LEFT))
    {
        // sending mouse action is still allowed when player is dead, since server will treat that
        // as respawn request
        proofps_dd::MsgUserCmdMove::setMouse(pkt, true);
    }

    if (m_mapPlayers[m_sUserName].m_legacyPlayer.getHealth() == 0)
    {
        return false;
    }

    const int oldmx = mouse.getCursorPosX();
    const int oldmy = mouse.getCursorPosY();

    mouse.SetCursorPos(
        getPRRE().getWindow().getX() + getPRRE().getWindow().getWidth()/2,
        getPRRE().getWindow().getY() + getPRRE().getWindow().getHeight()/2);

    const int dx = oldmx - mouse.getCursorPosX();
    const int dy = oldmy - mouse.getCursorPosY();

    if ((dx == 0) && (dy == 0))
    {
        return false;
    }
    
    m_pObjXHair->getPosVec().Set(
        m_pObjXHair->getPosVec().getX() + dx,
        m_pObjXHair->getPosVec().getY() - dy,
        0.f);

    return true;
}


bool PRooFPSddPGE::Colliding(const PRREObject3D& a, const PRREObject3D& b)
{
    return Colliding2(
        a.getPosVec().getX(),  a.getPosVec().getY(),  a.getPosVec().getZ(),
        a.getSizeVec().getX(), a.getSizeVec().getY(), a.getSizeVec().getZ(),
        b.getPosVec().getX(),  b.getPosVec().getY(),  b.getPosVec().getZ(),
        b.getSizeVec().getX(), b.getSizeVec().getY(), b.getSizeVec().getZ()
    );
}       

bool PRooFPSddPGE::Colliding2(
    float o1px, float o1py, float o1pz, float o1sx, float o1sy, float o1sz,
    float o2px, float o2py, float o2pz, float o2sx, float o2sy, float o2sz )
{
    return (
        (
            (o1px - o1sx / 2 <= o2px + o2sx / 2)
            &&
            (o1px + o1sx / 2 >= o2px - o2sx / 2)
        )
        &&
        (
            (o1py - o1sy / 2 <= o2py + o2sy / 2)
            &&
            (o1py + o1sy / 2 >= o2py - o2sy / 2)
        )
        &&
        (
            (o1pz - o1sz / 2 <= o2pz + o2sz / 2)
            &&
            (o1pz + o1sz / 2 >= o2pz - o2sz / 2)
        )
    );            
}

bool PRooFPSddPGE::Colliding3(
    const PRREVector& vecPosMin, const PRREVector& vecPosMax,
    const PRREVector& vecObjPos, const PRREVector& vecObjSize)
{
    const PRREVector vecSize(
        vecPosMax.getX() - vecPosMin.getX(),
        vecPosMax.getY() - vecPosMin.getY(),
        vecPosMax.getZ() - vecPosMin.getZ()
    );
    const PRREVector vecPos(
        vecPosMin.getX() + vecSize.getX() / 2.f,
        vecPosMin.getY() + vecSize.getY() / 2.f,
        vecPosMin.getZ() + vecSize.getZ() / 2.f
    );

    return Colliding2(
        vecPos.getX(),  vecPos.getY(),  vecPos.getZ(),
        vecSize.getX(), vecSize.getY(), vecSize.getZ(),
        vecObjPos.getX(),  vecObjPos.getY(),  vecObjPos.getZ(),
        vecObjSize.getX(), vecObjSize.getY(), vecObjSize.getZ()
    );
}

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
        
        if ( (legacyPlayer.getHealth() > 0) && (legacyPlayer.getPos1().getY() < m_maps.getBlockPosMin().getY() - 5.0f))
        {
            getConsole().OLn("PRooFPSddPGE::%s(): Player %s out of map low bound!", __func__, player.first.c_str());
            legacyPlayer.SetHealth(0);
            HandlePlayerDied(player.first == m_sUserName, player.second.m_legacyPlayer);
        }
    }
}

void PRooFPSddPGE::ShowFragTable(bool bWin) const
{
    const int nXPosPlayerName = 20;
    const int nXPosFrags = 200;
    const int nXPosDeaths = 250;
    int nYPosStart = getPRRE().getWindow().getClientHeight() - 20;
    if (bWin)
    {
        getPRRE().getUImanager().text("Game Ended! Waiting for restart ...", nXPosPlayerName, nYPosStart);
        nYPosStart -= 2 * getPRRE().getUImanager().getDefaultFontSize();
    }

    getPRRE().getUImanager().text("Player Name", nXPosPlayerName, nYPosStart);
    getPRRE().getUImanager().text("Frags", nXPosFrags, nYPosStart);
    getPRRE().getUImanager().text("Deaths", nXPosDeaths, nYPosStart);
    getPRRE().getUImanager().text("========================================================",
        20, nYPosStart - getPRRE().getUImanager().getDefaultFontSize());

    int i = 0;
    for (const auto& player : m_deathMatchMode->getPlayerData())
    {
        i++;
        const int nThisRowY = nYPosStart - (i + 1) * getPRRE().getUImanager().getDefaultFontSize();
        getPRRE().getUImanager().text(player.m_sName, nXPosPlayerName, nThisRowY);
        getPRRE().getUImanager().text(std::to_string(player.m_nFrags), nXPosFrags, nThisRowY);
        getPRRE().getUImanager().text(std::to_string(player.m_nDeaths), nXPosDeaths, nThisRowY);
    }
}

void PRooFPSddPGE::UpdateBullets()
{
    // on the long run this function needs to be part of the game engine itself, however currently game engine doesn't handle collisions,
    // so once we introduce the collisions to the game engine, it will be an easy move of this function as well there
    pge_network::PgePacket newPktBulletUpdate;
    std::list<Bullet>& bullets = getWeaponManager().getBullets();
    auto it = bullets.begin();
    while (it != bullets.end())
    {
        auto& bullet = *it;

        bool bDeleteBullet = false;
        if (m_gameMode->checkWinningConditions())
        {
            bDeleteBullet = true;
        }
        else
        {
            bullet.Update();
        }

        proofps_dd::MsgBulletUpdate::initPkt(
            newPktBulletUpdate,
            0,
            bullet.getId(),
            bullet.getObject3D().getPosVec().getX(),
            bullet.getObject3D().getPosVec().getY(),
            bullet.getObject3D().getPosVec().getZ(),
            bullet.getObject3D().getAngleVec().getX(),
            bullet.getObject3D().getAngleVec().getY(),
            bullet.getObject3D().getAngleVec().getZ(),
            bullet.getObject3D().getSizeVec().getX(),
            bullet.getObject3D().getSizeVec().getY(),
            bullet.getObject3D().getSizeVec().getZ());

        if (!bDeleteBullet)
        {
            // check if bullet is hitting a player
            for (auto& player : m_mapPlayers)
            {
                if (bullet.getOwner() == player.second.m_connHandleServerSide)
                {
                    // bullet cannot hit the owner, at least for now ...
                    // in the future, when bullets start in proper position, we won't need this check ...
                    // this check will be bad anyway in future when we will have the guided rockets that actually can hit the owner if guided in suicide way!
                    continue;
                }

                if ((player.second.m_legacyPlayer.getHealth() > 0) &&
                    Colliding(*(player.second.m_legacyPlayer.getAttachedObject()), bullet.getObject3D()))
                {
                    bDeleteBullet = true;
                    player.second.m_legacyPlayer.DoDamage(bullet.getDamageHp());
                    if (player.second.m_legacyPlayer.getHealth() == 0)
                    {
                        const auto itKiller = getPlayerMapItByConnectionHandle(bullet.getOwner());
                        if (itKiller == m_mapPlayers.end())
                        {
                            getConsole().OLn("PRooFPSddPGE::%s(): Player %s has been killed by a player already left!",
                                __func__, player.first.c_str());
                        }
                        else
                        {
                            itKiller->second.m_legacyPlayer.getFrags()++;
                            getConsole().OLn("PRooFPSddPGE::%s(): Player %s has been killed by %s, who now has %d frags!",
                                __func__, player.first.c_str(), itKiller->first.c_str(), itKiller->second.m_legacyPlayer.getFrags());
                        }
                        // server handles death here, clients will handle it when they receive MsgUserUpdate
                        HandlePlayerDied(player.first == m_sUserName, player.second.m_legacyPlayer);
                    }
                    break; // we can stop since 1 bullet can touch 1 player only at a time
                }
            }

            if (!bDeleteBullet)
            {
                // check if bullet is out of map bounds
                // we relax map bounds a bit to let the bullets leave map area a bit more before destroying them ...
                const PRREVector vRelaxedMapMinBounds(
                    m_maps.getBlocksVertexPosMin().getX() - GAME_BLOCK_SIZE_X * 4,
                    m_maps.getBlocksVertexPosMin().getY() - GAME_BLOCK_SIZE_Y,
                    m_maps.getBlocksVertexPosMin().getZ() - GAME_BLOCK_SIZE_Z); // ah why dont we have vector-scalar subtract operator defined ...
                const PRREVector vRelaxedMapMaxBounds(
                    m_maps.getBlocksVertexPosMax().getX() + GAME_BLOCK_SIZE_X * 4,
                    m_maps.getBlocksVertexPosMax().getY() + GAME_BLOCK_SIZE_Y,
                    m_maps.getBlocksVertexPosMax().getZ() + GAME_BLOCK_SIZE_Z);
                if (!Colliding3(vRelaxedMapMinBounds, vRelaxedMapMaxBounds, bullet.getObject3D().getPosVec(), bullet.getObject3D().getSizeVec()))
                {
                    bDeleteBullet = true;
                }
            }

            if (!bDeleteBullet)
            {
                // check if bullet is hitting a map element
                // 
                // TODO: this part slows the game down in debug mode with a few bullets.
                // Originally with only 6 bullets, with VSync FPS went down from 60 to 45 on main dev machine, so
                // I recommend using spatial hierarchy to effectively reduce the number of collision checks ...
                // For now with the small bullet direction optimization in the loop I managed to keep FPS around 45-50
                // with 10-15 bullets.
                for (int i = 0; i < m_maps.getBlockCount(); i++)
                {
                    PRREObject3D* const obj = m_maps.getBlocks()[i];
                    if (obj->isColliding_TO_BE_REMOVED())
                    {
                        const bool bGoingLeft = bullet.getObject3D().getAngleVec().getY() == 0.f; // otherwise it would be 180.f
                        if ((bGoingLeft && (obj->getPosVec().getX() > bullet.getObject3D().getPosVec().getX())) ||
                            (!bGoingLeft && (obj->getPosVec().getX() < bullet.getObject3D().getPosVec().getX())))
                        {
                            // optimization: rule out those blocks which are not in bullet's direction
                            continue;
                        }
                        if (Colliding(*obj, bullet.getObject3D()))
                        {
                            bDeleteBullet = true;
                            break; // we can stop since 1 bullet can touch only 1 map element
                        }
                    }
                }
            }
        }

        if ( bDeleteBullet )
        {
            it = bullets.erase(it); // delete it right now, otherwise later we would send further updates to clients about this bullet
            proofps_dd::MsgBulletUpdate::getDelete(newPktBulletUpdate) = true; // clients will also delete this bullet on their side
        }
        else
        {
            // bullet didn't touch anything, go to next
            it++;
        }
        
        // 'it' is referring to next bullet, don't use it from here!
        for (const auto& sendToThisPlayer : m_mapPlayers)
        {
            if (sendToThisPlayer.second.m_connHandleServerSide != 0)
            {   // since bullet.Update() updates the bullet position already, server doesn't send this to itself
                getNetwork().getServer().SendPacketToClient(sendToThisPlayer.second.m_connHandleServerSide, newPktBulletUpdate);
            }
        }
    }

    if (m_gameMode->checkWinningConditions() && (Bullet::getGlobalBulletId() > 0))
    {
        Bullet::ResetGlobalBulletId();
    }
}

void PRooFPSddPGE::UpdateWeapons()
{
    for (auto& player : m_mapPlayers)
    {
        Weapon* const wpn = player.second.m_legacyPlayer.getWeapon();
        if (!wpn)
        {
            continue;
        }
        wpn->Update();
    }
}

void PRooFPSddPGE::HandlePlayerDied(bool bMe, CPlayer& player)
{
    player.getAttachedObject()->Hide();
    player.getWeapon()->getObject3D().Hide();
    player.getTimeDied() = std::chrono::steady_clock::now();

    if (bMe)
    {
        getConsole().OLn("PRooFPSddPGE::%s(): I died!", __func__);
        m_pObjXHair->Hide();
        getPRRE().getUImanager().addText("Waiting to respawn ...", 200, getPRRE().getWindow().getClientHeight() / 2);
    }
    else
    {
        getConsole().OLn("PRooFPSddPGE::%s(): other player died!", __func__);
    }

    if (getNetwork().isServer())
    {
        player.getDeaths()++;
        getConsole().OLn("PRooFPSddPGE::%s(): new death count: %d!", __func__, player.getDeaths());
    }
}

void PRooFPSddPGE::HandlePlayerRespawned(bool bMe, CPlayer& player)
{
    player.getAttachedObject()->Show();
    player.getWeapon()->getObject3D().Show();
    player.getWeapon()->Reset();
    if (bMe)
    {
        m_pObjXHair->Show();
        // well, this won't work if clientHeight is being changed in the meantime, but anyway this supposed to be a temporal feature ...
        getPRRE().getUImanager().RemoveText(
            "Waiting to respawn ...", 200, getPRRE().getWindow().getClientHeight() / 2, getPRRE().getUImanager().getDefaultFontSize());
    }
}

void PRooFPSddPGE::UpdateRespawnTimers()
{
    if (m_gameMode->checkWinningConditions())
    {
        return;
    }

    for (auto& player : m_mapPlayers)
    {
        if (player.second.m_legacyPlayer.getHealth() > 0)
        {
            continue;
        }

        const std::chrono::duration<float> timeDiff = std::chrono::steady_clock::now() - player.second.m_legacyPlayer.getTimeDied();
        if (timeDiff.count() > 3.f)
        {
            // to respawn, we just need to set these values, because SendUserUpdates() will automatically send out changes to everyone
            player.second.m_legacyPlayer.getPos1() = m_maps.getRandomSpawnpoint();
            player.second.m_legacyPlayer.SetHealth(100);
            player.second.m_legacyPlayer.getRespawnFlag() = true;
        }
    }
}

void PRooFPSddPGE::PickupAndRespawnItems()
{
    pge_network::PgePacket newPktMapItemUpdate;
    // TODO: MSGWPNUPD add another packet definition here when we have pkt for weapon update that we cen send to client

    for (auto& itemPair : m_maps.getItems())
    {
        if (!itemPair.second)
        {
            continue;
        }

        MapItem& mapItem = *(itemPair.second);
        bool bSendItemUpdate = false;

        if (mapItem.isTaken())
        {
            const auto nSecsSinceTake = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - mapItem.getTimeTaken()).count();
            if (nSecsSinceTake < MapItem::getItemRespawnTimeSecs(mapItem))
            {
                continue;
            }

            mapItem.UnTake();
            bSendItemUpdate = true;
        }
        else
        {
            for (auto& player : m_mapPlayers)
            {
                auto& legacyPlayer = player.second.m_legacyPlayer;
                if (legacyPlayer.getHealth() <= 0)
                {
                    continue;
                }

                const PRREObject3D* const plobj = legacyPlayer.getAttachedObject();

                // TODO: from performance perspective, maybe it would be better to check canTakeItem() first since that might be faster
                // decision than collision check ...
                if (Colliding(*plobj, mapItem.getObject3D()))
                {
                    if (legacyPlayer.canTakeItem(mapItem))
                    {
                        // TODO: MSGWPNUPD we can pass that msg to TakeItem because it knows how to update it
                        legacyPlayer.TakeItem(mapItem, m_mapItemTypeToWeaponName);  // this also invokes mapItem.Take()
                        bSendItemUpdate = true;
                        break; // a player can collide with only one item at a time since there are no overlapping items
                    }
                } // colliding
            } // for player
        }

        if (bSendItemUpdate)
        {
            // TODO: MSGWPNUPD and here we can check if that pkt should be sent, and send that to that specific client only
            proofps_dd::MsgMapItemUpdate::initPkt(
                newPktMapItemUpdate,
                0,
                mapItem.getId(),
                mapItem.isTaken());

            for (const auto& sendToThisPlayer : m_mapPlayers)
            {
                if (sendToThisPlayer.second.m_connHandleServerSide != 0)
                { 
                    getNetwork().getServer().SendPacketToClient(sendToThisPlayer.second.m_connHandleServerSide, newPktMapItemUpdate);
                }
            }
        }
    } // for item
}

void PRooFPSddPGE::UpdateGameMode()
{
    const bool bPrevWinningConditions = m_gameMode->checkWinningConditions();

    // not nice, in the future players will be stored in a more general way that could be easily accessed by GameMode
    std::vector<proofps_dd::FragTableRow> players;
    for (const auto& player : m_mapPlayers)
    {
        const proofps_dd::FragTableRow fragTableRow = { player.first, player.second.m_legacyPlayer.getFrags(), player.second.m_legacyPlayer.getDeaths() };
        players.push_back(fragTableRow);
    }
    m_deathMatchMode->UpdatePlayerData(players);

    const bool bNewWinningConditions = m_gameMode->checkWinningConditions();
    if (bNewWinningConditions)
    {
        const auto nSecsSinceWin = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_gameMode->getWinTime()).count();
        if (nSecsSinceWin >= 15)
        {
            if (getNetwork().isServer())
            {
                for (auto& player : m_mapPlayers)
                {
                    // to respawn, we just need to set these values, because SendUserUpdates() will automatically send out changes to everyone
                    player.second.m_legacyPlayer.getPos1() = m_maps.getRandomSpawnpoint();
                    player.second.m_legacyPlayer.SetHealth(100);
                    player.second.m_legacyPlayer.getFrags() = 0;
                    player.second.m_legacyPlayer.getDeaths() = 0;
                    player.second.m_legacyPlayer.getRespawnFlag() = true;
                }

                // respawn all map items
                pge_network::PgePacket newPktMapItemUpdate;
                for (auto& itemPair : m_maps.getItems())
                {
                    if (!itemPair.second)
                    {
                        continue;
                    }

                    MapItem& mapItem = *(itemPair.second);
                    if (!mapItem.isTaken())
                    {
                        continue;
                    }

                    mapItem.UnTake();

                    proofps_dd::MsgMapItemUpdate::initPkt(
                        newPktMapItemUpdate,
                        0,
                        mapItem.getId(),
                        mapItem.isTaken());

                    for (const auto& sendToThisPlayer : m_mapPlayers)
                    {
                        if (sendToThisPlayer.second.m_connHandleServerSide != 0)
                        {
                            getNetwork().getServer().SendPacketToClient(sendToThisPlayer.second.m_connHandleServerSide, newPktMapItemUpdate);
                        }
                    }
                } // end for items
            } // end server
            m_gameMode->Reset(); // now both server and clients execute this on their own, in future only server should do this ...
        }
        else
        {
            ShowFragTable(true);
            if (bPrevWinningConditions != bNewWinningConditions)
            {
                m_pObjXHair->Hide();
                for (auto& player : m_mapPlayers)
                {
                    player.second.m_legacyPlayer.getAttachedObject()->Hide();
                    player.second.m_legacyPlayer.getWeapon()->getObject3D().Hide();
                }
            }
        }
    }
}

void PRooFPSddPGE::SendUserUpdates()
{
    if (!getNetwork().isServer())
    {
        // should not happen, but we log it anyway, if in future we might mess up something during a refactor ...
        getConsole().EOLn("PRooFPSddPGE::%s(): NOT server!", __func__);
        return;
    }

    for (auto& player : m_mapPlayers)
    {
        auto& legacyPlayer = player.second.m_legacyPlayer;

        if ((legacyPlayer.getPos1() != legacyPlayer.getOPos1()) || (legacyPlayer.getOldAngleY() != legacyPlayer.getAngleY())
            || (legacyPlayer.getWeaponAngle() != legacyPlayer.getOldWeaponAngle())
            || (legacyPlayer.getHealth() != legacyPlayer.getOldHealth())
            || (legacyPlayer.getFrags() != legacyPlayer.getOldFrags())
            || (legacyPlayer.getDeaths() != legacyPlayer.getOldDeaths()))
        {
            pge_network::PgePacket newPktUserUpdate;
            proofps_dd::MsgUserUpdate::initPkt(
                newPktUserUpdate,
                player.second.m_connHandleServerSide,
                legacyPlayer.getPos1().getX(),
                legacyPlayer.getPos1().getY(),
                legacyPlayer.getPos1().getZ(),
                legacyPlayer.getAngleY(),
                legacyPlayer.getWeaponAngle().getY(),
                legacyPlayer.getWeaponAngle().getZ(),
                legacyPlayer.getHealth(),
                legacyPlayer.getRespawnFlag(),
                legacyPlayer.getFrags(),
                legacyPlayer.getDeaths());

            // we always reset respawn flag here
            player.second.m_legacyPlayer.getRespawnFlag() = false;

            // Note that health is not needed by server since it already has the updated health, but for convenience
            // we put that into MsgUserUpdate and send anyway like all the other stuff.

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
    for (auto& player : m_mapPlayers)
    {
        auto& legacyPlayer = player.second.m_legacyPlayer;
        if (getNetwork().isServer())
        {
            if (legacyPlayer.getPos1().getY() != legacyPlayer.getOPos1().getY())
            { // elõzõ frame-ben még tudott zuhanni, tehát egyelõre nem ugorhatunk
                legacyPlayer.SetJumpAllowed(false);
            }
            else
            {
                legacyPlayer.SetJumpAllowed(true);
            }
        }
        // having a username means that server accepted the connection and sent us a username, for which we have initialized our player;
        // otherwise m_mapPlayers[m_sUserName] is dangerous as it implicitly creates entry even with empty username ...
        const bool bValidConnection = !m_sUserName.empty();
        if (bValidConnection)
        {
            legacyPlayer.UpdateOldPos();
            legacyPlayer.UpdateOldHealth();
            legacyPlayer.UpdateFragsDeaths();
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

    m_fps_ms = GetTickCount();

    // having a username means that server accepted the connection and sent us a username, for which we have initialized our player;
    // otherwise m_mapPlayers[m_sUserName] is dangerous as it implicitly creates entry even with empty username ...
    const bool bValidConnection = !m_sUserName.empty();

    if (bValidConnection)
    {
        if (getNetwork().isServer())
        {
            if (!m_gameMode->checkWinningConditions())
            {
                Gravity(m_fps);
                Collision(m_bWon);
            }
        }
        else
        {
            getPRRE().getUImanager().text(
                "Ping: " + std::to_string(getNetwork().getClient().getPing(true)) + " ms",
                10, 50);
            getPRRE().getUImanager().text(
                "Quality: local: " + std::to_string(getNetwork().getClient().getQualityLocal(false)) +
                "; remote: " + std::to_string(getNetwork().getClient().getQualityRemote(false)),
                10, 70);
            getPRRE().getUImanager().text(
                "Tx Speed: " + std::to_string(getNetwork().getClient().getTxByteRate(false)) +
                " Bps; Rx Speed: " + std::to_string(getNetwork().getClient().getRxByteRate(false)) + " Bps",
                10, 90);
            getPRRE().getUImanager().text(
                "Internal Queue Time: " + std::to_string(getNetwork().getClient().getInternalQueueTimeUSecs(false)) + " us",
                10, 110);
        }  

        if (window.isActive())
        {
            pge_network::PgePacket pkt;
            proofps_dd::MsgUserCmdMove::initPkt(pkt);

            KeyBoard(m_fps, m_bWon, pkt);
            Mouse(m_fps, m_bWon, pkt);

            CPlayer& legacyPlayer = m_mapPlayers[m_sUserName].m_legacyPlayer;

            legacyPlayer.getAngleY() = (m_pObjXHair->getPosVec().getX() < 0.f) ? 0.f : 180.f;
            legacyPlayer.getAttachedObject()->getAngleVec().SetY(legacyPlayer.getAngleY());
            if (proofps_dd::MsgUserCmdMove::shouldSend(pkt) ||
                (legacyPlayer.getOldAngleY() != legacyPlayer.getAngleY()))
            {
                proofps_dd::MsgUserCmdMove::setAngleY(pkt, legacyPlayer.getAngleY());
            }

            Weapon* const wpn = legacyPlayer.getWeapon();
            if (wpn)
            {
                // my xhair is used to update weapon angle
                wpn->UpdatePositions(legacyPlayer.getAttachedObject()->getPosVec(), m_pObjXHair->getPosVec());
                legacyPlayer.getWeaponAngle().Set(0.f, wpn->getObject3D().getAngleVec().getY(), wpn->getObject3D().getAngleVec().getZ());

                if (proofps_dd::MsgUserCmdMove::shouldSend(pkt) || (legacyPlayer.getOldWeaponAngle() != legacyPlayer.getWeaponAngle()))
                {
                    proofps_dd::MsgUserCmdMove::setWpnAngles(pkt, legacyPlayer.getWeaponAngle().getY(), legacyPlayer.getWeaponAngle().getZ());
                }
            }

            if (proofps_dd::MsgUserCmdMove::shouldSend(pkt))
            {   // shouldSend() at this point means that there were actual input so MsgUserCmdMove will be sent out
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
                
            }
            else
            {
                wpn->UpdatePosition(player.second.m_legacyPlayer.getAttachedObject()->getPosVec());
            }
        }
        
        if (getNetwork().isServer())
        {
            UpdateWeapons();
            UpdateBullets();
            UpdateRespawnTimers();
            PickupAndRespawnItems();
        }

        m_maps.Update();

        UpdateGameMode();  // TODO: on the long run this should be also executed only by server, now for fraglimit every instance executes ...

        if (getNetwork().isServer())
        {
            SendUserUpdates();
        }

        //map.UpdateVisibilitiesForRenderer();
    } // endif validConnection

    m_fps_ms = GetTickCount() - m_fps_ms;
    // this is horrible that FPS measuring is still not available from outside of PRRE .........
    m_fps_counter++;
    if ( GetTickCount() - GAME_FPS_INTERVAL >= m_fps_lastmeasure )
    {
        m_fps = m_fps_counter * (1000/GAME_FPS_INTERVAL);
        m_fps_counter = 0;
        m_fps_lastmeasure = GetTickCount();

        std::stringstream str;
        str << GAME_NAME << " " << GAME_VERSION << " :: FPS: " << m_fps;
        window.SetCaption(str.str());
    } 
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
        case proofps_dd::MsgBulletUpdate::id:
            HandleBulletUpdate(m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgBulletUpdate&>(pkt.msg.app.cData));
            break;
        case proofps_dd::MsgMapItemUpdate::id:
            HandleMapItemUpdate(m_connHandleServerSide, reinterpret_cast<const proofps_dd::MsgMapItemUpdate&>(pkt.msg.app.cData));
            break;
        default:
            getConsole().EOLn("CustomPGE::%s(): unknown msgId %u in MsgApp!", __func__, pkt.msg.app.msgId);
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
    m_sServerMapFilenameToLoad.clear();
    getPRRE().getObject3DManager().DeleteAll();
    getPRRE().getWindow().SetCursorVisible(true);

    getConsole().OOOLn("PRooFPSddPGE::onGameDestroying() done!");
    getConsole().Deinitialize();
}

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

std::map<std::string, Player_t>::iterator PRooFPSddPGE::getPlayerMapItByConnectionHandle(pge_network::PgeNetworkConnectionHandle connHandleServerSide)
{
    auto playerIt = m_mapPlayers.begin();
    while (playerIt != m_mapPlayers.end())
    {
        if (playerIt->second.m_connHandleServerSide == connHandleServerSide)
        {
            break;
        }
        playerIt++;
    }
    return playerIt;
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
        getConsole().OLn("PRooFPSddPGE::%s(): this is me, my name is %s, connHandleServerSide: %u, my IP: %s, map: %s",
            __func__, msg.m_szUserName, connHandleServerSide, msg.m_szIpAddress, msg.m_szMapFilename);
        // store our username so we can refer to it anytime later
        m_sUserName = msg.m_szUserName;

        if (getNetwork().isServer())
        {
            getPRRE().getUImanager().addText("Server, User name: " + m_sUserName, 10, 30);
            // server is not loading map here, it already loaded earlier for itself
        }
        else
        {
            getPRRE().getUImanager().addText("Client, User name: " + m_sUserName + "; IP: " + msg.m_szIpAddress, 10, 30);

            getPRRE().getUImanager().text("Loading Map: " + std::string(msg.m_szMapFilename) + " ...", 200, getPRRE().getWindow().getClientHeight() / 2);
            getPRRE().getRenderer()->RenderScene();

            const bool mapLoaded = m_maps.load(("gamedata/maps/" + std::string(msg.m_szMapFilename)).c_str());
            assert(mapLoaded);
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

    // each client will load all weapons into their weaponManager for their own setup, when they initialie themselves
    if (msg.m_bCurrentClient)
    {
        for (const auto& entry : std::filesystem::directory_iterator("gamedata/weapons/"))
        {
            if (entry.path().extension().string() == ".txt")
            {
                const bool bWpnLoaded = getWeaponManager().load(entry.path().string().c_str(), connHandleServerSide);
                assert(bWpnLoaded);
            }
        }

        // TODO: server should send the default weapon to client in this setup message, but for now we set same hardcoded
        // value on both side ... cheating is not possible anyway, since on server side server will always know what is
        // the default weapon for the player, so there is no use of overriding it on client side ...
        const bool bWpnDefaultSet = getWeaponManager().setDefaultAvailableWeapon("machinegun");
        assert(bWpnDefaultSet);
        
        for (const auto pWpn : getWeaponManager().getWeapons())
        {
            if (!pWpn)
            {
                continue;
            }

            // these will be the reference weapons, never visible, never moving, just to be copied!
            pWpn->getObject3D().Hide();
        }
    }

    Weapon* const wpnDefaultAvailable = getWeaponManager().getWeaponByName(getWeaponManager().getDefaultAvailableWeapon());
    assert(wpnDefaultAvailable);

    // and here the actual weapons for the specific player, these can be visible when active, moving with player, etc.
    // client doesnt do anything else with these, server also uses these to track and validate player weapons and related actions.
    for (const auto pWpn : getWeaponManager().getWeapons())
    {
        if (!pWpn)
        {
            continue;
        }

        Weapon* const pNewWeapon = new Weapon(*pWpn);
        m_mapPlayers[msg.m_szUserName].m_legacyPlayer.getWeapons().push_back(pNewWeapon);
        pNewWeapon->SetOwner(connHandleServerSide);
        if (pNewWeapon->getFilename() == wpnDefaultAvailable->getFilename())
        {
            pNewWeapon->getObject3D().Show();
            pNewWeapon->SetAvailable(true);
            m_mapPlayers[msg.m_szUserName].m_legacyPlayer.SetWeapon(pNewWeapon);
        }
        else
        {
            pNewWeapon->getObject3D().Hide();
        }
    }

    // Note that this is a waste of resources this way.
    // Because clients also store the full weapon instances for themselves, even though they dont use weapon cvars at all!
    // Task: On the long run, there should be a WeaponProxy or WeaponClient or something for the clients which are basically
    // just the image for their current weapon.
    // Task: And I also think that each CPlayer should have a WeaponManager instance, so player's weapons would be loaded there.
    // Task: the only problem here would be that the bullets list should be extracted into separate entity, so all WeaponManager
    // instances would refer to the same bullets list. And some functions may be enough to be static, but thats all!

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
    pge_network::PgePacket newPktUserUpdate;

    if (msg.bCurrentClient)
    {
        // server is processing its own birth
        if (m_mapPlayers.size() == 0)
        {
            getPRRE().getUImanager().text("Loading Map: " + m_sServerMapFilenameToLoad + " ...", 200, getPRRE().getWindow().getClientHeight() / 2);
            getPRRE().getRenderer()->RenderScene();

            Bullet::ResetGlobalBulletId();

            // server already loads the map for itself at this point, so no need for map filename in PktSetup, but we fill it anyway ...
            //const bool mapLoaded = m_maps.load("gamedata/maps/map_test_good.txt");
            const bool mapLoaded = m_maps.load((std::string("gamedata/maps/") + m_sServerMapFilenameToLoad).c_str());
            assert(mapLoaded);

            char szNewUserName[proofps_dd::MsgUserSetup::nUserNameMaxLength];
            genUniqueUserName(szNewUserName);
            getConsole().OLn("PRooFPSddPGE::%s(): first (local) user %s connected and I'm server, so this is me (connHandleServerSide: %u)",
                __func__, szNewUserName, connHandleServerSide);

            szConnectedUserName = szNewUserName;

            pge_network::PgePacket newPktSetup;
            proofps_dd::MsgUserSetup::initPkt(newPktSetup, connHandleServerSide, true, szConnectedUserName, msg.szIpAddress, m_maps.getFilename());

            const PRREVector& vecStartPos = m_maps.getRandomSpawnpoint();
            proofps_dd::MsgUserUpdate::initPkt(
                newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, 0.f, 100, false, 0, 0);

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
        proofps_dd::MsgUserSetup::initPkt(newPktSetup, connHandleServerSide, false, szConnectedUserName, msg.szIpAddress, m_maps.getFilename());

        const PRREVector& vecStartPos = m_maps.getRandomSpawnpoint();
        proofps_dd::MsgUserUpdate::initPkt(
            newPktUserUpdate, connHandleServerSide, vecStartPos.getX(), vecStartPos.getY(), vecStartPos.getZ(), 0.f, 0.f, 0.f, 100, false, 0, 0);

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
        // we also send MsgUserUpdate about each player so new client will immediately have their positions and other data updated.
        for (const auto& it : m_mapPlayers)
        {
            proofps_dd::MsgUserSetup::initPkt(
                newPktSetup,
                it.second.m_connHandleServerSide,
                false,
                it.first, it.second.m_sIpAddress,
                "" /* here mapFilename is irrelevant */);
            getNetwork().getServer().SendPacketToClient(connHandleServerSide, newPktSetup);

            proofps_dd::MsgUserUpdate::initPkt(
                newPktUserUpdate,
                it.second.m_connHandleServerSide,
                it.second.m_legacyPlayer.getAttachedObject()->getPosVec().getX(),
                it.second.m_legacyPlayer.getAttachedObject()->getPosVec().getY(),
                it.second.m_legacyPlayer.getAttachedObject()->getPosVec().getZ(),
                it.second.m_legacyPlayer.getAttachedObject()->getAngleVec().getY(),
                it.second.m_legacyPlayer.getWeapon()->getObject3D().getAngleVec().getY(),
                it.second.m_legacyPlayer.getWeapon()->getObject3D().getAngleVec().getZ(),
                it.second.m_legacyPlayer.getHealth(),
                false,
                it.second.m_legacyPlayer.getFrags(),
                it.second.m_legacyPlayer.getDeaths());
            getNetwork().getServer().SendPacketToClient(connHandleServerSide, newPktUserUpdate);
        }

        // we also send the state of all map items
        pge_network::PgePacket newPktMapItemUpdate;
        for (auto& itemPair : m_maps.getItems())
        {
            if (!itemPair.second)
            {
                continue;
            }

            proofps_dd::MsgMapItemUpdate::initPkt(
                newPktMapItemUpdate,
                0,
                itemPair.first,
                itemPair.second->isTaken());
            getNetwork().getServer().SendPacketToClient(connHandleServerSide, newPktMapItemUpdate);
        }
    }
}

void PRooFPSddPGE::HandleUserDisconnected(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const pge_network::MsgUserDisconnected&)
{
    const auto playerIt = getPlayerMapItByConnectionHandle(connHandleServerSide);
    if (m_mapPlayers.end() == playerIt)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return;
    }

    const std::string& sClientUserName = playerIt->first;

    if (getNetwork().isServer())
    {
        getConsole().OLn("PRooFPSddPGE::%s(): user %s disconnected and I'm server", __func__, sClientUserName.c_str());
    }
    else
    {
        getConsole().OLn("PRooFPSddPGE::%s(): user %s disconnected and I'm client", __func__, sClientUserName.c_str());
    }

    if (playerIt->second.m_legacyPlayer.getAttachedObject())
    {
        delete playerIt->second.m_legacyPlayer.getAttachedObject();  // yes, dtor will remove this from its Object3DManager too!
    }

    Weapon* const wpn = playerIt->second.m_legacyPlayer.getWeapon();
    if (wpn)
    {
        const auto wpnIt = std::find(getWeaponManager().getWeapons().begin(), getWeaponManager().getWeapons().end(), wpn);
        if (wpnIt != getWeaponManager().getWeapons().end())
        {
            getWeaponManager().getWeapons().erase(wpnIt);
        }
        delete wpn;
    }

    m_mapPlayers.erase(playerIt);

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

    const auto it = getPlayerMapItByConnectionHandle(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return;
    }

    const std::string& sClientUserName = it->first;

    if ((pktUserCmdMove.m_strafe == proofps_dd::Strafe::NONE) &&
        (!pktUserCmdMove.m_bJumpAction) && (!pktUserCmdMove.m_bSendSwitchToRunning) &&
        (pktUserCmdMove.m_fPlayerAngleY == -1.f) && (!pktUserCmdMove.m_bShouldSend) &&
        (!pktUserCmdMove.m_bShootAction))
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): user %s sent invalid cmdMove!", __func__, sClientUserName.c_str());
        return;
    }

    auto& legacyPlayer = it->second.m_legacyPlayer;

    if (legacyPlayer.getHealth() == 0)
    {
        if (pktUserCmdMove.m_bShootAction)
        {
            //getConsole().OLn("PRooFPSddPGE::%s(): user %s is requesting respawn", __func__, sClientUserName.c_str());
            return;
        }

        // for dead player, only the shoot action is allowed which is treated as respawn request
        getConsole().OLn("PRooFPSddPGE::%s(): ignoring cmdMove for user %s due to health is 0!", __func__, sClientUserName.c_str());
        return;
    }

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
        legacyPlayer.getAttachedObject()->getAngleVec().SetY(pktUserCmdMove.m_fPlayerAngleY);
    }

    Weapon* const wpn = legacyPlayer.getWeapon();
    if (wpn)
    {
        legacyPlayer.getWeaponAngle().Set(0.f, pktUserCmdMove.m_fWpnAngleY, pktUserCmdMove.m_fWpnAngleZ);
        wpn->getObject3D().getAngleVec().SetY(pktUserCmdMove.m_fWpnAngleY);
        wpn->getObject3D().getAngleVec().SetZ(pktUserCmdMove.m_fWpnAngleZ);

        if (pktUserCmdMove.m_bShootAction)
        {
            if (getNetwork().isServer())
            {
                // server will have the new bullet, clients will learn about the new bullet when server is sending out
                // the regular bullet updates
                wpn->shoot();
            }
        }
    }
}

void PRooFPSddPGE::HandleUserUpdate(pge_network::PgeNetworkConnectionHandle connHandleServerSide, const proofps_dd::MsgUserUpdate& msg)
{
    const auto it = getPlayerMapItByConnectionHandle(connHandleServerSide);
    if (m_mapPlayers.end() == it)
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): failed to find user with connHandleServerSide: %u!", __func__, connHandleServerSide);
        return;
    }

    //getConsole().OLn("PRooFPSddPGE::%s(): user %s received MsgUserUpdate: %f", __func__, it->first.c_str(), msg.m_pos.x);

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
        //it->second.m_legacyPlayer.getAngleY() = msg.m_fPlayerAngleY;
        it->second.m_legacyPlayer.getAttachedObject()->getAngleVec().SetY(msg.m_fPlayerAngleY);
    }

    it->second.m_legacyPlayer.getWeapon()->getObject3D().getAngleVec().SetY(msg.m_fWpnAngleY);
    it->second.m_legacyPlayer.getWeapon()->getObject3D().getAngleVec().SetZ(msg.m_fWpnAngleZ);

    //getConsole().OLn("PRooFPSddPGE::%s(): rcvd health: %d, health: %d, old health: %d",
    //    __func__, msg.m_nHealth, it->second.m_legacyPlayer.getHealth(), it->second.m_legacyPlayer.getOldHealth());

    it->second.m_legacyPlayer.getFrags() = msg.m_nFrags;
    it->second.m_legacyPlayer.getDeaths() = msg.m_nDeaths;

    it->second.m_legacyPlayer.SetHealth(msg.m_nHealth);

    if (msg.m_bRespawn)
    {
        getConsole().OLn("PRooFPSddPGE::%s(): player %s has respawned!", __func__, it->first.c_str());
        HandlePlayerRespawned(it->first == m_sUserName, it->second.m_legacyPlayer);
    }
    else
    {
        if ((it->second.m_legacyPlayer.getOldHealth() > 0) && (it->second.m_legacyPlayer.getHealth() == 0))
        {
            // only clients fall here, since server already set oldhealth to 0 at the beginning of this frame
            // because it had already set health to 0 in previous frame
            getConsole().OLn("PRooFPSddPGE::%s(): player %s has died!", __func__, it->first.c_str());
            HandlePlayerDied(it->first == m_sUserName, it->second.m_legacyPlayer);
        }
    }
}

void PRooFPSddPGE::HandleBulletUpdate(pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/, const proofps_dd::MsgBulletUpdate& msg)
{
    if (getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): server received MsgBulletUpdate, CANNOT HAPPEN!", __func__);
        assert(false);
        return;
    }

    Bullet* pBullet = nullptr;

    auto it = getWeaponManager().getBullets().begin();
    while (it != getWeaponManager().getBullets().end())
    {
        if (it->getId() == msg.m_bulletId)
        {
            break;
        }
        it++;
    }

    if (getWeaponManager().getBullets().end() == it)
    {
        if (msg.m_bDelete)
        {
            // this is valid scenario: when 2 players are at almost same position (partially overlapping), the bullet will immediately hit the other player
            // when being fired. In such case, we can just ignore doing anything here on client side.
            // TODO: btw why does sender send the message like this anyway to clients?!
            return;
        }
        // need to create this new bullet first on our side
        //getConsole().OLn("PRooFPSddPGE::%s(): user %s received MsgBulletUpdate: NEW bullet id %u", __func__, m_sUserName.c_str(), msg.m_bulletId);

        // TODO: here it is okay to get all the properties of the bullet, but if it is not a new bullet, it is not nice to
        // send every property in all BulletUpdate, this should be improved in future ...
        getWeaponManager().getBullets().push_back(
            Bullet(
                getPRRE(),
                msg.m_bulletId,
                msg.m_pos.x, msg.m_pos.y, msg.m_pos.z,
                msg.m_angle.x, msg.m_angle.y, msg.m_angle.z,
                msg.m_size.x, msg.m_size.y, msg.m_size.z) );
        pBullet = &(getWeaponManager().getBullets().back());
        it = getWeaponManager().getBullets().end();
        it--; // iterator points to this newly inserted last bullet
    }
    else
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): user %s received MsgBulletUpdate: old bullet id %u", __func__, m_sUserName.c_str(), msg.m_bulletId);
        pBullet = &(*it);
    }

    if (msg.m_bDelete)
    {
        getWeaponManager().getBullets().erase(it);
        return;
    }

    pBullet->getObject3D().getPosVec().Set(msg.m_pos.x, msg.m_pos.y, msg.m_pos.z);
}

void PRooFPSddPGE::HandleMapItemUpdate(pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/, const proofps_dd::MsgMapItemUpdate& msg)
{
    if (getNetwork().isServer())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): server received MsgMapItemUpdate, CANNOT HAPPEN!", __func__);
        assert(false);
        return;
    }

    const auto it = m_maps.getItems().find(msg.m_mapItemId);
    if (it == m_maps.getItems().end())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): unknown map item id %u, CANNOT HAPPEN!", __func__, msg.m_mapItemId);
        assert(false);
        return;
    }

    MapItem* const pMapItem = it->second;

    if (msg.m_bTaken)
    {
        pMapItem->Take();
    }
    else
    {
        pMapItem->UnTake();
    }
}