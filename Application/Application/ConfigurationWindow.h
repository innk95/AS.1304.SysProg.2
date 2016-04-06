#pragma once
#include "afxwin.h"
#include "afxeditbrowsectrl.h"
//#include "ApplicationDlg.h"


// диалоговое окно ConfigurationWindow

class ConfigurationWindow : public CDialogEx
{
	DECLARE_DYNAMIC(ConfigurationWindow)

public:
	ConfigurationWindow(CWnd* pParent = NULL);   // стандартный конструктор
	virtual ~ConfigurationWindow();

// Данные диалогового окна
	enum { IDD = IDD_DIALOG1 };

protected:
	void fillEdits(UINT numberRound);
	void setTempConfigRound(UINT numberRound);
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	BOOL OnInitDialog();

	ConfigStore* tempConfigStore;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
	UINT comboBoxSelect;
	CEdit W_edit;
	CEdit H_edit;
	CEdit N_edit;
	CEdit T_edit;
	CEdit E_max_edit;
	CEdit L_max_edit;
	CEdit V_max_edit;
	CEdit R_max_edit;
	CEdit dL_edit;
	CEdit dE_s_edit;
	CEdit dE_v_edit;
	CEdit dE_a_edit;
	CEdit dE_p_edit;
	CEdit dE_edit;
	CEdit N_e_edit;
	CEdit N_L_edit;
	CEdit RND_min_edit;
	CEdit RND_max_edit;
	CEdit K_edit;
	CMFCEditBrowseCtrl filePath_edit;
	CComboBox m_ComboBox;
	afx_msg void OnEnChangeMfceditbrowse2();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnCbnSelchangeCombo1();
};
