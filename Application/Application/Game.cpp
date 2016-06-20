#include "stdafx.h"
#include "Game.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CONDITIONS_START_X			670
#define CONDITIONS_START_Y			220
#define CONDITIONS_WIDTH			300
#define CONDITIONS_HEIGHT			20
#define CONDITIONS_SPACE			5
#define CONDITIONS_TOTAL_HEIGHT		370

#define LEFT_SIDE_OF_STATS			670
#define RIGHT_SIDE_OF_STATS			(LEFT_SIDE_OF_STATS + CONDITIONS_WIDTH)
#define HEIGHT_PER_FIELD			180

#define MIN_CELL_SIZE				2
#define MAX_CELL_SIZE				50

#define MAX_HEIGHT					600
#define MAX_WIDTH					600


IMPLEMENT_SERIAL(ConfigStore, CObject, 1);

void ConfigStore::Serialize(CArchive& ar){
	CObject::Serialize(ar);
	if (ar.IsStoring()) {
		for (int i = 0; i < 5; ++i) {
			ar << m_Store.at(i).W << m_Store.at(i).H << m_Store.at(i).N << m_Store.at(i).T << 
				m_Store.at(i).E_max << m_Store.at(i).L_max << m_Store.at(i).V_max << m_Store.at(i).R_max <<
				m_Store.at(i).dL << m_Store.at(i).dE_S << m_Store.at(i).dE_V << m_Store.at(i).dE_A <<
				m_Store.at(i).dE_P << m_Store.at(i).dE << m_Store.at(i).N_E << m_Store.at(i).N_L <<
				m_Store.at(i).RND_min << m_Store.at(i).RND_max << m_Store.at(i).K;
		}
	}
	else {
		for (int i = 0; i < 5; ++i) {
			GameConfig gameConfig;
			ar >> gameConfig.W >> gameConfig.H >> gameConfig.N >> gameConfig.T >>
				gameConfig.E_max >> gameConfig.L_max >> gameConfig.V_max >> gameConfig.R_max >>
				gameConfig.dL >> gameConfig.dE_S >> gameConfig.dE_V >> gameConfig.dE_A >>
				gameConfig.dE_P >> gameConfig.dE >> gameConfig.N_E >> gameConfig.N_L >>
				gameConfig.RND_min >> gameConfig.RND_max >> gameConfig.K;
			m_Store.push_back(gameConfig);
		}
	}
}

Game::Game()
{	
	setDefaultParameters();
	srand(time(NULL) + clock());
	InitializeCriticalSection(&m_CS_Draw);
	m_hPauseDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hPausePlayEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	//m_hConfirmPausePlay = CreateEvent(NULL, FALSE, FALSE, NULL);
	//m_hConfirmPauseDraw = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hResumeDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hResumePlayEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEndDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hConfirmEndDraw = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hConfirmEnd = CreateEvent(NULL, FALSE, FALSE, NULL);
	//InitializeCriticalSection(&m_CS_ChangeVar);

	
	m_Directory = new char[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, m_Directory);
}

//Game::Game(CDialogEx* pDlg)
//{
//	m_pDialog = pDlg;
//	//m_pDC = m_pDialog->GetDC();
//}

Game::~Game()
{
	if (m_pField){
		delete m_pField;
 	}	

	clearRoundLogList();

	for (auto it = m_Robots_global.begin(); it != m_Robots_global.end(); ++it){
		if ((*it)->hDll){
			FreeLibrary((*it)->hDll);
		}
		if (*it)
			delete *it;
	}

	if (m_ConfigStore) {
		delete m_ConfigStore;
	}

	if (m_Directory){
		delete m_Directory;
	}
	CloseHandle(m_hPlayThread);
	CloseHandle(m_hEndEvent);
	CloseHandle(m_hDrawThread);
	CloseHandle(m_hPauseDrawEvent);
	CloseHandle(m_hPausePlayEvent);
	//CloseHandle(m_hConfirmPauseDraw);
	//CloseHandle(m_hConfirmPausePlay);
	CloseHandle(m_hResumeDrawEvent);
	CloseHandle(m_hResumePlayEvent);

	DeleteCriticalSection(&m_CS_Draw);
	//DeleteCriticalSection(&m_CS_ChangeVar);

}

void ConfigStore::addRoundConfig(GameConfig gameConfig)
{
	m_Store.push_back(gameConfig);
}
void ConfigStore::changeRoundConfig(UINT roundNumber, GameConfig gameConfig)
{
	m_Store[roundNumber] = gameConfig;
}
GameConfig ConfigStore::getRoundConfig(UINT numberRound)
{
	return m_Store.at(numberRound);
}


void Game::setDefaultParameters()
{
	m_ConfigStore = new ConfigStore();
	for (int i = 0; i < 5; ++i) {
		GameConfig config;
		config.W = 200;
		config.H = 200;
		config.N = 1000;
		config.T = 100;
		config.E_max = 1000;
		config.L_max = 100;
		config.V_max = 20;
		config.R_max = 5;
		config.dL = 10;
		config.dE_S = 1;
		config.dE_V = 2; 
		//config.dE_V = 20;
		config.dE_A = 10;
		config.dE_P = 5;
		config.dE = 100;
		config.N_E = 10;
		config.N_L = 10;
		config.RND_min = 0.4;
		config.RND_max = 0.8;
		config.K = 100;
		m_ConfigStore->addRoundConfig(config);
	}
}

void Game::setConfigStore(ConfigStore* _configStore)
{
	m_ConfigStore = _configStore;
}


ConfigStore* Game::getConfigStore()
{
	return m_ConfigStore;
}

GameConfig Game::getRoundConfig(UINT numberRound)
{
	return m_ConfigStore->getRoundConfig(numberRound);
}


void Game::drawPlayingField(bool flag/* = 0*/)
{
	////if (!flag){
	//	if (clock() - m_lastRender < RENDER_PAUSE){
	//		return;
	//	}
	////}


	LOGFONT lf;
	lf.lfHeight = 14;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = PROOF_QUALITY;
	lf.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
	strcpy(lf.lfFaceName, "Times New Roman");
	
	LOGFONT lf2;
	lf2.lfHeight = 12;
	lf2.lfWidth = 0;
	lf2.lfEscapement = 900;
	lf2.lfOrientation = 900;
	lf2.lfWeight = FW_NORMAL;
	lf2.lfItalic = 0;
	lf2.lfUnderline = 0;
	lf2.lfStrikeOut = 0;
	lf2.lfCharSet = ANSI_CHARSET;
	lf2.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf2.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf2.lfQuality = PROOF_QUALITY;
	lf2.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
	strcpy(lf2.lfFaceName, "Times New Roman");


	if (m_pDC == NULL){
		AfxMessageBox("m_pDC is NULL");
		return;
	}

	CFont ft;
	ft.CreateFontIndirect(&lf);
	CFont* oldf = m_pDC->GetCurrentFont();
	m_pDC->SelectObject(&ft);
	
	CFont ft2;
	ft2.CreateFontIndirect(&lf2);



	UINT start_x = 0;
	UINT start_y = 0;

	//TEXTMETRIC textmetric;
	//m_pDC->GetTextMetrics(&textmetric);
	////UINT txtHeight = textmetric.tmHeight;
	UINT txtHeight = 30;
	UINT txtWidth = 30;

	CPen* oldPen = m_pDC->GetCurrentPen();
	CBrush* oldBrush = m_pDC->GetCurrentBrush();
	this->clearArea(Color::GRAY, start_x, start_y, start_x + txtWidth + MAX_WIDTH, start_y + txtHeight + MAX_HEIGHT);
	m_pDC->SelectObject(oldPen);
	m_pDC->SelectObject(oldBrush);
	
	m_Border_start.x = start_x + txtWidth;
	m_Border_start.y = start_y + txtHeight;

	UINT W = this->m_Config.W;
	UINT H = this->m_Config.H;

	UINT border_width = W * m_CellSize;
	if (border_width > MAX_WIDTH) {
		W = MAX_WIDTH / m_CellSize;
		border_width = W * m_CellSize;
	}
	if (m_CurCell.x + W > m_Config.W) {
		W = m_Config.W - m_CurCell.x;
		border_width = W * m_CellSize;
	}

	UINT border_height = H * m_CellSize;
	if (border_height > MAX_WIDTH) {
		H = MAX_HEIGHT / m_CellSize;
		border_height = H * m_CellSize;
	}
	if (m_CurCell.y + H > m_Config.H) {
		H = m_Config.H - m_CurCell.y;
		border_height = H * m_CellSize;
	}

	m_Border_end.x = m_Border_start.x + border_width;
	m_Border_end.y = m_Border_start.y + border_height;

	m_pDC->Rectangle(
		m_Border_start.x, m_Border_start.y,
		m_Border_end.x, m_Border_end.y
		);

	CString cellNum;
	UINT difference;
	CSize txt_size;
	if (m_CellSize > 10) {
		cellNum.Format("%d", m_CurCell.y);
		txt_size = m_pDC->GetTextExtent(cellNum);
		difference = txt_size.cx + 2;
		m_pDC->TextOut(m_Border_start.x - difference, m_Border_start.y, cellNum);
		for (int i = 1; i < H; ++i) {
			int cur_y = m_Border_start.y + i*m_CellSize;
			m_pDC->MoveTo(m_Border_start.x, cur_y);
			m_pDC->LineTo(m_Border_end.x, cur_y);

			cellNum.Format("%d", m_CurCell.y + i);
			txt_size = m_pDC->GetTextExtent(cellNum);
			difference = txt_size.cx + 2;
			m_pDC->TextOut(m_Border_start.x - difference, cur_y, cellNum);
		}

		m_pDC->SelectObject(&ft2);
		cellNum.Format("%d", m_CurCell.x);
		txt_size = m_pDC->GetTextExtent(cellNum);
		difference = txt_size.cy;
		difference = 2;
		m_pDC->TextOut(m_Border_start.x, m_Border_start.y - difference, cellNum);
		for (int i = 1; i < W; ++i) {
			int cur_x = m_Border_start.x + i*m_CellSize;
			m_pDC->MoveTo(cur_x, m_Border_start.y);
			m_pDC->LineTo(cur_x, m_Border_end.y);

			cellNum.Format("%d", m_CurCell.x + i);
			m_pDC->TextOut(cur_x, m_Border_start.y - difference, cellNum);

		}
	}
	else {
		
		m_pDC->SelectObject(oldf);

		cellNum.Format("%d", m_CurCell.y);
		txt_size = m_pDC->GetTextExtent(cellNum);
		difference = txt_size.cx;
		m_pDC->TextOut(m_Border_start.x - difference, m_Border_start.y, cellNum);

		cellNum.Format("%d", m_CurCell.y + H - 1);
		txt_size = m_pDC->GetTextExtent(cellNum);
		difference = txt_size.cx;
		m_pDC->TextOut(
			m_Border_start.x - difference,
			m_Border_end.y - txt_size.cy,
			cellNum
			);

		cellNum.Format("%d", m_CurCell.x);
		txt_size = m_pDC->GetTextExtent(cellNum);
		difference = txt_size.cy;
		m_pDC->TextOut(m_Border_start.x, m_Border_start.y - difference, cellNum);

		cellNum.Format("%d", m_CurCell.x + W - 1);
		txt_size = m_pDC->GetTextExtent(cellNum);
		difference = txt_size.cy;
		m_pDC->TextOut(m_Border_end.x - txt_size.cx, m_Border_start.y - difference, cellNum);
	}

	m_pDC->SelectObject(oldf);
	ft.DeleteObject();
	ft2.DeleteObject();

	oldPen = m_pDC->GetCurrentPen();
	oldBrush = m_pDC->GetCurrentBrush();
	this->drawStations();
	m_pDC->SelectObject(oldPen);
	m_pDC->SelectObject(oldBrush);

	this->drawRobots();
	m_pDC->SelectObject(oldPen);
	m_pDC->SelectObject(oldBrush);

}


