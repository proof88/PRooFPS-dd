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

        void refreshAvailableMaps();
        const std::vector<std::string>& getAvailableMaps() const;

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

        /* Mapcycle handling */
        
        const std::vector<std::string>& mapcycleGet() const;
        std::string mapcycleGetCurrent() const;
        bool mapcycleIsCurrentLast() const;
        bool mapcycleReload();
        std::string mapcycleNext();
        std::string mapcycleRewindToFirst();
        std::string mapcycleForwardToLast();

    protected:

    private:

        const float GAME_PLAYERS_POS_Z = -1.2f;
        const float GAME_ITEMS_POS_Z = GAME_PLAYERS_POS_Z + 0.1f;

        const std::set<char> foregroundBlocks = {
            'B', 'D', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'Q', 'T'
        };

        const std::set<char> backgroundBlocks = {
            'a', 'c', 'e', 'n', 'n', 'o', 'r', 'v', 'u', 'w', 'x', 'y',
            /* the special foreground stuff (e.g. items) are treated as background blocks too, see special handling in lineHandleLayout(): */
            '+', 'M', 'P', 'Z', 'S'
        };

        PGEcfgProfiles& m_cfgProfiles;
        PR00FsUltimateRenderingEngine& m_gfx;
        PureTexture* m_texRed;  // TODO: unique_ptr
        std::string m_sServerMapFilenameToLoad;                      /**< We set this as soon as we get to know which map we should load. */

        std::vector<std::string> m_availableMaps;

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

        /* Mapcycle handling */

        std::vector<std::string> m_mapcycle;
        std::vector<std::string>::iterator m_mapcycleItCurrent;

        // ---------------------------------------------------------------------------

        static bool lineShouldBeIgnored(const std::string& sLine);
        static bool lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue, bool& bParseError);

        void lineHandleAssignment(std::string& sVar, std::string& sValue);
        bool lineHandleLayout(const std::string& sLine, TPureFloat& y, bool bDryRun);

    }; // class Maps

} // namespace proofps_dd