/*
    ###################################################################################
    Mapcycle.cpp
    Mapcycle handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>
#include <filesystem>  // requires Cpp17

#include "Mapcycle.h"
#include "PRooFPS-dd-packet.h"

static constexpr char* GAME_MAPS_MAPCYCLE = "gamedata/maps/mapcycle.txt";


// ############################### PUBLIC ################################


proofps_dd::Mapcycle::Mapcycle()
{
}

proofps_dd::Mapcycle::~Mapcycle()
{
    shutdown();
}

const char* proofps_dd::Mapcycle::getLoggerModuleName()
{
    return "Mapcycle";
}

CConsole& proofps_dd::Mapcycle::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

bool proofps_dd::Mapcycle::isValidMapFilename(const std::string& sFilename)
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
    Updates list of available maps and mapcycle content.

    @return True on success, false otherwise.
*/
bool proofps_dd::Mapcycle::initialize()
{
    if (isInitialized())
    {
        return true;
    }

    // TODO: should check return value
    mapcycle_availableMaps_Synchronize();

    m_bInitialized = true;

    return m_bInitialized;
}

bool proofps_dd::Mapcycle::isInitialized() const
{
    return m_bInitialized;
}

/**
    Releases any allocated resources related to available maps list and mapcycle.
    After calling this, initialize() can be invoked again.
*/
void proofps_dd::Mapcycle::shutdown()
{
    getConsole().OLnOI("Mapcycle::shutdown() ...");
    if (isInitialized())
    {
        /* Mapcycle handling */
        mapcycleClear();

        /* Available maps handling */
        m_availableMapsNoChanging.clear();
        delete m_vszAvailableMaps;
        m_vszAvailableMaps = nullptr;
        m_availableMaps.clear();

        m_bInitialized = false;
    }
    getConsole().OOOLn("Mapcycle::shutdown() done!");
}

const std::set<std::string>& proofps_dd::Mapcycle::availableMapsGet() const
{
    return m_availableMaps;
}

const std::string& proofps_dd::Mapcycle::availableMapsGetElem(const size_t& index) const
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
const char** proofps_dd::Mapcycle::availableMapsGetAsCharPtrArray() const
{
    return m_vszAvailableMaps;
}

const std::set<std::string>& proofps_dd::Mapcycle::availableMapsNoChangingGet() const
{
    return m_availableMapsNoChanging;
}

const std::string& proofps_dd::Mapcycle::availableMapsNoChangingGetElem(const size_t& index) const
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

const std::vector<std::string>& proofps_dd::Mapcycle::mapcycleGet() const
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
const char** proofps_dd::Mapcycle::mapcycleGetAsCharPtrArray() const
{
    return m_vszMapcycle;
}

std::string proofps_dd::Mapcycle::mapcycleGetCurrent() const
{
    return (m_mapcycleItCurrent == m_mapcycle.end()) ?
        "" :
        *m_mapcycleItCurrent;
}

bool proofps_dd::Mapcycle::mapcycleIsCurrentLast() const
{
    return ((m_mapcycleItCurrent == m_mapcycle.end()) || ((m_mapcycleItCurrent + 1) == m_mapcycle.end()));
}

bool proofps_dd::Mapcycle::mapcycleSaveToFile()
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

std::string proofps_dd::Mapcycle::mapcycleNext()
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

std::string proofps_dd::Mapcycle::mapcycleRewindToFirst()
{
    if (m_mapcycleItCurrent == m_mapcycle.end())
    {
        // no valid mapcycle
        return "";
    }
    m_mapcycleItCurrent = m_mapcycle.begin();
    return *m_mapcycleItCurrent;
}

std::string proofps_dd::Mapcycle::mapcycleForwardToLast()
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

/**
    This one is recommended during initialization, since it reloads both the mapcycle and available maps list, and
    makes sure they become disjoint sets.
*/
void proofps_dd::Mapcycle::mapcycle_availableMaps_Synchronize()
{
    availableMapsRefresh();
    mapcycleReload();
    // First we remove invalid items from mapcycle, then we remove the remanining elements from available maps.
    // Then the 2 lists are disjoint sets, and can be presented to the application/GUI.
    mapcycleRemoveNonExisting();
    availableMapsRemove(m_mapcycle);
}

