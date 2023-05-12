#pragma once

/*
    ###################################################################################
    InputSim.h
    Simulating input for PRooFPS-dd Regression Tests.
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <chrono>

#include <Windows.h>

namespace input_sim_test
{
    void keybdPress(BYTE bVk, unsigned long nSleepMillisecs);
    void keybdPressNoRelease(BYTE bVk);
    void keybdRelease(BYTE bVk);
    void mouseClick(unsigned long nSleepMillisecs);
    void mouseScroll(bool bForward);
    void mouseMoveRelative(DWORD dx, DWORD dy);
}; // namespace