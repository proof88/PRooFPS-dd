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

#include <cassert>

#include "Consts.h"
#include "Maps.h"


// ############################### PUBLIC ################################


proofps_dd::Maps::Maps(PR00FsUltimateRenderingEngine& gfx) :
    m_gfx(gfx),
    m_blocks(NULL),
    m_blocks_h(0),
    m_foregroundBlocks(NULL),
    m_foregroundBlocks_h(0),
    m_texRed(PGENULL),
    m_width(0),
    m_height(0)
{
    proofps_dd::MapItem::ResetGlobalData();
}

proofps_dd::Maps::~Maps()
{
    shutdown();
}

CConsole& proofps_dd::Maps::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* proofps_dd::Maps::getLoggerModuleName()
{
    return "Maps";
}

bool proofps_dd::Maps::initialize()
{
    m_texRed = m_gfx.getTextureManager().createFromFile( (std::string(proofps_dd::GAME_TEXTURES_DIR) + "red.bmp").c_str() );
    return true;
}

bool proofps_dd::Maps::loaded() const
{
    return ( m_blocks != NULL );
}

bool proofps_dd::Maps::load(const char* fname)
{
    getConsole().OLnOI("Maps::load(%s) ...", fname);

    if (loaded())
    {
        getConsole().EOLnOO("ERROR: %s is already loaded, should call unload first!", m_sFileName.c_str());
        return false;
    }

    // this wont be needed after we require unload() before consecutive load()
    proofps_dd::MapItem::ResetGlobalData();

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

    const TPURE_ISO_TEX_FILTERING texFilterMinOriginal = m_gfx.getTextureManager().getDefaultMinFilteringMode();
    const TPURE_ISO_TEX_FILTERING texFilterMagOriginal = m_gfx.getTextureManager().getDefaultMagFilteringMode();
    m_gfx.getTextureManager().setDefaultIsoFilteringMode(
        TPURE_ISO_TEX_FILTERING::PURE_ISO_LINEAR_MIPMAP_LINEAR,
        TPURE_ISO_TEX_FILTERING::PURE_ISO_LINEAR);

    bool bParseError = false;
    bool bMapLayoutReached = false;
    const std::streamsize nBuffSize = 1024;
    char cLine[nBuffSize];
    TPureFloat y = 4.0f;
    std::streampos fStreamPosMapLayoutStart, fStreamPosPrevLine;
    // First the map layout handling will be "dry", meaning that the map won't be actually created but
    // the number of required blocks will be counted. This is useful, because then in the next non-dry run,
    // we preallocate exactly that array size required for that number of blocks, this way no reallocation of array will be needed.
    // Since the maps are typed into text file by humans, we cannot expect them to save the exact number of blocks. :)
    while ( !bParseError && !f.eof() )
    {
        fStreamPosPrevLine = f.tellg();
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
            if (!bMapLayoutReached)
            {
                getConsole().OLn("Just reached map layout ...");
                bMapLayoutReached = true;
                fStreamPosMapLayoutStart = fStreamPosPrevLine;
            }
            bParseError = !lineHandleLayout(sLine, y, true);
        }
    };

    if (!bParseError)
    {
        getConsole().OLn(
            "Just finished dry run, now building up the map with width %u, height %u, m_blocks_h %d, m_foregroundBlocks_h %d ...",
            m_width, m_height, m_blocks_h, m_foregroundBlocks_h);
        f.clear();  // clears error states, including eof bit
        f.seekg(fStreamPosMapLayoutStart);
        y = 4.0f; // just reset this to same value as it was before the loop
        while (!bParseError && !f.eof())
        {
            f.getline(cLine, nBuffSize);
            // TODO: we should finally have a strClr() version for std::string or FINALLY UPGRADE TO NEWER CPP THAT MAYBE HAS THIS FUNCTIONALITY!!!
            PFL::strClr(cLine);
            const std::string sLine(cLine);
            if (lineShouldBeIgnored(sLine))
            {
                continue;
            }
            bParseError = !lineHandleLayout(sLine, y, false);
        }
     }
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

    getConsole().OLn("Just built up the map with m_blocks_h %d, m_foregroundBlocks_h %d ...", m_blocks_h, m_foregroundBlocks_h);

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
    m_blocksVertexPosMin.Set(m_blockPosMin.getX() - proofps_dd::GAME_BLOCK_SIZE_X / 2.f, m_blockPosMin.getY() - proofps_dd::GAME_BLOCK_SIZE_Y / 2.f, m_blockPosMin.getZ() - proofps_dd::GAME_BLOCK_SIZE_Z / 2.f);
    m_blocksVertexPosMax.Set(m_blockPosMax.getX() + proofps_dd::GAME_BLOCK_SIZE_X / 2.f, m_blockPosMax.getY() + proofps_dd::GAME_BLOCK_SIZE_Y / 2.f, m_blockPosMax.getZ() + proofps_dd::GAME_BLOCK_SIZE_Z / 2.f);

    getConsole().SOLnOO("Map loaded with width %d and height %d!", m_width, m_height);
    return true;
}

