/*
    ###################################################################################
    Maps.cpp
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "Maps.h"

#include "Consts.h"


// ############################### PUBLIC ################################


Maps::Maps(PR00FsReducedRenderingEngine& gfx) :
    m_gfx(gfx)
{
    m_objects_h  = 0;
    m_objects    = NULL;
    m_tex_brick1 = PGENULL;
    m_tex_brick2 = PGENULL;
    m_tex_brick3 = PGENULL;
    m_tex_brick4 = PGENULL;
    m_tex_crate  = PGENULL;
    m_tex_floor  = PGENULL;
    m_tex_aztec1 = PGENULL;
    m_objectsMinY = 0.0f;
}

Maps::~Maps()
{
    shutdown();
}

CConsole& Maps::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* Maps::getLoggerModuleName()
{
    return "Maps";
}

bool Maps::initialize()
{
    m_tex_brick1 = m_gfx.getTextureManager().createFromFile("gamedata\\textures\\brick1.bmp");
    m_tex_brick2 = m_gfx.getTextureManager().createFromFile("gamedata\\textures\\brick2.bmp");
    m_tex_brick3 = m_gfx.getTextureManager().createFromFile("gamedata\\textures\\brick3.bmp");
    m_tex_brick4 = m_gfx.getTextureManager().createFromFile("gamedata\\textures\\brick4.bmp");
    m_tex_aztec1 = m_gfx.getTextureManager().createFromFile("gamedata\\textures\\aztec01.bmp");
    m_tex_castle4 = m_gfx.getTextureManager().createFromFile("gamedata\\textures\\castle04.bmp");
    m_tex_crate  = m_gfx.getTextureManager().createFromFile("gamedata\\textures\\crate.bmp");
    m_tex_floor  = m_gfx.getTextureManager().createFromFile("gamedata\\textures\\floor.bmp");
    return true;
}

bool Maps::loaded() const
{
    return ( m_objects != NULL );
}

bool Maps::load(const char* fname)
{
    getConsole().OLnOI("Maps::load(%s) ...", fname);
    std::ifstream f;
    f.open (fname, std::ifstream::in);
    char c;
    TPRREfloat x = 0.0f, y = 4.0f;
    TPRREfloat maxx = x;

    if ( !f.good() )
    {
        getConsole().EOLnOO("ERROR: failed to open file!");
        return false;
    }

    do
    {
        c = static_cast<char>(f.get());
        switch (c)
        {
          case 'A':
          case 'C':
          case 'Q':
          case 'X':
          case 'H':
          case 'B':
          case 'D':
          case 'I':
          case 'V':
          case 'P': m_objects_h++;
                    m_objects = (PRREObject3D**) realloc(m_objects, m_objects_h * sizeof(PRREObject3D*));
                    if ( m_objects ) {
                      switch (c) {
                        case 'A':
                        case 'C':
                        case 'Q':
                        case 'H':
                        case 'B':
                        case 'D':
                        case 'I':
                        case 'V':
                        case 'X':
                        case 'P': m_objects[m_objects_h-1] = m_gfx.getObject3DManager().createBox(GAME_BLOCK_SIZE_X, GAME_BLOCK_SIZE_X, GAME_BLOCK_SIZE_X);
                                  m_objects[m_objects_h-1]->SetLit(true);
                                  break;
                      }
                      switch (c) {
                        case 'H': 
                        case 'X': m_objects[m_objects_h-1]->getMaterial().setTexture(m_tex_brick1);
                                  m_objects[m_objects_h-1]->SetColliding_TO_BE_REMOVED(true);
                                  break;
                        case 'B': m_objects[m_objects_h-1]->getMaterial().setTexture(m_tex_brick2);
                                  m_objects[m_objects_h-1]->SetColliding_TO_BE_REMOVED(true);
                                  break;
                        case 'P': m_objects[m_objects_h-1]->getMaterial().setTexture(m_tex_floor);
                                  m_objects[m_objects_h-1]->SetColliding_TO_BE_REMOVED(true);
                                  break;
                        case 'D': m_objects[m_objects_h-1]->getMaterial().setTexture(m_tex_crate);
                                  m_objects[m_objects_h-1]->SetColliding_TO_BE_REMOVED(true);
                                  break;
                        case 'Q': m_objects[m_objects_h-1]->getMaterial().setTexture(m_tex_brick1);
                                  m_objects[m_objects_h-1]->SetColliding_TO_BE_REMOVED(true);
                                  break;
                        case 'A': m_objects[m_objects_h-1]->getMaterial().setTexture(m_tex_aztec1);
                                  m_objects[m_objects_h-1]->SetColliding_TO_BE_REMOVED(true);
                                  break;
                        case 'C': m_objects[m_objects_h-1]->getMaterial().setTexture(m_tex_castle4);
                                  m_objects[m_objects_h-1]->SetColliding_TO_BE_REMOVED(true);
                                  break;
                        case 'I': m_objects[m_objects_h-1]->getMaterial().setTexture(m_tex_brick3);
                                  break;
                        case 'V': m_objects[m_objects_h-1]->getMaterial().setTexture(m_tex_brick4);
                                  break;                                           
                      }

                      //m_objects[m_objects_h-1]->setVertexTransferMode(PRRE_VT_RETAINED);

                      switch (c) {
                        case 'Q': PRREVector tmp(x, y, 0.0f);
                                  m_candleLights.push_back(tmp);
                                  break;
                      }

                      switch (c) {
                        case 'A':
                        case 'Q':
                        case 'X':
                        case 'H':
                        case 'I':
                        case 'V':
                        case 'B': m_objects[m_objects_h-1]->getPosVec().Set(x, y, 0.0f); 
                                  break;
                      }
                      switch (c) {
                        case 'C':
                        case 'D':
                        case 'P': m_objects[m_objects_h-1]->getPosVec().Set(x, y, -GAME_BLOCK_SIZE_Z); 
                                  break;
                      } 
                      switch (c) {
                        case 'I': m_start.SetX(x);
                                  m_start.SetY(y);
                                  m_start.SetZ(GAME_PLAYERS_POS_Z);
                                  break;
                        case 'V': m_end.SetX(x);
                                  m_end.SetY(y);
                                  m_end.SetZ(GAME_PLAYERS_POS_Z);
                                  break;
                      }
                      switch (c) {
                        case 'X': //EnemyManager->Create(x,y,GAME_PLAYERS_POS_Z);
                                  break;
                      }
                      x = x + GAME_BLOCK_SIZE_X;
                      if ( x > maxx ) maxx = x;
                    }
                    else
                    {
                      return false;
                    }
                    break;
          case '\n': y = y - GAME_BLOCK_SIZE_Y;
                     x = 0.0;
                     break;
        }
    } while (f.good());

    f.close();

    m_gfx.getCamera().getPosVec().Set(m_end.getX(), m_end.getY(), GAME_CAM_Z);

    m_objectsMinY = m_objects[0]->getPosVec().getY();
    for (int i = 0; i < m_objects_h; i++)
    {
        if ( m_objects[i] != PGENULL )
        {
            if ( m_objects[i]->getPosVec().getY() < m_objectsMinY )
                m_objectsMinY = m_objects[i]->getPosVec().getY();
        }
    }

    getConsole().SOLnOO("Map loaded!");
    return true;
}

void Maps::unload()
{
    m_candleLights.clear();
    if ( m_objects )
    {
        for (int i = 0; i < m_objects_h; i++)
        {
            m_gfx.getObject3DManager().DeleteAttachedInstance( *(m_objects[i]) );
        }
        free( m_objects );
        m_objects = NULL;
        m_objects_h = 0;
    }
}

void Maps::shutdown()
{
    unload();
    if ( m_gfx.isInitialized() )
    {
        m_gfx.getTextureManager().DeleteAll();
        m_tex_brick1 = PGENULL;
        m_tex_brick2 = PGENULL;
        m_tex_brick3 = PGENULL;
        m_tex_brick4 = PGENULL;
        m_tex_crate = PGENULL;
        m_tex_floor = PGENULL;   
        m_tex_aztec1 = PGENULL;
        m_tex_castle4 = PGENULL;
    }
}

void Maps::updateVisibilitiesForRenderer()
{
    const PRREVector campos = m_gfx.getCamera().getPosVec();

    for (int i = 0; i < m_objects_h; i++)
    {
        PRREObject3D* obj = m_objects[i];
        if ( obj != PGENULL )
        {
            if ( (obj->getPosVec().getX() + obj->getSizeVec().getX()/2.0f) <= campos.getX()-RENDERER_MIN_X )
            {
                obj->SetRenderingAllowed(false);
            }
            else
            {
                if ( (obj->getPosVec().getX() - obj->getSizeVec().getX()/2.0f) >= campos.getX()+RENDERER_MAX_X )
                {
                    obj->SetRenderingAllowed(false);
                }
                else
                {
                    obj->SetRenderingAllowed(true);
                }
            }
        }
    }
}

PRREVector& Maps::getStartPos()
{
    return m_start;
}

PRREVector& Maps::getEndPos()
{
    return m_end;
}

float Maps::getObjectsMinY() const
{
    return m_objectsMinY;
}

std::vector<PRREVector>& Maps::getCandleLights()
{
    return m_candleLights;
}



// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################

