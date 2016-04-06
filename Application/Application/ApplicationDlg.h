
// ApplicationDlg.h : файл заголовка
//

#pragma once
#include "afxwin.h"
#include "ResultDlg.h"

// диалоговое окно CApplicationDlg
class CApplicationDlg : public CDialogEx
{
// Создание
public:
	CApplicationDlg(CWnd* pParent = NULL);	// стандартный конструктор

// Данные диалогового окна
	enum { IDD = IDD_APPLICATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


// Реализация
protected:
	HICON m_hIcon;
	CDC* m_pDC;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//afx_msg void OnBnClickedButton1();
	//void fillStatsField(CDC*);

	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	CScrollBar m_VScroll;
//	afx_msg void OnNMThemeChangedScrollbar1(NMHDR *pNMHDR, LRESULT *pResult);
	CScrollBar m_HScroll;
	afx_msg void OnButtonStart();
	afx_msg void OnBnClickedButtonSettings();
	//afx_msg void OnLbnSelchangeList1();
	//CListBox m_RobotsList;
	afx_msg void OnButtonPause();
	afx_msg void OnBnClickedButtonResume();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnGameEnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRoundEnd(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
};
