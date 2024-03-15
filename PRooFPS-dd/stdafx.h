#pragma once

/*
    ###################################################################################
    stdafx.h
    Precompiled header for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

// C RunTime Header Files
#include <conio.h>
#include <direct.h>
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

// C++ std headers
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <set>
#include <string>
#include <vector>

// Windows Header Files
#include "targetver.h"
#ifndef WINPROOF88_ALLOW_CONTROLS_AND_DIALOGS
#define WINPROOF88_ALLOW_CONTROLS_AND_DIALOGS
#endif
#ifndef WINPROOF88_ALLOW_MSG_USER_WINMESSAGES
#define WINPROOF88_ALLOW_MSG_USER_WINMESSAGES
#endif
// otherwise SoLoud.h will have problem due to colliding macro NOSOUND!
#ifndef WINPROOF88_ALLOW_SOUND
#define WINPROOF88_ALLOW_SOUND
#endif
#ifndef WINPROOF88_ALLOW_VIRTUALKEYCODES
#define WINPROOF88_ALLOW_VIRTUALKEYCODES
#endif
#include "winproof88.h"

// Own headers
#include "PFL.h"
