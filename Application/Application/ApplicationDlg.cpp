
// ApplicationDlg.cpp : файл реализации
//

#include "stdafx.h"
#include "Application.h"
#include "ApplicationDlg.h"
#include "afxdialogex.h"
#include "ConfigurationWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// диалоговое окно CApplicationDlg



CApplicationDlg::CApplicationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CApplicationDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CApplicationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCROLLBAR1, m_VScroll);
	DDX_Control(pDX, IDC_SCROLLBAR2, m_HScroll);
	//DDX_Control(pDX, IDC_LIST1, m_RobotsList);
}

BEGIN_MESSAGE_MAP(CApplicationDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//ON_BN_CLICKED(IDC_BUTTON1, &CApplicationDlg::OnBnClickedButton1)
	ON_WM_MOUSEWHEEL()
//	ON_NOTIFY(NM_THEMECHANGED, IDC_SCROLLBAR1, &CApplicationDlg::OnNMThemeChangedScrollbar1)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_START, &CApplicationDlg::OnButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, &CApplicationDlg::OnBnClickedButtonSettings)
	//ON_LBN_SELCHANGE(IDC_LIST1, &CApplicationDlg::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CApplicationDlg::OnButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_RESUME, &CApplicationDlg::OnBnClickedButtonResume)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CApplicationDlg::OnBnClickedButtonStop)
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(WM_ON_GAME_END, &CApplicationDlg::OnGameEnd)
	ON_MESSAGE(WM_ON_ROUND_END, &CApplicationDlg::OnRoundEnd)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// обработчики сообщений CApplicationDlg

BOOL CApplicationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	HBITMAP bit_button = LoadBitmap((HINSTANCE)GetWindowLong(m_hWnd, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDB_BITMAP_SETTINGS));
	SendDlgItemMessage(IDC_BUTTON_SETTINGS, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bit_button);

	bit_button = LoadBitmap((HINSTANCE)GetWindowLong(m_hWnd, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDB_BITMAP_START));
	SendDlgItemMessage(IDC_BUTTON_START, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bit_button);

	bit_button = LoadBitmap((HINSTANCE)GetWindowLong(m_hWnd, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDB_BITMAP_PAUSE));
	SendDlgItemMessage(IDC_BUTTON_PAUSE, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bit_button);

	bit_button = LoadBitmap((HINSTANCE)GetWindowLong(m_hWnd, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDB_BITMAP_RESUME));
	SendDlgItemMessage(IDC_BUTTON_RESUME, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bit_button);

	bit_button = LoadBitmap((HINSTANCE)GetWindowLong(m_hWnd, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDB_BITMAP_STOP));
	SendDlgItemMessage(IDC_BUTTON_STOP, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bit_button);


	m_pDC = this->GetDC();

	CApplicationApp* app = (CApplicationApp*)AfxGetApp();

	app->m_Game.m_pDialog = this;

	this->GetDC()->SetBkMode(TRANSPARENT);
	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CApplicationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();

		CApplicationApp* app = (CApplicationApp*)AfxGetApp();
		CDC* pDC = this->GetDC();
		app->m_Game.m_pDialog = this;
		app->m_Game.m_pDC = pDC;
		//app->m_Game.drawPlayingField(true);
		app->m_Game.draw();
		//app->m_Game.fillStatsField(pDC);
		//app->m_Game.drawStations(pDC);


		//app->m_Game.drawСonditions(pDC);

		m_VScroll.SetScrollRange(0, app->m_Game.m_Config.H - 1);
		m_HScroll.SetScrollRange(0, app->m_Game.m_Config.W - 1);
	}
	
	
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CApplicationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



//void CApplicationDlg::OnBnClickedButton1()
//{
//	ConfigurationWindow configDlg;
//	configDlg.DoModal();
//	//this->RedrawWindow();
//}


BOOL CApplicationDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	RECT Rect;
	GetWindowRect(&Rect);
	pt.x -= Rect.left;
	pt.y -= Rect.top;

	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	if (pt.x > app->m_Game.m_Border_start.x && pt.y > app->m_Game.m_Border_start.y
		&& pt.x < app->m_Game.m_Border_end.x && pt.y < app->m_Game.m_Border_end.y
		){
		if (zDelta < 0){
			app->m_Game.decCellSize();
		}
		else{
			app->m_Game.incCellSize();
		}
	}
	else if (pt.x > app->m_Game.m_Conditions_start.x && pt.y > app->m_Game.m_Conditions_start.y
		&& pt.x < app->m_Game.m_Conditions_end.x && pt.y < app->m_Game.m_Conditions_end.y)
	{
		if (zDelta < 0){
			app->m_Game.incConditionsTopSel();
		}
		else{			
			app->m_Game.decConditionsTopSel();
		}
	}
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}


void CApplicationDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int pos = pScrollBar->GetScrollPos();
	switch (nSBCode)
	{
	case SB_LINEDOWN:
		++pos;
		break;
	case SB_PAGEDOWN:
		++pos;
		break;
	case SB_PAGEUP:
		--pos;
		break;
	case SB_LINEUP:
		--pos;
		break;
	case SB_THUMBTRACK:
		pos = nPos;		
		break;
	default:
		break;
	}
	pScrollBar->SetScrollPos(pos);

	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	CSize curCell = app->m_Game.getCurCell();
	curCell.cy = pos;
	
	app->m_Game.setCurCell(curCell);
}

void CApplicationDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{

	int pos = pScrollBar->GetScrollPos();
	switch (nSBCode)
	{
	case SB_LINERIGHT:
		++pos;
		break;
	case SB_PAGERIGHT:
		++pos;
		break;
	case SB_PAGELEFT:
		--pos;
		break;
	case SB_LINELEFT:
		--pos;
		break;
	case SB_THUMBTRACK:
		pos = nPos;		
		break;
	default:
		break;
	}
	pScrollBar->SetScrollPos(pos);

	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	CSize curCell = app->m_Game.getCurCell();
	curCell.cx = pos;

	app->m_Game.setCurCell(curCell);
}

void CApplicationDlg::OnButtonStart()
{
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	app->m_Game.start();
	GetDlgItem(IDC_BUTTON_START)->ShowWindow(SW_HIDE);
	//GetDlgItem(IDC_BUTTON_SETTINGS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_SETTINGS)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_PAUSE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_STOP)->ShowWindow(SW_SHOW);
}


void CApplicationDlg::OnBnClickedButtonSettings()
{
	ConfigurationWindow configDlg;
	configDlg.DoModal();
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	app->m_Game.draw();
}


void CApplicationDlg::OnButtonPause()
{
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	app->m_Game.pause();
	GetDlgItem(IDC_BUTTON_PAUSE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_RESUME)->ShowWindow(SW_SHOW);
}


void CApplicationDlg::OnBnClickedButtonResume()
{
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	DWORD result = app->m_Game.resume();
	GetDlgItem(IDC_BUTTON_RESUME)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_PAUSE)->ShowWindow(SW_SHOW);
}


void CApplicationDlg::OnBnClickedButtonStop()
{
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	DWORD result = app->m_Game.end();
	GetDlgItem(IDC_BUTTON_PAUSE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_RESUME)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_STOP)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_START)->ShowWindow(SW_SHOW);
	//GetDlgItem(IDC_BUTTON_SETTINGS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_SETTINGS)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC)->ShowWindow(SW_SHOW);
}


void CApplicationDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	if (point.x > app->m_Game.m_Conditions_start.x && point.y > app->m_Game.m_Conditions_start.y
		&& point.x < app->m_Game.m_Conditions_end.x && point.y < app->m_Game.m_Conditions_end.y)
	{
		app->m_Game.setCurSelWithY(point.y);
	}


	

	CDialogEx::OnLButtonDown(nFlags, point);
}


LRESULT CApplicationDlg::OnGameEnd(WPARAM wParam, LPARAM lParam)
{
	GetDlgItem(IDC_BUTTON_PAUSE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_RESUME)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_STOP)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_START)->ShowWindow(SW_SHOW);
	//GetDlgItem(IDC_BUTTON_SETTINGS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_SETTINGS)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC)->ShowWindow(SW_SHOW);

	CResultDlg resultDlg(END_GAME);
	resultDlg.DoModal();
	return 1;
}

LRESULT CApplicationDlg::OnRoundEnd(WPARAM wParam, LPARAM lParam)
{
	CResultDlg resultDlg((USHORT)END_ROUND);
	resultDlg.DoModal();
	return 1;
}

void CApplicationDlg::OnClose()
{
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	DWORD result = app->m_Game.end();
	CDialogEx::OnClose();
}