bool proofps_dd::Mapcycle::mapcycleAdd_availableMapsRemove(const std::string& sMapFilename)
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

bool proofps_dd::Mapcycle::mapcycleAdd_availableMapsRemove(const std::vector<std::string>& vMapFilenames)
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

bool proofps_dd::Mapcycle::mapcycleAdd_availableMapsRemove()
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

bool proofps_dd::Mapcycle::mapcycleRemove_availableMapsAdd(const std::string& sMapFilename)
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

bool proofps_dd::Mapcycle::mapcycleRemove_availableMapsAdd(const size_t& indexToMapcycle)
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

bool proofps_dd::Mapcycle::mapcycleRemove_availableMapsAdd(const std::vector<std::string>& vMapFilenames)
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

bool proofps_dd::Mapcycle::mapcycleRemove_availableMapsAdd()
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


void proofps_dd::Mapcycle::availableMapsRefresh()
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

bool proofps_dd::Mapcycle::availableMapsAdd(const std::string& sMapFilename)
{
    const bool bRet = mapFilenameAddToSet_NoDuplicatesAllowed(sMapFilename, m_availableMaps);

    availableMapsRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Mapcycle::availableMapsAdd(const std::vector<std::string>& vMapFilenames)
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

bool proofps_dd::Mapcycle::availableMapsRemove(const std::string& sMapFilename)
{
    const bool bRet = mapFilenameRemoveFromSet(sMapFilename, m_availableMaps);

    availableMapsRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Mapcycle::availableMapsRemove(const size_t& index)
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

bool proofps_dd::Mapcycle::availableMapsRemove(const std::vector<std::string>& vMapFilenames)
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

bool proofps_dd::Mapcycle::mapcycleReload()
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

    // TODO: I do not know why I'm treating empty mapcycle as error, at least this should not be error at this level but maybe at higher game logic level!
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

bool proofps_dd::Mapcycle::mapcycleAdd(const std::string& sMapFilename)
{
    const bool bRet = mapFilenameAddToVector_NoDuplicatesAllowed(sMapFilename, m_mapcycle);
    m_mapcycleItCurrent = m_mapcycle.begin();

    mapcycleRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Mapcycle::mapcycleAdd(const std::vector<std::string>& vMapFilenames)
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

bool proofps_dd::Mapcycle::mapcycleAdd(const std::set<std::string>& vMapFilenames)
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

bool proofps_dd::Mapcycle::mapcycleRemove(const std::string& sMapFilename)
{
    const bool bRet = mapFilenameRemoveFromVector(sMapFilename, m_mapcycle);
    m_mapcycleItCurrent = m_mapcycle.begin();

    mapcycleRefreshCharPtrArray();

    return bRet;
}

bool proofps_dd::Mapcycle::mapcycleRemove(const size_t& index)
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

bool proofps_dd::Mapcycle::mapcycleRemove(const std::vector<std::string>& vMapFilenames)
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

void proofps_dd::Mapcycle::mapcycleClear()
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
size_t proofps_dd::Mapcycle::mapcycleRemoveNonExisting()
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


bool proofps_dd::Mapcycle::lineShouldBeIgnored(const std::string& sLine)
{
    return sLine.empty() || (sLine[0] == '#');
}

bool proofps_dd::Mapcycle::mapFilenameAddToVector_NoDuplicatesAllowed(const std::string& sMapFilename, std::vector<std::string>& vec)
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

bool proofps_dd::Mapcycle::mapFilenameAddToSet_NoDuplicatesAllowed(const std::string& sMapFilename, std::set<std::string>& settt)
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

bool proofps_dd::Mapcycle::mapFilenameRemoveFromVector(const std::string& sMapFilename, std::vector<std::string>& vec)
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

bool proofps_dd::Mapcycle::mapFilenameRemoveFromSet(const std::string& sMapFilename, std::set<std::string>& settt)
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

void proofps_dd::Mapcycle::availableMapsRefreshCharPtrArray()
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

void proofps_dd::Mapcycle::mapcycleRefreshCharPtrArray()
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
