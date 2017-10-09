/*
 *  Encapsulation of OgMessageBox function
 *  Copyright (c) 2000  Ogmios
 *  Dev : March 2000
 *  Version 1.0
*/
#include <loggen.h>


#if (DPcSystem == DPcSystemUnix)

PUBLIC(void) OgMessageBox(void *hwnd, char *lptext, char *lpcaption, unsigned int utype)
{
  char *str_type;
  switch (utype)
  {
    case DOgMessageBoxInformation:
      str_type = "information";
      break;
    case DOgMessageBoxQuestion:
      str_type = "question";
      break;
    case DOgMessageBoxWarning:
      str_type = "warning";
      break;
    case DOgMessageBoxError:
      str_type = "error";
      break;
    default:
      str_type = "error";
      break;
  }

  if (DOgMessageBoxInformation == utype)
  {
    printf("%s\n", lptext);
  }
  else
  {
    printf("%s (%s):\n%s\n", lpcaption, str_type, lptext);
  }
}

#else
#if (DPcSystem == DPcSystemWin32)

#include <windows.h>

PUBLIC(void) OgMessageBox(hwnd,lptext,lpcaption,utype)
void *hwnd;
char *lptext;
char *lpcaption;
unsigned utype;
{
unsigned icon_type;
switch(utype) {
 case DOgMessageBoxInformation: icon_type=MB_OK|MB_ICONINFORMATION; break;
 case DOgMessageBoxQuestion: icon_type=MB_OK|MB_ICONQUESTION; break;
 case DOgMessageBoxWarning: icon_type=MB_OK|MB_ICONWARNING; break;
 case DOgMessageBoxError: icon_type=MB_OK|MB_ICONERROR; break;
 default: icon_type=MB_OK|MB_ICONERROR; break;
 }
MessageBox((HWND)hwnd,lptext,lpcaption,icon_type);
}

#endif
#endif

