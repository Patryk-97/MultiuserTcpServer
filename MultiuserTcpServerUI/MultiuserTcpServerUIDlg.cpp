
// MultiuserTcpServerUIDlg.cpp: plik implementacji
//

#include "pch.h"
#include "framework.h"
#include "MultiuserTcpServerUI.h"
#include "MultiuserTcpServerUIDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Okno dialogowe CAboutDlg używane na potrzeby informacji o aplikacji

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dane okna dialogowego
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // obsługa DDX/DDV

// Implementacja
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// okno dialogowe CMultiuserTcpServerUIDlg



CMultiuserTcpServerUIDlg::CMultiuserTcpServerUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MULTIUSERTCPSERVERUI_DIALOG, pParent)
	, logs(_T(""))
   , port(7)
   , clientsCount(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	winsockManager = std::make_unique<WinsockManager>();
	serverSocket = nullptr;
	isStartButtonActive = true;
	isStopButtonActive = false;
}

CMultiuserTcpServerUIDlg::~CMultiuserTcpServerUIDlg()
{

}

afx_msg LRESULT CMultiuserTcpServerUIDlg::OnMessage(WPARAM wParam, LPARAM lParam)
{
	if (lParam != NULL)
	{
		CString* wparam = (CString*)wParam;
		CString* lparam = (CString*)lParam;

		CString tempCString;
		tempCString.Format(L"%s%s\r\n", *wparam, *lparam);
		logs = tempCString + logs;
	}
	else
	{
		++clientsCount;
	}
	UpdateData(false);

	return 0;
}

void CMultiuserTcpServerUIDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialogEx::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_EDIT_LOGS, logs);
   DDX_Text(pDX, IDC_EDIT_PORT, port);
   DDX_Text(pDX, IDC_STATIC_CLIENTS_COUNT, clientsCount);
}

BEGIN_MESSAGE_MAP(CMultiuserTcpServerUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
   ON_EN_CHANGE(IDC_EDIT_PORT, &CMultiuserTcpServerUIDlg::OnEnChangeEditPort)
	ON_BN_CLICKED(IDC_BUTTON_START, &CMultiuserTcpServerUIDlg::OnBnClickedButtonStart)
	ON_EN_CHANGE(IDC_EDIT_LOGS, &CMultiuserTcpServerUIDlg::OnEnChangeEditLogs)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMultiuserTcpServerUIDlg::OnBnClickedButtonStop)
	ON_MESSAGE(WM_MESSAGE, OnMessage)
END_MESSAGE_MAP()


// Procedury obsługi komunikatów CMultiuserTcpServerUIDlg

BOOL CMultiuserTcpServerUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Dodaj pozycję „Informacje...” do menu systemowego.

	// Element IDM_ABOUTBOX musi należeć do zakresu poleceń systemowych.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Ustaw ikonę dla tego okna dialogowego. Struktura wykonuje to automatycznie
	//  gdy okno główne aplikacji nie jest oknem dialogowym
	SetIcon(m_hIcon, TRUE);			// Ustaw duże ikony
	SetIcon(m_hIcon, FALSE);		// Ustaw małe ikony

	// TODO: Dodaj tutaj dodatkowe inicjowanie

	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false); // disable stop button

	if (false == winsockManager->startup(DLL_WINSOCK_VERSION))
	{
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(false); // disable start button
		isStartButtonActive = false;
		logs = _T("Winsock initialization error\n") + logs;
		UpdateData(false);
	}

	return TRUE;  // zwracaj wartość TRUE, dopóki fokus nie zostanie ustawiony na formant
}

void CMultiuserTcpServerUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// Jeśli dodasz przycisk minimalizacji do okna dialogowego, będziesz potrzebować poniższego kodu
//  aby narysować ikonę. Dla aplikacji MFC używających modelu dokumentu/widoku
//  to jest wykonywane automatycznie przez strukturę.

void CMultiuserTcpServerUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // kontekst urządzenia dotyczący malowania

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Wyśrodkuj ikonę w prostokącie klienta
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Rysuj ikonę
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// System wywołuje tę funkcję, aby uzyskać kursor wyświetlany podczas przeciągania przez użytkownika
//  zminimalizowane okno.
HCURSOR CMultiuserTcpServerUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMultiuserTcpServerUIDlg::OnEnChangeEditPort()
{
	UpdateData(true);
}


void CMultiuserTcpServerUIDlg::OnBnClickedButtonStart()
{
	if (true == isStartButtonActive)
	{
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(false); isStartButtonActive = false;
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(true); isStopButtonActive = true;
		serverSocket.reset(new ServerSocket());
		if (true == serverSocket->init(IpProtocol::IPV4, TxProtocol::TCP))
		{
			logs = _T("Server socket initialized\r\n") + logs;
		}
		else
		{
			CString tempCString;

			logs = _T("Cannot initialiaze a socket\r\n") + logs;

			tempCString.Format(L"Error: %s\r\n", WinsockManager::getErrorMessage());
			logs = tempCString + logs;

			GetDlgItem(IDC_BUTTON_START)->EnableWindow(false); isStartButtonActive = false;
			GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false); isStopButtonActive = false;
			winsockManager->cleanup();
		}

		if (true == serverSocket->bind(nullptr, (uint16_t)port))
		{
			CString tempCString;
			tempCString.Format(L"Server socket bound. Port: %u\r\n", port);

			logs = tempCString + logs;
		}
		else
		{
			CString tempCString;
			logs = _T("Cannot bind socket server.\r\n") + logs;
			
			tempCString.Format(L"Error: %s\r\n", WinsockManager::getErrorMessage());
			logs = tempCString + logs;

			serverSocket->close();
			logs = _T("Server socket closed\r\n") + logs;

			GetDlgItem(IDC_BUTTON_START)->EnableWindow(false); isStartButtonActive = false;
			GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false); isStopButtonActive = false;
			winsockManager->cleanup();
		}

		if (true == serverSocket->listen(MAX_SOCKETS_CONNECTION))
		{
			CString tempCString;
			tempCString.Format(L"Server socket started listening [max %d connections]\r\n",
				MAX_SOCKETS_CONNECTION);
			logs = tempCString + logs;
		}
		else
		{
			CString tempCString;
			logs = _T("Cannot start listening socket server\r\n") + logs;

			tempCString.Format(L"Error: %s\r\n", WinsockManager::getErrorMessage());
			logs = tempCString + logs;

			serverSocket->close();
			logs = _T("Server socket closed\r\n") + logs;

			GetDlgItem(IDC_BUTTON_START)->EnableWindow(false); isStartButtonActive = false;
			GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false); isStopButtonActive = false;
			winsockManager->cleanup();
		}
	}
	UpdateData(false);

	auto pair = std::make_pair((ServerSocket*)serverSocket.get(), (CDialog*)this);

	std::unique_ptr<WinapiThreadAdaptor> listenThread =
		std::make_unique<WinapiThreadAdaptor>(ServerSocket::listenThread, &pair);
}

void CMultiuserTcpServerUIDlg::OnEnChangeEditLogs()
{

}


void CMultiuserTcpServerUIDlg::OnBnClickedButtonStop()
{
	if (true == isStopButtonActive)
	{
		serverSocket->winapiMutex->lock();
		serverSocket->isStopped = true;
		logs = _T("Closing...\r\n") + logs;
		serverSocket->winapiMutex->unlock();
		WinapiThreadAdaptor::sleep(1000);
		serverSocket->close();
		logs = _T("Server socket closed\r\n") + logs;
		winsockManager->cleanup();
		UpdateData(false);

		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false); isStopButtonActive = false;
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(true); isStartButtonActive = true;
	}
}
