// ResultDlg.cpp: файл реализации
//

#include "stdafx.h"
#include "Application.h"
#include "ResultDlg.h"
#include "afxdialogex.h"


// диалоговое окно CResultDlg

IMPLEMENT_DYNAMIC(CResultDlg, CDialogEx)

CResultDlg::CResultDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CResultDlg::IDD, pParent)
{

}

CResultDlg::CResultDlg(USHORT _type)
	: CDialogEx(CResultDlg::IDD, NULL)
{
	m_Type = _type;
}


CResultDlg::~CResultDlg()
{
}

void CResultDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);

}


BEGIN_MESSAGE_MAP(CResultDlg, CDialogEx)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// обработчики сообщений CResultDlg


int CResultDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect;
	GetClientRect(rect);
	
	if (m_Type == END_ROUND){		
		m_List.Create(/*LVS_ICON |*/ LVS_REPORT | WS_VISIBLE, rect, this, 100);
		m_List.InsertColumn(0, _T("Место"), LVCFMT_CENTER, rect.Width() / 9);
		m_List.InsertColumn(1, _T("Автор"), LVCFMT_LEFT, rect.Width() / 8);
		m_List.InsertColumn(2, _T("Робот"), LVCFMT_LEFT, rect.Width() / 7);
		m_List.InsertColumn(3, _T("Энергия"), LVCFMT_CENTER, rect.Width() / 8);
		m_List.InsertColumn(4, _T("Убийства"), LVCFMT_CENTER, rect.Width() / 8);
		m_List.InsertColumn(5, _T("Шаги"), LVCFMT_CENTER, rect.Width() / 8);
		m_List.InsertColumn(6, _T("Очки"), LVCFMT_CENTER, rect.Width() / 8);
		m_List.InsertColumn(7, _T("Баллы"), LVCFMT_CENTER, rect.Width() / 8);
	}
	else if (m_Type == END_GAME){
		m_List.Create(/*LVS_ICON |*/ LVS_REPORT | WS_VISIBLE, rect, this, 100);
		m_List.InsertColumn(0, _T("Автор"), LVCFMT_LEFT, rect.Width() * 3 / 4);
		m_List.InsertColumn(1, _T("Баллы"), LVCFMT_CENTER, rect.Width() / 4);
	}

	return 0;
}




BOOL CResultDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CApplicationApp* app = (CApplicationApp*)AfxGetApp();

	USHORT num = 0;

	if (m_Type == END_ROUND){
		
		LVCOLUMN Column;
		Column.mask = LVCF_FMT;
		Column.fmt = LVCFMT_CENTER;
		m_List.SetColumn(0, &Column);

		for (auto it = app->m_Game.m_RoundResults.begin(); it != app->m_Game.m_RoundResults.end(); ++it){

			CString str;
			str.Format("%d", (*it)->Rank);
			//m_List.InsertItem(LVIF_TEXT | LVIF_STATE, num, str, 0, LVIS_SELECTED | LVCFMT_CENTER, 0, 0);
			m_List.InsertItem(num, str);

			m_List.SetItemText(num, 1, (*it)->Author.c_str());

			m_List.SetItemText(num, 2, (*it)->Name.c_str());

			str.Format("%d", (*it)->E);
			m_List.SetItemText(num, 3, str);

			str.Format("%d", (*it)->RoundFrags);
			m_List.SetItemText(num, 4, str);

			str.Format("%d", (*it)->RoundAliveSteps);
			m_List.SetItemText(num, 5, str);

			str.Format("%d", (*it)->RoundScore);
			m_List.SetItemText(num, 6, str);

			str.Format("%d", (*it)->Points);
			m_List.SetItemText(num, 7, str);

			++num;
		}
	}
	else if (m_Type == END_GAME){
		for (auto it = app->m_Game.m_GameResults.begin(); it != app->m_Game.m_GameResults.end(); ++it){
			m_List.InsertItem(num, it->first.c_str());

			CString str;
			str.Format("%d", it->second);
			m_List.SetItemText(num, 1, str);

			++num;		
		}
	
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// Исключение: страница свойств OCX должна возвращать значение FALSE
}
