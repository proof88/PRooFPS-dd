/*
    ###################################################################################
    Maps.cpp
    Map loader for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>
#include <filesystem>  // requires Cpp17

#include "Consts.h"
#include "Maps.h"
#include "PRooFPS-dd-packet.h"

static constexpr char* GAME_MAPS_DIR = "gamedata/maps/";
static constexpr char* GAME_MAPS_MAPCYCLE = "gamedata/maps/mapcycle.txt";
static const float MAPITEM_VERTICAL_ANIM_UPDATE_SPEED = 480.0f;


// ############################### PUBLIC ################################


proofps_dd::Maps::Maps(
    PGEcfgProfiles& cfgProfiles,
    PR00FsUltimateRenderingEngine& gfx) :
    m_cfgProfiles(cfgProfiles),
    m_gfx(gfx),
    m_texRed(PGENULL),
    m_blocks(NULL),
    m_blocks_h(0),
    m_foregroundBlocks(NULL),
    m_foregroundBlocks_h(0),
    m_width(0),
    m_height(0),
    m_vszAvailableMaps(nullptr),
    m_mapcycleItCurrent(m_mapcycle.end()),
    m_vszMapcycle(nullptr)
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

bool proofps_dd::Maps::isValidMapFilename(const std::string& sFilename)
{
    // we assume the given string is valid filename, so we are not checking for general filename validity,
    // such as illegal characters like '?', we just check for validity from Maps perspective.

    if (sFilename.length() >= proofps_dd::MsgMapChangeFromServer::nMapFilenameMaxLength)
    {
        return false;
    }

    if (sFilename.length() < 8 /* minimum name: map_.txt */)
    {
        return false;
    }

    if (PFL::getExtension(sFilename.c_str()) != "txt")
    {
        return false;
    }

    if (sFilename.substr(0, 4) != "map_")
    {
        return false;
    }

    return true;
}

/**
    Initializes the map handler.
    Reads the mapcycle file if it exists.
    You need to invoke this once before trying to load any map.

    @return True on success, false otherwise.
*/
bool proofps_dd::Maps::initialize()
{
    if (isInitialized())
    {
        return true;
    }

    m_texRed = m_gfx.getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "red.bmp").c_str());

    if (m_texRed)
    {
        mapcycle_availableMaps_Synchronize();
    }
    
    return m_texRed != PGENULL;
}

bool proofps_dd::Maps::isInitialized() const
{
    return m_texRed != PGENULL;
}

/**
    Shuts down the map handler.
    The currently loaded map will be also unloaded.
    After calling this, initialize() can be invoked again.
*/
void proofps_dd::Maps::shutdown()
{
    getConsole().OLnOI("Maps::shutdown() ...");
    if (isInitialized())
    {
        /* Current map handling */
        unload();
        m_sServerMapFilenameToLoad.clear();

        /* Mapcycle handling */
        mapcycleClear();

        /* Available maps handling */
        m_availableMapsNoChanging.clear();
        delete m_vszAvailableMaps;
        m_vszAvailableMaps = nullptr;
        m_availableMaps.clear();

        delete m_texRed;
        m_texRed = PGENULL;
    }
    getConsole().OOOLn("Maps::shutdown() done!");
}

