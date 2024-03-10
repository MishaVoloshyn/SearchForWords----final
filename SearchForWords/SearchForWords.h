#pragma once

#define SEARCH_FOR_WORDS_H
#include "resource.h"
#include "header.h"

class SearchForWords
{

public:

	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	static SearchForWords* ptr;
	SearchForWords(void);
	~SearchForWords(void);
	
	void Cls_OnClose(HWND hwnd);
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnTimer(HWND hwnd, UINT id);

	void WriteInfo();
	void GetFileInfo(const string& filePath, string& fileName, int& fileSize);
	void GetInfo(const string& filePath);
};