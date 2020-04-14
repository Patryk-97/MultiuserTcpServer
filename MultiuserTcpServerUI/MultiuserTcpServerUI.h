
// MultiuserTcpServerUI.h: główny plik nagłówkowy aplikacji PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "dołącz nagłówek „pch.h” przed dołączeniem tego pliku na potrzeby optymalizacji PCH"
#endif

#include "resource.h"		// główne symbole


// CMultiuserTcpServerUIApp:
// Aby uzyskać implementację klasy, zobacz MultiuserTcpServerUI.cpp
//

class CMultiuserTcpServerUIApp : public CWinApp
{
public:
	CMultiuserTcpServerUIApp();

// Przesłania
public:
	virtual BOOL InitInstance();

// Implementacja

	DECLARE_MESSAGE_MAP()
};

extern CMultiuserTcpServerUIApp theApp;
