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

// TODO cpp11 initializer list, and then it can be moved into the function too
static std::set<char> foregroundBlocks;
static std::set<char> backgroundBlocks;

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
    m_width = 0;
    m_height = 0;

    foregroundBlocks.insert('A');
    foregroundBlocks.insert('B');
    foregroundBlocks.insert('C');
    foregroundBlocks.insert('D');
    foregroundBlocks.insert('E');
    foregroundBlocks.insert('F');
    foregroundBlocks.insert('G');

    backgroundBlocks.insert('H');
    backgroundBlocks.insert('I');
    backgroundBlocks.insert('J');
    backgroundBlocks.insert('K');
    backgroundBlocks.insert('L');
    backgroundBlocks.insert('M');
    backgroundBlocks.insert('N');
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
    if ( !f.good() )
    {
        getConsole().EOLnOO("ERROR: failed to open file!");
        return false;
    }

    bool bParseError = false;
    bool bMapLayoutReached = false;
    const std::streamsize nBuffSize = 1024;
    char cLine[nBuffSize];
    TPRREfloat y = 4.0f;
    while ( !bParseError && !f.eof() )
    {
        f.getline(cLine, nBuffSize);
        // TODO: we should finally have a strClr() version for std::string or FINALLY UPGRADE TO NEWER CPP THAT MAYBE HAS THIS FUNCTIONALITY!!!
        PFL::strClr( cLine );
        const std::string sLine(cLine);
        std::string sVar, sValue;
        if ( lineShouldBeIgnored(sLine) )
        {
            continue;
        }
        else if ( lineIsValueAssignment(sLine, sVar, sValue) )
        {
            // TODO assign value
            if ( bMapLayoutReached )
            {
                getConsole().EOLn("ERROR: parse: assignment after map layout block: %s!", sLine.c_str());
                bParseError = true;
            }
        }
        else
        {
            bMapLayoutReached = true;
            bParseError &= lineHandleLayout(sLine, y);
        }
    };
    f.close();

    if ( bParseError )
    {
        getConsole().EOLnOO("ERROR: failed to parse file!");
        return false;
    }

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
    m_width = 0;
    m_height = 0;
}

void Maps::shutdown()
{
    if ( m_gfx.isInitialized() )
    {
        unload();
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

unsigned int Maps::width() const
{
    return m_width;
}

unsigned int Maps::height() const
{
    return m_height;
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

bool Maps::lineShouldBeIgnored(const std::string& sLine)
{
    return sLine.empty() || (sLine[0] == '#');
}

bool Maps::lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue)
{
    const std::string::size_type nAssignmentPos = sLine.find('=');
    if ( nAssignmentPos == std::string::npos )
    {
        return false;
    }

    if ( (nAssignmentPos == (sLine.length() - 1)) || (nAssignmentPos == 0 ) )
    {
        CConsole::getConsoleInstance("Maps").EOLn("ERROR: erroneous assignment: %s!", sLine.c_str());
        return false;
    }

    sVar = sLine.substr(0, nAssignmentPos);
    sValue = sLine.substr(nAssignmentPos+1);
    return true;
}

bool Maps::lineHandleLayout(const std::string& sLine, TPRREfloat& y)
{
    m_height++;
    if ( m_width < sLine.length() )
    {
        m_width = sLine.length();
    }

    TPRREfloat x = 0.0f;
    TPRREfloat maxx = x;
    std::string::size_type i = 0;

    while ( i != sLine.length() )
    {
        const char c = sLine[i];
        i++;
        const bool bForeground = foregroundBlocks.find(c) != foregroundBlocks.end();
        const bool bBackground = backgroundBlocks.find(c) != backgroundBlocks.end();

        x = x + GAME_BLOCK_SIZE_X;
        if ( x > maxx )
        {
            maxx = x;
        }
        
        if ( !bForeground && !bBackground )
        {
            continue;
        }

        m_objects_h++;
        m_objects = (PRREObject3D**) realloc(m_objects, m_objects_h * sizeof(PRREObject3D*));
        m_objects[m_objects_h-1] = m_gfx.getObject3DManager().createBox(GAME_BLOCK_SIZE_X, GAME_BLOCK_SIZE_X, GAME_BLOCK_SIZE_X);
        m_objects[m_objects_h-1]->SetLit(true);
        m_objects[m_objects_h-1]->getMaterial().setTexture(m_tex_brick1);
        m_objects[m_objects_h-1]->SetColliding_TO_BE_REMOVED(true);

        m_objects[m_objects_h-1]->getPosVec().Set(x, y, bBackground ? 0.0f : -GAME_BLOCK_SIZE_Z);

        /*switch (c) {
            case 'Q': PRREVector tmp(x, y, 0.0f);
                      m_candleLights.push_back(tmp);
                      break;
          }*/

        /*case 'I': m_start.SetX(x);
                    m_start.SetY(y);
                    m_start.SetZ(GAME_PLAYERS_POS_Z);
                    break;
          case 'V': m_end.SetX(x);
                    m_end.SetY(y);
                    m_end.SetZ(GAME_PLAYERS_POS_Z);
                    break;*/
    }
    y = y - GAME_BLOCK_SIZE_Y;
    return true;
}
