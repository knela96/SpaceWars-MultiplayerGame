#include "Networks.h"


// Data
const char *windowClassStr = "Networks and Online Games";
const char *windowTitleStr = "Networks and Online Games";
HINSTANCE instance = NULL;                           // Application instance
HWND hwnd = NULL;                                    // Window handle
WNDCLASSEX windowClass = {};                         // Window class

static InputController GamepadInput;
static InputController KeyboardInput;

static bool IsFocused = false;


// XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

static LONGLONG GlobalPerfCountFrequency;
static LARGE_INTEGER StartTime;
static LARGE_INTEGER EndTime;

static real32 GameUpdateHz;
static real32 TargetSecondsPerFrame;


inline LARGE_INTEGER
Win32GetWallClock(void)
{
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return(Result);
}

inline float
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	float Result = ((float)(End.QuadPart - Start.QuadPart) /
		(float)GlobalPerfCountFrequency);
	return(Result);
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	App->modUI->HandleWindowsEvents(msg, wParam, lParam);

	switch (msg)
	{
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED && wParam != SIZE_MAXSHOW)
		{
			Window.width = (UINT)LOWORD(lParam);
			Window.height = (UINT)HIWORD(lParam);
			App->modRender->resizeBuffers(Window.width, Window.height);
		}
		return 0;

	case WM_KILLFOCUS:
		Input = {};
		GamepadInput = {};
		KeyboardInput = {};
		IsFocused = false;
		return 0;

	case WM_SETFOCUS:
		IsFocused = true;
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}

	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

static void Win32ProcessKeyboardButton(ButtonState *NewState, bool IsDown)
{
	// Maybe control half transition counts?

	if (IsDown)
	{
		*NewState = Press;
	}
	else
	{
		*NewState = Release;
	}
}

static void Win32ProcessMouseButton(ButtonState *NewState, bool IsDown)
{
	if (*NewState == Idle && IsDown)
	{
		*NewState = Press;
	}
	else if (*NewState == Pressed && !IsDown)
	{
		*NewState = Release;
	}
}

static void Win32ProcessGamepadButton(ButtonState *NewState, XINPUT_GAMEPAD *pad, int button_flag)
{
	bool IsPressed = pad->wButtons & button_flag;
	if (IsPressed)
	{
		if (*NewState == Idle)
		{
			*NewState = Press;
		}
		else
		{
			*NewState = Pressed;
		}
	}
	else
	{
		if (*NewState == Pressed || *NewState == Press)
		{
			*NewState = Release;
		}
		else
		{
			*NewState = Idle;
		}
	}
}

static bool Win32IsGamepadButtonPressed(XINPUT_GAMEPAD *pad, int button_flag)
{
	bool IsPressed = pad->wButtons & button_flag;
	return IsPressed;
}

static float Win32ProcessGamepadThumb(SHORT value, SHORT deadZoneThreshold)
{
	float res = 0;

	if (value < -deadZoneThreshold)
	{
		res = (float)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
	}
	else if (value > deadZoneThreshold)
	{
		res = (float)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
	}

	return(res);
}

bool ModulePlatform::init()
{
	// Get the application instance handle
	instance = ::GetModuleHandle("");

	// Create window class
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpszClassName = _T(windowClassStr);
	windowClass.hInstance = instance;
	windowClass.lpfnWndProc = WndProc;
	windowClass.style = CS_OWNDC;
	if (::RegisterClassEx(&windowClass) == 0)
	{
		ELOG("ModuleWindow::init() - RegisterClassEx() failed");
		return false;
	}

	// Create application window
	hwnd = ::CreateWindowEx(
		0,
		windowClass.lpszClassName,
		_T(windowTitleStr),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1024, //CW_USEDEFAULT,
		768, //CW_USEDEFAULT,
		NULL,
		NULL,
		windowClass.hInstance,
		NULL);

#if 0
	// Place the window on top of all others
	RECT windowRect;
	::GetWindowRect(hwnd, &windowRect);
	::SetWindowPos(
		hwnd,
		HWND_TOPMOST,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		0);
#endif

	// Init XInput
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (!XInputLibrary) { XInputLibrary = LoadLibraryA("xinput9_1_0.dll"); }
	if (!XInputLibrary) { XInputLibrary = LoadLibraryA("xinput1_3.dll"); }
	if (XInputLibrary)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		if (!XInputGetState) { XInputGetState = XInputGetStateStub; }
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
		if (!XInputSetState) { XInputSetState = XInputSetStateStub; }
	}
	else
	{
		ELOG("ModulePlatform::init() - XInput library loading failed");
		::DestroyWindow(hwnd);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return false;
	}

	// Initialize button states
	Input = {};
	GamepadInput = {};
	KeyboardInput = {};

	// Is this query reliable?
	int MonitorRefreshHz = 60;
	HDC RefreshDC = GetDC(hwnd);
	int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
	ReleaseDC(hwnd, RefreshDC);
	if (Win32RefreshRate > 1)
	{
		MonitorRefreshHz = Win32RefreshRate;
	}
	GameUpdateHz = (MonitorRefreshHz / 1.0f);
	TargetSecondsPerFrame = 1.0f / (real32)GameUpdateHz;

	// Get the clock frequency
	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

	// Initialize time
	StartTime = Win32GetWallClock();

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Initial information
	LOG("Press the <F1> key to toggle UI visibility.");
	LOG("Keyboard/gamepad are mapped to the global Input object.");
	LOG("Keyboard mappings are : ");
	LOG(" - A, S, D, W: Directional pad.");
	LOG(" - Q, E: Left and right shoulder buttons.");
	LOG(" - ESC, SPACE: Back and start buttons.");
	LOG(" - Arrows: Action buttons.");

	return true;
}