bool Game::decCellSize()
{
	if (m_CellSize > MIN_CELL_SIZE){
		//EnterCriticalSection(&m_CS_ChangeVar);
		EnterCriticalSection(&m_CS_Draw);
		--m_CellSize;
		drawPlayingField(true);
		//draw();
		LeaveCriticalSection(&m_CS_Draw);
		//LeaveCriticalSection(&m_CS_ChangeVar);
		return 1;
	}
	return 0;
}

bool Game::incCellSize()
{
	if (m_CellSize < MAX_CELL_SIZE){
		//EnterCriticalSection(&m_CS_ChangeVar);
		EnterCriticalSection(&m_CS_Draw);
		++m_CellSize;
		drawPlayingField(true);
		//draw();
		LeaveCriticalSection(&m_CS_Draw);
		//LeaveCriticalSection(&m_CS_ChangeVar);
		return 1;
	}
	return 0;
}




/*void Game::drawStatsField(CDC* m_pDC)
{
	this->clearArea(m_pDC, Color::WHITE, LEFT_SIDE_OF_STATS, 18 + 5, RIGHT_SIDE_OF_STATS, 18 + HEIGHT_PER_FIELD * 5);
	Color color = Color::RED;
	int counter = 0;
	for (int posY = 18; posY < 18 + HEIGHT_PER_FIELD * 5; posY += HEIGHT_PER_FIELD) {
		Color color = getColor(counter);
		CPen pen1(PS_ALL, 5, color);
		pDC->SelectObject(pen1);
		pDC->Rectangle(LEFT_SIDE_OF_STATS, posY + 5, RIGHT_SIDE_OF_STATS, posY + HEIGHT_PER_FIELD);

		char* tmp = new char[10];
		if (!m_Robots_const.empty()) {
			CSize nameLength = pDC->GetTextExtent(m_Robots_const.at(counter).Name.c_str());
			string copyName = m_Robots_const.at(counter).Name;
			while (nameLength.cx > RIGHT_SIDE_OF_STATS - LEFT_SIDE_OF_STATS - 20) {
				copyName.erase(copyName.end() - 1);
				nameLength = pDC->GetTextExtent(copyName.c_str());
			}
			pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 10, copyName.c_str());//name
			itoa(m_Robots_const.at(counter).E, tmp, 10);
			pDC->TextOutA(LEFT_SIDE_OF_STATS + 140, posY + 35, tmp);//energy
			itoa(m_Robots_const.at(counter).A, tmp, 10);
			pDC->TextOutA(LEFT_SIDE_OF_STATS + 140, posY + 55, tmp);//attack
			itoa(m_Robots_const.at(counter).P, tmp, 10);
			pDC->TextOutA(LEFT_SIDE_OF_STATS + 140, posY + 75, tmp);//protection
			itoa(m_Robots_const.at(counter).V, tmp, 10);
			pDC->TextOutA(LEFT_SIDE_OF_STATS + 140, posY + 95, tmp);//movespeed
		}
		delete tmp;
		pDC->MoveTo(LEFT_SIDE_OF_STATS, posY + 30);//horisontal line
		pDC->LineTo(RIGHT_SIDE_OF_STATS - 5, posY + 30);

		pDC->MoveTo(LEFT_SIDE_OF_STATS + 100, posY + 30);//vertical line
		pDC->LineTo(LEFT_SIDE_OF_STATS + 100, posY + HEIGHT_PER_FIELD - 5);

		CPen pen2(PS_ALL, 2, color);
		pDC->SelectObject(pen2);//second horisontal line
		pDC->MoveTo(LEFT_SIDE_OF_STATS, posY + 53);
		pDC->LineTo(RIGHT_SIDE_OF_STATS, posY + 53);

		pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 35, "Energy");

		pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 55, "Attack");

		pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 75, "Protection");

		pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 95, "Movespeed");
		++counter;
	}
}*/



void Game::drawStatsField(COLORREF color)
{

	

	int counter = 0;
	int posY = 30;
	this->clearArea(Color::WHITE, LEFT_SIDE_OF_STATS, posY + 5, RIGHT_SIDE_OF_STATS, posY + HEIGHT_PER_FIELD);
	CPen pen1;
	pen1.CreatePen(PS_ALL, 5, color);
	m_pDC->SelectObject(&pen1);
	m_pDC->Rectangle(LEFT_SIDE_OF_STATS, posY + 5, RIGHT_SIDE_OF_STATS, posY + HEIGHT_PER_FIELD);
	//m_pDC->Rectangle(LEFT_SIDE_OF_STATS, 28, RIGHT_SIDE_OF_STATS + 10,280);

	m_pDC->MoveTo(LEFT_SIDE_OF_STATS, posY + 30);//horisontal line
	m_pDC->LineTo(RIGHT_SIDE_OF_STATS - 5, posY + 30);

	m_pDC->MoveTo(LEFT_SIDE_OF_STATS + 100, posY + 30);//vertical line
	m_pDC->LineTo(LEFT_SIDE_OF_STATS + 100, posY + HEIGHT_PER_FIELD - 5);

	CPen pen2;
	pen2.CreatePen(PS_ALL, 2, color);
	m_pDC->SelectObject(&pen2);//second horisontal line
	m_pDC->MoveTo(LEFT_SIDE_OF_STATS, posY + 53);
	m_pDC->LineTo(RIGHT_SIDE_OF_STATS, posY + 53);

	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 35, "Energy");

	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 55, "Attack");

	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 75, "Protection");

	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 95, "Movespeed");
	
	m_pDC->MoveTo(LEFT_SIDE_OF_STATS, posY + 115);//third horisontal line
	m_pDC->LineTo(RIGHT_SIDE_OF_STATS, posY + 115);

	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 116, "Alive");

	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 135, "X");
	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 155, "Y");
	
	pen1.DeleteObject();
	pen2.DeleteObject();
}



CString cutString(CString str, CDC* pDC, UINT width)
{
	CSize cur_size = pDC->GetTextExtent(str);
	while (cur_size.cx > width){
		str = str.Left(str.GetLength() - 1);
		cur_size = pDC->GetTextExtent(str);
	}
	return str;
}

