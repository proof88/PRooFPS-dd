#pragma once

// copied from: https://stackoverflow.com/questions/42531/how-do-i-call-createprocess-in-c-to-launch-a-windows-executable

#ifdef _WIN32

#include <stdio.h>
#include <tchar.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

#include "../../../PFL/PFL/winproof88.h"

#include "../../../Console/CConsole/src/CConsole.h"

namespace process_stackoverflow_42531
{
    class Process
    {
    public:

        static PROCESS_INFORMATION launchProcess(std::string app, std::string arg)
        {
            // Prepare handles.
            STARTUPINFOW si;
            PROCESS_INFORMATION pi; // The function returns this
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            //Prepare CreateProcess args
            std::wstring app_w(app.length(), L' '); // Make room for characters
            std::copy(app.begin(), app.end(), app_w.begin()); // Copy string to wstring.

            std::wstring arg_w(arg.length(), L' '); // Make room for characters
            std::copy(arg.begin(), arg.end(), arg_w.begin()); // Copy string to wstring.

            std::wstring input = app_w + L" " + arg_w;
            wchar_t* arg_concat = const_cast<wchar_t*>(input.c_str());
            const wchar_t* app_const = app_w.c_str();

            // Start the child process.
            if (!CreateProcessW(
                app_const,      // app path
                arg_concat,     // Command line (needs to include app path as first argument. args seperated by whitepace)
                NULL,           // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                FALSE,          // Set handle inheritance to FALSE
                0,              // No creation flags
                NULL,           // Use parent's environment block
                NULL,           // Use parent's starting directory
                &si,            // Pointer to STARTUPINFO structure
                &pi)            // Pointer to PROCESS_INFORMATION structure
                )
            {
                CConsole::getConsoleInstance().EOLn("CreateProcess failed (%d).", GetLastError());
                throw std::exception("Could not create child process");
            }
            else
            {
                CConsole::getConsoleInstance().SOLn("Successfully launched child process: %s", app.c_str());
            }

            // Return process handle
            return pi;
        }

        static bool checkIfProcessIsActive(PROCESS_INFORMATION pi)
        {
            // Check if handle is closed
            if (pi.hProcess == NULL)
            {
                CConsole::getConsoleInstance().EOLn("Process handle is closed or invalid (%d).", GetLastError());
                return false;
            }

            // If handle open, check if process is active
            DWORD lpExitCode = 0;
            if (GetExitCodeProcess(pi.hProcess, &lpExitCode) == 0)
            {
                CConsole::getConsoleInstance().EOLn("Cannot return exit code (%d).", GetLastError());
                throw std::exception("Cannot return exit code");
            }
            else
            {
                if (lpExitCode == STILL_ACTIVE)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        static bool stopProcess(PROCESS_INFORMATION& pi)
        {
            // Check if handle is invalid or has allready been closed
            if (pi.hProcess == NULL)
            {
                CConsole::getConsoleInstance().EOLn("Process handle invalid. Possibly already been closed (%d).");
                return false;
            }

            // Terminate Process
            if (!TerminateProcess(pi.hProcess, 1))
            {
                CConsole::getConsoleInstance().EOLn("ExitProcess failed (%d).", GetLastError());
                return false;
            }

            // Wait until child process exits.
            if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_FAILED)
            {
                CConsole::getConsoleInstance().EOLn("Wait for exit process failed(%d).", GetLastError());
                return false;
            }

            // Close process and thread handles.
            if (!CloseHandle(pi.hProcess))
            {
                CConsole::getConsoleInstance().EOLn("Cannot close process handle(%d).", GetLastError());
                return false;
            }
            else
            {
                pi.hProcess = NULL;
            }

            if (!CloseHandle(pi.hThread))
            {
                CConsole::getConsoleInstance().EOLn("Cannot close thread handle (%d).", GetLastError());
                return 0;
            }
            else
            {
                pi.hProcess = NULL;
            }

            return true;
        }
    }; //class Process
}; // namespace

#endif //win32