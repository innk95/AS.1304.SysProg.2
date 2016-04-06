// ConfigurationWindow.cpp: файл реализации
//

#include "stdafx.h"
#include "Application.h"
#include "ConfigurationWindow.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// диалоговое окно ConfigurationWindow

IMPLEMENT_DYNAMIC(ConfigurationWindow, CDialogEx)

ConfigurationWindow::ConfigurationWindow(CWnd* pParent /*=NULL*/)
	: CDialogEx(ConfigurationWindow::IDD, pParent)
{
	comboBoxSelect = 0;
}

ConfigurationWindow::~ConfigurationWindow()
{
	this->DestroyWindow();

}

void ConfigurationWindow::fillEdits(UINT numberRound)
{
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	char* tmp = new char[100];
	GameConfig config = tempConfigStore->getRoundConfig(numberRound);
	itoa(config.W, tmp, 10);
	W_edit.SetWindowText(tmp);

	itoa(config.H, tmp, 10);
	H_edit.SetWindowText(tmp);

	itoa(config.N, tmp, 10);
	N_edit.SetWindowText(tmp);

	itoa(config.T, tmp, 10);
	T_edit.SetWindowText(tmp);

	itoa(config.E_max, tmp, 10);
	E_max_edit.SetWindowText(tmp);

	itoa(config.L_max, tmp, 10);
	L_max_edit.SetWindowText(tmp);

	itoa(config.V_max, tmp, 10);
	V_max_edit.SetWindowText(tmp);

	itoa(config.R_max, tmp, 10);
	R_max_edit.SetWindowText(tmp);

	itoa(config.dL, tmp, 10);
	dL_edit.SetWindowText(tmp);

	itoa(config.dE_S, tmp, 10);
	dE_s_edit.SetWindowText(tmp);

	itoa(config.dE_V, tmp, 10);
	dE_v_edit.SetWindowText(tmp);

	itoa(config.dE_A, tmp, 10);
	dE_a_edit.SetWindowText(tmp);

	itoa(config.dE_P, tmp, 10);
	dE_p_edit.SetWindowText(tmp);

	itoa(config.dE, tmp, 10);
	dE_edit.SetWindowText(tmp);

	itoa(config.N_E, tmp, 10);
	N_e_edit.SetWindowText(tmp);

	itoa(config.N_L, tmp, 10);
	N_L_edit.SetWindowText(tmp);

	sprintf(tmp, "%f", config.RND_min);
	RND_min_edit.SetWindowText(tmp);

	sprintf(tmp, "%f", config.RND_max);
	RND_max_edit.SetWindowText(tmp);

	itoa(config.K, tmp, 10);
	K_edit.SetWindowText(tmp);

	filePath_edit.SetWindowText(app->m_Game.getFilePath());
	
	delete tmp;
}

BOOL ConfigurationWindow::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	tempConfigStore = app->m_Game.getConfigStore();

	m_ComboBox.AddString("Round 1");
	m_ComboBox.AddString("Round 2");
	m_ComboBox.AddString("Round 3");
	m_ComboBox.AddString("Round 4");
	m_ComboBox.AddString("Round 5");
	m_ComboBox.SetCurSel(0);

	fillEdits(0);
	return 1;
}

void ConfigurationWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, W_edit);
	DDX_Control(pDX, IDC_EDIT2, H_edit);
	DDX_Control(pDX, IDC_EDIT3, N_edit);
	DDX_Control(pDX, IDC_EDIT4, T_edit);
	DDX_Control(pDX, IDC_EDIT5, E_max_edit);
	DDX_Control(pDX, IDC_EDIT6, L_max_edit);
	DDX_Control(pDX, IDC_EDIT7, V_max_edit);
	DDX_Control(pDX, IDC_EDIT8, R_max_edit);
	DDX_Control(pDX, IDC_EDIT9, dL_edit);
	DDX_Control(pDX, IDC_EDIT10, dE_s_edit);
	DDX_Control(pDX, IDC_EDIT11, dE_v_edit);
	DDX_Control(pDX, IDC_EDIT12, dE_a_edit);
	DDX_Control(pDX, IDC_EDIT13, dE_p_edit);
	DDX_Control(pDX, IDC_EDIT14, dE_edit);
	DDX_Control(pDX, IDC_EDIT15, N_e_edit);
	DDX_Control(pDX, IDC_EDIT16, N_L_edit);
	DDX_Control(pDX, IDC_EDIT17, RND_min_edit);
	DDX_Control(pDX, IDC_EDIT18, RND_max_edit);
	DDX_Control(pDX, IDC_EDIT19, K_edit);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, filePath_edit);

	DDX_Control(pDX, IDC_COMBO1, m_ComboBox);
}


