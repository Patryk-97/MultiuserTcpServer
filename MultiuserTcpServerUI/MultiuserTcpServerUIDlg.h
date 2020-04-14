
// MultiuserTcpServerUIDlg.h: plik nagłówkowy
//

#pragma once

#include "WinsockManager.h"
#include "ServerSocket.h"

#include <stdint.h>
#include <memory>

#define DLL_WINSOCK_VERSION MAKEWORD(2, 2)
#define MAX_SOCKETS_CONNECTION 1

#define WM_YOUR_MESSAGE (WM_USER + 10)

// okno dialogowe CMultiuserTcpServerUIDlg
class CMultiuserTcpServerUIDlg : public CDialogEx
{
// Konstrukcja
public:
	CMultiuserTcpServerUIDlg(CWnd* pParent = nullptr);	// konstruktor standardowy
	~CMultiuserTcpServerUIDlg();	// destruktor

	//the message handler
	afx_msg LRESULT OnYourMessage(WPARAM wParam, LPARAM lParam);

// Dane okna dialogowego
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MULTIUSERTCPSERVERUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// obsługa DDX/DDV


// Implementacja
protected:
	HICON m_hIcon;

	// Wygenerowano funkcje mapy komunikatów
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString logs;
	std::unique_ptr<WinsockManager> winsockManager;
	std::unique_ptr<ServerSocket> serverSocket;
   afx_msg void OnEnChangeEditPort();
   afx_msg void OnBnClickedButtonStart();
   afx_msg void OnEnChangeEditLogs();
	bool isStartButtonActive;
	bool isStopButtonActive;
   uint32_t port;
	afx_msg void OnBnClickedButtonStop();
};