const std::string& proofps_dd::Maps::serverDecideFirstMapAndUpdateNextMapToBeLoaded()
{
    // PGEcfgProfiles allows the value of a CVAR be full of spaces (it is a valid case), which means that here we should trim
    // the SV_MAP value properly since we at this level KNOW that spaces should NOT be present in this specific CVAR.
    {
        // this is ridiculous, PFL still doesnt have std::string-compatible strClr() !!!
        const std::string sOriginalMapName = m_cfgProfiles.getVars()[CVAR_SV_MAP].getAsString();
        char cLine[200]{};
        strncpy_s(cLine, sizeof(cLine), sOriginalMapName.c_str(), sOriginalMapName.length());
        PFL::strClr(cLine);
        m_cfgProfiles.getVars()[CVAR_SV_MAP].Set(cLine);
    }

    if (m_cfgProfiles.getVars()[CVAR_SV_MAP].getAsString().empty())
    {
        mapcycleRewindToFirst();
        m_sServerMapFilenameToLoad = mapcycleGetCurrent();
        if (m_sServerMapFilenameToLoad.empty())
        {
            return m_sServerMapFilenameToLoad; // error
        }
        else
        {
            getConsole().OLn("First map by mapcycle: %s", m_sServerMapFilenameToLoad.c_str());
        }
    }
    else
    {
        m_sServerMapFilenameToLoad = m_cfgProfiles.getVars()[CVAR_SV_MAP].getAsString();
        getConsole().OLn("First map by config (%s): %s", CVAR_SV_MAP, m_sServerMapFilenameToLoad.c_str());

        // we go to last map in mapcycle so that when server switches from CVAR_SV_MAP to next map, it will be the
        // first map in mapcycle, this is how it will properly start looping mapcycle.
        mapcycleForwardToLast();
    }
    return m_sServerMapFilenameToLoad;
}

const std::string& proofps_dd::Maps::getNextMapToBeLoaded() const
{
    return m_sServerMapFilenameToLoad;
}

bool proofps_dd::Maps::loaded() const
{
    return ( m_blocks != NULL );
}

