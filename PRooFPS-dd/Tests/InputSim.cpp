/*
    ###################################################################################
    InputSim.cpp
    Simulating input for PRooFPS-dd Regression Tests.
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include "stdafx.h" // PCH
#include "InputSim.h"

#include <thread>

void input_sim_test::keybdPress(BYTE bVk, unsigned long nSleepMillisecs)
{
    keybd_event(bVk, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMillisecs));
    keybd_event(bVk, 0, KEYEVENTF_KEYUP, 0);
}

void input_sim_test::keybdPressNoRelease(BYTE bVk)
{
    keybd_event(bVk, 0, 0, 0);
}

void input_sim_test::keybdRelease(BYTE bVk)
{
    keybd_event(bVk, 0, KEYEVENTF_KEYUP, 0);
}

void input_sim_test::mouseClick(unsigned long nSleepMillisecs)
{
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMillisecs));
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void input_sim_test::mouseScroll(bool bForward)
{
    // positive value in dwData is scrolling forward aka away from user aka scrolling up
    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, bForward ? WHEEL_DELTA : -WHEEL_DELTA, 0);
}

// To be used with mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, ...)
//static int CalculateAbsoluteCoordinateX(int x)
//{
//    return (x * 65536) / GetSystemMetrics(SM_CXSCREEN);
//}
//
//static int CalculateAbsoluteCoordinateY(int y)
//{
//    return (y * 65536) / GetSystemMetrics(SM_CYSCREEN);
//}

void input_sim_test::mouseMoveRelative(DWORD dx, DWORD dy)
{
    mouse_event(MOUSEEVENTF_MOVE, dx, dy, 0, 0);
        
    // SetCursorPos() also uses screen coordinates like our RECTs
    // However, this didnt have any effect (absolute coords):
    //   SetCursorPos(rectServerGameWindow.right - 10, rectServerGameWindow.top + (rectServerGameWindow.bottom - rectServerGameWindow.top) / 2 + 50);

    // I also tried mouse_event() and wanted to avoid using relative coordinates, because "relative mouse motion is subject to the settings for mouse
    // speed and acceleration level. An end user sets these values using the Mouse application in Control Panel."
    // I didn't want to rely on such properties like speed and acceleration level.
    // However, specifying MOUSEEVENTF_ABSOLUTE to use absolute coordinates with CalculateAbsoluteCoordinateX/Y() functions somehow always put
    // the cursor far into right bottom corner, and I gave up finding out why. "If MOUSEEVENTF_ABSOLUTE value is specified, dx and dy contain normalized
    // absolute coordinates between 0 and 65,535. The event procedure maps these coordinates onto the display surface.
    // Coordinate (0,0) maps onto the upper-left corner of the display surface, (65535,65535) maps onto the lower-right corner."
    // My values were between 0 and 65535 for sure, still it always went into lower-right corner all the time.
    // So I ended up using relative coordinates because anyway on my dev machine, they worked as expected.
    // int mpx = CalculateAbsoluteCoordinateX(rectServerGameWindow.right - 10);
    // int mpy = CalculateAbsoluteCoordinateY(rectServerGameWindow.top + (rectServerGameWindow.bottom - rectServerGameWindow.top) / 2 + 50);
    //    mouse_event(
    //        MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE,
    //        mpx,
    //        mpy,
    //        0, 0);
}

void input_sim_test::bringWindowToFront(HWND hTargetWindow) noexcept(false)
{
    // technique copied from: https://stackoverflow.com/questions/916259/win32-bring-a-window-to-top
    // slightly modified: try not to accept NULL foreground window, try a bit more ...
    HWND hTmpWnd = GetForegroundWindow();
    for (int i = 0; (i < 3) && (hTmpWnd == NULL); i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        hTmpWnd = GetForegroundWindow();
    }
    const HWND hCurWnd = hTmpWnd;
    if (hCurWnd == NULL)
    {
        // Win32 SDK: "The foreground window can be NULL in certain circumstances, such as when a window is losing activation."
        // I still want to report this as an error since we use hCurWnd as input to other functions.
        throw std::exception(
            std::string("ERROR: GetForegroundWindow() returned NULL!").c_str());
    }

    const DWORD dwMyID = GetCurrentThreadId();
    if (dwMyID == 0)
    {
        throw std::exception(
            std::string("ERROR: GetCurrentThreadId() returned 0!").c_str());
    }

    const DWORD dwCurID = GetWindowThreadProcessId(hCurWnd, NULL);
    if (dwCurID == 0)
    {
        throw std::exception(
            std::string("ERROR: GetWindowThreadProcessId() returned 0!").c_str());
    }

    if (0 == AttachThreadInput(dwCurID, dwMyID, TRUE))
    {
        throw std::exception(
            std::string("ERROR: AttachThreadInput(..., TRUE) failed: " + std::to_string(GetLastError()) + "!").c_str());
    }

    if (0 == SetWindowPos(hTargetWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE))
    {
        throw std::exception(
            std::string("ERROR: SetWindowPos(TOPMOST) failed: " + std::to_string(GetLastError()) + "!").c_str());
    }

    if (0 == SetWindowPos(hTargetWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE))
    {
        throw std::exception(
            std::string("ERROR: SetWindowPos(NOTOPMOST) failed: " + std::to_string(GetLastError()) + "!").c_str());
    }

    if (0 == SetForegroundWindow(hTargetWindow))
    {
        throw std::exception(
            std::string("ERROR: SetForegroundWindow() failed!").c_str());
    }

    // This actually returns NULL for some reason, on for the client window, with error code 5: access denied.
    //if (NULL == SetFocus(hTargetWindow))
    //{
    //    throw std::exception(
    //        std::string("ERROR: SetFocus() failed: " + std::to_string(GetLastError()) + "!").c_str());
    //}

    // This also returns NULL for some reason, only for the client window, with error code 0: error_success.
    //if (NULL == SetActiveWindow(hTargetWindow))
    //{
    //    throw std::exception(
    //        std::string("ERROR: SetActiveWindow() failed: " + std::to_string(GetLastError()) + "!").c_str());
    //}

    if (0 == AttachThreadInput(dwCurID, dwMyID, FALSE))
    {
        throw std::exception(
            std::string("ERROR: AttachThreadInput(..., FALSE) failed: " + std::to_string(GetLastError()) + "!").c_str());
    }
} // BringWindowToFront()
