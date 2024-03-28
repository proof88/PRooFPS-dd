#pragma once

/*
    ###################################################################################
    Maps.h
    Map loader for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "CConsole.h"
#include "Config/PGEcfgVariable.h"
#include "Pure/include/external/PR00FsUltimateRenderingEngine.h"

#include "MapItem.h"

namespace proofps_dd
{
    class Maps
    {
    public:

        static constexpr char* szCVarSvMap = "sv_map";

        static constexpr float fMapBlockSizeWidth = 1.0f;
        static constexpr float fMapBlockSizeHeight = 1.0f;
        static constexpr float fMapBlockSizeDepth = 1.0f;

        static const char* getLoggerModuleName();

        static bool isValidMapFilename(const std::string& sFilename);

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

        const std::set<std::string>& availableMapsGet() const;
        const std::string& availableMapsGetElem(const size_t& index) const;
        const char** availableMapsGetAsCharPtrArray() const;
        const std::set<std::string>& availableMapsNoChangingGet() const;
        const std::string& availableMapsNoChangingGetElem(const size_t& index) const;

        /* Mapcycle handling */
        /* Logic is restricted to mapcycle, not impacting available maps. */

        const std::vector<std::string>& mapcycleGet() const;
        const char** mapcycleGetAsCharPtrArray() const;
        std::string mapcycleGetCurrent() const;
        std::string mapcycleNext();
        bool mapcycleSaveToFile();

        /* Available maps and Mapcycle handling together */
        /* Here we tie both together, these complex functions are recommended to be used by GUI. */

        void mapcycle_availableMaps_Synchronize();
        bool mapcycleAdd_availableMapsRemove(const std::string& sMapFilename);
        bool mapcycleAdd_availableMapsRemove(const std::vector<std::string>& vMapFilenames);
        bool mapcycleAdd_availableMapsRemove();
        bool mapcycleRemove_availableMapsAdd(const std::string& sMapFilename);
        bool mapcycleRemove_availableMapsAdd(const size_t& indexToMapcycle);
        bool mapcycleRemove_availableMapsAdd(const std::vector<std::string>& vMapFilenames);
        bool mapcycleRemove_availableMapsAdd();

    protected:

        /* Available maps handling */
        /* Logic is restricted to available maps, not impacting mapcycle. */

        void availableMapsRefresh();
        bool availableMapsAdd(const std::string& sMapFilename);
        bool availableMapsAdd(const std::vector<std::string>& vMapFilenames);
        bool availableMapsRemove(const std::string& sMapFilename);
        bool availableMapsRemove(const size_t& index);
        bool availableMapsRemove(const std::vector<std::string>& vMapFilenames);

        /* Mapcycle handling */
        /* Logic is restricted to mapcycle, not impacting available maps. */

        bool mapcycleIsCurrentLast() const;
        bool mapcycleReload();
        std::string mapcycleRewindToFirst();
        std::string mapcycleForwardToLast();
        bool mapcycleAdd(const std::string& sMapFilename);
        bool mapcycleAdd(const std::vector<std::string>& vMapFilenames);
        bool mapcycleAdd(const std::set<std::string>& vMapFilenames);
        bool mapcycleRemove(const std::string& sMapFilename);
        bool mapcycleRemove(const size_t& index);
        bool mapcycleRemove(const std::vector<std::string>& vMapFilenames);
        void mapcycleClear();
        size_t mapcycleRemoveNonExisting();

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

        std::set<std::string> m_availableMaps;
        const char** m_vszAvailableMaps;

        std::set<std::string> m_availableMapsNoChanging;
        std::string m_sEmptyStringToReturn;

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
        bool mapFilenameAddToSet_NoDuplicatesAllowed(
            const std::string& sMapFilename,
            std::set<std::string>& settt);
        bool mapFilenameRemoveFromVector(
            const std::string& sMapFilename,
            std::vector<std::string>& vec);
        bool mapFilenameRemoveFromSet(
            const std::string& sMapFilename,
            std::set<std::string>& settt);
        void availableMapsRefreshCharPtrArray();
        void mapcycleRefreshCharPtrArray();

    }; // class Maps

} // namespace proofps_dd