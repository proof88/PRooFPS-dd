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

#include <cassert>

#include "Consts.h"

// TODO cpp11 initializer list, and then it can be moved into the function too
static std::set<char> foregroundBlocks;
static std::set<char> backgroundBlocks;


// ############################### PUBLIC ################################


Maps::Maps(PR00FsReducedRenderingEngine& gfx) :
    m_gfx(gfx)
{
    m_blocks_h  = 0;
    m_blocks     = NULL;
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
    foregroundBlocks.insert('K');
    foregroundBlocks.insert('L');
    foregroundBlocks.insert('Q');
    foregroundBlocks.insert('T');

    backgroundBlocks.insert('a');
    backgroundBlocks.insert('c');
    backgroundBlocks.insert('e');
    backgroundBlocks.insert('n');
    backgroundBlocks.insert('n');
    backgroundBlocks.insert('o');
    backgroundBlocks.insert('r');
    backgroundBlocks.insert('v');
    backgroundBlocks.insert('u');
    backgroundBlocks.insert('w');
    backgroundBlocks.insert('x');
    backgroundBlocks.insert('y');
    backgroundBlocks.insert('z');
    
    // the special foreground stuff (e.g. items) are treated as background blocks too, see special handling in lineHandleLayout()
    backgroundBlocks.insert('+');
    backgroundBlocks.insert('M');
    backgroundBlocks.insert('P');
    backgroundBlocks.insert('S');

    MapItem::ResetGlobalMapItemId();
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
    return ( m_blocks != NULL );
}

bool Maps::load(const char* fname)
{
    getConsole().OLnOI("Maps::load(%s) ...", fname);

    // this wont be needed after we require unload() before consecutive load()
    MapItem::ResetGlobalMapItemId();

    m_sFileName = PFL::getFilename(fname);
    m_sRawName = PFL::changeExtension(m_sFileName.c_str(), "");
    if (m_sRawName.empty())
    {
        getConsole().EOLnOO("ERROR: empty raw name!");
        unload();
        return false;
    }

    std::ifstream f;
    f.open(fname, std::ifstream::in);
    if ( !f.good() )
    {
        getConsole().EOLnOO("ERROR: failed to open file %s!", fname);
        unload();
        return false;
    }

    const TPRRE_ISO_TEX_FILTERING texFilterMinOriginal = m_gfx.getTextureManager().getDefaultMinFilteringMode();
    const TPRRE_ISO_TEX_FILTERING texFilterMagOriginal = m_gfx.getTextureManager().getDefaultMagFilteringMode();
    m_gfx.getTextureManager().setDefaultIsoFilteringMode(
        TPRRE_ISO_TEX_FILTERING::PRRE_ISO_LINEAR_MIPMAP_LINEAR,
        TPRRE_ISO_TEX_FILTERING::PRRE_ISO_LINEAR);

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

    m_gfx.getTextureManager().setDefaultIsoFilteringMode(
        texFilterMinOriginal,
        texFilterMagOriginal);

    if ( bParseError )
    {
        getConsole().EOLnOO("ERROR: failed to parse file!");
        unload();
        return false;
    }

    m_blockPosMin = m_blocks[0]->getPosVec();
    m_blockPosMax = m_blocks[0]->getPosVec();
    for (int i = 0; i < m_blocks_h; i++)
    {
        if ( m_blocks[i] != PGENULL )
        {
            if (m_blocks[i]->getPosVec().getX() < m_blockPosMin.getX())
            {
                m_blockPosMin.SetX(m_blocks[i]->getPosVec().getX());
            }
            else if (m_blocks[i]->getPosVec().getX() > m_blockPosMax.getX())
            {
                m_blockPosMax.SetX(m_blocks[i]->getPosVec().getX());
            }

            if (m_blocks[i]->getPosVec().getY() < m_blockPosMin.getY())
            {
                m_blockPosMin.SetY(m_blocks[i]->getPosVec().getY());
            }
            else if (m_blocks[i]->getPosVec().getY() > m_blockPosMax.getY())
            {
                m_blockPosMax.SetY(m_blocks[i]->getPosVec().getY());
            }

            if (m_blocks[i]->getPosVec().getZ() < m_blockPosMin.getZ())
            {
                m_blockPosMin.SetZ(m_blocks[i]->getPosVec().getZ());
            }
            else if (m_blocks[i]->getPosVec().getZ() > m_blockPosMax.getZ())
            {
                m_blockPosMax.SetZ(m_blocks[i]->getPosVec().getZ());
            }
        }
    }
    m_blocksVertexPosMin.Set(m_blockPosMin.getX() - GAME_BLOCK_SIZE_X / 2.f, m_blockPosMin.getY() - GAME_BLOCK_SIZE_Y / 2.f, m_blockPosMin.getZ() - GAME_BLOCK_SIZE_Z / 2.f);
    m_blocksVertexPosMax.Set(m_blockPosMax.getX() + GAME_BLOCK_SIZE_X / 2.f, m_blockPosMax.getY() + GAME_BLOCK_SIZE_Y / 2.f, m_blockPosMax.getZ() + GAME_BLOCK_SIZE_Z / 2.f);

    getConsole().SOLnOO("Map loaded with width %d and height %d!", m_width, m_height);
    return true;
}

