/*
 *  Editbox that can be use with programs that have not started 
 *  yet the Windows messaging system.
 *  Copyright (c) 2003 Pertimm, Inc. by Patrick Constant
 *  Dev : March 2003
 *  Version 1.0
*/ 
#include <winsock2.h>
#include <windows.h>
#include <loggen.h>

#define IDC_MAIN_EDIT  101

const char OgEditBoxClassName[] = "OgEditWindowClass";
char *OgEditBoxContent = 0;



LRESULT CALLBACK OgEditBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
switch(msg) {
  case WM_CREATE:
    {
    HFONT hFont=0;
    HWND hEdit;

    //hEdit = CreateWindowEx(WS_EX_STATICEDGE, "EDIT", OgEditBoxContent, 
    hEdit = CreateWindowEx(0, "EDIT", OgEditBoxContent, 
      WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
      0, 0, 0, 0, hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);
    if(hEdit == NULL)
      MessageBox(hwnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);

    hFont = CreateFont (12,7,0,0,FW_LIGHT, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Courrier");
    //hfDefault = GetStockObject(DEFAULT_GUI_FONT);
    //SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
    SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
    }
    break;

  case WM_SIZE:
    {
    HWND hEdit;
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);
    SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
    }
    break;

  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
return 0;
}




PUBLIC(int) OgEditBox(hInstance,nCmdShow,name,content,width,heigth)
HINSTANCE hInstance; int nCmdShow;
char *name,*content;
int width,heigth;
{
int cxLargeur=GetSystemMetrics(SM_CXSCREEN);
int cyHauteur=GetSystemMetrics(SM_CYSCREEN);
int x=(cxLargeur-width)/2;
int y=(cyHauteur-heigth)/2;
WNDCLASSEX wc;
HWND hwnd;
MSG Msg;

OgEditBoxContent = content;

wc.cbSize     = sizeof(WNDCLASSEX);
wc.style     = 0;
wc.lpfnWndProc   = OgEditBoxWndProc;
wc.cbClsExtra   = 0;
wc.cbWndExtra   = 0;
wc.hInstance   = hInstance;
wc.hIcon     = LoadIcon(NULL, IDI_APPLICATION);
wc.hCursor     = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = OgEditBoxClassName;
wc.hIconSm     = LoadIcon(NULL, IDI_ASTERISK);

if(!RegisterClassEx(&wc)) {
  MessageBox(NULL, "Window Registration Failed!", "Error!",
    MB_ICONEXCLAMATION | MB_OK);
  return 0;
  }

if (x<0) x=CW_USEDEFAULT;
if (y<0) y=CW_USEDEFAULT;

hwnd = CreateWindowEx(
  0,
  OgEditBoxClassName,
  name,
  WS_OVERLAPPEDWINDOW,
  x, y, width, heigth,
  NULL, NULL, hInstance, NULL);

if(hwnd == NULL) {
  MessageBox(NULL, "Window Creation Failed!", "Error!",
    MB_ICONEXCLAMATION | MB_OK);
  return 0;
  }

ShowWindow(hwnd, nCmdShow);
UpdateWindow(hwnd);

while(GetMessage(&Msg, NULL, 0, 0) > 0) {
  TranslateMessage(&Msg);
  DispatchMessage(&Msg);
  }
return Msg.wParam;
}


