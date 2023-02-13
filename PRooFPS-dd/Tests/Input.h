#pragma once

/*
    ###################################################################################
    MapsTest.h
    Unit test for PRooFPS-dd Maps.
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <chrono>

#include <Windows.h>

namespace input
{
    void keybdPress(BYTE bVk, unsigned long nSleepMillisecs);
    void mouseClick();
    void mouseMoveRelative(DWORD dx, DWORD dy);
}; // namespace