#pragma once

// NOTE(jesus):
// This is a precompiled header.
// We can include here all files that are not likely going
// to change. System files and standard headers that we will
// never edit are good examples of files to include here.

#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <mmsystem.h>
#include <dsound.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <xinput.h>

#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <deque>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h> // ptrdiff_t on osx
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>  // ldexp, pow

// dear imgui: standalone example application for DirectX 9
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

// Sean Barret's STB image loading library
#include "stb/stb_image.h"
