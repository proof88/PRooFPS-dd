#pragma once

/*
    ###################################################################################
    InputSim.h
    Simulating input for PRooFPS-dd Regression Tests.
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include <chrono>

#include "../../../PFL/PFL/winproof88.h"

namespace input_sim_test
{
    void keybdPress(BYTE bVk, unsigned long nSleepMillisecs);
    void keybdPressNoRelease(BYTE bVk);
    void keybdRelease(BYTE bVk);
    void mouseClick(unsigned long nSleepMillisecs);
    void mouseScroll(bool bForward);
    void mouseMoveRelative(DWORD dx, DWORD dy);
    void bringWindowToFront(HWND hTargetWindow) noexcept(false);
}; // namespace