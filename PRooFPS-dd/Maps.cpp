/*
    ###################################################################################
    Maps.cpp
    Map loader for PRooFPS-dd
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
    m_texRed     = PGENULL;
    m_width = 0;
    m_height = 0;

    // TODO cpp11 initializer list
    foregroundBlocks.insert('B');
    foregroundBlocks.insert('D');
    foregroundBlocks.insert('F');
    foregroundBlocks.insert('G');
    foregroundBlocks.insert('H');
    foregroundBlocks.insert('I');
    foregroundBlocks.insert('J');

    backgroundBlocks.insert('a');
    backgroundBlocks.insert('c');
    backgroundBlocks.insert('e');
    backgroundBlocks.insert('m');
    backgroundBlocks.insert('n');
    backgroundBlocks.insert('o');
    backgroundBlocks.insert('r');
    backgroundBlocks.insert('S');
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
    m_texRed = m_gfx.getTextureManager().createFromFile("gamedata\\textures\\red.bmp");
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
        else if ( lineIsValueAssignment(sLine, sVar, sValue, bParseError) )
        {
            // TODO assign value
            if ( bMapLayoutReached )
            {
                getConsole().EOLn("ERROR: parse: assignment after map layout block: %s!", sLine.c_str());
                bParseError = true;
            }
            else
            {
                lineHandleAssignment(sVar, sValue);
            }
        }
        else if ( !bParseError )
        {
            bMapLayoutReached = true;
            bParseError &= lineHandleLayout(sLine, y);
        }
    };
    f.close();

    if ( bParseError )
    {
        getConsole().EOLnOO("ERROR: failed to parse file!");
        unload();
        return false;
    }

    m_posMin = m_objects[0]->getPosVec();
    m_posMax = m_objects[0]->getPosVec();
    for (int i = 0; i < m_objects_h; i++)
    {
        if ( m_objects[i] != PGENULL )
        {
            if (m_objects[i]->getPosVec().getX() < m_posMin.getX())
            {
                m_posMin.SetX(m_objects[i]->getPosVec().getX());
            }
            else if (m_objects[i]->getPosVec().getX() > m_posMax.getX())
            {
                m_posMax.SetX(m_objects[i]->getPosVec().getX());
            }

            if (m_objects[i]->getPosVec().getY() < m_posMin.getY())
            {
                m_posMin.SetY(m_objects[i]->getPosVec().getY());
            }
            else if (m_objects[i]->getPosVec().getY() > m_posMax.getY())
            {
                m_posMax.SetY(m_objects[i]->getPosVec().getY());
            }

            if (m_objects[i]->getPosVec().getZ() < m_posMin.getZ())
            {
                m_posMin.SetZ(m_objects[i]->getPosVec().getZ());
            }
            else if (m_objects[i]->getPosVec().getZ() > m_posMax.getZ())
            {
                m_posMax.SetZ(m_objects[i]->getPosVec().getZ());
            }
        }
    }

    getConsole().SOLnOO("Map loaded with width %d and height %d!", m_width, m_height);
    return true;
}

void Maps::unload()
{
    getConsole().OLnOI("Maps::unload() ...");
    m_Block2Texture.clear();
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
    m_posMin.SetZero();
    m_posMax.SetZero();
    m_vars.clear();
    m_spawnpoints.clear();
    getConsole().OOOLn("Maps::unload() done!");
}

void Maps::shutdown()
{
    getConsole().OLnOI("Maps::shutdown() ...");
    if ( m_gfx.isInitialized() )
    {
        unload();
    }
    getConsole().OOOLn("Maps::shutdown() done!");
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

const std::set<PRREVector>& Maps::getSpawnpoints() const
{
    return m_spawnpoints;
}

const PRREVector& Maps::getRandomSpawnpoint() const
{
    if ( m_spawnpoints.empty() )
    {
        throw std::runtime_error("No spawnpoints!");
    }
    const int iElem = PFL::random(0, m_spawnpoints.size()-1);
    auto it = m_spawnpoints.begin();
    int i = 0;
    while ( i < iElem )
    {
        i++;
        it++;
    }
    return *it;
}

const PRREVector& Maps::getObjectsMin() const
{
    return m_posMin;
}

const PRREVector& Maps::getObjectsMax() const
{
    return m_posMax;
}

std::vector<PRREVector>& Maps::getCandleLights()
{
    return m_candleLights;
}

const std::map<std::string, PGEcfgVariable>& Maps::getVars() const
{
    return m_vars;
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


bool Maps::lineShouldBeIgnored(const std::string& sLine)
{
    return sLine.empty() || (sLine[0] == '#');
}

bool Maps::lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue, bool& bParseError)
{
    const std::string::size_type nAssignmentPos = sLine.find('=');
    if ( nAssignmentPos == std::string::npos )
    {
        return false;
    }

    if ( (nAssignmentPos == (sLine.length() - 1)) || (nAssignmentPos == 0 ) )
    {
        CConsole::getConsoleInstance("Maps").EOLn("ERROR: erroneous assignment: %s!", sLine.c_str());
        bParseError = true;
        return false;
    }

    // sLine is already trimmed: neither leading nor trailing spaces

    // get rid of trailing spaces from the variable name itself, standing before the '=' char
    // TODO: should rather use std::string compatible PFL::strClr()
    std::string::size_type nSpPos = sLine.find(' ');
    if ( nSpPos != std::string::npos )
    {
        if ( nSpPos < nAssignmentPos )
        {
            sVar = sLine.substr(0, nSpPos);
            if ( sVar.find(' ') != std::string::npos )
            {
                // we should not have more space before '=' char
                CConsole::getConsoleInstance("Maps").EOLn("ERROR: erroneous assignment, failed to parse variable in line: %s!", sLine.c_str());
                bParseError = true;
                return false;
            }
        }
        else
        {
            // should never reach this point based on above 2 conditions
            CConsole::getConsoleInstance("Maps").EOLn("ERROR: erroneous assignment: %s!", sLine.c_str());
            bParseError = true;
            return false;
        }
    }
    else
    {
        sVar = sLine.substr(0, nAssignmentPos);
    }

    // get rid of leading spaces from the value itself, standing after the '=' char
    std::string::size_type i = nAssignmentPos+1;
    while ( (i < sLine.length()) && sLine[i] == ' ' )
    {
        i++;
    }

    if ( i < sLine.length() )
    {
        sValue = sLine.substr(i);
    }
    else
    {
        CConsole::getConsoleInstance("Maps").EOLn("ERROR: erroneous assignment, failed to parse value in line: %s!", sLine.c_str());
        bParseError = true;
        return false;
    }

    return true;
}

void Maps::lineHandleAssignment(std::string& sVar, std::string& sValue)
{
    if ( sVar.length() == 1 )
    {
        // dont store these variables, they just for block texture assignment
        m_Block2Texture[sVar[0]] = sValue;
        getConsole().OLn("Block %s has texture %s", sVar.c_str(), sValue.c_str());
        return;
    }
    // only vars with length > 1 are to be stored as actual variables
    getConsole().OLn("Var \"%s\" = \"%s\"", sVar.c_str(), sValue.c_str());
    m_vars[sVar] = sValue.c_str();
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

        PRRETexture* tex = PGENULL;
        if ( m_Block2Texture.find(c) == m_Block2Texture.end() )
        {
            const std::string sc(1,c); // WA for CConsole lack support of %c
            getConsole().EOLn("%s No texture defined for block %s!", __FUNCTION__, sc.c_str());
            tex = m_texRed;
        }
        else
        {
            const std::string sTexName = "gamedata\\textures\\" + m_Block2Texture[c];
            tex = m_gfx.getTextureManager().createFromFile(sTexName.c_str());
            if ( !tex )
            {
                getConsole().EOLn("%s Could not load texture %s!", __FUNCTION__, sTexName.c_str());
                tex = m_texRed;
            }
        }
        if ( !tex )
        {
            // should happen only if default red texture could not be loaded, but that should had been detected in initialize() tho
            const std::string sc(1,c); // WA for CConsole lack support of %c
            getConsole().EOLn("%s Not assigning any texture for block %s!", __FUNCTION__, sc.c_str());
        }

        m_objects[m_objects_h-1]->getMaterial().setTexture(tex);
        m_objects[m_objects_h-1]->SetColliding_TO_BE_REMOVED(true);

        m_objects[m_objects_h-1]->getPosVec().Set(x, y, bBackground ? 0.0f : -GAME_BLOCK_SIZE_Z);

        switch (c)
        {
        case 'S': m_spawnpoints.insert(PRREVector(x, y, GAME_PLAYERS_POS_Z));
                  break;
        default: /* NOP */;
        }
    }
    y = y - GAME_BLOCK_SIZE_Y;
    return true;
}
