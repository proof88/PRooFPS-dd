#pragma once

/*
    ###################################################################################
    MapsTest.h
    Unit test for PRooFPS-dd Maps.
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../../../../PGE/PGE/UnitTests/UnitTest.h"

class MapsTest :
    public UnitTest
{
public:

    MapsTest() :
        UnitTest( __FILE__ )
    {
        AddSubTest("test1", (PFNUNITSUBTEST) &MapsTest::test1);
    } // PGEcfgVariableTest()

protected:

private:

    // ---------------------------------------------------------------------------

    MapsTest(const MapsTest&)
    {};         

    MapsTest& operator=(const MapsTest&)
    {
        return *this;
    };

    bool test1()
    {

        return true;
    }

}; 