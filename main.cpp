#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_THREAD_LOCALS
#include "stb_image.h"

#pragma comment(lib, "comdlg32.lib")

static HBITMAP gBitmap = NULL;
static int gImageWidth = 0;
static int gImageHeight = 0;

static HBRUSH gBackgroundBrush = NULL;

#define WM_LOAD_CMDLINE (WM_USER + 1)

static BOOL LoadImageFile(HWND hwnd, const char* filename)
{
	int width;
	int height;
	int channels;

	unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);

	if (!data)
	{
		MessageBoxA(hwnd,
					stbi_failure_reason(),
					"Failed to load image",
					MB_ICONERROR);

		return FALSE;
	}

	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(bmi));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height; // top-down bitmap
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* bits = NULL;

	HDC hdc = GetDC(hwnd);

	HBITMAP bitmap = CreateDIBSection(
		hdc,
		&bmi,
		DIB_RGB_COLORS,
		&bits,
		NULL,
		0);

	ReleaseDC(hwnd, hdc);

	if (!bitmap)
	{
		stbi_image_free(data);

		MessageBoxA(hwnd,
					"CreateDIBSection failed",
					"Error",
					MB_ICONERROR);

		return FALSE;
	}

	// Convert RGBA -> BGRA
	unsigned char* dst = (unsigned char*)bits;

	int pixelCount = width * height;

	int i;

	for (i = 0; i < pixelCount; i++)
	{
		dst[i * 4 + 0] = data[i * 4 + 2];
		dst[i * 4 + 1] = data[i * 4 + 1];
		dst[i * 4 + 2] = data[i * 4 + 0];
		dst[i * 4 + 3] = data[i * 4 + 3];
	}

	stbi_image_free(data);

	if (gBitmap)
	{
		DeleteObject(gBitmap);
	}

	gBitmap = bitmap;
	gImageWidth = width;
	gImageHeight = height;

	InvalidateRect(hwnd, NULL, TRUE);

	return TRUE;
}

