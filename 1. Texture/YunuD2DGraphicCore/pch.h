// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include "SingletonClass.h"
#include "YunuD2DGraphicCore.h"
#include <wincodec.h>
#include <d2d1.h>
#include <d2d1_1.h>
#pragma comment(lib,"d2d1.lib")
#include <dwrite.h>
#pragma comment(lib,"Dwrite.lib")

#endif //PCH_H

