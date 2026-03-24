//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#ifndef _PCH_BASE_H_
#define _PCH_BASE_H_


// Disable warning C4786: 'identifier was truncated to ### characters in the debug information'
#pragma warning(disable:4786)

//******************************* TEMPORAL - TEMPORAL - TEMPORAL **********************************
//******************************* TEMPORAL - TEMPORAL - TEMPORAL **********************************
// Disable old CRT functions deprecation warnings
#define _CRT_SECURE_NO_DEPRECATE
//******************************* TEMPORAL - TEMPORAL - TEMPORAL **********************************
//******************************* TEMPORAL - TEMPORAL - TEMPORAL **********************************


// STL includes
// TODO: Remove when "using namespace std" is removed. Prevents C++17 std::byte from conflicting with Windows SDK's byte typedef.
#define _HAS_STD_BYTE 0
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <unordered_map>

using namespace std;    // No need to specify std:: for STL symbols

// Standard C includes
#include <stdio.h>
#include <math.h>

// Windows includes
#include <CrtDbg.h>

#define OEMRESOURCE    // This enables definition of OCR_ macros
#include <Windows.h>
#include <CommCtrl.h>

// Own general purpose includes
#include "kmbGlobalDefs.h"
#include "Debug/kmbAsserts.h"
#include "Misc/kmbSingleton.h"

// Third-party includes
#include "Archive/miniz/miniz.h"


#endif // _PCH_BASE_H_