void Maps::unload()
{
    getConsole().OLnOI("Maps::unload() ...");
    m_sRawName.clear();
    m_sFileName.clear();
    m_Block2Texture.clear();
    if ( m_blocks )
    {
        for (int i = 0; i < m_blocks_h; i++)
        {
            m_gfx.getObject3DManager().DeleteAttachedInstance( *(m_blocks[i]) );
        }
        free( m_blocks );
        m_blocks = NULL;
        m_blocks_h = 0;
    }
    for (auto& itemPair : m_items)
    {
        delete itemPair.second;
    }
    m_items.clear();
    m_width = 0;
    m_height = 0;
    m_blockPosMin.SetZero();
    m_blockPosMax.SetZero();
    m_blocksVertexPosMin.SetZero();
    m_blocksVertexPosMax.SetZero();
    m_vars.clear();
    m_spawnpoints.clear();

    MapItem::ResetGlobalMapItemId();

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

    for (int i = 0; i < m_blocks_h; i++)
    {
        PRREObject3D* obj = m_blocks[i];
        if ( obj != PGENULL )
        {
            if ( (obj->getPosVec().getX() + obj->getSizeVec().getX()/2.0f) <= campos.getX() - 7.f )
            {
                obj->SetRenderingAllowed(false);
            }
            else
            {
                if ( (obj->getPosVec().getX() - obj->getSizeVec().getX()/2.0f) >= campos.getX() + 7.f )
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

const std::string& Maps::getFilename() const
{
    return m_sFileName;
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

const PRREVector& Maps::getBlockPosMin() const
{
    return m_blockPosMin;
}

const PRREVector& Maps::getBlockPosMax() const
{
    return m_blockPosMax;
}

const PRREVector& Maps::getBlocksVertexPosMin() const
{
    return m_blocksVertexPosMin;
}

const PRREVector& Maps::getBlocksVertexPosMax() const
{
    return m_blocksVertexPosMax;
}

PRREObject3D** Maps::getBlocks()
{
    return m_blocks;
}

int Maps::getBlockCount() const
{
    return m_blocks_h;
}

const std::map<MapItem::MapItemId, MapItem*>& Maps::getItems() const
{
    return m_items;
}

const std::map<std::string, PGEcfgVariable>& Maps::getVars() const
{
    return m_vars;
}

void Maps::Update()
{
    // invoked by both server and client
    for (auto& itemPair : getItems())
    {
        if (!itemPair.second)
        {
            continue;
        }

        MapItem& mapItem = *(itemPair.second);
        if (mapItem.isTaken())
        {
            continue;
        }

        mapItem.Update(8.f);
    }
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
    std::string::size_type iLinePos = 0;
    
    // Item character specifies the item type, but not the background behind the item.
    // So the idea is to copy the previous _neighbor_ background block to be used behind the item, but
    // only if there is a neighbor block created previously, otherwise we should not put any
    // background block behind the item.
    // So iObjectBgToBeCopied is > -1 only if there is neighbor background block created previously.
    int iObjectBgToBeCopied = -1;

    while ( iLinePos != sLine.length() )
    {
        const char c = sLine[iLinePos];
        iLinePos++;
        const bool bForeground = foregroundBlocks.find(c) != foregroundBlocks.end();
        const bool bBackground = backgroundBlocks.find(c) != backgroundBlocks.end();

        x = x + GAME_BLOCK_SIZE_X;
        if ( x > maxx )
        {
            maxx = x;
        }
        
        if ( !bForeground && !bBackground )
        {
            iObjectBgToBeCopied = -1;
            continue;
        }

        if (bForeground && bBackground)
        {
            assert(false);
            return false;
        }

        // special background block handling
        bool bCopyPreviousBgBlock = false;
        bool bSpecialBlock = false;
        MapItem* pMapItem;
        switch (c)
        {
        case '+':
            pMapItem = new MapItem(m_gfx, MapItemType::ITEM_HEALTH, PRREVector(x, y, GAME_PLAYERS_POS_Z));
            m_items.insert({pMapItem->getId(), pMapItem});
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        case 'M':
            pMapItem = new MapItem(m_gfx, MapItemType::ITEM_WPN_MACHINEGUN, PRREVector(x, y, GAME_PLAYERS_POS_Z));
            m_items.insert({ pMapItem->getId(), pMapItem });
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        case 'P':
            pMapItem = new MapItem(m_gfx, MapItemType::ITEM_WPN_PISTOL, PRREVector(x, y, GAME_PLAYERS_POS_Z));
            m_items.insert({ pMapItem->getId(), pMapItem });
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        case 'S':
            // spawnpoint is background block by default
            m_spawnpoints.insert(PRREVector(x, y, GAME_PLAYERS_POS_Z));
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        default: /* NOP */;
        }

        PRREObject3D* pNewBlockObj = nullptr;
        if (!bSpecialBlock || (bSpecialBlock && bCopyPreviousBgBlock))
        {
            m_blocks_h++;
            if (!bSpecialBlock && bBackground)
            {
                iObjectBgToBeCopied = m_blocks_h - 1;
            }
            // TODO: handle memory allocation error
            m_blocks = (PRREObject3D**)realloc(m_blocks, m_blocks_h * sizeof(PRREObject3D*));
            pNewBlockObj = m_gfx.getObject3DManager().createBox(GAME_BLOCK_SIZE_X, GAME_BLOCK_SIZE_X, GAME_BLOCK_SIZE_X);
            m_blocks[m_blocks_h - 1] = pNewBlockObj;
            m_blocks[m_blocks_h - 1]->SetLit(true);
            //m_blocks[m_blocks_h - 1]->Hide();
        }

        if (!pNewBlockObj)
        {
            continue;
        }

        PRRETexture* tex = PGENULL;
        if (bSpecialBlock && bCopyPreviousBgBlock)
        {
            tex = m_blocks[iObjectBgToBeCopied]->getMaterial().getTexture();
        }
        else
        {
            if (m_Block2Texture.find(c) == m_Block2Texture.end())
            {
                const std::string sc(1, c); // WA for CConsole lack support of %c
                getConsole().EOLn("%s No texture defined for block %s!", __FUNCTION__, sc.c_str());
                tex = m_texRed;
            }
            else
            {
                const std::string sTexName = "gamedata\\textures\\" + m_sRawName + "\\" + m_Block2Texture[c];
                tex = m_gfx.getTextureManager().createFromFile(sTexName.c_str());
                if (!tex)
                {
                    getConsole().EOLn("%s Could not load texture %s!", __FUNCTION__, sTexName.c_str());
                    tex = m_texRed;
                }
            }
            if (!tex)
            {
                // should happen only if default red texture could not be loaded, but that should had been detected in initialize() tho
                const std::string sc(1, c); // WA for CConsole lack support of %c
                getConsole().EOLn("%s Not assigning any texture for block %s!", __FUNCTION__, sc.c_str());
            }
        }

        pNewBlockObj->getMaterial().setTexture(tex);

        if (bForeground)
        {
            // only foreground blocks should be checked for collision
            pNewBlockObj->SetColliding_TO_BE_REMOVED(true);
        }

        pNewBlockObj->getPosVec().Set(x, y, bBackground ? 0.0f : -GAME_BLOCK_SIZE_Z);
    }
    y = y - GAME_BLOCK_SIZE_Y;
    return true;
}