void Game::drawСonditions()
{
	m_Conditions_start.x = CONDITIONS_START_X;
	m_Conditions_start.y = CONDITIONS_START_Y;
	m_Conditions_end.x = CONDITIONS_START_X + CONDITIONS_WIDTH;
	m_Conditions_end.y = CONDITIONS_START_Y + CONDITIONS_TOTAL_HEIGHT;

	CPen* oldPen = m_pDC->GetCurrentPen();
	CBrush* oldBrush = m_pDC->GetCurrentBrush();
	
	m_pDC->SetBkMode(TRANSPARENT);

	CPen blackPen;
	blackPen.CreatePen(PS_ALL, 1, RGB(0, 0, 0));
	CBrush whiteBrush;
	whiteBrush.CreateSolidBrush(RGB(255, 255, 255));

	CString str;
	str.Format("Step: %d / %d Round: %d/ 5", m_StepNum, m_Config.N, m_RoundNum);
		
	m_Conditions_txtSize.cy = m_pDC->GetTextExtent(str).cy;
	UINT cur_x = m_Conditions_start.x;
	UINT cur_y = m_Conditions_start.y + m_Conditions_txtSize.cy + CONDITIONS_SPACE;
	UINT size = m_Robots_ptr.size();

	clearArea(		
		Color::GRAY,
		m_Conditions_start.x - 3,
		m_Conditions_start.y - 3,
		m_Conditions_end.x + 3,
		m_Conditions_end.y + 3
	);

	m_pDC->TextOut(m_Conditions_start.x, m_Conditions_start.y, str);
	CPen borderPen;
	borderPen.CreatePen(PS_ALL, 3, Color::RED);

	if (!m_Robots_ptr.begin()._Ptr)
		return;

	for (UINT i = m_ConditionsTopSel; i < size; ++i){
		Robot* it = m_Robots_ptr[i];
		m_pDC->SelectObject(&blackPen);
		m_pDC->SelectObject(&whiteBrush);		
		if (i == m_curSel){			
			m_pDC->SelectObject(&borderPen);
		}
		m_pDC->Rectangle(
			cur_x,
			cur_y,
			cur_x + CONDITIONS_WIDTH,
			cur_y + CONDITIONS_HEIGHT
			);
		//m_pDC->SelectObject(blackPen);
		float hp = float(it->E) / float(m_Config.E_max);
		if (hp < 0)
			hp = 0;
		BYTE green, red;
		if (hp >= 0.5){
			green = 255;
			red = 255 * 2 * (1 - hp);
		}
		else{
			green = 255 * 2 * hp;
			red = 255;
		}
		UINT width = hp * CONDITIONS_WIDTH;
		CBrush brush;
		brush.CreateSolidBrush(RGB(red, green, 0));
		CPen pen;
		pen.CreatePen(PS_ALL, 0, RGB(red, green, 0));
		m_pDC->SelectObject(&brush);	
		m_pDC->SelectObject(&pen);
		m_pDC->Rectangle(
			cur_x + 2,
			cur_y + 2,
			cur_x + width - 2,
			cur_y + CONDITIONS_HEIGHT - 2
		);
		m_pDC->SelectObject(&blackPen);
		CString name = cutString(it->Name.c_str(), m_pDC, CONDITIONS_WIDTH);//CString name(it->Name.c_str());
		m_pDC->TextOut(cur_x+2, cur_y+2, name);

		cur_y += CONDITIONS_HEIGHT + CONDITIONS_SPACE;
		if (cur_y > m_Conditions_end.y - CONDITIONS_HEIGHT) break;
		
		pen.DeleteObject();
		brush.DeleteObject();
	}


	m_pDC->SelectObject(oldPen);
	m_pDC->SelectObject(oldBrush);

	borderPen.DeleteObject();
	blackPen.DeleteObject();
	whiteBrush.DeleteObject();
}

void Game::setCurSelWithY(int y)
{
	if (y > m_Conditions_txtSize.cy + m_Conditions_start.y + m_Robots_ptr.size() * (CONDITIONS_HEIGHT + CONDITIONS_SPACE))
		return;
	UINT selNumFromTop = (y - m_Conditions_start.y) / (CONDITIONS_HEIGHT + CONDITIONS_SPACE);
	m_curSel = m_ConditionsTopSel + selNumFromTop - 1;
	draw();
}

void Game::setCurCell(CPoint position)
{
	EnterCriticalSection(&m_CS_Draw);
	m_CurCell = position;
	drawPlayingField();
	LeaveCriticalSection(&m_CS_Draw);
}

CPoint Game::getCurCell()
{
	return m_CurCell;
}

bool Game::setStations()
{
	srand(time(NULL) + clock());
	if (m_Config.H * m_Config.W < m_Config.N_E + m_Config.N_L) {
		AfxMessageBox("All stations don't fit on the field!");
		return 0;
	}
	m_ChargingStations.clear();
	m_Maintenance.clear();
	for (int i = 0; i < m_Config.N_E; ++i) {
		UINT cellX = rand() % m_Config.W;
		UINT cellY = rand() % m_Config.H;
		auto it = find(m_ChargingStations.begin(), m_ChargingStations.end(), CPoint(cellX, cellY));
		while (it != m_ChargingStations.end()) {
			cellX = rand() % m_Config.W;
			cellY = rand() % m_Config.H;
			it = find(m_ChargingStations.begin(), m_ChargingStations.end(), CPoint(cellX, cellY));
		}
		m_ChargingStations.push_back(CPoint(cellX, cellY));
	}
	
	for (int i = 0; i < m_Config.N_L; ++i) {
		UINT cellX = rand() % m_Config.W;
		UINT cellY = rand() % m_Config.H;
		auto it1 = find(m_Maintenance.begin(), m_Maintenance.end(), CPoint(cellX, cellY));
		auto it2 = find(m_ChargingStations.begin(), m_ChargingStations.end(), CPoint(cellX, cellY));
		while (it1 != m_Maintenance.end() || it2 != m_ChargingStations.end()) {
			cellX = rand() % m_Config.W;
			cellY = rand() % m_Config.H;
			it1 = find(m_Maintenance.begin(), m_Maintenance.end(), CPoint(cellX, cellY));
			it2 = find(m_ChargingStations.begin(), m_ChargingStations.end(), CPoint(cellX, cellY));
		}
		m_Maintenance.push_back(CPoint(cellX, cellY));
	}
	return 1;
}


void Game::drawStations()
{	
	BYTE mode;
	if (m_CellSize > 10){
		mode = 0;
	}
	else{
		mode = 1;
	}

	CPen* oldPen = m_pDC->GetCurrentPen();
	CBrush* oldBrush = m_pDC->GetCurrentBrush();

	CBrush brush1;
	brush1.CreateSolidBrush(Color::RED);
	CPen pen1;
	pen1.CreatePen(PS_ALL, 0, Color::RED);
	m_pDC->SelectObject(&brush1);
	m_pDC->SelectObject(&pen1);
	for (int i = 0; i < m_ChargingStations.size(); ++i) {
		UINT cellX = m_ChargingStations[i].x;
		UINT cellY = m_ChargingStations[i].y;
		UINT x = (cellX - m_CurCell.x) * m_CellSize + m_Border_start.x;
		UINT y = (cellY - m_CurCell.y) * m_CellSize + m_Border_start.y;

		if (x >= m_Border_start.x && x < m_Border_end.x 
			&& y >= m_Border_start.y && y < m_Border_end.y){
			UINT x1, y1, x2, y2;
			switch (mode)
			{
			case 0:
				x1 = x;
				y1 = y;
				x2 = x + m_CellSize;
				y2 = y + m_CellSize;
				break;
			case 1:
				x1 = x - 3;
				if (x1 < m_Border_start.x)
					x1 = m_Border_start.x;
				y1 = y - 3;
				if (y1 < m_Border_start.y)
					y1 = m_Border_start.y;
				x2 = x + 3;
				if (x2 > m_Border_end.x)
					x2 = m_Border_end.x;
				y2 = y + 3;
				if (y2 > m_Border_end.y)
					y2 = m_Border_end.y;
				break;

			default:
				break;
			}
			m_pDC->Rectangle(x1, y1, x2, y2);
		}
	}

	CBrush brush2;
	brush2.CreateSolidBrush(Color::BLUE);
	CPen pen2;
	pen2.CreatePen(PS_ALL, 0, Color::BLUE);
	m_pDC->SelectObject(&brush2);
	m_pDC->SelectObject(&pen2);
	for (int i = 0; i < m_Maintenance.size(); ++i) {
		UINT cellX = m_Maintenance[i].x;
		UINT cellY = m_Maintenance[i].y;
		UINT x = (cellX - m_CurCell.x) * m_CellSize + m_Border_start.x;
		UINT y = (cellY - m_CurCell.y) * m_CellSize + m_Border_start.y;
		if (x >= m_Border_start.x && x < m_Border_end.x
			&& y >= m_Border_start.y && y < m_Border_end.y) {
			UINT x1, y1, x2, y2;
			switch (mode)
			{
			case 0:
				x1 = x;
				y1 = y;
				x2 = x + m_CellSize;
				y2 = y + m_CellSize;
				break;
			case 1:
				x1 = x - 3;
				if (x1 < m_Border_start.x)
					x1 = m_Border_start.x;
				y1 = y - 3;
				if (y1 < m_Border_start.y)
					y1 = m_Border_start.y;
				x2 = x + 3;
				if (x2 > m_Border_end.x)
					x2 = m_Border_end.x;
				y2 = y + 3;
				if (y2 > m_Border_end.y)
					y2 = m_Border_end.y;
				break;

			default:
				break;
			}
			m_pDC->Rectangle(x1, y1, x2, y2);;
		}
	}

	m_pDC->SelectObject(oldPen);
	m_pDC->SelectObject(oldBrush);

	pen1.DeleteObject();
	brush1.DeleteObject();
	pen2.DeleteObject();
	brush2.DeleteObject();
}

void Game::clearArea(COLORREF color, UINT x1, UINT y1, UINT x2, UINT y2)
{
	CPen* oldPen = m_pDC->GetCurrentPen();
	CBrush* oldBrush = m_pDC->GetCurrentBrush();

	CBrush brush;
	brush.CreateSolidBrush(color);
	CPen pen;
	pen.CreatePen(PS_ALL, 0, Color::GRAY);
	//CBrush* oldBrush = m_pDC->GetCurrentBrush();
	m_pDC->SelectObject(&brush);
	//CPen* oldPen = m_pDC->GetCurrentPen();
	m_pDC->SelectObject(&pen);
	m_pDC->Rectangle(x1, y1, x2, y2);

	m_pDC->SelectObject(oldBrush);
	m_pDC->SelectObject(oldPen);

	pen.DeleteObject();
	brush.DeleteObject();
}