bool ModulePlatform::preUpdate()
{
	MSG msg = {};

	InputController KeyboardInput = Input;
	InputController GamepadInput = Input;

	// Poll and handle messages (inputs, window resize, etc.)
	// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
	// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
	// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
	for(;;)
	{
		BOOL GotMessage = FALSE;
		DWORD LastMessage = 0;

		// NOTE(jesus): This is in case we want to skip certain messages (in sorted order)
		DWORD SkipMessages[] =
		{
			// NOTE(jesus): Add messages to skip above 0xFFFFFFFF
			0xFFFFFFFF,
		};
		for (auto Skip : SkipMessages)
		{
			GotMessage = ::PeekMessage(&msg, 0, LastMessage, Skip - 1, PM_REMOVE);
			LastMessage = Skip + 1;
			if (GotMessage)
			{
				break;
			}
		}

		if (!GotMessage)
		{
			break;
		}

		switch (msg.message)
		{
		case WM_QUIT:
			App->exit();
			return false;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
			if (ImGui::GetIO().WantCaptureKeyboard == false)
			{
				unsigned int VKCode = (unsigned int)msg.wParam;

				bool AltKeyWasDown = (msg.lParam & (1 << 29));
				bool ShiftKeyWasDown = (GetKeyState(VK_SHIFT) & (1 << 15));

				bool WasDown = ((msg.lParam & (1UL << 30)) != 0);
				bool IsDown = ((msg.lParam & (1UL << 31)) == 0);

				if (WasDown != IsDown)
				{
					if (VKCode == 'Q')
					{
						Win32ProcessKeyboardButton(&KeyboardInput.leftShoulder, IsDown);
					}
					else if (VKCode == 'E')
					{
						Win32ProcessKeyboardButton(&KeyboardInput.rightShoulder, IsDown);
					}
					else if (VKCode == VK_UP)
					{
						Win32ProcessKeyboardButton(&KeyboardInput.actionUp, IsDown);
					}
					else if (VKCode == VK_LEFT)
					{
						Win32ProcessKeyboardButton(&KeyboardInput.actionLeft, IsDown);
					}
					else if (VKCode == VK_DOWN)
					{
						Win32ProcessKeyboardButton(&KeyboardInput.actionDown, IsDown);
					}
					else if (VKCode == VK_RIGHT)
					{
						Win32ProcessKeyboardButton(&KeyboardInput.actionRight, IsDown);
					}
					else if (VKCode == VK_ESCAPE)
					{
						Win32ProcessKeyboardButton(&KeyboardInput.back, IsDown);
					}
					else if (VKCode == VK_RETURN)
					{
						Win32ProcessKeyboardButton(&KeyboardInput.start, IsDown);
					}
					else if (VKCode == VK_F1 && IsDown)
					{
						App->modUI->setEnabled(!App->modUI->isEnabled());
					}
					else if (VKCode == VK_F4 && IsDown)
					{
						return false; // Finish the application
					}
				}
			}
			::TranslateMessage(&msg);
			break;

		default:;
		}

		::DispatchMessage(&msg);
	}

	static bool firstIteration = true;
	if (firstIteration)
	{
		StartTime = Win32GetWallClock();
		firstIteration = false;
	}

	// Time management
	EndTime = Win32GetWallClock();
	Time.frameTime = Win32GetSecondsElapsed(StartTime, EndTime);
	Time.deltaTime = TargetSecondsPerFrame;
	Time.time += (double)Time.deltaTime;
	StartTime = EndTime;

	if (IsFocused)
	{
		// Keyboard
		KeyboardInput.horizontalAxis = ((GetKeyState('D') & (1 << 15)) ? 1.0f : 0.0f) - ((GetKeyState('A') & (1 << 15)) ? 1.0f : 0.0f);
		KeyboardInput.verticalAxis = ((GetKeyState('W') & (1 << 15)) ? 1.0f : 0.0f) - ((GetKeyState('S') & (1 << 15)) ? 1.0f : 0.0f);

		// Mouse
		POINT mousePosition;
		GetCursorPos(&mousePosition);
		ScreenToClient(hwnd, &mousePosition);
		Mouse.x = (int16)mousePosition.x;
		Mouse.y = (int16)mousePosition.y;
		Win32ProcessMouseButton(&Mouse.buttons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
		Win32ProcessMouseButton(&Mouse.buttons[1], GetKeyState(VK_MBUTTON) & (1 << 15));
		Win32ProcessMouseButton(&Mouse.buttons[2], GetKeyState(VK_RBUTTON) & (1 << 15));
		Win32ProcessMouseButton(&Mouse.buttons[3], GetKeyState(VK_XBUTTON1) & (1 << 15));
		Win32ProcessMouseButton(&Mouse.buttons[4], GetKeyState(VK_XBUTTON2) & (1 << 15));

		// Input gamepads
		// XInput has a serious bug here. If there is no controller plugged
		// XInputGetState() stalls for several milliseconds before returning. We should
		// make sure we only try to get the state of connected devices.
		XINPUT_STATE controllerState;
		if (XInputGetState(0, &controllerState) == ERROR_SUCCESS)
		{
			XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

			Win32ProcessGamepadButton(&GamepadInput.actionUp, pad, XINPUT_GAMEPAD_Y);
			Win32ProcessGamepadButton(&GamepadInput.actionDown, pad, XINPUT_GAMEPAD_A);
			Win32ProcessGamepadButton(&GamepadInput.actionLeft, pad, XINPUT_GAMEPAD_X);
			Win32ProcessGamepadButton(&GamepadInput.actionRight, pad, XINPUT_GAMEPAD_B);
			Win32ProcessGamepadButton(&GamepadInput.leftShoulder, pad, XINPUT_GAMEPAD_LEFT_SHOULDER);
			Win32ProcessGamepadButton(&GamepadInput.rightShoulder, pad, XINPUT_GAMEPAD_RIGHT_SHOULDER);
			Win32ProcessGamepadButton(&GamepadInput.back, pad, XINPUT_GAMEPAD_BACK);
			Win32ProcessGamepadButton(&GamepadInput.start, pad, XINPUT_GAMEPAD_START);

			GamepadInput.horizontalAxis = Win32ProcessGamepadThumb(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			GamepadInput.verticalAxis = Win32ProcessGamepadThumb(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			if (GamepadInput.verticalAxis == 0.0f && GamepadInput.horizontalAxis == 0.0f)
			{
				GamepadInput.verticalAxis += Win32IsGamepadButtonPressed(pad, XINPUT_GAMEPAD_DPAD_UP) ? 1.0f : 0.0f;
				GamepadInput.verticalAxis -= Win32IsGamepadButtonPressed(pad, XINPUT_GAMEPAD_DPAD_DOWN) ? 1.0f : 0.0f;
				GamepadInput.horizontalAxis -= Win32IsGamepadButtonPressed(pad, XINPUT_GAMEPAD_DPAD_LEFT) ? 1.0f : 0.0f;
				GamepadInput.horizontalAxis += Win32IsGamepadButtonPressed(pad, XINPUT_GAMEPAD_DPAD_RIGHT) ? 1.0f : 0.0f;
			}

			Input = GamepadInput;
		}
		else
		{
			// NOTE(jesus): No controller was found
			Input = KeyboardInput;
		}
	}

	return true;
}

bool ModulePlatform::postUpdate()
{
	// Update buttons state
	for (auto &buttonState : Input.buttons)
	{
		if (buttonState == Press)
		{
			buttonState = Pressed;
		}
		else if (buttonState == Release)
		{
			buttonState = Idle;
		}
	}

	for (auto &buttonState : Mouse.buttons)
	{
		if (buttonState == Press)
		{
			buttonState = Pressed;
		}
		else if (buttonState == Release)
		{
			buttonState = Idle;
		}
	}

	return true;
}

bool ModulePlatform::cleanUp()
{
	::DestroyWindow(hwnd);
	::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

	return true;
}
