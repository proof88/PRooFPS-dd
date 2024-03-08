#pragma once

/*
    ###################################################################################
    Maps.h
    Map loader for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../../Console/CConsole/src/CConsole.h"
#include "Config/PGEcfgVariable.h"
#include "Pure/include/external/PR00FsUltimateRenderingEngine.h"

#include "MapItem.h"

namespace proofps_dd
{

    class Maps
    {
    public:

        static constexpr char* CVAR_SV_MAP = "sv_map";

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        Maps(
            PGEcfgProfiles& cfgProfiles,
            PR00FsUltimateRenderingEngine& gfx);
        virtual ~Maps();

        Maps(const Maps&) = delete;
        Maps& operator=(const Maps&) = delete;
        Maps(Maps&&) = delete;
        Maps&& operator=(Maps&&) = delete;

        bool initialize();                                   /**< Initializes the map handler. */
        bool isInitialized() const;
        void shutdown();                                     /**< Shuts down the map handler. */
        
        const std::string& serverDecideFirstMapAndUpdateNextMapToBeLoaded();
        const std::string& getNextMapToBeLoaded() const;

        /* Current map handling */

        bool loaded() const;
        bool load(
            const char* fname,
            std::function<void(int)>& cbDisplayProgressUpdate);
        void unload();
        unsigned int width() const;
        unsigned int height() const;
        void UpdateVisibilitiesForRenderer();
        const std::string& getFilename() const;              /**< Retrieves the currently loaded map filename. */
        const std::set<PureVector>& getSpawnpoints() const;  /**< Retrieves the set of spawnpoints of the currently loaded map. */
        const PureVector& getRandomSpawnpoint() const;       /**< Retrieves a randomly selected spawnpoint from the set of spawnpoints of the currently loaded map. */
        const PureVector& getLeftMostSpawnpoint() const;
        const PureVector& getRightMostSpawnpoint() const;
        const PureVector& getBlockPosMin() const;
        const PureVector& getBlockPosMax() const;
        const PureVector& getBlocksVertexPosMin() const;
        const PureVector& getBlocksVertexPosMax() const;
        PureObject3D** getBlocks(); // TODO: not nice access
        PureObject3D** getForegroundBlocks(); // TODO: not nice access
        int getBlockCount() const;
        int getForegroundBlockCount() const;
        const std::map<MapItem::MapItemId, MapItem*>& getItems() const;
        const std::map<std::string, PGEcfgVariable>& getVars() const;
        void Update(const float& fps);

        /* Available maps handling */
        /* Logic is restricted to available maps, not impacting mapcycle. */

        void availableMapsRefresh();
        const std::vector<std::string>& availableMapsGet() const;
        const char** availableMapsGetAsCharPtrArray() const;
        bool availableMapsAdd(const std::string& sMapFilename);
        bool availableMapsAdd(const std::vector<std::string>& vMapFilenames);
        bool availableMapsRemove(const std::string& sMapFilename);
        bool availableMapsRemove(const size_t& index);
        bool availableMapsRemove(const std::vector<std::string>& vMapFilenames);

        /* Mapcycle handling */
        /* Logic is restricted to mapcycle, not impacting available maps. */
        
        const std::vector<std::string>& mapcycleGet() const;
        const char** mapcycleGetAsCharPtrArray() const;
        std::string mapcycleGetCurrent() const;
        bool mapcycleIsCurrentLast() const;
        bool mapcycleReload();
        std::string mapcycleNext();
        std::string mapcycleRewindToFirst();
        std::string mapcycleForwardToLast();
        bool mapcycleAdd(const std::string& sMapFilename);
        bool mapcycleAdd(const std::vector<std::string>& vMapFilenames);
        bool mapcycleRemove(const std::string& sMapFilename);
        bool mapcycleRemove(const size_t& index);
        bool mapcycleRemove(const std::vector<std::string>& vMapFilenames);
        void mapcycleClear();
        size_t mapcycleRemoveNonExisting();

        /* Available maps and Mapcycle handling together */
        /* Here we tie both together, these complex functions are recommended to be used by GUI. */

        bool mapcycleAdd_availableMapsRemove(const std::string& sMapFilename);
        bool mapcycleAdd_availableMapsRemove(const std::vector<std::string>& vMapFilenames);
        bool mapcycleRemove_availableMapsAdd(const std::string& sMapFilename);
        bool mapcycleRemove_availableMapsAdd(const size_t& indexToMapcycle);
        bool mapcycleRemove_availableMapsAdd(const std::vector<std::string>& vMapFilenames);


    protected:

    private:

        const float GAME_PLAYERS_POS_Z = -1.2f;
        const float GAME_ITEMS_POS_Z = GAME_PLAYERS_POS_Z + 0.1f;  // avoid Z-fighting with items the player cannot take

        const std::set<char> foregroundBlocks = {
            'B', 'D', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'Q', 'T'
        };

        const std::set<char> backgroundBlocks = {
            'a', 'c', 'e', 'm', 'n', 'p', 'o', 'r', 'u', 'v', 'w', 'x', 'y', 'z',
            /* the special foreground stuff (e.g. items) are treated as background blocks too, see special handling in lineHandleLayout(): */
            '+', '2', '3', '4', 'S'
        };

        PGEcfgProfiles& m_cfgProfiles;
        PR00FsUltimateRenderingEngine& m_gfx;
        PureTexture* m_texRed;  // TODO: unique_ptr
        std::string m_sServerMapFilenameToLoad;                      /**< We set this as soon as we get to know which map we should load. */

        /* Current map handling */

        std::map<char, PureObject3D*> m_mapReferenceBlockObject3Ds;

        PureObject3D** m_blocks; // TODO: not nice, in future we switch to cpp container
        int m_blocks_h;

        PureObject3D** m_foregroundBlocks;
        int m_foregroundBlocks_h;

        std::map<std::string, PGEcfgVariable> m_vars;
        std::string m_sRawName;
        std::string m_sFileName;
        std::map<char, std::string> m_Block2Texture;
        std::set<PureVector> m_spawnpoints;
        PureVector m_blocksVertexPosMin, m_blocksVertexPosMax;
        PureVector m_blockPosMin, m_blockPosMax;
        PureVector m_spawnpointLeftMost, m_spawnpointRightMost;
        unsigned int m_width, m_height;
        std::map<MapItem::MapItemId, MapItem*> m_items;

        /* Available maps handling */

        // here an std::set would be a more proper choice since available maps is basically built up from the filesystem,
        // and all items are expected to be unique, and actually we would like them to be sorted.
        // Probably soon I will switch to set.
        std::vector<std::string> m_availableMaps;
        const char** m_vszAvailableMaps;

        /* Mapcycle handling */

        // I want unique elements in mapcycle. Yes, std::set would be trivial, but:
        // - I don't want elements to be sorted;
        // - I'm not sure if uniqueness will be forever, maybe I want to allow repeating maps!
        // Yes I know about std::unordered_multiset, but for some reason I wanted to stick for vector for now.
        std::vector<std::string> m_mapcycle;
        std::vector<std::string>::iterator m_mapcycleItCurrent;
        const char** m_vszMapcycle;

        // ---------------------------------------------------------------------------

        static bool lineShouldBeIgnored(const std::string& sLine);
        static bool lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue, bool& bParseError);

        void lineHandleAssignment(std::string& sVar, std::string& sValue);
        bool lineHandleLayout(const std::string& sLine, TPureFloat& y, bool bDryRun);

        bool mapFilenameAddToVector_NoDuplicatesAllowed(
            const std::string& sMapFilename,
            std::vector<std::string>& vec);
        bool mapFilenameRemoveFromVector(
            const std::string& sMapFilename,
            std::vector<std::string>& vec);
        void availableMapsRefreshCharPtrArray();
        void mapcycleRefreshCharPtrArray();

    }; // class Maps

} // namespace proofps_dd