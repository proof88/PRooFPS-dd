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

#include <map>
#include <string>

#include "../../../CConsole/CConsole/src/CConsole.h"
#include "../../../PGE/PGE/PGEcfgVariable.h"
#include "../../../PGE/PGE/PRRE/include/external/PR00FsReducedRenderingEngine.h"

class Maps
{
public:

    static const char* getLoggerModuleName();
    
    // ---------------------------------------------------------------------------

    CConsole& getConsole() const;
   
    Maps(PR00FsReducedRenderingEngine& gfx);
    virtual ~Maps();

    bool initialize();
    bool loaded() const;
    bool load(const char* fname);
    void unload();
    void shutdown();
    unsigned int width() const;
    unsigned int height() const;
    void updateVisibilitiesForRenderer();
    PRREVector& getStartPos();
    PRREVector& getEndPos();
    float getObjectsMinY() const;
    std::vector<PRREVector>& getCandleLights();
    const std::map<std::string, PGEcfgVariable>& getVars() const;

protected:

    Maps(const Maps&) :
        m_gfx(m_gfx)
    {}

    Maps& operator=(const Maps&)
    {
        return *this;
    }

private:
    PRREObject3D** m_objects;
    int m_objects_h;

    std::vector<PRREVector> m_candleLights;

    std::map<std::string, PGEcfgVariable> m_vars;
    PR00FsReducedRenderingEngine& m_gfx;
    std::map<char, std::string> m_Block2Texture;
    PRRETexture* m_texRed;
    PRREVector m_start, m_end;
    float m_objectsMinY;
    unsigned int m_width, m_height;

    // ---------------------------------------------------------------------------

    static bool lineShouldBeIgnored(const std::string& sLine);
    static bool lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue, bool& bParseError);

    void lineHandleAssignment(std::string& sVar, std::string& sValue);
    bool lineHandleLayout(const std::string& sLine, TPRREfloat& y);

}; // class Maps