CPoint Game::normalizeСoordinates(CPoint pos)
{
	CPoint norm_pos(pos);
	while (norm_pos.x < 0){
		norm_pos.x += m_Config.W;
	}
	while (norm_pos.x >= m_Config.W){
		norm_pos.x -= m_Config.W;
	}
	while (norm_pos.y < 0){
		norm_pos.y += m_Config.H;
	}
	while (norm_pos.y >= m_Config.H){
		norm_pos.y -= m_Config.H;
	}
	return norm_pos;
}

inline UINT Pifagor(INT dx, INT dy)
{
	return sqrt(pow(dx, 2) + pow(dy, 2));
} 

UINT Game::getDistance(CPoint posA, CPoint posB)
{
	UINT dist1_x = abs(posA.x - posB.x);
	UINT dist1_y = abs(posA.y - posB.y);

	UINT left_x, right_x;
	if (posA.x > posB.x){
		right_x = posA.x;
		left_x = posB.x;
	}
	else{
		right_x = posB.x;
		left_x = posA.x;
	}
	UINT dist2_x = left_x + (m_Config.W - right_x);

	UINT bottom_y, top_y;
	if (posA.y > posB.y){
		bottom_y = posA.y;
		top_y = posB.y;
	}
	else{
		bottom_y = posB.y;
		top_y = posA.y;
	}
	UINT dist2_y = top_y + (m_Config.H - bottom_y);

	return Pifagor(
		((dist1_x < dist2_x) ? dist1_x : dist2_x),
		((dist1_y < dist2_y) ? dist1_y : dist2_y)
		);

}

UINT Game::getDistance(UINT x1, UINT y1, UINT x2, UINT y2)
{
	return getDistance(CPoint((INT)x1, (INT)y1), CPoint((INT)x2, (INT)y2));
}


bool Game::doActionMove(UINT robot_id, INT dx, INT dy)
{
	ActionLog actionLog;
	actionLog.pAction = new Move(dx, dy);
	actionLog.robotID = robot_id;
	actionLog.step = m_StepNum;

	Robot* self_robot = &(m_Robots_self[robot_id]);

	UINT max_distance = m_Config.V_max * self_robot->V * self_robot->E / (m_Config.L_max * m_Config.E_max);
	UINT distance = Pifagor(dx, dy);
	
	if (distance > max_distance){
		actionLog.result = false;
		m_RoundLogList.push_back(actionLog);
		return false;
	}
	CPoint new_pos = normalizeСoordinates(CPoint(self_robot->x + dx, self_robot->y + dy));
	
	//Непонятная ситуация в случае перемещения роботов в одну клетку
	//if (m_pField->isOccupied(new_pos))
	//	return false;

	self_robot->x = new_pos.x;
	self_robot->y = new_pos.y;
	self_robot->E -= m_Config.dE_V;
	if (self_robot->E <= 0){
		self_robot->Alive = 0;
	}
	actionLog.result = true;
	m_RoundLogList.push_back(actionLog);
	return true;
}


bool Game::doActionRedistribution(UINT robot_id, UINT _A, UINT _P, UINT _V)
{
	ActionLog actionLog;
	actionLog.pAction = new Redistribution(_A, _P, _V);
	actionLog.robotID = robot_id;
	actionLog.step = m_StepNum;

	Robot* self_robot = &(m_Robots_self[robot_id]);
		
	UINT max_dL = m_Config.dL;
	UINT cur_L = self_robot->L;
	UINT old_A = self_robot->A;
	UINT old_P = self_robot->P;
	UINT old_V = self_robot->V;

	if (_A + _P + _V > cur_L){
		actionLog.result = false;
		m_RoundLogList.push_back(actionLog);
		return false;
	}
	INT dA = old_A - _A;
	INT dP = old_P - _P;
	INT dV = old_V - _V;
	INT dL = (dA + dP + dV) / 2;
	if (dL > max_dL){
		actionLog.result = false;
		m_RoundLogList.push_back(actionLog);
		return false;
	}
	self_robot->A = _A;
	self_robot->P = _P;
	self_robot->V = _V;
	actionLog.result = true;
	m_RoundLogList.push_back(actionLog);
	return true;
}




bool Game::doActionAttack(UINT robot_id, UINT victim_id)
{
	ActionLog actionLog;
	actionLog.pAction = new Attack(victim_id);
	actionLog.robotID = robot_id;
	actionLog.step = m_StepNum;

	UINT size = m_Robots_start.size();
	if (m_Robots_start.find(victim_id) == m_Robots_start.end() ){
		actionLog.result = false;
		m_RoundLogList.push_back(actionLog);
		return false;
	}

	Robot* cur_robot = &(m_Robots_start[robot_id]);
	Robot* self_robot = &(m_Robots_self[robot_id]);
	Robot* cur_victim = &(m_Robots_start[victim_id]);
	Robot* other_robot = &(m_Robots_other[victim_id]);

	if (!cur_victim->Alive){
		actionLog.result = false;
		m_RoundLogList.push_back(actionLog);
		return false;
	}

	CPoint robot_pos(self_robot->x, self_robot->y);
	CPoint victim_pos(cur_victim->x, cur_victim->y);

	UINT max_distance = m_Config.R_max * self_robot->V * self_robot->E / (m_Config.E_max * m_Config.L_max);

	UINT distance = getDistance(robot_pos, victim_pos);

	if (distance > max_distance){
		actionLog.result = false;
		m_RoundLogList.push_back(actionLog);
		return false;		
	}

	float RND = m_Config.RND_min + (float)(rand() % (int)((m_Config.RND_max - m_Config.RND_min) * 1000)) / 1000;
	float A_r = RND * self_robot->A;
	float P_r = (1 - RND) * cur_victim->P;

	int delta = (A_r * self_robot->E - P_r * cur_victim->E) / m_Config.E_max;
	int dE = abs(delta) * m_Config.E_max / m_Config.L_max;
	if (delta > 0){
		other_robot->P -= delta;
		other_robot->L -= delta;
		if (other_robot->P < 0){
			other_robot->P = 0;			
			other_robot->E -= dE;
		}
	}
	else{
		self_robot->A += delta;
		self_robot->L += delta;
		if (self_robot->A < 0){
			self_robot->A = 0;			
			self_robot->E -= dE;			
		}
	}
	self_robot->E -= m_Config.dE_A;
	if (self_robot->E <= 0){
		self_robot->Alive = 0;
		self_robot->E = 0;
		cur_victim->RoundFrags++;
		other_robot->E += m_Config.K;
		if (other_robot->E > m_Config.E_max)
			other_robot->E = m_Config.E_max;
	}
	other_robot->E -= m_Config.dE_P;
	if (other_robot->E <= 0){
		other_robot->Alive = 0;
		other_robot->E = 0;
		self_robot->E += m_Config.K;
		if (self_robot->E > m_Config.E_max)
			self_robot->E = m_Config.E_max;
		cur_robot->RoundFrags++;
	}
	actionLog.result = true;
	m_RoundLogList.push_back(actionLog);
	return true;
}


struct ThreadStruct{
	ThreadStruct(StepInfo* _sendStruct, RobotFunction _robotFunction) :
		sendStruct(_sendStruct), robotFunction(_robotFunction){}
	StepInfo* sendStruct;
	RobotFunction robotFunction;
};


DWORD WINAPI robotThread(LPVOID lpParam)
{
	ThreadStruct* threadStruct = (ThreadStruct*)lpParam;
	__try{
		if (threadStruct->robotFunction != NULL){
			threadStruct->robotFunction(threadStruct->sendStruct);
		}
		else{
			AfxMessageBox("Функция не инициализирована");
		}
	}
	__except(EXCEPTION_DEBUG_EVENT){
		AfxMessageBox("Ошибка функции dll");
		//exit(-1);
		return -1;
	}
	return 1;
}

DWORD WINAPI Game::playThread(LPVOID lpParam)
{
	while (m_RoundNum < 5){
		INT res = startRound();
		if (res == 0)
			return 0;
	}
	m_isNormalEnd = true;
	end();
	SendMessage(m_pDialog->m_hWnd, WM_ON_GAME_END, NULL, NULL);

	
	//CloseHandle(m_hPlayThread);

	return 1;
}


DWORD WINAPI Game_PlayThread(LPVOID lpParam)
{
	(static_cast<Game*>(lpParam))->playThread(lpParam);
	return 1;
}


DWORD WINAPI Game_DrawThread(LPVOID lpParam)
{
	(static_cast<Game*>(lpParam))->drawThread(lpParam);
	return 1;	
}

DWORD WINAPI Game::drawThread(LPVOID lpParam)
{
	HANDLE hEvents[] = { m_hPauseDrawEvent, m_hEndDrawEvent };
	HANDLE hEvents2[] = { m_hResumeDrawEvent, m_hEndDrawEvent };

	while (1) {
		draw();
		switch (WaitForMultipleObjects(2, hEvents, FALSE, RENDER_PAUSE) - WAIT_OBJECT_0) {
		case 0:
			switch (WaitForMultipleObjects(2, hEvents2, FALSE, INFINITE) - WAIT_OBJECT_0) {
			case 0:
				continue;
				break;
			case 1:
				SetEvent(m_hConfirmEndDraw);
				return 0;
				break;
			}
			break;
		case 1:
			SetEvent(m_hConfirmEndDraw);
			return 0;
			break;
		default:
			break;
		}
	}
}

void Game::start()
{
	m_GameResults.clear();
	m_RoundNum = 0;
	m_hPlayThread = CreateThread(NULL, 0, Game_PlayThread, this, 0, NULL);
	
	//this->draw();
}


Robot Game::getRobotInfo(USHORT num)
{
	return *m_Robots_ptr[num];
}

