#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "Resource.h"

#define ESM_POKECODEANDLOOKUP (WM_USER + 100)
const TCHAR g_szAppName[] = TEXT("Error Show");

BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SendMessage(
		hwnd,
		WM_SETICON,
		TRUE,
		(LPARAM)LoadIcon(
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			MAKEINTRESOURCE(IDI_ERRORSHOW)));
	SendMessage(
		hwnd,
		WM_SETICON,
		FALSE,
		(LPARAM)LoadIcon(
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			MAKEINTRESOURCE(IDI_ERRORSHOW)));

	Edit_LimitText(GetDlgItem(hwnd, IDC_ERRORCODE), 5);
	SendMessage(hwnd, ESM_POKECODEANDLOOKUP, lParam, 0);

	return(TRUE);
}

void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
		case IDCANCEL:
			EndDialog(hwnd, id);
			break;

		case IDC_ALWAYSONTOP:
			SetWindowPos(
				hwnd,
				IsDlgButtonChecked(hwnd, IDC_ALWAYSONTOP)
					? HWND_TOPMOST
					: HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			break;

		case IDC_ERRORCODE:
			EnableWindow(GetDlgItem(hwnd, IDOK), Edit_GetTextLength(hwndCtl) > 0);
			break;

		case IDOK:
			DWORD dwError = GetDlgItemInt(hwnd, IDC_ERRORCODE, NULL, FALSE);

			HLOCAL hlocal = NULL;

			BOOL fOk = FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
				NULL,
				dwError,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
				(PTSTR)& hlocal,
				0,
				NULL);
			
			if (!fOk)
			{
				HMODULE hDll = LoadLibraryEx(
					TEXT("netmsg.dll"),
					NULL,
					DONT_RESOLVE_DLL_REFERENCES);

				if (hDll != NULL)
				{
					FormatMessage(
						FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
						hDll,
						dwError,
						MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
						(PTSTR)& hlocal,
						0,
						NULL);
					FreeLibrary(hDll);
				}
			}

			if (hlocal != NULL)
			{
				SetDlgItemText(hwnd, IDC_ERRORTEXT, (PCTSTR)LocalLock(hlocal));
				LocalFree(hlocal);
			}
			else
			{
				SetDlgItemText(hwnd, IDC_ERRORTEXT, TEXT("Error number not found."));
			}
	}
}

INT_PTR WINAPI Dlg_Proc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (uMsg)
	{
		SetDlgMsgResult(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, Dlg_OnCommand);

	case ESM_POKECODEANDLOOKUP:
		SetDlgItemInt(hwnd, IDC_ERRORCODE, (UINT)wParam, FALSE);
		FORWARD_WM_COMMAND(hwnd, IDOK, GetDlgItem(hwnd, IDOK), BN_CLICKED, PostMessage);
		SetForegroundWindow(hwnd);
		break;
	}

	return(FALSE);
}

int APIENTRY WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	HWND hwnd = FindWindow(TEXT("#32770"), TEXT("Error Show"));
	if (IsWindow(hwnd))
	{
		SendMessage(
			hwnd,
			ESM_POKECODEANDLOOKUP,
			atoi(lpCmdLine),
			0);
	}
	else
	{
		DialogBoxParam(
			hInstance,
			MAKEINTRESOURCE(IDD_ERRORSHOW),
			NULL,
			Dlg_Proc,
			atoi(lpCmdLine));
	}

	return(0);
}