void proofps_dd::Maps::unload()
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

        free(m_foregroundBlocks);
        m_foregroundBlocks = NULL;
    }
    m_blocks_h = 0;
    m_foregroundBlocks_h = 0;

    for (auto& pairChar2RefBlockObject3D : m_mapReferenceBlockObject3Ds)
    {
        delete pairChar2RefBlockObject3D.second;
    }
    m_mapReferenceBlockObject3Ds.clear();

    for (auto& itemPair : m_items)
    {
        delete itemPair.second;
    }
    m_items.clear();
    proofps_dd::MapItem::ResetGlobalData();

    m_width = 0;
    m_height = 0;
    m_blockPosMin.SetZero();
    m_blockPosMax.SetZero();
    m_blocksVertexPosMin.SetZero();
    m_blocksVertexPosMax.SetZero();
    m_vars.clear();
    m_spawnpoints.clear();

    getConsole().OOOLn("Maps::unload() done!");
}

void proofps_dd::Maps::shutdown()
{
    getConsole().OLnOI("Maps::shutdown() ...");
    if ( m_gfx.isInitialized() )
    {
        unload();
    }
    getConsole().OOOLn("Maps::shutdown() done!");
}

unsigned int proofps_dd::Maps::width() const
{
    return m_width;
}

unsigned int proofps_dd::Maps::height() const
{
    return m_height;
}