bool Game::setRobotsInfo(CString filePath)
{
	SetCurrentDirectory(m_Directory);

	for (auto it = m_Robots_global.begin(); it != m_Robots_global.end(); ++it){
		if ((*it)->hDll){
			FreeLibrary((*it)->hDll);
		}
		if (*it)
			delete *it;
	}
	m_Robots_global.clear();
	ifstream f(filePath);
	if (!f.is_open()) {
		AfxMessageBox("Can not open file!");
		return 0;
	}
	USHORT ID = 0;
	char buffer[1024];
	USHORT startRoundNum = 1;
	while (f.getline(buffer, 1024)) {
		Robot* curRobot = new Robot;

		if (buffer[0] == '/' && buffer[1] == '/'){
			continue;
		}

		char * pch = strtok(buffer, " ");
		if (!pch){
			AfxMessageBox("Ошибка чтения пути к dll");
			return 0;
		}
		CString path(pch);

		pch = strtok(NULL, " ");
		if (!pch){
			AfxMessageBox("Ошибка чтения имени робота");
			return 0;
		}
		CString name(pch);

		pch = strtok(NULL, " ");
		if (!pch){
			AfxMessageBox("Ошибка чтения имени автора");
			return 0;
		}
		string author(pch);

		curRobot->Author = author;
		curRobot->startRound = startRoundNum;
		curRobot->isPlayer = 1;
		curRobot->Alive = 1;
		curRobot->Points = 0;
		curRobot->Rank = 0;
		curRobot->TotalScore = 0;
		curRobot->RoundScore = 0;
		curRobot->TotalFrags = 0;
		curRobot->RoundFrags = 0;
		curRobot->RoundAliveSteps = 0;
		curRobot->TotalAliveSteps = 0;
		curRobot->filePath = path;
		curRobot->Name = name;
		curRobot->E = m_Config.E_max;		

		curRobot->ID = ID++;		

		m_Robots_global.push_back(curRobot);

		auto cur_Author = m_Authors.find(curRobot->Author);

		if (cur_Author == m_Authors.end()){
			vector<Robot*> tmp;
			tmp.push_back(m_Robots_global.back());
			m_Authors.insert(make_pair(curRobot->Author, tmp));
		}
		else{
			cur_Author->second.push_back(m_Robots_global.back());
		}
	}

	for (auto it = m_Robots_global.begin(); it != m_Robots_global.end(); ++it){
		if ((*it)->isPlayer){
			(*it)->hDll = LoadLibrary((*it)->filePath);
			if ((*it)->hDll == NULL){
				CString str;
				str.Format("Ошибка загрузки dll: %s", (*it)->filePath);
				AfxMessageBox(str);
				return 0;
			}
			(*it)->robotFunction = (RobotFunction)GetProcAddress((*it)->hDll, "DoStep");
			if ((*it)->robotFunction == NULL){
				CString str;
				str.Format("Функция DoStep не найдена в %s", (*it)->filePath);
				AfxMessageBox(str);
				return 0;
			}
		}
	}
	
	UINT size = m_Robots_global.size();
	for (USHORT round = 2; round <= 5; ++round){
		for (UINT i = 0; i < size; ++i){
			if (!m_Robots_global[i]->isPlayer)
				break;
			Robot* curRobot = new Robot;
			*curRobot = *m_Robots_global[i];
			curRobot->startRound = round;
			curRobot->ID = ID++;
			m_Robots_global.push_back(curRobot);
			m_Authors.find(curRobot->Author)->second.push_back(curRobot);
		}
	}

	HINSTANCE standartDll = LoadLibrary(STANDART_ROBOT_DLL);
	if (standartDll == NULL){
		CString str;
		str.Format("Ошибка загрузки dll стандартного робота: %s", STANDART_ROBOT_DLL);
		AfxMessageBox(str);
		return 0;
	}
	RobotFunction standartFunction = (RobotFunction)GetProcAddress(standartDll, "DoStep");
	vector<Robot*> tmp;
	m_Authors.insert(make_pair("NPC", tmp));
	for (int i = 0; i < 5; ++i){
		Robot* curRobot = new Robot;
		curRobot->ID = ID++;

		curRobot->Author = "NPC";
		curRobot->startRound = 1;
		curRobot->isPlayer = 0;
		curRobot->Alive = 1;
		curRobot->Points = 0;
		curRobot->Rank = 0;
		curRobot->TotalScore = 0;
		curRobot->RoundScore = 0;
		curRobot->TotalFrags = 0;
		curRobot->RoundFrags = 0;
		curRobot->RoundAliveSteps = 0;
		curRobot->TotalAliveSteps = 0;
		curRobot->filePath = STANDART_ROBOT_DLL;
		curRobot->hDll = standartDll;
		curRobot->robotFunction = standartFunction;

		CString tmp;
		tmp.Format("%d", i + 1);
		curRobot->Name = "Gladiator_" + tmp;

		curRobot->E = m_Config.E_max;

		m_Robots_global.push_back(curRobot);

		m_Authors.find(curRobot->Author)->second.push_back(curRobot);
	}
	
	size = m_Robots_global.size();
	srand(time(NULL) + clock());
	for (int i = 0; i < size; ++i){
		USHORT r = rand() % 255;
		USHORT g = rand() % 255;
		USHORT b = rand() % 255;
		while (r > 150 && g > 150 && b > 150){
			r = rand() % 255;
			g = rand() % 255;
			b = rand() % 255;
		}
		m_Colors.push_back(RGB(r, g, b));
	}

	return 1;
}


void Game::drawRobots()
{
	CPen* oldPen = m_pDC->GetCurrentPen();
	CBrush* oldBrush = m_pDC->GetCurrentBrush();

	UINT size = m_Robots_ptr.size();
	BYTE mode;
	if (m_CellSize > 10){
		mode = 0;
	}
	else{
		mode = 1;
	}
	for (int i = 0; i < size; ++i){
		COLORREF color;
		bool alive = m_Robots_ptr.at(i)->Alive;
		if (alive){
			color = m_Colors[m_Robots_ptr.at(i)->ID];
		}
		else{
			color = RGB(0, 0, 0);
		}
		CBrush brush;
		brush.CreateSolidBrush(color);
		CPen pen;
		pen.CreatePen(PS_ALL, 0, color);
		m_pDC->SelectObject(&brush);
		m_pDC->SelectObject(&pen);

		UINT cellX = m_Robots_ptr.at(i)->x;
		UINT cellY = m_Robots_ptr.at(i)->y;
		UINT x = (cellX - m_CurCell.x) * m_CellSize + m_Border_start.x;
		UINT y = (cellY - m_CurCell.y) * m_CellSize + m_Border_start.y;		

		if (x >= m_Border_start.x && x < m_Border_end.x
			&& y >= m_Border_start.y && y < m_Border_end.y){
			UINT x1, y1, x2, y2;

			switch (mode)
			{
			case 0:
				x1 = x;
				y1 = y;
				x2 = x + m_CellSize;
				y2 = y + m_CellSize;
				break;
			case 1:
				x1 = x - 5;
				if (x1 < m_Border_start.x)
					x1 = m_Border_start.x;
				y1 = y - 5;
				if (y1 < m_Border_start.y)
					y1 = m_Border_start.y;
				x2 = x + 5;
				if (x2 > m_Border_end.x)
					x2 = m_Border_end.x;
				y2 = y + 5;
				if (y2 > m_Border_end.y)
					y2 = m_Border_end.y;
				break;
				
			default:
				break;
			}

			m_pDC->Ellipse(x1, y1, x2, y2);
			if (i == m_curSel){
				CPen pen_ptr;
				pen_ptr.CreatePen(PS_ALL, 2, color);
				m_pDC->SelectObject(&pen_ptr);
				m_pDC->MoveTo(x1, y1 - (mode ? 5 : m_CellSize / 2));
				m_pDC->LineTo(x1 +(x2 - x1) / 2, y1);
				m_pDC->LineTo(x2, y1 - (mode ? 5 : m_CellSize / 2));
				m_pDC->MoveTo(x1 + (x2 - x1) / 2, y1);
				m_pDC->LineTo(x1 + (x2 - x1) / 2, y1 - (mode ? 10 : m_CellSize));
				pen_ptr.DeleteObject();
			}

		}

		m_pDC->SelectObject(oldPen);
		m_pDC->SelectObject(oldBrush);
		pen.DeleteObject();
		brush.DeleteObject();
	}




}

void Game::setFilePath(CString path)
{
	m_File_path = path;
}

CString Game::getFilePath()
{
	return m_File_path;
}


DWORD WINAPI Game::pause()
{
	//DWORD exitCode;
	//GetExitCodeThread(m_hPlayThread, &exitCode);
	//if (exitCode == STILL_ACTIVE){
	//	SuspendThread(m_hPlayThread);
	//}
	//GetExitCodeThread(m_hDrawThread, &exitCode);
	//if (exitCode == STILL_ACTIVE){
	//	SuspendThread(m_hDrawThread);
	//}

	SetEvent(m_hPausePlayEvent);
	SetEvent(m_hPauseDrawEvent);
	
	//HANDLE hEvents[] = { m_hConfirmPausePlay, m_hConfirmPauseDraw };
	//
	//WaitForMultipleObjects(2, hEvents, TRUE, INFINITE);




	return 1;
}


DWORD WINAPI Game::resume()
{
	//ResumeThread(m_hPlayThread);
	//ResumeThread(m_hDrawThread);
	SetEvent(m_hResumeDrawEvent);
	SetEvent(m_hResumePlayEvent);

	return 1;
}


bool sortFunctionAuthors(pair<string, UINT> A1, pair<string, UINT> A2)
{
	return A1.second > A2.second;
}

