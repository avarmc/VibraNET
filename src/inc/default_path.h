#pragma once

inline CString GetDefaultPath()
{
	TCHAR fileName[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, fileName, MAX_PATH + 1);
	LPWSTR pS = StrRChr(fileName, NULL, '\\');
	if (pS)
		pS[1] = 0;
	return fileName;
}