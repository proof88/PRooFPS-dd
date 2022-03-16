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

#include <string>

#include "../../../CConsole/CConsole/src/CConsole.h"
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

    PR00FsReducedRenderingEngine& m_gfx;
    PRRETexture* m_tex_brick1, *m_tex_brick2, *m_tex_brick3, *m_tex_brick4, *m_tex_crate, *m_tex_floor, *m_tex_aztec1, *m_tex_castle4;
    PRREVector m_start, m_end;
    float m_objectsMinY;
    unsigned int m_width, m_height;

    // ---------------------------------------------------------------------------

    static bool lineShouldBeIgnored(const std::string& sLine);
    static bool lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue, bool& bParseError);

    bool lineHandleLayout(const std::string& sLine, TPRREfloat& y);

}; // class Maps
