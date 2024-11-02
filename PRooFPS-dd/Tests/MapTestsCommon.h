#pragma once

/*
    ###################################################################################
    MapTestsCommon.h
    Common functions for unit test for PRooFPS-dd Maps, Mapcycle, etc.
    Please see UnitTest.h about my statement of using "bitwise and" operator with bool operands.
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include <set>

#include "UnitTest.h"

class MapTestsCommon :
    public UnitTest
{
public:

    MapTestsCommon(const char* szTestName) :
        UnitTest(szTestName)
    {
    }

    MapTestsCommon(const MapTestsCommon&) = delete;
    MapTestsCommon& operator=(const MapTestsCommon&) = delete;
    MapTestsCommon(MapTestsCommon&&) = delete;
    MapTestsCommon& operator=(MapTestsCommon&&) = delete;

protected:

    bool checkConstCharPtrArrayElemsPointingToContainerElems(
        const std::vector<std::string>& vec,
        const char** vszArray
    )
    {
        bool bRet = true;
        for (size_t i = 0; i < vec.size(); i++)
        {
            bRet &= assertEquals(
                vec[i].c_str(),
                vszArray[i],
                (std::string("bad vszArray elem ") + std::to_string(i)).c_str());
        }
        return bRet;
    }

    bool checkConstCharPtrArrayElemsPointingToContainerElems(
        const std::set<std::string>& settt,
        const char** vszArray
    )
    {
        bool bRet = true;
        auto it = settt.begin();
        for (size_t i = 0; i < settt.size(); i++)
        {
            bRet &= assertEquals(
                it->c_str(),
                vszArray[i],
                (std::string("bad vszArray elem ") + std::to_string(i)).c_str());
            it++;
        }
        return bRet;
    }

private:

};