bool Game::end()
{
	DWORD exitCode;
	GetExitCodeThread(m_hPlayThread, &exitCode);
	if (exitCode == STILL_ACTIVE){		
		if (!m_isNormalEnd) {
			SetEvent(m_hEndDrawEvent);
			SetEvent(m_hEndEvent);
			HANDLE hEvents[] = { m_hConfirmEndDraw, m_hConfirmEnd };

			//if (GetExit)
			WaitForMultipleObjects(2, hEvents, TRUE, INFINITE);
		}
		else{
			for (auto it = m_Authors.begin(); it != m_Authors.end(); ++it){
				UINT points = 0;
				for (auto robot = it->second.begin(); robot != it->second.end(); ++robot){
					points += (*robot)->Points;
				}
				m_GameResults.push_back(make_pair(it->first, points));
			}
			std::sort(m_GameResults.begin(), m_GameResults.end(), sortFunctionAuthors);
		}
		clearArea(Color::GRAY, LEFT_SIDE_OF_STATS - 2, 30, LEFT_SIDE_OF_STATS + CONDITIONS_WIDTH, CONDITIONS_START_Y + CONDITIONS_TOTAL_HEIGHT);
		m_RoundNum = 0;
		m_StepNum = 0;
		
		m_Robots_start.clear();
		m_Robots_self.clear();
		m_Robots_other.clear();
		m_Robots_ptr.clear();		
		
		WriteResultsToFile();

		draw();

		clearRoundLogList();


	}
	else{
		return 0;
	}
	
}


void Game::fillStatsField(bool flag/*= 0*/)
{
	//if (!flag){
	//	if (clock() - m_lastRender < RENDER_PAUSE){
	//		return;
	//	}
	//}

	if (m_pDC == NULL){
		AfxMessageBox("m_pDC is NULL");
		return;
	}
	CPen* oldPen = m_pDC->GetCurrentPen();
	CBrush* oldBrush = m_pDC->GetCurrentBrush();

	Robot robot;

	COLORREF color(Color::RED);

	if (!m_Robots_ptr.size()){
		return;
	}

	if (!&m_Robots_ptr[0])
		return;

	if (m_curSel >= 0){
		robot = this->getRobotInfo(m_curSel);
		float hp = float(robot.E) / float(m_Config.E_max);
		BYTE green, red;
		if (hp < 0)
			hp = 0;
		if (hp >= 0.5){
			green = 255;
			red = 255 * 2 * (1 - hp);
		}
		else{
			green = 255 * 2 * hp;
			red = 255;
		}
		color = RGB(red, green, 0);
	}

	this->drawStatsField(color);

	if (m_curSel < 0){
		return;
	}

	int posY = 30;

	char tmp[10];	
	if (!&this->getRobotInfo(m_curSel)){
		return;
	}
	CSize nameLength = m_pDC->GetTextExtent(this->getRobotInfo(m_curSel).Name.c_str());
	string copyName = robot.Name.c_str();
	while (nameLength.cx > RIGHT_SIDE_OF_STATS - LEFT_SIDE_OF_STATS - 20) {
		copyName.erase(copyName.end() - 1);
		nameLength = m_pDC->GetTextExtent(copyName.c_str());
	}
	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 10, posY + 10, copyName.c_str());//name
	itoa(robot.E, tmp, 10);
	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 180, posY + 35, tmp);//energy
	itoa(robot.A, tmp, 10);
	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 180, posY + 55, tmp);//attack
	itoa(robot.P, tmp, 10);
	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 180, posY + 75, tmp);//protection
	itoa(robot.V, tmp, 10);
	m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 180, posY + 95, tmp);//movespeed

	robot.Alive ? m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 180, posY + 116, "Yes") :
		m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 180, posY + 116, "No");//alive

	if (robot.x < 100000) {
		itoa(robot.x, tmp, 10);
		m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 180, posY + 135, tmp);//x
	}
	else {
		m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 180, posY + 135, "null");//x
	}

	if (robot.y <  100000) {
		itoa(robot.y, tmp, 10);
		m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 180, posY + 155, tmp);//y
	}
	else {
		m_pDC->TextOutA(LEFT_SIDE_OF_STATS + 180, posY + 155, "null");//y
	}

	//delete tmp;

	m_pDC->SelectObject(oldPen);
	m_pDC->SelectObject(oldBrush);

	drawСonditions();	
}


void Game::incConditionsTopSel()
{
	if (m_ConditionsTopSel < m_Robots_ptr.size() - 1){
		//EnterCriticalSection(&m_CS_ChangeVar);
		EnterCriticalSection(&m_CS_Draw);
		++m_ConditionsTopSel;
		fillStatsField(true);
		LeaveCriticalSection(&m_CS_Draw);
		//LeaveCriticalSection(&m_CS_ChangeVar);
	}
}

void Game::decConditionsTopSel()
{
	if (m_ConditionsTopSel > 0){
		//EnterCriticalSection(&m_CS_ChangeVar);
		EnterCriticalSection(&m_CS_Draw);
		--m_ConditionsTopSel;
		fillStatsField(true);
		LeaveCriticalSection(&m_CS_Draw);
		//LeaveCriticalSection(&m_CS_ChangeVar);
	}

}


void Game::WriteLogToFile()
{

	SetCurrentDirectory(m_Directory);
	CString round;
	round.Format("%d", m_RoundNum);
	CString filePath = "Logs\\"+round+"_Round.txt";
	ofstream f;
	f.open(filePath);
	f << "-------------------Round " << m_RoundNum << "----------------------" << endl;
	for (auto it = m_RoundLogList.begin(); it != m_RoundLogList.end(); ++it){
		f << it->step << " ID=" << it->robotID << " NAME=" << m_Robots_start[it->robotID].Name.c_str();
		switch (it->pAction->getType())
		{
		case MOVE:{
			Move* pMove = static_cast<Move*>(it->pAction);
			f << " / " << "Move: dx=" << pMove->dx << " dy=" << pMove->dy << " RESULT=" << it->result << " / ";
			break;
		}
		case ATTACK:{
			Attack* pAttack = static_cast<Attack*>(it->pAction);
			f << " / " << "Attack: victimID=" << pAttack->victimID << " RESULT=" << it->result << " / ";
			break;
		}
		case REDISTRIBUTION:{
			Redistribution* pRedistribution = static_cast<Redistribution*>(it->pAction);
			f << " / " << "Redistribution: newA=" << pRedistribution->A << " newP=" << pRedistribution->P << " newV=" << pRedistribution->V << " RESULT=" << it->result << " / ";
			break;
		}
		default:
			break;
		}
		f << endl;
	}

	f << "-------------------Results----------------------" << endl;

	for (auto it = m_RoundResults.begin(); it != m_RoundResults.end(); ++it){
		f << "Место: "
			<< (*it)->Rank
			<< " Автор: "
			<< (*it)->Author.c_str()
			<< " Робот: "
			<< (*it)->Name.c_str()
			<< " Энергия: "
			<< (*it)->E
			<< " Убийства: "
			<< (*it)->RoundFrags
			<< " Шаги: "
			<< (*it)->RoundAliveSteps
			<< " Очки: "
			<< (*it)->RoundScore
			<< " Баллы: "
			<< (*it)->Points
			<< endl;
	}
	f.close();
}



