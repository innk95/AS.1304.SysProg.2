#pragma once
#include "stdafx.h"
#include "Field.h"
//#include "Application.h"

#define WM_ON_GAME_END		WM_USER+1
#define WM_ON_ROUND_END		WM_USER+2
#define	RENDER_PAUSE		30
#define STANDART_ROBOT_DLL	"StandartRobot.dll"

typedef void(*RobotFunction)(StepInfo* stepInfo);

enum Color
{
	RED = RGB(255, 0, 0),
	GREEN = RGB(0, 255, 0),
	BLUE = RGB(0, 0, 255),
	YELLOW = RGB(255, 255, 0),
	PURPLE = RGB(255, 0, 255),
	GRAY = RGB(240, 240, 240),
	WHITE = RGB(255, 255, 255)
};


class ConfigStore : public CObject
{
public:
	DECLARE_SERIAL(ConfigStore);
	void Serialize(CArchive&);

	void addRoundConfig(GameConfig);
	void changeRoundConfig(UINT roundNumber, GameConfig);
	GameConfig getRoundConfig(UINT numberRound);
private:
	vector<GameConfig> m_Store;

};

struct Robot :
	public RobotInfo
{
	CString filePath;
	RobotFunction robotFunction = NULL;
	HINSTANCE hDll = NULL;	
	USHORT startRound;
	UINT RoundAliveSteps;
	UINT TotalAliveSteps;
	UINT Points;
	UINT Rank;
	BOOL isPlayer;
	//UINT maxDistance = -1;	
};


struct ActionLog	
{
	Action* pAction;
	bool result;
	UINT robotID;
	UINT step;
};


class Game
{	
	CString m_File_path = "list.txt";

	map<UINT, Robot> m_Robots_start;	//Состояния роботов на начало шага
	map<UINT, Robot> m_Robots_self;		//Состояния роботов в результате собственных действий текущего шага
	map<UINT, Robot> m_Robots_other;	//Состояния роботов в результате внешних действий текущего шага
	vector<Robot*> m_Robots_ptr;

	map < string, vector<Robot*> > m_Authors;
	
	vector<Robot*> m_Robots_global;

	list<ActionLog> m_RoundLogList;

	Field* m_pField = NULL;
		
	UINT m_CellSize = 20;
	
	CPoint m_CurCell = CPoint(0, 0);

	vector<CPoint> m_ChargingStations;
	vector<CPoint> m_Maintenance;
	
	void setDefaultParameters();
	void clearArea(COLORREF color, UINT x1, UINT y1, UINT x2, UINT y2);
	void drawStations();
	void drawRobots();

	CPoint normalizeСoordinates(CPoint pos);
	UINT getDistance(CPoint pos1, CPoint pos2);
	UINT getDistance(UINT x1, UINT y1, UINT x2, UINT y2);

	HANDLE m_hPlayThread; 
	HANDLE m_hDrawThread;
	HANDLE m_hEndEvent;
	HANDLE m_hConfirmEnd;

	HANDLE m_hPausePlayEvent;
	//HANDLE m_hConfirmPausePlay;
	HANDLE m_hPauseDrawEvent;
	//HANDLE m_hConfirmPauseDraw;

	HANDLE m_hResumePlayEvent;
	HANDLE m_hResumeDrawEvent;

	HANDLE m_hEndDrawEvent;
	HANDLE m_hConfirmEndDraw;

	void drawStatsField(COLORREF color);

	//clock_t m_lastRender;

	USHORT m_RoundNum = 0;
	UINT m_StepNum;

	BOOL m_isNormalEnd;
	
	vector<list<ActionLog>*> m_RoundsLog;

	CRITICAL_SECTION m_CS_Draw;
	//CRITICAL_SECTION m_CS_ChangeVar;

	ConfigStore* m_ConfigStore;
	
	char* m_Directory;
	
	vector<COLORREF> m_Colors;

	void setDirectoryForRobot(Robot* pRobot);

	bool clearFolderForRobots();

	INT startRound();
	void endRound();
	void clearRoundLogList();

	bool doActionMove(UINT robot_id, INT dx, INT dy);
	bool doActionRedistribution(UINT robot_id, UINT A, UINT P, UINT V);
	bool doActionAttack(UINT robot_id, UINT victim_id);
	
	int m_curSel = 0;
	USHORT m_ConditionsTopSel = 0;

	bool setStations();

	
	void WriteLogToFile();
	void WriteResultsToFile();
public:
	
	void incConditionsTopSel();
	void decConditionsTopSel();

	GameConfig m_Config;
	vector<Robot*> m_RoundResults;
	vector<pair<string, UINT>> m_GameResults;

	CPoint m_Border_start;
	CPoint m_Border_end;
	CPoint m_Conditions_start;
	CPoint m_Conditions_end;
	CSize m_Conditions_txtSize;
	CDC* m_pDC;
	CDialogEx* m_pDialog;
	
	Game();
	~Game();

	void setConfigStore(ConfigStore*);
	ConfigStore* getConfigStore();
	GameConfig getRoundConfig(UINT numberRound);

	void draw();
	void drawPlayingField(bool flag = 0);
	void fillStatsField(bool flag = 0);

	void drawСonditions();

	void setCurSelWithY(int y);
		
	bool incCellSize();
	bool decCellSize();
	void setCurCell(CPoint position);
	CPoint getCurCell();
	
	void setFilePath(CString path);
	CString getFilePath();
	Robot getRobotInfo(USHORT number);
	bool setRobotsInfo(CString filePath);

	void start();
	DWORD WINAPI pause();
	DWORD WINAPI resume();
	DWORD WINAPI playThread(LPVOID lpParam);
	DWORD WINAPI drawThread(LPVOID lpParam);	
	bool end();
	
};