bool proofps_dd::Maps::load(const char* fname, std::function<void(int)>& cbDisplayProgressUpdate)
{
    getConsole().OLnOI("Maps::load(%s) ...", fname);

    if (!isInitialized())
    {
        getConsole().EOLnOO("ERROR: map handler is not initialized!");
        return false;
    }

    if (loaded())
    {
        getConsole().EOLnOO("ERROR: %s is already loaded, should call unload first!", m_sFileName.c_str());
        return false;
    }

    cbDisplayProgressUpdate(0);

    // this wont be needed after we require unload() before consecutive load()
    proofps_dd::MapItem::ResetGlobalData();

    const std::string sFilenameWithRelativePath = std::string(GAME_MAPS_DIR) + fname;
    m_sFileName = PFL::getFilename(sFilenameWithRelativePath.c_str());
    m_sRawName = PFL::changeExtension(m_sFileName.c_str(), "");
    if (m_sRawName.empty())
    {
        getConsole().EOLnOO("ERROR: empty raw name!");
        unload();
        return false;
    }

    std::ifstream f;                                                                   
    f.open(sFilenameWithRelativePath.c_str(), std::ifstream::in);
    if ( !f.good() )
    {
        getConsole().EOLnOO("ERROR: failed to open file %s!", m_sFileName.c_str());
        unload();
        return false;
    }

    m_sServerMapFilenameToLoad = fname;

    const TPURE_ISO_TEX_FILTERING texFilterMinOriginal = m_gfx.getTextureManager().getDefaultMinFilteringMode();
    const TPURE_ISO_TEX_FILTERING texFilterMagOriginal = m_gfx.getTextureManager().getDefaultMagFilteringMode();
    m_gfx.getTextureManager().setDefaultIsoFilteringMode(
        TPURE_ISO_TEX_FILTERING::PURE_ISO_LINEAR_MIPMAP_LINEAR,
        TPURE_ISO_TEX_FILTERING::PURE_ISO_LINEAR);

    bool bParseError = false;
    bool bMapLayoutReached = false;
    constexpr std::streamsize nBuffSize = 1024;
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

    // non-dry-run starts, this is the slowest part of map loading, thus we invoke cbDisplayProgressUpdate in this block
    if (!bParseError)
    {
        getConsole().OLn(
            "Just finished dry run, now building up the map with width %u, height %u, m_blocks_h %d, m_foregroundBlocks_h %d ...",
            m_width, m_height, m_blocks_h, m_foregroundBlocks_h);
        
        // During dry-run above, m_blocks_h and m_foregroundBlocks_h had been properly set, but they will be reset and counted again now
        // during the non-dry-run, so save m_blocks_h now to calculate loading progress
        const int nTotalBlocks_h = m_blocks_h;
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
            cbDisplayProgressUpdate(nTotalBlocks_h == 0 ? 0 : static_cast<int>((m_blocks_h / static_cast<float>(nTotalBlocks_h)) * 100));
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
    m_blocksVertexPosMin.Set(
        m_blockPosMin.getX() - proofps_dd::GAME_BLOCK_SIZE_X / 2.f,
        m_blockPosMin.getY() - proofps_dd::GAME_BLOCK_SIZE_Y / 2.f,
        m_blockPosMin.getZ() - proofps_dd::GAME_BLOCK_SIZE_Z / 2.f);
    m_blocksVertexPosMax.Set(
        m_blockPosMax.getX() + proofps_dd::GAME_BLOCK_SIZE_X / 2.f,
        m_blockPosMax.getY() + proofps_dd::GAME_BLOCK_SIZE_Y / 2.f,
        m_blockPosMax.getZ() + proofps_dd::GAME_BLOCK_SIZE_Z / 2.f);

    getConsole().SOLnOO("> Map loaded with width %u and height %u!", m_width, m_height);
    return true;
}

void proofps_dd::Maps::unload()
{
    getConsole().OLnOI("Maps::unload() ...");
    m_sServerMapFilenameToLoad.clear();
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
    const PureVector& campos = m_gfx.getCamera().getPosVec();

    for (int i = 0; i < m_blocks_h; i++)
    {
        PureObject3D* const obj = m_blocks[i];
        if ( obj != PGENULL )
        {
            if ( (obj->getPosVec().getX() + obj->getSizeVec().getX()/2.0f) <= campos.getX() - 13.f )
            {
                obj->SetRenderingAllowed(false);
            }
            else
            {
                if ( (obj->getPosVec().getX() - obj->getSizeVec().getX()/2.0f) >= campos.getX() + 13.f )
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

/**
    Retrieves the currently loaded map filename.

    @return Filename of the currently loaded map. Empty string if no map is loaded currently.
*/
const std::string& proofps_dd::Maps::getFilename() const
{
    return m_sFileName;
}


/**
    Retrieves the set of spawnpoints of the currently loaded map.
    A spawnpoint is a 3D coordinate where the player can spawn at.

    @return The set of spawnpoints of the currently loaded map.
*/
const std::set<PureVector>& proofps_dd::Maps::getSpawnpoints() const
{
    return m_spawnpoints;
}


/**
    Retrieves a randomly selected spawnpoint from the set of spawnpoints of the currently loaded map.
    A spawnpoint is a 3D coordinate where the player can spawn at.

    @return A randomly selected spawnpoint on the current map.
*/
const PureVector& proofps_dd::Maps::getRandomSpawnpoint() const
{
    if ( m_spawnpoints.empty() )
    {
        throw std::runtime_error("No spawnpoints!");
    }
    const int iElem = PFL::random(0, m_spawnpoints.size()-1);
    //getConsole().EOLn("Maps::%s(): %d", __func__, iElem);
    auto it = m_spawnpoints.begin();
    std::advance(it, iElem);

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

void proofps_dd::Maps::Update(const float& fps)
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

        mapItem.Update(MAPITEM_VERTICAL_ANIM_UPDATE_SPEED / fps);
    }
}

const std::set<std::string>& proofps_dd::Maps::availableMapsGet() const
{
    return m_availableMaps;
}

const std::string& proofps_dd::Maps::availableMapsGetElem(const size_t& index) const
{
    if (index >= m_availableMaps.size())
    {
        getConsole().EOLn("ERROR: %s invalid index: %u!", __func__, index);
        return m_sEmptyStringToReturn;
    }

    auto it = m_availableMaps.begin();
    std::advance(it, index);
    return *it;
}

/**
    This is convenience function to be used with GUI: Dear ImGUI requires this kind of array
    as source of items of Listbox. It is better if Maps provide it so we can avoid inconsistency.
    The elements are the same as in the availableMapsGet() container.

    @return A char array with same size as the availableMapsGet() container, where all elements point to the corresponding
            null-terminated string of the string object of the availableMapsGet() container.
            Null when size of availableMapsGet() container is 0.
*/
const char** proofps_dd::Maps::availableMapsGetAsCharPtrArray() const
{
    return m_vszAvailableMaps;
}

const std::set<std::string>& proofps_dd::Maps::availableMapsNoChangingGet() const
{
    return m_availableMapsNoChanging;
}

const std::string& proofps_dd::Maps::availableMapsNoChangingGetElem(const size_t& index) const
{
    if (index >= m_availableMapsNoChanging.size())
    {
        getConsole().EOLn("ERROR: %s invalid index: %u!", __func__, index);
        return m_sEmptyStringToReturn;
    }

    auto it = m_availableMapsNoChanging.begin();
    std::advance(it, index);
    return *it;
}

const std::vector<std::string>& proofps_dd::Maps::mapcycleGet() const
{
    return m_mapcycle;
}

/**
    This is convenience function to be used with GUI: Dear ImGUI requires this kind of array
    as source of items of Listbox. It is better if Maps provide it so we can avoid inconsistency.
    The elements are the same as in the mapcycleGet() container.

    @return A char array with same size as the mapcycleGet() container, where all elements point to the corresponding
            null-terminated string of the string object of the mapcycleGet() container.
            Null when size of mapcycleGet() container is 0.
*/
const char** proofps_dd::Maps::mapcycleGetAsCharPtrArray() const
{
    return m_vszMapcycle;
}

std::string proofps_dd::Maps::mapcycleGetCurrent() const
{
    return (m_mapcycleItCurrent == m_mapcycle.end()) ?
        "" :
        *m_mapcycleItCurrent;
}

bool proofps_dd::Maps::mapcycleSaveToFile()
{
    std::ofstream f;
    f.open(GAME_MAPS_MAPCYCLE, std::ofstream::out);
    if (!f.good())
    {
        getConsole().EOLn("%s ERROR: failed to open file %s!", __func__, GAME_MAPS_MAPCYCLE);
        return false;
    }

    f << "# Do not put comment lines into this file like this one, the game is not preserving them!" << std::endl << std::endl;

    bool bRet = f.good();
    for (const auto& sMapcycleItem : m_mapcycle)
    {
        f << sMapcycleItem << std::endl;
        if (!f.good())
        {
            getConsole().EOLn("%s ERROR: failed to write file: %s, might be partially saved and corrupted!", __func__, GAME_MAPS_MAPCYCLE);
            bRet = false;
            break;
        }
    }
    f.close();

    if (bRet)
    {
        getConsole().SOLn("%s done!", __func__);
    }

    return bRet;
} // mapcycleSaveToFile()

std::string proofps_dd::Maps::mapcycleNext()
{
    if (m_mapcycleItCurrent == m_mapcycle.end())
    {
        // no valid mapcycle
        return "";
    }

    ++m_mapcycleItCurrent;
    if (m_mapcycleItCurrent == m_mapcycle.end())
    {
        // with valid mapcycle, it never stays on end(), it should automatically go back to the beginning
        m_mapcycleItCurrent = m_mapcycle.begin();
    }
    return *m_mapcycleItCurrent;
}

/**
    This one is recommended during initialization, since it reloads both the mapcycle and available maps list, and
    makes sure they become disjoint sets.
*/
void proofps_dd::Maps::mapcycle_availableMaps_Synchronize()
{
    availableMapsRefresh();
    mapcycleReload();
    // First we remove invalid items from mapcycle, then we remove the remanining elements from available maps.
    // Then the 2 lists are disjoint sets, and can be presented to the application/GUI.
    mapcycleRemoveNonExisting();
    availableMapsRemove(m_mapcycle);
}

bool proofps_dd::Maps::mapcycleAdd_availableMapsRemove(const std::string& sMapFilename)
{
    bool bRet = mapcycleAdd(sMapFilename);
    if (bRet)
    {
        bRet = availableMapsRemove(sMapFilename);
        if (!bRet)
        {
            getConsole().EOLn("ERROR: %s: availableMapsRemove failed!", __func__);
        }
    }
    else
    {
        getConsole().EOLn("ERROR: %s: mapcycleAdd failed!", __func__);
    }

    return bRet;
}

bool proofps_dd::Maps::mapcycleAdd_availableMapsRemove(const std::vector<std::string>& vMapFilenames)
{
    bool bRet = mapcycleAdd(vMapFilenames);
    if (bRet)
    {
        bRet = availableMapsRemove(vMapFilenames);
        if (!bRet)
        {
            getConsole().EOLn("ERROR: %s: availableMapsRemove failed!", __func__);
        }
    }
    else
    {
        getConsole().EOLn("ERROR: %s: mapcycleAdd failed!", __func__);
    }

    return bRet;
}

bool proofps_dd::Maps::mapcycleAdd_availableMapsRemove()
{
    bool bRet = mapcycleAdd(m_availableMaps);
    if (bRet)
    {
        m_availableMaps.clear();
        availableMapsRefreshCharPtrArray();
    }
    else
    {
        getConsole().EOLn("ERROR: %s: mapcycleAdd failed!", __func__);
    }

    return bRet;
}

bool proofps_dd::Maps::mapcycleRemove_availableMapsAdd(const std::string& sMapFilename)
{
    bool bRet = availableMapsAdd(sMapFilename);
    if (bRet)                                                     
    {
        bRet = mapcycleRemove(sMapFilename);
        if (!bRet)
        {
            getConsole().EOLn("ERROR: %s: mapcycleRemove failed!", __func__);
        }
    }
    else
    {
        getConsole().EOLn("ERROR: %s: availableMapsAdd failed!", __func__);
    }

    return bRet;
}

bool proofps_dd::Maps::mapcycleRemove_availableMapsAdd(const size_t& indexToMapcycle)
{
    const std::string sRemoved = (indexToMapcycle < mapcycleGet().size()) ? mapcycleGet()[indexToMapcycle] : "";
    bool bRet = availableMapsAdd(sRemoved);
    if (bRet)
    {
        bRet = mapcycleRemove(indexToMapcycle);
        if (!bRet)
        {
            getConsole().EOLn("ERROR: %s: mapcycleRemove failed!", __func__);
        }
    }
    else
    {
        getConsole().EOLn("ERROR: %s: availableMapsAdd failed!", __func__);
    }

    return bRet;
}

bool proofps_dd::Maps::mapcycleRemove_availableMapsAdd(const std::vector<std::string>& vMapFilenames)
{
    bool bRet = availableMapsAdd(vMapFilenames);
    if (bRet)
    {
        bRet = mapcycleRemove(vMapFilenames);
        if (!bRet)
        {
            getConsole().EOLn("ERROR: %s: mapcycleRemove failed!", __func__);
        }
    }
    else
    {
        getConsole().EOLn("ERROR: %s: availableMapsAdd failed!", __func__);
    }

    return bRet;
}

bool proofps_dd::Maps::mapcycleRemove_availableMapsAdd()
{
    bool bRet = availableMapsAdd(m_mapcycle);
    if (bRet)
    {
        m_mapcycle.clear();
        m_mapcycleItCurrent = m_mapcycle.end();
        mapcycleRefreshCharPtrArray();
    }
    else
    {
        getConsole().EOLn("ERROR: %s: mapcycleAdd failed!", __func__);
    }

    return bRet;
}


// ############################## PROTECTED ##############################


void proofps_dd::Maps::availableMapsRefresh()
{
    delete m_vszAvailableMaps;
    m_vszAvailableMaps = nullptr;
    m_availableMaps.clear();
    m_availableMapsNoChanging.clear();

    for (const auto& fileEntry : std::filesystem::directory_iterator(GAME_MAPS_DIR))
    {
        //getConsole().OLn("PRooFPSddPGE::%s(): %s!", __func__, fileEntry.path().filename().string().c_str());
        if (!isValidMapFilename(fileEntry.path().filename().string()))
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): skip filename: %s!", __func__, fileEntry.path().string().c_str());
            continue;
        }

        m_availableMaps.insert(fileEntry.path().filename().string() /*PFL::getFilename(fname)*/);
    }
    m_availableMapsNoChanging = m_availableMaps;

    availableMapsRefreshCharPtrArray();
}

bool proofps_dd::Maps::availableMapsAdd(const std::string& sMapFilename)
{
    const bool bRet = mapFilenameAddToSet_NoDuplicatesAllowed(sMapFilename, m_availableMaps);

    availableMapsRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Maps::availableMapsAdd(const std::vector<std::string>& vMapFilenames)
{
    if (vMapFilenames.empty())
    {
        //getConsole().EOLn("ERROR: %s empty filename!", __func__);
        return true;
    }

    bool bRet = true;
    for (const auto& sMapFilename : vMapFilenames)
    {
        bRet &= mapFilenameAddToSet_NoDuplicatesAllowed(sMapFilename, m_availableMaps);
    }

    availableMapsRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Maps::availableMapsRemove(const std::string& sMapFilename)
{
    const bool bRet = mapFilenameRemoveFromSet(sMapFilename, m_availableMaps);

    availableMapsRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Maps::availableMapsRemove(const size_t& index)
{
    if (index >= m_availableMaps.size())
    {
        getConsole().EOLn("ERROR: %s invalid index: %u!", __func__, index);
        return false;
    }

    // std::vector solution:
    //m_availableMaps.erase(m_availableMaps.begin() + index);

    auto it = m_availableMaps.begin();
    std::advance(it, index);
    m_availableMaps.erase(it);

    availableMapsRefreshCharPtrArray();

    return true;
}

bool proofps_dd::Maps::availableMapsRemove(const std::vector<std::string>& vMapFilenames)
{
    if (vMapFilenames.empty())
    {
        //getConsole().EOLn("ERROR: %s empty filename!", __func__);
        return true;
    }

    bool bRet = true;
    for (const auto& sMapFilename : vMapFilenames)
    {
        bRet &= mapFilenameRemoveFromSet(sMapFilename, m_availableMaps);
    }

    availableMapsRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Maps::mapcycleIsCurrentLast() const
{
    return ((m_mapcycleItCurrent == m_mapcycle.end()) || ((m_mapcycleItCurrent + 1) == m_mapcycle.end()));
}

bool proofps_dd::Maps::mapcycleReload()
{
    getConsole().OLnOI("Maps::mapcycleReload(%s) ...", GAME_MAPS_MAPCYCLE);

    m_mapcycle.clear();
    m_mapcycleItCurrent = m_mapcycle.end();
    delete m_vszMapcycle;
    m_vszMapcycle = nullptr;

    std::ifstream f;
    f.open(GAME_MAPS_MAPCYCLE, std::ifstream::in);
    if (!f.good())
    {
        getConsole().EOLnOO("ERROR: failed to open file %s!", GAME_MAPS_MAPCYCLE);
        return false;
    }

    bool bParseError = false;
    constexpr std::streamsize nBuffSize = 200;
    char cLine[nBuffSize];
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
        bParseError = sLine.find(' ') != std::string::npos;  // TODO: this should be a better check for VALID filenames
        if (!bParseError)
        {
            m_mapcycle.push_back(sLine);
        }
    }

    f.close();
    if (bParseError)
    {
        getConsole().EOLnOO("ERROR: failed to parse file: %s!", GAME_MAPS_MAPCYCLE);
        m_mapcycle.clear();
        return false;
    }

    if (m_mapcycle.empty())
    {
        getConsole().EOLnOO("ERROR: mapcycle file %s parsed as empty!", GAME_MAPS_MAPCYCLE);
        return false;
    }

    m_mapcycleItCurrent = m_mapcycle.begin();

    mapcycleRefreshCharPtrArray();

    getConsole().SOLnOO("> Mapcycle loaded with %u maps!", m_mapcycle.size());
    return true;
}  // mapcycleReload()

std::string proofps_dd::Maps::mapcycleRewindToFirst()
{
    if (m_mapcycleItCurrent == m_mapcycle.end())
    {
        // no valid mapcycle
        return "";
    }
    m_mapcycleItCurrent = m_mapcycle.begin();
    return *m_mapcycleItCurrent;
}

std::string proofps_dd::Maps::mapcycleForwardToLast()
{
    if (m_mapcycleItCurrent == m_mapcycle.end())
    {
        // no valid mapcycle
        return "";
    }

    m_mapcycleItCurrent = m_mapcycle.end();
    --m_mapcycleItCurrent;
    return *m_mapcycleItCurrent;
}

bool proofps_dd::Maps::mapcycleAdd(const std::string& sMapFilename)
{
    const bool bRet = mapFilenameAddToVector_NoDuplicatesAllowed(sMapFilename, m_mapcycle);
    m_mapcycleItCurrent = m_mapcycle.begin();

    mapcycleRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Maps::mapcycleAdd(const std::vector<std::string>& vMapFilenames)
{
    if (vMapFilenames.empty())
    {
        //getConsole().EOLn("ERROR: %s empty vector!", __func__);
        return true;
    }

    bool bRet = true;
    for (const auto& sMapFilename : vMapFilenames)
    {
        bRet &= mapFilenameAddToVector_NoDuplicatesAllowed(sMapFilename, m_mapcycle);
    }

    m_mapcycleItCurrent = m_mapcycle.begin();

    mapcycleRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Maps::mapcycleAdd(const std::set<std::string>& vMapFilenames)
{
    if (vMapFilenames.empty())
    {
        //getConsole().EOLn("ERROR: %s empty vector!", __func__);
        return true;
    }

    bool bRet = true;
    for (const auto& sMapFilename : vMapFilenames)
    {
        bRet &= mapFilenameAddToVector_NoDuplicatesAllowed(sMapFilename, m_mapcycle);
    }

    m_mapcycleItCurrent = m_mapcycle.begin();

    mapcycleRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Maps::mapcycleRemove(const std::string& sMapFilename)
{
    const bool bRet = mapFilenameRemoveFromVector(sMapFilename, m_mapcycle);
    m_mapcycleItCurrent = m_mapcycle.begin();

    mapcycleRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Maps::mapcycleRemove(const size_t& index)
{
    if (index >= m_mapcycle.size())
    {
        getConsole().EOLn("ERROR: %s invalid index: %u!", __func__, index);
        return false;
    }

    m_mapcycle.erase(m_mapcycle.begin() + index);
    m_mapcycleItCurrent = m_mapcycle.begin();

    mapcycleRefreshCharPtrArray();

    return true;
}

bool proofps_dd::Maps::mapcycleRemove(const std::vector<std::string>& vMapFilenames)
{
    if (vMapFilenames.empty())
    {
        //getConsole().EOLn("ERROR: %s empty vector!", __func__);
        return true;
    }

    bool bRet = true;
    for (const auto& sMapFilename : vMapFilenames)
    {
        bRet &= mapFilenameRemoveFromVector(sMapFilename, m_mapcycle);
    }

    m_mapcycleItCurrent = m_mapcycle.begin();

    mapcycleRefreshCharPtrArray();

    return bRet;
}

void proofps_dd::Maps::mapcycleClear()
{
    delete m_vszMapcycle;
    m_vszMapcycle = nullptr;
    m_mapcycle.clear();
    m_mapcycleItCurrent = m_mapcycle.end();
}

/**
    Removes items from mapcycle that are referring to non-existing files in the current filesystem.

    It can happen that someone manually edits the mapcycle file, or someone
    copies another mapcycle file over our file, leading to having entries that are invalid
    in the current filesystem. So this function is making sure the mapcycle is
    valid.

    It uses availableMaps so it is recommended to invoke availableMapsRefresh() first!
    Make sure you never invoke this function without calling availableMapsRefresh() first, otherwise
    even the on-disk mapcycle items will be also removed from mapcycle.

    @return The number of removed items.
*/
size_t proofps_dd::Maps::mapcycleRemoveNonExisting()
{
    size_t nDeleted = 0;
    bool bFound;
    auto itMapcycle = m_mapcycle.begin();
    while (itMapcycle != m_mapcycle.end())
    {
        bFound = std::find(m_availableMaps.begin(), m_availableMaps.end(), *itMapcycle) != m_availableMaps.end();
        if (!bFound)
        {
            getConsole().OLn("%s Warning: removed: %s!", __func__, (*itMapcycle).c_str());
            itMapcycle = m_mapcycle.erase(itMapcycle);
            nDeleted++;
        }
        else
        {
            itMapcycle++;
        }
    }

    if (nDeleted > 0)
    {
        m_mapcycleItCurrent = m_mapcycle.begin();
    }

    return nDeleted;
}


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
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_HEALTH, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '2':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_PISTOL, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '3':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_MACHINEGUN, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '4':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_BAZOOKA, PureVector(x, y, GAME_ITEMS_POS_Z));
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
            break;
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
                // dont need to show, can stay hidden, since main game loop invokes UpdateVisibilitiesForRenderer() anyway which shows what is needed
                //pNewBlockObj->Show();
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
}  // lineHandleLayout()

bool proofps_dd::Maps::mapFilenameAddToVector_NoDuplicatesAllowed(const std::string& sMapFilename, std::vector<std::string>& vec)
{
    if (sMapFilename.empty())
    {
        //getConsole().EOLn("ERROR: %s empty filename!", __func__);
        return false;
    }

    auto itFound = std::find(vec.begin(), vec.end(), sMapFilename);
    if (itFound != vec.end())
    {
        getConsole().EOLn("ERROR: %s filename %s already present!", __func__, sMapFilename.c_str());
        return false; // maybe this could be success too
    }

    vec.push_back(sMapFilename);
    return true;
}

bool proofps_dd::Maps::mapFilenameAddToSet_NoDuplicatesAllowed(const std::string& sMapFilename, std::set<std::string>& settt)
{
    if (sMapFilename.empty())
    {
        //getConsole().EOLn("ERROR: %s empty filename!", __func__);
        return false;
    }

    // std::set provides item uniqueness
    const auto pairInserted = settt.insert(sMapFilename);
    if (!pairInserted.second)
    {
        getConsole().EOLn("ERROR: %s filename %s already present!", __func__, sMapFilename.c_str());
        return false; // maybe this could be success too
    }

    return true;
}

bool proofps_dd::Maps::mapFilenameRemoveFromVector(const std::string& sMapFilename, std::vector<std::string>& vec)
{
    if (sMapFilename.empty())
    {
        getConsole().EOLn("ERROR: %s empty filename!", __func__);
        return false;
    }

    auto itFound = std::find(vec.begin(), vec.end(), sMapFilename);
    if (itFound == vec.end())
    {
        getConsole().EOLn("ERROR: %s filename %s was not found!", __func__, sMapFilename.c_str());
        return false; // maybe this could be success too
    }

    vec.erase(itFound);
    return true;
}

bool proofps_dd::Maps::mapFilenameRemoveFromSet(const std::string& sMapFilename, std::set<std::string>& settt)
{
    if (sMapFilename.empty())
    {
        getConsole().EOLn("ERROR: %s empty filename!", __func__);
        return false;
    }

    const size_t nErased = settt.erase(sMapFilename);
    if (nErased == 0)
    {
        getConsole().EOLn("ERROR: %s filename %s was not found!", __func__, sMapFilename.c_str());
        return false; // maybe this could be success too
    }
    return true;
}

void proofps_dd::Maps::availableMapsRefreshCharPtrArray()
{
    delete m_vszAvailableMaps;
    m_vszAvailableMaps = nullptr;

    const size_t nArraySize = m_availableMaps.size(); // this way static analyzer won't warn about possible buffer overrun
    if (nArraySize > 0)
    {
        m_vszAvailableMaps = new const char* [nArraySize];
        auto it = m_availableMaps.begin();
        for (size_t i = 0; i < nArraySize; i++)
        {
            m_vszAvailableMaps[i] = it->c_str();
            it++;
        }
    }
}

void proofps_dd::Maps::mapcycleRefreshCharPtrArray()
{
    delete m_vszMapcycle;
    m_vszMapcycle = nullptr;

    const size_t nArraySize = m_mapcycle.size(); // this way static analyzer won't warn about possible buffer overrun
    if (nArraySize > 0)
    {
        m_vszMapcycle = new const char* [nArraySize];
        for (size_t i = 0; i < nArraySize; i++)
        {
            m_vszMapcycle[i] = m_mapcycle[i].c_str();
        }
    }
}