INT Game::startRound()
{
	m_Config = m_ConfigStore->getRoundConfig(m_RoundNum);

	++m_RoundNum;
	m_StepNum = 0;
	
	clearFolderForRobots();

	vector<map<UINT, Robot>::iterator> vec;
	for (auto it = m_Robots_start.begin(); it != m_Robots_start.end(); ++it){
		if (it->second.Alive == 0 && it->second.isPlayer == 1){
			vec.push_back(it);
		}
	}
	for (auto it = vec.begin(); it != vec.end(); ++it){
		//FreeLibrary((*it)->second.hDll);
		m_Robots_start.erase(*it);
	}
	
	for (auto it = m_Robots_global.begin(); it != m_Robots_global.end(); ++it){
		if ((*it)->startRound == m_RoundNum){
			m_Robots_start.insert(make_pair((*it)->ID, **it));
		}
		//else if ((*it)->startRound > m_RoundNum)
		//	break;
	}

	m_Robots_ptr.clear();
	for (auto it = m_Robots_start.begin(); it != m_Robots_start.end(); ++it){
		it->second.Alive = 1;
		it->second.RoundFrags = 0;
		it->second.RoundScore = 0;
		it->second.RoundAliveSteps = 0;
		it->second.Points = 0;
		it->second.E = m_Config.E_max;
		it->second.L = m_Config.L_max;
		if (m_Config.V_max < m_Config.L_max / 3) {
			it->second.V = m_Config.V_max;
		}
		else {
			it->second.V = m_Config.L_max / 3;
		}
		it->second.P = (m_Config.L_max - it->second.V) / 2;
		it->second.A = m_Config.L_max - it->second.V - it->second.P;
		m_Robots_ptr.push_back(&it->second);
	}

	//fillStatsField(true);


	//for (auto )


	list < pair<unsigned int, RobotActions> > old_actionsList;
	list < pair<unsigned int, RobotActions> > new_actionsList;

	setStations();

	if (m_pField){
		delete m_pField;
	}
	m_pField = new Field(m_Config.W, m_Config.H);
	for (auto it = m_ChargingStations.begin(); it != m_ChargingStations.end(); ++it){
		m_pField->addChargingStation(*it);
	}
	for (auto it = m_Maintenance.begin(); it != m_Maintenance.end(); ++it){
		m_pField->addMaintenance(*it);
	}


	UINT num = 0;
	UINT size = m_Robots_start.size();
	srand(time(NULL) + clock());
	for (UINT i = 0; i < size; ++i){
		while (1){
			UINT x = rand() % m_Config.W;
			UINT y = rand() % m_Config.H;
			bool flag = 0;
			for (int j = 0; j < i; ++j){
				UINT distance = getDistance(x, y, m_Robots_ptr[j]->x, m_Robots_ptr[j]->y);
				//UINT distance = abs(int(x - m_Robots_const[j].x)) + abs(int(y - m_Robots_const[j].y));
//				if (distance < 2 * m_Config.V_max) {
				if (distance < 1) {
						flag = 1;
					break;
				}
			}
			if (!flag){
				m_Robots_ptr[i]->x = x;
				m_Robots_ptr[i]->y = y;
				break;
			}
			++num;
			if (num > 100000){
				AfxMessageBox("Не могу расставить роботов :'(");
				m_isNormalEnd = true;
				end();
				return 0;
			}
		}
	}
	
	m_hDrawThread = CreateThread(NULL, 0, Game_DrawThread, this, 0, NULL);

	m_Robots_self = m_Robots_start;
	m_Robots_other = m_Robots_start;
	
	USHORT nAliveRobots = m_Robots_start.size();

	HANDLE hEvents[2];
	hEvents[0] = m_hPausePlayEvent;
	hEvents[1] = m_hEndEvent;

	HANDLE hEvents2[] = { m_hResumePlayEvent, m_hEndEvent };


	while (m_StepNum < m_Config.N && nAliveRobots > 1) {

		switch (WaitForMultipleObjects(2, hEvents, FALSE, 0) - WAIT_OBJECT_0) {
		case 0:
			switch (WaitForMultipleObjects(2, hEvents2, FALSE, INFINITE) - WAIT_OBJECT_0) {
			case 0:
				break;
			case 1:
				SetEvent(m_hConfirmEnd);
				return 0;
			}
			break;
		case 1:
			SetEvent(m_hConfirmEnd);
			return 0;
		default:
			break;
		}

		++m_StepNum;

		//Формирование структуры StepInfo
		StepInfo stepInfo;
		for (auto it = m_Robots_start.begin(); it != m_Robots_start.end(); ++it){
			RobotInfo robotInfo(it->second);
			stepInfo.robotsInfo.push_back(robotInfo);
		}
		stepInfo.stepNumber = m_StepNum;
		stepInfo.gameConfig = m_Config;
		for (auto it = m_ChargingStations.begin(); it != m_ChargingStations.end(); ++it){
			stepInfo.chargingStations.push_back(make_pair(it->x, it->y));
		}
		for (auto it = m_Maintenance.begin(); it != m_Maintenance.end(); ++it){
			stepInfo.maintenance.push_back(make_pair(it->x, it->y));
		}
		old_actionsList = new_actionsList;
		new_actionsList.clear();
		//stepInfo.actionsList = old_actionsList;

		
		nAliveRobots = 0;
		UINT size = m_Robots_start.size();
		for (UINT i = 0; i < size; ++i){
			Robot* cur_robot = m_Robots_ptr[i];
			Robot* self_robot = &m_Robots_self[m_Robots_ptr[i]->ID];
			if (!self_robot->Alive){
				continue;
			}
			++nAliveRobots;
			RobotActions robotActions;
			//RobotActions* pRobotActions = &robotActions;
			RobotActions* pRobotActions = new RobotActions;

			StepInfo* pSendStruct = new StepInfo(stepInfo);
			//sendStruct.pRobotActions = &robotActions;
			pSendStruct->pRobotActions = pRobotActions;
			pSendStruct->actionsList = old_actionsList;
			pSendStruct->ID = cur_robot->ID;
			ThreadStruct threadStruct(pSendStruct, cur_robot->robotFunction);
			setDirectoryForRobot(cur_robot);
			HANDLE hRobotThread = CreateThread(NULL, 0, robotThread, &threadStruct, NULL, NULL);
			DWORD state = WaitForSingleObject(hRobotThread, m_Config.T);
			delete pSendStruct;
			USHORT success_counter = 0;
			if (state == WAIT_OBJECT_0){
				//RobotActions tmp(*pRobotActions);
				new_actionsList.push_back(make_pair(cur_robot->ID, *pRobotActions));
				vector<Action*> actions = pRobotActions->getActions();
				bool isFailAction = false;
				for (auto this_action = actions.begin(); this_action != actions.end(); ++this_action) {
					if (!self_robot->Alive || isFailAction)
						break;
					switch ((*this_action)->getType()) {
					case MOVE:{
						Move* move = static_cast<Move*>(*this_action);
						int dx = move->dx;
						int dy = move->dy;
						doActionMove(cur_robot->ID, dx, dy) ? ++success_counter : isFailAction = true;
						break;
					}
					case ATTACK:{
						Attack* attack = static_cast<Attack*>(*this_action);
						UINT victimID = attack->victimID;
						doActionAttack(cur_robot->ID, victimID) ? ++success_counter : isFailAction = true;
						break;
					}
					case REDISTRIBUTION:{
						Redistribution* redistribution = static_cast<Redistribution*>(*this_action);
						UINT A = redistribution->A;
						UINT P = redistribution->P;
						UINT V = redistribution->V;
						doActionRedistribution(cur_robot->ID, A, P, V) ? ++success_counter : isFailAction = true;
						break;
					}
					default:
						break;
					}
				}
			}
			if (success_counter == 0){
				self_robot->E -= m_Config.dE_S;
			}
			delete pRobotActions;
		}

		//Обработка результатов шага
		for (UINT i = 0; i < size; ++i){
			Robot* start_robot = m_Robots_ptr[i];
			Robot* self_robot = &m_Robots_self[m_Robots_ptr[i]->ID];
			Robot* other_robot = &m_Robots_other[m_Robots_ptr[i]->ID];

			if (start_robot->Alive == false){
				continue;
			}

			UINT x = self_robot->x;
			UINT y = self_robot->y;
			start_robot->x = x;
			start_robot->y = y;

			//INT dScore = self_robot->score + other_robot->score - 2 * start_robot->score;
			//start_robot->score += dScore;

			INT dE = self_robot->E + other_robot->E - 2 * start_robot->E;
			start_robot->E += dE;
			if (start_robot->E > m_Config.E_max){
				start_robot->E = m_Config.E_max;
			}
			
			INT dA = self_robot->A - start_robot->A;
			start_robot->A += dA;
			INT dV = self_robot->V - start_robot->V;
			start_robot->V += dV;
			INT dP = self_robot->P + other_robot->P - 2 * start_robot->P;
			start_robot->P += dP;
			INT dL = self_robot->L + other_robot->L - 2 * start_robot->L;
			start_robot->L += dL;

			switch (m_pField->at(CPoint(x, y))->type)
			{
			case CHARGING_STATION:
				start_robot->E += m_Config.dE;
				if (start_robot->E > m_Config.E_max){
					start_robot->E = m_Config.E_max;
				}
				break;
			case MAINTENANCE:{
				INT dL;
				if (start_robot->L + m_Config.dL > m_Config.L_max){
					dL = m_Config.L_max - start_robot->L;
				}
				else{
					dL = m_Config.dL;
				}
				start_robot->L += dL;
				INT dV = m_Config.V_max - start_robot->V;
				if (dV <= dL){
					start_robot->V += dV;
					INT dA = (dL - dV) / 2;
					INT dP = dL - dV - dA;
					start_robot->A += dA;
					start_robot->P += dP;
				}
				else{
					start_robot->V += m_Config.dL;
				}
				break;
			}
			case BONES:{
				INT dL;
				if (start_robot->L + m_Config.dL > m_Config.L_max){
					dL = m_Config.L_max - start_robot->L;
				}
				else{
					dL = m_Config.dL;
				}
				start_robot->L += dL;
				INT dV = m_Config.V_max - start_robot->V;
				if (dV <= dL){
					start_robot->V += dV;
					INT dA = (dL - dV) / 2;
					INT dP = dL - dV - dA;
					start_robot->A += dA;
					start_robot->P += dP;
				}
				else{
					start_robot->V += m_Config.dL;
				}
				m_pField->at(CPoint(x, y))->pDeadRobot->L -= dL;
				break;
			}
			case CS_B:{
				start_robot->E += m_Config.dE;
				if (start_robot->E > m_Config.E_max){
					start_robot->E = m_Config.E_max;
				}
				INT dL;
				if (start_robot->L + m_Config.dL > m_Config.L_max){
					dL = m_Config.L_max - start_robot->L;
				}
				else{
					dL = m_Config.dL;
				}
				start_robot->L += dL;
				INT dV = m_Config.V_max - start_robot->V;
				if (dV <= dL){
					start_robot->V += dV;
					INT dA = (dL - dV) / 2;
					INT dP = dL - dV - dA;
					start_robot->A += dA;
					start_robot->P += dP;
				}
				else{
					start_robot->V += m_Config.dL;
				}
				m_pField->at(CPoint(x, y))->pDeadRobot->L -= dL;
				break;
			}
			case M_B:{
				INT dL;
				if (start_robot->L + m_Config.dL > m_Config.L_max){
					dL = m_Config.L_max - start_robot->L;
				}
				else{
					dL = m_Config.dL;
				}
				start_robot->L += dL;
				INT dV = m_Config.V_max - start_robot->V;
				if (dV <= dL){
					start_robot->V += dV;
					INT dA = (dL - dV) / 2;
					INT dP = dL - dV - dA;
					start_robot->A += dA;
					start_robot->P += dP;
				}
				else{
					start_robot->V += m_Config.dL;
				}

				if (start_robot->L + m_Config.dL > m_Config.L_max){
					dL = m_Config.L_max - start_robot->L;
				}
				else{
					dL = m_Config.dL;
				}
				start_robot->L += dL;
				dV = m_Config.V_max - start_robot->V;
				if (dV <= dL){
					start_robot->V += dV;
					INT dA = (dL - dV) / 2;
					INT dP = dL - dV - dA;
					start_robot->A += dA;
					start_robot->P += dP;
				}
				else{
					start_robot->V += m_Config.dL;
				}
				m_pField->at(CPoint(x, y))->pDeadRobot->L -= dL;
				break;
			}
			default:
				break;
			}

			if (start_robot->E <= 0 || self_robot->Alive == false || other_robot->Alive == false){
				start_robot->Alive = false;
				start_robot->E = 0;
				m_pField->at(CPoint(x, y))->pDeadRobot = start_robot;
				m_pField->addDeadRobot(CPoint(x, y), start_robot);
			}
			else{
				start_robot->RoundAliveSteps++;
			}

			*self_robot = *start_robot;
			*other_robot = *start_robot;
		}
	}

	endRound();

}