BEGIN_MESSAGE_MAP(ConfigurationWindow, CDialogEx)
	ON_BN_CLICKED(IDOK, &ConfigurationWindow::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_MFCEDITBROWSE2, &ConfigurationWindow::OnEnChangeMfceditbrowse2)
	ON_BN_CLICKED(IDC_BUTTON4, &ConfigurationWindow::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON2, &ConfigurationWindow::OnBnClickedButton2)
	ON_CBN_SELCHANGE(IDC_COMBO1, &ConfigurationWindow::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()

void ConfigurationWindow::setTempConfigRound(UINT roundNumber)
{
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	CString tmp;
	GameConfig config = app->m_Game.getRoundConfig(roundNumber);

	W_edit.GetWindowTextA(tmp);
	config.W = atoi(tmp);

	H_edit.GetWindowTextA(tmp);
	config.H = atoi(tmp);

	N_edit.GetWindowTextA(tmp);
	config.N = atoi(tmp);

	T_edit.GetWindowTextA(tmp);
	config.T = atoi(tmp);

	E_max_edit.GetWindowTextA(tmp);
	config.E_max = atoi(tmp);

	L_max_edit.GetWindowTextA(tmp);
	config.L_max = atoi(tmp);

	V_max_edit.GetWindowTextA(tmp);
	config.V_max = atoi(tmp);

	R_max_edit.GetWindowTextA(tmp);
	config.R_max = atoi(tmp);

	dL_edit.GetWindowTextA(tmp);
	config.dL = atoi(tmp);

	dE_s_edit.GetWindowTextA(tmp);
	config.dE_S = atoi(tmp);

	dE_v_edit.GetWindowTextA(tmp);
	config.dE_V = atoi(tmp);

	dE_a_edit.GetWindowTextA(tmp);
	config.dE_A = atoi(tmp);

	dE_p_edit.GetWindowTextA(tmp);
	config.dE_P = atoi(tmp);

	dE_edit.GetWindowTextA(tmp);
	config.dE = atoi(tmp);

	N_e_edit.GetWindowTextA(tmp);
	config.N_E = atoi(tmp);

	N_L_edit.GetWindowTextA(tmp);
	config.N_L = atoi(tmp);

	RND_min_edit.GetWindowTextA(tmp);
	config.RND_min = atof(tmp);

	RND_max_edit.GetWindowTextA(tmp);
	config.RND_max = atof(tmp);

	K_edit.GetWindowTextA(tmp);
	config.K = atoi(tmp);

	tempConfigStore->changeRoundConfig(roundNumber, config);
}

bool isFail = false;
void ConfigurationWindow::OnBnClickedOk()
{
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	//CWnd* wnd = this->GetParent();
	//INT oldW, oldH, oldN_E, oldN_L;
	//f (wnd) {
	//	oldW = app->m_Game.m_Config.W;
	//	oldH = app->m_Game.m_Config.H;
	//	oldN_E = app->m_Game.m_Config.N_E;
	//	oldN_L = app->m_Game.m_Config.N_L;
	//
	CString tmp;
	if (filePath_edit.GetWindowTextLengthA() != 0) {
		filePath_edit.GetWindowTextA(tmp);
		app->m_Game.setFilePath(tmp);
	}
	else {
		AfxMessageBox("Select the file!");
		return;
	}

	UINT selectRound = m_ComboBox.GetCurSel();
	setTempConfigRound(comboBoxSelect);
	app->m_Game.setConfigStore(tempConfigStore);
	
	if (app->m_Game.setRobotsInfo(app->m_Game.getFilePath())) {
		CDialogEx::OnOK();
	}

	app->m_Game.m_Config = app->m_Game.getRoundConfig(0);

	//if (wnd) {
	//	if (oldW != app->m_Game.m_Config.W || oldH != app->m_Game.m_Config.H ||
	//		oldN_E != app->m_Game.m_Config.N_E || oldN_L != app->m_Game.m_Config.N_L || isFail) {
	//		if (app->m_Game.setStations()) {
	//			isFail = false;
	//			wnd->RedrawWindow();
	//			CDialogEx::OnOK();
	//		}
	//		else
	//		isFail = true;
	//	}
	//	else {
	//		CDialogEx::OnOK();
	//	}
	//}
	//else {
	//	if (app->m_Game.setRobotsInfo(app->m_Game.getFilePath())) {
	//		CDialogEx::OnOK();
	//	}
	//}
	// TODO: добавьте свой код обработчика уведомлений
}


afx_msg void ConfigurationWindow::OnClose()
{
	CWnd* wnd = this->GetParent();
	//if (!wnd) {
	//	exit(0);
	//}
	CWnd::OnClose();

}

void ConfigurationWindow::OnEnChangeMfceditbrowse2()
{

}

//сохранение настроек
void ConfigurationWindow::OnBnClickedButton4()
{
	CString path;
	GetDlgItem(IDC_MFCEDITBROWSE3)->GetWindowTextA(path);

	UINT selectRound = m_ComboBox.GetCurSel();
	setTempConfigRound(comboBoxSelect);
	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	CFile myfile;
	if (!myfile.Open(path, CFile::modeCreate | CFile::modeWrite)) {
		AfxMessageBox("Can not open file for recording!");
		return;
	}

	CArchive archive(&myfile, CArchive::store);
	archive << tempConfigStore;
	AfxMessageBox("Done!");
}

//Загрузка настроек
void ConfigurationWindow::OnBnClickedButton2()
{
	UINT selectRound = m_ComboBox.GetCurSel();

	CString settingsFilePath;
	GetDlgItem(IDC_MFCEDITBROWSE2)->GetWindowTextA(settingsFilePath);

	CFile settingsFile;
	if (!settingsFile.Open(settingsFilePath, CFile::modeRead)) {
		AfxMessageBox("Can not open file for reading!");
		return;
	}

	CApplicationApp* app = (CApplicationApp*)AfxGetApp();
	CArchive archive(&settingsFile, CArchive::load);
	ConfigStore* configStore;

	archive >> configStore;

	tempConfigStore = configStore;
	fillEdits(selectRound);
	AfxMessageBox("Done!");
}


void ConfigurationWindow::OnCbnSelchangeCombo1()
{
	UINT selectRound = m_ComboBox.GetCurSel();
	setTempConfigRound(comboBoxSelect);
	fillEdits(selectRound);
	comboBoxSelect = selectRound;
}