static void OpenImageDialog(HWND hwnd)
{
	char filename[MAX_PATH];

	ZeroMemory(filename, sizeof(filename));

	OPENFILENAMEA ofn;

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter =
		"Images\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.gif\0"
		"All Files\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileNameA(&ofn))
	{
		LoadImageFile(hwnd, filename);
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd,
							UINT msg,
							WPARAM wParam,
							LPARAM lParam)
{
	switch (msg)
	{
		// Create the window
	case WM_CREATE:
		{
			PostMessage(hwnd, WM_LOAD_CMDLINE, 0, 0);
			/*LPSTR cmd = GetCommandLineA();
			while (*cmd && *cmd != ' ') cmd++;
			while (*cmd == ' ') cmd++;

			if (*cmd == '\0')
				OpenImageDialog(hwnd);
*/
			return 0;
		}
	// If the window gets resized, then redraw the window
	case WM_SIZE:
		{
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);
			return 0;
		}
		// Hopefully help save mine and yours eyes
	case WM_ERASEBKGND:
		{
			return 1;
		}

	case WM_KEYDOWN:
		{
			if (wParam == 'L')
			{
				OpenImageDialog(hwnd);
			}
			
			return 0;
		}

		// This is where the image gets shown on the screen
	case WM_PAINT:
		{
			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hwnd, &ps);

			RECT rc;
			GetClientRect(hwnd, &rc);

			FillRect(hdc, &rc, gBackgroundBrush);

			if (gBitmap)
			{
				HDC memdc = CreateCompatibleDC(hdc);

				HBITMAP oldbmp =
					(HBITMAP)SelectObject(memdc, gBitmap);

				RECT rc;
				GetClientRect(hwnd, &rc);

				int windowWidth = rc.right - rc.left;
				int windowHeight = rc.bottom - rc.top;

				/* Create back buffer
				HDC backdc = CreateCompatibleDC(hdc);

				HBITMAP backbmp =
					CreateCompatibleBitmap(hdc,
										   windowWidth,
										   windowHeight);

				HBITMAP oldback =
					(HBITMAP)SelectObject(backdc, backbmp);

				 Clear Background
				FillRect(backdc,						////////////////////////////////////////////////////////
						 &rc,							// Commented out because double buffering didn't work //
						 (HBRUSH)(COLOR_WINDOW + 1));	////////////////////////////////////////////////////////

				 if (gBitmap)
				{
					HDC memdc = CreateCompatibleDC(hdc);
					
					HBITMAP oldbmp =
						(HBITMAP)SelectObject(memdc, gBitmap); */

				float scaleX =
					(float)windowWidth / (float)gImageWidth;

				float scaleY = 
					(float)windowHeight / (float)gImageHeight;

				float scale =
					(scaleX < scaleY) ? scaleX : scaleY;

				int drawWidth =
					(int)(gImageWidth * scale);

				int drawHeight =
					(int)(gImageHeight * scale);

				float imageAspect =
					(float)gImageWidth / (float)gImageHeight;

				float windowAspect =
					(float)windowWidth / (float)windowHeight;

				if (windowAspect > imageAspect)
				{
					drawHeight = windowHeight;
					drawWidth = (int)(windowHeight * imageAspect);
				}
				else
				{
					drawWidth = windowWidth;
					drawHeight = (int)(windowWidth / imageAspect);
				}

				int x = (windowWidth - drawWidth) / 2;
				int y = (windowHeight - drawHeight) / 2;


				SetStretchBltMode(hdc, HALFTONE);
				SetBrushOrgEx(hdc, 0, 0, NULL);

				StretchBlt(hdc,
							x,
							y,
							drawWidth,
							drawHeight,
							memdc,
							0,
							0,
							gImageWidth,
							gImageHeight,
							SRCCOPY);

				SelectObject(memdc, oldbmp);

				DeleteDC(memdc);
				 }
			/*}

			BitBlt(hdc,
				   0,
				   0,
				   windowWidth,
				   windowHeight,    ////////////////////////////////////////////////////////
				   backdc,			// Commented out because double buffering didn't work //
				   0,				////////////////////////////////////////////////////////
				   0,
				   SRCCOPY);

			SelectObject(backdc, oldback);

			DeleteObject(backbmp);
			DeleteDC(backdc); */

			EndPaint(hwnd, &ps);

			return 0;
		}
	case WM_LOAD_CMDLINE:
		{
			LPSTR cmd = GetCommandLineA();

			if (*cmd == '"')
			{
				cmd++;
				while (*cmd && *cmd != '"') cmd++;
				if (*cmd == '"') cmd++;
			}
			else
			{
				while (*cmd && *cmd != ' ') cmd++;
			}

			while (*cmd == ' ') cmd++;

			if (*cmd != '\0')
			{
				if (*cmd == '"')
				{
					cmd++;
					char path[MAX_PATH];
					int i = 0;
					while (*cmd && *cmd != '"' && i < MAX_PATH - 1)
						path[i++] = *cmd++;
					path[i] = '\0';
					LoadImageFile(hwnd, path);
				}
				else
				{
					LoadImageFile(hwnd, cmd);
				}
			}
			else
			{
				OpenImageDialog(hwnd);
			}

			return 0;
		}

	// Demolish the window. Kaboom.
	case WM_DESTROY:
		{
			if (gBitmap)
			{
				if (gBackgroundBrush)
				{
					DeleteObject(gBackgroundBrush);
					gBackgroundBrush = NULL;
				}
				DeleteObject(gBitmap);
				gBitmap = NULL;
			}

			PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)

{
	WNDCLASSA wc;

	ZeroMemory(&wc, sizeof(wc));

	gBackgroundBrush =
		CreateSolidBrush(RGB(200, 200, 200));

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "TEST";
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	// wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hbrBackground = 
		gBackgroundBrush;



	RegisterClassA(&wc);

	HWND hwnd = CreateWindowA(
		"TEST",
		"TEST",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,
		600,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hwnd)
	{
		return 0;

		if (lpCmdLine && lpCmdLine[0] != '\0')
			LoadImageFile(hwnd, lpCmdLine);
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