bool sortFunctionSteps(Robot* r1, Robot* r2)
{
	return (r1->RoundAliveSteps > r2->RoundAliveSteps);
}
bool sortFunctionScore(Robot* r1, Robot* r2)
{
	return (r1->RoundScore > r2->RoundScore);
}

void Game::endRound()
{
	

	m_curSel = 0;
	//m_ConditionsTopSel = 0;
	m_RoundResults.clear();
		
	USHORT size;

	//if (m_RoundNum < 5){
	//	size = m_Robots_ptr.size();
	//	m_RoundResults.resize(size);
	//	for (USHORT i = 0; i < size; ++i){
	//		m_RoundResults[i] = m_Robots_ptr[i];
	//	}
	//}
	//else{
	//	size = m_Robots_global.size();
	//	m_RoundResults.resize(size);
	//	for (USHORT i = 0; i < size; ++i){
	//		m_RoundResults[i] = &m_Robots_global[i];
	//	}
	//}
		

	size = m_Robots_ptr.size();
	m_RoundResults.resize(size);

	//Обработка результатов раунда
	for (USHORT i = 0; i < size; ++i){
		Robot* cur_robot = m_Robots_ptr[i];
		m_RoundResults[i] = cur_robot;

		//Начисление очков за убийства в раунде
		if (cur_robot->Alive){
			cur_robot->RoundScore = m_Config.K * cur_robot->RoundFrags + cur_robot->E;
		}
		else{
			cur_robot->RoundScore = (m_Config.K * cur_robot->RoundFrags) / 2;
		}

		m_Robots_global[cur_robot->ID]->E = cur_robot->E;
		m_Robots_global[cur_robot->ID]->TotalScore += cur_robot->RoundScore;
		m_Robots_global[cur_robot->ID]->Points += cur_robot->Points;
		m_Robots_global[cur_robot->ID]->TotalAliveSteps += cur_robot->RoundAliveSteps;
	}

	std::sort(m_RoundResults.begin(), m_RoundResults.end(), sortFunctionScore);

	INT cur_Score = m_RoundResults[0]->RoundScore;
	USHORT start_num = 0;
	USHORT end_num = 0;
	for (USHORT i = 1; i < size; ++i){
		if (m_RoundResults[i]->RoundScore == cur_Score){
			++end_num;
		}
		else {
			if (start_num != end_num){
				std::sort(m_RoundResults.begin() + start_num, m_RoundResults.begin() + end_num + 1, sortFunctionSteps);
			}
			start_num = end_num = i;
			cur_Score = m_RoundResults[i]->RoundScore;
		}
	}
	if (start_num != end_num){
		std::sort(m_RoundResults.begin() + start_num, m_RoundResults.begin() + end_num + 1, sortFunctionSteps);
	}


	USHORT rank = 1;
	USHORT rank_skip = 0;
	USHORT counter = 0;

	INT cur_score = m_RoundResults[0]->RoundScore;
	INT cur_steps = m_RoundResults[0]->RoundAliveSteps;

	for (auto it = m_RoundResults.begin(); it != m_RoundResults.end(); ++it){
		if (cur_score == (*it)->RoundScore && cur_steps == (*it)->RoundAliveSteps){
			++counter;
			rank_skip = 0;
		}
		else{
			rank_skip = counter;
			counter = 1;
			cur_score = (*it)->RoundScore;
			cur_steps = (*it)->RoundAliveSteps;
		}
		rank += rank_skip;

		(*it)->Rank = rank;

		if (rank == 1){
			(*it)->Points = 15;
		}
		else if (rank >= 2 && rank <= 6){
			(*it)->Points = 11;
		}
		else if (rank >= 7 && rank <= 13){
			(*it)->Points = 9;
		}
		else if (rank >= 14 && rank <= 22){
			(*it)->Points = 7;
		}
		else if (rank >= 23 && rank <= 33){
			(*it)->Points = 5;
		}

		m_Robots_global[(*it)->ID]->Points += (*it)->Points;
	}

	
	WriteLogToFile();
	clearRoundLogList();

	SetEvent(m_hEndDrawEvent);
	WaitForSingleObject(m_hConfirmEndDraw, INFINITE);

	SendMessage(m_pDialog->m_hWnd, WM_ON_ROUND_END, NULL, NULL);

	

	//CString resultStr = "Name\t\t\tEnergt\t\tScore\n";
	//for (auto it = m_RoundResults.begin(); it != m_RoundResults.end(); ++it){
	//	CString strE;
	//	strE.Format("%d", (*it)->E);
	//	CString strScore;
	//	strScore.Format("%d", (*it)->score);	
	//	resultStr += (*it)->Name.c_str() + CString("\t\t\t") + strE + CString("\t\t\t") + strScore + CString("\n");
	//}
	
	//m_ResultsStr.push_back(resultStr);
	
	//CResultDlg resultDlg;
	//resultDlg.DoModal();


}


void Game::clearRoundLogList()
{
	for (auto it = m_RoundLogList.begin(); it != m_RoundLogList.end(); ++it){
		if (it->pAction)
			delete it->pAction;
	}
	m_RoundLogList.clear();
}


void Game::draw()
{
	EnterCriticalSection(&m_CS_Draw);
	drawPlayingField();
	fillStatsField();	
	LeaveCriticalSection(&m_CS_Draw);
}




void Game::WriteResultsToFile()
{
	SetCurrentDirectory(m_Directory);
	ofstream f;
	f.open("Logs\\GameResults.txt");
	f << "-------------------GAME RESULTS----------------------" << endl;
	for (auto it = m_GameResults.begin(); it != m_GameResults.end(); ++it){
		f << it->first.c_str() << "\t\t" << it->second << endl;
	}
}


void Game::setDirectoryForRobot(Robot* pRobot)
{
	SetCurrentDirectory(m_Directory);
	CString ID;
	ID.Format("%d", pRobot->ID);
	CString path = "..\\FolderForRobots\\" + ID;

	CreateDirectory(path, NULL);

	SetCurrentDirectory(path);
}


BOOL DeleteFolder(LPCSTR szPath)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError, dwAttrs;
	BOOL bRes;
	int nLength;
	char cPath[MAX_PATH], cCurrentFile[MAX_PATH];

	if (szPath == NULL)
		return FALSE;

	if (lstrcmpi(szPath + 1, TEXT(":\\")) == 0 || szPath[0] == '\\' || szPath[0] == '\0')
		return FALSE;

	//Обьекта уже нету
	dwAttrs = GetFileAttributes(szPath);
	if (dwAttrs == INVALID_FILE_ATTRIBUTES)
		return TRUE;

	//Если обьект не директория 
	if (~dwAttrs & FILE_ATTRIBUTE_DIRECTORY)
		return FALSE;

	SetLastError(0);

	//Пробуем сразу удалить директорию
	bRes = RemoveDirectory(szPath);
	if (bRes == TRUE)
		return TRUE;

	if (bRes == FALSE  && GetLastError() != ERROR_DIR_NOT_EMPTY)
		return FALSE;

	nLength = lstrlen(szPath);

	if (nLength + lstrlen("\\*.*") + 1> MAX_PATH)
		return FALSE;

	if (szPath[nLength - 1] == '\\')
		wsprintf(cPath, "%s*.*", szPath);
	else
		wsprintf(cPath, "%s\\*.*", szPath);

	hFind = FindFirstFile(cPath, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	lstrcpy(cPath, szPath);

	if (cPath[nLength - 1] == '\\')
		cPath[nLength - 1] = TEXT('\0');

	do
	{
		//Опускаем начальные . и ..
		if (lstrcmpi(FindFileData.cFileName, ".") == 0 || lstrcmpi(FindFileData.cFileName, "..") == 0)
			continue;

		//Пропускаем слишком длинные имена файлов
		if (lstrlen(cPath) + lstrlen("\\") + lstrlen(FindFileData.cFileName) + 1 > MAX_PATH)
			continue;

		wsprintf(cCurrentFile, "%s\\%s", cPath, FindFileData.cFileName);
		//Удаляем папку
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{    //Сбрасываем аттрибуты
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			{
				FindFileData.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
				SetFileAttributes(cCurrentFile, FindFileData.dwFileAttributes);
			}

			bRes = DeleteFolder(cCurrentFile);
		}
		//Иначе удаляем файл
		else
		{

			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
				(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
				SetFileAttributes(cCurrentFile, FILE_ATTRIBUTE_NORMAL);

			DeleteFile(cCurrentFile);
		}
	} while (FindNextFile(hFind, &FindFileData));

	dwError = GetLastError();

	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);

	if (dwError != ERROR_NO_MORE_FILES)
		return FALSE;

	bRes = RemoveDirectory(szPath);

	return bRes;
}

bool Game::clearFolderForRobots()
{
	SetCurrentDirectory(m_Directory);
	if (DeleteFolder("..\\FolderForRobots")){
		if (CreateDirectory("..\\FolderForRobots", NULL))
			return true;
	}
	return false;
}
