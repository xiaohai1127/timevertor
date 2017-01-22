// TimeVertor
// Copyright (c) 2012-2016 Henry++

#include <windows.h>

#include "main.h"
#include "rapp.h"
#include "routine.h"

#include "resource.h"

rapp app (APP_NAME, APP_NAME_SHORT, APP_VERSION, APP_COPYRIGHT);

#define MAC_TIMESTAMP 2082844800LL

double microsoft_timestamp = 0.0;

VOID _app_print (HWND hwnd)
{
	FILETIME ft = {0};
	SYSTEMTIME st = {0};
	tm tm_ = {0};
	ULARGE_INTEGER ul = {0};
	WCHAR buffer[MAX_PATH] = {0};

	SendDlgItemMessage (hwnd, IDC_INPUT, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);

	__time64_t ut = _r_unixtime_from_systemtime (&st);

	if (ut <= 0)
	{
		ut = _r_unixtime_now ();

		_r_unixtime_to_systemtime (ut, &st);

		SendDlgItemMessage (hwnd, IDC_INPUT, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
	}

	_r_unixtime_to_filetime (ut, &ft);

	gmtime_s (&tm_, &ut);

	ul.LowPart = ft.dwLowDateTime;
	ul.HighPart = ft.dwHighDateTime;

	wcsftime (buffer, _countof (buffer), L"%A, %B %e, %Y %r", &tm_);
	_r_listview_additem (hwnd, IDC_LISTVIEW, buffer, 0, 1);

	wcsftime (buffer, _countof (buffer), L"%a, %d %b %y %T %z", &tm_);
	_r_listview_additem (hwnd, IDC_LISTVIEW, buffer, 1, 1);

	wcsftime (buffer, _countof (buffer), L"%FT%T%z", &tm_);
	_r_listview_additem (hwnd, IDC_LISTVIEW, buffer, 2, 1);

	_r_listview_additem (hwnd, IDC_LISTVIEW, _r_fmt (L"%lld", ut), 3, 1);
	_r_listview_additem (hwnd, IDC_LISTVIEW, _r_fmt (L"%lld", ut + MAC_TIMESTAMP), 4, 1);
	_r_listview_additem (hwnd, IDC_LISTVIEW, _r_fmt (L"%.09f", (double (ul.QuadPart) / (24.0 * (60.0 * (60.0 * 10000000.0)))) - microsoft_timestamp), 5, 1);
	_r_listview_additem (hwnd, IDC_LISTVIEW, _r_fmt (L"%lld", ul.QuadPart), 6, 1);
}

BOOL initializer_callback (HWND hwnd, DWORD msg, LPVOID, LPVOID)
{
	static HWND tooltip = nullptr;

	switch (msg)
	{
		case _RM_INITIALIZE:
		{
			// initialize microsoft timestamp
			SYSTEMTIME st = {0};
			FILETIME ft = {0};
			ULARGE_INTEGER ul = {0};

			st.wDay = 31;
			st.wMonth = 12;
			st.wYear = 1899;

			SystemTimeToFileTime (&st, &ft);

			ul.LowPart = ft.dwLowDateTime;
			ul.HighPart = ft.dwHighDateTime;

			microsoft_timestamp = double (ul.QuadPart) / (24.0 * (60.0 * (60.0 * 10000000.0)));

			break;
		}

		case _RM_LOCALIZE:
		{
			// configure menu
			HMENU menu = GetMenu (hwnd);

			app.LocaleMenu (menu, I18N (&app, IDS_FILE, 0), 0, TRUE);
			app.LocaleMenu (menu, I18N (&app, IDS_EXIT, 0), IDM_EXIT, FALSE);
			app.LocaleMenu (menu, I18N (&app, IDS_SETTINGS, 0), 1, TRUE);
			app.LocaleMenu (menu, I18N (&app, IDS_ALWAYSONTOP_CHK, 0), IDM_ALWAYSONTOP_CHK, FALSE);
			app.LocaleMenu (menu, I18N (&app, IDS_CHECKUPDATES_CHK, 0), IDM_CHECKUPDATES_CHK, FALSE);
			app.LocaleMenu (GetSubMenu (menu, 1), I18N (&app, IDS_LANGUAGE, 0), 3, TRUE);
			app.LocaleMenu (menu, I18N (&app, IDS_HELP, 0), 2, TRUE);
			app.LocaleMenu (menu, I18N (&app, IDS_WEBSITE, 0), IDM_WEBSITE, FALSE);
			app.LocaleMenu (menu, I18N (&app, IDS_DONATE, 0), IDM_DONATE, FALSE);
			app.LocaleMenu (menu, I18N (&app, IDS_CHECKUPDATES, 0), IDM_CHECKUPDATES, FALSE);
			app.LocaleMenu (menu, I18N (&app, IDS_ABOUT, 0), IDM_ABOUT, FALSE);

			app.LocaleEnum ((HWND)GetSubMenu (menu, 1), 3, TRUE, IDM_DEFAULT); // enum localizations

			// configure listview
			_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);

			_r_listview_additem (hwnd, IDC_LISTVIEW, I18N (&app, IDS_FORMAT_TEXT, 0), LAST_VALUE, 0);
			_r_listview_additem (hwnd, IDC_LISTVIEW, I18N (&app, IDS_FORMAT_RFC822, 0), LAST_VALUE, 0);
			_r_listview_additem (hwnd, IDC_LISTVIEW, I18N (&app, IDS_FORMAT_ISO8601, 0), LAST_VALUE, 0);
			_r_listview_additem (hwnd, IDC_LISTVIEW, I18N (&app, IDS_FORMAT_UNIXTIME, 0), LAST_VALUE, 0);
			_r_listview_additem (hwnd, IDC_LISTVIEW, I18N (&app, IDS_FORMAT_MACTIME, 0), LAST_VALUE, 0);
			_r_listview_additem (hwnd, IDC_LISTVIEW, I18N (&app, IDS_FORMAT_MICROSOFTTIME, 0), LAST_VALUE, 0);
			_r_listview_additem (hwnd, IDC_LISTVIEW, I18N (&app, IDS_FORMAT_FILETIME, 0), LAST_VALUE, 0);

			// configure button
			if (tooltip)
			{
				DestroyWindow (tooltip);
				tooltip = nullptr;
			}

			tooltip = _r_ctrl_settip (hwnd, IDC_CURRENT, I18N (&app, IDS_CURRENT, 0));

			_r_wnd_addstyle (hwnd, IDC_CURRENT, app.IsClassicUI () ? WS_EX_STATICEDGE : 0, WS_EX_STATICEDGE, GWL_EXSTYLE);

			_app_print (hwnd); // print time

			RedrawWindow (hwnd, nullptr, nullptr, RDW_ERASENOW | RDW_INVALIDATE);

			break;
		}
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// configure listview
			_r_listview_setstyle (hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, nullptr, 35, 1, LVCFMT_LEFT);
			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, nullptr, 65, 2, LVCFMT_RIGHT);

			// configure controls
			rstring dt_format;
			WCHAR buffer[MAX_PATH] = {0};

			GetLocaleInfo (LOCALE_SYSTEM_DEFAULT, LOCALE_SSHORTDATE, buffer, _countof (buffer));
			dt_format.Append (buffer);
			dt_format.Append (L", ");

			GetLocaleInfo (LOCALE_SYSTEM_DEFAULT, LOCALE_STIMEFORMAT, buffer, _countof (buffer));
			dt_format.Append (buffer);

			SendDlgItemMessage (hwnd, IDC_INPUT, DTM_SETFORMAT, 0, (LPARAM)dt_format.GetBuffer ());
			dt_format.Clear ();

			SYSTEMTIME st = {0};
			_r_unixtime_to_systemtime (app.ConfigGet (L"LatestTimestamp", 0).AsLonglong (), &st);
			SendDlgItemMessage (hwnd, IDC_INPUT, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);

			// configure menu
			CheckMenuItem (GetMenu (hwnd), IDM_ALWAYSONTOP_CHK, MF_BYCOMMAND | (app.ConfigGet (L"AlwaysOnTop", 0).AsBool () ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem (GetMenu (hwnd), IDM_CHECKUPDATES_CHK, MF_BYCOMMAND | (app.ConfigGet (L"CheckUpdates", 1).AsBool () ? MF_CHECKED : MF_UNCHECKED));

			break;
		}

		case WM_DESTROY:
		{
			SYSTEMTIME st = {0};

			SendDlgItemMessage (hwnd, IDC_INPUT, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);

			app.ConfigSet (L"LatestTimestamp", _r_unixtime_from_systemtime (&st));

			PostQuitMessage (0);

			break;
		}

		case WM_QUERYENDSESSION:
		{
			SetWindowLongPtr (hwnd, DWLP_MSGRESULT, TRUE);
			return TRUE;
		}

		case WM_CONTEXTMENU:
		{
			if (GetDlgCtrlID ((HWND)wparam) == IDC_LISTVIEW)
			{
				HMENU menu = LoadMenu (nullptr, MAKEINTRESOURCE (IDM_LISTVIEW)), submenu = GetSubMenu (menu, 0);

				// localize
				app.LocaleMenu (submenu, I18N (&app, IDS_COPY, 0), IDM_COPY, FALSE);

				if (!SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_GETSELECTEDCOUNT, 0, 0))
				{
					EnableMenuItem (submenu, IDM_COPY, MF_BYCOMMAND | MF_DISABLED);
				}

				TrackPopupMenuEx (submenu, TPM_RIGHTBUTTON | TPM_LEFTBUTTON, LOWORD (lparam), HIWORD (lparam), hwnd, nullptr);

				DestroyMenu (menu);
				DestroyMenu (submenu);
			}

			break;
		}

		case WM_NOTIFY:
		{
			switch (LPNMHDR (lparam)->code)
			{
				case DTN_USERSTRING:
				{
					LPNMDATETIMESTRING lpds = (LPNMDATETIMESTRING)lparam;

					rstring datetime = lpds->pszUserString;

					if (datetime.IsNumeric ())
					{
						_r_unixtime_to_systemtime (datetime.AsLonglong (), &lpds->st);
					}

					break;
				}

				case DTN_DATETIMECHANGE:
				case DTN_CLOSEUP:
				{
					_app_print (hwnd);
					break;
				}

				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP lpnmgit = (LPNMLVGETINFOTIP)lparam;

					StringCchCopy (lpnmgit->pszText, lpnmgit->cchTextMax, I18N (&app, IDS_FORMAT_DESCRIPTION_1 + lpnmgit->iItem, _r_fmt (L"IDS_FORMAT_DESCRIPTION_%d", lpnmgit->iItem + 1)));

					break;
				}
			}

			break;
		}

		case WM_COMMAND:
		{
			if (HIWORD (wparam) == 0 && LOWORD (wparam) >= IDM_DEFAULT && LOWORD (wparam) <= IDM_DEFAULT + app.LocaleGetCount ())
			{
				app.LocaleApplyFromMenu (GetSubMenu (GetSubMenu (GetMenu (hwnd), 1), 3), LOWORD (wparam), IDM_DEFAULT);

				return FALSE;
			}

			switch (LOWORD (wparam))
			{
				case IDCANCEL: // process Esc key
				case IDM_EXIT:
				{
					DestroyWindow (hwnd);
					break;
				}

				case IDM_ALWAYSONTOP_CHK:
				{
					BOOL val = app.ConfigGet (L"AlwaysOnTop", 0).AsBool ();

					CheckMenuItem (GetMenu (hwnd), IDM_ALWAYSONTOP_CHK, MF_BYCOMMAND | (!val ? MF_CHECKED : MF_UNCHECKED));
					app.ConfigSet (L"AlwaysOnTop", !val);

					_r_wnd_top (hwnd, !val);

					break;
				}

				case IDM_CHECKUPDATES_CHK:
				{
					BOOL val = app.ConfigGet (L"CheckUpdates", 1).AsBool ();

					CheckMenuItem (GetMenu (hwnd), IDM_CHECKUPDATES_CHK, MF_BYCOMMAND | (!val ? MF_CHECKED : MF_UNCHECKED));
					app.ConfigSet (L"CheckUpdates", !val);

					break;
				}

				case IDM_WEBSITE:
				{
					ShellExecute (hwnd, nullptr, _APP_WEBSITE_URL, nullptr, nullptr, SW_SHOWDEFAULT);
					break;
				}

				case IDM_DONATE:
				{
					ShellExecute (hwnd, nullptr, _APP_DONATION_URL, nullptr, nullptr, SW_SHOWDEFAULT);
					break;
				}

				case IDM_CHECKUPDATES:
				{
					app.CheckForUpdates (FALSE);
					break;
				}

				case IDM_ABOUT:
				{
					app.CreateAboutWindow ();
					break;
				}

				case IDM_COPY:
				{
					rstring buffer;

					INT item = -1;

					while ((item = (INT)SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_GETNEXTITEM, item, LVNI_SELECTED)) != -1)
					{
						buffer.Append (_r_listview_gettext (hwnd, IDC_LISTVIEW, item, 1));
						buffer.Append (L"\r\n");
					}

					if (!buffer.IsEmpty ())
					{
						buffer.Trim (L"\r\n");

						_r_clipboard_set (hwnd, buffer, buffer.GetLength ());
					}

					break;
				}

				case IDC_CURRENT:
				{
					SYSTEMTIME st = {0};

					_r_unixtime_to_systemtime (_r_unixtime_now (), &st);

					SendDlgItemMessage (hwnd, IDC_INPUT, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);

					_app_print (hwnd);

					break;
				}
			}
		}
	}

	return FALSE;
}

INT APIENTRY wWinMain (HINSTANCE, HINSTANCE, LPWSTR, INT)
{
	if (app.CreateMainWindow (&DlgProc, &initializer_callback))
	{
		MSG msg = {0};

		while (GetMessage (&msg, nullptr, 0, 0) > 0)
		{
			if (!IsDialogMessage (app.GetHWND (), &msg))
			{
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
		}
	}

	return ERROR_SUCCESS;
}
