// This is viv, vmunix's image viewer. This is a program that I made out of spite for my beloved Dell Latitude D610 with Windows XP 
// and Microsoft Visual C++ 2008. It was out of spite because freaking all of the image viewers that I already had installed sucked
// and I didn't feel like going online and downloading some closed-source slop. I remembered that there was an image thing in stb
// from nothings. And fast forward to now, this image viewer now has support for all the things that stb does, on top of being
// compatible with Windows XP and also having other things like double buffering and (soon) animated gif loading. -vmunix, 5/24/26

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

			int windowWidth = rc.right - rc.left;
			int windowHeight = rc.bottom - rc.top;

			// Create Back Buffer
			HDC backDC = CreateCompatibleDC(hdc);
			HBITMAP backBmp = CreateCompatibleBitmap(hdc, windowWidth, windowHeight);
			HBITMAP oldBack = (HBITMAP)SelectObject(backDC, backBmp);

			// Draw background onto back buffer
			FillRect(backDC, &rc, gBackgroundBrush);

			if (gBitmap)
			{
				HDC memDC = CreateCompatibleDC(hdc);
				HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, gBitmap);

				float imageAspect = (float)gImageWidth / (float)gImageHeight;
				float windowAspect = (float)windowWidth / (float)windowHeight;

				int drawWidth, drawHeight;

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

				// Stretch image onto back buffer
				SetStretchBltMode(backDC, HALFTONE);
				SetBrushOrgEx(backDC, 0, 0, NULL);

				StretchBlt(backDC,
						   x, y,
						   drawWidth, drawHeight,
						   memDC,
						   0, 0,
						   gImageWidth, gImageHeight,
						   SRCCOPY);
				
				SelectObject(memDC, oldBmp);
				DeleteDC(memDC);
		}
			// Flip the completed back buffer to the screen in oneshot niko
			BitBlt(hdc, 0, 0, windowWidth, windowHeight, backDC, 0, 0, SRCCOPY);

			// Clean up back buffer
			SelectObject(backDC, oldBack);
			DeleteObject(backBmp);
			DeleteDC(backDC);

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

	SetWindowTextA(hwnd, "viv"); // See compilation instructions if this is not working
	
	if (!hwnd)
	{
		if (lpCmdLine && lpCmdLine[0] != '\0')
			LoadImageFile(hwnd, lpCmdLine);

		return 0;

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