void proofps_dd::Maps::UpdateVisibilitiesForRenderer()
{
    const PureVector campos = m_gfx.getCamera().getPosVec();

    for (int i = 0; i < m_blocks_h; i++)
    {
        PureObject3D* const obj = m_blocks[i];
        if ( obj != PGENULL )
        {
            if ( (obj->getPosVec().getX() + obj->getSizeVec().getX()/2.0f) <= campos.getX() - 9.f )
            {
                obj->SetRenderingAllowed(false);
            }
            else
            {
                if ( (obj->getPosVec().getX() - obj->getSizeVec().getX()/2.0f) >= campos.getX() + 9.f )
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

const std::string& proofps_dd::Maps::getFilename() const
{
    return m_sFileName;
}

const std::set<PureVector>& proofps_dd::Maps::getSpawnpoints() const
{
    return m_spawnpoints;
}

const PureVector& proofps_dd::Maps::getRandomSpawnpoint() const
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

const PureVector& proofps_dd::Maps::getLeftMostSpawnpoint() const
{
    if (m_spawnpoints.empty())
    {
        throw std::runtime_error("No spawnpoints!");
    }

    return m_spawnpointLeftMost;
}

const PureVector& proofps_dd::Maps::getRightMostSpawnpoint() const
{
    if (m_spawnpoints.empty())
    {
        throw std::runtime_error("No spawnpoints!");
    }

    return m_spawnpointRightMost;
}

const PureVector& proofps_dd::Maps::getBlockPosMin() const
{
    return m_blockPosMin;
}

const PureVector& proofps_dd::Maps::getBlockPosMax() const
{
    return m_blockPosMax;
}

const PureVector& proofps_dd::Maps::getBlocksVertexPosMin() const
{
    return m_blocksVertexPosMin;
}

const PureVector& proofps_dd::Maps::getBlocksVertexPosMax() const
{
    return m_blocksVertexPosMax;
}

PureObject3D** proofps_dd::Maps::getBlocks()
{
    return m_blocks;
}

PureObject3D** proofps_dd::Maps::getForegroundBlocks()
{
    return m_foregroundBlocks;
}

int proofps_dd::Maps::getBlockCount() const
{
    return m_blocks_h;
}

int proofps_dd::Maps::getForegroundBlockCount() const
{
    return m_foregroundBlocks_h;
}

const std::map<proofps_dd::MapItem::MapItemId, proofps_dd::MapItem*>& proofps_dd::Maps::getItems() const
{
    return m_items;
}

const std::map<std::string, PGEcfgVariable>& proofps_dd::Maps::getVars() const
{
    return m_vars;
}

void proofps_dd::Maps::Update()
{
    // invoked by both server and client
    for (auto& itemPair : getItems())
    {
        if (!itemPair.second)
        {
            continue;
        }

        proofps_dd::MapItem& mapItem = *(itemPair.second);
        if (mapItem.isTaken())
        {
            continue;
        }

        mapItem.Update(8.f);
    }
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


bool proofps_dd::Maps::lineShouldBeIgnored(const std::string& sLine)
{
    return sLine.empty() || (sLine[0] == '#');
}

bool proofps_dd::Maps::lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue, bool& bParseError)
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

void proofps_dd::Maps::lineHandleAssignment(std::string& sVar, std::string& sValue)
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

/**
 * This function to be invoked for every single line of the map layout definition.
 * Map layout definition is the last part of a map file, containing the blocks building up the map (walls, floor, etc.).
 * @param sLine   The current line of the map layout definition we want to process.
 * @param y       The current height we are currently placing newly created blocks for this line of the map definition layout.
 * @param bDryRun If true, blocks are not allocated thus the map is not actually created, however the following
 *                variables are actually updated: m_width, m_height, m_blocks_h, m_foregroundBlocks_h, m_items, y.
 *                If we finish processing all lines with bDryRun as true, we will know the actual size of the map and number of
 *                blocks, thus in the next non-dry run we will have to allocate memory only once for the blocks.
 *                And yes, dry run actually creates all the items in m_items.
 */
bool proofps_dd::Maps::lineHandleLayout(const std::string& sLine, TPureFloat& y, bool bDryRun)
{
    if (bDryRun)
    {
        m_height++;
        if (m_width < sLine.length())
        {
            m_width = sLine.length();
        }
    }
    else
    {
        if (m_blocks == nullptr)
        {
            // first line in non-dry run, now we can allocate memory for all blocks
            // TODO: handle memory allocation errors
            m_blocks = (PureObject3D**)malloc(m_blocks_h * sizeof(PureObject3D*));
            m_blocks_h = 0; // we are incrementing it again during non-dry run
            m_foregroundBlocks = (PureObject3D**)malloc(m_foregroundBlocks_h * sizeof(PureObject3D*));
            m_foregroundBlocks_h = 0; // we are incrementing it again during non-dry run
        }
    }

    TPureFloat x = 0.0f;
    TPureFloat maxx = x;
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

        x = x + proofps_dd::GAME_BLOCK_SIZE_X;
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
            const std::string sc(1, c); // WA for CConsole lack support of %c
            getConsole().EOLn("%s Block defined as both foreground and background: %s!", __FUNCTION__, sc.c_str());
            assert(false);
            return false;
        }

        // special background block handling
        bool bCopyPreviousBgBlock = false;
        bool bSpecialBlock = false;
        switch (c)
        {
        case '+':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_HEALTH, PureVector(x, y, GAME_PLAYERS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case 'M':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_MACHINEGUN, PureVector(x, y, GAME_PLAYERS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case 'P':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_PISTOL, PureVector(x, y, GAME_PLAYERS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case 'S':
        {
            if (!bDryRun)
            {
                // spawnpoint is background block by default
                const PureVector vecSpawnPointPos(x, y, GAME_PLAYERS_POS_Z);
                if (m_spawnpoints.empty())
                {
                    m_spawnpointLeftMost = vecSpawnPointPos;
                    m_spawnpointRightMost = vecSpawnPointPos;
                }
                else
                {
                    if (x < m_spawnpointLeftMost.getX())
                    {
                        m_spawnpointLeftMost = vecSpawnPointPos;
                    }
                    if (x > m_spawnpointRightMost.getX())
                    {
                        m_spawnpointRightMost = vecSpawnPointPos;
                    }
                }
                m_spawnpoints.insert(vecSpawnPointPos);
            }
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        default: /* NOP */;
        }

        PureObject3D* pNewBlockObj = nullptr;
        if (!bSpecialBlock || (bSpecialBlock && bCopyPreviousBgBlock))
        {
            m_blocks_h++;
            if (!bSpecialBlock && bBackground)
            {
                iObjectBgToBeCopied = m_blocks_h - 1;
            }
            if (!bDryRun)
            {
                if (bSpecialBlock && bCopyPreviousBgBlock)
                {
                    pNewBlockObj = m_gfx.getObject3DManager().createCloned(*(m_blocks[iObjectBgToBeCopied]->getReferredObject()));
                }
                else
                {
                    const auto it = m_mapReferenceBlockObject3Ds.find(c);
                    if (it == m_mapReferenceBlockObject3Ds.end())
                    {
                        m_mapReferenceBlockObject3Ds[c] = m_gfx.getObject3DManager().createBox(proofps_dd::GAME_BLOCK_SIZE_X, proofps_dd::GAME_BLOCK_SIZE_X, proofps_dd::GAME_BLOCK_SIZE_X);
                        m_mapReferenceBlockObject3Ds[c]->Hide();
                        PureTexture* tex = PGENULL;
                        if (m_Block2Texture.find(c) == m_Block2Texture.end())
                        {
                            const std::string sc(1, c); // WA for CConsole lack support of %c
                            getConsole().EOLn("%s No texture defined for block %s!", __FUNCTION__, sc.c_str());
                            tex = m_texRed;
                        }
                        else
                        {
                            const std::string sTexName = proofps_dd::GAME_TEXTURES_DIR + m_sRawName + "\\" + m_Block2Texture[c];
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
                        m_mapReferenceBlockObject3Ds[c]->getMaterial().setTexture(tex);
                    }
                    pNewBlockObj = m_gfx.getObject3DManager().createCloned(*(m_mapReferenceBlockObject3Ds[c]));
                }
                pNewBlockObj->Show();
                m_blocks[m_blocks_h - 1] = pNewBlockObj;
                m_blocks[m_blocks_h - 1]->SetLit(true);
            }

            if (bForeground)
            {
                m_foregroundBlocks_h++;
                if (!bDryRun)
                {
                    m_foregroundBlocks[m_foregroundBlocks_h - 1] = pNewBlockObj;
                }
            }
        }

        if (!pNewBlockObj)
        {
            continue;
        }

        if (bForeground)
        {
            // only foreground blocks should be checked for collision
            pNewBlockObj->SetColliding_TO_BE_REMOVED(true);
        }

        pNewBlockObj->getPosVec().Set(x, y, bBackground ? 0.0f : -proofps_dd::GAME_BLOCK_SIZE_Z);
    }  // while
    y = y - proofps_dd::GAME_BLOCK_SIZE_Y;
    return true;
}
