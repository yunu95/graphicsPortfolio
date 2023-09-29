// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <intrin.h>

#include <stdio.h>
#include <cassert>

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d12.lib")

#include "resource.h"

// if com call fails in debug mode, it will ouput debug strings as following below
#ifdef _DEBUG
#ifndef DXCall
#define DXCall(x)                           \
if(FAILED(x)) {                             \
    char line_number[32];                   \
    sprintf_s(line_number,32,"%u",__LINE__);\
    OutputDebugStringA("Error in : ");      \
    OutputDebugStringA(__FILE__);           \
    OutputDebugStringA("\nLine: ");         \
    OutputDebugStringA(line_number);        \
    OutputDebugStringA("\n");               \
    OutputDebugStringA(#x);                 \
    OutputDebugStringA("\n");               \
    __debugbreak();                         \
}
#endif
#else
#ifndef DXCall
#define DXCall(x) x
#endif
#endif

// by using this Macro call, one can track creations of COM objects 
#ifdef _DEBUG
#define NAME_D3D12_OBJECT(obj,name) \
    obj->SetName(name);\
    OutputDebugString(L"::D3D12 Object Created: ");\
    OutputDebugString(name);\
    OutputDebugString(L"\n");
#else
#define NAME_D3D12_OBJECT(obj,name)\
    obj->SetName(name);
#endif

