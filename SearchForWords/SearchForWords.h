#pragma once
#include "resource.h"
#include "header.h"

class SearchForWords
{
public:
	SearchForWords(void);
	~SearchForWords(void);
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	static SearchForWords* ptr;
	void Cls_OnClose(HWND hwnd);
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnTimer(HWND hwnd, UINT id);
};