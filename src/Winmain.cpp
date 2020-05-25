#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"
#include "GBSImport.h"

#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")

#pragma comment(lib, "ComCtl32.lib")

HWND w_hDlg;
OPENFILENAME* pOmf;
FILE* w_file;

wchar_t* w_title=new wchar_t[40];
wchar_t* w_author=new wchar_t[40];
wchar_t* w_copyright=new wchar_t[40];
wchar_t* w_error=new wchar_t[256];
char* w_temp=new char[40];
char* w_trash=new char[40];
char* w_filename8=new char[256];
int w_id;
int w_songs;
int w_track=1;
int w_seconds=120;
bool w_debug=false;
bool w_alltracks=false;
bool w_nosilence=true;

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NSF_Import_Dlg_Proc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

void set_progress(int value)
{
	SendDlgItemMessage(w_hDlg, IDC_PROGRESSINFO, PBM_SETPOS, value, 0);
}

bool start_import()
{
	w_track = GetDlgItemInt(w_hDlg, IDC_TRACK, NULL, FALSE);
	w_seconds = GetDlgItemInt(w_hDlg, IDC_SECONDS, NULL, FALSE);
	w_alltracks = (BST_CHECKED == SendDlgItemMessage(w_hDlg, IDC_ALLTRACKS, BM_GETCHECK, 0, 0));
	w_nosilence = (BST_CHECKED == SendDlgItemMessage(w_hDlg, IDC_NOSILENCE, BM_GETCHECK, 0, 0));

	if(w_track < 1)	w_track=1;
	if(w_track > w_songs) w_track=w_songs;
	if(w_seconds > 500) w_seconds=90;

	//w_seconds = 5;

	if(GBSImport::Import(w_filename8, w_track, w_seconds, w_alltracks, w_nosilence))
		MessageBoxW(NULL,_T("Import successful! ^o^"),_T("Yay"),0);
	else
		MessageBoxW(NULL,_T("Import failed! :("),NULL,0);

	return true;
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE h0, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg; BOOL ret; int error; size_t field=32;

	InitCommonControls();
	
	pOmf = new OPENFILENAME;
	wchar_t filename[256]={0};

	ZeroMemory(pOmf, sizeof(OPENFILENAME));
	pOmf->lStructSize = sizeof(OPENFILENAME);
	pOmf->hwndOwner = NULL;
	pOmf->lpstrFilter = _T("GBS Files (*.gbs)\0*.gbs\0All Files (*.*)\0*.*\0\0");
	pOmf->lpstrFile = filename;
	pOmf->nMaxFile = 256;
	pOmf->lpstrTitle = _T("Select GBS to import");
	pOmf->Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	ret=GetOpenFileName(pOmf);
	error=CommDlgExtendedError();
	wcstombs(w_filename8,pOmf->lpstrFile,256);

	if(ret==0){
		if(error==0) return 0;
		wsprintf(w_error, L"Error code: %X", error);
		return MessageBoxW(NULL,w_error,L"Error",0);		
	}
	
	_wfopen_s(&w_file,pOmf->lpstrFile,_T("rb"));
	fread(&w_id,3,1,w_file);	fread(w_trash,1,1,w_file);
	fread(&w_songs,1,1,w_file);	fread(w_trash,1,11,w_file);
	fread(w_temp,1,32,w_file);	mbstowcs_s(&field,w_title,33,w_temp,32);
	fread(w_temp,1,32,w_file);	mbstowcs_s(&field,w_author,33,w_temp,32);
	fread(w_temp,1,32,w_file);	mbstowcs_s(&field,w_copyright,33,w_temp,32);
	fclose(w_file);
	
	if(!GBSImport::Scan(w_filename8)){
		mbstowcs(w_error,GBSImport::GetError(),255);
		MessageBox(NULL,w_error,_T("Error"),0);
		return -1;
	}

	w_hDlg = CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_DIALOG_NSF_IMPORT),0,NSF_Import_Dlg_Proc,0);
	ShowWindow(w_hDlg, nCmdShow);

	while((ret=GetMessage(&msg,0,0,0))!=0)
	{
		if(ret==-1) return -1;
		if(!IsDialogMessage(w_hDlg, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}

INT_PTR CALLBACK NSF_Import_Dlg_Proc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch(Msg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hWndDlg, IDC_FILENAME, pOmf->lpstrFile);
        SetDlgItemText(hWndDlg, IDC_NSFINFO1, w_title);
        SetDlgItemText(hWndDlg, IDC_NSFINFO2, w_author);
        SetDlgItemText(hWndDlg, IDC_NSFINFO3, w_copyright);
        SetDlgItemInt(hWndDlg, IDC_TRACK, 1, FALSE);
        SetDlgItemInt(hWndDlg, IDC_TRACKCOUNT, w_songs, FALSE);
        SetDlgItemInt(hWndDlg, IDC_SECONDS, w_seconds, FALSE);
        SetDlgItemInt(hWndDlg, IDC_PATTERN, 256, FALSE);
        SetDlgItemInt(hWndDlg, IDC_ROW, 0, FALSE);
		SendDlgItemMessage(hWndDlg, IDC_ALLTRACKS, BM_SETCHECK, w_alltracks? BST_CHECKED: BST_UNCHECKED, 0);
		SendDlgItemMessage(hWndDlg, IDC_NOSILENCE, BM_SETCHECK, w_nosilence? BST_CHECKED: BST_UNCHECKED, 0);
		SendDlgItemMessage(hWndDlg, IDC_PROGRESSINFO, PBM_SETPOS, 0, 0);
		return TRUE;

    case WM_COMMAND:
        switch(wParam){
        case IDOK: start_import(); PostQuitMessage(0); return TRUE;
        case IDCANCEL: PostQuitMessage(-1); return TRUE;
        } break;
    }
    return FALSE;
}